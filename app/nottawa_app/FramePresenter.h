#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <cstdint>

class FrameExchange;

// Draws an engine node's newest frame (from its FrameExchange) as a textured
// quad filling the current GL viewport. One instance per GL context; call
// initialize() with that context current.
class FramePresenter : protected QOpenGLFunctions_3_3_Core {
public:
    // One per view that may show a node at the same time as the others.
    enum Reader { Stage = 0, NodeEditor = 1, Preview = 2, Inspector = 3, InspectorPad = 4 };

    void initialize();
    void destroy();
    bool isInitialized() const { return program_ != 0; }

    // Sets the GL viewport draw() fills (device pixels, GL's bottom-left
    // origin). Call it rather than glViewport before draw(): draw() needs the
    // rect, and asking GL for it (glGetIntegerv) makes the thread wait for
    // the driver, which waits for the GPU: with the GPU busy (a strand
    // loading, another app), that stalled the UI for seconds.
    void setViewport(int x, int y, int width, int height);

    // Returns false if the exchange has no frame yet. `newFrame` is set when
    // the frame wasn't shown by this reader before. `opacity` scales alpha.
    // A non-zero `cornerRadiusPx` rounds the corners of the viewport
    // (antialiased); blending is enabled whenever either applies.
    bool draw(FrameExchange* exchange, Reader reader, float opacity = 1.0f,
              float cornerRadiusPx = 0.0f, bool* newFrame = nullptr);

private:
    uint32_t program_ = 0;
    uint32_t vao_     = 0;
    uint32_t vbo_     = 0;
    int      locTex_     = -1;
    int      locOpacity_ = -1;
    int      locRect_    = -1;
    int      locRadius_  = -1;
    int      viewport_[4] = {0, 0, 0, 0};
};
