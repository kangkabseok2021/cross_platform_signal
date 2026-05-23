import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#161b22"
    border.color: "#30363d"
    border.width: 1
    radius: 6
    implicitHeight: col.implicitHeight + 24

    ColumnLayout {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
        spacing: 8

        Text {
            text: "FILTER"
            color: "#8b949e"
            font.pixelSize: 10
            font.letterSpacing: 1
        }

        ButtonGroup { id: filterGroup }

        Repeater {
            model: ["RamLak", "SheppLogan"]
            RadioButton {
                text: modelData === "RamLak" ? "Ram-Lak (high-res)" : "Shepp-Logan (smooth)"
                checked: controller.filterType === modelData
                ButtonGroup.group: filterGroup
                onClicked: controller.filterType = modelData

                contentItem: Text {
                    leftPadding: parent.indicator.width + 6
                    text: parent.text
                    color: parent.checked ? "#58a6ff" : "#8b949e"
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                }
                indicator: Rectangle {
                    x: parent.leftPadding
                    anchors.verticalCenter: parent.verticalCenter
                    width: 14; height: 14; radius: 7
                    color: "transparent"
                    border.color: parent.checked ? "#58a6ff" : "#30363d"
                    Rectangle {
                        anchors.centerIn: parent
                        width: 7; height: 7; radius: 3.5
                        color: "#58a6ff"
                        visible: parent.parent.checked
                    }
                }
            }
        }

        Text {
            text: "CUTOFF FREQUENCY"
            color: "#8b949e"
            font.pixelSize: 10
            font.letterSpacing: 1
            topPadding: 4
        }

        RowLayout {
            Layout.fillWidth: true
            Slider {
                id: cutoffSlider
                Layout.fillWidth: true
                from: 0.1; to: 1.0; stepSize: 0.05
                value: controller.cutoffFreq
                onMoved: controller.cutoffFreq = value
                background: Rectangle {
                    x: cutoffSlider.leftPadding; y: cutoffSlider.topPadding + cutoffSlider.availableHeight / 2 - 2
                    width: cutoffSlider.availableWidth; height: 4; radius: 2
                    color: "#21262d"
                    Rectangle { width: cutoffSlider.visualPosition * parent.width; height: parent.height; radius: 2; color: "#58a6ff" }
                }
                handle: Rectangle {
                    x: cutoffSlider.leftPadding + cutoffSlider.visualPosition * cutoffSlider.availableWidth - width / 2
                    y: cutoffSlider.topPadding + cutoffSlider.availableHeight / 2 - height / 2
                    width: 14; height: 14; radius: 7
                    color: "#58a6ff"; border.color: "#0d1117"
                }
            }
            Text {
                text: cutoffSlider.value.toFixed(2)
                color: "#e6edf3"; font.pixelSize: 11
                Layout.preferredWidth: 32
            }
        }
    }
}
