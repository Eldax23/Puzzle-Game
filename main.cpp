// MathGrid - OpenGL Math Crossword Puzzle Game
// Compile: g++ main.cpp -o mathgrid -lGL -lGLU -lglut -lm
// Or with freeglut: g++ main.cpp -o mathgrid -lGL -lGLU -lfreeglut -lm

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
static const int GRID_OFF_X = 18;   // grid top-left x
static const int GRID_OFF_Y = 80;   // grid top-left y (from top)

// Tray area at the bottom
static const int TRAY_Y    = 600;   // y from top where tray starts
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
static const Col C_TEXT     = {1.0f, 1.0f, 1.0f, 1};
static const Col C_DIM      = {0.55f,0.55f,0.60f,1};
static const Col C_EMPTY    = {0.10f,0.10f,0.11f,1};
static const Col C_SHADOW   = {0.0f, 0.0f, 0.0f, 0.35f};

// ─── Cell types ─────────────────────────────────────────────────────────────
enum CellType {
    CT_VOID,      // black / unused
    CT_NUMBER,    // fixed number
    CT_OPERATOR,  // fixed operator (+−×÷=)
    CT_BLANK,     // slot to fill in (answer)
};

struct Cell {
    CellType type  = CT_VOID;
    std::string label;   // what to draw
    int  answerVal = 0;  // expected answer (for CT_BLANK)
    int  filledVal = -1; // currently placed value (-1 = empty)
    bool correct   = false;
    bool flash     = false;
    float flashT   = 0;
};

// ─── Tray tile ──────────────────────────────────────────────────────────────
struct Tile {
    int value;
    bool used;      // dragged onto a blank
    int  placedRow; // which blank it was placed in
    int  placedCol;
};

static Cell g_grid[GRID_ROWS][GRID_COLS];
static std::vector<Tile> g_tiles;
static int g_puzzleSolved = 0;

// ─── Drag state ──────────────────────────────────────────────────────────────
struct DragState {
    bool   active    = false;
    int    tileIndex = -1;    // index into g_tiles
    float  x, y;              // current mouse pos (screen, y-up)
    int    fromRow = -1, fromCol = -1; // if re-dragging from a blank
};
static DragState g_drag;

// Score / attempts
static int g_correct = 0;
static int g_total   = 0; // number of blanks

// ─── Helpers ────────────────────────────────────────────────────────────────
static float screenY(float winY) { return WIN_H - winY; } // flip y

static void setCol(const Col& c) { glColor4f(c.r,c.g,c.b,c.a); }

static void drawRect(float x, float y, float w, float h, const Col& c, float r=8.f) {
    // rounded rect via triangle fan approximation
    glColor4f(c.r,c.g,c.b,c.a);
    int segs = 8;
    float cx = x+w/2, cy = y+h/2;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx,cy);
    for(int i=0;i<=segs*4;i++){
        float angle = i * (3.14159f*2.f/(segs*4));
        float px = cx + (w/2 - r + r*cosf(angle));
        float py = cy + (h/2 - r + r*sinf(angle));
        // clamp to rect
        px = std::max(x, std::min(x+w, cx + (w/2)*cosf(angle)));
        py = std::max(y, std::min(y+h, cy + (h/2)*sinf(angle)));
        glVertex2f(px,py);
    }
    glEnd();

    // simpler: just two rects + corners
    // Actually let's just do a plain filled quad for reliability:
    (void)r;
    glBegin(GL_QUADS);
    glVertex2f(x,   y);
    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);
    glVertex2f(x,   y+h);
    glEnd();
}

static void drawRoundRect(float x, float y, float w, float h, const Col& c, float radius=10.f) {
    glColor4f(c.r,c.g,c.b,c.a);
    int segs = 10;
    float r = std::min(radius, std::min(w,h)/2.f);
    // center rect
    glBegin(GL_QUADS);
    glVertex2f(x+r,   y);    glVertex2f(x+w-r, y);
    glVertex2f(x+w-r, y+h);  glVertex2f(x+r,   y+h);
    // left
    glVertex2f(x,   y+r);    glVertex2f(x+r,   y+r);
    glVertex2f(x+r, y+h-r);  glVertex2f(x,     y+h-r);
    // right
    glVertex2f(x+w-r, y+r);    glVertex2f(x+w,   y+r);
    glVertex2f(x+w,   y+h-r);  glVertex2f(x+w-r, y+h-r);
    glEnd();
    // corners
    float cx[4] = {x+r,    x+w-r, x+w-r, x+r};
    float cy[4] = {y+r,    y+r,   y+h-r, y+h-r};
    float a0[4] = {3.14159f, 3.f*3.14159f/2.f, 0, 3.14159f/2.f};
    for(int c2=0;c2<4;c2++){
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx[c2],cy[c2]);
        for(int i=0;i<=segs;i++){
            float a = a0[c2] + i*(3.14159f/2.f/segs);
            glVertex2f(cx[c2]+r*cosf(a), cy[c2]+r*sinf(a));
        }
        glEnd();
    }
}

// Simple bitmap text using GLUT
static void drawText(const std::string& s, float x, float y, const Col& c, float scale=1.0f, void* font=nullptr) {
    if(!font) font = GLUT_BITMAP_HELVETICA_18;
    glColor4f(c.r,c.g,c.b,c.a);
    // measure width
    float tw = 0;
    for(char ch : s) tw += glutBitmapWidth(font, ch);
    float ox = x - tw/2.f;
    float oy = y - 7.f; // rough vertical center
    glRasterPos2f(ox, oy);
    for(char ch : s) glutBitmapCharacter(font, ch);
}

static void drawTextL(const std::string& s, float x, float y, const Col& c, void* font=nullptr) {
    if(!font) font = GLUT_BITMAP_HELVETICA_18;
    glColor4f(c.r,c.g,c.b,c.a);
    glRasterPos2f(x, y);
    for(char ch : s) glutBitmapCharacter(font, ch);
}

// Cell pixel rect (y is OpenGL screen coords, y-up)
static void cellRect(int row, int col, float& x, float& y, float& w, float& h) {
    x = GRID_OFF_X + col*(CELL_W+CELL_GAP);
    float yTop = GRID_OFF_Y + row*(CELL_H+CELL_GAP);
    y = WIN_H - yTop - CELL_H; // OpenGL y-up
    w = CELL_W;
    h = CELL_H;
}

// Tray tile pixel rect
static void tileRect(int idx, float& x, float& y, float& w, float& h) {
    int tilesPerRow = 5;
    int row = idx / tilesPerRow;
    int col = idx % tilesPerRow;
    float totalW = tilesPerRow*(TILE_W+TILE_GAP)-TILE_GAP;
    float startX = (WIN_W - totalW)/2.f;
    x = startX + col*(TILE_W+TILE_GAP);
    float yTop = TRAY_Y + 15 + row*(TILE_H+TILE_GAP);
    y = WIN_H - yTop - TILE_H;
    w = TILE_W;
    h = TILE_H;
}

// ─── Puzzle setup ────────────────────────────────────────────────────────────
static void setupPuzzle() {
    // Clear
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
        g_grid[r][c].label     = "";
        g_grid[r][c].answerVal = ans;
        g_grid[r][c].filledVal = -1;
    };

    // Row 0:  VOID | + | VOID | = | 28 | VOID || VOID
    setBlank(0,0, 12);
    setOp   (0,1, "+");
    setBlank(0,2, 16);
    setOp(0,3,"=");
    setNum(0,4, "28");
    setBlank(0,6,19);

    // Row 1:  VOID | × | VOID | × | + | VOID | +
    setOp(1,0,"*");
    setOp(1,2,"*");
    setOp(1,4,"+");
    setOp(1,6,"-");

    // Row 2:  1 | × | 12 | = | 12 | VOID | 11
    setNum(2,0,"1");
    setOp (2,1,"x");
    setNum(2,2,"12");
    setOp (2,3,"=");
    setNum(2,4,"12");
    setNum(2,6,"11");

    // Row 3:  = | VOID | / | VOID | + | VOID | =
    setOp(3,0,"=");
    setOp(3,2,"/");
    setOp(3,4,"+");
    setOp(3,6,"=");

    // Row 4:  12 | VOID | 32 | + | [B=5] | = | VOID
    setNum  (4,0,"12");
    setNum  (4,2,"3");
    setOp   (4,3,"+");
    setBlank(4,4, 5);
    setOp   (4,5,"=");
    setBlank(4,6,8);

    // Row 5:  VOID | VOID | = | VOID | = | VOID | VOID
    setOp(5,2,"=");
    setOp(5,4,"=");

    // Row 6:  49 | - | [C=35] | = | [D=14] | VOID | VOID
    setNum  (6,0,"49");
    setOp   (6,1,"-");
    setBlank(6,2, 4);
    setOp   (6,3,"=");
    setBlank(6,4, 45);

    // Count blanks
    g_total = 0;
    for(int r=0;r<GRID_ROWS;r++)
        for(int c=0;c<GRID_COLS;c++)
            if(g_grid[r][c].type==CT_BLANK) g_total++;

    // Tray tiles: include correct answers + distractors
    g_tiles.clear();
    std::vector<int> vals = {19 , 5 , 8 , 4 , 45 , 12 , 16 , 32 , 63 , 52};
    for(int v : vals){
        Tile t; t.value=v; t.used=false; t.placedRow=-1; t.placedCol=-1;
        g_tiles.push_back(t);
    }

    g_correct = 0;
    g_puzzleSolved = 0;
}

// ─── Check if all blanks solved ──────────────────────────────────────────────
static void checkSolved() {
    g_correct = 0;
    for(int r=0;r<GRID_ROWS;r++)
        for(int c=0;c<GRID_COLS;c++)
            if(g_grid[r][c].type==CT_BLANK && g_grid[r][c].correct)
                g_correct++;
    if(g_correct == g_total) g_puzzleSolved = 1;
}

// ─── Hit test ────────────────────────────────────────────────────────────────
// Returns grid row,col under OpenGL mouse (x,y), -1 if none
static bool hitCell(float mx, float my, int& row, int& col) {
    for(int r=0;r<GRID_ROWS;r++){
        for(int c=0;c<GRID_COLS;c++){
            if(g_grid[r][c].type==CT_VOID) continue;
            float x,y,w,h;
            cellRect(r,c,x,y,w,h);
            if(mx>=x && mx<=x+w && my>=y && my<=y+h){
                row=r; col=c; return true;
            }
        }
    }
    return false;
}

static int hitTile(float mx, float my) {
    for(int i=0;i<(int)g_tiles.size();i++){
        if(g_tiles[i].used) continue;
        float x,y,w,h;
        tileRect(i,x,y,w,h);
        if(mx>=x && mx<=x+w && my>=y && my<=y+h) return i;
    }
    return -1;
}

// ─── Drawing ────────────────────────────────────────────────────────────────
static void drawCell(int row, int col) {
    Cell& cell = g_grid[row][col];
    if(cell.type == CT_VOID) return;

    float x,y,w,h;
    cellRect(row,col,x,y,w,h);

    Col bg;
    if(cell.type == CT_BLANK){
        if(cell.filledVal < 0)       bg = C_EMPTY;
        else if(cell.correct)        bg = C_CORRECT;
        else                         bg = C_WRONG;
    } else if(cell.type == CT_NUMBER){
        // teal highlight for the "answer row" (row 2 in screenshot)
        if(row==2 && (col==0||col==2||col==4||col==6)) bg = C_TEAL;
        else bg = C_FIXED;
    } else { // operator
        bg = C_CELL;
    }

    // shadow
    drawRoundRect(x+3, y-3, w, h, C_SHADOW, 10);
    drawRoundRect(x, y, w, h, bg, 10);

    // label
    std::string lbl = cell.label;
    if(cell.type==CT_BLANK && cell.filledVal>=0){
        char buf[16]; sprintf(buf,"%d",cell.filledVal);
        lbl = buf;
    }
    if(!lbl.empty()){
        Col tc = (cell.type==CT_OPERATOR) ? C_DIM : C_TEXT;
        void* font = GLUT_BITMAP_HELVETICA_18;
        if(lbl.size()>=3) font = GLUT_BITMAP_HELVETICA_12;
        drawText(lbl, x+w/2, y+h/2, tc, 1.0f, font);
    }

    // empty blank indicator
    if(cell.type==CT_BLANK && cell.filledVal<0){
        // draw a subtle "?" 
        drawText("?", x+w/2, y+h/2, C_DIM, 1.0f, GLUT_BITMAP_HELVETICA_18);
    }
}

static void drawTile(int idx, float ox, float oy, bool dragging) {
    float x,y,w,h;
    tileRect(idx,x,y,w,h);
    if(dragging){ x=ox-w/2; y=oy-h/2; }

    Col bg = dragging ? C_TEAL : C_TILE;

    // shadow
    if(!dragging) drawRoundRect(x+3,y-3,w,h,C_SHADOW,10);
    drawRoundRect(x,y,w,h,bg,10);

    char buf[16]; sprintf(buf,"%d",g_tiles[idx].value);
    drawText(std::string(buf), x+w/2, y+h/2, C_TEXT);
}

static void drawTray() {
    // tray background
    float ty = WIN_H - TRAY_Y - TRAY_H;
    drawRoundRect(0, ty, WIN_W, TRAY_H, C_TRAY, 0);
    // separator line
    glColor3f(0.25f,0.25f,0.28f);
    glBegin(GL_LINES);
    glVertex2f(0, WIN_H-TRAY_Y);
    glVertex2f(WIN_W, WIN_H-TRAY_Y);
    glEnd();

    for(int i=0;i<(int)g_tiles.size();i++){
        if(g_tiles[i].used) continue;
        if(g_drag.active && g_drag.tileIndex==i) continue; // draw last (on top)
        drawTile(i, 0, 0, false);
    }
}

static void drawHUD() {
    // Title
    drawText("MATHGRID", WIN_W/2, WIN_H-20, C_TEXT, 1.0f, GLUT_BITMAP_HELVETICA_18);

    // Score
    char buf[64];
    sprintf(buf,"Solved: %d / %d", g_correct, g_total);
    drawText(buf, WIN_W/2, WIN_H-45, C_DIM, 1.0f, GLUT_BITMAP_HELVETICA_12);

    if(g_puzzleSolved){
        // big win banner
        Col bannerBG = {0.05f,0.35f,0.25f,0.92f};
        drawRoundRect(WIN_W/2-140, WIN_H/2-40, 280, 80, bannerBG, 16);
        drawText("PUZZLE SOLVED!", WIN_W/2, WIN_H/2+10, C_TEXT, 1.0f, GLUT_BITMAP_HELVETICA_18);
        drawText("Press R to restart", WIN_W/2, WIN_H/2-18, C_DIM, 1.0f, GLUT_BITMAP_HELVETICA_12);
    }
}

// ─── GLUT Callbacks ──────────────────────────────────────────────────────────
static void display() {
    glClearColor(C_BG.r,C_BG.g,C_BG.b,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw grid cells
    for(int r=0;r<GRID_ROWS;r++)
        for(int c=0;c<GRID_COLS;c++)
            drawCell(r,c);

    // Draw equation hints (small arrows/lines)
    // (omitted for clarity)

    drawTray();
    drawHUD();

    // Draw dragged tile on top
    if(g_drag.active && g_drag.tileIndex>=0){
        drawTile(g_drag.tileIndex, g_drag.x, g_drag.y, true);
    }

    glutSwapBuffers();
}



static int g_actualW = WIN_W;
static int g_actualH = WIN_H;
static void reshape(int w, int h) {
    g_actualW = w;
    g_actualH = h;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,WIN_W,0,WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void mouseButton(int button, int state, int wx, int wy) {
    float mx = (float)wx / g_actualW * WIN_W;
    float my = (1.0f - (float)wy / g_actualH) * WIN_H;

    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Check tray tiles first
        int ti = hitTile(mx, my);
        if(ti >= 0) {
            g_drag.active    = true;
            g_drag.tileIndex = ti;
            g_drag.x = mx;
            g_drag.y = my;
            g_drag.fromRow = -1;
            g_drag.fromCol = -1;
            glutPostRedisplay();
            return;
        }
        // Check grid blanks (pick up placed tile)
        int gr=-1, gc=-1;
        if(hitCell(mx,my,gr,gc)){
            Cell& cell = g_grid[gr][gc];
            if(cell.type==CT_BLANK && cell.filledVal>=0){
                // Find which tile index
                for(int i=0;i<(int)g_tiles.size();i++){
                    if(g_tiles[i].used && g_tiles[i].placedRow==gr && g_tiles[i].placedCol==gc){
                        // pick up
                        g_drag.active    = true;
                        g_drag.tileIndex = i;
                        g_drag.x = mx;
                        g_drag.y = my;
                        g_drag.fromRow = gr;
                        g_drag.fromCol = gc;
                        g_tiles[i].used = false;
                        cell.filledVal  = -1;
                        cell.correct    = false;
                        checkSolved();
                        glutPostRedisplay();
                        return;
                    }
                }
            }
        }
    }

    if(button == GLUT_LEFT_BUTTON && state == GLUT_UP && g_drag.active) {
        float dx = mx, dy = my;
        int gr=-1,gc=-1;
        bool placed = false;

        if(hitCell(dx,dy,gr,gc)){
            Cell& cell = g_grid[gr][gc];
            if(cell.type == CT_BLANK){
                // If there's already something here, swap back to tray
                if(cell.filledVal >= 0){
                    // Return old tile
                    for(int i=0;i<(int)g_tiles.size();i++){
                        if(g_tiles[i].used && g_tiles[i].placedRow==gr && g_tiles[i].placedCol==gc){
                            g_tiles[i].used=false; g_tiles[i].placedRow=-1; g_tiles[i].placedCol=-1;
                            break;
                        }
                    }
                }
                int val = g_tiles[g_drag.tileIndex].value;
                cell.filledVal = val;
                cell.correct   = (val == cell.answerVal);
                g_tiles[g_drag.tileIndex].used      = true;
                g_tiles[g_drag.tileIndex].placedRow = gr;
                g_tiles[g_drag.tileIndex].placedCol = gc;
                placed = true;
                checkSolved();
            }
        }

        if(!placed){
            // Return to tray
            g_tiles[g_drag.tileIndex].used=false;
            g_tiles[g_drag.tileIndex].placedRow=-1;
            g_tiles[g_drag.tileIndex].placedCol=-1;
        }

        g_drag.active = false;
        g_drag.tileIndex = -1;
        glutPostRedisplay();
    }
}

static void mouseMotion(int wx, int wy) {
    if(g_drag.active){
        g_drag.x = (float)wx / g_actualW * WIN_W;
        g_drag.y = (1.0f - (float)wy / g_actualH) * WIN_H;
        glutPostRedisplay();
    }
}

static void keyboard(unsigned char key, int, int) {
    if(key=='r'||key=='R'){
        setupPuzzle();
        glutPostRedisplay();
    }
    if(key==27) exit(0); // ESC
}

static void timer(int) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ─── Main ────────────────────────────────────────────────────────────────────
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("MathGrid");

    setupPuzzle();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
