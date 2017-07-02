TEMPLATE = subdirs

SUBDIRS = \
    src

CONFIG += ordered

src.file = src/Notes.pro
tests.depends = src
