#ifndef PLAYLIST_MODEL_H
#define PLAYLIST_MODEL_H

#include <QAbstractListModel>
#include <QUrl>
#include <QQmlEngine>

struct PlaylistItem {
    QUrl url;
    QString title;
    QString artist;
    QString duration;
};

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    
public:
    enum PlaylistRoles {
        UrlRole = Qt::UserRole + 1,
        TitleRole,
        ArtistRole,
        DurationRole
    };
    
    explicit PlaylistModel(QObject *parent = nullptr);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    Q_INVOKABLE void addMedia(const QUrl &url);
    Q_INVOKABLE void addMedias(const QVariantList &urls);
    Q_INVOKABLE void removeMedia(int index);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QString getMediaUrl(int index) const;
    
    int count() const { return m_items.size(); }
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);
    
signals:
    void countChanged();
    void currentIndexChanged();
    
private:
    QList<PlaylistItem> m_items;
    int m_currentIndex;
};

#endif // PLAYLIST_MODEL_H
