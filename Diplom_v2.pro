QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    add_math.cpp \
    collision.cpp \
    game.cpp \
    graph.cpp \
    list.cpp \
    main.cpp \
    objects.cpp \
    path.cpp \
    tanks.cpp

HEADERS += \
    add_math.h \
    collision.h \
    game.h \
    graph.h \
    list.h \
    objects.h \
    tanks.h \
    ui.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Diplom_v2.pro.user \
    TODO_list

STATECHARTS += \
    state.scxml
