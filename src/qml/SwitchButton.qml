import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Universal 2.12

Switch {
    id: buttonContainer
    property var themeData: {{thme: "Light"}}
    signal switched()
    Universal.theme: themeData.theme === "Dark" ? Universal.Dark : Universal.Light

    onClicked: {
        buttonContainer.switched();
    }
}
