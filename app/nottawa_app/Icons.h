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
    Lock, Unlock, Stop,
};

void  paint(QPainter* p, const QRectF& rect, Kind kind, const QColor& color);
QIcon icon(Kind kind, const QColor& color, const QColor& activeColor = QColor());

} // namespace Icons
