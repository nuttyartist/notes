import QtQuick 2.12

MouseArea {
    id: optionItemMouseArea
    property int contentWidth
    property string displayText
    property string displayFontFamily: "Roboto"
    property string platform: "Other"
    property string mainFontColor: themeData.theme === "Dark" ? "#d6d6d6" : "#37352e"
    property string highlightBackgroundColor: themeData.theme === "Dark" ? "#313131" : "#efefef"
    property string pressedBackgroundColor: themeData.theme === "Dark" ? "#2c2c2c" : "#dfdfde"
    property bool isContainingMouse: optionItemMouseArea.containsMouse
    property bool checked: false
    property var themeData: {{thme: "Light"}}
    property int pointSizeOffset: -4

    enabled: true
    hoverEnabled: true
    width: innerRectangle.width
    height: innerRectangle.height

    signal switched
    signal unswitched

    onEntered: {
         if (optionItemMouseArea.enabled) {
             innerRectangle.color = optionItemMouseArea.highlightBackgroundColor;
             optionItemMouseArea.cursorShape = Qt.PointingHandCursor;
         }
    }

    onExited: {
        if (optionItemMouseArea.enabled) {
            innerRectangle.color = "transparent";
            optionItemMouseArea.cursorShape = Qt.ArrowCursor;
        }
    }

    onPressed: {
        if (optionItemMouseArea.enabled) {
            innerRectangle.color = optionItemMouseArea.pressedBackgroundColor;
        }
    }

    onReleased: {
        if (optionItemMouseArea.enabled) {
            if (optionItemMouseArea.containsMouse) {
                innerRectangle.color = optionItemMouseArea.highlightBackgroundColor;
            } else {
                innerRectangle.color = "transparent";
            }
        }
    }

    onClicked: {
        if (optionItemMouseArea.enabled) {
            optionItemMouseArea.checked = !optionSwitch.checked;
            if (optionItemMouseArea.checked) {
                optionItemMouseArea.switched();
            } else {
                optionItemMouseArea.unswitched();
            }
        }
    }

    function setOptionSelected (isSelected) {
        optionItemMouseArea.checked = isSelected;
    }

    Rectangle {
        id: innerRectangle
        width: optionItemMouseArea.contentWidth
        height: 30
        radius: 5
        color: "transparent"

        Row {
            id: rowItems
            anchors.verticalCenter: parent.verticalCenter
            x: 0
            property int leftRightMargins: 10

            Item {
                height: 1
                width: rowItems.leftRightMargins
            }

            Text {
                anchors.verticalCenter: optionSwitch.verticalCenter
                id: optionText
                text: optionItemMouseArea.displayText
                color: optionItemMouseArea.mainFontColor
                font.pointSize: optionItemMouseArea.platform === "Apple" ? 14 : 14 + optionItemMouseArea.pointSizeOffset
                font.family: optionItemMouseArea.displayFontFamily
                opacity: optionItemMouseArea.enabled ? 1.0 : 0.2
            }

            Item {
                height: 1
                width: innerRectangle.width - optionText.width - optionSwitch.width - parent.x - rowItems.leftRightMargins
            }

            SwitchButton {
                id: optionSwitch
                checkable: true
                themeData: optionItemMouseArea.themeData
                checked: optionItemMouseArea.checked
                enabled: optionItemMouseArea.enabled

                onClicked: {
                    if (optionItemMouseArea.enabled) {
                        if (checked) {
                            optionItemMouseArea.switched();
                        } else {
                            optionItemMouseArea.unswitched();
                        }
                    }
                }
            }
        }
    }
}
