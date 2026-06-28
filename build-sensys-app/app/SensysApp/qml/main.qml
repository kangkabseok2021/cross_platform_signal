import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    visible: true
    width: 1280; height: 800
    title: "Magnetometer Survey Visualizer"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ── Control panel ──────────────────────────────────────────────────
        Rectangle {
            width: 260
            Layout.fillHeight: true
            color: "#1e1e2e"

            ColumnLayout {
                anchors { fill: parent; margins: 12 }
                spacing: 10

                Label {
                    text: "Magnetometer Visualizer"
                    font.bold: true; font.pixelSize: 14
                    color: "#cdd6f4"
                }

                Button {
                    text: surveyController.processing ? "Processing…" : "Run Survey"
                    enabled: !surveyController.processing
                    Layout.fillWidth: true
                    onClicked: surveyController.startSurvey("survey_" + Date.now())
                }

                ProgressBar {
                    Layout.fillWidth: true
                    value: surveyController.progress
                    visible: surveyController.processing
                }

                BusyIndicator { running: surveyController.processing }

                Label {
                    text: "Anomalies: " + surveyController.anomalyCount
                    color: "#f38ba8"
                }

                Label { text: "Depth plane"; color: "#a6adc8" }
                Slider {
                    Layout.fillWidth: true
                    from: 0; to: 0; stepSize: 1
                    value: surveyController.depthIndex
                    onMoved: surveyController.depthIndex = value
                }

                Label { text: "Threshold (nT)"; color: "#a6adc8" }
                Slider {
                    Layout.fillWidth: true
                    from: 10; to: 200; value: surveyController.thresholdNt
                    onMoved: surveyController.thresholdNt = value
                }

                CheckBox {
                    text: "Show anomaly markers"
                    checked: surveyController.showAnomalies
                    onToggled: surveyController.showAnomalies = checked
                    contentItem: Label { text: parent.text; color: "#cdd6f4" }
                }

                RowLayout {
                    ComboBox {
                        id: exportFormat
                        model: ["CSV", "GeoJSON"]
                        Layout.fillWidth: true
                    }
                    Button {
                        text: "Export"
                        onClicked: surveyController.exportResults(
                            exportFormat.currentText,
                            "/tmp/sensys_export." + (exportFormat.currentIndex === 0 ? "csv" : "geojson"))
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // ── 3D view placeholder (Qt3DWindow embedded via createWindowContainer) ──
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#181825"
            Label {
                anchors.centerIn: parent
                text: "Qt3D viewport\n(run via Qt3DWindow)"
                color: "#585b70"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
