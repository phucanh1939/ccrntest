# Step by step to import cocos engine to react-native ios

1. Create group ccrn/resource in xcode
- Define Group in project file
- Add this group hash to project node or parent group

2. Drag game bundle to this group (game bundle will be store at ./games/<game-id>/)
- Add files to File Reference & Build file in project file
- Add reference in group defined in step #1

3. Add build phase (compile source) to compile .cpp files in the main target (the application one)

4. Add target dependencies for the main target: cocos_engine, zero_check, boost container

5. Add files, build setting, build phases for these targets (cocos_engine, zero_check, boost container)


