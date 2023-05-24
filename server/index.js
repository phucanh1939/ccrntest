const express = require('express');
const morgan = require('morgan');
const path = require('path');
const fs = require('fs');
const app = express();
const port = 3000;
const GAMES_DIR = path.join(__dirname, 'games');
const VERION_FILE_PATH = './public/games/version.json';
let versionConfig = {};

function readJsonFile(filePath) {
    let data = fs.readFileSync(filePath);
    return JSON.parse(data);
};

function getGameVersion(gameId) {
    return versionConfig[gameId] || 0;
};

app.use(morgan('combined'));
app.use('/static', express.static(path.join(__dirname, 'public')));

app.get('/update-game', (req, res) => {
    // TODO check client version and only send file to client if client version < current version of the game
    const gameId = req.query.gameId;
    if (!gameId) return;
    const fileName = gameId + ".zip";
    res.download(fileName, {root: GAMES_DIR}, (err) => {
        if (!err) return; // file sent
        if (err.status !== 404) return next(err); // non-404 error
        // file for download not found
        res.statusCode = 404;
        res.send(JSON.stringify({
            "errorCode": 404,
            "message": "Game file not found"
        }));
    });
});

app.get('/check-for-update', (req, res) => {
    const gameId = req.query.gameId;
    const localVersion = req.query.version;
    const remoteVersion = getGameVersion(gameId);
    const needUpdate = localVersion < remoteVersion;
    res.send(JSON.stringify({gameId: gameId, needUpdate: needUpdate, version: remoteVersion}));
});

versionConfig = readJsonFile(VERION_FILE_PATH);
console.log("Read version file: " + JSON.stringify(versionConfig));

fs.watch(VERION_FILE_PATH, (eventType, fileName) => {
    versionConfig = readJsonFile(VERION_FILE_PATH);
    console.log("Version file has updated. New config is: " + JSON.stringify(versionConfig));
});

app.listen(port, () => {
    console.log(`Example app listening on port ${port}`);
});

