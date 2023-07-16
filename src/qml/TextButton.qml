import QtQuick 2.12

Item {
    id: root
    width: buttonMouseArea.width
    height: buttonMouseArea.height
    property string text
    signal clicked
    property var themeData: {{theme: "Light"}}
    property string highlightBackgroundColor: root.themeData.theme === "Dark" ? "#313131" : "#efefef"
    property string pressedBackgroundColor: root.themeData.theme === "Dark" ? "#2c2c2c" : "#dfdfde"
    property string mainFontColor: root.themeData.theme === "Dark" ? "#d6d6d6" : "#37352e"
    property string platform: ""
    property string displayFontFamily
    property int backgroundWidth
    property string icon: ""
    property string iconColor: root.mainFontColor
    property int pointSizeOffset: -4
    property int textAlignment: TextButton.TextAlign.Left
    property int textLeftRightMargin: 32
    property int iconLeftRightMargin: 8
    property string iconFontFamily: fontIconLoader.fa_solid

    enum TextAlign {
        Left,
        Right,
        Middle
    }

    FontIconLoader {
        id: fontIconLoader
    }

    MouseArea {
        id: buttonMouseArea
        hoverEnabled: true
        width: innerButtonRect.width
        height: innerButtonRect.height

        onEntered: {
            innerButtonRect.color = root.highlightBackgroundColor;
            buttonMouseArea.cursorShape = Qt.PointingHandCursor;
        }

        onExited: {
            innerButtonRect.color = "transparent";
            buttonMouseArea.cursorShape = Qt.ArrowCursor;
        }

        onPressed: {
            innerButtonRect.color = root.pressedBackgroundColor;
        }

        onReleased: {
            if (buttonMouseArea.containsMouse) {
                innerButtonRect.color = root.highlightBackgroundColor;
            } else {
                innerButtonRect.color = "transparent";
            }
        }

        onClicked: {
            root.clicked();
        }

        Rectangle {
            id: innerButtonRect
            width: root.backgroundWidth
            height: 30
            radius: 5
            color: "transparent"

            Text {
                visible: root.icon !== ""
                text: root.icon
                font.family: root.iconFontFamily
                color: root.iconColor
                font.pointSize: root.platform === "Apple" ? 15 : 15 + root.pointSizeOffset
                anchors.verticalCenter: buttonText.verticalCenter
                anchors.right: root.textAlignment === TextButton.TextAlign.Left || root.textAlignment === TextButton.TextAlign.Middle ? buttonText.left : undefined
                anchors.rightMargin: root.iconLeftRightMargin
                anchors.left: root.textAlignment === TextButton.TextAlign.Right ? buttonText.right : undefined
                anchors.leftMargin: root.iconLeftRightMargin
            }

            Text {
                id: buttonText
                text: root.text
                color: root.mainFontColor
                font.pointSize: root.platform === "Apple" ? 14 : 14 + root.pointSizeOffset
                font.family: root.displayFontFamily
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: root.textAlignment === TextButton.TextAlign.Left ? parent.left : undefined
                anchors.leftMargin: root.textLeftRightMargin
                anchors.right: root.textAlignment === TextButton.TextAlign.Right ? parent.right : undefined
                anchors.rightMargin: root.textLeftRightMargin
                anchors.horizontalCenter: root.textAlignment === TextButton.TextAlign.Middle ? parent.horizontalCenter : undefined
            }
        }
    }
}
