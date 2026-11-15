// The Little Light - main program
//   ./little_light                         interactive window (OpenGL by default)
//   ./little_light --software              start with the pure C++ software renderer
//   ./little_light --shot 215 out.ppm      save one frame (software renderer) and quit
//   ./little_light --video | ffmpeg ...    stream raw RGB24 frames to stdout (software renderer)
// Keys: SPACE pause | LEFT/RIGHT seek 5s | UP/DOWN speed | R switch renderer | HOME restart | P screenshot | ESC quit
#include "cp1_raster.h"
#include "cp2_transform.h"
#include "scene.h"
#include <cstdio>
#include <cstring>
#include <string>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#ifndef NO_WINDOW
#include "cp3_opengl.h"
#endif

static void writePPM(const char* path, const std::vector<uint8_t>& rgb, int w, int h) {
    FILE* f = std::fopen(path, "wb"); if (!f) return;
    std::fprintf(f, "P6\n%d %d\n255\n", w, h);
    std::fwrite(rgb.data(), 1, rgb.size(), f); std::fclose(f);
}

int main(int argc, char** argv) {
    const int W = 1280, H = 720;
    bool video = false, startSoftware = false, glShot = false; float shotT = -1; const char* shotPath = "frame.ppm";
    float vStart = 0, vEnd = ll::DURATION; int fps = 30;
    for (int i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "--video")) video = true;
        else if (!std::strcmp(argv[i], "--software")) startSoftware = true;
        else if (!std::strcmp(argv[i], "--shot") && i + 2 < argc) { shotT = (float)std::atof(argv[++i]); shotPath = argv[++i]; }
        else if (!std::strcmp(argv[i], "--glshot") && i + 2 < argc) { glShot = true; shotT = (float)std::atof(argv[++i]); shotPath = argv[++i]; }
        else if (!std::strcmp(argv[i], "--from") && i + 1 < argc) vStart = (float)std::atof(argv[++i]);
        else if (!std::strcmp(argv[i], "--to") && i + 1 < argc) vEnd = (float)std::atof(argv[++i]);
        else if (!std::strcmp(argv[i], "--fps") && i + 1 < argc) fps = std::atoi(argv[++i]);
    }
    ll::Scene scene;
    std::vector<uint8_t> rgb;

    // ---------- headless modes (CP1/CP2: no OpenGL involved at all) ----------
    if (video || (shotT >= 0 && !glShot)) {
        SoftwareRenderer sw(W, H); Canvas C(sw);
        if (shotT >= 0) {
            scene.draw(C, shotT); sw.toRGB8(rgb); writePPM(shotPath, rgb, W, H); return 0;
        }
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        int total = (int)((vEnd - vStart) * fps);
        for (int f = 0; f < total; f++) {
            scene.draw(C, vStart + (float)f / fps); sw.toRGB8(rgb);
            std::fwrite(rgb.data(), 1, rgb.size(), stdout);
            if (f % fps == 0) std::fprintf(stderr, "\rrendering %5.1fs / %5.1fs", (float)f / fps + vStart, vEnd);
        }
        std::fprintf(stderr, "\n");
        return 0;
    }

#ifdef NO_WINDOW
    std::fprintf(stderr, "built without a window; use --shot or --video\n");
    return 1;
#else
    // ---------- interactive window (CP3) ----------
    if (!glfwInit()) { std::fprintf(stderr, "GLFW init failed\n"); return 1; }
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* win = glfwCreateWindow(W, H, "The Little Light", nullptr, nullptr);
    if (!win) { std::fprintf(stderr, "window creation failed\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);
    glfwSetWindowAspectRatio(win, 16, 9);

    GLRenderer gl; SoftwareRenderer sw(W, H); GLPresenter presenter;
    Canvas Cgl(gl), Csw(sw);
    bool useGL = !startSoftware, paused = false, down[512] = {false};
    float t = 0, speed = 1.f; double last = glfwGetTime();
    if (glShot) t = shotT;

    auto screenshot = [&](const char* path) {
        int fw, fh; glfwGetFramebufferSize(win, &fw, &fh);
        std::vector<uint8_t> px((size_t)fw * fh * 3), flip((size_t)fw * fh * 3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1); glReadPixels(0, 0, fw, fh, GL_RGB, GL_UNSIGNED_BYTE, px.data());
        for (int y = 0; y < fh; y++) std::memcpy(&flip[(size_t)y * fw * 3], &px[(size_t)(fh - 1 - y) * fw * 3], (size_t)fw * 3);
        writePPM(path, flip, fw, fh);
    };

    int frames = 0;
    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime(); float dt = (float)(now - last); last = now;
        glfwPollEvents();
        auto key = [&](int k) { bool p = glfwGetKey(win, k) == GLFW_PRESS; bool r = p && !down[k]; down[k] = p; return r; };
        if (key(GLFW_KEY_ESCAPE)) break;
        if (key(GLFW_KEY_SPACE)) paused = !paused;
        if (key(GLFW_KEY_R)) { useGL = !useGL; std::printf("renderer: %s\n", useGL ? "OpenGL" : "pure C++ software"); }
        if (key(GLFW_KEY_LEFT)) t = std::max(0.f, t - 5.f);
        if (key(GLFW_KEY_RIGHT)) t = std::min(ll::DURATION, t + 5.f);
        if (key(GLFW_KEY_UP)) speed = std::min(8.f, speed * 2.f);
        if (key(GLFW_KEY_DOWN)) speed = std::max(0.25f, speed * 0.5f);
        if (key(GLFW_KEY_HOME)) t = 0;
        bool shot = key(GLFW_KEY_P);
        if (!paused && !glShot) t += dt * speed;
        if (t > ll::DURATION) t = ll::DURATION;

        int fw, fh; glfwGetFramebufferSize(win, &fw, &fh);
        glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
        if (useGL) { gl.fbw = fw; gl.fbh = fh; gl.beginFrame(); ll::Scene& s = scene; s.draw(Cgl, t); gl.endFrame(); }
        else { scene.draw(Csw, t); sw.toRGB8(rgb); presenter.draw(rgb.data(), W, H, fw, fh); }
        if (shot) screenshot("screenshot.ppm");
        if (glShot && ++frames == 2) { screenshot(shotPath); break; }
        glfwSwapBuffers(win);
    }
    glfwDestroyWindow(win); glfwTerminate();
    return 0;
#endif
}
