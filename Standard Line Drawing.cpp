// bresenham_glut.cpp
// Compile (Linux): g++ bresenham_glut.cpp -o bresenham -lGL -lGLU -lglut -std=c++17
// In Code::Blocks, add -lglut -lGL -lGLU to linker settings.

#include <GL/glut.h>
#include <vector>
#include <utility>
#include <cmath>
#include <iostream>

int winWidth = 800;
int winHeight = 600;

// store the pixels produced by Bresenham
std::vector<std::pair<int,int>> pixels;

// Bresenham's line algorithm (handles all octants)
void bresenhamLine(int x0, int y0, int x1, int y1, std::vector<std::pair<int,int>>& outPixels) {
    // Handle trivial case
    if (x0 == x1 && y0 == y1) {
        outPixels.emplace_back(x0, y0);
        return;
    }

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) { std::swap(x0, y0); std::swap(x1, y1); }

    if (x0 > x1) { // ensure left-to-right
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

// OpenGL display callback
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glPointSize(2.0f);
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
    // left, right, bottom, top  -> origin at bottom-left
    gluOrtho2D(0.0, width, 0.0, height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    std::cout << "Bresenham Line Drawing (GLUT)\n";
    std::cout << "Enter coordinates as integers within window size (" << winWidth << " x " << winHeight << ")\n";
    int x0, y0, x1, y1;
    std::cout << "Enter x0 y0: ";
    if (!(std::cin >> x0 >> y0)) return 0;
    std::cout << "Enter x1 y1: ";
    if (!(std::cin >> x1 >> y1)) return 0;

    // Optional: clamp to window bounds
    auto clamp = [](int v, int lo, int hi) { if (v < lo) return lo; if (v > hi) return hi; return v; };
    x0 = clamp(x0, 0, winWidth-1);
    x1 = clamp(x1, 0, winWidth-1);
    y0 = clamp(y0, 0, winHeight-1);
    y1 = clamp(y1, 0, winHeight-1);

    // compute pixels
    bresenhamLine(x0, y0, x1, y1, pixels);

    // init GLUT & create window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Bresenham Line Drawing");

    // background black, drawing in white
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    setupOrtho(winWidth, winHeight);

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}
