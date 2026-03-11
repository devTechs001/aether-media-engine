#include "player-core.h"
#include <QDebug>

PlayerCore::PlayerCore(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_updateTimer(new QTimer(this))
    , m_currentIndex(-1)
{
    m_player->setAudioOutput(m_audioOutput);
    
    // Connect signals
    connect(m_player, &QMediaPlayer::positionChanged, this, &PlayerCore::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &PlayerCore::onDurationChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &PlayerCore::onPlayingChanged);
    connect(m_player, &QMediaPlayer::errorOccurred, this, &PlayerCore::onErrorOccurred);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &PlayerCore::mediaStatusChanged);
    
    // Update timer for smooth progress bar
    m_updateTimer->setInterval(100);
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        emit positionChanged();
    });
}

void PlayerCore::setVolume(double vol)
{
    m_audioOutput->setVolume(vol);
    emit volumeChanged();
}

void PlayerCore::setPlaybackRate(double rate)
{
    m_player->setPlaybackRate(rate);
    emit playbackRateChanged();
}

void PlayerCore::setMuted(bool muted)
{
    m_audioOutput->setMuted(muted);
    emit mutedChanged();
}

void PlayerCore::play()
{
    m_player->play();
    m_updateTimer->start();
}

void PlayerCore::pause()
{
    m_player->pause();
}

void PlayerCore::stop()
{
    m_player->stop();
    m_updateTimer->stop();
}

void PlayerCore::seek(qint64 position)
{
    m_player->setPosition(position);
}

void PlayerCore::setSource(const QUrl &url)
{
    m_player->setSource(url);
    m_updateTimer->start();
}

void PlayerCore::playNext()
{
    if (m_currentIndex < m_playlist.size() - 1) {
        m_currentIndex++;
        setSource(m_playlist[m_currentIndex]);
        play();
    }
}

void PlayerCore::playPrevious()
{
    if (m_currentIndex > 0) {
        m_currentIndex--;
        setSource(m_playlist[m_currentIndex]);
        play();
    }
}

void PlayerCore::onPositionChanged()
{
    emit positionChanged();
}

void PlayerCore::onDurationChanged()
{
    emit durationChanged();
}

void PlayerCore::onPlayingChanged()
{
    emit playingChanged();
    if (!isPlaying()) {
        m_updateTimer->stop();
    }
}

void PlayerCore::onErrorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    qWarning() << "Player error:" << error << errorString;
    emit errorOccurred(static_cast<int>(error), errorString);
}
