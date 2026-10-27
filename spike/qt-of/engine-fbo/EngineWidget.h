#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMutex>
#include <atomic>
#include <cstdint>

// EngineWidget — the UI-side compositor. Owns the QOpenGLContext the
// main window renders through. Each paint it draws a fullscreen quad
// textured with whatever `textureId_` currently names.
//
// The engine thread writes textureId_ (atomic). We never free that
// texture here — it's owned by the engine context and shared via
// AA_ShareOpenGLContexts. Reading it from this context is safe once
// the engine signals the frame is done.
class EngineWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit EngineWidget(QWidget* parent = nullptr);
    ~EngineWidget() override;

    // Called by the engine thread after it finishes writing a new frame
    // into the texture named `texId`. We just stash the id and trigger
    // a repaint on the GUI thread.
    void setEngineTexture(uint32_t texId);

signals:
    void glReady();   // emitted once from initializeGL, on the GUI thread

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    // Fullscreen-quad program + VAO used to draw the engine texture.
    uint32_t program_   = 0;
    uint32_t vao_       = 0;
    uint32_t vbo_       = 0;
    int      locTex_    = -1;

    // Shared engine texture id. 0 = engine hasn't produced a frame yet.
    std::atomic<uint32_t> textureId_{0};
    std::atomic<uint64_t> paintCount_{0};
};
