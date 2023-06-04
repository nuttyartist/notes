import QtQuick

Rectangle {
    id: dotsButtonContainer
    width: dotsContainer.width*1.5
    height: dotsContainer.height*6
    radius: 5
    color: "transparent"
    property var themeData
    signal clicked()

    MouseArea {
        id: dotsButtonMouseArea
        anchors.fill: parent
        hoverEnabled: true
        property bool isExited: false

        onEntered: {
            if (dotsButtonContainer.themeData.theme === "Dark") {
                dotsButtonContainer.color = "#262626";
            } else {
                dotsButtonContainer.color = "#EFEFEF";
            }
            isExited = false;
        }

        onExited: {
            if (dotsButtonContainer.themeData.theme === "Dark") {
                dotsButtonContainer.color = "transparent";
            } else {
                dotsButtonContainer.color = "transparent";
            }
            isExited = true;
        }

        onPressed: {
            if (dotsButtonContainer.themeData.theme === "Dark") {
                dotsButtonContainer.color = "#202020";
            } else {
                dotsButtonContainer.color = "#DFDFDE";
            }
        }

        onReleased: {
            if (isExited) {
                dotsButtonContainer.color = "transparent";
            } else {
                if (dotsButtonContainer.themeData.theme === "Dark") {
                    dotsButtonContainer.color = "#262626";
                } else {
                    dotsButtonContainer.color = "#EFEFEF";
                }
            }
        }

        onClicked: {
            dotsButtonContainer.clicked();
        }
    }

    Item {
        id: dotsContainer
        width: (dotsRepeater.count-1)*offsetFromEachOther + circleSize
        height: circleSize
        anchors.centerIn: parent
        property double circleSize: 4.5
        property double offsetFromEachOther: circleSize*1.7

        Repeater {
            id: dotsRepeater
            model: 3
            delegate: Rectangle {
                id: dotDelegate
                required property int index

                color: dotsButtonContainer.themeData.theme === "Dark" ? "#5b94f5" : "#55534E"
                width: dotsContainer.circleSize
                height: dotsContainer.circleSize
                x: dotDelegate.index * dotsContainer.offsetFromEachOther
                radius: width * 0.5
            }
        }
    }
}
