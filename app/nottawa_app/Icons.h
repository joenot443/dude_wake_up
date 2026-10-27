#pragma once

#include <QColor>
#include <QIcon>
#include <QRectF>

class QPainter;

// Line icons drawn with QPainter, standing in for the SF Symbols the macOS
// app uses. Designed on a 16×16 grid and scaled to the target rect.
namespace Icons {

enum Kind {
    Video, Sparkles, Waveform, Books, Stack,
    Search, Star, StarFilled, Refresh, Close,
    Text, Smiley, Photo, Webcam,
    ChevronDown, ChevronRight, ChevronUp, Film, Cloud,
    Note, Keys, Reset, Plus, Minus, Play, Pause,
    Undo, Redo, Sidebar, Sliders, Trash, Camera, Help, Fit, ChevronLeft, Clear,
    Lock, Unlock, Stop, Folder, NewSquare, Speaker, SpeakerMuted,
};

void  paint(QPainter* p, const QRectF& rect, Kind kind, const QColor& color);
// Drawn at the size and pixel ratio it's shown at. `activeColor`: when
// checked (QIcon::On).
QIcon icon(Kind kind, const QColor& color, const QColor& activeColor = QColor());
// Two shapes: `off`, and `on` when checked.
QIcon toggle(Kind off, const QColor& offColor, Kind on, const QColor& onColor);

} // namespace Icons
