import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 900
    height: 640
    visible: true
    title: "CT Slice Reconstruction Viewer"
    color: "#0d1117"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Left: reconstructed image viewport
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 512
            color: "#161b22"
            border.color: "#30363d"
            border.width: 1
            radius: 6

            Image {
                id: ctImage
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height) - 16
                height: width
                source: "image://ct/slice"
                cache: false
                fillMode: Image.PreserveAspectFit
                smooth: true

                // Reload on each reconstruction
                Connections {
                    target: controller
                    function onReconstructionDone() { ctImage.source = ""; ctImage.source = "image://ct/slice" }
                }
            }

            Text {
                anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 8 }
                text: controller.isRunning ? "Reconstructing…" : "FBP Reconstruction"
                color: "#8b949e"
                font.pixelSize: 11
            }
        }

        // Right: controls panel
        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: 280
            spacing: 12

            Text {
                text: "CT Reconstruction Engine"
                color: "#e6edf3"
                font.pixelSize: 14
                font.weight: Font.Medium
            }

            FilterControls {
                Layout.fillWidth: true
            }

            Button {
                Layout.fillWidth: true
                text: controller.isRunning ? "Reconstructing…" : "Run FBP"
                enabled: !controller.isRunning
                onClicked: controller.startReconstruction()
                palette.buttonText: "#e6edf3"
                background: Rectangle {
                    color: parent.enabled ? "#238636" : "#21262d"
                    radius: 6
                    border.color: "#30363d"
                }
            }

            Item { Layout.fillHeight: true }

            Text {
                text: "f(x,y) = Σ Pθᵢ(x·cosθ + y·sinθ)"
                color: "#484f58"
                font.pixelSize: 10
                font.family: "monospace"
            }
        }
    }
}
