TEMPLATE = subdirs
SUBDIRS = app lib
app.file = qmarkdowntextedit-app.pro
lib.file = qmarkdowntextedit-lib.pro
app.depends = lib
