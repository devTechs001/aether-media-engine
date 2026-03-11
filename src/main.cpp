#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QMediaDevices>
#include <QMediaMetaData>
#include <QVariantList>
#include <QVariantMap>

#include "player-core.h"
#include "playlist-model.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("MediaGUI");
    app.setOrganizationName("DarkHat");
    
    // Register custom types
    qmlRegisterType<PlayerCore>("MediaGUI", 1, 0, "PlayerCore");
    qmlRegisterType<PlaylistModel>("MediaGUI", 1, 0, "PlaylistModel");
    
    QQmlApplicationEngine engine;
    
    // Create player core
    PlayerCore playerCore;
    
    // Expose to QML
    engine.rootContext()->setContextProperty("playerCore", &playerCore);
    
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    
    engine.load(url);
    
    return app.exec();
}
