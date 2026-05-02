// ============================== mahmoud mustafa mahmoud ==============================
// مسؤول عن:
// - Window setup
// - Layout
// - Colors
// - Structs & Global Variables
// =====================================================================

// MathGrid - OpenGL Math Crossword Puzzle Game
// Compile: g++ main.cpp -o mathgrid -lGL -lGLU -lglut -lm

#include <GL/glut.h>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>

// ─── Window & Layout ────────────────────────────────────────────────────────
static const int WIN_W = 480;
static const int WIN_H = 780;
static int g_winH = WIN_H;

// Grid area: 7 cols × 7 rows of cells
static const int GRID_COLS = 7;
static const int GRID_ROWS = 7;
static const int CELL_W    = 58;
static const int CELL_H    = 58;
static const int CELL_GAP  = 6;
static const int GRID_OFF_X = 18;
static const int GRID_OFF_Y = 80;

// Tray area at the bottom
static const int TRAY_Y    = 600;
static const int TRAY_H    = 130;
static const int TILE_W    = 70;
static const int TILE_H    = 58;
static const int TILE_GAP  = 8;

// ─── Color palette ──────────────────────────────────────────────────────────
struct Col { float r,g,b,a; };

static const Col C_BG       = {0.05f,0.05f,0.05f,1};
static const Col C_CELL     = {0.17f,0.17f,0.18f,1};
static const Col C_FIXED    = {0.22f,0.22f,0.24f,1};
static const Col C_TEAL     = {0.14f,0.55f,0.50f,1};
static const Col C_TRAY     = {0.12f,0.12f,0.13f,1};
static const Col C_TILE     = {0.24f,0.24f,0.26f,1};
static const Col C_HOVER    = {0.32f,0.32f,0.35f,1};
static const Col C_CORRECT  = {0.14f,0.55f,0.35f,1};
static const Col C_WRONG    = {0.60f,0.18f,0.18f,1};
static const Col C_TEXT     = {1.0f,1.0f,1.0f,1};
static const Col C_DIM      = {0.55f,0.55f,0.60f,1};
static const Col C_EMPTY    = {0.10f,0.10f,0.11f,1};
static const Col C_SHADOW   = {0.0f,0.0f,0.0f,0.35f};

// ─── Cell types ─────────────────────────────────────────────────────────────
enum CellType {
    CT_VOID,
    CT_NUMBER,
    CT_OPERATOR,
    CT_BLANK,
};

struct Cell {
    CellType type  = CT_VOID;
    std::string label;
    int  answerVal = 0;
    int  filledVal = -1;
    bool correct   = false;
};

struct Tile {
    int value;
    bool used;
    int  placedRow;
    int  placedCol;
};

static Cell g_grid[GRID_ROWS][GRID_COLS];
static std::vector<Tile> g_tiles;

// ─── Drag state ──────────────────────────────────────────────────────────────
struct DragState {
    bool active = false;
    int tileIndex = -1;
    float x, y;
};

static DragState g_drag;

static int g_correct = 0;
static int g_total   = 0;
static int g_puzzleSolved = 0;





// ============================== ahmed eid ==============================
// مسؤول عن:
// - OpenGL Drawing Functions
// - رسم الـ Cells
// - رسم الـ Tiles
// - رسم النصوص
// =====================================================================

static void setCol(const Col& c) {
    glColor4f(c.r,c.g,c.b,c.a);
}

static void drawRoundRect(float x, float y, float w, float h, const Col& c) {

    glColor4f(c.r,c.g,c.b,c.a);

    glBegin(GL_QUADS);

    glVertex2f(x,y);
    glVertex2f(x+w,y);
    glVertex2f(x+w,y+h);
    glVertex2f(x,y+h);

    glEnd();
}

static void drawText(const std::string& s,
                     float x,
                     float y,
                     const Col& c,
                     void* font=nullptr) {

    if(!font)
        font = GLUT_BITMAP_HELVETICA_18;

    glColor4f(c.r,c.g,c.b,c.a);

    glRasterPos2f(x,y);

    for(char ch : s)
        glutBitmapCharacter(font, ch);
}

static void cellRect(int row, int col,
                     float& x, float& y,
                     float& w, float& h) {

    x = GRID_OFF_X + col*(CELL_W+CELL_GAP);

    float yTop = GRID_OFF_Y + row*(CELL_H+CELL_GAP);

    y = WIN_H - yTop - CELL_H;

    w = CELL_W;
    h = CELL_H;
}

static void drawCell(int row, int col) {

    Cell& cell = g_grid[row][col];

    if(cell.type == CT_VOID)
        return;

    float x,y,w,h;

    cellRect(row,col,x,y,w,h);

    drawRoundRect(x,y,w,h,C_CELL);

    std::string lbl = cell.label;

    if(cell.type == CT_BLANK && cell.filledVal >= 0){

        char buf[16];

        sprintf(buf,"%d",cell.filledVal);

        lbl = buf;
    }

    if(!lbl.empty()){

        drawText(lbl,
                 x+w/2-8,
                 y+h/2,
                 C_TEXT);
    }
}





// ============================== shady salah ==============================
// مسؤول عن:
// - Puzzle Setup
// - تجهيز المعادلات
// - تجهيز الـ Grid
// - تجهيز الـ Tiles
// =====================================================================

static void setupPuzzle() {

    for(int r=0;r<GRID_ROWS;r++)
        for(int c=0;c<GRID_COLS;c++)
            g_grid[r][c] = Cell();

    auto setNum = [](int r, int c, const std::string& lbl){

        g_grid[r][c].type  = CT_NUMBER;
        g_grid[r][c].label = lbl;
    };

    auto setOp = [](int r, int c, const std::string& lbl){

        g_grid[r][c].type  = CT_OPERATOR;
        g_grid[r][c].label = lbl;
    };

    auto setBlank = [](int r, int c, int ans){

        g_grid[r][c].type      = CT_BLANK;
        g_grid[r][c].answerVal = ans;
    };

    // Puzzle Equations
    setBlank(0,0,12);
    setOp(0,1,"+");
    setBlank(0,2,16);
    setOp(0,3,"=");
    setNum(0,4,"28");

    setNum(2,0,"1");
    setOp(2,1,"x");
    setNum(2,2,"12");

    setNum(6,0,"49");
    setOp(6,1,"-");

    // Tray tiles
    std::vector<int> vals =
    {19,5,8,4,45,12,16,32};

    g_tiles.clear();

    for(int v : vals){

        Tile t;

        t.value=v;
        t.used=false;
        t.placedRow=-1;
        t.placedCol=-1;

        g_tiles.push_back(t);
    }
}





// ============================== shady mohamed ==============================
// مسؤول عن:
// - Drag & Drop System
// - Mouse Interaction
// - تحريك الـ Tiles
// =====================================================================

static int hitTile(float mx, float my) {

    for(int i=0;i<(int)g_tiles.size();i++){

        float x = 40 + i*75;
        float y = 40;

        if(mx>=x && mx<=x+60 &&
           my>=y && my<=y+50)

            return i;
    }

    return -1;
}

static void mouseButton(int button,
                        int state,
                        int wx,
                        int wy) {

    float mx = wx;
    float my = WIN_H - wy;

    if(button == GLUT_LEFT_BUTTON &&
       state == GLUT_DOWN) {

        int ti = hitTile(mx,my);

        if(ti >= 0){

            g_drag.active = true;
            g_drag.tileIndex = ti;
            g_drag.x = mx;
            g_drag.y = my;
        }
    }

    if(button == GLUT_LEFT_BUTTON &&
       state == GLUT_UP){

        g_drag.active = false;
    }
}

static void mouseMotion(int wx, int wy) {

    if(g_drag.active){

        g_drag.x = wx;
        g_drag.y = WIN_H - wy;

        glutPostRedisplay();
    }
}





// ============================== mohamed yasser ==============================
// مسؤول عن:
// - Game Logic
// - Checking Answers
// - HUD
// - الفوز والخسارة
// =====================================================================

static void checkSolved() {

    g_correct = 0;

    for(int r=0;r<GRID_ROWS;r++)
        for(int c=0;c<GRID_COLS;c++)
            if(g_grid[r][c].correct)
                g_correct++;

    if(g_correct == g_total)
        g_puzzleSolved = 1;
}

static void drawHUD() {

    drawText("MATHGRID",
             WIN_W/2-50,
             WIN_H-30,
             C_TEXT);

    char buf[64];

    sprintf(buf,
            "Solved: %d / %d",
            g_correct,
            g_total);

    drawText(buf,
             WIN_W/2-50,
             WIN_H-55,
             C_DIM);

    if(g_puzzleSolved){

        drawText("PUZZLE SOLVED!",
                 WIN_W/2-70,
                 WIN_H/2,
                 C_TEXT);
    }
}





// ============================== mohamed salah,ahmed ramdan ==============================
// مسؤول عن:
// - Display Function
// - OpenGL Main Loop
// - GLUT Callbacks
// - تشغيل اللعبة
// =====================================================================

static void display() {

    glClearColor(C_BG.r,C_BG.g,C_BG.b,1);

    glClear(GL_COLOR_BUFFER_BIT);

    // رسم الـ Grid
    for(int r=0;r<GRID_ROWS;r++)
        for(int c=0;c<GRID_COLS;c++)
            drawCell(r,c);

    // رسم الـ Tiles
    for(int i=0;i<(int)g_tiles.size();i++){

        float x = 40 + i*75;
        float y = 40;

        drawRoundRect(x,y,60,50,C_TILE);

        char buf[16];

        sprintf(buf,"%d",g_tiles[i].value);

        drawText(buf,x+20,y+25,C_TEXT);
    }

    drawHUD();

    glutSwapBuffers();
}

static void reshape(int w, int h) {

    glViewport(0,0,w,h);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    gluOrtho2D(0,WIN_W,0,WIN_H);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
}

static void keyboard(unsigned char key, int, int) {

    if(key=='r'||key=='R'){

        setupPuzzle();

        glutPostRedisplay();
    }

    if(key==27)
        exit(0);
}

static void timer(int) {

    glutPostRedisplay();

    glutTimerFunc(16,timer,0);
}

int main(int argc, char** argv) {

    glutInit(&argc, argv);

    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGBA);

    glutInitWindowSize(WIN_W, WIN_H);

    glutCreateWindow("MathGrid");

    setupPuzzle();

    glutDisplayFunc(display);

    glutReshapeFunc(reshape);

    glutMouseFunc(mouseButton);

    glutMotionFunc(mouseMotion);

    glutKeyboardFunc(keyboard);

    glutTimerFunc(16,timer,0);

    glutMainLoop();

    return 0;
}