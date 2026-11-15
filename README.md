# The Little Light - C++ / OpenGL computer graphics animation

A 5 minute, dialogue-free animation of the story "The Little Light" (Lumi, the tiny creature that
gives away its light). Everything on screen is drawn by code: no images, no models, no engine.

## Build and run

**Linux (Ubuntu/Debian)**
```
sudo apt install g++ cmake libglfw3-dev libgl1-mesa-dev
cmake -S . -B build && cmake --build build
./build/little_light
```
or without CMake: `g++ -std=c++17 -O2 main.cpp -o little_light -lglfw -lGL`

**macOS**: `brew install glfw` then `g++ -std=c++17 -O2 main.cpp -o little_light -lglfw -framework OpenGL`

**Windows**: install GLFW with vcpkg (`vcpkg install glfw3`) and open the folder in Visual Studio
(CMake project), or with MSYS2: `pacman -S mingw-w64-x86_64-glfw` then
`g++ -std=c++17 -O2 main.cpp -o little_light.exe -lglfw3 -lopengl32 -lgdi32`

**No OpenGL at all (pure C++)**: `g++ -std=c++17 -O2 -DNO_WINDOW main.cpp -o little_light_software`

## Controls (window)
| Key | Action |
|---|---|
| SPACE | pause / resume |
| LEFT / RIGHT | jump back / forward 5 seconds |
| UP / DOWN | double / halve playback speed |
| R | switch renderer: **OpenGL** <-> **pure C++ software rasteriser** |
| HOME | restart |
| P | save `screenshot.ppm` |
| ESC | quit |

## Command line (no window needed)
```
./little_light_software --shot 218 frame.ppm         # one frame at t = 218 s
./little_light_software --video --fps 30 | ffmpeg -f rawvideo -pix_fmt rgb24 -s 1280x720 -r 30 -i - -c:v libx264 -pix_fmt yuv420p out.mp4
./little_light --glshot 218 frame.ppm                # one frame rendered through OpenGL
```

## How the project follows the course stages

| Stage | File | What it contains |
|---|---|---|
| **CP1 - technical foundation (pure C++)** | `cp1_raster.h` | Bresenham line, midpoint circle (filled), scanline polygon fill with 4x anti-aliasing, alpha blending, additive radial glow, float framebuffer |
| **CP2 - transformation and animation (pure C++)** | `cp2_transform.h` | 3x3 homogeneous matrices (translate, rotate, scale, concatenation), matrix stack (`push/pop`), cubic Bezier ribbons, keyframe animation curves (monotone cubic Hermite), hierarchical animation (Lumi's tuft, feet, eyes; tree branches) |
| **CP3 - OpenGL integration** | `cp3_opengl.h` | `GLRenderer`: same scene through OpenGL (triangle fans, alpha + additive blending, MSAA), GLFW window, texture presenter for the software renderer |
| **Prototype / Final** | `scene.h`, `main.cpp` | The complete film, timeline, camera, characters, lighting, title and final message |

Design idea: the scene only talks to an abstract `Renderer` (polygon, disc, glow, line). The CP1
software rasteriser and the CP3 OpenGL renderer both implement it, so the same film can be
played by either one (press **R**). Every transformation is computed by our own `Mat3`, not by
`glTranslate/glRotate`.

## Film timeline (300 s)
| Time | Scene |
|---|---|
| 0:00-0:40 | Title, dark city, Lumi wakes and wanders |
| 0:40-1:20 | Lumi finds the trembling creature under the broken streetlamp and shares its light |
| 1:20-2:10 | The old robot's dead lamp; the bird trapped under branches |
| 2:10-3:00 | The wilting plant; the lost child in the alley. Lumi grows dimmer |
| 3:00-4:00 | The square and the dark tree: fear, hesitation, the final light, Lumi falls dark |
| 4:00-4:40 | The friends return, each carrying a tiny light, and gather around Lumi |
| 4:40-5:00 | Spark, Lumi wakes, the city lights up, final message |
