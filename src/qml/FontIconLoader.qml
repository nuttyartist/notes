import QtQuick 2.12

Item {
    id: awesome

    property alias icons: variables

    readonly property FontLoader fontAwesomeRegular: FontLoader {
        source: "qrc:/fonts/fontawesome/fa-regular-400.ttf"
    }

    readonly property FontLoader fontAwesomeSolid: FontLoader {
        source: "qrc:/fonts/fontawesome/fa-solid-900.ttf"
    }

    readonly property FontLoader fontAwesomeBrands: FontLoader {
        source: "qrc:/fonts/fontawesome/fa-brands-400.ttf"
    }

    readonly property FontLoader materialSymbols: FontLoader {
        source: "qrc:/fonts/material/material-symbols-outlined.ttf"
    }

    readonly property string fa_regular: fontAwesomeRegular.name
    readonly property string fa_solid: fontAwesomeSolid.name
    readonly property string fa_brands: fontAwesomeBrands.name
    readonly property string mt_symbols: materialSymbols.name

    FontIconsCodes {
        id: variables
    }
}
