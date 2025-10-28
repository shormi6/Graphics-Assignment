// concentric_circles_gradient_fixed.cpp
// Enhanced colorful concentric rings with smooth gradient & glow effect
// Fixed compilation/runtime issues and made code robust.
// Compile:
//   g++ concentric_circles_gradient_fixed.cpp -o concentric_circles_gradient_fixed -lGL -lGLU -lglut -std=c++17

#include <GL/glut.h>
#include <cmath>
#include <algorithm>

// Window size
const int WINDOW_W = 800;
const int WINDOW_H = 800;

// Rings parameters
const int NUM_RINGS = 25;        // number of concentric rings
const float START_RADIUS = 15.0f; // inner radius (pixels)
const float RADIUS_STEP = 12.0f;  // radius increment per ring
const float BASE_THICKNESS = 8.0f; // thickness of each ring (pixels)
const int SEGMENTS = 360;         // number of segments to approximate a circle (>= 3)

// Constant PI (portable)
constexpr double PI = 3.14159265358979323846;

// Convert HSV (h in degrees, s and v in [0..1]) to RGB (outputs in [0..1])
void hsvToRgb(float h, float s, float v, float &r, float &g, float &b) {
    if (s <= 0.0001f) { r = g = b = v; return; }
    // wrap hue
    while (h < 0.0f) h += 360.0f;
    while (h >= 360.0f) h -= 360.0f;

    float hh = h / 60.0f;
    int i = static_cast<int>(floor(hh));
    float ff = hh - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * ff);
    float t = v * (1.0f - s * (1.0f - ff));

    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5:
        default: r = v; g = p; b = q; break;
    }
}

// Draw a single ring (as a triangle strip) centered at (cx, cy)
void drawRing(float cx, float cy, float innerR, float outerR,
              float hue, float sat, float val, float alpha)
{
    // Ensure at least 3 segments
    int segs = std::max(3, SEGMENTS);

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segs; ++i) {
        float theta = static_cast<float>( (2.0 * PI * i) / segs );
        float c = cosf(theta);
        float s = sinf(theta);

        // Outer vertex (slightly brighter)
        float ro, go, bo;
        hsvToRgb(hue, sat, std::min(1.0f, val + 0.10f), ro, go, bo);
        glColor4f(ro, go, bo, alpha);
        glVertex2f(cx + outerR * c, cy + outerR * s);

        // Inner vertex (slightly dimmer)
        float ri, gi, bi;
        hsvToRgb(hue, sat, val * 0.85f, ri, gi, bi);
        glColor4f(ri, gi, bi, alpha);
        glVertex2f(cx + innerR * c, cy + innerR * s);
    }
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    float cx = WINDOW_W * 0.5f;
    float cy = WINDOW_H * 0.5f;

    // Gradient path: warm (pink/red) -> cool (cyan/blue/violet)
    float startHue = 330.0f; // pink-magenta
    float endHue   = 210.0f; // blue-cyan

    for (int i = 0; i < NUM_RINGS; ++i) {
        float innerR = START_RADIUS + i * RADIUS_STEP;
        float outerR = innerR + BASE_THICKNESS;

        float t = (NUM_RINGS == 1) ? 0.0f : static_cast<float>(i) / (NUM_RINGS - 1);

        // Smooth hue interpolation (you can change easing here if desired)
        float hue = startHue + t * (endHue - startHue);

        // Slight modulation for richer palette
        float sat = 0.78f + 0.18f * sinf(t * static_cast<float>(PI)); // 0.6..0.96
        float val = 0.95f - 0.28f * t; // inner brighter, outer slightly dimmer

        // Transparency for soft blending
        float alpha = 0.78f + 0.22f * (1.0f - t);

        drawRing(cx, cy, innerR, outerR, hue, sat, val, alpha);
    }

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    // Keep coordinate system fixed to pixel-like coords for simplicity
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Map x: [0..WINDOW_W], y: [0..WINDOW_H]
    gluOrtho2D(0.0, static_cast<double>(WINDOW_W), 0.0, static_cast<double>(WINDOW_H));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int, int)
{
    if (key == 27 || key == 'q' || key == 'Q') {
        // Try to exit politely. If using older GLUT without glutLeaveMainLoop, use exit().
#if defined(GLUT_API_VERSION) && (GLUT_API_VERSION >= 4)
        // freeglut provides glutLeaveMainLoop()
        //glutLeaveMainLoop();
#else
        exit(0);
#endif
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    // Request double-buffered RGBA window with multisampling if supported
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutInitWindowPosition(200, 100);
    glutCreateWindow("Beautiful Concentric Circles - Fixed");

    // Enable blending and multisampling for smooth, glowing look
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_MULTISAMPLE);

    // Background (dark)
    glClearColor(0.03f, 0.03f, 0.05f, 1.0f);

    // Callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    // Enter main loop
    glutMainLoop();

    return 0;
}
