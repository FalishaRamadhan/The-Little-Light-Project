// cp1_raster.h  -  CHECKPOINT 1: graphics primitives in PURE C++ (no OpenGL, no libraries)
//   * Bresenham line
//   * Midpoint circle (filled)
//   * Scanline polygon fill (even-odd rule, 4x vertical supersampling for anti-aliasing)
//   * Additive radial glow (the "light" of Lumi)
//   * Alpha blending into a floating point framebuffer
#pragma once
#include "renderer.h"
#include <cstdio>

class SoftwareRenderer : public Renderer {
public:
    std::vector<float> fb;  // RGB, 0..1 (can exceed 1 while accumulating light)
    SoftwareRenderer(int w, int h) { W = w; H = h; fb.assign((size_t)w * h * 3, 0.f); }

    void beginFrame() override {}
    void endFrame() override {}

    void sky(Color top, Color bot) override {
        for (int y = 0; y < H; y++) {
            float t = y / (float)(H - 1);
            float r = lerp(top.r, bot.r, t), g = lerp(top.g, bot.g, t), b = lerp(top.b, bot.b, t);
            float* p = &fb[(size_t)y * W * 3];
            for (int x = 0; x < W; x++) { p[0] = r; p[1] = g; p[2] = b; p += 3; }
        }
    }

    // ---- pixel level -------------------------------------------------------
    inline void blend(int x, int y, const Color& c, float cov) {
        float a = c.a * cov;
        float* p = &fb[((size_t)y * W + x) * 3];
        p[0] += (c.r - p[0]) * a; p[1] += (c.g - p[1]) * a; p[2] += (c.b - p[2]) * a;
    }
    inline void addLight(int x, int y, float r, float g, float b) {
        float* p = &fb[((size_t)y * W + x) * 3];
        p[0] += r; p[1] += g; p[2] += b;
    }

    // ---- Bresenham line ------------------------------------------------------
    void thinLine(Vec2 a, Vec2 b, Color c) override {
        int x0 = (int)std::lround(a.x), y0 = (int)std::lround(a.y);
        int x1 = (int)std::lround(b.x), y1 = (int)std::lround(b.y);
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        for (int guard = 0; guard < 20000; guard++) {
            if (x0 >= 0 && x0 < W && y0 >= 0 && y0 < H) blend(x0, y0, c, 1.f);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    // ---- Midpoint circle algorithm (filled) ----------------------------------
    void midpointDisc(int cx, int cy, int r, const Color& c) {
        if (r <= 0) { if (cx >= 0 && cx < W && cy >= 0 && cy < H) blend(cx, cy, c, 1.f); return; }
        half.assign(r + 1, -1);
        int x = r, y = 0, d = 1 - r;
        while (x >= y) {
            half[y] = std::max(half[y], x);
            half[x] = std::max(half[x], y);
            y++;
            if (d < 0) d += 2 * y + 1; else { x--; d += 2 * (y - x) + 1; }
        }
        for (int dy = 0; dy <= r; dy++) {
            int hw = half[dy]; if (hw < 0) continue;
            for (int s = 0; s < (dy == 0 ? 1 : 2); s++) {
                int yy = s == 0 ? cy + dy : cy - dy;
                if (yy < 0 || yy >= H) continue;
                int xa = std::max(0, cx - hw), xb = std::min(W - 1, cx + hw);
                for (int xx = xa; xx <= xb; xx++) blend(xx, yy, c, 1.f);
            }
        }
    }
    void disc(Vec2 c, float r, Color col) override {
        if (c.x + r < 0 || c.x - r >= W || c.y + r < 0 || c.y - r >= H || r < 0.2f) return;
        if (r < 3.5f) {
            // tiny discs: midpoint algorithm, coverage compensates sub-pixel radius
            Color cc = col; if (r < 1.f) cc.a *= r * r;
            midpointDisc((int)std::lround(c.x), (int)std::lround(c.y), (int)std::lround(r), cc);
        } else {
            int n = std::min(72, std::max(14, (int)(r * 1.3f)));
            tmp.resize(n);
            for (int i = 0; i < n; i++) {
                float a = 6.2831853f * i / n;
                tmp[i] = Vec2(c.x + r * std::cos(a), c.y + r * std::sin(a));
            }
            fillPolygon(tmp, col);
        }
    }

    // ---- Scanline polygon fill with coverage anti-aliasing --------------------
    void polygon(const std::vector<Vec2>& p, Color c) override { fillPolygon(p, c); }
    void fillPolygon(const std::vector<Vec2>& p, const Color& c) {
        size_t n = p.size();
        if (n < 3 || c.a <= 0.002f) return;
        float miny = 1e9f, maxy = -1e9f, minx = 1e9f, maxx = -1e9f;
        for (auto& v : p) { miny = std::min(miny, v.y); maxy = std::max(maxy, v.y); minx = std::min(minx, v.x); maxx = std::max(maxx, v.x); }
        if (maxy < 0 || miny >= H || maxx < 0 || minx >= W) return;
        int y0 = std::max(0, (int)std::floor(miny)), y1 = std::min(H - 1, (int)std::floor(maxy));
        int xmin = std::max(0, (int)std::floor(minx)), xmax = std::min(W - 1, (int)std::floor(maxx));
        if (cov.size() < (size_t)W + 2) cov.assign(W + 2, 0.f);
        const int SS = 4; const float inv = 1.f / SS;
        for (int y = y0; y <= y1; y++) {
            std::fill(cov.begin() + xmin, cov.begin() + xmax + 2, 0.f);
            bool any = false;
            for (int s = 0; s < SS; s++) {
                float sy = y + (s + 0.5f) * inv;
                xs.clear();
                for (size_t i = 0; i < n; i++) {
                    const Vec2& a = p[i]; const Vec2& b = p[(i + 1) % n];
                    if ((a.y <= sy && b.y > sy) || (b.y <= sy && a.y > sy))
                        xs.push_back(a.x + (sy - a.y) * (b.x - a.x) / (b.y - a.y));
                }
                if (xs.size() < 2) continue;
                std::sort(xs.begin(), xs.end());
                for (size_t k = 0; k + 1 < xs.size(); k += 2) {
                    float xa = std::min(std::max(xs[k], (float)xmin), (float)xmax + 1.f);
                    float xb = std::min(std::max(xs[k + 1], (float)xmin), (float)xmax + 1.f);
                    if (xb <= xa) continue;
                    any = true;
                    int ia = (int)xa, ib = (int)xb;
                    if (ia == ib) cov[ia] += (xb - xa) * inv;
                    else {
                        cov[ia] += (ia + 1 - xa) * inv;
                        for (int i = ia + 1; i < ib; i++) cov[i] += inv;
                        cov[ib] += (xb - ib) * inv;
                    }
                }
            }
            if (!any) continue;
            for (int x = xmin; x <= xmax; x++) if (cov[x] > 0.003f) blend(x, y, c, std::min(1.f, cov[x]));
        }
    }

    // ---- additive radial glow ---------------------------------------------------
    void glow(Vec2 c, float r, Color col, float I) override {
        if (I <= 0.001f || r < 0.5f) return;
        int x0 = std::max(0, (int)std::floor(c.x - r)), x1 = std::min(W - 1, (int)std::ceil(c.x + r));
        int y0 = std::max(0, (int)std::floor(c.y - r)), y1 = std::min(H - 1, (int)std::ceil(c.y + r));
        float r2 = r * r, ir = 1.f / r;
        for (int y = y0; y <= y1; y++) {
            float dy = y + 0.5f - c.y;
            for (int x = x0; x <= x1; x++) {
                float dx = x + 0.5f - c.x;
                float d2 = dx * dx + dy * dy;
                if (d2 >= r2) continue;
                float t = 1.f - std::sqrt(d2) * ir;
                float k = t * t * I * col.a;
                addLight(x, y, col.r * k, col.g * k, col.b * k);
            }
        }
    }

    void toRGB8(std::vector<uint8_t>& out) const {
        out.resize((size_t)W * H * 3);
        for (size_t i = 0; i < out.size(); i++) {
            float v = fb[i]; v = v < 0 ? 0 : (v > 1 ? 1 : v);
            // light gamma so the dark world keeps some detail
            v = std::pow(v, 0.92f);
            out[i] = (uint8_t)(v * 255.f + 0.5f);
        }
    }

private:
    std::vector<float> cov;
    std::vector<float> xs;
    std::vector<int> half;
    std::vector<Vec2> tmp;
};
