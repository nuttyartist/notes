import QtQuick

Rectangle {
    id: buttonContainer
    width: contentContainer.width*2
    height: contentContainer.height*2
    radius: 5
    color: "transparent"
    property var themeData
    property color themeColor: themeData.theme === "Dark" ? "#5b94f5" : "black"

    signal clicked()

    MouseArea {
        id: buttonMouseArea
        anchors.fill: parent
        hoverEnabled: true
        property bool isExited: false

        onEntered: {
            if (buttonContainer.themeData.theme === "Dark") {
                buttonContainer.color = "#262626";
            } else {
                buttonContainer.color = "#EFEFEF";
            }
            isExited = false;
        }

        onExited: {
            if (buttonContainer.themeData.theme === "Dark") {
                buttonContainer.color = "transparent";
            } else {
                buttonContainer.color = "transparent";
            }
            isExited = true;
        }

        onPressed: {
            if (buttonContainer.themeData.theme === "Dark") {
                buttonContainer.color = "#202020";
            } else {
                buttonContainer.color = "#DFDFDE";
            }
        }

        onReleased: {
            if (isExited) {
                buttonContainer.color = "transparent";
            } else {
                if (buttonContainer.themeData.theme === "Dark") {
                    buttonContainer.color = "#262626";
                } else {
                    buttonContainer.color = "#EFEFEF";
                }
            }
        }

        onClicked: {
            buttonContainer.clicked();
        }
    }

    Item {
        id: contentContainer
        width: 14
        height: 14
        anchors.centerIn: parent

        Rectangle {
            id: horizontalLine
            anchors.centerIn: parent
            width: 14
            height: 2
            color: buttonContainer.themeColor
        }

        Rectangle {
            id: verticalLine
            anchors.centerIn: parent
            width: 2
            height: 14
            color: buttonContainer.themeColor
        }
    }
}
