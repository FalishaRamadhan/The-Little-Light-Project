// cp3_opengl.h - CHECKPOINT 3: OpenGL integration (fixed-function pipeline, GLFW window)
// GLRenderer draws the same scene through OpenGL: triangle fans, alpha blending,
// additive blending for light, 4x MSAA. GLPresenter shows the CP1 software framebuffer as a texture.
#pragma once
#include "renderer.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

class GLRenderer : public Renderer {
public:
    int fbw = 1280, fbh = 720;
    void beginFrame() override {
        glViewport(0, 0, fbw, fbh);
        glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0, W, H, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);
    }
    void endFrame() override {}
    void sky(Color t, Color b) override {
        glBegin(GL_QUADS);
        glColor3f(t.r, t.g, t.b); glVertex2f(0, 0); glVertex2f((float)W, 0);
        glColor3f(b.r, b.g, b.b); glVertex2f((float)W, (float)H); glVertex2f(0, (float)H);
        glEnd();
    }
    void polygon(const std::vector<Vec2>& p, Color c) override {
        if (p.size() < 3) return;
        Vec2 ctr; for (auto& v : p) { ctr.x += v.x; ctr.y += v.y; }
        ctr.x /= p.size(); ctr.y /= p.size();
        glColor4f(c.r, c.g, c.b, c.a);
        glBegin(GL_TRIANGLE_FAN);               // fan from the centroid (works for star-shaped polygons)
        glVertex2f(ctr.x, ctr.y);
        for (auto& v : p) glVertex2f(v.x, v.y);
        glVertex2f(p[0].x, p[0].y);
        glEnd();
    }
    void disc(Vec2 c, float r, Color col) override {
        if (c.x + r < 0 || c.x - r > W || c.y + r < 0 || c.y - r > H) return;
        int n = std::min(72, std::max(10, (int)(r * 1.3f)));
        glColor4f(col.r, col.g, col.b, col.a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(c.x, c.y);
        for (int i = 0; i <= n; i++) { float a = 6.2831853f * i / n; glVertex2f(c.x + r * std::cos(a), c.y + r * std::sin(a)); }
        glEnd();
    }
    void glow(Vec2 c, float r, Color col, float I) override {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // additive light
        const int N = 48, RINGS = 5;
        for (int k = 0; k < RINGS; k++) {
            float t0 = (float)k / RINGS, t1 = (float)(k + 1) / RINGS;
            float a0 = (1 - t0) * (1 - t0) * I * col.a, a1 = (1 - t1) * (1 - t1) * I * col.a;
            glBegin(GL_TRIANGLE_STRIP);
            for (int i = 0; i <= N; i++) {
                float a = 6.2831853f * i / N, cs = std::cos(a), sn = std::sin(a);
                glColor4f(col.r, col.g, col.b, a0); glVertex2f(c.x + r * t0 * cs, c.y + r * t0 * sn);
                glColor4f(col.r, col.g, col.b, a1); glVertex2f(c.x + r * t1 * cs, c.y + r * t1 * sn);
            }
            glEnd();
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    void thinLine(Vec2 a, Vec2 b, Color c) override {
        glColor4f(c.r, c.g, c.b, c.a);
        glBegin(GL_LINES); glVertex2f(a.x, a.y); glVertex2f(b.x, b.y); glEnd();
    }
};

// Shows an RGB8 image (from the software renderer) as a full-window textured quad.
class GLPresenter {
public:
    GLuint tex = 0;
    void draw(const uint8_t* rgb, int w, int h, int fbw, int fbh) {
        if (!tex) {
            glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        glViewport(0, 0, fbw, fbh);
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);
        glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0, 1, 1, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        glColor3f(1, 1, 1);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(0, 0); glTexCoord2f(1, 0); glVertex2f(1, 0);
        glTexCoord2f(1, 1); glVertex2f(1, 1); glTexCoord2f(0, 1); glVertex2f(0, 1);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
};
