import QtQuick
import QtCharts

ChartView {
    id: chartView
    antialiasing: true
    backgroundColor: "#1e293b"
    legend.visible: false
    
    property var points: []

    ValueAxis {
        id: xAxis
        min: 0
        max: 60
        titleText: "Time (s)"
        labelsColor: "#94a3b8"
        titleBrush: "#94a3b8"
    }

    ValueAxis {
        id: yAxis
        min: -10
        max: 10
        titleText: "Charge (kV)"
        labelsColor: "#94a3b8"
        titleBrush: "#94a3b8"
    }

    LineSeries {
        id: decaySeries
        name: "Q(t)"
        color: "#60a5fa"
        width: 2
        axisX: xAxis
        axisY: yAxis
    }

    LineSeries {
        id: threshUpper
        color: "#f87171"
        style: Qt.DashLine
        axisX: xAxis
        axisY: yAxis
    }

    LineSeries {
        id: threshLower
        color: "#f87171"
        style: Qt.DashLine
        axisX: xAxis
        axisY: yAxis
    }

    Component.onCompleted: {
        threshUpper.append(0, 2.0);
        threshUpper.append(10000, 2.0);
        threshLower.append(0, -2.0);
        threshLower.append(10000, -2.0);
    }

    onPointsChanged: {
        decaySeries.clear();
        if (points.length > 0) {
            var minX = points[0].x;
            var maxX = Math.max(60, points[points.length - 1].x);
            xAxis.min = Math.max(0, maxX - 60);
            xAxis.max = maxX;
            
            for (var i = 0; i < points.length; ++i) {
                decaySeries.append(points[i].x, points[i].y);
            }
        }
    }
}
