import QtQuick 2.12

Item {
    id: root
    width: root.backgroundSizeFitText ? buttonText.implicitWidth : root.backgroundWidth
    height: root.backgroundSizeFitText ? buttonText.implicitHeight : root.backgroundHeight
    property string text
    signal clicked
    property var themeData: {{theme: "Light"}}
    property real backgroundOpacity: 1.0
    property string defaultBackgroundColor: "transparent"
    property string highlightBackgroundColor: root.themeData.theme === "Dark" ? "#313131" : "#efefef"
    property string pressedBackgroundColor: root.themeData.theme === "Dark" ? "#2c2c2c" : "#dfdfde"
    property string mainFontColorDefault: root.themeData.theme === "Dark" ? "#d6d6d6" : "#37352e"
    property string mainFontColorPressed: mainFontColorDefault
    property string iconColorDefault: root.mainFontColorDefault
    property int textFontWeight: Font.Normal
    property string platform: ""
    property string displayFontFamily
    property int backgroundWidthOffset: 50
    property int backgroundHeightOffset: 15
    property int backgroundHeight: buttonText.implicitHeight + backgroundHeightOffset
    property int backgroundWidth: buttonText.implicitWidth + backgroundWidthOffset
    property string icon: ""
    property int pointSizeOffset: root.platform === "Apple" ? 0 : -4
    property int textFontPointSize: 14
    property int iconFontPointSize: 15
    property int textAlignment: TextButton.TextAlign.Left
    property int textLeftRightMargin: 32
    property int iconLeftRightMargin: 8
    property string iconFontFamily: fontIconLoader.fa_solid
    property bool usePointingHand: true
    property bool pressed: false
    property bool entered: false
    property bool backgroundSizeFitText: false

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
        anchors.fill: parent

        onEntered: {
            root.entered = true;
            innerButtonRect.color = root.highlightBackgroundColor;
            if (root.usePointingHand) {
                buttonMouseArea.cursorShape = Qt.PointingHandCursor;
            }
        }

        onExited: {
            root.entered = false;
            innerButtonRect.color = Qt.binding(function () { return root.defaultBackgroundColor });
            buttonMouseArea.cursorShape = Qt.ArrowCursor;
        }

        onPressed: {
            root.pressed = true;
            innerButtonRect.color = root.pressedBackgroundColor;
            buttonText.color = root.mainFontColorPressed;
        }

        onReleased: {
            root.pressed = false;
            buttonText.color = Qt.binding(function () { return root.mainFontColorDefault });
            if (buttonMouseArea.containsMouse) {
                innerButtonRect.color = root.highlightBackgroundColor;
            } else {
                innerButtonRect.color = Qt.binding(function () { return root.defaultBackgroundColor });
            }
        }

        onClicked: {
            root.clicked();
        }

        Rectangle {
            id: innerButtonRect
            anchors.fill: parent
            radius: 5
            color: root.defaultBackgroundColor
            opacity: root.backgroundOpacity
        }

        Text {
            visible: root.icon !== ""
            text: root.icon
            font.family: root.iconFontFamily
            color: root.iconColorDefault
            font.pointSize: root.iconFontPointSize + root.pointSizeOffset
            anchors.verticalCenter: buttonText.verticalCenter
            anchors.right: root.textAlignment === TextButton.TextAlign.Left || root.textAlignment === TextButton.TextAlign.Middle ? buttonText.left : undefined
            anchors.rightMargin: root.iconLeftRightMargin
            anchors.left: root.textAlignment === TextButton.TextAlign.Right ? buttonText.right : undefined
            anchors.leftMargin: root.iconLeftRightMargin
        }

        Text {
            id: buttonText
            text: root.text
            color: root.mainFontColorDefault
            font.pointSize: root.textFontPointSize + root.pointSizeOffset
            font.family: root.displayFontFamily
            font.weight: root.textFontWeight
            anchors.verticalCenter: innerButtonRect.verticalCenter
            anchors.left: root.textAlignment === TextButton.TextAlign.Left ? innerButtonRect.left : undefined
            anchors.leftMargin: root.textLeftRightMargin
            anchors.right: root.textAlignment === TextButton.TextAlign.Right ? innerButtonRect.right : undefined
            anchors.rightMargin: root.textLeftRightMargin
            anchors.horizontalCenter: root.textAlignment === TextButton.TextAlign.Middle ? innerButtonRect.horizontalCenter : undefined
        }
    }
}
