import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

MouseArea {
    id: themeMouseArea
    width: themeBackgroundRectangle.width
    height: themeBackgroundRectangle.height
    hoverEnabled: true
    property bool isExited: false
    property string displayFontFamily: "Roboto"
    property string platform: ""
    property string mainFontColor
    property string highlightBackgroundColor
    property string pressedBackgroundColor
    property string themeColor: "white"
    property string themeName: "Light"
    property var themeData: {{theme: "Light"}}
    property int pointSizeOffset: -4
    property int qtVersion: 6

    signal themeSelected (bool isSelected)
    signal unclicked
    signal themeChanged

    onEntered: {
        themeBackgroundRectangle.color = themeMouseArea.highlightBackgroundColor;
        isExited = false;
        cursorShape = Qt.PointingHandCursor;
    }

    onPressed: {
        themeBackgroundRectangle.color = themeMouseArea.pressedBackgroundColor;
    }

    onExited: {
        themeBackgroundRectangle.color = "transparent";
        isExited = true;
        cursorShape = Qt.ArrowCursor;
    }

    onReleased: {
        if (isExited) {
            themeBackgroundRectangle.color = "transparent";
        } else {
            themeBackgroundRectangle.color = themeMouseArea.highlightBackgroundColor;
        }
    }

    onClicked: {
//        themeRectangle.border.color = "#2383e2";
        themeText.color = "#2383e2";
    }

    onUnclicked: {
        themeBackgroundRectangle.color = "transparent";
//        themeRectangle.border.color = "gray";
        themeText.color = themeMouseArea.mainFontColor;
    }

    onThemeSelected: (isSelected) => {
//        themeRectangle.border.color = "#2383e2";
        if (isSelected)
            themeText.color = "#2383e2";
        else
            themeMouseArea.unclicked();
    }

    onThemeChanged: {
        if (themeMouseArea.containsPress) {
            themeBackgroundRectangle.color = themeMouseArea.pressedBackgroundColor;
        } else if (themeMouseArea.containsMouse) {
            themeBackgroundRectangle.color = themeMouseArea.highlightBackgroundColor;
        }
    }

    Rectangle {
        id: themeBackgroundRectangle
        width: 71
        height: 71
        color: "transparent"
        radius: 3

        Column {
            anchors.centerIn: parent

            Rectangle {
//                visible: themeMouseArea.qtVersion < 6
                width: 30
                height: 30
                radius: width/2
                border.width: themeMouseArea.themeData.theme === "Dark" ? 0 : 1
                border.color: "gray"
                color: themeMouseArea.themeColor
            }

            // Should look better when Qt 6.5.1+ won't be buggy
//            Pane {
//                visible: themeMouseArea.qtVersion >= 6
//                width: 30
//                height: 30
//                padding: 0
//                Material.elevation: 2
//                Material.theme: themeMouseArea.themeData.theme === "Dark" ? Material.Dark : Material.Light
//                Material.background: themeMouseArea.themeColor
////                Material.roundedScale: Material.FullScale
//            }

            Item {
                width: 1
                height: 7
            }

            Text {
                id: themeText
                anchors.horizontalCenter: parent.horizontalCenter
                text: themeMouseArea.themeName
                color: themeMouseArea.mainFontColor
                font.pointSize: themeMouseArea.platform === "Apple" ? 13 : 13 + themeMouseArea.pointSizeOffset
                font.family: themeMouseArea.displayFontFamily
            }
        }
    }
}
