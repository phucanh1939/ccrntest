import React from 'react';
import {View, Button} from 'react-native';
import CCRNModule from './ts/CCRNModule';

function App(): JSX.Element {
  return (
    <View>
      <Button
        onPress={() => {
          CCRNModule.startGame("happy-pop");
        }}
        title={'happy-pop-1'}
      />
      <Button
        onPress={() => {
          CCRNModule.startGame("space-shooter");
        }}
        title={'space-shooter'}
      />
    </View>
  );
}

export default App;
