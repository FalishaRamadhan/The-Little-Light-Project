// scene.h - "The Little Light": the whole 5 minute film as a pure function of time t (seconds).
// Everything is built from the CP1 primitives (polygons, discs, glow, lines) and the CP2
// tools (matrix stack, Bezier curves, keyframe tracks). The same scene is drawn by either
// the software renderer (CP1/CP2) or the OpenGL renderer (CP3).
#pragma once
#include "cp2_transform.h"
#include "font.h"
#include <string>

namespace ll {

constexpr float GROUND = 560.f, SQUARE_X = 5300.f, DURATION = 300.f;
inline const Color GOLD = rgb(255, 196, 84), PALE = rgb(255, 238, 190), WARM = rgb(255, 150, 60);
inline float ramp(float t, float a, float b) { return smooth(remap(t, a, b)); }
inline float pulse(float t, float a, float b, float c, float d) { return ramp(t, a, b) * (1.f - ramp(t, c, d)); }

class Scene {
public:
    Scene() {
        lumiX = Track{{0, 300}, {10, 300}, {26, 720}, {36, 1100}, {44, 1350}, {47, 1370}, {49, 1370}, {51, 1395},
                      {53, 1395}, {55, 1420}, {66, 1420}, {72, 1480}, {84, 2400}, {104, 2400}, {116, 3000},
                      {132, 3000}, {140, 3500}, {152, 3500}, {160, 4000}, {174, 4000}, {196, 5150}, {214, 5160},
                      {232, 5165}, {300, 5165}};
        lumiL = Track{{0, 0.5f}, {10, 0.95f}, {54, 0.95f}, {62, 0.82f}, {90, 0.84f}, {100, 0.7f}, {120, 0.68f},
                      {130, 0.55f}, {142, 0.52f}, {150, 0.4f}, {164, 0.38f}, {172, 0.22f}, {196, 0.12f},
                      {214, 0.08f}, {218, 1.6f}, {226, 0.9f}, {232, 0.f}, {275, 0.f}, {280, 0.12f}, {284, 0.3f},
                      {292, 1.f}, {300, 1.1f}};
        camX = Track{{0, 700}, {12, 760}, {30, 950}, {44, 1380}, {52, 1460}, {64, 1470}, {70, 1600}, {74, 1850},
                     {84, 2330}, {92, 2450}, {104, 2450}, {112, 2850}, {120, 3050}, {132, 3050}, {140, 3480},
                     {144, 3560}, {152, 3560}, {160, 3960}, {166, 4060}, {174, 4060}, {186, 4750}, {196, 5120},
                     {206, 5230}, {216, 5230}, {228, 5240}, {240, 5200}, {262, 5200}, {274, 5200}, {283, 5200},
                     {290, 5000}, {300, 4450}};
        camZ = Track{{0, 0.7f}, {12, 0.72f}, {30, 1.0f}, {44, 1.3f}, {52, 1.9f}, {64, 1.9f}, {68, 1.05f},
                     {76, 1.05f}, {84, 1.4f}, {94, 2.0f}, {104, 2.0f}, {108, 1.3f}, {116, 1.4f}, {122, 2.1f},
                     {132, 2.1f}, {136, 1.3f}, {142, 2.0f}, {152, 2.0f}, {156, 1.3f}, {164, 2.0f}, {174, 2.0f},
                     {178, 1.3f}, {190, 1.5f}, {200, 2.3f}, {214, 2.6f}, {218, 1.7f}, {228, 1.0f}, {240, 1.15f},
                     {262, 1.6f}, {274, 2.3f}, {283, 2.6f}, {290, 1.0f}, {300, 0.32f}};
        camYend = Track{{283, 470}, {290, 380}, {300, -20}};
        liftT = Track{{0, 0}, {214, 0}, {222, -30}, {232, -30}, {240, 4}, {300, 4}};
        fallT = Track{{0, 0}, {231, 0}, {240, 1.25f}, {282, 1.25f}, {288, 0}, {300, 0}};
        eyeT = Track{{0, 0.06f}, {7, 0.06f}, {10, 1}, {194, 1}, {197, 1.3f}, {214, 1.3f}, {217, 0.9f}, {231, 0.8f},
                     {235, 0.05f}, {281, 0.05f}, {284, 1}, {300, 1}};
        mouthT = Track{{0, 0}, {10, 0.7f}, {44, 0.5f}, {60, 0.9f}, {100, 0.8f}, {170, 0.3f}, {190, 0}, {198, -0.7f},
                       {214, -0.4f}, {217, 0.6f}, {231, 0.5f}, {234, 0}, {283, 0}, {288, 0.9f}, {300, 1}};
        lookYT = Track{{0, 0}, {196, 0}, {199, 0.9f}, {206, 0.9f}, {209, 0.1f}, {300, 0.1f}};
        warmT = Track{{0, 0}, {214, 0}, {222, 0.45f}, {236, 0.12f}, {283, 0.15f}, {300, 0.55f}};
        burstT = Track{{0, 0}, {214, 0}, {219, 1}, {227, 0.9f}, {238, 0}, {300, 0}};
        treeLitT = Track{{0, 0}, {215, 0}, {224, 1}, {240, 0.5f}, {300, 0.6f}};
        genBuildings();
        uint32_t seed = 4242;
        growTree(Vec2(5330, GROUND), 0.f, 118.f, 26.f, 0, seed);
        for (int i = 0; i < 170; i++) motes.push_back({-300 + hash1(i * 5 + 1) * 6600, 150 + hash1(i * 5 + 2) * 400,
                                                       hash1(i * 5 + 3) * 6.28f, 0.2f + hash1(i * 5 + 4) * 0.5f});
        for (int i = 0; i < 900; i++) orbs.push_back({-200 + hash1(i * 7 + 11) * 6800, 190 + hash1(i * 7 + 12) * 350,
                                                      9 + hash1(i * 7 + 13) * 14, hash1(i * 7 + 14), hash1(i * 7 + 15) * 6.28f});
    }

    void draw(Canvas& C, float t) {
        Renderer& R = C.R;
        float z = camZ.at(t), cx = camX.at(t);
        float cy = t < 283.f ? (z < 1.f ? lerp(331.f, 430.f, clamp01((z - 0.7f) / 0.3f)) : 430.f + 40.f * clamp01((z - 1.f) / 1.6f)) : camYend.at(t);
        auto cam = [&](float p) { return Mat3::translate(640, 360) * Mat3::scale(z, z) * Mat3::translate(-cx * p, -cy); };

        float w = warmT.at(t);
        R.sky(mix(rgb(7, 10, 24), rgb(46, 30, 58), w), mix(rgb(30, 42, 66), rgb(120, 76, 66), w));

        C.reset(cam(0.35f)); drawBuildings(C, t, 0);
        C.reset(cam(0.65f)); drawBuildings(C, t, 1);
        C.reset(cam(1.f));
        drawBuildings(C, t, 2);
        drawGround(C, t);
        C.rect(4030, GROUND - 520, 170, 520, Color(0.01f, 0.015f, 0.03f, 0.82f));  // dark alley

        // ---- props and characters (world space) ----
        drawStreetlamp(C, t);
        drawJunk(C, 2560);
        drawTree(C, t);
        drawBeanA(C, t);
        drawRobotScene(C, t);
        drawBirdScene(C, t);
        drawPlantScene(C, t);
        drawChildScene(C, t);
        drawCreatureC(C, t);
        drawReturnOrbs(C, t);
        drawLumi(C, t);

        // light streams (Lumi sharing its light)
        Vec2 lp(lumiX.at(t) + 12, GROUND - 26);
        stream(C, t, 54, 62, lp, Vec2(1505, GROUND - 18));
        stream(C, t, 90, 97, lp, Vec2(2470, GROUND - 56));
        stream(C, t, 120, 127, lp, Vec2(3100, GROUND - 14));
        stream(C, t, 142, 149, lp, Vec2(3600, GROUND - 44));
        stream(C, t, 164, 171, lp, Vec2(4078, GROUND - 38));

        drawMotes(C, t);
        drawFinalLights(C, t);

        // burst of the final sacrifice (a huge additive light centred on Lumi)
        float b = burstT.at(t);
        if (b > 0.001f) {
            Vec2 c(lumiX.at(t), GROUND - 40 + liftT.at(t));
            C.glowLater(c, 160 + 1900 * ramp(t, 214, 227), GOLD, 0.55f * b);
            C.glowLater(c, 100 + 420 * ramp(t, 214, 224), PALE, 0.5f * b);
        }
        C.flushGlows();

        // ---- screen space overlays ----
        C.reset(Mat3());
        float flash = pulse(t, 216.5f, 218.5f, 220.f, 224.f);
        if (flash > 0.002f) C.rect(0, 0, 1280, 720, Color(1.f, 0.93f, 0.7f, 0.14f * flash));
        float black = 1.f - ramp(t, 3.8f, 6.8f);
        if (black > 0.001f) C.rect(0, 0, 1280, 720, Color(0, 0, 0, black));
        float ta = ramp(t, 0.8f, 2.2f) * (1.f - ramp(t, 3.6f, 5.2f));
        if (ta > 0.01f) drawText(C, "THE LITTLE LIGHT", 640, 320, 9, fade(PALE, ta));
        float ma = ramp(t, 289.f, 292.5f);
        if (ma > 0.01f) {
            C.rect(0, 22, 1280, 214, Color(0.02f, 0.02f, 0.05f, 0.42f * ma));
            Color tc = fade(PALE, ma);
            drawText(C, "EVEN THE SMALLEST LIGHT CAN BRIGHTEN", 640, 44, 4, tc);
            drawText(C, "THE DARKEST WORLD.", 640, 88, 4, tc);
            drawText(C, "AND KINDNESS, ONCE SHARED,", 640, 150, 4, tc);
            drawText(C, "NEVER TRULY DISAPPEARS.", 640, 194, 4, tc);
        }
        float ea = ramp(t, 296.f, 298.f);
        if (ea > 0.01f) drawText(C, "THE END", 640, 650, 5, fade(GOLD, ea));
    }

private:
    // ------------------------------------------------------------------ data
    Track lumiX, lumiL, camX, camZ, camYend, liftT, fallT, eyeT, mouthT, lookYT, warmT, burstT, treeLitT;
    struct Building { float x, w, h; int layer; uint32_t seed; Color col; bool door; };
    std::vector<Building> bld;
    struct Seg { Vec2 a, c1, c2, b; float w0, w1; };
    std::vector<Seg> tree;
    std::vector<Vec2> tips;
    std::vector<float> tipScale;
    struct Mote { float x, y, ph, sp; };
    std::vector<Mote> motes;
    struct Orb { float x, y, r, delay, ph; };
    std::vector<Orb> orbs;

    // ------------------------------------------------------------------ world building
    void genBuildings() {
        struct Layer { float x0, x1, wmin, wmax, hmin, hmax; Color col; };
        Layer ls[3] = {{-1700, 6900, 110, 240, 480, 1050, rgb(17, 23, 38)},
                       {-1200, 7300, 100, 210, 330, 760, rgb(13, 18, 31)},
                       {-700, 7100, 140, 300, 230, 540, rgb(9, 12, 22)}};
        struct F { float x, w, h; bool door; };
        std::vector<F> forced = {{2150, 230, 300, true}, {4640, 220, 520, false}, {5760, 230, 500, false}};
        std::vector<std::pair<float, float>> gaps = {{4030, 4200}, {4860, 5760}};
        uint32_t seed = 1;
        for (int l = 0; l < 3; l++) {
            float cur = ls[l].x0;
            while (cur < ls[l].x1) {
                float w = ls[l].wmin + hash1(seed++) * (ls[l].wmax - ls[l].wmin);
                float h = ls[l].hmin + hash1(seed++) * (ls[l].hmax - ls[l].hmin);
                if (l == 2) {
                    bool jumped = false;
                    for (auto& g : gaps) if (cur >= g.first - 0.01f && cur < g.second) { cur = g.second; jumped = true; }
                    if (jumped) continue;
                    bool placed = false;
                    for (auto& f : forced) if (cur >= f.x - 0.01f && cur < f.x + 1.f) {
                        bld.push_back({f.x, f.w, f.h, 2, seed++, ls[l].col, f.door}); cur = f.x + f.w + 4; placed = true; break;
                    }
                    if (placed) continue;
                    float stop = 1e9f;
                    for (auto& g : gaps) if (g.first > cur) stop = std::min(stop, g.first);
                    for (auto& f : forced) if (f.x > cur) stop = std::min(stop, f.x);
                    if (cur + w > stop) {
                        w = stop - cur;
                        if (w < 60) { cur = stop; continue; }
                        bld.push_back({cur, w, h, l, seed++, ls[l].col, false});
                        cur = stop; continue;
                    }
                }
                bld.push_back({cur, w, h, l, seed++, ls[l].col, false});
                cur += w + hash1(seed++) * 10.f;
            }
        }
    }
    void growTree(Vec2 base, float ang, float len, float w, int depth, uint32_t& seed) {
        auto rnd = [&]() { seed = seed * 1664525u + 1013904223u; return ((seed >> 8) & 0xFFFF) / 65536.f; };
        Vec2 dir(std::sin(ang), -std::cos(ang)), perp(-dir.y, dir.x), b = base + dir * len;
        float bend = (rnd() - 0.5f) * len * 0.35f;
        tree.push_back({base, base + dir * (len * 0.33f) + perp * bend, base + dir * (len * 0.66f) + perp * (bend * 0.5f), b, w, w * 0.72f});
        if (depth >= 4) { tips.push_back(b); tipScale.push_back(depth >= 6 ? 1.f : (depth == 5 ? 0.85f : 0.7f)); }
        if (depth >= 6) return;
        int kids = depth < 2 ? 3 : 2;
        for (int i = 0; i < kids; i++) {
            float spread = kids == 2 ? (i == 0 ? -1.f : 1.f) * (0.32f + rnd() * 0.28f) : (i - 1) * (0.45f + rnd() * 0.25f);
            growTree(b, ang + spread, len * (0.72f + rnd() * 0.08f), w * 0.7f, depth + 1, seed);
        }
    }

    float windowLit(float x, float refX, uint32_t id, float t) const {
        if (t < 283.f) return 0.f;
        float T = 284.f + std::fabs(x - refX) / 600.f + hash1(id + 5) * 3.f;
        return smooth(remap(t, T, T + 1.2f));
    }

    void drawBuildings(Canvas& C, float t, int layer) {
        const float winW[3] = {9, 11, 14}, winH[3] = {13, 16, 20}, stX[3] = {20, 24, 32}, stY[3] = {26, 32, 42};
        float refX = layer == 2 ? SQUARE_X : (layer == 1 ? 3690.f : 2310.f);
        for (auto& b : bld) {
            if (b.layer != layer) continue;
            Vec2 a = C.tf({b.x, GROUND - b.h}), d = C.tf({b.x + b.w, GROUND});
            if (d.x < 0 || a.x > C.R.W || d.y < 0 || a.y > C.R.H) continue;
            C.rect(b.x, GROUND - b.h, b.w, b.h + 40, b.col);
            C.rect(b.x - 3, GROUND - b.h - 6, b.w + 6, 6, mix(b.col, rgb(40, 52, 78), 0.35f));
            if (hash1(b.seed + 3) < 0.4f) C.line({b.x + b.w * 0.7f, GROUND - b.h - 6}, {b.x + b.w * 0.7f, GROUND - b.h - 30 - hash1(b.seed + 4) * 30}, 3, b.col);
            if (d.x - a.x < 14) continue;
            int cols = std::max(1, (int)((b.w - 14) / stX[layer])), rows = std::max(1, (int)((b.h - 40) / stY[layer]));
            float ox = b.x + (b.w - cols * stX[layer]) * 0.5f + (stX[layer] - winW[layer]) * 0.5f, oy = GROUND - b.h + 22;
            for (int r = 0; r < rows; r++) for (int c = 0; c < cols; c++) {
                uint32_t id = b.seed * 977u + r * 31u + c;
                if (hash1(id) < 0.14f) continue;
                float wx = ox + c * stX[layer], wy = oy + r * stY[layer];
                float lit = hash1(id + 99) < 0.86f ? windowLit(wx, refX, id, t) : 0.f;
                Color col = mix(mix(b.col, rgb(32, 44, 68), 0.5f), rgb(255, 208, 120), lit);
                C.rect(wx, wy, winW[layer], winH[layer], col);
                if (lit > 0.35f) C.glowLater({wx + winW[layer] * 0.5f, wy + winH[layer] * 0.5f}, 26.f, rgb(255, 190, 100), 0.30f * lit);
            }
            if (b.door) {
                C.rect(b.x + 100, GROUND - 46, 34, 46, rgb(3, 4, 8));
                float on = ramp(t, 73.f, 76.f);
                C.rect(b.x + 40, GROUND - 120, 26, 34, mix(rgb(16, 20, 32), WARM, on));
                C.glowLater({b.x + 53, GROUND - 103}, 110, WARM, 0.8f * on);
                C.glowLater({b.x + 117, GROUND - 24}, 70, WARM, 0.6f * on);
            }
        }
    }

    void drawGround(Canvas& C, float t) {
        (void)t;
        C.poly({{-3000, GROUND}, {9000, GROUND}, {9000, 4000}, {-3000, 4000}}, rgb(10, 13, 22));
        C.rect(-3000, GROUND, 12000, 3, rgb(30, 40, 60));
        C.ellipse({SQUARE_X, GROUND + 8}, 430, 11, rgb(22, 29, 45));
        for (int i = 0; i < 140; i++) {
            float x = -500 + hash1(i * 3 + 700) * 7000, s = 3 + hash1(i * 3 + 701) * 7;
            C.poly({{x - s, GROUND + 6}, {x - s * 0.3f, GROUND + 6 - s * 0.7f}, {x + s, GROUND + 6}}, rgb(18, 23, 36));
        }
    }

    void drawStreetlamp(Canvas& C, float t) {
        C.push(); C.translate(1500, GROUND);
        Color metal = rgb(26, 30, 40);
        C.poly({{-9, 0}, {9, 0}, {5, -12}, {-5, -12}}, metal);
        C.poly({{-3.5f, -12}, {3.5f, -12}, {2.5f, -196}, {-2.5f, -196}}, metal);
        C.bezier({0, -196}, {0, -214}, {18, -222}, {36, -214}, 5, 4, metal);
        C.push(); C.translate(36, -214); C.rotate(0.55f);
        C.poly({{-16, 0}, {16, 0}, {11, -9}, {-11, -9}}, metal);
        C.poly({{-12, 0}, {12, 0}, {9, 14}, {-9, 14}}, rgb(36, 42, 54));
        C.line({-4, 2}, {3, 12}, 1.2f, rgb(10, 12, 18));
        C.pop();
        if (t < 60.f) {
            float f = std::max(0.f, std::sin(t * 7.3f) * std::sin(t * 3.1f + 1.f) - 0.72f) * 3.5f;
            C.glowLater({40, -204}, 90, rgb(190, 215, 255), 0.6f * f);
        }
        C.pop();
    }

    void drawJunk(Canvas& C, float x) {
        C.push(); C.translate(x, GROUND);
        for (int i = 0; i < 16; i++) {
            float h1 = hash1(100 + i), h2 = hash1(200 + i), h3 = hash1(300 + i);
            Color col = mix(rgb(34, 32, 32), rgb(78, 60, 48), h3);
            C.push(); C.translate((h1 - 0.5f) * 170, -h2 * 26); C.rotate(h3 * 6.f);
            int k = i % 4;
            if (k == 0) C.rect(-9, -6, 18, 12, col);
            else if (k == 1) { C.circle({0, 0}, 6 + h2 * 5, col); C.circle({0, 0}, 3, rgb(10, 12, 18)); }
            else if (k == 2) C.poly({{-10, 5}, {0, -9}, {11, 6}}, col);
            else {
                std::vector<Vec2> g;
                for (int j = 0; j < 16; j++) { float a = j * 0.3927f, r = (j % 2) ? 8.f : 11.f; g.push_back({r * std::cos(a), r * std::sin(a)}); }
                C.poly(g, col);
            }
            C.pop();
        }
        C.pop();
    }

    // ------------------------------------------------------------------ Lumi
    float give(float t) const {
        static const float G[5][2] = {{54, 62}, {90, 97}, {120, 127}, {142, 149}, {164, 171}};
        float g = 0; for (auto& p : G) g = std::max(g, pulse(t, p[0], p[0] + 1.f, p[1] - 1.f, p[1]));
        return g;
    }

    void drawLumi(Canvas& C, float t) {
        float x = lumiX.at(t);
        float v = (lumiX.at(t + 0.04f) - lumiX.at(t - 0.04f)) / 0.08f;
        float walking = clamp01(std::fabs(v) / 40.f);
        float L = std::max(0.f, lumiL.at(t));
        float flick = L < 0.3f ? 1.f - 0.18f * (0.5f + 0.5f * std::sin(t * 23.f)) * (0.3f - L) / 0.3f : 1.f;
        float Le = L * flick * (1.f + 0.05f * std::sin(t * 3.f));
        float phase = x / 16.f, bob = walking * std::fabs(std::sin(phase)) * 7.f;
        float scared = ramp(t, 194, 198) * (1.f - ramp(t, 214, 216));
        float jx = scared * std::sin(t * 70.f) * 1.3f;
        float g = give(t);
        float y = GROUND - 19 + liftT.at(t) - bob;
        float sq = walking * 0.06f * std::sin(phase * 2.f) + 0.025f * std::sin(t * 2.2f);
        float kk = std::fmod(t + 1.3f, 4.7f), blink = kk < 0.14f ? 0.1f + 0.9f * std::fabs(kk - 0.07f) / 0.07f : 1.f;
        float eyeOpen = eyeT.at(t) * blink;
        float look = 1.f, lookY = lookYT.at(t);

        C.push(); C.translate(x + jx, y); C.rotate(fallT.at(t) + 0.1f * g); C.scale(1 + sq + 0.05f * g, 1 - sq);
        Color body = mix(rgb(62, 70, 92), GOLD, clamp01(Le * 1.5f));
        // flame tuft
        C.push(); C.translate(0, -17); C.rotate(0.12f * std::sin(t * 4.f));
        C.poly({{-5, 2}, {0, -16 - 6 * Le}, {5, 2}}, mix(rgb(70, 78, 100), PALE, clamp01(Le * 1.5f)));
        C.pop();
        float stride = std::sin(phase) * 5.f * walking;
        C.ellipse({-8 + stride, 18}, 5.5f, 3.5f, mix(rgb(50, 56, 76), WARM, clamp01(Le)));
        C.ellipse({8 - stride, 18}, 5.5f, 3.5f, mix(rgb(50, 56, 76), WARM, clamp01(Le)));
        C.ellipse({0, 0}, 20, 19, body);
        C.ellipse({-5, -6}, 9, 7, mix(body, PALE, 0.3f * clamp01(Le)));
        Color white = mix(rgb(120, 130, 150), rgb(255, 252, 240), clamp01(Le * 2.f));
        float ew = 4.6f * (eyeOpen > 1.1f ? 1.2f : 1.f), eh = 6.2f * std::max(0.08f, eyeOpen > 1.f ? 1.f + (eyeOpen - 1.f) * 0.8f : eyeOpen);
        for (int s = -1; s <= 1; s += 2) {
            float ex = s * 7.f;
            C.ellipse({ex, -3}, ew, eh, white);
            if (eyeOpen > 0.25f) C.ellipse({ex + look * 1.6f, -3 + lookY * 2.2f}, 2.4f, std::min(3.2f, eh * 0.55f), rgb(26, 22, 30));
        }
        float m = mouthT.at(t);
        C.bezier({-4, 8}, {-2, 8 + 3.5f * m}, {2, 8 + 3.5f * m}, {4, 8}, 1.4f, 1.4f, rgb(60, 36, 28));
        C.pop();

        Vec2 c(x + jx, y);
        C.glowLater(c, 90 + 300 * Le, GOLD, 0.45f * Le);
        C.glowLater(c, 34 + 40 * Le, PALE, 0.22f * Le);
    }

    // light-sharing particles flowing from Lumi to the target
    void stream(Canvas& C, float t, float a, float b, Vec2 from, Vec2 to) {
        if (t < a || t > b + 1.3f) return;
        const int n = 48;
        for (int i = 0; i < n; i++) {
            float ti = a + (b - a) * i / n, s = (t - ti) / 1.2f;
            if (s < 0 || s > 1) continue;
            float e = smooth(s), lift = -(40 + 30 * hash1(i * 7 + 11)), side = (hash1(i * 13 + 3) - 0.5f) * 30;
            Vec2 p = bezierPoint(from, from + Vec2(side, lift), to + Vec2(-side, lift * 0.6f), to, e);
            float k = std::sin(s * 3.14159f);
            C.glowLater(p, 14 + 8 * k, PALE, 0.9f * k);
            C.circle(p, 1.8f + 1.2f * k, alpha(PALE, k));
        }
    }

    // ------------------------------------------------------------------ creatures
    struct BeanStyle { Color dark, lit; };
    void drawBean(Canvas& C, float x, float t, const BeanStyle& s, float g, float tremble, float hop, float eyeOpen,
                  float smile, float lookDown, float face, float squat, float walk) {
        C.push();
        C.translate(x + tremble * std::sin(t * 52.f) * 1.3f, GROUND - hop - (walk > 0 ? std::fabs(std::sin(t * 12.f)) * 3.f : 0.f));
        C.scale(face, 1.f);
        float stretch = clamp01(hop / 18.f);
        C.scale(1.f - 0.08f * stretch + 0.1f * squat, 1.f + 0.1f * stretch - 0.18f * squat);
        Color body = mix(s.dark, s.lit, g);
        Color ear = mix(s.dark, s.lit, g * 0.8f);
        float wig = 0.12f * std::sin(t * 3.f + x);
        C.push(); C.translate(-9, -27); C.rotate(-0.25f + wig);
        C.poly({{-4, 0}, {-2, -17}, {5, 0}}, ear); C.pop();
        C.push(); C.translate(9, -27); C.rotate(0.25f - wig);
        C.poly({{-5, 0}, {2, -17}, {4, 0}}, ear); C.pop();
        float f = walk > 0 ? std::sin(t * 12.f) * 4.f : 0.f;
        C.ellipse({-7 + f, -2}, 5, 3, mix(s.dark, s.lit, g * 0.6f));
        C.ellipse({7 - f, -2}, 5, 3, mix(s.dark, s.lit, g * 0.6f));
        C.ellipse({0, -15}, 17, 15, body);
        C.ellipse({-4, -20}, 8, 6, mix(body, PALE, 0.25f * g));
        for (int e = -1; e <= 1; e += 2) {
            C.ellipse({e * 6.f, -18}, 4.4f, 5.f * std::max(0.1f, eyeOpen), rgb(235, 235, 240));
            C.ellipse({e * 6.f + 1.f, -18 + lookDown * 2.f}, 2.2f, 3.f * std::max(0.1f, eyeOpen), rgb(26, 22, 30));
        }
        if (g > 0.3f) { C.ellipse({-10, -11}, 3.5f, 2.2f, Color(1.f, 0.55f, 0.5f, 0.5f * g)); C.ellipse({10, -11}, 3.5f, 2.2f, Color(1.f, 0.55f, 0.5f, 0.5f * g)); }
        C.bezier({-3.5f, -9}, {-2, -9 + 3.5f * smile}, {2, -9 + 3.5f * smile}, {3.5f, -9}, 1.3f, 1.3f, rgb(50, 30, 30));
        C.pop();
        C.glowLater({x, GROUND - 16}, 60 + 40 * g, GOLD, 0.45f * g);
    }

    const BeanStyle styleA{rgb(58, 66, 90), rgb(255, 190, 140)};
    const BeanStyle styleC{rgb(60, 62, 96), rgb(200, 190, 255)};

    float aX(float t) const {
        if (t < 66) return 1505;
        if (t < 75) { float u = (t - 66) / 9.f; return 1505 + 745 * u * u; }
        return lerp(4700, 5085, smooth(remap(t, 244, 254)));
    }
    void drawBeanA(Canvas& C, float t) {
        if (t > 75 && t < 244) return;
        float g = ramp(t, 57, 63);
        float trem = t < 57 ? 1.f : 0.f;
        float hop = (t >= 63 && t < 67) ? std::max(0.f, std::sin((t - 63) * 6.f)) * 18.f : 0.f;
        bool run = (t >= 66 && t < 75), enter = (t >= 244 && t < 254);
        float eyes = t < 57 ? 0.7f : 1.f;
        float smile = t < 57 ? -0.8f : (t < 244 ? 1.f : 0.5f);
        float down = (t > 258) ? 0.8f : 0.f;
        drawBean(C, aX(t), t, styleA, t < 244 ? g : 1.f, trem, hop, eyes, smile, down, 1.f, t < 57 ? 0.5f : 0.f, (run || enter) ? 1.f : 0.f);
    }

    void drawRobot(Canvas& C, float x, bool seated, float face, float tilt, float eyes, float lamp, float press, float walk, float trem, float t) {
        C.push();
        C.translate(x + trem * std::sin(t * 55.f), GROUND + (seated ? 0.f : -std::fabs(std::sin(walk)) * 2.5f));
        C.scale(face, 1.f);
        Color body = rgb(84, 78, 72), dark = rgb(54, 50, 48);
        float ty = seated ? -26.f : -34.f;
        if (seated) {
            C.rect(-30, -26, 60, 26, rgb(48, 40, 36)); C.rect(-30, -26, 60, 4, rgb(60, 50, 44));
            C.rect(2, -34, 26, 9, dark); C.rect(22, -34, 9, 34, dark); C.rect(20, -4, 18, 4, body);
        } else {
            float sw = std::sin(walk) * 6.f;
            C.rect(-10 + sw, -34, 8, 34, dark); C.rect(2 - sw, -34, 8, 34, dark);
            C.rect(-12 + sw, -4, 12, 4, body); C.rect(0 - sw, -4, 12, 4, body);
        }
        C.poly({{-15, -72}, {13, -72}, {15, ty}, {-13, ty}}, body);
        C.rect(-8, -62, 14, 12, dark);
        C.circle({-1, -56}, 2.2f, mix(rgb(40, 38, 40), WARM, eyes));
        C.push(); C.translate(2, -72); C.rotate(tilt);
        C.rect(-13, -22, 28, 22, body); C.rect(-13, -22, 28, 3, dark);
        Color e = mix(rgb(34, 38, 44), rgb(255, 226, 150), eyes);
        C.circle({-3, -12}, 3.6f, e); C.circle({8, -12}, 3.6f, e);
        C.bezier({0, -22}, {-2, -30}, {-9, -31}, {-10, -38}, 1.6f, 1.2f, dark);
        C.circle({-10, -39}, 2.f, dark);
        C.glowLater({3, -12}, 44, WARM, 0.5f * eyes);
        C.pop();
        Vec2 sh(8, -60), hand(26, -44 + press * 4);
        C.bezier(sh, {sh.x + 8, sh.y + 2}, {hand.x - 2, hand.y - 8}, hand, 6, 5, dark);
        C.poly({{hand.x - 1, hand.y - 2}, {hand.x + 9, hand.y - 2}, {hand.x + 7, hand.y - 12}, {hand.x + 1, hand.y - 12}}, mix(rgb(60, 56, 52), rgb(140, 120, 70), lamp));
        C.circle({hand.x + 4, hand.y - 14}, 3.5f, mix(rgb(40, 44, 50), PALE, lamp));
        C.glowLater({hand.x + 4, hand.y - 12}, 95, GOLD, 0.7f * lamp);
        C.pop();
    }
    void drawRobotScene(Canvas& C, float t) {
        if (t < 246) {
            float lamp = ramp(t, 96, 98), eyes = ramp(t, 97, 99);
            float tilt = lerp(0.32f, -0.05f, ramp(t, 98, 104)) + 0.25f * pulse(t, 102, 103, 104, 105.5f);
            float press = (t > 84 && t < 95) ? std::max(0.f, std::sin(t * 2.8f)) : 0.f;
            drawRobot(C, 2500, true, -1.f, tilt, eyes, lamp, press, 0, t < 97 ? 0.4f : 0.f, t);
        } else {
            float x = lerp(4620, 4990, smooth(remap(t, 246, 262)));
            bool walking = t < 262;
            float lampOn = 1.f - ramp(t, 258, 260);
            drawRobot(C, x, false, 1.f, 0.1f, 1.f, lampOn, 0, walking ? t * 5.f : 0.f, 0, t);
        }
    }

    void drawBird(Canvas& C, Vec2 p, float face, float flap, float lit, float amp, float t) {
        C.push(); C.translate(p.x, p.y); C.scale(face, 1.f);
        Color body = mix(rgb(70, 76, 92), rgb(255, 214, 120), lit), wing = mix(rgb(52, 58, 74), rgb(255, 236, 170), lit);
        C.poly({{-10, 0}, {-24, -4}, {-26, 4}, {-12, 5}}, wing);
        C.ellipse({0, 0}, 13, 10, body);
        C.circle({10, -6}, 6, body);
        C.poly({{15, -7}, {22, -5}, {15, -3}}, rgb(220, 150, 70));
        C.circle({12, -7.5f}, 1.6f, rgb(15, 15, 20));
        C.push(); C.translate(-2, -3); C.rotate(-0.2f - amp * (0.5f + 0.5f * std::sin(flap)) * 1.2f);
        C.poly({{0, 0}, {-6, -14}, {-22, -18}, {-24, -4}}, wing); C.pop();
        C.pop();
        (void)t;
        C.glowLater(p, 70, GOLD, 0.55f * lit);
    }
    Vec2 birdPos(float t) const {
        if (t < 131) return {3100, GROUND - 11};
        if (t < 139) return bezierPoint({3100, GROUND - 11}, {3040, 450}, {3200, 380}, {3260, 420}, smooth(remap(t, 131, 139)));
        if (t < 250) return {3260 + 8 * std::sin(t * 0.8f), 420 + 6 * std::sin(t * 1.3f)};
        return bezierPoint({4400, 300}, {4700, 200}, {4950, 420}, {5120, 440}, smooth(remap(t, 250, 262)));
    }
    void drawBirdScene(Canvas& C, float t) {
        float lit = ramp(t, 124, 128);
        Vec2 p = birdPos(t);
        bool flying = t >= 131 && !(t >= 139 && t < 250 && false);
        float amp = flying ? (t < 262 ? 1.f : 0.25f) : 0.25f * ramp(t, 124, 130);
        bool lowTrapped = t < 131;
        drawBird(C, p, 1.f, t * (lowTrapped ? 18.f : 14.f), lit, amp, t);
        if (t < 250) {
            float rel = ramp(t, 129, 134), shake = pulse(t, 125, 126, 128, 130) * std::sin(t * 40.f) * 3.f;
            C.push(); C.translate(3100, GROUND);
            Color br = rgb(36, 28, 24);
            for (int i = 0; i < 5; i++) {
                float dir = (i % 2) ? 1.f : -1.f, h = hash1(500 + i);
                C.push(); C.translate(-50 + i * 24 + shake, -2 - h * 6); C.rotate(-0.6f + h * 0.5f + dir * rel * 1.3f);
                C.translate(dir * rel * 50, -rel * 12);
                C.bezier({0, 0}, {26, -4}, {50, -3}, {82, -5 - h * 8}, 6, 3, br);
                C.circle({40, -4}, 3, rgb(48, 40, 26)); C.pop();
            }
            C.pop();
        }
    }

    void drawPlant(Canvas& C, float x, float t, float h, float bloom) {
        C.push(); C.translate(x, GROUND);
        C.line({-30, 5}, {-8, 3}, 1.6f, rgb(4, 5, 9)); C.line({-8, 3}, {2, 6}, 1.6f, rgb(4, 5, 9)); C.line({2, 6}, {22, 2}, 1.6f, rgb(4, 5, 9));
        float d = 1.f - h, sway = std::sin(t * 1.3f) * 2.f * h;
        Vec2 p0(0, 0), p1(2, -26), p2(lerp(4, 26, d) + sway, lerp(-52, -40, d)), p3(lerp(2, 40, d) + sway, lerp(-78, -26, d));
        Color stem = mix(rgb(72, 62, 52), rgb(60, 170, 120), h), leaf = mix(rgb(66, 58, 46), rgb(70, 190, 130), h);
        C.bezier(p0, p1, p2, p3, 5, 3, stem);
        for (int i = 0; i < 2; i++) {
            float u = i ? 0.62f : 0.35f; float side = i ? -1.f : 1.f;
            Vec2 q = bezierPoint(p0, p1, p2, p3, u);
            C.push(); C.translate(q.x, q.y); C.rotate(side * (1.1f - 0.6f * h) + (side > 0 ? 0 : 3.14159f) * 0.f);
            C.ellipse({side * 10, 0}, 11, 4, leaf); C.pop();
        }
        if (bloom < 0.02f) C.circle(p3, 4.5f, mix(rgb(70, 60, 50), leaf, h));
        else {
            float len = 5 + 11 * bloom;
            for (int i = 0; i < 7; i++) {
                C.push(); C.translate(p3.x, p3.y); C.rotate(i * 0.8976f + 0.3f);
                C.ellipse({len * 0.5f, 0}, len * 0.5f, 3.5f + 2.f * bloom, mix(rgb(120, 100, 70), GOLD, bloom)); C.pop();
            }
            C.circle(p3, 4.f, PALE);
        }
        C.glowLater(p3, 50 + 90 * bloom, GOLD, 0.15f * h + 0.6f * bloom);
        C.pop();
    }
    void drawPlantScene(Canvas& C, float t) { drawPlant(C, 3600, t, ramp(t, 146, 152), ramp(t, 150, 157)); }

    void drawChild(Canvas& C, float x, float t, float crouch, float trem, float lantern, float run, float face) {
        C.push();
        C.translate(x + trem * std::sin(t * 48.f) * 1.4f, GROUND - (run > 0 ? std::fabs(std::sin(t * 14.f)) * 4.f : 0.f));
        C.scale(face, 1.f);
        float ph = std::sin(t * 14.f) * run;
        Color coat = mix(rgb(70, 34, 44), rgb(150, 84, 70), lantern * 0.6f), skin = mix(rgb(98, 80, 78), rgb(214, 170, 140), lantern * 0.7f + 0.2f), hair = rgb(24, 20, 24);
        float hy = -54 + crouch * 12;
        C.rect(-6 + ph * 5, -20 + crouch * 6, 5, 20 - crouch * 6, rgb(30, 28, 40));
        C.rect(1 - ph * 5, -20 + crouch * 6, 5, 20 - crouch * 6, rgb(30, 28, 40));
        C.poly({{-9, -46 + crouch * 12}, {9, -46 + crouch * 12}, {12, -18 + crouch * 6}, {-12, -18 + crouch * 6}}, coat);
        C.circle({0, hy}, 9, skin);
        C.poly({{-9.5f, hy - 1}, {-7, hy - 9}, {0, hy - 11}, {7, hy - 9}, {9.5f, hy - 1}, {5, hy - 5}, {-5, hy - 5}}, hair);
        C.circle({-3, hy + 1}, 1.1f, rgb(15, 15, 20)); C.circle({3, hy + 1}, 1.1f, rgb(15, 15, 20));
        float sm = lantern > 0.5f ? 1.f : -0.8f;
        C.bezier({-3, hy + 5}, {-1, hy + 5 + 2 * sm}, {1, hy + 5 + 2 * sm}, {3, hy + 5}, 1.f, 1.f, rgb(60, 30, 30));
        if (lantern < 0.1f) {
            C.line({-8, -40 + crouch * 12}, {3, -32 + crouch * 10}, 4, coat); C.line({8, -40 + crouch * 12}, {-3, -31 + crouch * 10}, 4, coat);
        } else {
            C.bezier({8, -42}, {16, -40}, {20, -38}, {25, -35}, 4, 3.5f, coat);
            C.line({-8, -40}, {-12, -26}, 4, coat);
            C.circle({27, -36}, 4.f, alpha(PALE, 1.f)); C.glowLater({27, -36}, 80, GOLD, 0.85f * lantern);
        }
        C.pop();
    }
    void drawChildScene(Canvas& C, float t) {
        float lan = ramp(t, 168, 171);
        if (t < 171.5f) drawChild(C, 4100, t, 1.f - ramp(t, 167, 170), t < 168 ? 1.f : 0.f, lan, 0.f, -1.f);
        else if (t < 175.f) { float u = (t - 171.5f) / 2.5f; drawChild(C, 4100 + 900 * u * u, t, 0, 0, 1, 1, 1.f); }
        else if (t >= 256.f) {
            float x = lerp(5900, 5245, smooth(remap(t, 256, 264)));
            bool run = t < 264;
            drawChild(C, x, t, 0, 0, 1.f - ramp(t, 262, 264), run ? 1.f : 0.f, -1.f);
        }
    }

    void drawCreatureC(Canvas& C, float t) {
        float lit = ramp(t, 217, 224);
        float stand = ramp(t, 226, 230);
        float tearsOn = 1.f - ramp(t, 214, 218);
        if (tearsOn > 0.01f) for (int i = 0; i < 3; i++) {
            float ph = std::fmod(t * 0.9f + i * 0.37f, 1.f);
            for (int e = -1; e <= 1; e += 2)
                C.circle({5310.f + e * 6.f, GROUND - 22 + ph * 16}, 1.6f, Color(0.6f, 0.78f, 1.f, (1.f - ph) * tearsOn));
        }
        float smile = lerp(-0.9f, 1.f, ramp(t, 222, 228));
        drawBean(C, 5310, t, styleC, lit, t < 214 ? 0.6f : 0.f, 0.f, 0.75f, smile, lerp(0.9f, 0.f, ramp(t, 214, 222)), -1.f, 1.f - stand, 0.f);
    }

    // orbs carried by the returning friends, placed next to Lumi, then merged back into it
    void drawReturnOrbs(Canvas& C, float t) {
        if (t < 244 || t > 277) return;
        const float slot[6] = {5133, 5115, 5097, 5197, 5215, 5233};
        const float placeA[6] = {258, 259, 260, 262, 264, 263}, placeB[6] = {263, 264, 265, 267, 268, 268};
        Vec2 carry[6];
        carry[0] = {aX(t) + 16, GROUND - 26};
        carry[1] = {lerp(4620, 4990, smooth(remap(t, 246, 262))) + 30, GROUND - 58};
        Vec2 bp = birdPos(t); carry[2] = {bp.x + 4, bp.y + 16};
        carry[3] = {lerp(5900, 5245, smooth(remap(t, 256, 264))) - 27, GROUND - 36};
        carry[4] = bezierPoint({4450, 430}, {4700, 300}, {4950, 470}, {5140, 430}, smooth(remap(t, 252, 264)));
        carry[5] = lerp(0.f, 1.f, ramp(t, 252, 257)) < 1.f ? Vec2(5300 + 10 * ramp(t, 252, 257), lerp(GROUND - 260, GROUND - 30, ramp(t, 252, 257))) : Vec2(5292, GROUND - 30);
        float vis[6] = {ramp(t, 244, 246), ramp(t, 258, 259), ramp(t, 250, 252), ramp(t, 256, 258), ramp(t, 252, 254), ramp(t, 252, 253)};
        for (int i = 0; i < 6; i++) {
            float u = ramp(t, placeA[i], placeB[i]);
            Vec2 sl(slot[i], GROUND - 8);
            Vec2 p(lerp(carry[i].x, sl.x, u), lerp(carry[i].y, sl.y, u) - 28 * std::sin(u * 3.14159f));
            float m = ramp(t, 270 + 0.5f * i, 275 + 0.5f * i);
            Vec2 lc(lumiX.at(t), GROUND - 22);
            p = Vec2(lerp(p.x, lc.x, m), lerp(p.y, lc.y, m) - 36 * std::sin(m * 3.14159f));
            float a = vis[i] * (1.f - smooth(remap(m, 0.9f, 1.f)));
            if (a < 0.01f) continue;
            float flick = 1.f + 0.12f * std::sin(t * 9.f + i * 2.f);
            C.glowLater(p, 46 * flick, GOLD, 0.8f * a);
            C.circle(p, 4.2f, alpha(PALE, a));
        }
    }

    // ------------------------------------------------------------------ the tree of the square
    void drawTree(Canvas& C, float t) {
        float lit = treeLitT.at(t);
        Color trunk = mix(rgb(14, 16, 26), rgb(112, 76, 42), lit), leaf = mix(rgb(20, 26, 40), rgb(214, 196, 92), lit);
        auto sway = [&](Vec2 p) { return Vec2(p.x + std::sin(t * 0.7f + p.y * 0.02f) * (GROUND - p.y) * 0.012f, p.y); };
        for (auto& s : tree) C.bezier(sway(s.a), sway(s.c1), sway(s.c2), sway(s.b), s.w0, s.w1, trunk);
        for (size_t i = 0; i < tips.size(); i++) {
            Vec2 q = sway(tips[i]);
            for (int k = 0; k < 3; k++) {
                float ox = (hash1((uint32_t)(i * 9 + k)) - 0.5f) * 30.f, oy = (hash1((uint32_t)(i * 9 + k + 3)) - 0.5f) * 24.f;
                C.circle({q.x + ox * tipScale[i], q.y + oy * tipScale[i]}, (11.f + 6.f * hash1((uint32_t)(i * 9 + k + 6))) * tipScale[i], mix(leaf, rgb(255, 230, 130), 0.25f * lit * hash1((uint32_t)(i + k))));
            }
            if (lit > 0.05f && hash1((uint32_t)i + 77) < 0.45f) {
                C.circle(q, 2.6f, alpha(PALE, lit));
                C.glowLater(q, 26, GOLD, 0.55f * lit);
            }
        }
    }

    // ------------------------------------------------------------------ atmosphere
    void drawMotes(Canvas& C, float t) {
        float L = std::max(0.f, lumiL.at(t)) + burstT.at(t);
        Vec2 lp(lumiX.at(t), GROUND - 30);
        for (auto& m : motes) {
            float x = m.x + std::sin(t * m.sp + m.ph) * 40.f, y = m.y + std::sin(t * 0.5f * m.sp + m.ph * 2.f) * 22.f;
            float d = length(Vec2(x - lp.x, y - lp.y));
            float k = 0.08f + clamp01(L) * std::exp(-d / 320.f) * 0.9f;
            C.circle({x, y}, 1.3f, Color(1.f, 0.85f, 0.55f, std::min(1.f, k)));
        }
    }
    void drawFinalLights(Canvas& C, float t) {
        if (t < 283.f) return;
        for (auto& o : orbs) {
            float T = 284.f + std::fabs(o.x - SQUARE_X) / 650.f + o.delay * 2.f;
            float a = smooth(remap(t, T, T + 1.5f));
            if (a < 0.01f) continue;
            float x = o.x + std::sin(t * 0.6f + o.ph) * 8.f, y = o.y - (t - T) * 6.f + std::sin(t * 0.9f + o.ph) * 5.f;
            float tw = 0.75f + 0.25f * std::sin(t * 3.f + o.ph * 3.f);
            C.glowLater({x, y}, o.r, mix(PALE, GOLD, o.delay), 0.6f * a * tw);
            C.circle({x, y}, 2.f, alpha(PALE, a));
        }
    }
};

}  // namespace ll
