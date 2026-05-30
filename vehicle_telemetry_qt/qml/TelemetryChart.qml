import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtCharts 2.15

Item {
    id: root
    property int activeChannel: 0

    // Channel config: [label, minVal, maxVal]
    readonly property var channels: [
        ["engine_temp",   60,    120],
        ["battery_v",     11.5,  14.5],
        ["oil_pressure",  2.0,   5.0],
        ["rpm",           800,   4000]
    ]

    property var rawBuffers:      [[],[],[],[]]
    property var filteredBuffers: [[],[],[],[]]

    function pushSample(ch, raw, filtered) {
        var now = Date.now()
        rawBuffers[ch].push({x: now, y: raw})
        filteredBuffers[ch].push({x: now, y: filtered})
        // Keep 60 s rolling window (10 Hz × 60 s = 600 points)
        var cutoff = now - 60000
        while (rawBuffers[ch].length > 0 && rawBuffers[ch][0].x < cutoff)
            rawBuffers[ch].shift()
        while (filteredBuffers[ch].length > 0 && filteredBuffers[ch][0].x < cutoff)
            filteredBuffers[ch].shift()
        if (ch === activeChannel)
            refreshSeries()
    }

    function refreshSeries() {
        var ch = activeChannel
        rawSeries.clear()
        filteredSeries.clear()
        for (var i = 0; i < rawBuffers[ch].length; ++i)
            rawSeries.append(rawBuffers[ch][i].x, rawBuffers[ch][i].y)
        for (var j = 0; j < filteredBuffers[ch].length; ++j)
            filteredSeries.append(filteredBuffers[ch][j].x, filteredBuffers[ch][j].y)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        RowLayout {
            spacing: 8
            Layout.leftMargin: 8
            Text { text: "Channel:"; color: "#94a3b8"; font.pixelSize: 11 }
            ComboBox {
                model: ["engine_temp", "battery_v", "oil_pressure", "rpm"]
                font.pixelSize: 11
                background: Rectangle { color: "#1e1e2e"; radius: 4; border.color: "#3a3a5e" }
                contentItem: Text { text: parent.displayText; color: "#e2e8f0"; font: parent.font; verticalAlignment: Text.AlignVCenter; leftPadding: 6 }
                onCurrentIndexChanged: { root.activeChannel = currentIndex; root.refreshSeries() }
            }
            Text { text: "─── raw   ─── filtered"; color: "#94a3b8"; font.pixelSize: 10 }
        }

        ChartView {
            id: chart
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: ChartView.ChartThemeDark
            animationOptions: ChartView.NoAnimation
            backgroundColor: "#0d0d1a"
            legend.visible: true

            DateTimeAxis {
                id: xAxis
                format: "hh:mm:ss"
                tickCount: 6
                labelsColor: "#94a3b8"
                gridLineColor: "#1e1e3e"
            }

            ValueAxis {
                id: yAxis
                min: root.channels[root.activeChannel][1]
                max: root.channels[root.activeChannel][2]
                labelsColor: "#94a3b8"
                gridLineColor: "#1e1e3e"
            }

            LineSeries {
                id: rawSeries
                name: "Raw"
                axisX: xAxis
                axisY: yAxis
                color: "#f87171"
                width: 1
                opacity: 0.5
            }

            LineSeries {
                id: filteredSeries
                name: "Filtered"
                axisX: xAxis
                axisY: yAxis
                color: "#60a5fa"
                width: 2
            }
        }
    }
}
