import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Item {
    id: checkBoxContainer
    width: checkBox.width - 9 // needed for some reason
    height: checkBox.height
    property bool checked
    property string theme: "Light"
    signal taskChecked
    signal taskUnchecked
    property bool isReadOnlyMode

    CheckBox {
        id: checkBox
        checked: checkBoxContainer.checked
        anchors.verticalCenter: parent.verticalCenter
//        Material.theme: checkBoxContainer.theme === "Dark" ? Material.Dark : Material.Light
        Material.accent: "#2383e2";
        enabled: !checkBoxContainer.isReadOnlyMode

        onClicked: {
            if (checked) {
                checkBoxContainer.taskChecked();
            } else {
                checkBoxContainer.taskUnchecked()
            }
        }
    }
}
