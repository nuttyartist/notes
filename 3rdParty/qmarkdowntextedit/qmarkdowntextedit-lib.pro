TARGET = QMarkdownTextedit
TEMPLATE = lib
QT += core gui widgets
CONFIG += c++11

include(qmarkdowntextedit.pri)

TRANSLATIONS += trans/qmarkdowntextedit_de.ts

target.path = $$[QT_INSTALL_LIBS]

headers.files = $$HEADERS
headers.path = $$[QT_INSTALL_PREFIX]/include/$$TARGET/

license.files = LICENSE
license.path = $$[QT_INSTALL_PREFIX]/share/licenses/$$TARGET/

INSTALLS += target license headers
