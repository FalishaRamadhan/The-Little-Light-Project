// common.h - tiny math toolkit shared by every checkpoint
#pragma once
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdint>

struct Vec2 {
    float x = 0, y = 0;
    Vec2() = default;
    Vec2(float X, float Y) : x(X), y(Y) {}
    Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};
inline float length(Vec2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }

struct Color {
    float r = 0, g = 0, b = 0, a = 1;
    Color() = default;
    Color(float R, float G, float B, float A = 1) : r(R), g(G), b(B), a(A) {}
};
inline Color rgb(int r, int g, int b, float a = 1.f) { return Color(r / 255.f, g / 255.f, b / 255.f, a); }
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
inline float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
inline float smooth(float t) { t = clamp01(t); return t * t * (3 - 2 * t); }
inline float remap(float t, float a, float b) { return clamp01((t - a) / (b - a)); }
inline Color mix(Color a, Color b, float t) {
    return Color(lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t), lerp(a.a, b.a, t));
}
inline Color alpha(Color c, float a) { c.a = a; return c; }
inline Color fade(Color c, float k) { c.a *= k; return c; }

// deterministic hash -> [0,1)  (everything in the film is a pure function of time + hashes)
inline float hash1(uint32_t n) {
    n = (n << 13) ^ n;
    n = n * (n * n * 15731u + 789221u) + 1376312589u;
    return (n & 0x7fffffffu) / 2147483648.0f;
}
