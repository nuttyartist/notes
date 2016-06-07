TEMPLATE = subdirs
SUBDIRS = \
    src\
    tests

CONFIG+=ordered

src.file = src/Notes.pro

tests.depends = src
