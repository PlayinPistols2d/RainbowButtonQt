#ifndef RAINBOWBUTTON_H
#define RAINBOWBUTTON_H

#include <QPushButton>
#include <QList>
#include <QQueue>
#include "EmojiParticle.h"

/**
 * @brief RainbowButton is a custom button with a moving gradient
 * and emoji particle effects.
 */
class RainbowButton : public QPushButton
{
    Q_OBJECT
public:
    explicit RainbowButton(QWidget *parent = nullptr);
    ~RainbowButton() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    void spawnEmojis(const QPoint &clickPos);
    void cleanupOldEmojis();

    int m_offset;
    bool m_pressed;
    bool m_hovered;
    int m_currentSpawnGroup;
    QQueue<int> m_activeSpawnGroups;
    QList<EmojiParticle*> m_activeParticles;
};

#endif // RAINBOWBUTTON_H
