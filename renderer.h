// renderer.h - the abstract drawing surface. Scene code only talks to this.
// Two back-ends implement it: SoftwareRenderer (CP1, pure C++) and GLRenderer (CP3, OpenGL).
#pragma once
#include "common.h"

struct Renderer {
    int W = 1280, H = 720;
    virtual ~Renderer() {}
    virtual void beginFrame() = 0;
    virtual void sky(Color top, Color bottom) = 0;                       // vertical gradient
    virtual void polygon(const std::vector<Vec2>& pts, Color c) = 0;     // simple polygon, screen space
    virtual void disc(Vec2 c, float r, Color col) = 0;                   // filled circle, screen space
    virtual void glow(Vec2 c, float r, Color col, float intensity) = 0;  // additive radial light
    virtual void thinLine(Vec2 a, Vec2 b, Color c) = 0;                  // 1px line
    virtual void endFrame() = 0;
};
