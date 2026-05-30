import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property string label: "Channel"
    property string unit:  "?"
    property double minVal:        0.0
    property double maxVal:      100.0
    property double filteredValue: 0.0
    property double rawValue:      0.0
    property bool   fault:         false

    width: 220
    height: 230

    Rectangle {
        anchors.fill: parent
        radius: 12
        color: fault ? "#2d1a1a" : "#1a1a2e"
        border.color: fault ? "#ef4444" : "#2a2a4e"
        border.width: 1

        Behavior on color { ColorAnimation { duration: 300 } }
        Behavior on border.color { ColorAnimation { duration: 300 } }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 4

            Text {
                text: root.label
                font.pixelSize: 12
                color: "#a78bfa"
                Layout.alignment: Qt.AlignHCenter
                font.weight: Font.Medium
            }

            Canvas {
                id: gauge
                Layout.alignment: Qt.AlignHCenter
                width: 170; height: 170

                property double normalised: Math.max(0, Math.min(1,
                    (root.filteredValue - root.minVal) / (root.maxVal - root.minVal)))
                property double rawNorm: Math.max(0, Math.min(1,
                    (root.rawValue - root.minVal) / (root.maxVal - root.minVal)))

                onNormalisedChanged: requestPaint()
                onRawNormChanged:    requestPaint()

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    var cx = width / 2, cy = height / 2
                    var r  = width / 2 - 14
                    var startAngle = Math.PI * 0.75
                    var sweep      = Math.PI * 1.5

                    // Background arc
                    ctx.beginPath()
                    ctx.arc(cx, cy, r, startAngle, startAngle + sweep, false)
                    ctx.strokeStyle = "#2a2a3e"
                    ctx.lineWidth   = 14
                    ctx.lineCap     = "round"
                    ctx.stroke()

                    // Colour based on fill level
                    var n = normalised
                    var arcColor = n < 0.7 ? "#22c55e" : n < 0.9 ? "#f59e0b" : "#ef4444"
                    if (root.fault) arcColor = "#ef4444"

                    // Filtered value arc
                    ctx.beginPath()
                    ctx.arc(cx, cy, r, startAngle, startAngle + sweep * n, false)
                    ctx.strokeStyle = arcColor
                    ctx.lineWidth   = 14
                    ctx.lineCap     = "round"
                    ctx.stroke()

                    // Raw value tick (thin outer ring)
                    var rawAngle = startAngle + sweep * rawNorm
                    ctx.beginPath()
                    ctx.arc(cx, cy, r + 8, rawAngle - 0.04, rawAngle + 0.04, false)
                    ctx.strokeStyle = "#f87171"
                    ctx.lineWidth   = 4
                    ctx.stroke()

                    // Centre value text
                    ctx.fillStyle = root.fault ? "#ef4444" : "#e2e8f0"
                    ctx.font      = "bold 26px sans-serif"
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText(root.filteredValue.toFixed(1), cx, cy - 8)

                    ctx.fillStyle = "#6b7280"
                    ctx.font      = "11px sans-serif"
                    ctx.fillText(root.unit, cx, cy + 16)
                }
            }
        }
    }
}
