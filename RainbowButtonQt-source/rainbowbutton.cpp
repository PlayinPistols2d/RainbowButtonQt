#include "RainbowButton.h"
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QRandomGenerator>

RainbowButton::RainbowButton(QWidget *parent)
    : QPushButton(parent),
      m_offset(0),
      m_pressed(false),
      m_hovered(false),
      m_currentSpawnGroup(0)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QFont btnFont = font();
    btnFont.setBold(true);
    setFont(btnFont);

    // Timer for updating the gradient offset.
    startTimer(30);
}

RainbowButton::~RainbowButton()
{
    // Clean up active emoji particles.
    qDeleteAll(m_activeParticles);
    m_activeParticles.clear();
}

void RainbowButton::spawnEmojis(const QPoint &clickPos)
{
    static QStringList emojis = { "🎉", "🥳", "✨", "🔥", "💖", "🎊", "😃", "🌟", "🎁", "👾" };

    // If there are already 3 active spawn groups, clean up the oldest group immediately.
    if (m_activeSpawnGroups.size() >= 3) {
        cleanupOldEmojis();
    }

    m_currentSpawnGroup++;
    m_activeSpawnGroups.enqueue(m_currentSpawnGroup);

    QPoint globalPos = mapTo(window(), clickPos);

    // Create 10 emoji particles with slight random offset.
    for (int i = 0; i < 10; ++i) {
        QPoint spawnPos = globalPos + QPoint(
                              QRandomGenerator::global()->bounded(-15, 16),
                              QRandomGenerator::global()->bounded(-15, 16)
                              );

        EmojiParticle *emoji = new EmojiParticle(
                    emojis[QRandomGenerator::global()->bounded(emojis.size())],
                    spawnPos,
                    window()
                    );
        emoji->spawnGroupId = m_currentSpawnGroup;
        emoji->show();

        m_activeParticles.append(emoji);
        // When an emoji is deleted, remove its pointer from the list.
        connect(emoji, &QObject::destroyed, this, [this, emoji]() {
            m_activeParticles.removeAll(emoji);
        });
    }
}

void RainbowButton::cleanupOldEmojis()
{
    if (m_activeSpawnGroups.isEmpty())
        return;

    int oldestGroup = m_activeSpawnGroups.dequeue();
    // Immediately trigger fade-out for all emojis belonging to the oldest group.
    for (EmojiParticle *particle : m_activeParticles) {
        if (particle->spawnGroupId == oldestGroup) {
            particle->startFadeOut();
        }
    }
}

void RainbowButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Create a linear gradient for the button background.
    QLinearGradient gradient(0, 0, width(), 0);
    const int steps = 8;
    for (int i = 0; i <= steps; ++i) {
        float pos = static_cast<float>(i) / steps;
        int hue = (m_offset + static_cast<int>(pos * 360)) % 360;
        QColor color = QColor::fromHsv(hue, 255, 255);
        if (m_pressed)
            color = color.darker(120);
        else if (m_hovered)
            color = color.lighter(110);
        gradient.setColorAt(pos, color);
    }

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    QRectF btnRect = m_pressed ? rect().adjusted(2, 2, -2, -2) : rect();
    painter.drawRoundedRect(btnRect, 10, 10);

    // Draw the button text.
    painter.setFont(font());
    painter.setPen(Qt::white);
    painter.drawText(btnRect.toRect(), Qt::AlignCenter, text());
}

void RainbowButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);
    m_hovered = true;
    update();
}

void RainbowButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    m_hovered = false;
    update();
}

void RainbowButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    m_pressed = true;
    spawnEmojis(event->pos());
    update();
}

void RainbowButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    m_pressed = false;
    update();
}

void RainbowButton::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    m_offset = (m_offset + 1) % 360;
    update();
}
