import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Particles 2.12
import QtQuick.Layouts 2.12
import nuttyartist.notes 1.0

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
    property string displayFontFamily: "Roboto"
    property bool showColumnsBorders: false
    property int newTaskInColumnID: -1
    property var taskModelByColumnIDDict: ({})
    property string platform: ""
    property bool showEditorSettings: false
    property int pointSizeOffset: -4
    property bool alignKanbanToMiddleOfRoot: false
    property bool isMultipleNotesSelected: false
    property bool isProVersion: false
    property bool enableConfetti: false

    property real parentWindowHeight
    property real parentWindowWidth
    property real parentWindowX
    property real parentWindowY

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
                                                                                                               "doNeedAnimateTaskCreation": false});
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

                                                                 var animateNoteCreation = false;
                                                                 if (root.newTaskInColumnID === colID && taskIndex === tasksLength-1) {
                                                                     root.newTaskInColumnID = -1;
                                                                     animateNoteCreation = true;
                                                                 }


                                                                 root.taskModelByColumnIDDict[colID].set(taskIndex, {  "taskText": taskData.text, "doNeedAnimateTaskCreation": animateNoteCreation ? true : false,
                                                                                                             "taskChecked": taskData.checked,
                                                                                                             "taskStartLine": taskData.taskStartLine,
                                                                                                             "taskEndLine": taskData.taskEndLine});

                                                                 taskIndex++;

                                                             });
                }
            }
        }
    }

    FontIconLoader {
        id: fontIconLoader
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

            if (root.isMultipleNotesSelected && root.isForcedReadOnly) {
                root.informationPopupText = qsTr("Since you selected multiple notes you're in Read-only mode.");
                root.showInformationPopup = true;
            }
        }

        function onClearKanbanModel () {
            columnModel.clear();
            root.showEditorSettings = false;
        }

        function onResetKanbanSettings () {
            root.areTasksReversed = false;
        }

        function onCheckMultipleNotesSelected (isMultipleNotesSelected) {
            root.isMultipleNotesSelected = isMultipleNotesSelected;
            if (isMultipleNotesSelected) {
                root.isReadOnlyMode = true;
                root.isForcedReadOnly = true;
            } else if (root.isProVersion) {
                root.isReadOnlyMode = false;
                root.isForcedReadOnly = false;
            }
        }
    }

    Connections {
        target: mainWindow

        function onMainWindowResized (data) {
           root.parentWindowHeight = data.parentWindowHeight;
           root.parentWindowWidth = data.parentWindowWidth;
        }

        function onMainWindowMoved (data) {
            root.parentWindowX = data.parentWindowX;
            root.parentWindowY = data.parentWindowY;
        }

        function onThemeChanged (data) {
            root.themeData = data;
        }

        function onPlatformSet (data) {
            root.platform = data;
        }

        function onFontsChanged (data) {
            root.bodyFontFamily = data.listOfSansSerifFonts[data.chosenSansSerifFontIndex];
        }

        function onDisplayFontSet (data) {
            root.displayFontFamily = data.displayFont;
        }

        function onToggleEditorSettingsKeyboardShorcutFired () {
            root.showEditorSettings = !root.showEditorSettings;
            mainWindow.setEditorSettingsFromQuickViewVisibility(root.showEditorSettings);
        }

        function onProVersionCheck (data) {
            root.setProVersion(data);

        }
    }

    function setProVersion (isPro) {
        root.isProVersion = isPro;
        if (!root.isProVersion) {
            root.isReadOnlyMode = true;
            root.isForcedReadOnly = true;
        } else {
            root.isReadOnlyMode = false;
            root.isForcedReadOnly = false;
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

    function addNewTask (endLinePosition: int, columnID: int, newTaskText: string) {
        root.totalTasks++;
        root.newTaskInColumnID = columnID;
        root.taskModelByColumnIDDict[columnID].insert(root.taskModelByColumnIDDict[columnID].count, {"taskText": newTaskText, "taskStartLine:": endLinePosition, "taskEndLine": endLinePosition, "taskChecked": false, "doNeedAnimateTaskCreation": true})
        noteEditorLogic.addNewTask(endLinePosition, newTaskText);
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
        root.taskModelByColumnIDDict[columnID].insert(taskIndex, taskObject);
    }

    SubscriptionWindow {
        id: proPaymentWindow
        visible: false
        platform: root.platform
        color: root.themeData.theme === "Dark" ? "#1e1e1e" : "white"
        displayFontFamily: root.displayFontFamily
        themeData: root.themeData
        x: root.parentWindowX + root.parentWindowWidth/2 - proPaymentWindow.width/2
        y: root.parentWindowY + root.parentWindowHeight/2 - proPaymentWindow.height/2
        forceSubscriptionStatus: true
        subscriptionStatus: SubscriptionStatus.NoSubscription
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
            font.pointSize: root.platform === "Apple" ? 23 : 23 + root.pointSizeOffset
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            font.family: root.bodyFontFamily
            textFormat: Text.RichText
        }

        Item {
            id: todosContainer
            width: root.alignKanbanToMiddleOfRoot && todosColumnsView.contentWidth < root.width - root.marginsSize ? todosColumnsView.contentWidth + root.marginsSize : root.width - root.marginsSize
            height: root.height - y
            anchors.top: parent.top
            anchors.topMargin: toolBarRow.height
            anchors.leftMargin: root.alignKanbanToMiddleOfRoot && todosColumnsView.contentWidth < root.width - root.marginsSize ? 0 : root.marginsSize
            property int scrollEdgeSize: 50
            property int scrollingDirection: 0
            property bool isItemScrollingColumnsView: false

            states:
            [State {
                when: root.alignKanbanToMiddleOfRoot && todosColumnsView.contentWidth < root.width - root.marginsSize
                AnchorChanges {
                    target: todosContainer
                    anchors { left: undefined; horizontalCenter: appBackgroundContainer.horizontalCenter }
                }
            }
            ,State {
                when: !root.alignKanbanToMiddleOfRoot || todosColumnsView.contentWidth >= root.width - root.marginsSize
                AnchorChanges {
                    target: todosContainer
                    anchors { left: appBackgroundContainer.left; horizontalCenter: undefined }
                }
            }
            ]

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
                spacing: root.showColumnsBorders ? 25 : 5
                cacheBuffer: todosContainer.isItemScrollingColumnsView ? 100000000 : 1000
                // This must be very large so while we drag a task to different column that's far,
                // the task won't be destroyed by the window/virtual rendering

                ScrollBar.horizontal: CustomHorizontalScrollBar {
                    themeData: root.themeData
                }
            }
        }

        RowLayout {
            id: toolBarRow
            width: parent.width - root.marginsSize*4
            anchors.left: parent.left
            anchors.leftMargin: root.marginsSize*2
            spacing: 15

            IconButton {
                icon: fontIconLoader.icons.mt_article
                themeData: root.themeData
                themeColor: root.themeData.theme === "Dark" ? "#5b94f5" : "#55534E"
                platform: root.platform
                iconPointSize: root.platform === "Apple" ? 24 : 18
                iconFontFamily: fontIconLoader.mt_symbols

                onClicked: {
                    mainWindow.setKanbanVisibility(false);
                }
            }

            IconButton {
                icon: fontIconLoader.icons.mt_view_kanban
                themeData: root.themeData
                themeColor: root.themeData.theme === "Dark" ? "#5b94f5" : "#55534E"
                platform: root.platform
                iconPointSize: root.platform === "Apple" ? 24 : 18
                iconFontFamily: fontIconLoader.mt_symbols
                enabled: false
            }

            Item {
                Layout.fillWidth: true
            }

            TextButton {
                visible: !root.isProVersion
                text: qsTr("Upgrade to Pro")
                platform: root.platform
                displayFontFamily: root.bodyFontFamily
                textAlignment: TextButton.TextAlign.Middle
                themeData: root.themeData
                textFontPointSize: 13
                defaultBackgroundColor: root.themeData.theme === "Dark" ? "#27272e" : "#ebecf8"
                highlightBackgroundColor: root.themeData.theme === "Dark" ? "#313131" : "#e4e5f0"
                mainFontColorDefault: root.themeData.theme === "Dark" ? "#2383e2" : "#007bff"
                textFontWeight: Font.Bold
                backgroundOpacity: root.themeData.theme === "Dark" ? 0.5 : 1.0
                backgroundHeightOffset: 17

                onClicked: {
                    proPaymentWindow.width = 650;
                    proPaymentWindow.height = 400;
                    proPaymentWindow.visible = true
                    proPaymentWindow.raise();
                }
            }

            IconButton {
                id: settingsPlusButton
                icon: fontIconLoader.icons.fa_plus
                themeData: root.themeData
                themeColor: root.themeData.theme === "Dark" ? "#5b94f5" : "#55534E"
                platform: root.platform

                onClicked: {
                    root.addNewColumn();
                }
            }

            Text {
                id: progressBarText
                visible: root.showProgressBar
                color: root.themeData.theme === "Dark" ? "white" : "black"
                text: root.totalCompletedTasks.toString() + " of " + root.totalTasks.toString() + " tasks"
                font.family: root.bodyFontFamily
                font.pointSize: root.platform === "Apple" ? 13 : 13 + root.pointSizeOffset
            }

            ProgressBar {
                id: progressBar
                visible: root.showProgressBar
                width: 150
                value: root.totalCompletedTasks / root.totalTasks
            }

            IconButton {
                id: settingsDotsButton
                themeData: root.themeData
                icon: fontIconLoader.icons.fa_sliders
                platform: root.platform
                iconPointSizeOffset: -4

                onClicked: {
                    if (root.showSettingsPopup) {
                        closeSettignsPopupAnimation.start();
                    } else {
                        root.showSettingsPopup = true;
                    }
                }

                Item {
                    id: settingsPopupContainer
                    visible: root.showSettingsPopup
                    anchors.top: settingsDotsButton.bottom
                    anchors.right: settingsDotsButton.right
                    width: settingsPopup.width
                    height: settingsPopup.height

                    Popup {
                        id: settingsPopup
                        visible: root.showSettingsPopup
                        width:  230
                        height: settingsPopupColumn.height + 22
                        Material.elevation: 8
                        Material.background: root.themeData.theme === "Dark" ? "#252525" : "white"
                        //                        Material.roundedScale: Material.SmallScale
                        property int rowWidth: width - settingsPopup.padding*2
                        padding: 10

                        Column {
                            id: settingsPopupColumn

                            Row {
                                OptionItemButton {
                                    contentWidth: settingsPopup.rowWidth
                                    displayText: qsTr("Reverse tasks' order")
                                    displayFontFamily: root.bodyFontFamily
                                    platform: root.platform
                                    themeData: root.themeData
                                    checked: root.areTasksReversed

                                    onSwitched: {
                                        root.toggleReverseTasks();
                                    }

                                    onUnswitched: {
                                        root.toggleReverseTasks();
                                    }
                                }
                            }

                            Row {
                                OptionItemButton {
                                    contentWidth: settingsPopup.rowWidth
                                    displayText: qsTr("Show progress bar")
                                    displayFontFamily: root.bodyFontFamily
                                    platform: root.platform
                                    themeData: root.themeData
                                    checked: root.showProgressBar

                                    onSwitched: {
                                        root.showProgressBar = !root.showProgressBar;
                                    }

                                    onUnswitched: {
                                        root.showProgressBar = !root.showProgressBar;
                                    }
                                }
                            }

                            Row {
                                OptionItemButton {
                                    contentWidth: settingsPopup.rowWidth
                                    displayText: qsTr("Read-only mode")
                                    displayFontFamily: root.bodyFontFamily
                                    platform: root.platform
                                    themeData: root.themeData
                                    checked: root.isForcedReadOnly || root.isReadOnlyMode
                                    enabled: !root.isForcedReadOnly

                                    onSwitched: {
                                        if (!root.isForcedReadOnly) {
                                            root.isReadOnlyMode = !root.isReadOnlyMode;
                                        }
                                    }

                                    onUnswitched: {
                                        if (!root.isForcedReadOnly) {
                                            root.isReadOnlyMode = !root.isReadOnlyMode;
                                        }
                                    }
                                }
                            }

                            Row {
                                OptionItemButton {
                                    contentWidth: settingsPopup.rowWidth
                                    displayText: qsTr("Show column border")
                                    displayFontFamily: root.bodyFontFamily
                                    platform: root.platform
                                    themeData: root.themeData
                                    checked: root.showColumnsBorders

                                    onSwitched: {
                                        root.showColumnsBorders = !root.showColumnsBorders;
                                    }

                                    onUnswitched: {
                                        root.showColumnsBorders = !root.showColumnsBorders;
                                    }
                                }
                            }

                            Row {
                                OptionItemButton {
                                    contentWidth: settingsPopup.rowWidth
                                    displayText: qsTr("Align to middle")
                                    displayFontFamily: root.bodyFontFamily
                                    platform: root.platform
                                    themeData: root.themeData
                                    checked: root.alignKanbanToMiddleOfRoot

                                    onSwitched: {
                                        root.alignKanbanToMiddleOfRoot = !root.alignKanbanToMiddleOfRoot;
                                    }

                                    onUnswitched: {
                                        root.alignKanbanToMiddleOfRoot = !root.alignKanbanToMiddleOfRoot;
                                    }
                                }
                            }

                            Row {
                                OptionItemButton {
                                    contentWidth: settingsPopup.rowWidth
                                    displayText: qsTr("Enable Confetti")
                                    displayFontFamily: root.bodyFontFamily
                                    platform: root.platform
                                    themeData: root.themeData
                                    checked: root.enableConfetti

                                    onSwitched: {
                                        root.enableConfetti = !root.enableConfetti;
                                    }

                                    onUnswitched: {
                                        root.enableConfetti = !root.enableConfetti;
                                    }
                                }
                            }
                        }
                    }

                    PropertyAnimation {
                        target: settingsPopupContainer
                        running: settingsPopupContainer.visible
                        property: "opacity"
                        from: 0.0
                        to: 1.0
                        duration: 300
                        easing.type: Easing.OutExpo
                    }

                    PropertyAnimation {
                        id: closeSettignsPopupAnimation
                        target: settingsPopupContainer
                        running: !settingsPopupContainer.visible
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

            IconButton {
                id: editorSettingsPopupButton
                themeData: root.themeData
                icon: fontIconLoader.icons.fa_ellipsis_h
                platform: root.platform

                Item {
                    id: editorSettingsPopupContainer
                    visible: root.showEditorSettings
                    anchors.top: editorSettingsPopupButton.bottom
                    anchors.right: editorSettingsPopupButton.right
                    width: editorSettings.width
                    height: editorSettings.height
                    Popup {
                        visible: root.showEditorSettings
                        width: editorSettings.width
                        height: editorSettings.height
                        padding: 0
                        background: Rectangle {
                            color: "transparent"
                        }

                        EditorSettings {
                            id: editorSettings
                            visible: root.showEditorSettings
                            anchors.centerIn: parent
                            extraWidthForQWidgets: 0
                            extraHeightForQWidgets: 0

                            onVisibleChanged: {
                                if (editorSettingsPopupContainer.visible) {
                                    editorSettings.upadteScrollBarPosition();
                                }
                            }
                        }

                        onClosed: {
                            root.showEditorSettings = false;
                            mainWindow.setEditorSettingsFromQuickViewVisibility(root.showEditorSettings);
                        }
                    }
                }

                onClicked: {
                    root.showEditorSettings = !root.showEditorSettings
                    mainWindow.setEditorSettingsFromQuickViewVisibility(root.showEditorSettings);
                }
            }
        }
    }

    Timer {
        id: informationPopupTimer
        interval: 2500
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
//        Material.roundedScale: Material.SmallScale

        Text {
            id: informationPopupText
            text: root.informationPopupText
            anchors.centerIn: parent
            color: root.themeData.theme === "Dark" ? "white" : "black"
            font.family: root.bodyFontFamily
            font.weight: Font.Bold
            font.pointSize: root.platform === "Apple" ? 16 : 16 + root.pointSizeOffset
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
