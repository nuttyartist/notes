import QtQuick 2.12
import QtQuick.Controls 2.12

MouseArea {
    id: viewMouseArea
    width: backgroundRectangle.width
    height: backgroundRectangle.height
    hoverEnabled: true
    property bool isExited: false
    enum ViewType {
        Text,
        Kanban
    }
    property int currentViewType: ViewChooserButton.ViewType.Text
    signal viewClicked
    signal viewUnclicked
    property string displayFontFamily: "Roboto"
    property string platform: ""
    property string mainFontColor
    property string highlightBackgroundColor
    property string pressedBackgroundColor
    property bool checked: false
    property int pointSizeOffset: -4

    signal viewSelected
    signal themeChanged

    onEntered: {
        backgroundRectangle.color = viewMouseArea.highlightBackgroundColor;
        isExited = false;
        cursorShape = Qt.PointingHandCursor;
    }

    onPressed: {
        backgroundRectangle.color = viewMouseArea.pressedBackgroundColor
//        themeRectangle.border.color = "#2383e2";
    }

    onExited: {
        backgroundRectangle.color = "transparent";
        isExited = true;
        cursorShape = Qt.ArrowCursor;
    }

    onReleased: {
        if (isExited) {
            backgroundRectangle.color = "transparent";
        } else {
            backgroundRectangle.color = viewMouseArea.highlightBackgroundColor;
        }
    }

    onClicked: {
        viewMouseArea.checked = true;
        viewMouseArea.viewClicked();
        themeText.color = "#2383e2";
    }

    onViewSelected: {
        viewMouseArea.checked = true;
        themeText.color = "#2383e2";
    }

    Rectangle {
        id: backgroundRectangle
        width: 107
        height: 80
        color: "transparent"
        radius: 3

        Column {
            anchors.centerIn: parent

            Item {
                id: viewContainer
                Item {
                    id: textViewContainer
                    visible: viewMouseArea.currentViewType === ViewChooserButton.ViewType.Text
                    width: paperTextRect.width
                    height: paperTextRect.height

                    Component.onCompleted: {
                        if (viewMouseArea.currentViewType === ViewChooserButton.ViewType.Text) {
                            viewContainer.width = textViewContainer.width;
                            viewContainer.height = textViewContainer.height;
                        }
                    }

                    function updateTextViewContainerColors () {
                        paperTextRect.border.color = viewMouseArea.mainFontColor;
                        paperTextRect2.color = viewMouseArea.mainFontColor;
                        paperTextRect3.color = viewMouseArea.mainFontColor;
                        paperTextRect4.color = viewMouseArea.mainFontColor;
                        paperTextRect5.color = viewMouseArea.mainFontColor;
                        themeText.color = viewMouseArea.mainFontColor;
                    }

                    Connections {
                        target: viewMouseArea

                        function onViewClicked () {
                            paperTextRect.border.color = "#2383e2";
                            paperTextRect2.color = "#2383e2";
                            paperTextRect3.color = "#2383e2";
                            paperTextRect4.color = "#2383e2";
                            paperTextRect5.color = "#2383e2";
                        }

                        function onViewUnclicked () {
                            viewMouseArea.checked = false;
                            textViewContainer.updateTextViewContainerColors();
                        }

                        function onViewSelected () {
                            paperTextRect.border.color = "#2383e2";
                            paperTextRect2.color = "#2383e2";
                            paperTextRect3.color = "#2383e2";
                            paperTextRect4.color = "#2383e2";
                            paperTextRect5.color = "#2383e2";
                        }

                        function onThemeChanged () {
                            if (!viewMouseArea.checked)
                                textViewContainer.updateTextViewContainerColors();
                        }
                    }

                    Rectangle {
                        id: paperTextRect
                        width: 20
                        height: 32
                        border.width: 1
                        radius: 5
                        border.color: viewMouseArea.mainFontColor
                        color: "transparent"

                        Rectangle {
                            id: paperTextRect2
                            x: 5
                            y: 10
                            width: parent.width - 10
                            height: 1
                            color: viewMouseArea.mainFontColor
                        }

                        Rectangle {
                            id: paperTextRect3
                            x: 5
                            y: 14
                            width: parent.width - 10
                            height: 1
                            color: viewMouseArea.mainFontColor
                        }

                        Rectangle {
                            id: paperTextRect4
                            x: 5
                            y: 18
                            width: parent.width - 10
                            height: 1
                            color: viewMouseArea.mainFontColor
                        }

                        Rectangle {
                            id: paperTextRect5
                            x: 5
                            y: 22
                            width: parent.width - 10
                            height: 1
                            color: viewMouseArea.mainFontColor
                        }
                    }
                }

                Item {
                    id: kanbanViewContainer
                    visible: viewMouseArea.currentViewType === ViewChooserButton.ViewType.Kanban
                    width: kanbanViewRect.width
                    height: kanbanViewRect.height

                    Component.onCompleted: {
                        if (viewMouseArea.currentViewType === ViewChooserButton.ViewType.Kanban) {
                            viewContainer.width = kanbanViewContainer.width;
                            viewContainer.height = kanbanViewContainer.height;
                        }
                    }

                    function updateKanbanViewContainerColors () {
                        kanbanViewRect.border.color = viewMouseArea.mainFontColor;
                        kanbanViewRect2.border.color = viewMouseArea.mainFontColor;
                        kanbanViewRect3.border.color = viewMouseArea.mainFontColor;
                        kanbanViewRect4.border.color = viewMouseArea.mainFontColor;
                        themeText.color = viewMouseArea.mainFontColor;
                    }

                    Connections {
                        target: viewMouseArea

                        function onViewClicked () {
                            kanbanViewRect.border.color = "#2383e2";
                            kanbanViewRect2.border.color = "#2383e2";
                            kanbanViewRect3.border.color = "#2383e2";
                            kanbanViewRect4.border.color = "#2383e2";
                        }

                        function onViewUnclicked () {
                            viewMouseArea.checked = false;
                            kanbanViewContainer.updateKanbanViewContainerColors();
                        }

                        function onViewSelected () {
                            kanbanViewRect.border.color = "#2383e2";
                            kanbanViewRect2.border.color = "#2383e2";
                            kanbanViewRect3.border.color = "#2383e2";
                            kanbanViewRect4.border.color = "#2383e2";
                        }

                        function onThemeChanged () {
                            if (!viewMouseArea.checked)
                                kanbanViewContainer.updateKanbanViewContainerColors();
                        }
                    }

                    Rectangle {
                        id: kanbanViewRect
                        width: 32
                        height: 32
                        border.color: viewMouseArea.mainFontColor
                        border.width: 1
                        color: "transparent"
                        radius: 5

                        Rectangle {
                            id: kanbanViewRect2
                            x: 5
                            y: 7
                            width: 5
                            height: 15
                            radius: 3
                            border.width: 1
                            color: "transparent"
                            border.color: viewMouseArea.mainFontColor
                        }

                        Rectangle {
                            id: kanbanViewRect3
                            x: 13
                            y: 7
                            width: 5
                            height: 10
                            radius: 3
                            border.width: 1
                            color: "transparent"
                            border.color: viewMouseArea.mainFontColor
                        }

                        Rectangle {
                            id: kanbanViewRect4
                            x: 21
                            y: 7
                            width: 5
                            height: 20
                            radius: 3
                            border.width: 1
                            color: "transparent"
                            border.color: viewMouseArea.mainFontColor
                        }
                    }
                }
            }

            Item {
                width: 1
                height: 7
            }

            Text {
                id: themeText
                anchors.horizontalCenter: viewContainer.horizontalCenter
                text: {
                    if (viewMouseArea.currentViewType === ViewChooserButton.ViewType.Text) {
                        "Text"
                    } else if (viewMouseArea.currentViewType === ViewChooserButton.ViewType.Kanban) {
                        "Kanban"
                    }
                }
                color: viewMouseArea.mainFontColor
                font.family: viewMouseArea.displayFontFamily
                font.pointSize: viewMouseArea.platform === "Apple" ? 13 : 13 + viewMouseArea.pointSizeOffset
            }
        }
    }
}
