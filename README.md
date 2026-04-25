# PuzzleGame — OpenGL Math Crossword Puzzle

A drag-and-drop math puzzle game inspired by the screenshot, built with **OpenGL + GLUT/FreeGLUT**.
## Controls

| Action | Input |
|--------|-------|
| Pick up tile | Left-click & hold |
| Place tile | Release over a `?` slot |
| Pick up from slot | Click a filled slot, then drag |
| Restart | Press **R** |
| Quit | Press **Esc** |

## Visual feedback

- 🟩 **Green** cell → correct answer placed  
- 🟥 **Red** cell → wrong answer placed  
- 🟦 **Teal** cell → fixed given numbers (teal row highlight)  
- **?** → empty slot waiting for a tile

## Build

### Linux / macOS

```bash
# Install deps (Ubuntu/Debian)
sudo apt install freeglut3-dev

# Compile directly
g++ main.cpp -o puzzle -lGL -lGLU -lglut -lm

# Or with CMake
mkdir build && cd build
cmake .. && make

./puzzle
```

### Windows (MinGW)

```bash
g++ main.cpp -o puzzle.exe -lopengl32 -lglu32 -lfreeglut
```

### macOS (Homebrew)

```bash
brew install freeglut
g++ main.cpp -o puzzle -framework OpenGL -framework GLUT
```
