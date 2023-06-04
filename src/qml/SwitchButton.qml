import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal

Switch {
    id: buttonContainer
    property var themeData
    signal switched()
    Universal.theme: themeData.theme === "Dark" ? Universal.Dark : Universal.Light

    onClicked: {
        buttonContainer.switched();
    }
}
