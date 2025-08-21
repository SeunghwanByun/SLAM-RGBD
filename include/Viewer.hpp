#pragma once
#include <string>
#include <functional>

struct GLFWwindow;

namespace Youth{

struct ClearColor {
    float r = 0.1f, g = 0.1f, b = 0.12f, a = 1.0f;
};

class Viewer{
public:
    struct Config {
        int width = 1280;
        int height = 720;
        std::string title = "Youth Viewer";
        bool vsync = true;
        ClearColor clear;
    };

    explicit Viewer(const Config& cfg);
    ~Viewer();

    Viewer(const Viewer&) = delete;
    Viewer& operator=(const Viewer&) = delete;

    bool ok( )const noexcept { return window_ != nullptr;}
    void run();

    void setDrawCallback(std::function<void()> callback) { draw_callback_ = std::move(callback); }

private:
    bool initGLFW();
    bool initGLEW();
    void drawTriangle();
    void poll();

    Config cfg_;
    GLFWwindow* window_ = nullptr;
    unsigned vao_ = 0, vbo_ = 0;
    unsigned shader_prog_ = 0;
    std::function<void()> draw_callback_ = {};
};

} // namespace Youth