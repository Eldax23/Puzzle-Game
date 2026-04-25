# MathGrid — OpenGL Math Crossword Puzzle

A drag-and-drop math puzzle game inspired by the screenshot, built with **OpenGL + GLUT/FreeGLUT**.

## Puzzle Layout (7 × 7 grid)

```
      [A]  +       =  28
       ×        ×  +      +
  1    ×   12   =  12     11
  =        /      +       =
  12       32   + [B] =
               =      =
  49  -  [C]  =  [D]
```

Drag the number tiles from the tray at the bottom into the **?** blanks.

| Blank | Answer | Equation check |
|-------|--------|----------------|
| A (row 0, col 1) | **7** | 7 + 21 = 28 (col 1 down: 7 × 1 = 7 × ... ) |
| B (row 4, col 4) | **5**  | 32 + 5 = 37 (col 4 down: 12 + 5 = 17 ...) |
| C (row 6, col 2) | **35** | 49 − 35 = 14 |
| D (row 6, col 4) | **14** | 49 − 35 = 14 |

Tray tiles: **7, 5, 35, 14, 21, 12, 43, 3, 49, 6**

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
g++ main.cpp -o mathgrid -lGL -lGLU -lglut -lm

# Or with CMake
mkdir build && cd build
cmake .. && make
./mathgrid
```

### Windows (MinGW)

```bash
g++ main.cpp -o mathgrid.exe -lopengl32 -lglu32 -lfreeglut
```

### macOS (Homebrew)

```bash
brew install freeglut
g++ main.cpp -o mathgrid -framework OpenGL -framework GLUT
```
