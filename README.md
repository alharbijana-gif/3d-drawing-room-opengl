# 3D Interactive Drawing Room

An interactive 3D art-studio scene written in **C++ with OpenGL and GLUT**.
Course project for Computer Graphics.

## Features
- Room, easel, stool and wall artwork built with hierarchical transformations (push/pop matrix).
- Two light sources (warm ceiling light and cool secondary light).
- Different materials: matte floor and a reflective mirror wall.
- Orbit camera controlled by mouse and keyboard.
- Right-click menu for camera presets and light placement.

## Controls
| Input | Action |
|---|---|
| Arrow keys / mouse drag | Orbit the camera |
| `+` / `-` | Zoom in / out |
| `U` `O` / `I` `K` / `J` `L` | Move the camera target left-right / down-up / forward-back |
| `R` | Reset the view |
| `Esc` | Exit |
| Right click | Camera views and lighting menu |

## Build and run
macOS:
```bash
g++ drawing_room.cpp -o drawing_room -framework OpenGL -framework GLUT -Wno-deprecated-declarations
./drawing_room
```
Linux (Ubuntu / WSL2):
```bash
sudo apt install freeglut3-dev
g++ drawing_room.cpp -o drawing_room -lGL -lGLU -lglut
./drawing_room
```

## Skills shown
C++, OpenGL, GLUT, 3D transformations, lighting and materials, event-driven input.
