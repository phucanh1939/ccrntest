import React from 'react';
import {View, Button} from 'react-native';
import CocosGameStarter from './ts/CocosGameStarter';

function App(): JSX.Element {
  return (
    <View>
      <Button
        onPress={() => {
          CocosGameStarter.startGame("happy-pop");
        }}
        title={'happy-pop-1'}
      />
      <Button
        onPress={() => {
          CocosGameStarter.startGame("space-shooter");
        }}
        title={'space-shooter'}
      />
    </View>
  );
}

export default App;
