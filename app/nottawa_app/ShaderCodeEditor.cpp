#include "ShaderCodeEditor.h"

#include "EngineBridge.h"
#include "Theme.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QShortcut>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QVBoxLayout>

namespace {

// Keywords, types, built-ins, numbers, comments and preprocessor lines.
class GlslHighlighter : public QSyntaxHighlighter {
public:
    explicit GlslHighlighter(QTextDocument* doc) : QSyntaxHighlighter(doc) {
        auto format = [](const QColor& c, bool bold = false) {
            QTextCharFormat f;
            f.setForeground(c);
            if (bold) f.setFontWeight(QFont::DemiBold);
            return f;
        };
        const QStringList keywords = {"if", "else", "for", "while", "do", "return", "break", "continue", "discard",
                                      "const", "uniform", "in", "out", "inout", "struct", "layout", "precision",
                                      "highp", "mediump", "lowp", "flat", "smooth"};
        const QStringList types = {"void", "bool", "int", "uint", "float", "double", "vec2", "vec3", "vec4", "ivec2",
                                   "ivec3", "ivec4", "bvec2", "bvec3", "bvec4", "mat2", "mat3", "mat4",
                                   "sampler2D", "sampler2DRect", "samplerCube"};
        const QStringList builtins = {"sin", "cos", "tan", "asin", "acos", "atan", "pow", "exp", "log", "sqrt",
                                      "abs", "sign", "floor", "ceil", "fract", "mod", "min", "max", "clamp", "mix",
                                      "step", "smoothstep", "length", "distance", "dot", "cross", "normalize",
                                      "reflect", "texture", "texelFetch", "textureSize", "gl_FragCoord", "gl_Position"};
        auto words = [](const QStringList& list) { return QRegularExpression("\\b(" + list.join('|') + ")\\b"); };
        rules_ = {
            {words(keywords), format(QColor(0xc6, 0x78, 0xdd), true)},
            {words(types), format(QColor(0x61, 0xaf, 0xef))},
            {words(builtins), format(QColor(0x56, 0xb6, 0xc2))},
            {QRegularExpression("\\b[0-9]+(\\.[0-9]*)?([eE][-+]?[0-9]+)?f?\\b"), format(QColor(0xd1, 0x9a, 0x66))},
            {QRegularExpression("^\\s*#.*"), format(QColor(0xe5, 0xc0, 0x7b))},
            {QRegularExpression("//.*"), format(QColor(0x7f, 0x84, 0x8e))},
        };
        comment_ = format(QColor(0x7f, 0x84, 0x8e));
    }

protected:
    void highlightBlock(const QString& text) override {
        for (const auto& [re, f] : rules_) {
            auto it = re.globalMatch(text);
            while (it.hasNext()) { const auto m = it.next(); setFormat(int(m.capturedStart()), int(m.capturedLength()), f); }
        }
        // /* block comments */, possibly spanning lines (state 1 = open).
        setCurrentBlockState(0);
        int start = previousBlockState() == 1 ? 0 : int(text.indexOf("/*"));
        while (start >= 0) {
            const int end = int(text.indexOf("*/", start));
            int length;
            if (end < 0) {
                setCurrentBlockState(1);
                length = int(text.length()) - start;
            } else {
                length = end - start + 2;
            }
            setFormat(start, length, comment_);
            start = int(text.indexOf("/*", start + length));
        }
    }

private:
    QVector<std::pair<QRegularExpression, QTextCharFormat>> rules_;
    QTextCharFormat comment_;
};

QPushButton* toolbarButton(const QString& text, QWidget* parent) {
    auto* b = new QPushButton(text, parent);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

} // anon

ShaderCodeEditor::ShaderCodeEditor(const QString& shaderId, const QString& name, QWidget* parent)
    : QWidget(parent, Qt::Window), shaderId_(shaderId) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString("%1 — Shader Code").arg(name));
    resize(820, 620);
    setStyleSheet(QString("ShaderCodeEditor { background: %1; }").arg(Theme::background.name()));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* toolbar = new QWidget(this);
    toolbar->setStyleSheet(QString("background: %1;").arg(Theme::backgroundSecondary.name()));
    auto* bar = new QHBoxLayout(toolbar);
    bar->setContentsMargins(12, 8, 12, 8);
    bar->setSpacing(10);
    auto* title = new QLabel(name, toolbar);
    title->setStyleSheet(QString("color: %1; font-size: 11pt; font-weight: 600;").arg(Theme::textPrimary.name()));
    modified_ = new QLabel("MODIFIED", toolbar);
    modified_->setStyleSheet(QString("color: %1; border: 1px solid %1; border-radius: 4px; padding: 1px 6px;"
                                     " font-family: Consolas; font-size: 8pt; font-weight: 700;").arg(Theme::warning.name()));
    status_ = new QLabel(toolbar);
    revert_ = toolbarButton("Revert", toolbar);
    revert_->setToolTip("Drop the session override and reload the shader from disk");
    apply_ = toolbarButton("Apply", toolbar);
    apply_->setToolTip("Compile and apply now (Ctrl+Enter)");
    auto* close = toolbarButton("Close", toolbar);
    connect(revert_, &QPushButton::clicked, this, &ShaderCodeEditor::revert);
    connect(apply_, &QPushButton::clicked, this, &ShaderCodeEditor::apply);
    connect(close, &QPushButton::clicked, this, &QWidget::close);
    bar->addWidget(title);
    bar->addWidget(modified_);
    bar->addStretch(1);
    bar->addWidget(status_);
    bar->addWidget(revert_);
    bar->addWidget(apply_);
    bar->addWidget(close);
    layout->addWidget(toolbar);
    auto* line = new QFrame(this);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    layout->addWidget(line);

    editor_ = new QPlainTextEdit(this);
    QFont mono("Consolas");
    mono.setPointSize(10);
    editor_->setFont(mono);
    editor_->setLineWrapMode(QPlainTextEdit::NoWrap);
    editor_->setTabStopDistance(QFontMetricsF(mono).horizontalAdvance(' ') * 2);
    editor_->setStyleSheet(QString("QPlainTextEdit { background: %1; color: %2; border: none; padding: 8px;"
                                   " selection-background-color: %3; }")
                               .arg(Theme::background.name(), Theme::textPrimary.name(), Theme::surfacePressed.name()));
    new GlslHighlighter(editor_->document());
    layout->addWidget(editor_, 1);

    errorBox_ = new QWidget(this);
    QColor tint = Theme::warning;
    tint.setAlphaF(0.12);
    errorBox_->setStyleSheet(QString("background: %1; border-top: 1px solid %2;").arg(tint.name(QColor::HexArgb), Theme::border.name()));
    auto* errorLayout = new QVBoxLayout(errorBox_);
    errorLayout->setContentsMargins(10, 8, 10, 8);
    errorTitle_ = new QLabel(errorBox_);
    errorTitle_->setStyleSheet(QString("color: %1; font-weight: 600; background: transparent; border: none;").arg(Theme::textPrimary.name()));
    errorLog_ = new QLabel(errorBox_);
    errorLog_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    errorLog_->setWordWrap(true);
    errorLog_->setStyleSheet(QString("color: %1; font-family: Consolas; font-size: 9pt; background: transparent; border: none;")
                                 .arg(Theme::textSecondary.name()));
    auto* logScroll = new QScrollArea(errorBox_);
    logScroll->setWidget(errorLog_);
    logScroll->setWidgetResizable(true);
    logScroll->setFrameShape(QFrame::NoFrame);
    logScroll->setMaximumHeight(100);
    logScroll->setStyleSheet("background: transparent; border: none;");
    errorLayout->addWidget(errorTitle_);
    errorLayout->addWidget(logScroll);
    errorBox_->hide();
    layout->addWidget(errorBox_);

    // Window-wide, so they work while typing in the editor.
    new QShortcut(QKeySequence("Ctrl+Return"), this, this, &ShaderCodeEditor::apply);
    new QShortcut(QKeySequence("Ctrl+Enter"), this, this, &ShaderCodeEditor::apply);
    new QShortcut(QKeySequence(Qt::Key_Escape), this, this, &QWidget::close);
    debounce_.setSingleShot(true);
    debounce_.setInterval(500);
    connect(&debounce_, &QTimer::timeout, this, &ShaderCodeEditor::apply);
    connect(editor_, &QPlainTextEdit::textChanged, this, [this] {
        debounce_.start();
        updateState();
    });
    load();
}

void ShaderCodeEditor::load() {
    const QSignalBlocker block(editor_);
    editor_->setPlainText(EngineBridge::instance()->shaderSource(shaderId_));
    lastApplied_ = editor_->toPlainText();
    status_->clear();
    errorBox_->hide();
    errorLine_ = 0;
    editor_->setExtraSelections({});
    updateState();
}

void ShaderCodeEditor::updateState() {
    const bool hasOverride = EngineBridge::instance()->shaderHasOverride(shaderId_);
    modified_->setVisible(hasOverride);
    revert_->setEnabled(hasOverride);
    apply_->setEnabled(editor_->toPlainText() != lastApplied_);
}

void ShaderCodeEditor::apply() {
    debounce_.stop();
    const QString source = editor_->toPlainText();
    if (source == lastApplied_) return;
    const ShaderCompileResult result = EngineBridge::instance()->setShaderSource(shaderId_, source);
    status_->setText(QString("<span style='color:%1'>%2</span>")
                         .arg((result.ok ? Theme::success : Theme::warning).name(), result.ok ? "✓ Compiled" : "⚠ Compile error"));
    errorBox_->setVisible(!result.ok);
    QList<QTextEdit::ExtraSelection> marks;
    if (result.ok) {
        lastApplied_ = source;
        errorLine_ = 0;
        emit overrideChanged();
    } else {
        errorLine_ = result.errorLine;
        errorTitle_->setText(result.errorLine > 0 ? QString("Compile error — line %1").arg(result.errorLine) : "Compile error");
        errorLog_->setText(result.log.trimmed());
        if (result.errorLine > 0) {
            QTextEdit::ExtraSelection mark;
            mark.cursor = QTextCursor(editor_->document()->findBlockByLineNumber(result.errorLine - 1));
            mark.format.setBackground(QColor(0xd2, 0x99, 0x22, 50));
            mark.format.setProperty(QTextFormat::FullWidthSelection, true);
            marks << mark;
        }
    }
    editor_->setExtraSelections(marks);
    updateState();
}

void ShaderCodeEditor::revert() {
    EngineBridge::instance()->revertShaderSource(shaderId_);
    load();
    emit overrideChanged();
}


