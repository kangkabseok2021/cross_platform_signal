import QtQuick 2.15

Rectangle {
    id: marker
    width: 16; height: 16; radius: 8
    color: "#f59e0b"
    border.color: "white"; border.width: 2

    required property real cellSize
    signal positionChanged(real newX, real newY)

    MouseArea {
        anchors.fill: parent
        drag.target: marker
        onPositionChanged: {
            marker.positionChanged(marker.x / marker.cellSize,
                                   marker.y / marker.cellSize)
        }
    }
}
