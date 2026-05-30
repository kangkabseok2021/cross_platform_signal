import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: root
    visible: true
    width: 1024; height: 600
    title: "Vehicle Telemetry"
    color: "#0d0d1a"

    // ─── Status bar ───────────────────────────────────────────────────────────
    Rectangle {
        id: statusBar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 36
        color: "#0d0d2e"

        RowLayout {
            anchors { fill: parent; leftMargin: 16; rightMargin: 16 }
            Text {
                text: "VEHICLE TELEMETRY"
                color: "#60a5fa"
                font { pixelSize: 13; weight: Font.Bold; letterSpacing: 2 }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                visible: Telemetry.engineFault
                width: 90; height: 22; radius: 4
                color: "#7f1d1d"
                SequentialAnimation on opacity {
                    running: Telemetry.engineFault
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: 400 }
                    NumberAnimation { to: 1.0; duration: 400 }
                }
                Text { anchors.centerIn: parent; text: "⚠ OVERHEAT"; color: "#ef4444"; font.pixelSize: 10; font.weight: Font.Bold }
            }
            Text {
                id: clock
                color: "#64748b"
                font.pixelSize: 11
                Timer {
                    interval: 1000; running: true; repeat: true
                    onTriggered: clock.text = Qt.formatDateTime(new Date(), "hh:mm:ss")
                }
                Component.onCompleted: text = Qt.formatDateTime(new Date(), "hh:mm:ss")
            }
        }
    }

    // ─── Main content ─────────────────────────────────────────────────────────
    ColumnLayout {
        anchors { top: statusBar.bottom; bottom: parent.bottom; left: parent.left; right: parent.right }
        anchors.margins: 12
        spacing: 10

        // Gauge row
        RowLayout {
            spacing: 10
            Layout.fillWidth: true

            GaugeCard {
                label: "Engine Temp"; unit: "°C"; minVal: 60; maxVal: 120
                filteredValue: Telemetry.engineTempFiltered
                rawValue: Telemetry.engineTempRaw
                fault: Telemetry.engineFault
            }
            GaugeCard {
                label: "Battery V"; unit: "V"; minVal: 11.5; maxVal: 14.5
                filteredValue: Telemetry.batteryVFiltered
                rawValue: Telemetry.batteryVRaw
                fault: Telemetry.batteryFault
            }
            GaugeCard {
                label: "Oil Pressure"; unit: "bar"; minVal: 2.0; maxVal: 5.0
                filteredValue: Telemetry.oilPressureFiltered
                rawValue: Telemetry.oilPressureRaw
                fault: false
            }
            GaugeCard {
                label: "RPM"; unit: "RPM"; minVal: 800; maxVal: 4000
                filteredValue: Telemetry.rpmFiltered
                rawValue: Telemetry.rpmRaw
                fault: false
            }

            // Alpha controls
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true
                Text { text: "EMA Alpha"; color: "#a78bfa"; font.pixelSize: 11 }
                Repeater {
                    model: ["Eng", "Bat", "Oil", "RPM"]
                    RowLayout {
                        spacing: 6
                        Text { text: modelData; color: "#94a3b8"; font.pixelSize: 10; width: 28 }
                        Slider {
                            from: 0.01; to: 1.0; stepSize: 0.01; value: 0.1
                            width: 100
                            onMoved: Telemetry.setAlpha(index, value)
                            background: Rectangle {
                                width: parent.availableWidth; height: 4; radius: 2
                                color: "#2a2a4e"
                                Rectangle { width: parent.width * parent.parent.position; height: 4; radius: 2; color: "#a78bfa" }
                            }
                            handle: Rectangle { x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width); y: parent.topPadding + parent.availableHeight / 2 - height / 2; width: 14; height: 14; radius: 7; color: "#a78bfa" }
                        }
                        Text { text: parent.children[1].value.toFixed(2); color: "#64748b"; font.pixelSize: 9; width: 32 }
                    }
                }
            }
        }

        // Chart
        TelemetryChart {
            id: theChart
            Layout.fillWidth: true
            Layout.fillHeight: true

            Connections {
                target: Telemetry
                function onDataChanged() {
                    theChart.pushSample(0, Telemetry.engineTempRaw,      Telemetry.engineTempFiltered)
                    theChart.pushSample(1, Telemetry.batteryVRaw,         Telemetry.batteryVFiltered)
                    theChart.pushSample(2, Telemetry.oilPressureRaw,      Telemetry.oilPressureFiltered)
                    theChart.pushSample(3, Telemetry.rpmRaw,              Telemetry.rpmFiltered)
                }
            }
        }
    }
}
