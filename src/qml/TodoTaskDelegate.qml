import QtQuick
import QtQuick.Controls
//import QtQuick.Controls.Material

MouseArea {
    id: taskDragArea

    // TODO: replace var with proper types
    property bool held: false
    property int taskContentWidth
    property var rootContainer
    property var rootTodoContainer
    property var listViewParent
    property int originalHeight
    property var todosColumnsViewPointerFromTask
    property bool makeRoomForTask: false
    property string makeRoomDirection: "up"
    property bool isAnotherTaskEntered: false
    property int columnHeight
    property var currentTargetTaskDraggedInto: null
    property var currentTargetColumnDraggedInto: null
    property int scrollEdgeSize: 50
    property int scrollingDirection: 0
    property var themeData
    property bool areTasksReversed
    property string accentColor: "#2383e2"
    property bool isMouseEnteredTaskContent: false
    property bool isDeletingTask: false
    property var columnParent
    property int taskContentHeight: taskTextEdit.height + 30

    // This properties are initialized by the model's view
    required property string taskText
    required property bool taskChecked
    required property int taskStartLine
    required property int taskEndLine
    required property bool doNeedAnimateFirstNote

    // required property var model // If we want to change the underline model


    anchors {
        left: parent !== null ? parent.left : undefined
        right: parent !== null ? parent.right : undefined
    }
    width: taskContent.width
    height: taskContent.height
    drag.target: held ? taskContent : undefined
    drag.axis: Drag.XAndYAxis
    hoverEnabled: true
    pressAndHoldInterval: 100

//    onPressAndHold: {
//        held = true;
//    }

    onPressed: {
        taskContent.forceActiveFocus(); // For some reason this is needed for taskContent to update its height when taskTextEdit's onEditFinished is called
        held = true;
        taskDragArea.cursorShape = Qt.ClosedHandCursor
    }

    onEntered: {
        isMouseEnteredTaskContent = true;
        taskDragArea.cursorShape = Qt.OpenHandCursor
    }

    onExited: {
        isMouseEnteredTaskContent = false;
        if (!held) taskDragArea.cursorShape = Qt.ArrowCursor
    }

    PropertyAnimation {
        id: firstTaskAnimation
        target: taskContent
        running: taskDragArea.doNeedAnimateFirstNote
        property: "height"
        to: taskDragArea.originalHeight
        from: 0
        duration: 500
        easing.type: Easing.OutExpo

        onFinished: {
            taskDragArea.doNeedAnimateFirstNote = false;
//            taskDragArea.columnParent.isFirstTaskFinishedAnimating = true;
            taskDragArea.originalHeight = taskContent.height;
        }
    }

    PropertyAnimation {
        id: firstTaskAnimation2
        target: taskDragArea
        running: taskDragArea.doNeedAnimateFirstNote
        property: "height"
        to: taskDragArea.originalHeight
        from: 0
        duration: 500
        easing.type: Easing.OutExpo
    }

    PropertyAnimation {
        id: deleteTaskAnimation
        target: taskContent
        running: taskDragArea.isDeletingTask
        property: "height"
        to: 0
        duration: 500
        easing.type: Easing.OutExpo

        onFinished: {
            if (!taskDragArea.rootContainer.isReadOnlyMode) {
                taskDragArea.rootContainer.totalTasks--;
                if (taskDragArea.taskChecked) {
                    taskDragArea.rootContainer.totalCompletedTasks--;
                    taskDragArea.columnParent.numberOfCompletedTasks--;
                }
                let columnID = columnParent.columnID;
                let taskIndex = DelegateModel.itemsIndex;
                taskDragArea.rootContainer.removeTask(taskDragArea.taskStartLine, taskDragArea.taskEndLine, columnID, taskIndex);
            }
        }
    }

    PropertyAnimation {
        id: deleteTaskAnimation2
        target: taskDragArea
        running: taskDragArea.isDeletingTask
        property: "height"
        to: 0
        duration: 500
        easing.type: Easing.OutExpo
    }

    PropertyAnimation {
        id: deleteTaskAnimation3
        target: taskTextEdit
        running: taskDragArea.isDeletingTask
        property: "opacity"
        to: 0.0
        duration: 500
        easing.type: Easing.OutExpo
    }

    PropertyAnimation {
        id: deleteTaskAnimation4
        target: taskCheckBox
        running: taskDragArea.isDeletingTask
        property: "opacity"
        to: 0.0
        duration: 500
        easing.type: Easing.OutExpo
    }

    function callForTasksRearrangement (columnEntered, endLinePosition, taskIndexToInsert) {
        if (columnEntered !== taskDragArea.columnParent && taskDragArea.taskChecked) {
            columnEntered.numberOfCompletedTasks++;
            taskDragArea.columnParent.numberOfCompletedTasks--;
        }

        var columnInsertingIntoID = columnEntered.columnID;
        var columnRemovingFromID = taskDragArea.columnParent.columnID;
        var taskIndexToRemove = DelegateModel.itemsIndex;
        var taskObject = {"taskText": taskDragArea.taskText, "taskChecked": taskDragArea.taskChecked, "taskStartLine": taskDragArea.taskStartLine, "taskEndLine": taskDragArea.taskEndLine, "doNeedAnimateFirstNote": false};
//        var taskObject = Object.assign({}, model);
        rootContainer.rearrangeTasks(taskDragArea.taskStartLine, taskDragArea.taskEndLine, endLinePosition, columnInsertingIntoID, taskIndexToInsert, columnRemovingFromID, taskIndexToRemove, taskObject);
    }

    onReleased: {
        taskDragArea.cursorShape = Qt.ArrowCursor

        if(currentTargetTaskDraggedInto !== null && !taskDragArea.rootContainer.isReadOnlyMode) {
            var colParent = currentTargetTaskDraggedInto.columnParent;
            var endLinePosition;
            var taskIndexToInsert;
            if(currentTargetTaskDraggedInto.makeRoomDirection === "down") {
                if (!areTasksReversed) {
                    endLinePosition = currentTargetTaskDraggedInto.taskStartLine;
                    taskIndexToInsert = currentTargetTaskDraggedInto.DelegateModel.itemsIndex;
                } else {
                    endLinePosition = currentTargetTaskDraggedInto.taskEndLine + 1;
                    taskIndexToInsert = currentTargetTaskDraggedInto.DelegateModel.itemsIndex;
                }

            } else if (currentTargetTaskDraggedInto.makeRoomDirection === "up"){
                if (!areTasksReversed) {
                    endLinePosition = currentTargetTaskDraggedInto.taskEndLine + 1;
                    taskIndexToInsert = currentTargetTaskDraggedInto.DelegateModel.itemsIndex + 1;
                } else {
                    endLinePosition = currentTargetTaskDraggedInto.taskStartLine;
                    taskIndexToInsert = currentTargetTaskDraggedInto.DelegateModel.itemsIndex + 1;
                }
            }

            taskDragArea.callForTasksRearrangement(colParent, endLinePosition, taskIndexToInsert);
        }

        // Drag and drop into columns
        if ((currentTargetTaskDraggedInto === null || currentTargetTaskDraggedInto === undefined) && !taskDragArea.rootContainer.isReadOnlyMode) {
            for (let i = 0; i < todosColumnsViewPointerFromTask.model.items.count; i++) {
                let currentColumn = todosColumnsViewPointerFromTask.itemAtIndex(i);
                if (currentColumn !== null) {
                    let taskContentX = taskContent.mapToItem(currentColumn.parent, 0, 0).x;
                    let taskContentY = taskContent.mapToItem(currentColumn.parent, 0, 0).y;
                    let currentColumnX = currentColumn.mapToItem(currentColumn.parent, 0, 0).x;
                    let currentColumnY = currentColumn.mapToItem(currentColumn.parent, 0, 0).y;
                    if (taskContentX > currentColumnX - taskContent.width/2 &&
                            taskContentX < currentColumnX + currentColumn.width - taskContent.width/2 &&
                            taskContentY > currentColumnY + currentColumn.yUntilTasks &&
                            taskContentY < currentColumnY + currentColumn.height - currentColumn.yUntilTasks) {
                            // Item is inside a colum
                            // TODO: currently this only supports columns with at least one task since our parser can only detect a todo column if it has at least one task (header + task)
                            let lastItemInCurrentColumn = currentColumn.tasksViewPointer.itemAtIndex(currentColumn.tasksViewPointer.count - 1);
                            let lastItemInCurrentColumnY;
                            if (lastItemInCurrentColumn) {
                                lastItemInCurrentColumnY = lastItemInCurrentColumn.mapToItem(currentColumn.parent, 0, 0).y;
                            }
                            if ((lastItemInCurrentColumn && taskContentY + taskContent.width > lastItemInCurrentColumnY + lastItemInCurrentColumn.width) || currentColumn.tasksViewPointer.model.items.count === 0) {
                                // And if the task's Y is higher than the last tax in the current colum or there are no tasks
                                if (!taskDragArea.areTasksReversed) {
                                    let endLinePosition = lastItemInCurrentColumn.taskEndLine + 1;
                                    taskDragArea.callForTasksRearrangement(currentColumn, endLinePosition, currentColumn.taskModel.count);
                                } else {
                                    let endLinePosition = lastItemInCurrentColumn.taskStartLine;
                                    taskDragArea.callForTasksRearrangement(currentColumn, endLinePosition, 0);
                                }
                            }
                        break;
                    }
                }
            }
        }

        if ((currentTargetTaskDraggedInto !== null ||
             currentTargetTaskDraggedInto === null ||
             currentTargetTaskDraggedInto === undefined)
                && taskDragArea.rootContainer.isReadOnlyMode) {
            taskDragArea.rootContainer.informationPopupText = qsTr("Can't edit while in Read-only mode.");
            taskDragArea.rootContainer.showInformationPopup = true;
        }

        held = false;
        if (currentTargetColumnDraggedInto) {
            currentTargetColumnDraggedInto.tasksScrollingDirection = 0;
        }
        if (taskDragArea.columnParent) {
            taskDragArea.columnParent.isTaskFromCurrentColumnScrolling = false;
        }
    }

    Component.onCompleted: {
        taskDragArea.originalHeight = taskTextEdit.height + 30;
    }

    Rectangle {
        id: taskContent
        radius: 5
        width: taskDragArea.taskContentWidth - 20
        height: taskDragArea.doNeedAnimateFirstNote ? 0 : taskTextEdit.height + 30
        antialiasing: true
        border.width: 1
        border.color: {
            if (taskDragArea.taskChecked) {
                if (taskDragArea.themeData.theme === "Dark") {
                    "gray"
                } else {
                    Qt.rgba(55/255, 53/255, 47/255, 0.45)
                }
            } else {
                if (taskDragArea.themeData.theme === "Dark") {
                    "white"
                } else {
                    "#4f4d4d";
                }
            }
        }
        color: taskDragArea.themeData.backgroundColor
        scale: taskDragArea.held ? 1.07 : 1.0
        anchors {
            id: taskContentAnchors
            horizontalCenter: parent.horizontalCenter
        }
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        Drag.active: taskDragArea.held
        Drag.source: taskDragArea
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        Drag.keys: ["task"]

        onYChanged: {
            if (taskDragArea.currentTargetTaskDraggedInto) {
                // Entering another ListView
                let taskContentY = taskContent.mapToGlobal(taskContent.x, taskContent.y).y;
                let currentTargetTaskDraggedIntoY = taskDragArea.currentTargetTaskDraggedInto.mapToGlobal(taskDragArea.currentTargetTaskDraggedInto.x, taskDragArea.currentTargetTaskDraggedInto.y).y;
                if (taskContentY - taskContent.height*2 > currentTargetTaskDraggedIntoY) {
                    taskDragArea.currentTargetTaskDraggedInto.makeRoomDirection = "up";
                } else {
                    taskDragArea.currentTargetTaskDraggedInto.makeRoomDirection = "down";
                }
            }

            // TODO: This is not very efficient, we need to make DropArea in columns work
            if (taskDragArea.held && taskDragArea.currentTargetColumnDraggedInto) {

                let taskContentY = taskContent.mapToItem(taskDragArea.currentTargetColumnDraggedInto.parent, 0, 0).y;
                let currentColumnY = taskDragArea.currentTargetColumnDraggedInto.mapToItem(currentTargetColumnDraggedInto.parent, 0, 0).y;

                if (taskContentY + taskContent.height/2 > currentColumnY + taskDragArea.currentTargetColumnDraggedInto.height - taskDragArea.currentTargetColumnDraggedInto.yUntilTasks) {
                    taskDragArea.currentTargetColumnDraggedInto.tasksScrollingDirection = 1;
                    if (taskDragArea.currentTargetColumnDraggedInto === taskDragArea.columnParent) {
                        taskDragArea.columnParent.isTaskFromCurrentColumnScrolling = true;
                    }
                } else if (taskContentY !==  0 && taskContentY < currentColumnY + taskContent.height +taskDragArea.currentTargetColumnDraggedInto.yUntilTasks) {
                    taskDragArea.currentTargetColumnDraggedInto.tasksScrollingDirection = -1;
                    if (taskDragArea.currentTargetColumnDraggedInto === taskDragArea.columnParent) {
                        taskDragArea.columnParent.isTaskFromCurrentColumnScrolling = true;
                    }
                } else {
                    taskDragArea.currentTargetColumnDraggedInto.tasksScrollingDirection = 0;
                }
            }
        }

        onXChanged: {
            if (taskDragArea.held) {
                let taskContentX = taskContent.mapToItem(todosColumnsViewPointerFromTask, 0, 0).x;
                if (taskContentX < taskDragArea.scrollEdgeSize - taskContent.width/2 + taskDragArea.rootTodoContainer.scrollEdgeSize - 10) {
                    taskDragArea.rootTodoContainer.scrollingDirection = -1;
                } else if (taskContentX + taskContent.width/2 + taskDragArea.rootTodoContainer.scrollEdgeSize > todosColumnsViewPointerFromColumn.width) {
                    taskDragArea.rootTodoContainer.scrollingDirection = 1;
                } else {
                    taskDragArea.rootTodoContainer.scrollingDirection = 0;
                }

                // TODO: This is not very efficient, we need to make DropArea in columns work
                for (let i = 0; i < todosColumnsViewPointerFromTask.model.items.count; i++) {
                    let currentColumn = todosColumnsViewPointerFromTask.itemAtIndex(i);
                    if (currentColumn !== null) {
                        let taskContentX = taskContent.mapToItem(currentColumn.parent, 0, 0).x;
                        let taskContentY = taskContent.mapToItem(currentColumn.parent, 0, 0).y;
                        let currentColumnX = currentColumn.mapToItem(currentColumn.parent, 0, 0).x;
                        let currentColumnY = currentColumn.mapToItem(currentColumn.parent, 0, 0).y;
                        if (taskContentX > currentColumnX - taskContent.width/2 &&
                                taskContentX < currentColumnX + currentColumn.width - taskContent.width/2 &&
                                taskContentY > currentColumnY + currentColumn.yUntilTasks &&
                                taskContentY < currentColumnY + currentColumn.height - currentColumn.yUntilTasks) {
                                taskDragArea.currentTargetColumnDraggedInto = currentColumn;
                            break;
                        }
                    }
                }
            }
        }

        Behavior on scale {
            ScaleAnimator {
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }

        transitions: [
        Transition {
            enabled: taskDragArea.makeRoomForTask && taskDragArea.isAnotherTaskEntered
            AnchorAnimation {
                duration: 100;
                easing.type: Easing.OutCirc;

            }
        },
        Transition {
            enabled: taskDragArea.makeRoomForTask && taskDragArea.isAnotherTaskEntered === false
            SequentialAnimation {

                PropertyAnimation {
                    target: taskDragArea
                    property: "height"
                    to: taskDragArea.originalHeight
                    duration: 100
                    easing.type: Easing.OutCirc
                }

                ScriptAction {
                    script: {
                        if (taskDragArea.isAnotherTaskEntered === false) {
                            taskDragArea.makeRoomForTask = false;
                        }
                    }

                }
            }

        },
        Transition {
            enabled: taskDragArea.held

            PropertyAnimation {
                target: taskDragArea
                property: "height"
                to: 0
                duration: 100
                easing.type: Easing.OutCirc
            }

        },
        Transition {
            enabled: !taskDragArea.held

            PropertyAnimation {
                target: taskDragArea
                property: "height"
                to: taskDragArea.originalHeight
                duration: 100
                easing.type: Easing.OutCirc
            }

        }
        ]

        states: [State {
                when: taskDragArea.held

                ParentChange { target: taskContent; parent: taskDragArea.rootTodoContainer}
                AnchorChanges {
                    target: taskContent
                    anchors { top: undefined; horizontalCenter: undefined; verticalCenter: undefined }
                }
            }
            ,State {
                when: taskDragArea.makeRoomForTask && taskDragArea.makeRoomDirection === "up"

                AnchorChanges {
                    target: taskContent
                    anchors { top: taskDragArea.top; bottom: undefined}
                }
            }
            ,State {
                when: taskDragArea.makeRoomForTask && taskDragArea.makeRoomDirection === "down"

                AnchorChanges {
                    target: taskContent
                    anchors { top: undefined; bottom: taskDragArea.bottom}
                }
            }]

        Row {
            id: taskTextContentContainer
            property int rightLeftMargins: 10
            width: taskContent.width - rightLeftMargins*2
            y: taskContent.height/2 - taskTextEdit.height/2

            CheckBoxMaterial {
                id: taskCheckBox
                checked: taskDragArea.taskChecked
                anchors.verticalCenter: taskTextEdit.verticalCenter
                isReadOnlyMode: taskDragArea.rootContainer.isReadOnlyMode

                onTaskChecked: {
                    taskDragArea.rootContainer.checkTaskInLine(taskDragArea.taskStartLine);
                    taskDragArea.taskChecked = true;
                    taskDragArea.columnParent.numberOfCompletedTasks++;
                    taskDragArea.rootContainer.totalCompletedTasks++;
                    taskDragArea.columnParent.checkTasksForConfetti();
                }

                onTaskUnchecked: {
                    taskDragArea.rootContainer.uncheckTaskInLine(taskDragArea.taskStartLine);
                    taskDragArea.taskChecked = false;
                    taskDragArea.columnParent.numberOfCompletedTasks--;
                    taskDragArea.rootContainer.totalCompletedTasks--;
                }
            }

            TextArea {
                id: taskTextEdit
                property int rightLeftMargins: 10
                width: taskTextContentContainer.width - taskCheckBox.width - trashButton.width/2 // - rightLeftMargins/2
                text: taskDragArea.taskText
                font.family: taskDragArea.rootContainer.bodyFontFamily
                font.pointSize: taskDragArea.rootContainer.platform === "Apple" ? 13 : 11 // TODO: Why setting pixel size causes lines to be rendered differently?
                textFormat: TextEdit.MarkdownText
                background: Rectangle {
                    color: "transparent"
                }
                selectionColor: taskDragArea.accentColor
                selectedTextColor: "white"
                property bool isTextReallyChanged: false

                property bool cursorAnimationRunning: true
                signal anyKeyPressed
                signal cursorHidden
                signal cursorShowed
                readOnly: taskDragArea.rootContainer.isReadOnlyMode

                Keys.onPressed: {
                    taskTextEdit.isTextReallyChanged = true;
                    taskTextEdit.anyKeyPressed();
                    taskTextEdit.cursorAnimationRunning = false;
                }

                Keys.onReleased: {
                    taskTextEdit.cursorAnimationRunning = true;
                }

                onPressed: {
                    taskTextEdit.cursorShowed();
                }

//                Keys.onPressed: {
//                    var textToSend = taskTextEdit.text;
//                    var tempCurs = taskTextEdit.cursorPosition;
//                    taskTextEdit.clear();
//                    taskTextEdit.append(textToSend);
//                    taskTextEdit.forceActiveFocus();
//                    taskTextEdit.cursorPosition = tempCurs;
//                }

                onTextChanged: {
                    if (taskTextEdit.isTextReallyChanged && !taskDragArea.rootContainer.isReadOnlyMode) {
                        var textToSend = taskTextEdit.text;
                        taskDragArea.taskText = taskTextEdit.text;

                        textToSend = textToSend.replace("\n", ""); // We need this due to wrapping
                        root.updateTaskText(taskDragArea.taskStartLine, taskDragArea.taskEndLine, textToSend);
                    }
                }

                onHeightChanged: {
                    if (!taskDragArea.held) {
                        taskContent.height = taskTextEdit.height + 30;
                        taskDragArea.originalHeight = taskTextEdit.height + 30;
                        taskDragArea.height = taskTextEdit.height + 30;
                    }
                }

                cursorDelegate: Rectangle {
                    id: cursorDelegateObject
                    visible: true
                    color: taskDragArea.accentColor
                    width: 2

                    Connections {
                        target: taskTextEdit

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
                        running: taskTextEdit.cursorAnimationRunning

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
                color: {
                    if (taskDragArea.taskChecked) {
                        if (taskDragArea.themeData.theme === "Dark") {
                            "gray"
                        } else {
                            Qt.rgba(55/255, 53/255, 47/255, 0.45)
                        }
                    } else {
                        if (taskDragArea.themeData.theme === "Dark") {
                            "white"
                        } else {
                            "black";
                        }
                    }
                }
                font.strikeout: taskDragArea.taskChecked
                wrapMode: Text.Wrap

                onEditingFinished: {
                    if (!taskDragArea.rootContainer.isReadOnlyMode) {
                        taskTextEdit.isTextReallyChanged = false;
                        var tempText = taskTextEdit.text;
                        taskTextEdit.clear();
                        taskTextEdit.append(tempText);
                        taskTextEdit.cursorAnimationRunning = false;
                        taskTextEdit.cursorHidden();
                    }
                }
            }

            TrashButton {
                id: trashButton
                visible: taskDragArea.isMouseEnteredTaskContent && !taskDragArea.held && !taskDragArea.isDeletingTask
                themeData: taskDragArea.themeData
                anchors.verticalCenter: taskTextEdit.verticalCenter

                onClicked: {
                    if (!taskDragArea.rootContainer.isReadOnlyMode) {
                        taskDragArea.isDeletingTask = true;
                    }

                    if (taskDragArea.rootContainer.isReadOnlyMode) {
                        taskDragArea.rootContainer.informationPopupText = qsTr("Can't edit while in Read-only mode.");
                        taskDragArea.rootContainer.showInformationPopup = true;
                    }
                }
            }
        }
    }

    DropArea {
        id: dropArea
        anchors { fill: parent}
        keys: ["task"]

        onEntered: (drag)=> {
                       taskDragArea.isAnotherTaskEntered = true;
                       var sourceListView = drag.source.listViewParent;
                       var targetListView = taskDragArea.listViewParent;

                       drag.source.currentTargetTaskDraggedInto = taskDragArea;
                       taskDragArea.height = taskContent.height + drag.source.originalHeight + taskDragArea.listViewParent.spacing;
                       taskDragArea.makeRoomForTask = true;
                   }

        onExited: {
            taskDragArea.isAnotherTaskEntered = false;
            var sourceListView = drag.source.listViewParent;
            var targetListView = taskDragArea.listViewParent;


            if (taskDragArea.makeRoomDirection === "up") {
                taskDragArea.makeRoomDirection = "down";
            } else if (taskDragArea.makeRoomDirection === "down"){
                taskDragArea.makeRoomDirection = "up";
            }
            taskDragArea.makeRoomForTask = true;

//                taskContent.anchors.top = undefined;
//                taskContent.anchors.bottom = undefined;
            drag.source.currentTargetTaskDraggedInto = null;
        }
    }
}

