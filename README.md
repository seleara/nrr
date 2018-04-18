# NewRockRaiders

NewRockRaiders (NRR) is a game engine with the goal of allowing one to play Rock Raiders on modern systems at arbitrary resolutions, as well as hopefully making modding easier in the future.

## Setup

For now, there's only a Visual Studio project available, no CMake or anything like that. So in order to build it, you will have to edit the project settings and change the include and library paths to match your installation.

First, you need to copy the files `LegoRR0.wad` and `LegoRR1.wad`, as well as the `AVI` and `Sounds` folders from your original Rock Raiders game folder and paste them into the `NewRockRaiders/data` folder.
Then, while using `NewRockRaiders/` as the working directory, run `NewRockRaiders.exe`.

## License

NRR is licensed under the MIT license.

## Dependencies

NRR uses the following libraries:

- Dear ImGui (https://github.com/ocornut/imgui)
- GLFW (http://www.glfw.org/)
- GLEW (http://glew.sourceforge.net/)
- GLM (http://glm.g-truc.net)
- stb_image (https://github.com/nothings/stb)
