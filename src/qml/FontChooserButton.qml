import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

Column {
    id: rootContainer
    property string displayFontFamily: "Roboto"
    property string platform: ""
    property string mainFontColor
    property string highlightBackgroundColor
    property string pressedBackgroundColor
    property string categoriesFontColor
    property var themeData: {{theme: "Light"}}
    property string fontTypeface: "Sans"
    property bool checked: false
    property var fontsModel
    property int currentlyChosenFontIndex: 0
    property int  pointSizeOffset: -4
    property int qtVersion: 6
    property string mainBackgroundColor
    property bool enabled: true

    signal themeChanged
    signal clicked(int chosenFontIndex)

    onClicked: {
        rootContainer.checked = true;
    }

    MouseArea {
        id: fontMouseArea
        width: fontRectangle.width
        height: fontRectangle.height
        hoverEnabled: true
        property bool isExited: false

        onEntered: {
            if(rootContainer.enabled) {
                fontRectangle.color = rootContainer.highlightBackgroundColor;
                isExited = false;
                cursorShape = Qt.PointingHandCursor;
            }
        }

        onPressed: {
            if(rootContainer.enabled) {
                fontRectangle.color = rootContainer.pressedBackgroundColor;
            }
        }

        onExited: {
            if(rootContainer.enabled) {
                fontRectangle.color = "transparent";
                isExited = true;
                cursorShape = Qt.ArrowCursor;
            }
        }

        onReleased: {
            if(rootContainer.enabled) {
                if (isExited) {
                    fontRectangle.color = "transparent";
                } else {
                    fontRectangle.color = rootContainer.highlightBackgroundColor;
                }
            }
        }

        onClicked: {
            if(rootContainer.enabled) {
                rootContainer.clicked(-1);
            }
        }

        Rectangle {
            id: fontRectangle
            width: 71
            height: 61
            color: "transparent"
            radius: 3

            Column {
                anchors.centerIn: parent

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    id: fontMainText
                    text: "Ag"
                    font.pointSize: rootContainer.platform === "Apple" ? 24 : 24 + rootContainer.pointSizeOffset
                    font.weight: Font.Medium
                    font.family: comboBoxControl.currentText
                    color: rootContainer.checked ? "#2383e2" : rootContainer.mainFontColor
                    opacity: rootContainer.enabled ? 1.0 : 0.2
                }

                Item {
                    width: 1
                    height: 3
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: rootContainer.fontTypeface
                    color: rootContainer.categoriesFontColor
                    font.pointSize: rootContainer.platform === "Apple" ? 12 : 12 + rootContainer.pointSizeOffset
                    font.family: rootContainer.displayFontFamily
                    opacity: rootContainer.enabled ? 1.0 : 0.2
                }
            }
        }
    }

    ComboBox {
        id: comboBoxControl
        anchors.horizontalCenter: parent.horizontalCenter
        model: rootContainer.fontsModel
        width: fontRectangle.width - 10
        height: 30
        font.pointSize: rootContainer.platform === "Apple" ? 10 : 10 + rootContainer.pointSizeOffset
        Material.accent: "#2383e2"
        currentIndex: rootContainer.currentlyChosenFontIndex
        opacity: rootContainer.enabled ? 1.0 : 0.2
        enabled: rootContainer.enabled

        onActivated: {
            rootContainer.clicked(comboBoxControl.currentIndex);
        }

        background: Rectangle {
            implicitWidth: 120
            implicitHeight: 40
            border.color: "#7d7c78"
            border.width: comboBoxControl.visualFocus ? 2 : 1
            radius: 3
            color: "transparent"
        }

        contentItem: Text {
            leftPadding: 5
            rightPadding: comboBoxControl.indicator.width + 5
            text: comboBoxControl.displayText
            width: comboBoxControl.width - comboBoxControl.indicator.width - comboBoxControl.spacing
            font.pointSize: rootContainer.platform === "Apple" ? 11 : 11 + rootContainer.pointSizeOffset
            color: rootContainer.mainFontColor
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        delegate: ItemDelegate {
            id: comboItemDelegate
            width: comboBoxControl.width*2.5
            height: comboBoxControl.height
            contentItem: Text {
                text: comboBoxControl.textRole
                    ? (Array.isArray(comboBoxControl.model) ? modelData[comboBoxControl.textRole] : model[comboBoxControl.textRole])
                    : modelData
                color: comboBoxControl.currentIndex === index ? "#2383e2" : rootContainer.mainFontColor
                font.pointSize: comboBoxControl.font.pointSize+1
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                font.family: rootContainer.displayFontFamily
            }
            highlighted: comboBoxControl.highlightedIndex === index
        }

        indicator: Canvas {
            id: canvas
            x: comboBoxControl.width - width - comboBoxControl.rightPadding
            y: comboBoxControl.topPadding + (comboBoxControl.availableHeight - height) / 2
            width: 9
            height: 6
            contextType: "2d"

//            Connections {
//                target: comboBoxControl
//                function onPressedChanged() { requestPaint(); }
//            }

            Connections {
                target: rootContainer

                function onThemeChanged () {
                    canvas.requestPaint();
                }
            }

            onPaint: {
                var context = getContext("2d");
                context.reset();
                context.moveTo(0, 0);
                context.lineTo(width, 0);
                context.lineTo(width / 2, height);
                context.closePath();
                context.fillStyle = rootContainer.mainFontColor;
                context.fill();
            }
        }

        popup: Popup {
            width: comboBoxControl.width*2.5
            implicitHeight: contentItem.implicitHeight
            padding: 1
            Material.theme: rootContainer.themeData.theme === "Dark" ? Material.Dark : Material.Light

            background: dynamicBackground

            Item {
                id: dynamicBackground

                Rectangle {
                    visible: rootContainer.qtVersion < 6
                    color: rootContainer.mainBackgroundColor
                    anchors.fill: parent
                    border.width: 1
                    border.color: rootContainer.mainFontColor
                }

                Pane {
                    anchors.fill: parent
                    visible: rootContainer.qtVersion >= 6
                    Material.theme: rootContainer.themeData.theme === "Dark" ? Material.Dark : Material.Light
                    Material.elevation: 8
    //                Material.roundedScale: Material.SmallScale
                }
            }


            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: comboBoxControl.popup.visible ? comboBoxControl.delegateModel : null
                currentIndex: comboBoxControl.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }
        }
    }
}
