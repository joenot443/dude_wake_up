#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <cstdint>

class FrameExchange;

// Draws an engine node's newest frame (from its FrameExchange) as a textured
// quad filling the current GL viewport. One instance per GL context; call
// initialize() with that context current.
class FramePresenter : protected QOpenGLFunctions_3_3_Core {
public:
    enum Reader { Stage = 0, NodeEditor = 1 };

    void initialize();
    void destroy();
    bool isInitialized() const { return program_ != 0; }

    // Returns false if the exchange has no frame yet. `newFrame` is set when
    // the frame wasn't shown by this reader before. `opacity` scales alpha
    // (blending is enabled only when it is below 1).
    bool draw(FrameExchange* exchange, Reader reader, float opacity = 1.0f, bool* newFrame = nullptr);

private:
    uint32_t program_ = 0;
    uint32_t vao_     = 0;
    uint32_t vbo_     = 0;
    int      locTex_     = -1;
    int      locOpacity_ = -1;
};
