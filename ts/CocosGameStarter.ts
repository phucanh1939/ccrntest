import {NativeModules} from 'react-native';
const {CocosGameStarter} = NativeModules;
interface CocosGameStarterInterface {
  startGame(name: string): void;
}
export default CocosGameStarter as CocosGameStarterInterface;