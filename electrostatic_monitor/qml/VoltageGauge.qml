import QtQuick
import QtQuick.Controls

Item {
    id: gauge
    property double value: 0
    property color accentColor: "#3b82f6"
    property double warningThreshold: 1.0
    property double criticalThreshold: 2.0

    width: 250
    height: 250

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var centerX = width / 2;
            var centerY = height / 2;
            var radius = Math.min(width, height) / 2 - 20;

            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, 0.75 * Math.PI, 2.25 * Math.PI);
            ctx.lineWidth = 15;
            ctx.strokeStyle = "#1e293b";
            ctx.stroke();

            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, 0.75 * Math.PI, 1.25 * Math.PI);
            ctx.lineWidth = 15;
            ctx.strokeStyle = "#10b981";
            ctx.stroke();

            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, 1.25 * Math.PI, 1.75 * Math.PI);
            ctx.lineWidth = 15;
            ctx.strokeStyle = "#f59e0b";
            ctx.stroke();

            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, 1.75 * Math.PI, 2.25 * Math.PI);
            ctx.lineWidth = 15;
            ctx.strokeStyle = "#ef4444";
            ctx.stroke();
        }
    }

    Item {
        anchors.centerIn: parent
        width: 10
        height: parent.height - 60
        rotation: {
            var v = Math.abs(gauge.value);
            v = Math.min(v, 4.0);
            var ratio = v / 4.0;
            return -135 + (ratio * 270);
        }

        Behavior on rotation {
            SmoothedAnimation { velocity: 200 }
        }

        Rectangle {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: 4
            height: parent.height / 2
            color: "white"
            radius: 2
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: 12
        height: 12
        radius: 6
        color: "white"
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        text: gauge.value.toFixed(2) + " kV"
        color: "white"
        font.family: "monospace"
        font.pixelSize: 24
        font.bold: true
    }
}
