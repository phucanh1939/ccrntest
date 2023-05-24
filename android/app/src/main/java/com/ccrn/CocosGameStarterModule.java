package com.ccrn;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;

import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;

public class CocosGameStarterModule extends ReactContextBaseJavaModule {
    public static String GAME_ID;

    CocosGameStarterModule(ReactApplicationContext context) {
        super(context);
    }

    @NonNull
    @Override
    public String getName() {
        return "CocosGameStarter";
    }

    @ReactMethod
    public void startGame(String gameId) {
        GAME_ID = gameId;
        Activity currentActivity = getCurrentActivity();
        Intent intent = new Intent(currentActivity, CocosGameActivity.class);
        currentActivity.startActivity(intent);
    }
}
