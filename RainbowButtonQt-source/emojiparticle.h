#ifndef EMOJIPARTICLE_H
#define EMOJIPARTICLE_H

#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QPaintEvent>

/**
 * @brief EmojiParticle displays an emoji that falls with physics,
 * then fades out after settling.
 */
class EmojiParticle : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
public:
    explicit EmojiParticle(const QString &emoji, const QPoint &spawnPos, QWidget *parent = nullptr);
    ~EmojiParticle() override = default;

    float opacity() const;
    void setOpacity(float opacity);

    // Initiates the fade-out animation immediately.
    void startFadeOut();

    // Group ID used for cleaning up groups of particles.
    int spawnGroupId = 0;

signals:
    void opacityChanged(float opacity);

protected:
    void paintEvent(QPaintEvent *event) override;
    void updatePosition();
    QRect getApplicationBounds() const;

private:
    QTimer *moveTimer = nullptr;
    QPropertyAnimation *fadeAnimation = nullptr;
    float vx, vy;
    float gravity;
    bool peakReached;
    float m_opacity;

    // Flag to ensure fade-out is scheduled only once after reaching ground.
    bool m_fadeOutScheduled = false;
};

#endif // EMOJIPARTICLE_H
