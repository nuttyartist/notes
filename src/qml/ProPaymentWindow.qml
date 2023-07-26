import QtQuick 2.12
import QtQuick.Layouts 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import "Utilities.js" as Utils

Window {
    id: root
    width: 650
    height: 400
    property string platform: "Other"
    property string displayFontFamily
    property string accentColor: "#0a84ff"
    property int pointSizeOffset: platform === "Apple" ? 0: -3
    property var themeData
    property int columnHeight: 280
    property bool canActivate: licenseTextField.length === 36 && !licenseTextField.isPlaceHolderTextShown

    property string getPurchaseDataAlt1: "https://raw.githubusercontent.com/nuttyartist/notes/master/notes_purchase_data.json"
    property string getPurchaseDataAlt2: "https://www.rubymamistvalove.com/notes/notes_purchase_data.json"

    property string checkoutURLDefault: "https://notesapp.lemonsqueezy.com/checkout/buy/76150e98-2c13-4367-b784-01bb4105c3fd?discount=0"
    property string purchaseApiBaseUrlDefault: "https://api.lemonsqueezy.com"
    property string activateLicenseEndpointDefault: "/v1/licenses/activate"
    property string validateLicenseEndpointDefault: "/v1/licenses/validate"
    property string deactivateLicenseEndpointDefault: "/v1/licenses/deactivate"

    property bool isLoadingPurchseLink: false
    property bool isActivatingLicense: false

    property bool showLicenseErrorMessage: false
    property string licenseErrorMessageText: ""

    signal subscribedSuccessfully

    FontIconLoader {
        id: fontIconLoader
    }

    function getData (BASE, cb) {
        Utils.request('GET', BASE, null, null, cb);
    }

    function postData (BASE, endpoint, entry, cb) {
        Utils.request('POST', BASE, endpoint, entry, cb);
    }

    function openPurchaseLink () {
        root.isLoadingPurchseLink = true;
        root.getData(root.getPurchaseDataAlt1, function(response) {
            if (response.error !== null) {
                console.log("Failed first attempt at getting data. Trying second...");

                root.getData(root.getPurchaseDataAlt2, function(response) {
                    if (response.error !== null) {
                        console.log("Failed second attempt at getting data. Using embedded link...");
                        root.isLoadingPurchseLink = false;
                        Qt.openUrlExternally(root.checkoutURLDefault);
                    } else {
                        root.isLoadingPurchseLink = false;
                        Qt.openUrlExternally(response.purchase_pro_url);
                    }
                });
            } else {
                root.isLoadingPurchseLink = false;
                Qt.openUrlExternally(response.purchase_pro_url);
            }
        });
    }

    function activateLicenseHelper (purchaseApiBaseUrl, activateLicenseEndpoint, req) {
        root.postData(purchaseApiBaseUrl, activateLicenseEndpoint, req, function (response) {
            if (response.error !== null) {
                root.isActivatingLicense = false;
                if (response.error === "error") {
                    root.licenseErrorMessageText = "Error occurred. Check your internet connection.";
                } else {
                    root.licenseErrorMessageText = response.error;
                }
                root.showLicenseErrorMessage = true;
            } else {
                mainWindow.setActivationSuccessful();
                root.isActivatingLicense = false;
                activationPopup.close();
                activationPopup.visible = false;
                successActivationPopup.visible = true;
            }
        });
    }

    function activateLicense () {
        root.showLicenseErrorMessage = false;
        root.isActivatingLicense = true;
        var purchaseApiBase;
        var activateLicenseEndpoint;
        var req = {license_key: licenseTextField.text, instance_name: "Notes_Pro"};

        root.getData(root.getPurchaseDataAlt1, function(response) {
            if (response.error !== null) {
                console.log("Failed first attempt at getting data. Trying second...");
                root.getData(root.getPurchaseDataAlt2, function(response) {
                    if (response.error !== null) {
                        console.log("Failed second attempt at getting data. Using embedded link...");
                        root.activateLicenseHelper(root.purchaseApiBaseUrlDefault, root.activateLicenseEndpointDefault, req);
                    } else {
                        purchaseApiBase = response.purchaseApiBase;
                        activateLicenseEndpoint = response.activateLicenseEndpoint;
                        root.activateLicenseHelper(purchaseApiBase, activateLicenseEndpoint, req);
                    }
                });
            } else {
                purchaseApiBase = response.purchaseApiBase;
                activateLicenseEndpoint = response.activateLicenseEndpoint;
                root.activateLicenseHelper(purchaseApiBase, activateLicenseEndpoint, req);
            }
        });
    }

    Popup {
        id: successActivationPopup
        anchors.centerIn: parent
        visible: false
        width: root.width
        height: root.height
        Material.background: root.themeData.theme === "Dark" ? "#252525" : "white"
        Material.elevation: 100
        padding: 0

        Column {
            anchors.centerIn: parent
            spacing: 0

            Text {
                id: circleCheckIcon
                anchors.horizontalCenter: parent.horizontalCenter
                text: fontIconLoader.icons.fa_circle_check
                font.family: fontIconLoader.fontAwesomeRegular.name
                color: "#2dd272"
                font.pointSize: 66 + root.pointSizeOffset
            }

            Item {
                width: 1
                height: 30
            }

            Text {
                text: qsTr("Successfully Activated Notes Pro")
                anchors.horizontalCenter: parent.horizontalCenter
                color: root.themeData.theme === "Dark" ? "#dfdedf" : "#272727"
                font.family: root.displayFontFamily
                font.pointSize: 26 + root.pointSizeOffset
                font.weight: Font.Black
            }

            Item {
                width: 1
                height: 30
            }

            Text {
                text: qsTr("Thank you for purchasing Notes Pro and supporting open source development. You'll continue to get free Pro updates for a year. Enjoy!")
                width: successActivationPopup.width*2/3
                anchors.horizontalCenter: parent.horizontalCenter
                color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
                font.family: root.displayFontFamily
                font.pointSize: 15 + root.pointSizeOffset
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Item {
                width: 1
                height: 50
            }

            TextButton {
                id: continueButton
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Continue to Notes Pro")
                height: 35
                platform: root.platform
                displayFontFamily: root.displayFontFamily
                textAlignment: TextButton.TextAlign.Middle
                themeData: root.themeData
                textFontPointSize: 14
                defaultBackgroundColor: "#0e70d2"
                highlightBackgroundColor: root.themeData.theme === "Dark" ? "#1a324b" : defaultBackgroundColor
                pressedBackgroundColor: highlightBackgroundColor
                mainFontColorDefault: "white"
                mainFontColorPressed: root.themeData.theme === "Dark" ? "#c6ccd2" : "#edebeb"
                textFontWeight: Font.Bold
                backgroundOpacity: continueButton.entered && root.themeData.theme !== "Dark " ? 0.5 : 1.0
                Layout.alignment: Qt.AlignHCenter
                pointSizeOffset: root.platform === "Apple" ? 0 : -3

                onClicked: {
                    root.close();
                    root.visible = false;
                }
            }
        }
    }

    Popup {
        id: activationPopup
        anchors.centerIn: parent
        visible: false
        width: root.width*5/6
        height: root.height*5/6
        Material.background: root.themeData.theme === "Dark" ? "#252525" : "white"
        Material.elevation: 100
        padding: 0

        MouseArea {
            anchors.fill: parent

            onPressed: {
                activationPopup.forceActiveFocus();
            }
        }

        Column {
            anchors.fill: parent
            spacing: 0
            topPadding: 20

            Text {
                text: qsTr("Activate Notes Pro")
                anchors.horizontalCenter: parent.horizontalCenter
                color: root.themeData.theme === "Dark" ? "#dfdedf" : "#272727"
                font.family: root.displayFontFamily
                font.pointSize: 26 + root.pointSizeOffset
                font.weight: Font.Black
            }

            Item {
                width: 1
                height: 40
            }

            Text {
                text: qsTr("Fill in your license key to unlock Notes Pro")
                anchors.horizontalCenter: parent.horizontalCenter
                color: root.themeData.theme === "Dark" ? "#dfdedf" : "#272727"
                font.family: root.displayFontFamily
                font.pointSize: 13 + root.pointSizeOffset
                font.weight: Font.Bold
            }

            Item {
                width: 1
                height: 10
            }

            CustomTextField {
                id: licenseTextField
                placeholderText: "ABCDEFGH-IJKLMNOP-QRSTUVWX-YZ123456"
                placeholderTextColor: root.themeData.theme === "Dark" ? "#4c454f" : "#c4c4c4"
                anchors.horizontalCenter: parent.horizontalCenter
                color: originalTextColor
                width: 450
                height: 30
                background: Rectangle {
                    border.width: licenseTextField.focus ? 2 : 0
                    border.color: "#2383e2"
                    radius: 5
                    color: root.themeData.theme === "Dark" ? "#313131" : "#efefef"
                }
                horizontalAlignment: Text.AlignHCenter
                property bool isPlaceHolderTextShown: false
                property string originalTextColor: root.themeData.theme === "Dark" ? "white" : "black"

                onAccepted: {
                    if (root.canActivate) {
                        root.activateLicense();
                    }
                }

                onFocusChanged: {
                    if (focus) {
                        if (!licenseTextField.isPlaceHolderTextShown && licenseTextField.placeholderText !== undefined && licenseTextField.placeholderText !== "" && licenseTextField.text === "") {
                            licenseTextField.text = licenseTextField.placeholderText;
                            licenseTextField.color = Qt.binding(function () { return licenseTextField.placeholderTextColor });
                            licenseTextField.cursorPosition = licenseTextField.length/2;
                            licenseTextField.isPlaceHolderTextShown = true;
                        }
                    }
                }

                onTextChanged: {
                    root.showLicenseErrorMessage = false;
                    if (!licenseTextField.isPlaceHolderTextShown && licenseTextField.placeholderText !== undefined && licenseTextField.placeholderText !== "" && licenseTextField.text === "") {
                        licenseTextField.text = licenseTextField.placeholderText;
                        licenseTextField.color = Qt.binding(function () { return licenseTextField.placeholderTextColor });
                        licenseTextField.cursorPosition = licenseTextField.length/2;
                        licenseTextField.isPlaceHolderTextShown = true;
                    } else {
                        if (licenseTextField.isPlaceHolderTextShown) {
                            licenseTextField.color =  Qt.binding(function () { return licenseTextField.originalTextColor });
                            licenseTextField.isPlaceHolderTextShown = false;
                            var middleFirst = licenseTextField.placeholderText.substring(0, licenseTextField.placeholderText.length/2);
                            var middleSecond = licenseTextField.placeholderText.substring(licenseTextField.placeholderText.length/2, licenseTextField.placeholderText.length);
                            var textToReplace = licenseTextField.text;
                            textToReplace = textToReplace.replace(middleFirst, "");
                            textToReplace = textToReplace.replace(middleSecond, "");
                            licenseTextField.text = textToReplace;
                            if (licenseTextField.text.length === 36) {
                                root.activateLicense();
                            }
                        }
                    }
                }
            }

            Item {
                width: 1
                height: 10
            }

            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: root.isActivatingLicense
                running: root.isActivatingLicense
                Material.theme: root.themeData.theme === "Dark" ? Material.Dark : Material.Light
                Material.accent: "#5b94f5"
                Layout.alignment: Qt.AlignHCenter
                implicitHeight: 50
                implicitWidth: 50
            }

            Text {
                id: licenseErrorMessageText
                visible: root.showLicenseErrorMessage
                text: root.licenseErrorMessageText
                width: implicitWidth <  licenseTextField.width ? implicitWidth : licenseTextField.width
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#d93e37"
                font.family: root.displayFontFamily
                font.pointSize: 13 + root.pointSizeOffset
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
            }

            Item {
                visible: root.isActivatingLicense || root.showLicenseErrorMessage
                width: 1
                height: 10
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 10

                TextButton {
                    text: qsTr("Cancel")
                    platform: root.platform
                    displayFontFamily: root.displayFontFamily
                    textAlignment: TextButton.TextAlign.Middle
                    themeData: root.themeData
                    textFontPointSize: 14
                    textFontWeight: Font.Normal
                    backgroundWidthOffset: 22
                    backgroundHeightOffset: 14
                    pointSizeOffset: root.platform === "Apple" ? 0 : -3

                    onClicked: {
                        activationPopup.close();
                    }
                }

                TextButton {
                    text: qsTr("Activate")
                    platform: root.platform
                    displayFontFamily: root.displayFontFamily
                    textAlignment: TextButton.TextAlign.Middle
                    themeData: root.themeData
                    textFontPointSize: 14
                    textFontWeight: Font.Normal
                    backgroundWidthOffset: 22
                    backgroundHeightOffset: 14
                    defaultBackgroundColor: "transparent"
                    highlightBackgroundColor: root.canActivate ? root.themeData.theme === "Dark" ? "#313131" : "#efefef" : "transparent"
                    pressedBackgroundColor: root.canActivate ? root.themeData.theme === "Dark" ? "#2c2c2c" : "#dfdfde" : "transparent"
                    usePointingHand: root.canActivate
                    mainFontColorDefault: root.canActivate ? root.themeData.theme === "Dark" ? "#0a84ff" : "#007bff" : root.themeData.theme === "Dark" ? "#1b579c" : "#79b9ff"
                    pointSizeOffset: root.platform === "Apple" ? 0 : -3

                    onClicked:  {
                        if (root.canActivate) {
                            root.activateLicense();
                        }
                    }
                }
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            text: 'No license yet? &nbsp; <a href="https://github.com/nuttyartist/notes?utm_source=awesome_app" style="color:' + (root.themeData.theme === "Dark" ? '#2383e2' : '#007bff') + '; text-decoration: none;">Buy now</a>'
            width: 185
            color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
            font.family: root.displayFontFamily
            font.pointSize: 13 + root.pointSizeOffset
            font.weight: Font.Bold
            wrapMode: Text.WordWrap
            onLinkActivated: root.openPurchaseLink();
            textFormat: TextEdit.RichText

            MouseArea {
                // TODO: How to make cursor change only when hovering upon the link??
                anchors.fill: parent
                acceptedButtons: Qt.NoButton // Don't eat the mouse clicks
                cursorShape: Qt.PointingHandCursor
            }
        }
    }

    ColumnLayout {
        id: contentContainer

        Item {
            width: 1
            height: 30
        }

        Row {
            Rectangle {
                width: root.width/2
                height: root.columnHeight
                color: "transparent"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 40

                    Row {
                        spacing: 10

                        Text {
                            id: pointIcon1
                            text: fontIconLoader.icons.mt_view_kanban
                            font.family: fontIconLoader.materialSymbols.name
                            color: root.accentColor
                            font.pointSize: 31 + root.pointSizeOffset
                        }

                        Text {
                            anchors.verticalCenter: pointIcon1.verticalCenter
                            text: qsTr("Unlock editing within the Kanban view")
                            width: root.width/2 - pointIcon1.width - 40
                            color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
                            font.family: root.displayFontFamily
                            font.pointSize: 15 + root.pointSizeOffset
                            font.weight: Font.Bold
                            wrapMode: Text.WordWrap
                        }
                    }

                    Row {
                        spacing: 10

                        Text {
                            id: pointIcon2
                            text: fontIconLoader.icons.fa_bell
                            font.family: fontIconLoader.fontAwesomeSolid.name
                            color: root.accentColor
                            font.pointSize: 31 + root.pointSizeOffset
                        }

                        Text {
                            anchors.verticalCenter: pointIcon2.verticalCenter
                            text: qsTr("Enjoy a year's worth of free Pro updates")
                            width: root.width/2 - pointIcon1.width - 40
                            color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
                            font.family: root.displayFontFamily
                            font.pointSize: 15 + root.pointSizeOffset
                            font.weight: Font.Bold
                            wrapMode: Text.WordWrap
                        }
                    }

                    Row {
                        spacing: 10

                        Text {
                            id: pointIcon3
                            text: fontIconLoader.icons.fa_linux
                            font.family: fontIconLoader.fontAwesomeBrands.name
                            color: root.accentColor
                            font.pointSize: 31 + root.pointSizeOffset
                        }

                        Text {
                            anchors.verticalCenter: pointIcon3.verticalCenter
                            text: qsTr("Help foster the growth of open-source development")
                            width: root.width/2 - pointIcon1.width - 40
                            color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
                            font.family: root.displayFontFamily
                            font.pointSize: 15 + root.pointSizeOffset
                            font.weight: Font.Bold
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }

            Rectangle {
                width: root.width/2
                height: root.columnHeight
                color: "transparent"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 0

                    Rectangle {
                        border.width: 3
                        border.color: "gray"
                        width: 128
                        height: 128
                        radius: 28
                        Layout.alignment: Qt.AlignHCenter

                        Image {
                            source: "qrc:/images/notes_system_tray_icon.png"
                            anchors.fill: parent
                        }
                    }

                    Item {
                        width: 1
                        height: 22
                    }

                    Text {
                        text: qsTr("Upgrade to")
                        color: root.themeData.theme === "Dark" ? "#b6b6b6" : "#525252"
                        font.family: root.displayFontFamily
                        Layout.margins: 0
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: qsTr("Notes Pro")
                        color: root.accentColor
                        font.family: root.displayFontFamily
                        font.pointSize: 21 + root.pointSizeOffset
                        font.weight: Font.Bold
                        Layout.margins: 0
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Item {
                        width: 1
                        height: 10
                    }

                    BusyIndicator {
                        visible: root.isLoadingPurchseLink
                        running: root.isLoadingPurchseLink
                        Material.theme: root.themeData.theme === "Dark" ? Material.Dark : Material.Light
                        Material.accent: "#5b94f5"
                        Layout.alignment: Qt.AlignHCenter
                        implicitHeight: 50
                        implicitWidth: 50
                    }

                    TextButton {
                        id: upgradeButton
                        visible: !root.isLoadingPurchseLink
                        text: qsTr("Upgrade to Notes Pro")
                        height: 35
                        platform: root.platform
                        displayFontFamily: root.displayFontFamily
                        textAlignment: TextButton.TextAlign.Middle
                        themeData: root.themeData
                        textFontPointSize: 14
                        defaultBackgroundColor: "#0e70d2"
                        highlightBackgroundColor: root.themeData.theme === "Dark" ? "#1a324b" : defaultBackgroundColor
                        pressedBackgroundColor: highlightBackgroundColor
                        mainFontColorDefault: "white"
                        mainFontColorPressed: root.themeData.theme === "Dark" ? "#c6ccd2" : "#edebeb"
                        textFontWeight: Font.Bold
                        backgroundOpacity: upgradeButton.entered && root.themeData.theme !== "Dark " ? 0.5 : 1.0
                        Layout.alignment: Qt.AlignHCenter
                        backgroundWidthOffset: 100
                        pointSizeOffset: root.platform === "Apple" ? 0 : -3

                        onClicked: {
                            root.openPurchaseLink();
                        }
                    }

                    TextButton {
                        text: qsTr("Or fill in your license key")
                        height: 35
                        platform: root.platform
                        displayFontFamily: root.displayFontFamily
                        textAlignment: TextButton.TextAlign.Middle
                        themeData: root.themeData
                        textFontPointSize: 14
                        defaultBackgroundColor: "transparent"
                        highlightBackgroundColor: "transparent"
                        pressedBackgroundColor: "transparent"
                        usePointingHand: false
                        mainFontColorDefault: root.themeData.theme === "Dark" ? "#2383e2" : "#007bff"
                        mainFontColorPressed: root.themeData.theme === "Dark" ? "#0f68c2" : "#459fff"
                        textFontWeight: Font.Normal
                        Layout.alignment: Qt.AlignHCenter
                        backgroundSizeFitText: true
                        pointSizeOffset: root.platform === "Apple" ? 0 : -3

                        onClicked: {
                            activationPopup.visible = true;
                            licenseTextField.forceActiveFocus();
                        }
                    }
                }
            }
        }

        Row {
            Layout.preferredWidth: root.width/2

            Item {
                width: 20
                height: 1
            }

            Text {
                text: 'Or access all Pro features at no cost by building the app from <a href="https://github.com/nuttyartist/notes?utm_source=awesome_app" style="color:' + (root.themeData.theme === "Dark" ? '#2383e2' : '#007bff') + '; text-decoration: none;">source.</a>'
                width: 185
                color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
                font.family: root.displayFontFamily
                font.pointSize: 10 + root.pointSizeOffset
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
                onLinkActivated: Qt.openUrlExternally(link)
                textFormat: TextEdit.RichText

                MouseArea {
                    // TODO: How to make cursor change only when hovering upon the link??
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton // Don't eat the mouse clicks
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        Item {
            width: 1
            height: 12
        }

        Row {
            spacing: 10
            Layout.alignment: Qt.AlignHCenter

            TextButton {
                text: qsTr("Terms of Use")
                height: 35
                platform: root.platform
                displayFontFamily: root.displayFontFamily
                textAlignment: TextButton.TextAlign.Middle
                themeData: root.themeData
                textFontPointSize: 11
                defaultBackgroundColor: "transparent"
                highlightBackgroundColor: "transparent"
                pressedBackgroundColor: "transparent"
                usePointingHand: true
                mainFontColorDefault: root.themeData.theme === "Dark" ? "#7c7c7c" : "#7a7979"
                mainFontColorPressed: root.themeData.theme === "Dark" ? "#656565" : "#c0c0c0"
                textFontWeight: Font.Bold
                Layout.alignment: Qt.AlignHCenter
                backgroundSizeFitText: true
                pointSizeOffset: root.platform === "Apple" ? 0 : -3

                onClicked: {
                    Qt.openUrlExternally("https://www.get-notes.com/notes-app-terms-privacy-policy");
                }
            }

            Rectangle {
                height: 10
                width: 1
                color: "gray"
                anchors.verticalCenter: privacyPolicybutton.verticalCenter
            }

            TextButton {
                id: privacyPolicybutton
                text: qsTr("Privacy Policy")
                height: 35
                platform: root.platform
                displayFontFamily: root.displayFontFamily
                textAlignment: TextButton.TextAlign.Middle
                themeData: root.themeData
                textFontPointSize: 11
                defaultBackgroundColor: "transparent"
                highlightBackgroundColor: "transparent"
                pressedBackgroundColor: "transparent"
                usePointingHand: true
                mainFontColorDefault: root.themeData.theme === "Dark" ? "#7c7c7c" : "#7a7979"
                mainFontColorPressed: root.themeData.theme === "Dark" ? "#656565" : "#c0c0c0"
                textFontWeight: Font.Bold
                Layout.alignment: Qt.AlignHCenter
                backgroundSizeFitText: true
                pointSizeOffset: root.platform === "Apple" ? 0 : -3

                onClicked: {
                    Qt.openUrlExternally("https://www.get-notes.com/notes-app-terms-privacy-policy");
                }
            }
        }
    }
}
