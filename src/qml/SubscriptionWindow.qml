import QtQuick 2.12
import QtQuick.Layouts 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import nuttyartist.notes 1.0
import "Utilities.js" as Utils

ApplicationWindow {
    id: root
    width: 650
    height: 400
    property string platform: "Other"
    property string displayFontFamily
    property string accentColor: "#0a84ff"
    property int pointSizeOffset: platform === "Apple" ? 0: -3
    property var themeData: {"theme": "Light", "backgroundColor": "white"}
    property int columnHeight: 280
    property bool canActivate: licenseTextField.length === 36 && !licenseTextField.isPlaceHolderTextShown
    property string enteredLicense: ""

    property string getPurchaseDataAlt1: "https://raw.githubusercontent.com/nuttyartist/notes/master/notes_purchase_data.json"
    property string getPurchaseDataAlt2: "https://www.rubymamistvalove.com/notes/notes_purchase_data.json"
    property string checkoutURLDefault: "https://www.get-notes.com/pricing"
    property string checkoutUtmSource: "?utm_source=notes_app_pro_payment_window"
    property string purchaseApiBaseUrlDefault: "https://api.lemonsqueezy.com"
    property string activateLicenseEndpointDefault: "/v1/licenses/activate"
    property string validateLicenseEndpointDefault: "/v1/licenses/validate"
    property string deactivateLicenseEndpointDefault: "/v1/licenses/deactivate"
    property string getSubscriptionsApiBaseUrl: "https://rubymamistvalove.com/api"
    property string getSubscriptionsApiEndpoint: "/getSubscription"
    property string cencelSubscriptionsApiEndpoint: "/cancelSubscription"

    property bool isLoadingPurchseLink: false
    property bool isActivatingLicense: false
    property bool showLicenseErrorMessage: false
    property string licenseErrorMessageText: ""
    property bool isVerifyingAgain: false
    property bool showMessageAfterFailedInternet: false
    signal subscribedSuccessfully

    property bool isProVersion: false
    property int subscriptionStatus: SubscriptionStatus.NoSubscription
    property bool forceSubscriptionStatus: false
    property string sadIconColor: root.themeData.theme === "Dark" ? "#f5ce42" : "#ebb434"

    property string successActivationPopupMainCopy: qsTr("Successfully Activated Notes Pro")
    property string successActivationPopupSecondaryCopy: qsTr("Thank you for purchasing Notes Pro and supporting open source development. Your access to Pro features will continue as long as your subscription remains active. Enjoy!")
    property var subscriptionObject
    property date nextPaymentDate: root.subscriptionObject ? root.parseISO8601Date(root.subscriptionObject["attributes"]["renews_at"]) : new Date();
    property string nextPaymentDateString: root.subscriptionObject && nextPaymentDate != null ? root.formatDate(root.nextPaymentDate) : ""
    property date endsAtDate: root.subscriptionObject && root.subscriptionObject["attributes"]["ends_at"] !== null ? root.parseISO8601Date(root.subscriptionObject["attributes"]["ends_at"]) : new Date();

    FontIconLoader {
        id: fontIconLoader
    }

    Connections {
        target: mainWindow

        function onDisplayFontSet (data) {
            root.displayFontFamily = data.displayFont;
        }

        function onPlatformSet (data) {
            root.platform = data;
        }

        function onThemeChanged (data) {
            root.themeData = data;
        }

        function onProVersionCheck (data) {
            root.isProVersion = data;

            if (root.isVerifyingAgain && (!root.isProVersion || root.subscriptionStatus === SubscriptionStatus.NoInternetConnection)) {
                root.showMessageAfterFailedInternet = true;
            }

            root.isVerifyingAgain = false;

            if (root.isProVersion && (root.subscriptionStatus === SubscriptionStatus.GracePeriodOver ||
                                      root.subscriptionStatus === SubscriptionStatus.EnteredGracePeriod)) {
                root.showMessageAfterFailedInternet = false;
                root.successActivationPopupMainCopy = qsTr("Successfully Verified Notes Pro");
                root.successActivationPopupSecondaryCopy = qsTr("Your access to Pro features will continue as long as your subscription remains active. Enjoy!")
                successActivationPopup.visible = true;
            }
        }

        function onSubscriptionStatusChanged (data) {
            if (!root.forceSubscriptionStatus) {
                root.subscriptionStatus = data;
            }
        }
    }

    function getData (BASE, cb) {
        Utils.request('GET', BASE, null, null, cb);
    }

    function postData (BASE, endpoint, entry, cb) {
        Utils.request('POST', BASE, endpoint, entry, cb);
    }

    function deleteData (BASE, cb) {
        Utils.request('DELETE', BASE, null, null, cb);
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
                        Qt.openUrlExternally(root.checkoutURLDefault + root.checkoutUtmSource);
                    } else {
                        root.isLoadingPurchseLink = false;
                        Qt.openUrlExternally(response.purchase_pro_url + root.checkoutUtmSource);
                    }
                });
            } else {
                root.isLoadingPurchseLink = false;
                Qt.openUrlExternally(response.purchase_pro_url + root.checkoutUtmSource);
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
                mainWindow.setActivationSuccessful(root.enteredLicense);
                root.isActivatingLicense = false;
                activationPopup.close();
                activationPopup.visible = false;
                successActivationPopup.visible = true;
            }
        });
    }

    function activateLicense () {
        root.enteredLicense = licenseTextField.text;
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

    function parseISO8601Date(dateStr) {
        if (dateStr) {
            var dateParts = dateStr.split("T")[0].split("-");
            return new Date(dateParts[0], dateParts[1]-1, dateParts[2]);
        }
    }

    function formatDate(date : date) {
        if (date) {
            var monthNames = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
            var month = monthNames[date.getMonth()];
            return month + " " + date.getDate() + ", " + date.getFullYear();
        }
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
                text: root.successActivationPopupMainCopy
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
                text: root.successActivationPopupSecondaryCopy
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

    Component {
        id: upgradeToProCopy

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
                        text: qsTr("Unlock editing directly within the Kanban view for a seamless workflow")
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
                        text: fontIconLoader.icons.fa_linux
                        font.family: fontIconLoader.fontAwesomeBrands.name
                        color: root.accentColor
                        font.pointSize: 31 + root.pointSizeOffset
                    }

                    Text {
                        anchors.verticalCenter: pointIcon2.verticalCenter
                        text: qsTr("Support the growth of open-source development")
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
    }

    Component {
        id: upgradeToProCta

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

    Component {
        id: termsAndPrivacy

        Rectangle {
            width: root.width
            height: privacyPolicybutton.height
            color: "transparent"

            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter

                TextButton {
                    text: qsTr("Terms of Use")
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

    Component {
        id: accessByCompilationCopy

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
                font.pointSize: 12 + root.pointSizeOffset
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
    }

    Component {
        id: licenseInvalidCopy

        Rectangle {
            width: root.width/2
            height: root.columnHeight
            color: "transparent"

            Column {
                width: parent.width
                anchors.centerIn: parent

                Item {
                    width: 1
                    height: 20
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: fontIconLoader.icons.fa_face_sad_tear
                    font.family: fontIconLoader.fontAwesomeRegular.name
                    color: root.sadIconColor
                    font.pointSize: 40 + root.pointSizeOffset
                }

                Item {
                    width: 1
                    height: 10
                }

                Text {
                    width: 300
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("License Invalid")
                    color: root.themeData.theme === "Dark" ? "#dfdedf" : "#272727"
                    font.family: root.displayFontFamily
                    font.pointSize: 26 + root.pointSizeOffset
                    font.weight: Font.Black
                    horizontalAlignment: Text.AlignHCenter
                }

                Item {
                    width: 1
                    height: 20
                }

                Text {
                    width: 300
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("We couldn't locate your license key in our database. For assistance, please contact us at the email address below:")
                    color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
                    font.family: root.displayFontFamily
                    font.pointSize: 15 + root.pointSizeOffset
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Item {
                    width: 1
                    height: 20
                }

                CustomTextField {
                    anchors.horizontalCenter: parent.horizontalCenter
                    readOnly: true
                    text: "contact@get-notes.com"
                    background: Rectangle {
                        border.width: licenseTextField.focus ? 2 : 0
                        border.color: "#2383e2"
                        radius: 5
                        color: root.themeData.theme === "Dark" ? "#313131" : "#efefef"
                    }
                    color: root.themeData.theme === "Dark" ? "white" : "black"
                }
            }
        }
    }

    Component {
        id: unknownErrorCopy

        Rectangle {
            width: root.width
            height: root.columnHeight
            color: "transparent"

            Column {
                width: parent.width
                anchors.centerIn: parent

                Item {
                    width: 1
                    height: 20
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: fontIconLoader.icons.fa_face_sad_tear
                    font.family: fontIconLoader.fontAwesomeRegular.name
                    color: root.sadIconColor
                    font.pointSize: 40 + root.pointSizeOffset
                }

                Item {
                    width: 1
                    height: 10
                }

                Text {
                    width: 300
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Unknown Error")
                    color: root.themeData.theme === "Dark" ? "#dfdedf" : "#272727"
                    font.family: root.displayFontFamily
                    font.pointSize: 26 + root.pointSizeOffset
                    font.weight: Font.Black
                    horizontalAlignment: Text.AlignHCenter
                }

                Item {
                    width: 1
                    height: 20
                }

                Text {
                    width: 300
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("An unknown error occurred while verifying your license. For assistance, please contact us at the email address below:")
                    color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
                    font.family: root.displayFontFamily
                    font.pointSize: 15 + root.pointSizeOffset
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Item {
                    width: 1
                    height: 20
                }

                CustomTextField {
                    anchors.horizontalCenter: parent.horizontalCenter
                    readOnly: true
                    text: "contact@get-notes.com"
                    background: Rectangle {
                        border.width: licenseTextField.focus ? 2 : 0
                        border.color: "#2383e2"
                        radius: 5
                        color: root.themeData.theme === "Dark" ? "#313131" : "#efefef"
                    }
                    color: root.themeData.theme === "Dark" ? "white" : "black"
                }

                Item {
                    width: 1
                    height: 50
                }

                TextButton {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Close")
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
                    }
                }
            }
        }
    }

    Component {
        id: gracePeriodOverCopy

        Rectangle {
            width: root.width/2
            height: root.columnHeight
            color: "transparent"

            Column {
                width: parent.width
                anchors.centerIn: parent

                Item {
                    width: 1
                    height: 20
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: fontIconLoader.icons.fa_face_sad_tear
                    font.family: fontIconLoader.fontAwesomeRegular.name
                    color: root.sadIconColor
                    font.pointSize: 40 + root.pointSizeOffset
                }

                Item {
                    width: 1
                    height: 10
                }

                Text {
                    width: 300
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Reached Grace Period Limit")
                    color: root.themeData.theme === "Dark" ? "#dfdedf" : "#272727"
                    font.family: root.displayFontFamily
                    font.pointSize: 26 + root.pointSizeOffset
                    font.weight: Font.Black
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }

                Item {
                    width: 1
                    height: 20
                }

                Text {
                    width: 300
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Notes couldn't verify your license because you have been out of network connection for 7 days or more. Please connect to the internet and try again to verify your license and use Notes Pro.")
                    color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
                    font.family: root.displayFontFamily
                    font.pointSize: 15 + root.pointSizeOffset
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Item {
                    width: 1
                    height: 30
                }
            }
        }
    }

    Component {
        id: checkInternetConnectionCta

        Rectangle {
            id: checkInternetConnectionCtaRect
            width: root.width/2
            height: root.columnHeight
            color: "transparent"

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 0

                BusyIndicator {
                    visible: root.isVerifyingAgain
                    running: root.isVerifyingAgain
                    Material.theme: root.themeData.theme === "Dark" ? Material.Dark : Material.Light
                    Material.accent: "#5b94f5"
                    Layout.alignment: Qt.AlignHCenter
                    implicitHeight: 50
                    implicitWidth: 50
                }

                Text {
                    visible: root.showMessageAfterFailedInternet && !root.isVerifyingAgain
                    Layout.preferredWidth: checkInternetConnectionCtaRect.width/2 + 10
                    Layout.alignment: Qt.AlignHCenter
                    text: "Error occurred. Check your internet connection."
                    color: "#d93e37"
                    font.family: root.displayFontFamily
                    font.pointSize: 13 + root.pointSizeOffset
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Item {
                    visible: root.isVerifyingAgain || root.showMessageAfterFailedInternet
                    width: 1
                    height: 20
                }

                TextButton {
                    visible: !root.isLoadingPurchseLink
                    text: qsTr("Try again")
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
                    backgroundOpacity: entered && root.themeData.theme !== "Dark " ? 0.5 : 1.0
                    Layout.alignment: Qt.AlignHCenter
                    backgroundWidthOffset: 100
                    pointSizeOffset: root.platform === "Apple" ? 0 : -3

                    onClicked: {
                        if (!root.isVerifyingAgain) {
                            root.isVerifyingAgain = true;
                            mainWindow.checkProVersion();
                        }
                    }
                }
            }
        }
    }

    Component {
        id: manageSubscription

        Rectangle {
            width: root.width
            height: root.columnHeight
            color: "transparent"

            Column {
                anchors.fill: parent
                spacing: 0
                topPadding: 20

                Text {
                    text: qsTr("Thank you for supporting Notes FOSS.")
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
                    text: qsTr("You're using the Pro version of Notes.")
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: root.themeData.theme === "Dark" ? "#dfdedf" : "#272727"
                    font.family: root.displayFontFamily
                    font.pointSize: 13 + root.pointSizeOffset
                    font.weight: Font.Bold
                }

                Item {
                    width: 1
                    height: 30
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: fontIconLoader.icons.fa_heart
                    font.family: fontIconLoader.fontAwesomeSolid.name
                    color: "#f1554d"
                    font.pointSize: 150 + root.pointSizeOffset
                }
            }
        }
    }

    Component {
        id: noInternetConnectionCopy

        Rectangle {
            width: root.width/2
            height: root.columnHeight
            color: "transparent"

            Column {
                width: parent.width
                anchors.centerIn: parent

                Item {
                    width: 1
                    height: 20
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: fontIconLoader.icons.fa_face_sad_tear
                    font.family: fontIconLoader.fontAwesomeRegular.name
                    color: root.sadIconColor
                    font.pointSize: 40 + root.pointSizeOffset
                }

                Item {
                    width: 1
                    height: 10
                }

                Text {
                    width: 300
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Error occurred")
                    color: root.themeData.theme === "Dark" ? "#dfdedf" : "#272727"
                    font.family: root.displayFontFamily
                    font.pointSize: 26 + root.pointSizeOffset
                    font.weight: Font.Black
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }

                Item {
                    width: 1
                    height: 20
                }

                Text {
                    width: 300
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Check your internet connection.")
                    color: root.themeData.theme === "Dark" ? "#dddddd" : "#272727"
                    font.family: root.displayFontFamily
                    font.pointSize: 15 + root.pointSizeOffset
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Item {
                    width: 1
                    height: 30
                }
            }
        }
    }

    Rectangle {
        id: rootBackground
        color: root.themeData.theme === "Dark" ? "#252525" : "white"
        anchors.fill: parent

        ColumnLayout {
            id: contentContainer
            anchors.fill: parent
            spacing: 0

            Item {
                width: 1
                height: 30
            }

            Row {
                Loader {
                    sourceComponent: if (root.subscriptionStatus === SubscriptionStatus.NoSubscription ) {
                                         upgradeToProCopy
                                     } else if (root.subscriptionStatus === SubscriptionStatus.Expired) {
                                         subscriptionExpiredCopy
                                     } else if (root.subscriptionStatus === SubscriptionStatus.Invalid) {
                                         licenseInvalidCopy
                                     } else if (root.subscriptionStatus === SubscriptionStatus.UnknownError) {
                                         unknownErrorCopy
                                     } else if (root.subscriptionStatus === SubscriptionStatus.GracePeriodOver) {
                                         gracePeriodOverCopy
                                     } else if (root.subscriptionStatus === SubscriptionStatus.Active) {
                                         manageSubscription
                                     } else if (root.subscriptionStatus === SubscriptionStatus.NoInternetConnection ||
                                                root.subscriptionStatus === SubscriptionStatus.EnteredGracePeriod) {
                                        noInternetConnectionCopy
                                     }

                    active: root.subscriptionStatus === SubscriptionStatus.NoSubscription ||
                            root.subscriptionStatus === SubscriptionStatus.Expired ||
                            root.subscriptionStatus === SubscriptionStatus.Invalid ||
                            root.subscriptionStatus === SubscriptionStatus.UnknownError ||
                            root.subscriptionStatus === SubscriptionStatus.GracePeriodOver ||
                            root.subscriptionStatus === SubscriptionStatus.Active ||
                            root.subscriptionStatus === SubscriptionStatus.NoInternetConnection ||
                            root.subscriptionStatus === SubscriptionStatus.EnteredGracePeriod
                }

                Loader {
                    sourceComponent: if (root.subscriptionStatus === SubscriptionStatus.GracePeriodOver ||
                                             root.subscriptionStatus === SubscriptionStatus.NoInternetConnection ||
                                             root.subscriptionStatus === SubscriptionStatus.EnteredGracePeriod) {
                                         checkInternetConnectionCta
                                     } else {
                                         upgradeToProCta
                                     }

                    active: root.subscriptionStatus === SubscriptionStatus.NoSubscription ||
                            root.subscriptionStatus === SubscriptionStatus.Expired ||
                            root.subscriptionStatus === SubscriptionStatus.Invalid ||
                            root.subscriptionStatus === SubscriptionStatus.GracePeriodOver ||
                            root.subscriptionStatus === SubscriptionStatus.NoInternetConnection ||
                            root.subscriptionStatus === SubscriptionStatus.EnteredGracePeriod
                }
            }

            Loader {
                sourceComponent: accessByCompilationCopy
                active: root.subscriptionStatus === SubscriptionStatus.NoSubscription
            }

            Loader {
                sourceComponent: termsAndPrivacy
                active: root.subscriptionStatus === SubscriptionStatus.NoSubscription ||
                        root.subscriptionStatus === SubscriptionStatus.Expired ||
                        root.subscriptionStatus === SubscriptionStatus.Invalid ||
                        root.subscriptionStatus === SubscriptionStatus.GracePeriodOver ||
                        root.subscriptionStatus === SubscriptionStatus.Active
            }

            Item {
                width: 1
                height: 10
            }
        }
    }
}
