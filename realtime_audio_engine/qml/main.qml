import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width:  820
    height: 560
    title:  "Real-Time IIR Low-Pass Filter"
    color:  "#1a1a2e"

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: 16
        spacing:         12

        Text {
            text:  "Real-Time IIR Low-Pass Filter"
            color: "#e0e0f0"
            font { pixelSize: 18; bold: true }
        }

        RowLayout {
            spacing: 24
            Layout.fillWidth: true

            // ── Rotary Knob ───────────────────────────────────────────────
            ColumnLayout {
                spacing: 4
                Text {
                    text:  "Cutoff"
                    color: "#9090b0"
                    font.pixelSize: 11
                    Layout.alignment: Qt.AlignHCenter
                }
                RotaryKnob {
                    id:    cutoffKnob
                    width: 96; height: 96
                    min:   20;  max:  20000
                    value: Engine.cutoffHz
                    Binding { target: Engine; property: "cutoffHz"; value: cutoffKnob.value }
                }
                Text {
                    text:  cutoffKnob.value.toFixed(0) + " Hz"
                    color: "#7c3aed"
                    font { pixelSize: 14; bold: true }
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // ── Waveform View ─────────────────────────────────────────────
            WaveformView {
                Layout.fillWidth: true
                height: 110
            }
        }

        // ── Status Bar ────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height:  28
            radius:  4
            color:   Engine.overruns > 10 ? "#3d0000"
                   : Engine.overruns >  0 ? "#2d2000" : "#0d1f0d"
            border.color: Engine.overruns > 10 ? "#ef4444"
                        : Engine.overruns >  0 ? "#f59e0b" : "#22c55e"
            Text {
                anchors.centerIn: parent
                text:  Engine.overruns > 0
                       ? ("⚠ Overruns: " + Engine.overruns)
                       : "● Audio OK"
                color: Engine.overruns > 10 ? "#ef4444"
                     : Engine.overruns >  0 ? "#f59e0b" : "#22c55e"
                font.pixelSize: 12
                Behavior on color { ColorAnimation { duration: 300 } }
            }
        }

        // ── Preset controls ───────────────────────────────────────────────
        RowLayout {
            spacing: 8
            Layout.fillWidth: true

            TextField {
                id: nameField
                placeholderText: "Preset name…"
                color: "#e0e0f0"
                background: Rectangle {
                    color:  "#16213e"
                    border.color: "#7c3aed"
                    radius: 4
                }
                Layout.fillWidth: true
            }
            Button {
                text: "Save"
                onClicked: {
                    if (nameField.text.length > 0) {
                        Engine.savePreset(nameField.text)
                        nameField.clear()
                    }
                }
            }
        }

        // ── Preset List ───────────────────────────────────────────────────
        ListView {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            model: Engine.presetModel
            clip:  true

            delegate: Rectangle {
                width:  ListView.view.width
                height: 32
                color:  hoverArea.containsMouse ? "#1e1e3e" : "transparent"

                RowLayout {
                    anchors { fill: parent; margins: 6 }
                    Text {
                        text:  model.name
                        color: "#c0c0e0"
                        Layout.fillWidth: true
                    }
                    Text {
                        text:  model.cutoff.toFixed(0) + " Hz"
                        color: "#7c3aed"
                    }
                }

                MouseArea {
                    id: hoverArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onDoubleClicked: Engine.presetModel.applyPreset(index)
                }
            }
        }
    }
}
