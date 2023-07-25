import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import nuttyartist.notes 1.0
//import notes

Item {
    id: settingsContainer
    property int extraWidthForQWidgets: platform === "Apple" ? 60 : 0
    property int extraHeightForQWidgets: platform === "Apple" ? 40 : 0
    width: settingsPane.width + extraWidthForQWidgets
    height: settingsPane.height + extraHeightForQWidgets
    property var themeData: {{theme: "Light"}}
    property string displayFontFamily: "Roboto"
    property string platform: ""
    property string categoriesFontColor: settingsContainer.themeData.theme === "Dark"  ? "#868686" : "#7d7c78"
    property string mainFontColor: settingsContainer.themeData.theme === "Dark" ? "#d6d6d6" : "#37352e"
    property string highlightBackgroundColor: settingsContainer.themeData.theme === "Dark" ? "#313131" : "#efefef"
    property string pressedBackgroundColor: settingsContainer.themeData.theme === "Dark" ? "#2c2c2c" : "#dfdfde"
    property string separatorLineColors: settingsContainer.themeData.theme === "Dark" ? "#3a3a3a" : "#ededec"
    property string mainBackgroundColor: settingsContainer.themeData.theme === "Dark" ? "#252525" : "white"
    signal themeChanged
    property int paddingRightLeft: 12
    property var listOfSansSerifFonts: []
    property var listOfSerifFonts: []
    property var listOfMonoFonts: []
    property int chosenSansSerifFontIndex: 0
    property int chosenSerifFontIndex: 0
    property int chosenMonoFontIndex: 0
    property string currentFontTypeface
    property real latestScrollBarPosition: 0.0
    property int pointSizeOffset: -4
    property int qtVersion: 6
    property string currentlySelectedView: "TextView" // "KanbanView"

//    signal changeFontType(fontType : EditorSettingsOptions) // TODO: It's better to use signal & slots for calling C++ functions
//    to change the editor settings rather than calling public slots directly. But I couldn't make it work between QML and C++
//    (QObject::connect: Error: No such signal EditorSettings_QMLTYPE_45::changeFontType(int))
//    So, currently following this:
//    https://scythe-studio.com/en/blog/how-to-integrate-c-and-qml-registering-enums

    Connections {
        target: mainWindow

        function onEditorSettingsShowed (data) {
            var settingsPaneHeightByParentWindow = 0.80 * data.parentWindowHeight; // 80 percent of the parent window's height
            settingsPane.height = scrollViewControl.contentHeight > settingsPaneHeightByParentWindow ? settingsPaneHeightByParentWindow : scrollViewControl.contentHeight;
            revealSettingsAnimation.start();
            settingsContainer.upadteScrollBarPosition();
        }

        function onMainWindowResized (data) {
            var settingsPaneHeightByParentWindow = 0.80 * data.parentWindowHeight; // 80 percent of the parent window's height
            settingsPane.height = scrollViewControl.contentHeight > settingsPaneHeightByParentWindow ? settingsPaneHeightByParentWindow : scrollViewControl.contentHeight;
        }

        function onDisplayFontSet (data) {
            settingsContainer.displayFontFamily = data.displayFont;
        }

        function onPlatformSet (data) {
            settingsContainer.platform = data;
        }

        function onQtVersionSet (data) {
            settingsContainer.qtVersion = data;
        }

        function onThemeChanged (data) {
            settingsContainer.themeData = data;
            themeChanged();
        }

        function onEditorSettingsScrollBarPositionChanged (data) {
            settingsContainer.latestScrollBarPosition = data;
        }

        function onSettingsChanged (data) {
            if (data.currentFontTypeface === "SansSerif") {
                fontChooserSans.checked = true;
                fontChooserSerif.checked = false;
                fontChooserMono.checked = false;
            } else if (data.currentFontTypeface === "Serif") {
                fontChooserSerif.checked = true;
                fontChooserSans.checked = false;
                fontChooserMono.checked = false;
            } else if (data.currentFontTypeface === "Mono") {
                fontChooserMono.checked = true;
                fontChooserSans.checked = false;
                fontChooserSerif.checked = false;
            }

            if (data.currentTheme === "Light") {
                lightThemeChooserButton.themeSelected(true);
                darkThemeChooserButton.themeSelected(false);
                sepiaThemeChooserButton.themeSelected(false);
            } else if (data.currentTheme === "Dark") {
                darkThemeChooserButton.themeSelected(true);
                sepiaThemeChooserButton.themeSelected(false);
                lightThemeChooserButton.themeSelected(false);
            } else if (data.currentTheme === "Sepia") {
                sepiaThemeChooserButton.themeSelected(true);
                darkThemeChooserButton.themeSelected(false);
                lightThemeChooserButton.themeSelected(false);
            }

            focusModeOption.setOptionSelected(!data.isTextFullWidth);

            if (data.currentView === "TextView") {
                settingsContainer.currentlySelectedView = "TextView";
                textView.viewSelected();
            } else if (data.currentView === "KanbanView") {
                settingsContainer.currentlySelectedView = "KanbanView";
                kanbanView.viewSelected();
            }

            notesListCollapsedOption.setOptionSelected(!data.isNoteListCollapsed);
            foldersTreeCollapsedOption.setOptionSelected(!data.isFoldersTreeCollapsed);
            markdownEnabledOption.setOptionSelected(!data.isMarkdownDisabled);
            stayOnTopOption.setOptionSelected(data.isStayOnTop);
        }

        function onFontsChanged (data) {
            settingsContainer.listOfSansSerifFonts = data.listOfSansSerifFonts;
            settingsContainer.listOfSerifFonts = data.listOfSerifFonts;
            settingsContainer.listOfMonoFonts = data.listOfMonoFonts;
            settingsContainer.chosenSansSerifFontIndex = data.chosenSansSerifFontIndex;
            settingsContainer.chosenSerifFontIndex = data.chosenSerifFontIndex;
            settingsContainer.chosenMonoFontIndex = data.chosenMonoFontIndex;
            settingsContainer.currentFontTypeface = data.currentFontTypeface;
        }
    }

    Connections {
        target: noteEditorLogic

        function onTextShown () {
            textView.viewSelected();
            kanbanView.viewUnclicked();
        }

        function onKanbanShown () {
            kanbanView.viewSelected();
            textView.viewUnclicked();
        }
    }

    FontIconLoader {
        id: fontIconLoader
    }

    PropertyAnimation {
        id: revealSettingsAnimation
        target: settingsContainer
        property: "opacity"
        from: 0.3
        to: 1.0
        duration: 100
        easing.type: Easing.InOutQuad
    }

    function upadteScrollBarPosition () {
        editorSettingsVerticalScrollBar.position = settingsContainer.latestScrollBarPosition;
    }

    Pane {
        id: settingsPane
        visible: true
        padding: 0
        width: 240
        height: 500
        contentWidth: 240
        Material.elevation: 10
        Material.background: settingsContainer.mainBackgroundColor
//        Material.roundedScale: Material.MediumScale
        x: settingsContainer.extraWidthForQWidgets === 0 ? 0 : 20
        y: settingsContainer.extraHeightForQWidgets === 0 ? 0 : 20

        Rectangle {
            visible: settingsContainer.qtVersion < 6
            anchors.fill: parent
            color: settingsContainer.mainBackgroundColor
        }

        ScrollView {
            id: scrollViewControl
            anchors.fill: parent
            clip: true

            ScrollBar.vertical: CustomVerticalScrollBar {
                id: editorSettingsVerticalScrollBar
                themeData: settingsContainer.themeData
                isDarkGray: false
                showBackground: true

                onPositionChanged: {
                    mainWindow.setEditorSettingsScrollBarPosition(editorSettingsVerticalScrollBar.position);

                }
            }

            Column {
                Item {
                    width: 1
                    height: settingsContainer.paddingRightLeft
                }

                Text {
                    text: qsTr("Style")
                    color: settingsContainer.categoriesFontColor
                    font.pointSize: settingsContainer.platform === "Apple" ? 12 : 12 + settingsContainer.pointSizeOffset
                    font.family: settingsContainer.displayFontFamily
                    x: settingsContainer.paddingRightLeft
                }

                Item {
                    width: 1
                    height: 5
                }

                Row {
                    x: settingsContainer.paddingRightLeft
                    FontChooserButton {
                        id: fontChooserSans
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData
                        categoriesFontColor: settingsContainer.categoriesFontColor
                        fontTypeface: "Sans"
                        fontsModel: settingsContainer.listOfSansSerifFonts
                        currentlyChosenFontIndex: settingsContainer.chosenSansSerifFontIndex
                        qtVersion: settingsContainer.qtVersion
                        mainBackgroundColor: settingsContainer.mainBackgroundColor

                        onClicked: (chosenFontIndex) => {
                            if (chosenFontIndex === -1) {
                                if (settingsContainer.currentFontTypeface === "SansSerif") {
                                    chosenFontIndex = currentlyChosenFontIndex < fontsModel.length - 1 ? currentlyChosenFontIndex + 1 : 0;
                                } else {
                                   chosenFontIndex = currentlyChosenFontIndex;
                                }
                            }
                            mainWindow.changeEditorFontTypeFromStyleButtons(FontTypeface.SansSerif, chosenFontIndex);
                            fontChooserSerif.checked = false;
                            fontChooserMono.checked = false;
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                fontChooserSans.themeChanged();
                            }
                        }
                    }

                    FontChooserButton {
                        id: fontChooserSerif
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData
                        categoriesFontColor: settingsContainer.categoriesFontColor
                        fontTypeface: "Serif"
                        fontsModel: settingsContainer.listOfSerifFonts
                        currentlyChosenFontIndex: settingsContainer.chosenSerifFontIndex
                        qtVersion: settingsContainer.qtVersion
                        mainBackgroundColor: settingsContainer.mainBackgroundColor
                        enabled: settingsContainer.currentlySelectedView === "TextView"

                        onClicked: (chosenFontIndex) => {
                           if (chosenFontIndex === -1) {
                               if (settingsContainer.currentFontTypeface === "Serif") {
                                   chosenFontIndex = currentlyChosenFontIndex < fontsModel.length - 1 ? currentlyChosenFontIndex + 1 : 0;
                               } else {
                                  chosenFontIndex = currentlyChosenFontIndex;
                               }
                           }
                            mainWindow.changeEditorFontTypeFromStyleButtons(FontTypeface.Serif, chosenFontIndex);
                            fontChooserSans.checked = false;
                            fontChooserMono.checked = false;
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                fontChooserSerif.themeChanged();
                            }
                        }
                    }

                    FontChooserButton {
                        id: fontChooserMono
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData
                        categoriesFontColor: settingsContainer.categoriesFontColor
                        fontTypeface: "Mono"
                        fontsModel: settingsContainer.listOfMonoFonts
                        currentlyChosenFontIndex: settingsContainer.chosenMonoFontIndex
                        qtVersion: settingsContainer.qtVersion
                        mainBackgroundColor: settingsContainer.mainBackgroundColor
                        enabled: settingsContainer.currentlySelectedView === "TextView"

                        onClicked: (chosenFontIndex) => {
                           if (chosenFontIndex === -1) {
                               if (settingsContainer.currentFontTypeface === "Mono") {
                                   chosenFontIndex = currentlyChosenFontIndex < fontsModel.length - 1 ? currentlyChosenFontIndex + 1 : 0;
                               } else {
                                  chosenFontIndex = currentlyChosenFontIndex;
                               }
                           }
                            mainWindow.changeEditorFontTypeFromStyleButtons(FontTypeface.Mono, chosenFontIndex);
                            fontChooserSans.checked = false;
                            fontChooserSerif.checked = false;
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                fontChooserMono.themeChanged();
                            }
                        }
                    }
                }

                Item {
                    width: 1
                    height: 10
                }

                Rectangle {
                    height: 1
                    width: settingsPane.contentWidth
                    color: settingsContainer.separatorLineColors
                }

                Column {
                    id: textSettingsGroup
                    visible: settingsContainer.currentlySelectedView === "TextView"

                    Item {
                        width: 1
                        height: 10
                    }

                    Text {
                        text: qsTr("Text")
                        color: settingsContainer.categoriesFontColor
                        font.pointSize: settingsContainer.platform === "Apple" ? 12 : 12 + settingsContainer.pointSizeOffset
                        font.family: settingsContainer.displayFontFamily
                        x: settingsContainer.paddingRightLeft
                    }

                    Item {
                        width: 1
                        height: 10
                    }

                    // Font size
                    Row {
                        id: fontSizeRow
                        x: settingsContainer.paddingRightLeft + 10
                        Text {
                            id: fontSizeText
                            x: 20
                            text: qsTr("Font size")
                            color: settingsContainer.mainFontColor
                            font.pointSize: settingsContainer.platform === "Apple" ? 14 : 14 + settingsContainer.pointSizeOffset
                            font.family: settingsContainer.displayFontFamily
                            opacity: settingsContainer.currentlySelectedView === "TextView" ? 1.0 : 0.2
                        }

                        Item {
                            width: settingsPane.contentWidth - fontSizePlusButton.width - fontSizeText.width - fontSizeMinusButton.width - 20 - fontSizeRow.x
                            height: 1
                        }

                        IconButton {
                            id: fontSizeMinusButton
                            icon: fontIconLoader.icons.fa_minus
                            anchors.verticalCenter: fontSizeText.verticalCenter
                            themeData: settingsContainer.themeData
                            platform: settingsContainer.platform
                            iconPointSizeOffset: -4
                            enabled: settingsContainer.currentlySelectedView === "TextView"

                            onClicked: {
                                mainWindow.changeEditorFontSizeFromStyleButtons(FontSizeAction.FontSizeDecrease);
                            }
                        }

                        Item {
                            width: 10
                            height: 1
                        }

                        IconButton {
                            id: fontSizePlusButton
                            icon: fontIconLoader.icons.fa_plus
                            anchors.verticalCenter: fontSizeText.verticalCenter
                            themeData: settingsContainer.themeData
                            platform: settingsContainer.platform
                            iconPointSizeOffset: -4
                            enabled: settingsContainer.currentlySelectedView === "TextView"

                            onClicked: {
                                mainWindow.changeEditorFontSizeFromStyleButtons(FontSizeAction.FontSizeIncrease);
                            }
                        }
                    }

                    // Full width
                    OptionItemButton {
                        id: focusModeOption
                        x: settingsContainer.paddingRightLeft
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Focus mode")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData
                        enabled: settingsContainer.currentlySelectedView === "TextView"

                        onSwitched: {
                            mainWindow.changeEditorTextWidthFromStyleButtons(EditorTextWidth.TextWidthFullWidth)
                        }

                        onUnswitched: {
                            mainWindow.changeEditorTextWidthFromStyleButtons(EditorTextWidth.TextWidthFullWidth)
                        }
                    }

                    Item {
                        visible: settingsContainer.currentlySelectedView === "TextView" && focusModeOption.checked
                        width: 1
                        height: 10
                    }

                    // Text width
                    Row {
                        id: textWidthRow
                        x: settingsContainer.paddingRightLeft + 10
                        visible: settingsContainer.currentlySelectedView === "TextView" && focusModeOption.checked
                        Text {
                            id: textWidthText
                            text: qsTr("Text width")
                            color: settingsContainer.mainFontColor
                            font.pointSize: settingsContainer.platform === "Apple" ? 14 : 14 + settingsContainer.pointSizeOffset
                            font.family: settingsContainer.displayFontFamily
                            opacity: settingsContainer.currentlySelectedView === "TextView" ? 1.0 : 0.2
                        }

                        Item {
                            width: settingsPane.contentWidth - textWidthPlusButton.width - textWidthText.width - textWidthMinusButton.width - 20 - textWidthRow.x
                            height: 1
                        }

                        IconButton {
                            id: textWidthMinusButton
                            icon: fontIconLoader.icons.fa_minus
                            anchors.verticalCenter: textWidthText.verticalCenter
                            themeData: settingsContainer.themeData
                            platform: settingsContainer.platform
                            iconPointSizeOffset: -4
                            enabled: settingsContainer.currentlySelectedView === "TextView"

                            onClicked: {
                                mainWindow.changeEditorTextWidthFromStyleButtons(EditorTextWidth.TextWidthDecrease)
                            }
                        }

                        Item {
                            width: 10
                            height: 1
                        }

                        IconButton {
                            id: textWidthPlusButton
                            icon: fontIconLoader.icons.fa_plus
                            anchors.verticalCenter: textWidthText.verticalCenter
                            themeData: settingsContainer.themeData
                            platform: settingsContainer.platform
                            iconPointSizeOffset: -4
                            enabled: settingsContainer.currentlySelectedView === "TextView"

                            onClicked: {
                                mainWindow.changeEditorTextWidthFromStyleButtons(EditorTextWidth.TextWidthIncrease)
                            }
                        }
                    }

                    Item {
                        width: 1
                        height: 7
                    }

                    Rectangle {
                        height: 1
                        width: settingsPane.contentWidth
                        color: settingsContainer.separatorLineColors
                    }
                }



                Item {
                    width: 1
                    height: 10
                }

                Text {
                    text: qsTr("Theme")
                    x: settingsContainer.paddingRightLeft
                    color: settingsContainer.categoriesFontColor
                    font.pointSize: settingsContainer.platform === "Apple" ? 12 : 12 + settingsContainer.pointSizeOffset
                    font.family: settingsContainer.displayFontFamily
                }

                Item {
                    width: 1
                    height: 5
                }

                // Theme buttons
                Row {
                    x: settingsContainer.paddingRightLeft
                    ThemeChooserButton {
                        id: lightThemeChooserButton
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeColor: "#f7f7f7"
                        themeName: "Light"
                        themeData: settingsContainer.themeData
                        qtVersion: settingsContainer.qtVersion

                        onClicked: {
                            mainWindow.setTheme(Theme.Light);
                            darkThemeChooserButton.unclicked();
                            sepiaThemeChooserButton.unclicked();
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                lightThemeChooserButton.themeChanged();
                            }
                        }
                    }

                    ThemeChooserButton {
                        id: darkThemeChooserButton
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeColor: "#191919"
                        themeName: "Dark"
                        themeData: settingsContainer.themeData
                        qtVersion: settingsContainer.qtVersion

                        onClicked: {
                            mainWindow.setTheme(Theme.Dark);
                            lightThemeChooserButton.unclicked();
                            sepiaThemeChooserButton.unclicked();
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                darkThemeChooserButton.themeChanged();
                            }
                        }
                    }

                    ThemeChooserButton {
                        id: sepiaThemeChooserButton
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeColor: "#f7cc6f"
                        themeName: "Sepia"
                        themeData: settingsContainer.themeData
                        qtVersion: settingsContainer.qtVersion

                        onClicked: {
                            mainWindow.setTheme(Theme.Sepia);
                            lightThemeChooserButton.unclicked();
                            darkThemeChooserButton.unclicked();
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                sepiaThemeChooserButton.themeChanged();
                            }
                        }
                    }
                }

                Item {
                    width: 1
                    height: 15
                }

                Column {
                    id: viewSettingContainer
                    visible: settingsContainer.qtVersion > 5

                    Rectangle {
                        height: 1
                        width: settingsPane.contentWidth
                        color: settingsContainer.separatorLineColors
                    }

                    Item {
                        width: 1
                        height: 10
                    }

                    Text {
                        text: qsTr("View")
                        x: settingsContainer.paddingRightLeft
                        color: settingsContainer.categoriesFontColor
                        font.pointSize: settingsContainer.platform === "Apple" ? 12 : 12 + settingsContainer.pointSizeOffset
                        font.family: settingsContainer.displayFontFamily
                    }

                    Item {
                        width: 1
                        height: 10
                    }

                    Row {
                        x: settingsContainer.paddingRightLeft
                        ViewChooserButton {
                            id: textView
                            currentViewType: ViewChooserButton.ViewType.Text
                            displayFontFamily: settingsContainer.displayFontFamily
                            platform: settingsContainer.platform
                            mainFontColor: settingsContainer.mainFontColor
                            highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                            pressedBackgroundColor: settingsContainer.pressedBackgroundColor

                            onViewClicked: {
                                mainWindow.setKanbanVisibility(false);
                                kanbanView.viewUnclicked();
                            }

                            Connections {
                                target: settingsContainer

                                function onThemeChanged () {
                                    textView.themeChanged();
                                }
                            }
                        }

                        Column {
                            ViewChooserButton {
                                id: kanbanView
                                currentViewType: ViewChooserButton.ViewType.Kanban
                                displayFontFamily: settingsContainer.displayFontFamily
                                platform: settingsContainer.platform
                                mainFontColor: settingsContainer.mainFontColor
                                highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                                pressedBackgroundColor: settingsContainer.pressedBackgroundColor

                                onViewClicked: {
                                    mainWindow.setKanbanVisibility(true);
                                    textView.viewUnclicked();
                                }

                                Connections {
                                    target: settingsContainer

                                    function onThemeChanged () {
                                        kanbanView.themeChanged();
                                    }
                                }
                            }

                            Rectangle {
    //                            anchors.horizontalCenter: kanbanView.horizontalCenter
                                x: kanbanView.x + kanbanView.width/2 - width/2 - 7
                                width: 35
                                height: 25
                                radius: 5
                                color: settingsContainer.themeData.theme === "Dark" ? "#342a3a" : "#f0f0f0"

                                Text {
                                    anchors.centerIn: parent
                                    text: "PRO"
                                    color: settingsContainer.themeData.theme === "Dark" ? "#a6a6a6" : "#787878"
                                    font.pointSize: settingsContainer.platform === "Apple" ? 13 : 13 + settingsContainer.pointSizeOffset
                                    font.family: settingsContainer.displayFontFamily
                                }
                            }
                        }
                    }

                    Item {
                        width: 1
                        height: 15
                    }
                }



                Rectangle {
                    height: 1
                    width: settingsPane.contentWidth
                    color: settingsContainer.separatorLineColors
                }

                Item {
                    width: 1
                    height: 10
                }

                Text {
                    text: qsTr("Options")
                    x: settingsContainer.paddingRightLeft
                    color: settingsContainer.categoriesFontColor
                    font.pointSize: settingsContainer.platform === "Apple" ? 12 : 12 + settingsContainer.pointSizeOffset
                    font.family: settingsContainer.displayFontFamily
                }

                Item {
                    width: 1
                    height: 10
                }

                Column {
                    x: settingsContainer.paddingRightLeft
                    OptionItemButton {
                        id: notesListCollapsedOption
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Show notes list")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData

                        onSwitched: {
                            mainWindow.expandNoteList();
                        }

                        onUnswitched: {
                            mainWindow.collapseNoteList();
                        }
                    }

                    OptionItemButton {
                        id: foldersTreeCollapsedOption
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Show folders tree")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData

                        onSwitched: {
                            mainWindow.expandFolderTree();
                        }

                        onUnswitched: {
                            mainWindow.collapseFolderTree();
                        }
                    }

                    OptionItemButton {
                        id: markdownEnabledOption
                        visible: settingsContainer.currentlySelectedView === "TextView"
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Markdown enabled")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData
                        enabled: settingsContainer.currentlySelectedView === "TextView"

                        onSwitched: {
                            mainWindow.setMarkdownEnabled(true);
                        }

                        onUnswitched: {
                            mainWindow.setMarkdownEnabled(false);
                        }
                    }

                    OptionItemButton {
                        id: stayOnTopOption
                        visible: settingsContainer.platform === "Apple"
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Always stay on top")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData

                        onSwitched: {
                            mainWindow.stayOnTop(true);
                        }

                        onUnswitched: {
                            mainWindow.stayOnTop(false);
                        }
                    }
                }

                Item {
                    width: 1
                    height: 15
                }

                Rectangle {
                    height: 1
                    width: settingsPane.contentWidth
                    color: settingsContainer.separatorLineColors
                }

                Item {
                    width: 1
                    height: 10
                }

                TextButton {
                    id: deleteNoteButton
                    text: qsTr("Move to Trash")
                    icon: fontIconLoader.icons.fa_trash
                    highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                    pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                    backgroundWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                    mainFontColorDefault: settingsContainer.mainFontColor
                    platform: settingsContainer.platform
                    displayFontFamily: settingsContainer.displayFontFamily
                    x: settingsContainer.paddingRightLeft

                    onClicked: {
                        mainWindow.moveCurrentNoteToTrash();
                    }
                }

                Item {
                    width: 1
                    height: 10
                }

                Rectangle {
                    height: 1
                    width: settingsPane.contentWidth
                    color: settingsContainer.separatorLineColors
                }

                Item {
                    width: 1
                    height: 10
                }

                TextButton {
                    id: resetButton
                    text: qsTr("Reset all settings")
                    icon: fontIconLoader.icons.fa_undo_alt
                    highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                    pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                    backgroundWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                    mainFontColorDefault: settingsContainer.mainFontColor
                    platform: settingsContainer.platform
                    displayFontFamily: settingsContainer.displayFontFamily
                    x: settingsContainer.paddingRightLeft

                    onClicked: {
                        resetEditorSettingsDialog.visible = true
                    }
                }

                Item {
                    width: 1
                    height: 10
                }
            }
        }
    }

    Dialog {
        id: resetEditorSettingsDialog
        visible: false
        title: qsTr("Reset settings?")
        standardButtons: Dialog.Yes | Dialog.Cancel
        Material.accent: "#2383e2";
        Material.theme: settingsContainer.themeData.theme === "Dark" ? Material.Dark : Material.Light
//        Material.roundedScale: Material.SmallScale
        width: settingsPane.contentWidth
        x: settingsContainer.extraWidthForQWidgets === 0 ? 0 : 25
        y: settingsPane.height - height/2 - 100
        font.family: settingsContainer.displayFontFamily

        Text {
            width: settingsPane.contentWidth - 20
            wrapMode: Text.WordWrap
            text: qsTr("Reset all editor settings to their defaults? This will not affect your data, only the app appearance.")
            font.family: settingsContainer.displayFontFamily
            color: settingsContainer.mainFontColor
        }

        onAccepted: {
            mainWindow.resetEditorSettings();
        }
        onRejected: {
        }
        onDiscarded: {
        }
    }
}

