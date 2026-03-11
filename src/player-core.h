#ifndef PLAYER_CORE_H
#define PLAYER_CORE_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoOutput>
#include <QProperty>
#include <QTimer>
#include <QUrl>

class PlayerCore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    
    Q_PROPERTY(QMediaPlayer* player READ player CONSTANT)
    Q_PROPERTY(QAudioOutput* audioOutput READ audioOutput CONSTANT)
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(double playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    
public:
    explicit PlayerCore(QObject *parent = nullptr);
    ~PlayerCore() = default;
    
    QMediaPlayer* player() const { return m_player; }
    QAudioOutput* audioOutput() const { return m_audioOutput; }
    
    bool isPlaying() const { return m_player->playbackState() == QMediaPlayer::PlayingState; }
    qint64 position() const { return m_player->position(); }
    qint64 duration() const { return m_player->duration(); }
    
    double volume() const { return m_audioOutput->volume(); }
    void setVolume(double vol);
    
    double playbackRate() const { return m_player->playbackRate(); }
    void setPlaybackRate(double rate);
    
    bool muted() const { return m_audioOutput->isMuted(); }
    void setMuted(bool muted);
    
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(qint64 position);
    Q_INVOKABLE void setSource(const QUrl &url);
    Q_INVOKABLE void playNext();
    Q_INVOKABLE void playPrevious();
    
signals:
    void playingChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void playbackRateChanged();
    void mutedChanged();
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void errorOccurred(int error, const QString &errorString);
    
private slots:
    void onPositionChanged();
    void onDurationChanged();
    void onPlayingChanged();
    void onErrorOccurred(QMediaPlayer::Error error, const QString &errorString);
    
private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QTimer *m_updateTimer;
    
    QList<QUrl> m_playlist;
    int m_currentIndex;
};

#endif // PLAYER_CORE_H
