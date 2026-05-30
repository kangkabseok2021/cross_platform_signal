import QtQuick

Canvas {
    id: canvas

    Timer {
        interval: 16; running: true; repeat: true
        onTriggered: canvas.requestPaint()
    }

    onPaint: {
        const ctx = getContext("2d")
        ctx.fillStyle = "#0d0d1a"
        ctx.fillRect(0, 0, width, height)

        const buf = Engine.waveformBuffer
        if (!buf || buf.length === 0) return

        const len  = buf.length
        const midY = height / 2
        const scaleY = height * 0.38

        ctx.beginPath()
        ctx.strokeStyle = "#7c3aed"
        ctx.lineWidth   = 1.5
        ctx.lineJoin    = "round"

        for (let i = 0; i < len; i++) {
            const x = (i / (len - 1)) * width
            const y = midY - buf[i] * scaleY
            if (i === 0) ctx.moveTo(x, y)
            else         ctx.lineTo(x, y)
        }
        ctx.stroke()
    }
}
