// bresenham_thick_glut.cpp
// Compile (Linux): g++ bresenham_thick_glut.cpp -o bresenham_thick -lGL -lGLU -lglut -std=c++17
// In Code::Blocks, add -lglut -lGL -lGLU to linker settings.

#include <GL/glut.h>
#include <vector>
#include <utility>
#include <cmath>
#include <iostream>
#include <algorithm>

int winWidth = 900;
int winHeight = 600;

// store pixel coordinates to draw
std::vector<std::pair<int,int>> pixels;

// Clamp helper
inline int clamp(int v, int lo, int hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

// Bresenham's line algorithm (handles all octants)
void bresenhamLine(int x0, int y0, int x1, int y1, std::vector<std::pair<int,int>>& outPixels) {
    if (x0 == x1 && y0 == y1) {
        outPixels.emplace_back(x0, y0);
        return;
    }

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) { std::swap(x0, y0); std::swap(x1, y1); }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = std::abs(y1 - y0);
    int error = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; ++x) {
        if (steep) outPixels.emplace_back(y, x);
        else       outPixels.emplace_back(x, y);

        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}

// Draw horizontal span from x1..x2 at y (append to vector if inside window)
inline void drawHSpan(int cx, int x1, int x2, int y, std::vector<std::pair<int,int>>& outPixels) {
    if (y < 0 || y >= winHeight) return;
    int sx = clamp(x1, 0, winWidth - 1);
    int ex = clamp(x2, 0, winWidth - 1);
    if (ex < 0 || sx > winWidth - 1) return;
    for (int x = sx; x <= ex; ++x) outPixels.emplace_back(x, y);
}

// Midpoint circle fill using 8-way symmetry with horizontal span filling
// center (cx, cy), radius r >= 0
// Uses integer arithmetic only for the circle rasterization
void drawFilledCircleSymmetry(int cx, int cy, int r, std::vector<std::pair<int,int>>& outPixels) {
    if (r <= 0) {
        // single pixel
        if (cx >= 0 && cx < winWidth && cy >= 0 && cy < winHeight)
            outPixels.emplace_back(cx, cy);
        return;
    }

    int x = r;
    int y = 0;
    int d = 1 - r;

    // For each (x,y) on the circle octant, draw horizontal spans for the symmetric y-levels.
    while (x >= y) {
        // Draw spans for the 8 symmetric points by converting each symmetric arc to horizontal spans:
        // For the pair (x, y): horizontal spans at cy +/- y from cx - x .. cx + x
        drawHSpan(cx, cx - x, cx + x, cy + y, outPixels);
        if (y != 0) drawHSpan(cx, cx - x, cx + x, cy - y, outPixels);

        // For the pair (y, x): horizontal spans at cy +/- x from cx - y .. cx + y
        if (x != y) {
            drawHSpan(cx, cx - y, cx + y, cy + x, outPixels);
            if (x != 0) drawHSpan(cx, cx - y, cx + y, cy - x, outPixels);
        }

        ++y;
        if (d < 0) {
            d += 2*y + 1;
        } else {
            --x;
            d += 2*(y - x) + 1;
        }
    }
}

// Build thick line: for each Bresenham center pixel draw a filled circle radius r
// r = floor(W/2)
void buildThickLine(int x0, int y0, int x1, int y1, int W, std::vector<std::pair<int,int>>& outPixels) {
    outPixels.clear();
    std::vector<std::pair<int,int>> centers;
    bresenhamLine(x0, y0, x1, y1, centers);

    int r = std::max(0, W/2);
    // To reduce duplicate pixels we can reserve and optionally unique later.
    // We'll just append and then unique at the end.
    for (const auto &p : centers) {
        drawFilledCircleSymmetry(p.first, p.second, r, outPixels);
    }

    // Remove duplicates to reduce drawing cost
    std::sort(outPixels.begin(), outPixels.end());
    outPixels.erase(std::unique(outPixels.begin(), outPixels.end()), outPixels.end());
}

// OpenGL display callback
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glPointSize(1.0f);
    glBegin(GL_POINTS);
    for (const auto &p : pixels) {
        glVertex2i(p.first, p.second);
    }
    glEnd();

    glutSwapBuffers();
}

// Set up orthographic 2D projection matching window pixels
void setupOrtho(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, width, 0.0, height); // origin at bottom-left
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    std::cout << "Bresenham Thick Line Drawing (GLUT)\n";
    std::cout << "Window size: " << winWidth << " x " << winHeight << "\n";
    std::cout << "Enter two endpoints (x0 y0 x1 y1) and desired integer line width W.\n";
    std::cout << "Coordinates should be integers within window. Example: 50 50 700 500 7\n\n";

    int x0, y0, x1, y1, W;
    std::cout << "Enter x0 y0 x1 y1 W: ";
    if (!(std::cin >> x0 >> y0 >> x1 >> y1 >> W)) {
        std::cerr << "Invalid input. Exiting.\n";
        return 0;
    }

    // clamp coords & ensure W >= 1
    x0 = clamp(x0, 0, winWidth - 1);
    x1 = clamp(x1, 0, winWidth - 1);
    y0 = clamp(y0, 0, winHeight - 1);
    y1 = clamp(y1, 0, winHeight - 1);
    if (W < 1) W = 1;

    // build the thick line pixel list
    buildThickLine(x0, y0, x1, y1, W, pixels);

    // init GLUT & create window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Bresenham Thick Line Drawing");

    // background black, drawing in white
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    setupOrtho(winWidth, winHeight);

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}
