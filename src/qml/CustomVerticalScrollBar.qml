import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

ScrollBar {
    id: scrollBarControl
//    parent: scrollViewControl
    x: parent.mirrored ? 0 : parent.width - width
//    y: parent.topPadding
    height: parent.availableHeight
    property var themeData: {{theme: "Light"}}
    property bool isDarkGray: true // Deterimnes wether the scrollbar color in Dark mode should be grayish or darkish
    property bool showBackground: false

    background: Rectangle {
        width: scrollBarControl.hovered ? 1 : 7
        radius: width / 2
        height: scrollBarControl.availableHeight
        color: "transparent"
        border.color: scrollBarControl.themeData.theme === "Dark" ? "#534c53" : "#dfdfdf"
        opacity: 0.25
        visible: scrollBarControl.active && scrollBarControl.hovered && scrollBarControl.showBackground
    }

    contentItem: Rectangle {
        implicitWidth: scrollBarControl.hovered ? 12 : 7
        radius: width / 2
        color: scrollBarControl.pressed ? (scrollBarControl.themeData.theme === "Dark" ? (scrollBarControl.isDarkGray ? "#858185" : "#040404") :"#5e5d5d") : (scrollBarControl.themeData.theme === "Dark" ? (scrollBarControl.isDarkGray ? "#a29ea2" : "#0e0e0e") : "#7d7d7d")
        opacity: scrollBarControl.policy === ScrollBar.AlwaysOn || (scrollBarControl.active && scrollBarControl.size < 1.0) ? 0.75 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: 250
            }
        }
    }
}

