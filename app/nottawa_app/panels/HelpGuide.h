#pragma once

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QTimer>

class BrowserPanel;
class HelpCard;
class InspectorPanel;
class NodeEditorView;
class QWidget;

// The guided tutorial, after the macOS HelpGuideViewModel and
// HelpOverlayView. The step is worked out from the graph each tick: place
// a source, an effect, connect them, a second source, blend the two by
// joining outputs, select the Blend, change its Mode and Main Input
// Strength, then the action-bar tour, sharing, and a thank-you. Cards with
// bouncing arrows point at the browser, the inspector or the nodes
// involved; only the last three take clicks.
class HelpGuide : public QObject {
    Q_OBJECT
public:
    enum Step {
        PlaceSource, PlaceEffect, ConnectSourceToEffect, PlaceSecondSource, BlendViaOutputs,
        OpenInspector, EditBlendMode, EditAlpha, ActionButtonsTour, ShareHelp, Completion,
    };

    // Cards are laid over `canvas` (the centre area holding `editor`).
    HelpGuide(QWidget* canvas, NodeEditorView* editor, InspectorPanel* inspector, BrowserPanel* browser,
              QObject* parent = nullptr);

    bool enabled() const { return enabled_; }
    void setEnabled(bool on);
    // Starts over from the first step (Help > Nottawa Tutorial).
    void restart();
    // First launch only, unless already completed.
    void autoStartIfFirstLaunch();
    // Cards hide while `blocker` (the welcome screen) is shown.
    void setBlocker(QWidget* blocker) { blocker_ = blocker; }

signals:
    void enabledChanged(bool on);
    void sidebarNeeded();

private:
    Step currentStep();
    void tick();
    void showCards(Step step);
    void updateTooltipTarget(Step step);
    void hideAll();
    void setCompleted(bool on);
    bool active() const;

    QWidget*         canvas_;
    NodeEditorView*  editor_;
    InspectorPanel*  inspector_;
    BrowserPanel*    browser_;
    QPointer<QWidget> blocker_;
    QTimer           timer_;
    bool             enabled_ = false;
    bool             completed_ = false;
    bool             editedMode_ = false, editedAlpha_ = false;
    bool             seenActions_ = false, seenShare_ = false;
    // Blend parameter values when the step began, to notice edits.
    QString          blendId_;
    int              modeAtStart_ = -1;
    float            alphaAtStart_ = -1;
    int              lastStep_ = -1;
    int              ticks_ = 0;
    QSize            lastArea_;
    // The node tooltip's target and text, refreshed with the step.
    QString          tipTarget_, tipTitle_, tipBody_;
    int              tipDir_ = 0;
    QPointer<HelpCard> card_;
    QPointer<HelpCard> nodeCard_;
};
