# VEngine - Vagrant Tactics 

![image](https://user-images.githubusercontent.com/45758254/147405666-9535d71b-c29c-4914-8652-f28f04de4b9d.png)

## Build Notes
* Visual Studio 2022 17.0.5
* Blender 2.93.7

## Packages

DirectXTK (directxtk_desktop_win10) ver. 2021.8.2.1 - https://github.com/microsoft/DirectXTK

## Extensions

* Qt VS Tools ver. 2.7.1.20 - https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools-19123
(Currently the engine is using VS2022 tools, the Qt VS extension holds over but VS2022 can't create new Qt projects but builds and settings still work.)

## Third party libs:

* Physx ver 4.1.2 | Throw FBXSDK headers/source into Code/Physx - https://github.com/NVIDIAGameWorks/PhysX (used https://vcpkg.io/en/index.html to download SDK [package name is physx:x64-windows])

* FBXSDK ver. 2020.1 | Throw FBXSDK headers/source into Code/FbxSdk - https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-1

* Qt ver. 5.15.0 - https://download.qt.io/official_releases/qt/5.15/

(The ImGui libs are already included in the repo)
* Dear ImGui - https://github.com/ocornut/imgui
* ImGuizmo - https://github.com/CedricGuillemet/ImGuizmo

## Uses
* https://opengameart.org/content/jrpg-style-ui-sounds
* https://opengameart.org/content/rpg-sound-pack
* https://opengameart.org/content/footsteps-leather-cloth-armor
* https://opengameart.org/content/gunshot-sounds
* https://opengameart.org/content/2-metal-weapon-clicks

## Project References
* FFTactics Maps: https://www.cavesofnarshe.com/fft/maps/index.php
