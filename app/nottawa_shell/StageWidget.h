#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <atomic>
#include <cstdint>

// StageWidget — displays the engine's current output FBO. One
// QOpenGLWidget on the UI thread; a background StageEngine renders into
// a shared texture; this widget binds that texture every paint. Mirror
// of the spike's EngineWidget.
class StageWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit StageWidget(QWidget* parent = nullptr);
    ~StageWidget() override;

    void setEngineTexture(uint32_t id);

signals:
    void glReady();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    uint32_t program_ = 0;
    uint32_t vao_     = 0;
    uint32_t vbo_     = 0;
    int      locTex_  = -1;
    std::atomic<uint32_t> textureId_{0};
};
