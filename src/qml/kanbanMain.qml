import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Particles

Item {
    id: root
    width: 900
    height: 480
    visible: true

    property int textAndTodosSpacing: 20
    property int todoColumnWidth: 250
    property int marginsSize: 10
    property var titlesAndTasksData
    property var themeData: {"theme": "Light", "backgroundColor": "#f7f7f7"}
    property bool areTasksReversed: false
    property bool showSettingsPopup: false
    property bool showProgressBar: true
    property bool showConfetti1: false
    property bool emitConfetti: false
    property int totalCompletedTasks: 0
    property int totalTasks: 0
    property bool isReadOnlyMode: false
    property bool showInformationPopup: false
    property string informationPopupText: ""
    property bool isForcedReadOnly: false
    property string headerFamilyFont: "PT Serif"
    property string bodyFontFamily: "Avenir Next"
    property bool showColumnsBorders: false
    property int newTaskInColumnID: -1
    property var taskModelByColumnIDDict: ({})
    property string platform: ""

    Material.theme: themeData.theme === "Dark" ? Material.Dark : Material.Light
    Material.accent: "#2383e2";

    function reverseTasksData () {
        for (let i = 0; i < titlesAndTasksData.length; i++) {
            titlesAndTasksData[i].tasks.reverse();
        }
    }

    function toggleReverseTasks () {
        root.reverseTasksData();
        root.areTasksReversed = !root.areTasksReversed;
        columnModel.clear();
        root.updateColumnModel();
    }

    function updateColumnModel() {
        // Add columns and tasks
        if (columnModel.count === 0) {
            root.totalTasks = 0;
            root.totalCompletedTasks = 0;

            for (let i = 0; i < root.titlesAndTasksData.length; i++) {
                root.totalTasks += root.titlesAndTasksData[i].tasks.length;
                columnModel.append({"title": root.titlesAndTasksData[i].title, "columnID": i,
                                       "columnStartLine": root.titlesAndTasksData[i].columnStartLine,
                                       "columnEndLine": root.titlesAndTasksData[i].columnEndLine});
            }

            for (var n = 0; n < taskModelsRepeater.count; n++) {
                root.titlesAndTasksData[n].tasks.forEach(taskData => {

                    if (taskData.checked) {
                         root.totalCompletedTasks++;
                     }

                     taskModelsRepeater.itemAt(n).taskModel.append({"taskText": taskData.text, "taskChecked": taskData.checked,
                                                             "taskStartLine": taskData.taskStartLine,
                                                         "taskEndLine": taskData.taskEndLine,
                                                         "doNeedAnimateFirstNote": false});
                 });
            }
        } else {
            // Update colums and tasks
            for (let k = 0; k < root.titlesAndTasksData.length; k++) {
                if (root.titlesAndTasksData[k]) {
                    columnModel.set(k, {"columnStartLine": root.titlesAndTasksData[k].columnStartLine,
                                        "columnEndLine": root.titlesAndTasksData[k].columnEndLine});

                    var taskIndex = 0;
                    var colID = columnModel.get(k).columnID;
                    var tasksLength = root.titlesAndTasksData[k].tasks.length;

                    root.titlesAndTasksData[k].tasks.forEach(taskData => {

                         var animateFirstNote = false;
                         if (root.newTaskInColumnID === colID && taskIndex === 0) {
                             root.newTaskInColumnID = -1;
                             animateFirstNote = true;
                         }


                         root.taskModelByColumnIDDict[colID].set(taskIndex, {  "taskText": taskData.text, "doNeedAnimateFirstNote": animateFirstNote ? true : false,
                                                   "taskChecked": taskData.checked,
                                                  "taskStartLine": taskData.taskStartLine,
                                              "taskEndLine": taskData.taskEndLine});

                       taskIndex++;

                     });
                }
            }
        }
    }

    ListModel {
        id: columnModel

        function getNewColumnID() {
            var previousLargest = -1;
            for (let i = 0; i < columnModel.count; i++) {
                if (columnModel.get(i).columnID > previousLargest) {
                    previousLargest = columnModel.get(i).columnID;
                }
            }

            return previousLargest + 1;
        }
    }

    Repeater {
        id: taskModelsRepeater
        model: columnModel

        // We replaced this with root.taskModelByColumnIDDict
        function getTaskModelByColumnID (columnID: int) {
            for (let i = 0; i < taskModelsRepeater.count; i++) {
                if (taskModelsRepeater.itemAt(i).columnID === columnID) {
                    return taskModelsRepeater.itemAt(i).taskModel;
                }
            }
        }

        delegate: Item {
            property alias taskModel: taskModelChild
            required property int columnID
            ListModel {
                id: taskModelChild
            }
        }

        onItemAdded: (index, item) => {
            root.taskModelByColumnIDDict[item.columnID] = item.taskModel;
        }

        onItemRemoved: (index, item) => {
            delete root.taskModelByColumnIDDict[item.columID];
        }
    }

    Connections {
        target: noteEditorLogic

        function onTasksFoundInEditor (data) {
            root.titlesAndTasksData = data;
            if (root.areTasksReversed) {
                root.reverseTasksData();
            }
            root.updateColumnModel();
            root.showSettingsPopup = false;

            if (root.isForcedReadOnly) {
                root.informationPopupText = qsTr("Since you select multiple notes you're in Read-only mode.");
                root.showInformationPopup = true;
            }
        }

        function onClearKanbanModel () {
            columnModel.clear();
        }

        function onResetKanbanSettings () {
            root.areTasksReversed = false;
        }

        function onKanbanForceReadOnly () {
            root.isReadOnlyMode = true;
            root.isForcedReadOnly = true;
        }

        function onKanbanFontChanged (data) {
            root.bodyFontFamily = data;
        }
    }

    Connections {
        target: mainWindow

        function onThemeChanged (data) {
            root.themeData = data;
        }

        function onPlatformSet (data) {
            root.platform = data;
        }
    }

    function rearrangeTasks (startLinePosition: int, endLinePosition: int, newLinePosition: int, columnInsertingIntoID, taskIndexToInsert, columnRemovingFromID, taskIndexToRemove, objectToInsert) {
        root.taskModelByColumnIDDict[columnInsertingIntoID].insert(taskIndexToInsert, objectToInsert);

        if (columnInsertingIntoID === columnRemovingFromID && taskIndexToRemove > taskIndexToInsert) {
            root.taskModelByColumnIDDict[columnRemovingFromID].remove(taskIndexToRemove + 1);
        } else {
            root.taskModelByColumnIDDict[columnRemovingFromID].remove(taskIndexToRemove);
        }

        if (root.taskModelByColumnIDDict[columnRemovingFromID].count === 0) {
            for (let i = 0; i < columnModel.count; i++) {
                if (columnModel.get(i).columnID === columnRemovingFromID) {
                    columnModel.remove(i);
                    break;
                }
            }
        }

        noteEditorLogic.rearrangeTasksInTextEditor(startLinePosition, endLinePosition, newLinePosition);
    }

    function rearrangeColumns (startLinePosition: int, endLinePosition: int, newLinePosition: int) {
        noteEditorLogic.rearrangeColumnsInTextEditor(startLinePosition, endLinePosition, newLinePosition);
    }

    function checkTaskInLine (line: int) {
        noteEditorLogic.checkTaskInLine(line);
    }

    function uncheckTaskInLine (line: int) {
        noteEditorLogic.uncheckTaskInLine(line);
    }

    function updateTaskText (startLinePosition: int, endLinePosition: int, newText: string) {
        noteEditorLogic.updateTaskText(startLinePosition, endLinePosition, newText);
    }

    function addNewTask (startLinePosition: int, columnID: int) {
        root.totalTasks++;
        root.newTaskInColumnID = columnID;
        root.taskModelByColumnIDDict[columnID].insert(0, {"taskText": "New task", "taskStartLine:": startLinePosition, "taskEndLine": startLinePosition, "taskChecked": false, "doNeedAnimateFirstNote": true})
        noteEditorLogic.addNewTask(startLinePosition);
    }

    function removeTask (startLinePosition: int, endLinePosition: int, columnRemovingFromID, taskIndex) {
        root.taskModelByColumnIDDict[columnRemovingFromID].remove(taskIndex);

        if (root.taskModelByColumnIDDict[columnRemovingFromID].count === 0) {
            for (let i = 0; i < columnModel.count; i++) {
                if (columnModel.get(i).columnID === columnRemovingFromID) {
                    columnModel.remove(i);
                    break;
                }
            }
        }

        noteEditorLogic.removeTask(startLinePosition, endLinePosition);
    }

    function addNewColumn () {
        if (!root.isReadOnlyMode) {
            var columnTitle = "\n\n# Untitled\n\n- [ ] New task";

            var newColumnID = columnModel.getNewColumnID();

            if (columnModel.count > 0) {
                let startLinePosition = columnModel.get(columnModel.count - 1).columnEndLine;
                columnModel.append({"title": "Untitled", "columnID": newColumnID,
                                       "columnStartLine": startLinePosition,
                                       "columnEndLine": startLinePosition+2});
                noteEditorLogic.addNewColumn(startLinePosition, columnTitle);
            } else {
                let columnTitle = "# Untitled\n\n- [ ] New task\n\n";
                columnModel.append({"title": "Untitled", "columnID": newColumnID,
                                       "columnStartLine": 0,
                                       "columnEndLine": 2});
                noteEditorLogic.addNewColumn(0, columnTitle);
            }

            root.totalTasks++;
            todosColumnsView.positionViewAtEnd();
        }

        if (root.isReadOnlyMode) {
            root.informationPopupText = qsTr("Can't edit while in Read-only mode.");
            root.showInformationPopup = true;
        }
    }

    function removeColumn (startLinePosition: int, endLinePosition: int, columnRemovingID: int) {
        var taskModelOfColumn = root.taskModelByColumnIDDict[columnRemovingID];

        root.totalTasks -= taskModelOfColumn.count;

        for (let k = 0; k < taskModelOfColumn.count; k++) {
            if (taskModelOfColumn.get(k).taskChecked)
                root.totalCompletedTasks--;
        }

        for (let i = 0; i < columnModel.count; i++) {
            if (columnModel.get(i).columnID === columnRemovingID) {
                columnModel.remove(i);
                break;
            }
        }

        noteEditorLogic.removeColumn(startLinePosition, endLinePosition);
    }

    function updateColumnTitle (lineNumber: int, newText: string) {
        noteEditorLogic.updateColumnTitle(lineNumber, newText);
    }

    function reRenderTask(columnID, taskIndex, taskObject) {
        root.taskModelByColumnIDDict[columnID].remove(taskIndex);
//        taskObject.taskText = "# Something\nYo";
//        console.log(taskObject.taskText);
        root.taskModelByColumnIDDict[columnID].insert(taskIndex, taskObject);
    }

    Rectangle {
        id: appBackgroundContainer
        anchors.fill: parent
        color: root.themeData.backgroundColor

        Text {
            id: noTasksFoundText
            visible: columnModel.count === 0
            text: "No tasks found in this note.<br/>Create a new column using the <b>+ button</b>."
            color: root.themeData.theme === "Dark" ? "white" : "black"
            font.pointSize: root.platform === "Apple" ? 23 : 21
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            font.family: root.bodyFontFamily
            textFormat: Text.RichText
        }

        Item {
            id: todosContainer
            x: root.marginsSize
            width: root.width - root.marginsSize
            height: root.height - y
            anchors.top: parent.top
            anchors.topMargin: toolBarRow.height
            property int scrollEdgeSize: 50
            property int scrollingDirection: 0

            SmoothedAnimation {
                id: columnsLeftAnimation
                target: todosColumnsView
                property: "contentX"
                to: 0
                running: todosContainer.scrollingDirection === -1
                duration: 800 * (todosColumnsView.contentWidth / todosContainer.width)
            }

            SmoothedAnimation {
                id: columnsRightAnimation
                target: todosColumnsView
                property: "contentX"
                to: todosColumnsView.contentWidth - todosColumnsView.width
                running: todosContainer.scrollingDirection === 1
                duration: 800 * (todosColumnsView.contentWidth / todosContainer.width)
            }


            DelegateModel {
                id: visualModel
                model: columnModel

                delegate: TodoColumnDelegate {
                    todoColumnContentHeight: todosContainer.height - root.marginsSize*3
                    rootContainer: root
                    rootTodoContainer: todosContainer
                    todosColumnsViewPointerFromColumn: todosColumnsView
                    titlesAndTasksData: root.titlesAndTasksData
                    themeData: root.themeData
                    areTasksReversed: root.areTasksReversed
                    columnModelPointer: columnModel
                    taskModel: root.taskModelByColumnIDDict[columnID]
                }
            }

            ListView {
                id: todosColumnsView
                model: visualModel
                anchors { fill: parent }
                anchors.rightMargin: root.marginsSize
                clip: true
                orientation: ListView.Horizontal
                spacing: 25
                cacheBuffer: 1000000  // This must be very large so while we drag a task to different column that's far,
                // the task won't be destroyed by the window/virtual rendering
            }
        }

        Row {
            id: toolBarRow
            visible: true
            anchors.right: parent.right
            anchors.rightMargin: root.marginsSize
            spacing: 15

            MouseArea {
                width: appBackgroundContainer.width
                height: toolBarRow.height

                onClicked: {
                    root.showSettingsPopup = false;
                }
            }

            Text {
                id: progressBarText
                visible: root.showProgressBar
                color: root.themeData.theme === "Dark" ? "white" : "black"
                text: root.totalCompletedTasks.toString() + " of " + root.totalTasks.toString() + " tasks"
                anchors.verticalCenter: settingsPlusButton.verticalCenter
                font.family: root.bodyFontFamily
                font.pointSize: root.platform === "Apple" ? 13 : 11
            }

            ProgressBar {
                id: progressBar
                visible: root.showProgressBar
                width: root.width/2 - progressBarText.width
                anchors.verticalCenter: settingsPlusButton.verticalCenter
                value: root.totalCompletedTasks / root.totalTasks
            }

            PlusButton {
                id: settingsPlusButton
                themeData: root.themeData
                themeColor: root.themeData.theme === "Dark" ? "#5b94f5" : "#55534E"

                onClicked: {
                    root.addNewColumn();
                }
            }

            DotsButton {
                id: settingsDotsButton
                themeData: root.themeData
                onClicked: {
                    if (root.showSettingsPopup) {
                        closeSettignsPopupAnimation.start();
                    } else {
                        root.showSettingsPopup = true;
                    }
                }

                Pane {
                    id: settingsPopup
                    visible: root.showSettingsPopup
                    Material.elevation: 5
                    Material.background: root.themeData.theme === "Dark" ? "#252525" : "white"
                    anchors.top: settingsDotsButton.bottom
                    anchors.right: settingsDotsButton.right
                    width:  210
                    height: settingsPopupColumn.height + 15
                    property int rowWidth: width - 15
//                     TODO: Find a way to make Pane's with rounded corners without Meterial's drop shadow disappearing
//                    background: Rectangle {
//                        color: root.themeData.theme === "Dark" ? "#252525" : "white"
//                        radius: 5
//                    }

                    Column {
                        id: settingsPopupColumn

                        Row {
                            id: reverseActionRow

                            Text {
                                id: reverseActionText
                                text: qsTr("Reverse tasks' order");
                                color: root.themeData.theme === "Dark" ? "white" : "black"
                                font.family: root.bodyFontFamily
                                font.weight: Font.Normal
                                font.pointSize: root.platform === "Apple" ? 13 : 11
                            }

                            Item {
                                // Spacer
                                height: 1
                                width: settingsPopup.rowWidth - reverseActionText.width - reverseActionSwitch.width
                            }

                            SwitchButton {
                                id: reverseActionSwitch
                                anchors.verticalCenter: reverseActionText.verticalCenter
                                checked: root.areTasksReversed
                                themeData: root.themeData

                                onSwitched: {
                                    root.toggleReverseTasks();
                                }
                            }

                        }

                        Row {
                            id: progressBarVisibilityActionRow

                            Text {
                                id: progressBarVisibilityActionText
                                text: qsTr("Show progress bar");
                                color: root.themeData.theme === "Dark" ? "white" : "black"
                                font.family: root.bodyFontFamily
                                font.weight: Font.Normal
                                font.pointSize: root.platform === "Apple" ? 13 : 11
                            }

                            Item {
                                // Spacer
                                height: 1
                                width: settingsPopup.rowWidth - progressBarVisibilityActionText.width - progressBarVisibilityActionSwitch.width
                            }

                            SwitchButton {
                                id: progressBarVisibilityActionSwitch
                                anchors.verticalCenter: progressBarVisibilityActionText.verticalCenter
                                checked: root.showProgressBar
                                themeData: root.themeData

                                onSwitched: {
                                    root.showProgressBar = !root.showProgressBar;
                                }
                            }

                        }

                        Row {
                            id: readOnlyModeActionRow

                            Text {
                                id: readOnlyModeActionText
                                text: qsTr("Read-only mode");
                                color: root.themeData.theme === "Dark" ? "white" : "black"
                                font.family: root.bodyFontFamily
                                font.weight: Font.Normal
                                font.pointSize: root.platform === "Apple" ? 13 : 11
                            }

                            Item {
                                // Spacer
                                height: 1
                                width: settingsPopup.rowWidth - readOnlyModeActionText.width - readOnlyModeActionSwitch.width
                            }

                            SwitchButton {
                                id: readOnlyModeActionSwitch
                                anchors.verticalCenter: readOnlyModeActionText.verticalCenter
                                checked: root.isForcedReadOnly || root.isReadOnlyMode
                                themeData: root.themeData
                                enabled: !root.isForcedReadOnly

                                onSwitched: {
                                    if (!root.isForcedReadOnly) {
                                        root.isReadOnlyMode = !root.isReadOnlyMode;
                                    }
                                }
                            }

                        }

                        Row {
                            id: showColumnsBordersActionRow

                            Text {
                                id: showColumnsBordersActionText
                                text: qsTr("Show column border");
                                color: root.themeData.theme === "Dark" ? "white" : "black"
                                font.family: root.bodyFontFamily
                                font.weight: Font.Normal
                                font.pointSize: root.platform === "Apple" ? 13 : 11
                            }

                            Item {
                                // Spacer
                                height: 1
                                width: settingsPopup.rowWidth - showColumnsBordersActionText.width - showColumnsBordersActionSwitch.width
                            }

                            SwitchButton {
                                id: showColumnsBordersActionSwitch
                                anchors.verticalCenter: showColumnsBordersActionText.verticalCenter
                                checked: root.showColumnsBorders
                                themeData: root.themeData

                                onSwitched: {
                                    root.showColumnsBorders = !root.showColumnsBorders;
                                }
                            }

                        }
                    }

                    PropertyAnimation {
                        target: settingsPopup
                        running: settingsPopup.visible
                        property: "opacity"
                        from: 0.0
                        to: 1.0
                        duration: 300
                        easing.type: Easing.OutExpo
                    }

                    PropertyAnimation {
                        id: closeSettignsPopupAnimation
                        target: settingsPopup
                        running: !settingsPopup.visible
                        property: "opacity"
                        to: 0.0
                        duration: 300
                        easing.type: Easing.OutExpo

                        onFinished: {
                            root.showSettingsPopup = false;
                        }
                    }
                }
            }
        }
    }

    Timer {
        id: informationPopupTimer
        interval: 1500
        running: root.showInformationPopup

        onTriggered: {
            informationPopupExitAnimation.start();
        }
    }

    PropertyAnimation {
        id: informationPopupEnterAnimation
        target: informationPopup
        running: root.showInformationPopup
        property: "y"
        from: root.y - informationPopup.height
        to: toolBarRow.y + toolBarRow.height
        duration: 600
        easing.type: Easing.OutExpo
    }

    PropertyAnimation {
        id: informationPopupExitAnimation
        target: informationPopup
        property: "y"
        to: root.y - informationPopup.height
        duration: 500
        easing.type: Easing.InExpo

        onFinished: {
            root.showInformationPopup = false;
        }
    }

    Pane {
        id: informationPopup
        visible: root.showInformationPopup
        Material.elevation: 15
        Material.background: root.themeData.theme === "Dark" ? "#252525" : "white"
        x: root.width/2 - informationPopup.width / 2
        width:  informationPopupText.width + 50
        height: informationPopupText.height + 40
//         TODO: Find a way to make Pane's with rounded corners without Meterial's drop shadow disappearing
//                    background: Rectangle {
//                        color: root.themeData.theme === "Dark" ? "#252525" : "white"
//                        radius: 5
//                    }


        Text {
            id: informationPopupText
            text: root.informationPopupText
            anchors.centerIn: parent
            color: root.themeData.theme === "Dark" ? "white" : "black"
            font.family: root.bodyFontFamily
            font.weight: Font.Bold
            font.pointSize: root.platform === "Apple" ? 16 : 14
        }
    }

    ParticleSystem {
        id: particleSystem
        anchors.fill: parent
        visible: root.showConfetti1
        running: root.showConfetti1
    }

    ImageParticle {
        system: particleSystem
        visible: root.showConfetti1
        source: "qrc:/images/confetti.png"
        colorVariation: 1.0
        //            sizeVariation: 0.5
//        rotation: -90
//        rotationVelocity: 180
//        rotationVariation: 180
        //            opacityVariation: 0.5
    }

    Emitter {
        id: confettiEmitter
        visible: root.showConfetti1
        anchors.top: parent.top
        width: root.width
        system: particleSystem
        emitRate: root.emitConfetti ? 300 : 0 // 1000
        lifeSpan: 1500
        size: 12
        // size: 10
//        size: 50
//        x: root.x
//        y: root.y
        velocity: AngleDirection {
            angle: 90
//            angleVariation: 360
            magnitude: 400
            magnitudeVariation: 100
        }

    }

    Timer {
        id: stopConfettiEmittingTimer
        interval: 1500
        running: root.emitConfetti

        onTriggered: {
            root.emitConfetti = false;
        }
    }

    Timer {
        id: stopConfettiTimer
        interval: 3000
        running: root.showConfetti1

        onTriggered: {
            root.showConfetti1 = false;
        }
    }
}
