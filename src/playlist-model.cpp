#include "playlist-model.h"
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

PlaylistModel::PlaylistModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentIndex(-1)
{
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();
    
    const PlaylistItem &item = m_items.at(index.row());
    
    switch (role) {
    case UrlRole:
        return item.url;
    case TitleRole:
        return item.title;
    case ArtistRole:
        return item.artist;
    case DurationRole:
        return item.duration;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    return {
        {UrlRole, "mediaUrl"},
        {TitleRole, "title"},
        {ArtistRole, "artist"},
        {DurationRole, "duration"}
    };
}

void PlaylistModel::addMedia(const QUrl &url)
{
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    
    PlaylistItem item;
    item.url = url;
    
    QFileInfo fi(url.toLocalFile());
    item.title = fi.completeBaseName();
    item.artist = "Unknown";
    item.duration = "00:00";
    
    m_items.append(item);
    endInsertRows();
    
    emit countChanged();
}

void PlaylistModel::addMedias(const QVariantList &urls)
{
    for (const QVariant &urlVar : urls) {
        addMedia(urlVar.toUrl());
    }
}

void PlaylistModel::removeMedia(int index)
{
    if (index < 0 || index >= m_items.size())
        return;
    
    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();
    
    if (m_currentIndex >= m_items.size()) {
        m_currentIndex = m_items.size() - 1;
    }
    
    emit countChanged();
}

void PlaylistModel::clear()
{
    beginResetModel();
    m_items.clear();
    m_currentIndex = -1;
    endResetModel();
    
    emit countChanged();
    emit currentIndexChanged();
}

QString PlaylistModel::getMediaUrl(int index) const
{
    if (index < 0 || index >= m_items.size())
        return QString();
    return m_items.at(index).url.toString();
}

void PlaylistModel::setCurrentIndex(int index)
{
    if (m_currentIndex != index) {
        m_currentIndex = index;
        emit currentIndexChanged();
    }
}
