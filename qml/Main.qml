import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

ApplicationWindow {
    id: window
    width: 1200
    height: 800
    visible: true
    title: qsTr("Media Player GUI")
    color: "#1a1a2e"
    
    // Header
    header: ToolBar {
        background: Rectangle { color: "#16213e" }
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            
            Label {
                text: "🎬 Media Player"
                font.pixelSize: 24
                font.bold: true
                color: "#e94560"
            }
            
            Item { Layout.fillWidth: true }
            
            ToolButton {
                text: "📂 Open File"
                onClicked: fileDialog.open()
            }
            
            ToolButton {
                text: "📁 Open Folder"
                onClicked: folderDialog.open()
            }
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // Video Display Area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 400
            color: "#0f0f23"
            radius: 10
            clip: true
            
            VideoOutput {
                id: videoOutput
                source: playerCore.player
                anchors.fill: parent
                fillMode: VideoOutput.PreserveAspectFit
                
                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: {
                        window.visibility = window.visibility === Window.FullScreen 
                                          ? Window.Windowed : Window.FullScreen
                    }
                    onClicked: {
                        if (playerCore.playing) {
                            playerCore.pause()
                        } else {
                            playerCore.play()
                        }
                    }
                }
            }
            
            // Play/Pause overlay
            Rectangle {
                id: playOverlay
                anchors.centerIn: parent
                width: 80
                height: 80
                radius: 40
                color: "#e94560"
                opacity: playerCore.playing ? 0 : 0.8
                visible: !playerCore.playing
                
                Text {
                    anchors.centerIn: parent
                    text: "▶"
                    font.pixelSize: 40
                    color: "white"
                }
            }
        }
        
        // Controls
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            color: "#16213e"
            radius: 10
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10
                
                // Progress Slider
                Slider {
                    id: progressSlider
                    Layout.fillWidth: true
                    from: 0
                    to: playerCore.duration
                    value: playerCore.position
                    
                    onMoved: playerCore.seek(value)
                    
                    background: Rectangle {
                        x: progressSlider.leftPadding
                        y: progressSlider.topPadding + progressSlider.availableHeight / 2 - height / 2
                        implicitWidth: 200
                        implicitHeight: 6
                        width: progressSlider.availableWidth
                        radius: 3
                        color: "#333355"
                        
                        Rectangle {
                            width: progressSlider.visualPosition * parent.width
                            height: parent.height
                            color: "#e94560"
                            radius: 3
                        }
                    }
                    
                    handle: Rectangle {
                        x: progressSlider.leftPadding + progressSlider.visualPosition * (progressSlider.availableWidth - width)
                        y: progressSlider.topPadding + progressSlider.availableHeight / 2 - height / 2
                        implicitWidth: 16
                        implicitHeight: 16
                        radius: 8
                        color: progressSlider.pressed ? "#e94560" : "#ffffff"
                    }
                }
                
                // Time labels and controls
                RowLayout {
                    Layout.fillWidth: true
                    
                    Label {
                        text: formatTime(playerCore.position)
                        color: "#aaaaaa"
                        font.pixelSize: 12
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    // Playback controls
                    RowLayout {
                        spacing: 10
                        
                        ToolButton {
                            text: "⏮"
                            font.pixelSize: 20
                            onClicked: playerCore.playPrevious()
                        }
                        
                        ToolButton {
                            text: playerCore.playing ? "⏸" : "▶"
                            font.pixelSize: 24
                            onClicked: playerCore.playing ? playerCore.pause() : playerCore.play()
                        }
                        
                        ToolButton {
                            text: "⏹"
                            font.pixelSize: 20
                            onClicked: playerCore.stop()
                        }
                        
                        ToolButton {
                            text: "⏭"
                            font.pixelSize: 20
                            onClicked: playerCore.playNext()
                        }
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    // Speed control
                    RowLayout {
                        Label {
                            text: "Speed:"
                            color: "#aaaaaa"
                            font.pixelSize: 12
                        }
                        
                        ComboBox {
                            model: ["0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "2.0x"]
                            currentIndex: 2
                            onCurrentIndexChanged: {
                                const speeds = [0.5, 0.75, 1.0, 1.25, 1.5, 2.0]
                                playerCore.playbackRate = speeds[currentIndex]
                            }
                        }
                    }
                    
                    Item { Layout.preferredWidth: 20 }
                    
                    // Volume control
                    RowLayout {
                        ToolButton {
                            text: playerCore.muted ? "🔇" : "🔊"
                            onClicked: playerCore.muted = !playerCore.muted
                        }
                        
                        Slider {
                            id: volumeSlider
                            width: 100
                            from: 0
                            to: 1
                            value: playerCore.volume
                            onMoved: playerCore.volume = value
                        }
                    }
                    
                    Item { Layout.preferredWidth: 20 }
                    
                    Label {
                        text: formatTime(playerCore.duration)
                        color: "#aaaaaa"
                        font.pixelSize: 12
                    }
                }
            }
        }
        
        // Playlist
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 200
            color: "#16213e"
            radius: 10
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5
                
                Label {
                    text: "📋 Playlist"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#e94560"
                }
                
                ListView {
                    id: playlistView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: playlistModel
                    
                    delegate: Rectangle {
                        width: playlistView.width - 10
                        height: 50
                        color: index === playlistModel.currentIndex ? "#e9456033" : "transparent"
                        radius: 5
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            
                            Text {
                                text: model.title
                                color: "white"
                                font.pixelSize: 14
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            
                            Text {
                                text: model.duration
                                color: "#888888"
                                font.pixelSize: 12
                            }
                            
                            ToolButton {
                                text: "❌"
                                onClicked: playlistModel.removeMedia(index)
                            }
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                playlistModel.setCurrentIndex(index)
                                playerCore.setSource(model.mediaUrl)
                                playerCore.play()
                            }
                        }
                    }
                    
                    ScrollBar.vertical: ScrollBar {
                        active: true
                        policy: ScrollBar.AsNeeded
                    }
                }
            }
        }
    }
    
    // File Dialog
    FileDialog {
        id: fileDialog
        title: "Open Media File"
        nameFilters: ["Media files (*.mp4 *.mkv *.avi *.mov *.wmv *.flv *.webm)",
                     "Audio files (*.mp3 *.wav *.flac *.aac *.ogg *.wma)",
                     "All files (*)"]
        onAccepted: {
            playerCore.setSource(selectedFile)
            playlistModel.addMedia(selectedFile)
            playerCore.play()
        }
    }
    
    // Folder Dialog
    FileDialog {
        id: folderDialog
        title: "Open Folder"
        fileMode: FileDialog.Folder
        onAccepted: {
            // Add all media files from folder
            playlistModel.addMedia(selectedFile)
        }
    }
    
    function formatTime(ms) {
        var totalSeconds = Math.floor(ms / 1000)
        var hours = Math.floor(totalSeconds / 3600)
        var minutes = Math.floor((totalSeconds % 3600) / 60)
        var seconds = totalSeconds % 60
        
        if (hours > 0) {
            return Qt.formatNumber(hours, "0") + ":" + 
                   Qt.formatNumber(minutes, "00") + ":" + 
                   Qt.formatNumber(seconds, "00")
        } else {
            return Qt.formatNumber(minutes, "00") + ":" + 
                   Qt.formatNumber(seconds, "00")
        }
    }
}
