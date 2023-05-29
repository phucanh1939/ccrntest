/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You
 shall not use Cocos Creator software for developing other software or tools
 that's used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to
 you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "Game.h"
#include <jni.h>
#include <application/ApplicationManager.h>
#include <audio/android/cutils/log.h>
#include <FileUtils.h>
#include <base/UTF8.h>
#include <rapidjson/document.h>
#include <network/Downloader.h>
#include <base/ZipUtils.h>
#include <storage/local-storage/LocalStorage.h>
#include "platform/java/jni/JniHelper.h"

#ifndef GAME_NAME
#define GAME_NAME "CocosGame";
#endif

#ifndef SCRIPT_XXTEAKEY
#define SCRIPT_XXTEAKEY "";
#endif

Game::Game() = default;

int Game::init()
{
    CC_LOG_INFO("Game::init()");
    _windowInfo.title = GAME_NAME;
    // configurate window size
    // _windowInfo.height = 600;
    // _windowInfo.width  = 800;

#if CC_DEBUG
    _debuggerInfo.enabled = true;
#else
    _debuggerInfo.enabled = false;
#endif
    _debuggerInfo.port = 6086;
    _debuggerInfo.address = "0.0.0.0";
    _debuggerInfo.pauseOnStart = false;

    _xxteaKey = SCRIPT_XXTEAKEY;

    CC_LOG_INFO("[Game] init");
    BaseGame::init();
    return 0;
}

void Game::onPause() { BaseGame::onPause(); }

void Game::onResume() { BaseGame::onResume(); }

void Game::onClose() { BaseGame::onClose(); }

void Game::startGame(const std::string &gameId)
{
    CC_LOG_INFO("Game::startGame(%s)", gameId.c_str());
    _gameId = gameId;
    if (hasGame(gameId)) {
        runGame(gameId);
        return;
    }
    checkForUpdate(gameId);
}

bool Game::hasGame(const std::string &gameId)
{
    const auto fileUtils = cc::FileUtils::getInstance();
    const auto cachePath = fileUtils->getWritablePath() + "games/" + gameId;
    CC_LOG_INFO("Game::hasGame(%s) - %d", gameId.c_str(), fileUtils->isDirectoryExist(gameId));
    CC_LOG_INFO("Game::hasGame(%s) - %d", cachePath.c_str(), fileUtils->isDirectoryExist(cachePath));
    return fileUtils->isDirectoryExist(gameId) || fileUtils->isDirectoryExist(cachePath);
}

void Game::runGame(const std::string &gameId)
{
    CC_LOG_INFO("Game::runGame(%s)", gameId.c_str());

    // Update Search Paths for current game
    const auto fileUtils = cc::FileUtils::getInstance();
    fileUtils->setSearchPaths(fileUtils->getOriginalSearchPaths());
    CC_LOG_INFO("Game::setSearchPaths(fileUtils->getOriginalSearchPaths())");
    const auto gamePath = fileUtils->getWritablePath() + "games/" + gameId;
    fileUtils->addSearchPath(gameId);
    fileUtils->addSearchPath(gamePath);
    CC_LOG_INFO("Game::addSearchPath(%s)", gamePath.c_str());

    // Run JS Entry
    runScript("jsb-adapter/web-adapter.js");
    runScript("main.js");
    CC_LOG_INFO("Game::startGames runJS done");
}

void Game::checkForUpdate(const std::string &gameId)
{
    // Since we run the game on simulator so it need to connect to "localhost" of our PC
    // The local host address on simulator is "10.0.2.2"
    CC_LOG_DEBUG("Game::checkForUpdate(%s)", gameId.c_str());
    ccstd::string versionStr;
    const auto versionKey = getVersionKey(gameId);
    CC_LOG_DEBUG("Game::checkForUpdate(%s) versionKey = %s", gameId.c_str(), versionKey.c_str());
    const auto hasVersion = localStorageGetItem(versionKey, &versionStr);
    _localVersion = hasVersion ? std::stoi(versionStr) : -1;
    CC_LOG_DEBUG("Game::checkForUpdate(%s) localVersion = %d", gameId.c_str(), _localVersion);
    const auto& url = cc::StringUtils::format("http://10.0.2.2:3000/check-for-update?gameId=%s&version=%d", gameId.c_str(), _localVersion);
    cc::network::HttpRequest *request = new (std ::nothrow) cc::network::HttpRequest();
    request->setUrl(url);
    request->setRequestType(cc::network::HttpRequest::Type::GET);
    request->setResponseCallback(CC_CALLBACK_2(Game::onCheckForUpdateResponse, this));
    cc::network::HttpClient::getInstance()->send(request);
}

void Game::onCheckForUpdateResponse(cc::network::HttpClient *sender, cc::network::HttpResponse *response)
{
    if (!response || !response->isSucceed()) {
        const auto errorCode = response ? response->getResponseCode() : -1939L;
        CC_LOG_WARNING("Game::onCheckForUpdateResponse(%s) Failed!, error code is %ld", _gameId.c_str(), errorCode);
        runGame(_gameId);
        return;
    }
    const auto responseData = response->getResponseData();
    responseData->push_back(0);
    const auto data = response->getResponseData()->data();
    rapidjson::Document document;
    CC_LOG_WARNING("Game::onCheckForUpdateResponse() jsonString %s", data);
    document.Parse(data);
    if (document.HasParseError()) {
        CC_LOG_WARNING("Game::onCheckForUpdateResponse(%s) ParseJsonFailed", _gameId.c_str());
        runGame(_gameId);
        return;
    }
    const std::string gameId = document.HasMember("gameId") ? document["gameId"].GetString() : "";
    const auto needUpdate = document.HasMember("needUpdate") ? document["needUpdate"].GetBool() : false;
    _remoteVersion = document.HasMember("version") ? document["version"].GetInt() : -1;
    CC_LOG_DEBUG("gameId: %s", gameId.c_str());
    CC_LOG_DEBUG("needUpdate: %d", needUpdate);
    CC_LOG_DEBUG("remoteVersion: %d", _remoteVersion);
    if (needUpdate) {
        updateGame(gameId);
    } else {
        runGame(gameId);
    }
}

void Game::updateGame(const std::string &gameId)
{
    CC_LOG_INFO("Game::updateGame(%s)", gameId.c_str());
    downloadGame(gameId);
}

void Game::downloadGame(const std::string &gameId)
{
    const auto fileName = gameId + ".zip";
    const auto url = cc::StringUtils::format("http://10.0.2.2:3000/static/games/%s", fileName.c_str());
    CC_LOG_INFO("Game::downloadGame(%s)", url.c_str());
    const auto fileUtils = cc::FileUtils::getInstance();
    const auto gameFolder = fileUtils->getWritablePath() + "games/";
    if (!fileUtils->isDirectoryExist(gameFolder)) {
        fileUtils->createDirectory(gameFolder);
    }
    const auto filePath = gameFolder + fileName;
    CC_LOG_INFO("Game::downloadGame() filePath = %s", filePath.c_str());
    cc::network::Downloader *downloader = new (std::nothrow) cc::network::Downloader();
    downloader->onTaskProgress = ([this] (const cc::network::DownloadTask& task, int64_t bytesReceived, int64_t totalBytesReceived, int64_t totalBytesExpected) {
        CC_LOG_DEBUG("Downloaded %d/%d bytes", totalBytesReceived, totalBytesExpected);
    });

    downloader->onFileTaskSuccess = ([this] (const cc::network::DownloadTask& task) {
        CC_LOG_DEBUG("File Downloaded!!!!");
        unzip();
        runGame(_gameId);
        updateVersion();
    });

    downloader->onTaskError = ([this] (const cc::network::DownloadTask& task, int errorCode, int errorCodeInternal, const std::string& errorStr) {
        CC_LOG_WARNING("Download Error %d", errorCode);
        runGame(_gameId);
    });

    downloader->createDownloadTask(url, filePath);
}

void Game::unzip()
{
    const auto zipFileName = _gameId + ".zip";
    const auto fileUtils = cc::FileUtils::getInstance();
    const auto folderPath = fileUtils->getWritablePath() + "games/";
    const auto gameFolderPath = folderPath + _gameId;
    const auto filePath = folderPath + zipFileName;
    // Delete old files before unzip new version
    if (fileUtils->isDirectoryExist(gameFolderPath)) {
        CC_LOG_DEBUG("Remove old folder %s", gameFolderPath.c_str());
        fileUtils->removeDirectory(gameFolderPath);
    }
    cc::ZipFile zFile = cc::ZipFile(filePath);
    std::string fileName = zFile.getFirstFilename();
    std::string file = fileName;
    unsigned int fileSize;
    unsigned char* data = zFile.getFileData(fileName, &fileSize);
    while (data != nullptr)
    {
        std::string fullFileName = folderPath + file;
        if(fullFileName[fullFileName.size() - 1] == '/'){
            CC_LOG_DEBUG("create dir: %s",fullFileName.c_str());
            fileUtils->createDirectory(fullFileName);
            free(data);
            fileName = zFile.getNextFilename();
            file = fileName;
            data = zFile.getFileData(fileName, &fileSize);
            continue;
        }
        CC_LOG_DEBUG("write file: %s",fullFileName.c_str());
        FILE *fp = fopen(fullFileName.c_str(), "wb");
        if (fp)
        {
            fwrite(data, 1, fileSize, fp);
            fclose(fp);
        }
        free(data);
        fileName = zFile.getNextFilename();
        file = fileName;
        data = zFile.getFileData(fileName, &fileSize);
    }
    fileUtils->removeFile(filePath);
}

void Game::updateVersion()
{
    _localVersion = _remoteVersion;
    const auto versionKey = getVersionKey(_gameId);
    const auto versionString = cc::StringUtils::toString(_localVersion);
    CC_LOG_DEBUG("Update %s version to %s", _gameId.c_str(), versionString.c_str());
    localStorageSetItem(versionKey, versionString);
}

std::string Game::getVersionKey(const std::string &gameId) {
    return gameId + "_version";
}

extern "C"
{
    JNIEXPORT void JNICALL Java_com_ccrn_CCRNActivity_startGame(JNIEnv* env, jobject activity, jstring game_id)
    {
        auto app = std::static_pointer_cast<Game>(CC_APPLICATION_MANAGER()->getCurrentApp());
        app->startGame(cc::JniHelper::jstring2string(game_id));
    }

    JNIEXPORT void JNICALL Java_com_ccrn_CCRNActivity_restartGame(JNIEnv* env, jobject activity)
    {
        auto app = std::static_pointer_cast<Game>(CC_APPLICATION_MANAGER()->getCurrentApp());
        app->restart();
    }
}

CC_REGISTER_APPLICATION(Game);
