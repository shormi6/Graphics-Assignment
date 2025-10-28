// liang_barsky_clipping.cpp
// Implements Liang-Barsky line clipping with OpenGL visualization
// Compile:
//   g++ liang_barsky_clipping.cpp -o liang_barsky -lGL -lGLU -lglut -std=c++17

#include <GL/glut.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iomanip>

struct Point { double x, y; };
struct Segment { Point a, b; };

double xmin_w = -50, ymin_w = -50, xmax_w = 50, ymax_w = 50;
std::vector<Segment> segments;    // original input segments
std::vector<Segment> clipped;     // clipped segments (only those or visible parts)

// viewport/window size for GLUT
int winWidth = 800, winHeight = 800;

// Liang-Barsky helper: clip a single param range
bool liangBarskyClip(double x0, double y0, double x1, double y1,
                     double xmin, double ymin, double xmax, double ymax,
                     Point &out0, Point &out1)
{
    double dx = x1 - x0;
    double dy = y1 - y0;

    // p and q arrays for 4 inequalities
    double p[4] = { -dx, dx, -dy, dy }; // left, right, bottom, top
    double q[4] = { x0 - xmin, xmax - x0, y0 - ymin, ymax - y0 };

    double u1 = 0.0; // max entering t
    double u2 = 1.0; // min leaving t

    for (int i = 0; i < 4; ++i) {
        double pi = p[i];
        double qi = q[i];
        if (pi == 0) {
            if (qi < 0) {
                // Line parallel to edge and outside
                return false;
            }
            // Line parallel and inside for this boundary -> continue
        } else {
            double t = qi / pi;
            if (pi < 0) {
                // potential entering
                if (t > u1) u1 = t;
            } else {
                // potential leaving
                if (t < u2) u2 = t;
            }
        }
    }

    if (u1 > u2) return false; // no visible segment

    // compute clipped points
    out0.x = x0 + u1 * dx;
    out0.y = y0 + u1 * dy;
    out1.x = x0 + u2 * dx;
    out1.y = y0 + u2 * dy;
    return true;
}

// A more robust Liang-Barsky variant using the standard p/q arrangement and handling q differences
bool liangBarsky(double x0, double y0, double x1, double y1,
                 double xmin, double ymin, double xmax, double ymax,
                 Point &out0, Point &out1)
{
    double dx = x1 - x0;
    double dy = y1 - y0;

    double p[4] = { -dx, dx, -dy, dy };
    double q[4] = { x0 - xmin, xmax - x0, y0 - ymin, ymax - y0 };

    double umin = 0.0;
    double umax = 1.0;

    for (int i = 0; i < 4; ++i) {
        if (p[i] == 0) {
            if (q[i] < 0) {
                return false; // parallel and outside
            }
            // parallel and inside -> no restriction
        } else {
            double t = q[i] / p[i];
            if (p[i] < 0) {
                // entering
                if (t > umin) umin = t;
            } else {
                // leaving
                if (t < umax) umax = t;
            }
        }
    }

    if (umin > umax) return false;

    out0.x = x0 + umin * dx;
    out0.y = y0 + umin * dy;
    out1.x = x0 + umax * dx;
    out1.y = y0 + umax * dy;
    return true;
}

// Draw utility: draw a line segment with given width and color
void drawLine(const Point &p1, const Point &p2, float width=2.0f)
{
    glLineWidth(width);
    glBegin(GL_LINES);
      glVertex2d(p1.x, p1.y);
      glVertex2d(p2.x, p2.y);
    glEnd();
}

// Display callback
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw clipping rectangle (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
      glVertex2d(xmin_w, ymin_w);
      glVertex2d(xmax_w, ymin_w);
      glVertex2d(xmax_w, ymax_w);
      glVertex2d(xmin_w, ymax_w);
    glEnd();

    // Draw original lines in red (thin, partially transparent look using stipple)
    glColor3f(0.8f, 0.1f, 0.1f); // red
    glLineWidth(1.5f);
    for (const auto &seg : segments) {
        drawLine(seg.a, seg.b, 1.5f);
    }

    // Draw clipped segments in green (overlay)
    glColor3f(0.05f, 0.6f, 0.05f); // green
    glLineWidth(3.5f);
    for (const auto &c : clipped) {
        drawLine(c.a, c.b, 3.5f);
    }

    // Optional: draw endpoints of clipped segments as small points
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for (const auto &c : clipped) {
        glVertex2d(c.a.x, c.a.y);
        glVertex2d(c.b.x, c.b.y);
    }
    glEnd();

    glutSwapBuffers();
}

// Reshape callback - set orthographic projection to cover the clipping window nicely
void reshape(int w, int h)
{
    winWidth = w; winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // compute a small margin around the clipping rectangle so we can see everything
    double width = xmax_w - xmin_w;
    double height = ymax_w - ymin_w;
    double marginX = std::max(10.0, width * 0.15);
    double marginY = std::max(10.0, height * 0.15);

    double left = xmin_w - marginX;
    double right = xmax_w + marginX;
    double bottom = ymin_w - marginY;
    double top = ymax_w + marginY;

    // keep aspect ratio by expanding whichever dimension is smaller
    double aspectWindow = (right - left) / (top - bottom);
    double aspectViewport = double(w) / double(h);
    if (aspectViewport > aspectWindow) {
        // viewport wider -> expand x-range
        double extra = ((top - bottom) * aspectViewport - (right - left)) * 0.5;
        left -= extra; right += extra;
    } else {
        double extra = ((right - left) / aspectViewport - (top - bottom)) * 0.5;
        bottom -= extra; top += extra;
    }

    gluOrtho2D(left, right, bottom, top);
}

// Keyboard: press ESC or q to quit
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27 || key == 'q' || key == 'Q') {
        exit(0);
    }
}

// Prepare clipping for all input segments
void computeClipped()
{
    clipped.clear();
    for (const auto &s : segments) {
        Point outA, outB;
        // Use liangBarsky (robust version). If visible, push the clipped part.
        if (liangBarsky(s.a.x, s.a.y, s.b.x, s.b.y, xmin_w, ymin_w, xmax_w, ymax_w, outA, outB)) {
            // If degenerate (zero-length) or extremely small, we still add it
            clipped.push_back({outA, outB});
        }
    }
}

int main(int argc, char** argv)
{
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Liang-Barsky Line Clipping Visualization\n";
    std::cout << "Enter clipping rectangle xmin ymin xmax ymax (space-separated):\n";
    if (!(std::cin >> xmin_w >> ymin_w >> xmax_w >> ymax_w)) {
        std::cerr << "Invalid clipping window input. Exiting.\n";
        return 1;
    }

    if (xmin_w > xmax_w) std::swap(xmin_w, xmax_w);
    if (ymin_w > ymax_w) std::swap(ymin_w, ymax_w);

    int n;
    std::cout << "Enter number of line segments: ";
    if (!(std::cin >> n) || n < 0) {
        std::cerr << "Invalid number of segments. Exiting.\n";
        return 1;
    }

    segments.clear();
    std::cout << "Enter each segment as: x0 y0 x1 y1 (space-separated), one per line.\n";
    for (int i = 0; i < n; ++i) {
        double x0, y0, x1, y1;
        if (!(std::cin >> x0 >> y0 >> x1 >> y1)) {
            std::cerr << "Invalid segment input; expected 4 numbers. Exiting.\n";
            return 1;
        }
        segments.push_back({{x0,y0},{x1,y1}});
    }

    // Precompute clipped portions
    computeClipped();

    // Initialize GLUT and run main loop
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("Liang-Barsky Line Clipping");

    // background white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    std::cout << "Press ESC or 'q' to quit the visualization window.\n";

    glutMainLoop();
    return 0;
}
