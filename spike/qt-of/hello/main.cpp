// Nottawa Qt spike — Day 1: proves the toolchain.
// Opens a plain Qt main window with a label. If this builds and runs,
// Qt + MSVC 2022 + windeployqt are all wired up correctly on the box.
//
// Day 2 wires in a QOpenGLWidget and shares a GL context with the OF
// engine. Day 3 renders an engine FBO through that widget at 1920x1080
// and measures 60fps on the RTX 3070.

#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    QMainWindow w;
    w.setWindowTitle("Nottawa Qt spike — Day 1");
    w.resize(800, 600);

    auto* central = new QWidget;
    auto* layout  = new QVBoxLayout(central);
    layout->addWidget(new QLabel(
        "Qt 6.8 toolchain OK.\n\n"
        "Day 2: embed OF engine, share GL context.\n"
        "Day 3: render engine FBO at 1920x1080, measure fps."
    ));
    w.setCentralWidget(central);
    w.show();

    return app.exec();
}
