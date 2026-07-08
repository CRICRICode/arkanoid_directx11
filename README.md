
# Arkanoid DirectX 11

A simplified Arkanoid-style game developed in C++ with DirectX 11 for the Graphics course.

The project implements a single playable level with a paddle, a round ball, colored destructible bricks, 2D collision handling, game over logic, temporary modifiers, and automatic level reset when all bricks are destroyed.

## Features

- Single playable level.
- Bricks are destroyed with one hit.
- Random brick colors, with at least five different colors on each level reset.
- Automatic level reset when all bricks are destroyed.
- One life only: if the paddle misses the ball, the game ends.
- Manual restart after game over with `Enter`.
- Paddle movement through keyboard input.
- Optional temporary modifiers randomly released by some bricks.
- 2D rendering using DirectX 11 and HLSL shaders.

## Controls

| Key | Action |
| --- | --- |
| `Left Arrow` / `A` | Move the paddle left |
| `Right Arrow` / `D` | Move the paddle right |
| `Enter` | Restart the game after game over |
| `Esc` | Close the application |

## Modifiers

Some bricks can randomly release temporary modifiers:

- bigger ball;
- inverted controls and rotated scene;
- longer paddle;
- increased ball and paddle speed.

Modifiers are optional features beyond the base project requirements.

## Requirements

- Windows.
- Visual Studio 2022.
- MSVC v143 toolset.
- Windows SDK 10.
- DirectX 11 support.

## Build

Open the solution file:

```text
directx11_1.sln
````

Recommended configuration:

```text
Debug | x64
```

To build from the terminal, with MSBuild available:

```powershell
MSBuild.exe directx11_1.sln /p:Configuration=Debug /p:Platform=x64 /m
```

The executable is generated in:

```text
x64\Debug\directx11_1.exe
```

## Project Structure

* `applicationclass.*`: game logic, game state, collision handling, and object rendering.
* `d3dclass.*`: DirectX 11 initialization and resource management.
* `modelclass.*`: reusable geometry for rectangles and circles.
* `colorshaderclass.*`: shader compilation and matrix transfer to the GPU.
* `inputclass.*`: keyboard state management.
* `systemclass.*`: Win32 window management and main application loop.
* `color.vs` / `color.ps`: HLSL shaders.

## Notes

This project was developed as an educational exercise. It prioritizes a simple and readable structure while keeping game logic, DirectX management, and the application loop separated.

## License

MIT License

Copyright (c) 2026 Giuseppe Cricrì

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files, to deal in the software
without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the
software, and to permit persons to whom the software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
