import QtQuick

Item {
    id: root

    property real min:         20
    property real max:         20000
    property real value:       1000
    property real sensitivity: 0.003   // Hz per pixel of vertical drag

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            const cx = width  / 2
            const cy = height / 2
            const r  = Math.min(width, height) / 2 - 6

            const startAngle = Math.PI * 0.75
            const endAngle   = Math.PI * 2.25
            const norm       = (root.value - root.min) / (root.max - root.min)
            const valAngle   = startAngle + norm * (endAngle - startAngle)

            // Track arc
            ctx.beginPath()
            ctx.arc(cx, cy, r, startAngle, endAngle)
            ctx.strokeStyle = "#2a2a4a"
            ctx.lineWidth   = 7
            ctx.lineCap     = "round"
            ctx.stroke()

            // Filled arc
            ctx.beginPath()
            ctx.arc(cx, cy, r, startAngle, valAngle)
            ctx.strokeStyle = "#7c3aed"
            ctx.lineWidth   = 7
            ctx.lineCap     = "round"
            ctx.stroke()

            // Centre dot
            ctx.beginPath()
            ctx.arc(cx, cy, 5, 0, Math.PI * 2)
            ctx.fillStyle = "#7c3aed"
            ctx.fill()
        }
    }

    onValueChanged: canvas.requestPaint()

    MouseArea {
        anchors.fill: parent
        property real startY:   0
        property real startVal: 0

        onPressed:  { startY = mouseY; startVal = root.value }
        onPositionChanged: {
            if (!pressed) return
            const range = root.max - root.min
            const delta = (startY - mouseY) * root.sensitivity * range
            root.value  = Math.max(root.min, Math.min(root.max, startVal + delta))
        }
    }
}
