import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Universal 2.12

TextArea {
    id: root
    textFormat: TextEdit.PlainText
    wrapMode: Text.Wrap
    background: Rectangle {
        radius: 5
        color: root.themeData.theme === "Dark" ? "#313131" : "#efefef"
    }

    property var themeData
    property string accentColor
    property bool cursorAnimationRunning: true
    signal anyKeyPressed
    signal cursorHidden
    signal cursorShowed
    signal returnPressed
    property bool isHoldingShift: false

    Keys.onPressed: (event) => {
        if (event.key === Qt.Key_Shift) {
          isHoldingShift = true;
        }

        root.anyKeyPressed();
        root.cursorAnimationRunning = false;
    }

    Keys.onReleased: (event) => {
         if (event.key === Qt.Key_Shift) {
           isHoldingShift = false;
         }
        root.cursorAnimationRunning = true;
    }

    onPressed: {
        root.cursorShowed();
    }

    Keys.onReturnPressed: {
        returnPressed();
    }

    cursorDelegate: Rectangle {
        id: cursorDelegateObject
        visible: true
        color: root.accentColor
        width: 2

        Connections {
            target: root

            function onAnyKeyPressed () {
                cursorDelegateObject.visible = true;
            }

            function onCursorHidden () {
                cursorDelegateObject.visible = false;
            }

            function onCursorShowed () {
                cursorDelegateObject.visible = true;
            }
        }

        SequentialAnimation {
            loops: Animation.Infinite
            running: root.cursorAnimationRunning

            PropertyAction {
                target: cursorDelegateObject
                property: 'visible'
                value: true
            }

            PauseAnimation {
                duration: 500
            }

            PropertyAction {
                target: cursorDelegateObject
                property: 'visible'
                value: false
            }

            PauseAnimation {
                duration: 500
            }
        }
    }
}
