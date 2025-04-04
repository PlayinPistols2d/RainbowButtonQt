#include "EmojiParticle.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QFontMetrics>
#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QLinearGradient>
#include <QtMath>

EmojiParticle::EmojiParticle(const QString &emoji, const QPoint &spawnPos, QWidget *parent)
    : QLabel(parent),
      gravity(1.0f),        // Realistic gravity
      peakReached(false),
      m_opacity(1.0f)
{
    setText(emoji);
    setAttribute(Qt::WA_TranslucentBackground);

    QFont emojiFont;
    emojiFont.setPointSize(18);
    setFont(emojiFont);

    QFontMetrics fm(emojiFont);
    setFixedSize(fm.horizontalAdvance(emoji) + 8, fm.height() + 8);

    setAlignment(Qt::AlignCenter);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    // Center the emoji at the spawn position.
    move(spawnPos.x() - width() / 2, spawnPos.y() - height() / 2);

    // Increase horizontal velocity range so the emoji "blows" further.
    vx = QRandomGenerator::global()->bounded(-7, 8);
    vy = QRandomGenerator::global()->bounded(-20, -12);

    // Start the movement timer.
    moveTimer = new QTimer(this);
    connect(moveTimer, &QTimer::timeout, this, &EmojiParticle::updatePosition);
    moveTimer->start(20);  // 16 will be ≈60 FPS, possible overkill tho.
}

float EmojiParticle::opacity() const
{
    return m_opacity;
}

void EmojiParticle::setOpacity(float opacity)
{
    if (qFuzzyCompare(m_opacity, opacity))
        return;
    m_opacity = qBound(0.0f, opacity, 1.0f);
    update();
}

void EmojiParticle::startFadeOut()
{
    // Prevent duplicate fade-out calls.
    if (fadeAnimation)
        return;

    // Stop the movement timer to avoid further physics updates.
    if (moveTimer)
        moveTimer->stop();

    fadeAnimation = new QPropertyAnimation(this, "opacity", this);
    fadeAnimation->setDuration(1000);  // Fade out over 1 second.
    fadeAnimation->setStartValue(m_opacity);
    fadeAnimation->setEndValue(0.0f);
    fadeAnimation->setEasingCurve(QEasingCurve::OutQuad);
    connect(fadeAnimation, &QPropertyAnimation::finished, this, &QObject::deleteLater);
    fadeAnimation->start();
}

void EmojiParticle::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setOpacity(m_opacity);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(font());
    painter.drawText(rect(), Qt::AlignCenter, text());
}

QRect EmojiParticle::getApplicationBounds() const
{
    if (QWidget* topLevel = window()) {
        QRect screenGeo = topLevel->geometry();
        if (parentWidget()) {
            QPoint topLeft = parentWidget()->mapFromGlobal(screenGeo.topLeft());
            return QRect(topLeft, screenGeo.size());
        }
        return screenGeo;
    }
    return QRect();
}

void EmojiParticle::updatePosition()
{
    QRect bounds = getApplicationBounds();
    if (bounds.isNull())
        return;

    int newX = x() + vx;
    int newY = y() + vy;

    // Check collision with left/right boundaries.
    if (newX < bounds.left()) {
        newX = bounds.left();
        vx = -vx * 0.8f;
    }
    else if (newX > bounds.right() - width()) {
        newX = bounds.right() - width();
        vx = -vx * 0.8f;
    }

    // Check collision with top boundary.
    if (newY < bounds.top()) {
        newY = bounds.top();
        vy = -vy * 0.8f;
    }
    // Check for collision with the bottom ("ground").
    else if (newY >= bounds.bottom() - height()) {
        newY = bounds.bottom() - height();
        // Once the emoji hits the ground, start fade-out immediately.
        if (!m_fadeOutScheduled) {
            m_fadeOutScheduled = true;
            QTimer::singleShot(0, this, &EmojiParticle::startFadeOut);
        }
    }
    else {
        // Apply slight air resistance when in the air.
        vx *= 0.99f;
    }

    // Apply gravitational acceleration only if not at the ground.
    if (newY < bounds.bottom() - height())
        vy += gravity;

    move(newX, newY);
}
