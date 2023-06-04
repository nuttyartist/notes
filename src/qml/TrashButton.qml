import QtQuick

Rectangle {
    id: buttonContainer
    width: contentContainer.width*2
    height: contentContainer.height*2
    radius: 5
    color: "transparent"
    property var themeData
    signal clicked()
    property string themeColor: themeData.theme === "Dark" ? "#5b94f5" : "black"

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
        width: 12
        height: 16
        anchors.centerIn: parent

        Rectangle {
            id: rec1
            anchors.centerIn: parent
            width: 12
            height: 16
            border.width: 2
            border.color: buttonContainer.themeColor
            color: "transparent"

            Rectangle {
                id: rec2
                width: rec1.width + 2
                height: rec1.border.width
                x: rec1.x + rec1.width/2  - width/2
                color: buttonContainer.themeColor
            }

            Rectangle {
                id: rec3
                width: rec1.width / 8
                height: rec1.height - rec1.height/3
                x: rec1.x + width * 2.5
                y: rec1.y + rec1.height/2 - height/2
                color: buttonContainer.themeColor
            }

            Rectangle {
                id: rec4
                width: rec1.width / 8
                height: rec1.height - rec1.height/3
                x: rec1.x + width * 4.5
                y: rec1.y + rec1.height/2 - height/2
                color: buttonContainer.themeColor
            }

            Rectangle {
                id: rec5
                width: 2
                height: 3
                x: rec1.x + 2
                y: rec1.y - height
                color: buttonContainer.themeColor
            }

            Rectangle {
                id: rec6
                width: 6
                height: 2
                x: rec5.x
                y: rec5.y
                color: buttonContainer.themeColor
            }

            Rectangle {
                id: rec7
                width: 2
                height: 3
                x: rec6.x + rec6.width
                y: rec5.y
                color: buttonContainer.themeColor
            }
        }
    }
}
