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




```bash
glClear          → امسح الشاشة
glMatrixMode     → اختر: كائنات ولا كاميرا؟
glLoadIdentity   → افرش الأرضية من جديد
glColor3f        → لون القلم
glPushMatrix     → احفظ مكاني قبل ما أتغير
glScaled         → كبّر/صغّر
glTranslated     → انقل من مكان لمكان
glutSolidCube    → ارسم مكعب ممتلئ
glutSolidSphere  → ارسم كرة ممتلئة
glPopMatrix      → ارجع لمكاني الأصلي
glFlush          → نفذ الأوامر دلوقتي
glutSwapBuffers  → بدّل الشاشات (لمنع الوميض)
glutTimerFunc    → عدّ 20 مللي وارجع
glutKeyboardFunc → استمع للحروف العادية
glutSpecialFunc  → استمع للأسهم والأزرار الخاصة
glClearColor     → لون المسح
glOrtho          → كاميرا 2D متوازية
glutInit         → جهز GLUT
glutInitWindowSize → حجم النافذة
glutInitWindowPosition → مكان النافذة
glutInitDisplayMode → إعدادات اللون والبافر
glutCreateWindow → أنشئ النافذة
glutDisplayFunc  → مين بيرسم؟
glutIdleFunc     → مين بيرسم لما مفيش حاجة؟
glutMainLoop     → ابدأ المحرك ولا ترجع!

```
