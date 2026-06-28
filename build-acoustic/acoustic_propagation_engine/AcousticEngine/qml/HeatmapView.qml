import QtQuick 2.15

Canvas {
    id: heatmap
    required property var splGrid
    required property int gridN

    onSplGridChanged: requestPaint()

    onPaint: {
        const ctx = getContext("2d")
        if (!splGrid || splGrid.length === 0) return

        const cw   = width  / gridN
        const ch   = height / gridN
        const minL = 30, maxL = 80   // dB display range

        for (let iy = 0; iy < gridN; ++iy) {
            for (let ix = 0; ix < gridN; ++ix) {
                const Lp = splGrid[iy * gridN + ix]
                const t  = Math.max(0, Math.min(1, (Lp - minL) / (maxL - minL)))
                // Blue (quiet) → green → red (loud)
                const r = Math.round(t > 0.5 ? 255 : t * 2 * 255)
                const g = Math.round(t < 0.5 ? t * 2 * 200 : (1 - t) * 2 * 200)
                const b = Math.round(t < 0.5 ? 255 - t * 2 * 255 : 0)
                ctx.fillStyle = `rgb(${r},${g},${b})`
                ctx.fillRect(ix * cw, iy * ch, cw + 1, ch + 1)
            }
        }
    }
}
