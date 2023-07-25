import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

MouseArea {
    id: dragArea

    property bool held: false
    property int todoColumnContentHeight: 300
    property var rootContainer
    property var rootTodoContainer
    property var todosColumnsViewPointerFromColumn
    property real yUntilTasks: columnTitle.y + columnTitle.height + tasksContainer.marginTop * 2
    property var tasksViewPointer: tasksView
    property var titlesAndTasksData
    property int tasksScrollingDirection: 0
    property var themeData
    property int modelIndexBeforeDragged: DelegateModel.itemsIndex
    property var columnDraggedInto
    property bool areTasksReversed
    property bool isDeleteColumnDialogShown: false
    property bool isMouseEnteredColumn: false
    property int numberOfCompletedTasks: 0
    property var columnModelPointer
    property var taskModel
    property bool isTaskFromCurrentColumnScrolling: false
    property string accentColor: "#2383e2"

    // This properties are initialized by the view's model
    required property int columnID
    required property string title
    required property int columnStartLine
    required property int columnEndLine

    anchors {
        top: parent !== null ? parent.top : undefined;
        bottom: parent !== null ? parent.bottom : undefined;
    }
    width: todoColumnContent.width
    height: todoColumnContent.height
    drag.target: held ? todoColumnContent : undefined
    drag.axis: Drag.XAxis
    hoverEnabled: true
    pressAndHoldInterval: 200

    function checkTasksForConfetti () {
        if (numberOfCompletedTasks === tasksView.model.items.count) {
            rootContainer.showConfetti1 = true;
            rootContainer.emitConfetti = true;
        }
    }

    function createNewTask (newTaskText : string) {
        if (!dragArea.rootContainer.isReadOnlyMode) {
            let lastTaskEndLine = taskModel.count === 0 ? dragArea.columnStartLine+1 : taskModel.get(taskModel.count-1).taskEndLine;
            dragArea.rootContainer.addNewTask(lastTaskEndLine, dragArea.columnID, newTaskText);
            var somet = tasksView.contentHeight - tasksView.height;
        }

        if (dragArea.rootContainer.isReadOnlyMode) {
            dragArea.rootContainer.informationPopupText = qsTr("Can't edit while in Read-only mode.");
            dragArea.rootContainer.showInformationPopup = true;
        }
    }

    Component.onCompleted: {
        for (let i = 0; i< taskModel.count; i++) {
            if (taskModel.get(i).taskChecked) {
                dragArea.numberOfCompletedTasks++;
            }
        }
    }

    onEntered: {
        isMouseEnteredColumn = true;
        if (mouseY < dragArea.yUntilTasks) dragArea.cursorShape = Qt.OpenHandCursor
    }

    onExited: {
        isMouseEnteredColumn = false;
        dragArea.cursorShape = Qt.ArrowCursor
    }

    onPressAndHold: {
        held = true
        dragArea.modelIndexBeforeDragged = DelegateModel.itemsIndex
        dragArea.cursorShape = Qt.ClosedHandCursor
    }

    onPressed: {
        newTaskTextEdit.text = "";
        addNewTaskEditorRect.visible = false;
        addNewTaskButton.visible = true;
    }

    onReleased: {
        held = false
        dragArea.cursorShape = Qt.ArrowCursor
        dragArea.rootTodoContainer.scrollingDirection = 0;

        if (dragArea.columnDraggedInto && dragArea.modelIndexBeforeDragged !== dragArea.DelegateModel.itemsIndex && !dragArea.rootContainer.isReadOnlyMode) {
            var newStartLinePosition;
            if (dragArea.DelegateModel.itemsIndex > dragArea.modelIndexBeforeDragged) {
                // Go down
                newStartLinePosition = dragArea.columnModelPointer.get(dragArea.DelegateModel.itemsIndex-1).columnEndLine;
            } else {
                // Go up
                newStartLinePosition = dragArea.columnModelPointer.get(dragArea.DelegateModel.itemsIndex+1).columnStartLine;
            }
            rootContainer.rearrangeColumns(dragArea.columnStartLine, dragArea.columnEndLine, newStartLinePosition);
        }

        if (columnDraggedInto && modelIndexBeforeDragged !== dragArea.DelegateModel.itemsIndex && dragArea.rootContainer.isReadOnlyMode) {
            dragArea.rootContainer.informationPopupText = qsTr("Can't edit while in Read-only mode.");
            dragArea.rootContainer.showInformationPopup = true;
        }

        dragArea.rootTodoContainer.isItemScrollingColumnsView = false;
    }

    FontIconLoader {
        id: fontIconLoader
    }

    PropertyAnimation {
        id: tasksUpAnimation
        target: tasksView
        property: "contentY"
        to: 0
        running: dragArea.tasksScrollingDirection === -1
        duration: 1000
    }

    PropertyAnimation {
        id: tasksDownAnimation
        target: tasksView
        property: "contentY"
        to: tasksView.contentHeight - tasksView.height
        running: dragArea.tasksScrollingDirection === 1
        duration: Math.max(700 * (tasksView.contentHeight / dragArea.todoColumnContentHeight), 1)
    }

    Rectangle {
        id: todoColumnContent
        radius: 7 //20
        width: 264
        height: dragArea.todoColumnContentHeight
        border.width: dragArea.rootContainer.showColumnsBorders ? 1 : 0
        border.color: dragArea.themeData.theme === "Dark" ? "white" : "black"
        antialiasing: true
        color: dragArea.rootContainer.showColumnsBorders ? dragArea.themeData.backgroundColor : "transparent"
        scale: dragArea.held ? 1.05 : 1.0
        anchors {
            id: todoColumnContentAnchors
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        Drag.active: dragArea.held
        Drag.source: dragArea
        Drag.hotSpot.x: width / 2
        Drag.keys: ["todoColumn"]

        onXChanged: {
            if (dragArea.held) {
                let todoColumnContentX = todoColumnContent.mapToItem(todosColumnsViewPointerFromColumn, 0, 0).x;
                if (todoColumnContentX < dragArea.rootTodoContainer.scrollEdgeSize) {
                    dragArea.rootTodoContainer.scrollingDirection = -1;
                    dragArea.rootTodoContainer.isItemScrollingColumnsView = true;
                } else if (todoColumnContentX + todoColumnContent.width/2 > todosColumnsViewPointerFromColumn.width - dragArea.rootTodoContainer.scrollEdgeSize) {
                    dragArea.rootTodoContainer.scrollingDirection = 1;
                    dragArea.rootTodoContainer.isItemScrollingColumnsView = true;
                } else {
                    dragArea.rootTodoContainer.scrollingDirection = 0;
                }
            }
        }

        Behavior on scale {
            ScaleAnimator {
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }

        states: [State {
                when: dragArea.held

                ParentChange { target: todoColumnContent; parent: dragArea.rootTodoContainer}
                AnchorChanges {
                    target: todoColumnContent
                    anchors { horizontalCenter: undefined; verticalCenter: undefined }
                }
            }]

        Item {
            id: titleContainer
            width: todoColumnContent.width
            height: columnTitle.height + numberOfTaks.height + columnTitle.topMargin + numberOfTaks.topMargin + (progressBarText.visible ? progressBarText.height + progressBarText.topMargin : 0)

            Text {
                id: columnTitle
                property int topMargin: !dragArea.rootContainer.showColumnsBorders ? 0 : 10
                width: todoColumnContent.width - trashButton.width*2 - 35
                elide: Text.ElideRight
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                y: topMargin
                text: dragArea.title
                color: dragArea.themeData.theme === "Dark" ? "white" : "black"
                font.pointSize: dragArea.rootContainer.platform === "Apple" ? 17 : 17 + dragArea.rootContainer.pointSizeOffset
                font.bold: true
                font.family: dragArea.rootContainer.headerFamilyFont

                MouseArea {
                    anchors.fill: parent

                    onDoubleClicked: {
                        if (!dragArea.rootContainer.isReadOnlyMode) {
                            columnTitle.visible = false;
                            columnTitleEditable.text = columnTitle.text;
                            columnTitleEditable.visible = true;
                            columnTitleEditable.forceActiveFocus();
                        }

                        if (dragArea.rootContainer.isReadOnlyMode) {
                            dragArea.rootContainer.informationPopupText = qsTr("Can't edit while in Read-only mode.");
                            dragArea.rootContainer.showInformationPopup = true;
                        }
                    }
                }
            }

            CustomTextField {
                id: columnTitleEditable
                visible: false
                property int topMargin: !dragArea.rootContainer.showColumnsBorders ? 0 : 10
                width: todoColumnContent.width - trashButton.width - columnTitleCancelEditingButton.width - 35
                height: columnTitleEditable.implicitHeight
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                y: topMargin - 5
                text: dragArea.title
                color: dragArea.themeData.theme === "Dark" ? "white" : "black"
                background: Rectangle {
                    height: columnTitleEditable.height
                    radius: 5
                    color: dragArea.themeData.theme === "Dark" ? "#313131" : "#efefef"
                }
                rightPadding: 12 // Neccesary so the text align perfectly with columnTitle
                font.pointSize: dragArea.rootContainer.platform === "Apple" ? 17 : 17 + dragArea.rootContainer.pointSizeOffset
                font.bold: true
                font.family: dragArea.rootContainer.headerFamilyFont

                onEditingFinished: {
                    columnTitle.visible = true;
                    columnTitleEditable.visible = false;
                }

                Keys.onEscapePressed: {
                    columnTitleEditable.text = columnTitle.text;
                    columnTitle.visible = true;
                    columnTitleEditable.visible = false;
                }

                onAccepted: {
                    dragArea.rootContainer.updateColumnTitle(dragArea.columnStartLine, columnTitleEditable.text);
                    columnTitle.text = columnTitleEditable.text;
                    columnTitle.visible = true;
                    columnTitleEditable.visible = false;
                }
            }

            IconButton {
                id: columnTitleCancelEditingButton
                visible: columnTitleEditable.visible
                themeData: dragArea.themeData
                anchors.verticalCenter: columnTitle.verticalCenter
                anchors.right: titleContainer.right
                anchors.rightMargin: numberOfTaks.topMargin
                platform: dragArea.rootContainer.platform
                icon: fontIconLoader.icons.fa_circle_xmark
                iconPointSizeOffset: dragArea.rootContainer.platform === "Apple" ? -4 : -5

                onClicked: {
                    columnTitleEditable.text = columnTitle.text;
                    columnTitle.visible = true;
                    columnTitleEditable.visible = false;
                }
            }

            IconButton {
                id: trashButton
                visible: dragArea.isMouseEnteredColumn && dragArea.mouseY < tasksContainer.y
                anchors.verticalCenter: columnTitle.verticalCenter
                anchors.left: titleContainer.left
                anchors.leftMargin: numberOfTaks.topMargin
                themeData: dragArea.rootContainer.themeData
                icon: fontIconLoader.icons.fa_trash
                platform: dragArea.rootContainer.platform
                iconPointSizeOffset: dragArea.rootContainer.platform === "Apple" ? -5 : -6
                width: 32
                height: 30

                onClicked: {
                    if (!dragArea.rootContainer.isReadOnlyMode){
                        dragArea.isDeleteColumnDialogShown = !dragArea.isDeleteColumnDialogShown;
                    }

                    if (dragArea.rootContainer.isReadOnlyMode) {
                        dragArea.rootContainer.informationPopupText = qsTr("Can't edit while in Read-only mode.");
                        dragArea.rootContainer.showInformationPopup = true;
                    }
                }

                Dialog {
                    parent: Overlay.overlay
                    x: Math.round((dragArea.rootContainer.width - width) / 2)
                    y: Math.round((dragArea.rootContainer.height - height) / 2)
                    id: deleteColumnDialog
                    visible: dragArea.isDeleteColumnDialogShown
                    title: qsTr("Are you sure you want to delete column ") + dragArea.title + "?"
                    standardButtons: Dialog.Yes | Dialog.Cancel
                    font.family: dragArea.rootContainer.bodyFontFamily
                    Material.theme: dragArea.themeData.theme === "Dark" ? Material.Dark : Material.Light
                    Material.accent: "#5b94f5"
//                    Material.roundedScale: Material.SmallScale

                    Text {
                        text: qsTr("This will delete ALL tasks inside ") + dragArea.title + "."
                        font.family: dragArea.rootContainer.bodyFontFamily
                        font.pointSize: root.platform === "Apple" ? 15 : 15 + dragArea.rootContainer.pointSizeOffset
                        color: dragArea.themeData.theme === "Dark" ? "white" : "black"
                    }

                    onAccepted: {
                        dragArea.isDeleteColumnDialogShown = false;
                        dragArea.rootContainer.removeColumn(dragArea.columnStartLine, dragArea.columnEndLine, dragArea.columnID);
                    }
                    onRejected: {
                        dragArea.isDeleteColumnDialogShown = false;
                    }
                    onDiscarded: {
                        dragArea.isDeleteColumnDialogShown = false;
                    }
                }
            }

            Text {
                id: numberOfTaks
                property int topMargin: 12
                x: todoColumnContent.width / 2 - numberOfTaks.width/2
                y: columnTitle.y + numberOfTaks.height + topMargin
                color: dragArea.themeData.theme === "Dark" ? "white" : "black"
                font.pointSize: dragArea.rootContainer.platform === "Apple" ? 11 : 11 + dragArea.rootContainer.pointSizeOffset
                font.family: dragArea.rootContainer.bodyFontFamily
                text: tasksView.model.items.count.toString() + (tasksView.model.items.count === 1 ? " task" : " tasks")
            }

            Text {
                id: progressBarText
                property int topMargin: 7
                visible: dragArea.rootContainer.showProgressBar
                x: 15
                y: numberOfTaks.y + numberOfTaks.height + topMargin
                color: dragArea.themeData.theme === "Dark" ? "white" : "black"
                text: dragArea.numberOfCompletedTasks.toString() + " of " + tasksView.model.items.count.toString()
                font.family: dragArea.rootContainer.bodyFontFamily
                font.pointSize: dragArea.rootContainer.platform === "Apple" ? 13 : 13 + dragArea.rootContainer.pointSizeOffset
            }

            ProgressBar {
                id: progressBar
                visible: dragArea.rootContainer.showProgressBar
                property int leftMargin: 7
                x: progressBarText.x + progressBarText.width + leftMargin
                anchors.verticalCenter: progressBarText.verticalCenter
                width: tasksContainer.width - progressBarText.width - leftMargin - 27
                value: dragArea.numberOfCompletedTasks / (tasksView.model.items.count === 0 ? 1 : tasksView.model.items.count)
            }
        }

        Item {
            id: tasksContainer
            property int marginTop: 5
            width: todoColumnContent.width
            height: todoColumnContent.height - (titleContainer.height + marginTop*2)
            y: titleContainer.y + titleContainer.height + marginTop
            property int rightLeftMargin: 10

            DelegateModel {
                id: tasksVisualModel
                model: dragArea.taskModel
                delegate: TodoTaskDelegate {
                    taskContentWidth: tasksContainer.width
                    rootContainer: dragArea.rootContainer
                    rootTodoContainer: dragArea.rootTodoContainer
                    listViewParent: tasksView
                    todosColumnsViewPointerFromTask: dragArea.todosColumnsViewPointerFromColumn
                    columnParent: dragArea
                    columnHeight: dragArea.todoColumnContentHeight
                    themeData: dragArea.themeData
                    areTasksReversed: dragArea.areTasksReversed
                }
            }

            ListView {
                id: tasksView
                property var itemAtBottom: addNewTaskButton.visible ? addNewTaskButton :addNewTaskEditorRect
                model: tasksVisualModel
//                anchors { fill: parent }
                width: parent.width
                height: tasksView.contentHeight < parent.height - itemAtBottom.height - parent.marginTop*2 ? tasksView.contentHeight : parent.height - itemAtBottom.height - parent.marginTop*2
                clip: true
                orientation: ListView.Vertical
                cacheBuffer: dragArea.isTaskFromCurrentColumnScrolling ? 100000000 : 1000 // TODO: is this a good solution
//                // so tasks won't be destroyed while dragging them during scrolling? Or maybe we can recreate a taskContent not tied to the
//                // ListView just when scrolling.

                ScrollBar.vertical: CustomVerticalScrollBar {
                    themeData: dragArea.themeData
                }
            }

            Rectangle {
                id: addNewTaskEditorRect
                visible: false
                anchors.top: tasksView.contentHeight < parent.height ? tasksView.bottom : undefined
                anchors.bottom: tasksView.contentHeight >= parent.height ? parent.bottom : undefined
                anchors.topMargin: 10
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - parent.rightLeftMargin*2
                height: newTaskTextEdit.height + 20
                radius: 5
                border.width: 1
                border.color: dragArea.accentColor
                color: dragArea.themeData.backgroundColor
                x: parent.rightLeftMargin

                CustomTextArea {
                    id: newTaskTextEdit
                    width: parent.width - tasksContainer.rightLeftMargin*2 - cancelEditingButton.width
                    height: newTaskTextEdit.implicitHeight
                    anchors.left: parent.left
                    anchors.leftMargin: tasksContainer.rightLeftMargin
                    anchors.verticalCenter: parent.verticalCenter
                    placeholderText: "Enter a new task..."
                    font.family: dragArea.rootContainer.bodyFontFamily
                    font.pointSize: dragArea.rootContainer.platform === "Apple" ? 13 : 10 // TODO: Why setting pixel size causes lines to be rendered differently?
                    themeData: dragArea.themeData
                    color: dragArea.themeData.theme === "Dark" ? "white" : "black"
                    placeholderTextColor: dragArea.themeData.theme === "Dark" ? "#676767" : "#787878"
                    selectionColor: dragArea.accentColor
                    selectedTextColor: "white"
                    accentColor: dragArea.accentColor
                    readOnly: dragArea.rootContainer.isReadOnlyMode

                    onReturnPressed: {
                        if (!newTaskTextEdit.isHoldingShift) {
                            if (newTaskTextEdit.text.trim() !== "") {
                                createNewTask(newTaskTextEdit.text);
                                newTaskTextEdit.text = "";
                            }
                        } else {
                            newTaskTextEdit.insert(newTaskTextEdit.cursorPosition, '\n');
                        }
                    }

                    Keys.onEscapePressed: {
                        newTaskTextEdit.text = "";
                        addNewTaskEditorRect.visible = false;
                        addNewTaskButton.visible = true;
                    }

                    onEditingFinished: {
                        newTaskTextEdit.text = "";
                        addNewTaskEditorRect.visible = false;
                        addNewTaskButton.visible = true;
                    }
                }

                IconButton {
                    id: cancelEditingButton
                    themeData: dragArea.themeData
                    anchors.verticalCenter: newTaskTextEdit.verticalCenter
                    anchors.left: newTaskTextEdit.right
                    anchors.leftMargin: tasksContainer.rightLeftMargin/2
                    platform: dragArea.rootContainer.platform
                    icon: fontIconLoader.icons.fa_circle_xmark
                    iconPointSizeOffset: dragArea.rootContainer.platform === "Apple" ? -4 : -5

                    onClicked: {
                        newTaskTextEdit.text = "";
                        addNewTaskEditorRect.visible = false;
                        addNewTaskButton.visible = true;
                    }
                }
            }

            TextButton {
                id: addNewTaskButton
                anchors.top: tasksView.contentHeight < parent.height ? tasksView.bottom : undefined
                anchors.bottom: tasksView.contentHeight >= parent.height ? parent.bottom : undefined
                anchors.topMargin: 10
                x: parent.rightLeftMargin
                text: qsTr("Add a task")
                icon: fontIconLoader.icons.fa_plus
                backgroundWidth: parent.width - parent.rightLeftMargin*2
                platform: dragArea.rootContainer.platform
                displayFontFamily: dragArea.rootContainer.bodyFontFamily
                textAlignment: TextButton.TextAlign.Middle
                themeData: dragArea.themeData
                iconColorDefault: dragArea.themeData.theme === "Dark" ? "#5b94f5" : "black"
                backgroundHeight: 30

                onClicked: {
                    if (!dragArea.rootContainer.isReadOnlyMode) {
                        addNewTaskButton.visible = false;
                        addNewTaskEditorRect.visible = true;
                        newTaskTextEdit.forceActiveFocus();
                    }
                }
            }
        }
    }

    DropArea {
        anchors.top: parent.top
        width: todoColumnContent.width
        height: titleContainer.height
        keys: ["todoColumn"]

        onEntered: (drag)=> {
           drag.source.columnDraggedInto = dragArea;

           if (!dragArea.rootContainer.isReadOnlyMode) {
               dragArea.columnModelPointer.move(
                   drag.source.DelegateModel.itemsIndex,
                   dragArea.DelegateModel.itemsIndex, 1);
            }
        }
    }

    // Todo: Why when this is on the drag and drop between tasks doesn't work
    // We work around this using custom "collision detection"
//    DropArea {
//        width: todoColumnContent.width
//        height: tasksContainer.height
//        y: tasksContainer.y
//        keys: ["task"] // Even a different key doesn't work

//        onEntered: (drag)=> {
//                       console.log("Task dragged into column")
//                   }
//    }
}

