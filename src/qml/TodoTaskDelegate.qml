import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Universal 2.12

MouseArea {
    id: taskDragArea

    // TODO: replace var with proper types
    property bool held: false
    property int taskContentWidth
    property var rootContainer
    property var rootTodoContainer
    property var listViewParent
    property var todosColumnsViewPointerFromTask
    property bool makeRoomForTask: false
    property string makeRoomDirection: "down"
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
    property int taskContentHeight: taskTextEdit.visible ? taskTextEdit.implicitHeight + 23 : taskTextNonEditable.implicitHeight + 35 // 30, 42
    property int originalHeight: taskDragArea.taskContentHeight + taskDragArea.spacingInPractice
    property bool makingRoomAnimationFinished: true
    property alias taskContentAlias: taskContent
    property var previouslyEnteredTask: null
    property int spacing: 10
    property int spacingInPractice: taskDragArea.index === listViewParent.count-1 ? 0 : taskDragArea.spacing // TODO: why taskDragArea.model.count, listViewParent.count, are all unreliable?
    property bool shouldRestoreOriginalSize: false
    property bool shouldHideRoot: false
    property var currentlyEnteredColumn: listViewParent
    property real moveTaskBeforeDeletionX
    property real moveTaskBeforeDeletionY
    property bool shouldMoveTaskBeforeDeletion: false

    // These properties are initialized by the model's view
    required property var model
    required property int index
    required property string taskText
    required property bool taskChecked
    required property int taskStartLine
    required property int taskEndLine
    required property bool doNeedAnimateTaskCreation

    anchors {
        left: parent !== null ? parent.left : undefined
        right: parent !== null ? parent.right : undefined
    }
    width: taskContent.width
    height: originalHeight
    drag.target: held ? taskContent : undefined
    drag.axis: Drag.XAndYAxis
    hoverEnabled: true
    pressAndHoldInterval: 200

    onPressAndHold: {
        taskContent.forceActiveFocus(); // For some reason this is needed for taskContent to update its height when taskTextEdit's onEditFinished is called
        held = true;
        cursorShape = Qt.ClosedHandCursor
    }

    onEntered: {
        isMouseEnteredTaskContent = true;
        cursorShape = Qt.OpenHandCursor
    }

    onExited: {
        isMouseEnteredTaskContent = false;
        if (!held) {
            cursorShape = Qt.ArrowCursor;
        }
    }

    FontIconLoader {
        id: fontIconLoader
    }

    PropertyAnimation {
        id: firstTaskAnimation
        target: taskContent
        running: taskDragArea.doNeedAnimateTaskCreation
        property: "height"
        to: taskDragArea.taskContentHeight
        from: 0
        duration: 500
        easing.type: Easing.OutExpo

        onStarted: {
            // Rather than using listViewParent.positionViewAtEnd(); This will smoothly scroll based on the animation
           taskDragArea.listViewParent.contentY = Qt.binding(function () { return taskDragArea.listViewParent.contentHeight - taskDragArea.listViewParent.height });
        }

        onFinished: {
            taskDragArea.doNeedAnimateTaskCreation = false;
            // Break the binding
            taskDragArea.listViewParent.contentY = taskDragArea.listViewParent.contentY;
        }
    }

    PropertyAnimation {
        id: firstTaskAnimation2
        target: taskDragArea
        running: taskDragArea.doNeedAnimateTaskCreation
        property: "height"
        to: taskDragArea.originalHeight
        from: 0
        duration: 500
        easing.type: Easing.OutExpo

        onFinished: {
            taskDragArea.height = Qt.binding(function () { return taskDragArea.originalHeight });
        }
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
        target: taskTextEdit.visible ? taskTextEdit : taskTextNonEditable
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

    PropertyAnimation {
        id: restoreOriginalSizeAnimation
        target: taskDragArea
        running: taskDragArea.shouldRestoreOriginalSize
        property: "height"
        to: taskDragArea.originalHeight
        duration: 300
        easing.type: Easing.OutCirc

        onFinished: {
            taskDragArea.shouldRestoreOriginalSize = false;
            taskDragArea.makeRoomForTask = false;
            taskDragArea.height = Qt.binding(function () { return taskDragArea.originalHeight });
        }
    }

    PropertyAnimation {
        id: hideRootAnimation
        target: taskDragArea
        running: taskDragArea.shouldHideRoot
        property: "height"
        to: 0
        duration: 300
        easing.type: Easing.OutCirc
    }

    PropertyAnimation {
        id: moveTaskBeforeDeletionXAnimation
        target: taskContent
        running: taskDragArea.shouldMoveTaskBeforeDeletion
        property: "x"
        to: taskDragArea.moveTaskBeforeDeletionX
        duration: 300
        easing.type: Easing.OutCirc

        onFinished: {
            timerBeforeDeletion.start();
        }
    }

    Timer {
        id: timerBeforeDeletion
        interval: 100

        onTriggered: {
            taskDragArea.shouldMoveTaskBeforeDeletion = false;
            taskDragArea.onReleaseAction();
        }
    }

    PropertyAnimation {
        id: moveTaskBeforeDeletionYAnimation
        target: taskContent
        running: taskDragArea.shouldMoveTaskBeforeDeletion
        property: "y"
        to: taskDragArea.moveTaskBeforeDeletionY
        duration: 300
        easing.type: Easing.OutCirc
    }

    function callForTasksRearrangement (columnEntered, endLinePosition, taskIndexToInsert) {
        if (columnEntered !== taskDragArea.columnParent && taskDragArea.taskChecked) {
            columnEntered.numberOfCompletedTasks++;
            taskDragArea.columnParent.numberOfCompletedTasks--;
        }

        var columnInsertingIntoID = columnEntered.columnID;
        var columnRemovingFromID = taskDragArea.columnParent.columnID;
        var taskIndexToRemove = DelegateModel.itemsIndex;
        var taskObject = {"taskText": taskDragArea.taskText, "taskChecked": taskDragArea.taskChecked, "taskStartLine": taskDragArea.taskStartLine, "taskEndLine": taskDragArea.taskEndLine, "doNeedAnimateTaskCreation": false};
//        var taskObject = Object.assign({}, model);
        rootContainer.rearrangeTasks(taskDragArea.taskStartLine, taskDragArea.taskEndLine, endLinePosition, columnInsertingIntoID, taskIndexToInsert, columnRemovingFromID, taskIndexToRemove, taskObject);
    }

    function moveTaskBeforeDeletion () {
        if (taskDragArea.currentTargetTaskDraggedInto !== null) {
            taskDragArea.moveTaskBeforeDeletionX = taskDragArea.currentTargetTaskDraggedInto.taskContentAlias.mapToItem(taskDragArea.rootTodoContainer, 0, 0).x;
            if (taskDragArea.currentTargetTaskDraggedInto.makeRoomDirection === "down") {
                taskDragArea.moveTaskBeforeDeletionY = taskDragArea.currentTargetTaskDraggedInto.mapToItem(taskDragArea.rootTodoContainer, 0, 0).y;
            } else {
                taskDragArea.moveTaskBeforeDeletionY = taskDragArea.currentTargetTaskDraggedInto.mapToItem(taskDragArea.rootTodoContainer, 0, 0).y + taskDragArea.currentTargetTaskDraggedInto.taskContentAlias.height + taskDragArea.spacing;
            }
            taskContent.scale = 1.0;
            taskDragArea.shouldMoveTaskBeforeDeletion = true;
        } else if (taskDragArea.currentTargetColumnDraggedInto !== null) {
            var lastItemInCurrentColumn = taskDragArea.currentTargetColumnDraggedInto.tasksViewPointer.itemAtIndex(taskDragArea.currentTargetColumnDraggedInto.tasksViewPointer.count - 1);
            if (lastItemInCurrentColumn) {
                taskDragArea.moveTaskBeforeDeletionX = lastItemInCurrentColumn.taskContentAlias.mapToItem(taskDragArea.rootTodoContainer, 0, 0).x;
                taskDragArea.moveTaskBeforeDeletionY = lastItemInCurrentColumn.mapToItem(taskDragArea.rootTodoContainer, 0, 0).y + lastItemInCurrentColumn.taskContentAlias.height + taskDragArea.spacing;
                taskContent.scale = 1.0;
                taskDragArea.shouldMoveTaskBeforeDeletion = true;
            } else {
                onReleaseAction();
            }
        } else {
            onReleaseAction();
        }
    }

    function onReleaseAction () {
        taskDragArea.cursorShape = Qt.ArrowCursor;

        if (taskDragArea.rootContainer.isReadOnlyMode) {
            taskDragArea.shouldRestoreOriginalSize = true;
        }

        if (taskDragArea.currentTargetTaskDraggedInto !== null && !taskDragArea.rootContainer.isReadOnlyMode) {
            var colParent = currentTargetTaskDraggedInto.columnParent;
            var endLinePosition;
            var taskIndexToInsert;
            if(taskDragArea.currentTargetTaskDraggedInto.makeRoomDirection === "down") {
                if (!areTasksReversed) {
                    endLinePosition = taskDragArea.currentTargetTaskDraggedInto.taskStartLine;
                    taskIndexToInsert = taskDragArea.currentTargetTaskDraggedInto.DelegateModel.itemsIndex;
                } else {
                    endLinePosition = taskDragArea.currentTargetTaskDraggedInto.taskEndLine + 1;
                    taskIndexToInsert = taskDragArea.currentTargetTaskDraggedInto.DelegateModel.itemsIndex;
                }

            } else if (taskDragArea.currentTargetTaskDraggedInto.makeRoomDirection === "up"){
                if (!areTasksReversed) {
                    endLinePosition = taskDragArea.currentTargetTaskDraggedInto.taskEndLine + 1;
                    taskIndexToInsert = taskDragArea.currentTargetTaskDraggedInto.DelegateModel.itemsIndex + 1;
                } else {
                    endLinePosition = taskDragArea.currentTargetTaskDraggedInto.taskStartLine;
                    taskIndexToInsert = taskDragArea.currentTargetTaskDraggedInto.DelegateModel.itemsIndex + 1;
                }
            }

            taskDragArea.callForTasksRearrangement(colParent, endLinePosition, taskIndexToInsert);
        }

        // Drag and drop into columns
        if ((taskDragArea.currentTargetTaskDraggedInto === null || taskDragArea.currentTargetTaskDraggedInto === undefined) && !taskDragArea.rootContainer.isReadOnlyMode) {
            for (let i = 0; i < taskDragArea.todosColumnsViewPointerFromTask.model.items.count; i++) {
                let currentColumn = taskDragArea.todosColumnsViewPointerFromTask.itemAtIndex(i);
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

        if ((taskDragArea.currentTargetTaskDraggedInto !== null ||
             taskDragArea.currentTargetTaskDraggedInto === null ||
             taskDragArea.currentTargetTaskDraggedInto === undefined)
                && taskDragArea.rootContainer.isReadOnlyMode) {
            taskDragArea.rootContainer.informationPopupText = qsTr("Can't edit while in Read-only mode.");
            taskDragArea.rootContainer.showInformationPopup = true;
        }

        held = false;
        if (currentTargetColumnDraggedInto) {
            taskDragArea.currentTargetColumnDraggedInto.tasksScrollingDirection = 0;
        }
        if (taskDragArea.columnParent) {
            taskDragArea.columnParent.isTaskFromCurrentColumnScrolling = false;
        }

        taskDragArea.rootTodoContainer.isItemScrollingColumnsView = false;

        if (taskDragArea.height === 0) {
            taskDragArea.shouldRestoreOriginalSize = true;
        }
    }

    function onReleasedTask () {
        taskDragArea.moveTaskBeforeDeletion();
    }

    onReleased: {
        onReleasedTask();
    }

    Rectangle {
        id: taskContent
        radius: 5
        width: taskDragArea.taskContentWidth - 20
        height: taskDragArea.doNeedAnimateTaskCreation ? 0 : taskDragArea.taskContentHeight
        antialiasing: true
        border.width: 1
        border.color: {
            if(taskDragArea.held) {
                taskDragArea.accentColor;
            } else if (taskDragArea.taskChecked) {
                if (taskDragArea.themeData.theme === "Dark") {
                    "gray";
                } else {
                    Qt.rgba(55/255, 53/255, 47/255, 0.45)
                }
            } else {
                if (taskDragArea.themeData.theme === "Dark") {
                    "white";
                } else {
                    "#4f4d4d";
                }
            }
        }
        color: taskDragArea.themeData.backgroundColor
        scale: taskDragArea.held ? 1.07 : 1.0
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        anchors.bottomMargin: taskDragArea.spacingInPractice
        Drag.active: taskDragArea.held
        Drag.source: taskDragArea
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        Drag.keys: ["task"]

        onYChanged: {
            if (taskDragArea.currentTargetTaskDraggedInto && taskDragArea.currentTargetTaskDraggedInto.makingRoomAnimationFinished) {
                let taskContentY = taskContent.mapToItem(taskDragArea.rootContainer, 0, 0).y;
                let currentTargetTaskDraggedIntoY = taskDragArea.currentTargetTaskDraggedInto.mapToItem(taskDragArea.rootContainer, 0, 0).y;
                if (taskContentY + taskContent.height/2 > currentTargetTaskDraggedIntoY + taskDragArea.currentTargetTaskDraggedInto.height/2) {
                    taskDragArea.shouldHideRoot = true;
                    taskDragArea.currentTargetTaskDraggedInto.makeRoomDirection = "up";
                    taskDragArea.currentTargetTaskDraggedInto.makeRoomForTask = true;
                } else {
                    taskDragArea.shouldHideRoot = true;
                    taskDragArea.currentTargetTaskDraggedInto.makeRoomDirection = "down";
                    taskDragArea.currentTargetTaskDraggedInto.makeRoomForTask = true;
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
                    if (taskDragArea.currentTargetColumnDraggedInto.tasksViewPointer.contentY !== 0) {
                        taskDragArea.currentTargetColumnDraggedInto.tasksScrollingDirection = -1;
                    }
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
                    taskDragArea.rootTodoContainer.isItemScrollingColumnsView = true;
                } else if (taskContentX + taskContent.width/2 + taskDragArea.rootTodoContainer.scrollEdgeSize > todosColumnsViewPointerFromColumn.width) {
                    taskDragArea.rootTodoContainer.scrollingDirection = 1;
                    taskDragArea.rootTodoContainer.isItemScrollingColumnsView = true;
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
                        } else {
                            taskDragArea.currentTargetColumnDraggedInto = null;
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
                id: anchorAnimationMakingRoon
                duration: 300;
                easing.type: Easing.OutCirc;
            }

            onRunningChanged: {
                if (running) {
                    taskDragArea.makingRoomAnimationFinished = false;
                } else {
                    taskDragArea.makingRoomAnimationFinished = true;
                }
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
            }
            ,State {
                when: taskDragArea.makingRoomAnimationFinished

                AnchorChanges {
                    target: taskContent
                    anchors { top: taskDragArea.top; bottom: undefined}
                }
            }]

        Row {
            id: taskTextContentContainer
            property int rightLeftMargins: 12
            width: taskContent.width - rightLeftMargins*2
            y: taskContent.height/2 - (taskTextEdit.visible ? taskTextEdit.height : taskTextNonEditable.height) /2

            CheckBoxMaterial {
                id: taskCheckBox
                checked: taskDragArea.taskChecked
                anchors.verticalCenter: taskTextEdit.visible ? taskTextEdit.verticalCenter : taskTextNonEditable.verticalCenter
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

            Item {
                visible: taskTextNonEditable.visible
                width: taskTextContentContainer.rightLeftMargins
                height: 1
            }

            Text {
                id: taskTextNonEditable
                width: taskTextContentContainer.width - taskCheckBox.width - trashButton.width/2 - taskTextContentContainer.rightLeftMargins
                text: taskDragArea.taskText
                font.family: taskDragArea.rootContainer.bodyFontFamily
                font.pointSize: taskDragArea.rootContainer.platform === "Apple" ? 13 : 10 // TODO: Why setting pixel size causes lines to be rendered differently?
                textFormat: TextEdit.MarkdownText
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

                MouseArea {
                    anchors.fill: parent
                    drag.target: taskDragArea.held ? taskContent : undefined
                    drag.axis: Drag.XAndYAxis
                    pressAndHoldInterval: 200
                    hoverEnabled: true

                    onReleased: {
                        taskDragArea.onReleasedTask();
                    }

                    onPressAndHold: {
                        taskContent.forceActiveFocus(); // For some reason this is needed for taskContent to update its height when taskTextEdit's onEditFinished is called
                        taskDragArea.held = true;
                        cursorShape = Qt.ClosedHandCursor
                    }

                    onDoubleClicked: {
                        if (!taskDragArea.rootContainer.isReadOnlyMode) {
                            taskTextNonEditable.visible = false;
                            taskTextEdit.visible = true;
                            taskTextEdit.cursorPosition = taskTextEdit.length;
                            taskTextEdit.forceActiveFocus();
                        }
                    }

                    onEntered: {
                        cursorShape = Qt.OpenHandCursor
                    }

                    onExited: {
                        if (!taskDragArea.held) {
                            cursorShape = Qt.ArrowCursor;
                        }
                    }
                }
            }

            CustomTextArea {
                id: taskTextEdit
                width: taskTextContentContainer.width - taskCheckBox.width - trashButton.width/2
                visible: false
                text: taskDragArea.taskText
                font.family: taskDragArea.rootContainer.bodyFontFamily
                font.pointSize: taskDragArea.rootContainer.platform === "Apple" ? 13 : 10 // TODO: Why setting pixel size causes lines to be rendered differently?
                themeData: taskDragArea.themeData
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
                selectionColor: taskDragArea.accentColor
                selectedTextColor: "white"
                accentColor: taskDragArea.accentColor
                readOnly: taskDragArea.rootContainer.isReadOnlyMode

                onReturnPressed: {
                    if (!taskDragArea.rootContainer.isReadOnlyMode) {
                        if (!taskTextEdit.isHoldingShift) {
                            var textToSend = taskTextEdit.text;
                            taskDragArea.taskText = taskTextEdit.text;
                            taskDragArea.rootContainer.updateTaskText(taskDragArea.taskStartLine, taskDragArea.taskEndLine, textToSend);
                            taskTextEdit.visible = false;
                            taskTextNonEditable.visible = true;
                        } else {
                            taskTextEdit.insert(taskTextEdit.cursorPosition, '\n');
                        }
                    }
                }

                Keys.onEscapePressed: {
                    taskTextEdit.text = taskTextNonEditable.text;
                    taskTextEdit.visible = false;
                    taskTextNonEditable.visible = true;
                }
            }

            IconButton {
                id: cancelEditingButton
                visible: taskTextEdit.visible
                themeData: taskDragArea.themeData
                anchors.verticalCenter: taskTextEdit.visible ? taskTextEdit.verticalCenter : taskTextNonEditable.verticalCenter
                platform: taskDragArea.rootContainer.platform
                icon: fontIconLoader.icons.fa_circle_xmark
                iconPointSizeOffset: taskDragArea.rootContainer.platform === "Apple" ? -4 : -5
                width: 32
                height: 34

                onClicked: {
                    taskTextEdit.text = taskTextNonEditable.text;
                    taskTextEdit.visible = false;
                    taskTextNonEditable.visible = true;
                }
            }

            IconButton {
                id: trashButton
                visible: taskDragArea.isMouseEnteredTaskContent && !taskDragArea.held && !taskDragArea.isDeletingTask && !taskTextEdit.visible
                themeData: taskDragArea.themeData
                anchors.verticalCenter: taskTextEdit.visible ? taskTextEdit.verticalCenter : taskTextNonEditable.verticalCenter
                platform: taskDragArea.rootContainer.platform
                icon: fontIconLoader.icons.fa_trash
                iconPointSizeOffset: taskDragArea.rootContainer.platform === "Apple" ? -4 : -5
                width: 28
                height: 34

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

           if (drag.source.previouslyEnteredTask !== taskDragArea) {
               if(drag.source.previouslyEnteredTask !== null) {
                   drag.source.previouslyEnteredTask.makeRoomForTask = true;
                   drag.source.previouslyEnteredTask.shouldRestoreOriginalSize = true;
               }

               drag.source.previouslyEnteredTask = taskDragArea;
           }

           drag.source.currentTargetTaskDraggedInto = taskDragArea;
           // TODO: Animate this for a smoother experience
           taskDragArea.height = taskDragArea.taskContentHeight + drag.source.originalHeight + taskDragArea.spacing;
           let taskContentY = drag.source.taskContentAlias.mapToItem(taskDragArea.rootContainer, 0, 0).y;
           let currentTargetTaskDraggedIntoY = taskDragArea.mapToItem(taskDragArea.rootContainer, 0, 0).y;

           if (drag.source.currentlyEnteredColumn !== taskDragArea.listViewParent) {
               drag.source.shouldHideRoot = true;
               taskDragArea.makeRoomDirection = "down";
               taskDragArea.makeRoomForTask = true;
               drag.source.currentlyEnteredColumn = taskDragArea.listViewParent;
           } else if (taskContentY + drag.source.taskContentHeight/2 < currentTargetTaskDraggedIntoY + taskContent.height/2) {
               if (taskDragArea.makeRoomDirection !== "up") {
                   drag.source.shouldHideRoot = true;
                   taskDragArea.makeRoomDirection = "up";
                   taskDragArea.makeRoomForTask = true;
                   drag.source.currentlyEnteredColumn = taskDragArea.listViewParent;
               } else if (taskDragArea.makeRoomDirection !== "down") {
                   drag.source.shouldHideRoot = true;
                   taskDragArea.makeRoomDirection = "down";
                   taskDragArea.makeRoomForTask = true;
                   drag.source.currentlyEnteredColumn = taskDragArea.listViewParent;
               }
           }
       }

        onExited: {
            taskDragArea.isAnotherTaskEntered = false;
            if (drag.source.held) {
                taskDragArea.shouldRestoreOriginalSize = true;
            } else {
                taskDragArea.height = Qt.binding(function () { return taskDragArea.originalHeight });
                taskDragArea.makeRoomForTask = false;
                taskDragArea.makingRoomAnimationFinished = true;
            }

            drag.source.currentTargetTaskDraggedInto = null;
        }
    }
}

