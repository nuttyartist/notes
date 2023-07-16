import QtQuick 2.12

Item {
    id: root
    property string icon
    property string platform: "Other"
    property int iconPointSize: root.platform === "Apple" ? 19 : 19 + root.iconPointSizeOffset
    property int iconPointSizeOffset: root.platform === "Apple" ? 0 : -4
    property var themeData: {{theme: "Light"}}
    property color themeColor: themeData.theme === "Dark" ? "#5b94f5" : "black"
    property bool enabled: true
    property string iconFontFamily: fontIconLoader.fa_solid
    width: 30
    height: 28

    signal clicked

    FontIconLoader {
        id: fontIconLoader
    }

    Rectangle {
        id: backgroundRect
        width: parent.width
        height: parent.height
        radius: 5
        color: "transparent"

        MouseArea {
            id: buttonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            property bool isExited: false

            onEntered: {
                if (root.enabled) {
                    backgroundRect.color = root.themeData.theme === "Dark" ? "#313131" : "#EFEFEF";
                    isExited = false;
    //                cursorShape = Qt.PointingHandCursor;
                }
            }

            onExited: {
                if (root.enabled) {
                    backgroundRect.color = "transparent";
                    isExited = true;
    //                cursorShape = Qt.ArrowCursor;
                }
            }

            onPressed: {
                if (root.enabled) {
                    backgroundRect.color = root.themeData.theme === "Dark" ? "#2c2c2c" : "#DFDFDE";
                }
            }

            onReleased: {
                if (root.enabled) {
                    if (isExited) {
                        backgroundRect.color = "transparent";
                    } else {
                        backgroundRect.color = root.themeData.theme === "Dark" ? "#313131" : "#EFEFEF";
                    }
                }
            }

            onClicked: {
                if (root.enabled) {
                    root.clicked();
                }
            }
        }

        Text {
            anchors.centerIn: parent
            id: iconContent
            text: root.icon
            font.family: root.iconFontFamily
            color: root.themeColor
            font.pointSize: root.iconPointSize
            opacity: root.enabled ? 1.0 : 0.2
        }
    }
}
