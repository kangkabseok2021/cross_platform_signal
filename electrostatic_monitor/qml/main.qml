import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Eltex 1.0

ApplicationWindow {
    id: window
    width: 1000
    height: 700
    visible: true
    title: "Eltex ESC Monitor"
    color: "#0f172a"

    MonitorContext {
        id: context
        onDecayPointsChanged: chart.points = decayPoints
    }
    
    ListModel {
        id: logModel
    }

    Connections {
        target: context
        function onDischargeEvent(v) {
            logModel.insert(0, {
                t: new Date().toISOString(), 
                v: v.toFixed(2)
            })
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "ELTEX SYSTEM MONITOR"
                color: "white"
                font.bold: true
                font.pixelSize: 20
                font.letterSpacing: 2
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                width: 15
                height: 15
                radius: 7.5
                color: context.networkError ? "#ef4444" : "#10b981"
            }
            Label {
                text: context.networkError ? "Net Err" : "Net OK"
                color: "white"
            }
            Item { width: 10 }
            Rectangle {
                width: 100
                height: 30
                radius: 15
                color: context.ionizing ? "#ef4444" : "#3b82f6"
                Label {
                    anchors.centerIn: parent
                    text: context.ionizing ? "IONIZING" : "IDLE"
                    color: "white"
                    font.bold: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20

            VoltageGauge {
                id: gauge
                value: context.voltage
                Layout.preferredWidth: 350
                Layout.fillHeight: true
            }

            DecayChart {
                id: chart
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            Button {
                text: "Reset Charge (+5kV)"
                onClicked: context.setInitialCharge(5.0)
            }
            Button {
                text: "Reset Charge (-5kV)"
                onClicked: context.setInitialCharge(-5.0)
            }
            Button {
                text: "Reset Ionizer"
                onClicked: context.resetIonizer()
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            model: logModel
            clip: true
            delegate: Rectangle {
                width: ListView.view.width
                height: 30
                color: index % 2 === 0 ? "#1e293b" : "#334155"
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    Label { text: model.t; color: "#94a3b8"; Layout.preferredWidth: 250 }
                    Label { text: "Discharge Event"; color: "white"; Layout.fillWidth: true }
                    Label { text: model.v + " kV"; color: "#f87171"; font.bold: true; Layout.preferredWidth: 100 }
                }
            }
        }
    }
}
