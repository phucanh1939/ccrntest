import {NativeModules} from 'react-native';
const {CCRNModule} = NativeModules;
interface CCRNModuleInterface {
  startGame(name: string): void;
}
export default CCRNModule as CCRNModuleInterface;