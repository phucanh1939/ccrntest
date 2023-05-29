# CCRN Test

Test Cocos Creator game importer for React Native npm package [ccrn](https://github.com/phucanh1939/ccrn.git)

In this test we will import cocos engine into a simple React Native app. We have 2 games: one game will be download from remote server and another will be shipped within the app (load locally)

- `space-shooter`: This game assets will be upload on a server - can be found at `server/public/games/space-shooter.zip`

- `happy-pop`: This game will be shipped within the app - can be found at `android/app/src/main/assets/happy-pop/`

# How to run

Install ccrn 
```console
$ npm install ccrn -g
```

Install npm packages
```console
$ npm install
```

Import cocos engine
```console
$ ccrn init
```

Run server
```console
$ cd server
$ npm start
```

Start react-native
```console
$ npx react-native start
```

Build & run android
```
$ npx react-native run-android
```

