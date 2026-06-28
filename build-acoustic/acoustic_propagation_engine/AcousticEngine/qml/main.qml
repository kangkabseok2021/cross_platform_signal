import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width: 700; height: 620
    title: "Acoustic SPL Field — " + acousticModel.modelName

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        /* ── Heatmap + source marker ─────── */
        Item {
            Layout.preferredWidth: 500
            Layout.fillHeight: true

            HeatmapView {
                id: heatmapView
                anchors.fill: parent
                splGrid: acousticModel.splGrid
                gridN: 50
            }

            SourceMarker {
                id: srcMarker
                cellSize: heatmapView.width / 50
                x: acousticModel.sourceX * cellSize - width  / 2
                y: acousticModel.sourceY * cellSize - height / 2
                onPositionChanged: (nx, ny) => acousticModel.setSource(nx, ny)
            }
        }

        /* ── Controls ────────────────────── */
        ColumnLayout {
            Layout.fillHeight: true
            spacing: 12

            Label { text: "Model"; font.bold: true }
            ComboBox {
                model: ["InverseSquareLaw", "SabineReverb"]
                onActivated: acousticModel.setModel(currentText)
            }

            Label { text: "Source power (Lw dB)"; font.bold: true }
            Slider {
                from: 60; to: 100; value: 80; stepSize: 1
                onValueChanged: acousticModel.setSourceLw(value)
            }

            Item { Layout.fillHeight: true }

            Label {
                text: "Drag the yellow marker\nto move the source"
                color: "#888"; wrapMode: Text.WordWrap
            }
        }
    }
}
