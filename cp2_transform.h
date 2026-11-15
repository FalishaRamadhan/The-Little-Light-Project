// cp2_transform.h - CHECKPOINT 2: transformations + animation in PURE C++
//   * Mat3 homogeneous 2D matrices (translate / rotate / scale, concatenation)
//   * Canvas: a matrix stack (push/pop) with shape helpers that transform local -> screen
//   * Cubic Bezier curves (Bernstein form) for stems, branches, wings, mouths
//   * Track: monotone cubic Hermite (PCHIP) keyframe interpolation for animation curves
#pragma once
#include "renderer.h"

struct Mat3 {
    float m[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};  // row major: x' = m0 x + m1 y + m2
    static Mat3 translate(float x, float y) { Mat3 r; r.m[2] = x; r.m[5] = y; return r; }
    static Mat3 scale(float sx, float sy) { Mat3 r; r.m[0] = sx; r.m[4] = sy; return r; }
    static Mat3 rotate(float a) {
        Mat3 r; float c = std::cos(a), s = std::sin(a);
        r.m[0] = c; r.m[1] = -s; r.m[3] = s; r.m[4] = c; return r;
    }
    Mat3 operator*(const Mat3& o) const {
        Mat3 r;
        for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
            float s = 0; for (int k = 0; k < 3; k++) s += m[i * 3 + k] * o.m[k * 3 + j];
            r.m[i * 3 + j] = s;
        }
        return r;
    }
    Vec2 apply(Vec2 p) const { return {m[0] * p.x + m[1] * p.y + m[2], m[3] * p.x + m[4] * p.y + m[5]}; }
    float uniformScale() const { return std::sqrt(std::fabs(m[0] * m[4] - m[1] * m[3])); }
};

inline Vec2 bezierPoint(Vec2 a, Vec2 b, Vec2 c, Vec2 d, float t) {
    float u = 1 - t;
    return a * (u * u * u) + b * (3 * u * u * t) + c * (3 * u * t * t) + d * (t * t * t);
}

// Keyframe curve: monotone cubic Hermite (no overshoot, flat where the key values are flat)
struct Track {
    std::vector<float> T, V, M;
    Track() {}
    Track(std::initializer_list<std::pair<float, float>> keys) {
        for (auto& k : keys) { T.push_back(k.first); V.push_back(k.second); }
        size_t n = T.size(); M.assign(n, 0.f);
        if (n < 3) return;
        std::vector<float> h(n - 1), d(n - 1);
        for (size_t i = 0; i + 1 < n; i++) { h[i] = T[i + 1] - T[i]; d[i] = (V[i + 1] - V[i]) / h[i]; }
        for (size_t i = 1; i + 1 < n; i++) {
            if (d[i - 1] * d[i] <= 0) M[i] = 0;
            else {
                float w1 = 2 * h[i] + h[i - 1], w2 = h[i] + 2 * h[i - 1];
                M[i] = (w1 + w2) / (w1 / d[i - 1] + w2 / d[i]);
            }
        }
    }
    float at(float t) const {
        if (T.empty()) return 0;
        if (t <= T.front()) return V.front();
        if (t >= T.back()) return V.back();
        size_t i = std::upper_bound(T.begin(), T.end(), t) - T.begin() - 1;
        float h = T[i + 1] - T[i], s = (t - T[i]) / h;
        float s2 = s * s, s3 = s2 * s;
        return (2 * s3 - 3 * s2 + 1) * V[i] + (s3 - 2 * s2 + s) * h * M[i] +
               (-2 * s3 + 3 * s2) * V[i + 1] + (s3 - s2) * h * M[i + 1];
    }
};

class Canvas {
public:
    Renderer& R;
    Mat3 M;
    explicit Canvas(Renderer& r) : R(r) {}

    void reset(const Mat3& base) { M = base; st.clear(); }
    void push() { st.push_back(M); }
    void pop() { M = st.back(); st.pop_back(); }
    void translate(float x, float y) { M = M * Mat3::translate(x, y); }
    void rotate(float a) { M = M * Mat3::rotate(a); }
    void scale(float sx, float sy) { M = M * Mat3::scale(sx, sy); }
    void scale(float s) { scale(s, s); }
    float sc() const { return M.uniformScale(); }
    Vec2 tf(Vec2 p) const { return M.apply(p); }

    bool visible(const std::vector<Vec2>& p) const {
        float x0 = 1e9f, x1 = -1e9f, y0 = 1e9f, y1 = -1e9f;
        for (auto& v : p) { x0 = std::min(x0, v.x); x1 = std::max(x1, v.x); y0 = std::min(y0, v.y); y1 = std::max(y1, v.y); }
        if (x1 < 0 || x0 > R.W || y1 < 0 || y0 > R.H) return false;
        if (x1 - x0 < 0.25f && y1 - y0 < 0.25f) return false;
        return true;
    }
    void poly(std::vector<Vec2> pts, Color c) {
        for (auto& p : pts) p = M.apply(p);
        if (visible(pts)) R.polygon(pts, c);
    }
    void rect(float x, float y, float w, float h, Color c) { poly({{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}}, c); }
    void circle(Vec2 c, float r, Color col) { R.disc(M.apply(c), r * sc(), col); }
    void ellipse(Vec2 c, float rx, float ry, Color col) {
        int n = std::min(64, std::max(10, (int)((rx + ry) * sc() * 0.7f)));
        std::vector<Vec2> p(n);
        for (int i = 0; i < n; i++) { float a = 6.2831853f * i / n; p[i] = {c.x + rx * std::cos(a), c.y + ry * std::sin(a)}; }
        poly(p, col);
    }
    void line(Vec2 a, Vec2 b, float w, Color c) {
        Vec2 d = b - a; float L = length(d); if (L < 1e-4f) return;
        Vec2 n(-d.y / L * w * 0.5f, d.x / L * w * 0.5f);
        poly({a + n, b + n, b - n, a - n}, c);
    }
    // tapered cubic Bezier ribbon
    void bezier(Vec2 a, Vec2 b, Vec2 c, Vec2 d, float w0, float w1, Color col, int seg = 0) {
        if (seg <= 0) {
            float L = (length(b - a) + length(c - b) + length(d - c)) * sc();
            seg = std::min(24, std::max(4, (int)(L / 10.f)));
        }
        Vec2 prev = a, pn;
        { Vec2 t0 = b - a; if (length(t0) < 1e-4f) t0 = c - a; float l = std::max(1e-4f, length(t0)); pn = {-t0.y / l, t0.x / l}; }
        for (int i = 1; i <= seg; i++) {
            float u = (float)i / seg;
            Vec2 p = bezierPoint(a, b, c, d, u);
            Vec2 t = p - prev; float l = std::max(1e-4f, length(t));
            Vec2 n(-t.y / l, t.x / l);
            float wp = lerp(w0, w1, (float)(i - 1) / seg) * 0.5f, wc = lerp(w0, w1, u) * 0.5f;
            poly({prev + pn * wp, p + n * wc, p - n * wc, prev - pn * wp}, col);
            if (wc * sc() > 1.6f) circle(p, wc, col);
            prev = p; pn = n;
        }
    }
    // lights are queued and drawn additively on top of the finished picture
    void glowLater(Vec2 p, float r, Color col, float I) {
        if (I <= 0.002f) return;
        pending.push_back({M.apply(p), r * sc(), col, I});
    }
    void flushGlows() {
        for (auto& g : pending) {
            if (g.p.x + g.r < 0 || g.p.x - g.r > R.W || g.p.y + g.r < 0 || g.p.y - g.r > R.H) continue;
            R.glow(g.p, g.r, g.c, g.I);
        }
        pending.clear();
    }

private:
    struct G { Vec2 p; float r; Color c; float I; };
    std::vector<Mat3> st;
    std::vector<G> pending;
};
