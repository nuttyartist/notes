import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

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
    //    property bool isFirstTaskFinishedAnimating: true

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

    function checkTasksForConfetti () {
        if (numberOfCompletedTasks === tasksView.model.items.count) {
            rootContainer.showConfetti1 = true;
            rootContainer.emitConfetti = true;
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

    onPressed: {
        held = true
        dragArea.modelIndexBeforeDragged = DelegateModel.itemsIndex
        dragArea.cursorShape = Qt.ClosedHandCursor
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
                } else if (todoColumnContentX + todoColumnContent.width/2 > todosColumnsViewPointerFromColumn.width - dragArea.rootTodoContainer.scrollEdgeSize) {
                    dragArea.rootTodoContainer.scrollingDirection = 1;
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
                property int topMargin: 10
                width: todoColumnContent.width - plusButton.width - trashButton.width - 35
                elide: Text.ElideRight
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                y: topMargin
                text: dragArea.title
                color: dragArea.themeData.theme === "Dark" ? "white" : "black"
                font.pointSize: root.platform === "Apple" ? 17 : 15
                font.bold: true
                font.family: dragArea.rootContainer.headerFamilyFont

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
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

            TextField {
                id: columnTitleEditable
                visible: false
                property int topMargin: 10
                width: todoColumnContent.width - plusButton.width - trashButton.width - 35
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                y: topMargin - 8
                text: dragArea.title
                color: dragArea.themeData.theme === "Dark" ? "white" : "black"
                background: Rectangle { // TODO: Causes type error on macOS, but looks fine?
                    color: "transparent"
                }
                font.pointSize: root.platform === "Apple" ? 17 : 15
                font.bold: true
                font.family: dragArea.rootContainer.headerFamilyFont

                onEditingFinished: {
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

            PlusButton {
                id: plusButton
                anchors.verticalCenter: columnTitle.verticalCenter
                anchors.right: titleContainer.right
                anchors.rightMargin: numberOfTaks.topMargin
                themeData: dragArea.themeData

                onClicked: {
                    if (!dragArea.rootContainer.isReadOnlyMode) {
//                    if (dragArea.isFirstTaskFinishedAnimating) { // This slows down new tasks creation, if we want that
//                        dragArea.isFirstTaskFinishedAnimating = false;
                        tasksView.positionViewAtBeginning();
                        let firstTaskStartLine = taskModel.count === 0 ? dragArea.columnStartLine+1 : taskModel.get(0).taskStartLine;
                        dragArea.rootContainer.addNewTask(firstTaskStartLine, dragArea.columnID);
//                    }
                    }

                    if (dragArea.rootContainer.isReadOnlyMode) {
                        dragArea.rootContainer.informationPopupText = qsTr("Can't edit while in Read-only mode.");
                        dragArea.rootContainer.showInformationPopup = true;
                    }
                }
            }

            TrashButton {
                id: trashButton
                visible: dragArea.isMouseEnteredColumn && dragArea.mouseY < tasksContainer.y
                anchors.verticalCenter: columnTitle.verticalCenter
                anchors.left: titleContainer.left
                anchors.leftMargin: numberOfTaks.topMargin
                themeData: dragArea.rootContainer.themeData

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
                    id: deleteColumnDialog
                    visible: dragArea.isDeleteColumnDialogShown
                    title: qsTr("Are you sure you want to delete column ") + dragArea.title + "?"
                    standardButtons: Dialog.Yes | Dialog.Cancel
                    font.family: dragArea.rootContainer.bodyFontFamily
                    Material.theme: dragArea.themeData.theme === "Dark" ? Material.Dark : Material.Light
                    Material.accent: "#5b94f5"

                    Text {
                        text: qsTr("This will delete ALL tasks inside ") + dragArea.title + "."
                        font.family: dragArea.rootContainer.bodyFontFamily
                        font.pointSize: root.platform === "Apple" ? 15 : 13
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
                font.pointSize: root.platform === "Apple" ? 11 : 9
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
                font.pointSize: root.platform === "Apple" ? 13 : 11
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
            height: todoColumnContent.height - (titleContainer.height + marginTop * 2)
            y: titleContainer.y + titleContainer.height + marginTop

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
                model: tasksVisualModel
                anchors { fill: parent }
                clip: true
                orientation: ListView.Vertical
                spacing: 10
                cacheBuffer: dragArea.isTaskFromCurrentColumnScrolling ? 100000000 : 1000 // TODO: is this a good solution
//                // so tasks won't be destroyed while dragging them during scrolling? Or maybe we can recreate a taskContent not tied to the
//                // ListView just when scrolling.
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

