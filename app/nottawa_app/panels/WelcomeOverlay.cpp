#include "WelcomeOverlay.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

WelcomeOverlay::WelcomeOverlay(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addStretch();

    auto* card = new QWidget;
    card->setFixedWidth(420);
    card->setStyleSheet("QWidget { background: rgba(30,32,41,230); border-radius: 12px; }");
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 28, 28, 28);
    cardLayout->setSpacing(14);

    auto* title = new QLabel("Welcome to Nottawa");
    title->setStyleSheet("QLabel { color: #ffffff; font-size: 20pt; font-weight: 600; background: transparent; }");
    title->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(title);

    auto* sub = new QLabel("Audio-reactive visuals for live performance.\nOpen a strand to begin, or start fresh.");
    sub->setStyleSheet("QLabel { color: #a8aab6; background: transparent; }");
    sub->setAlignment(Qt::AlignCenter);
    sub->setWordWrap(true);
    cardLayout->addWidget(sub);

    cardLayout->addSpacing(8);

    auto* buttons = new QHBoxLayout;
    buttons->setSpacing(10);
    auto* newBtn  = new QPushButton("New Strand");
    newBtn->setDefault(true);
    auto* openBtn = new QPushButton("Open…");
    buttons->addStretch();
    buttons->addWidget(openBtn);
    buttons->addWidget(newBtn);
    buttons->addStretch();
    cardLayout->addLayout(buttons);

    auto* center = new QHBoxLayout;
    center->addStretch();
    center->addWidget(card);
    center->addStretch();
    root->addLayout(center);
    root->addStretch();

    connect(newBtn,  &QPushButton::clicked, this, &WelcomeOverlay::newStrandRequested);
    connect(openBtn, &QPushButton::clicked, this, &WelcomeOverlay::openStrandRequested);
}

void WelcomeOverlay::paintEvent(QPaintEvent*) {
    // Darken whatever is behind — keeps the stage visible but puts the
    // focus on the welcome card.
    QPainter p(this);
    p.fillRect(rect(), QColor(14, 15, 19, 180));
}
