#pragma once

class QWidget;

namespace Flash {
// A white flash over `target` that fades out (after the macOS
// ScreenshotFlashOverlay): shown after a successful screenshot.
void over(QWidget* target);
}
