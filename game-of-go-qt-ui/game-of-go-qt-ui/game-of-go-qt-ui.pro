######################################################################
# Automatically generated by qmake (3.1) Sun Dec 17 12:39:39 2023
######################################################################

TEMPLATE = app
TARGET = game-of-go-qt-ui
INCLUDEPATH += .
QT += gui widgets

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += gameboardwidget.h \
           loginwindow.h \
           registerwindow.h \
           mainwindow.h \
           stonewidget.h \
           territorywidget.h \
           socket.h \
           gamewidget.h \
           menuwidget.h \
           playwidget.h \
           challengewindow.h \
           scoreboardwidget.h \
           logtablewidget.h \
           historywidget.h \
           historyitemwidget.h
FORMS += gameboardwidget.ui \
         loginwindow.ui \
         registerwindow.ui \
         mainwindow.ui \
         stonewidget.ui \
         territorywidget.ui \
         gamewidget.ui \
         menuwidget.ui \
         playwidget.ui \
         challengewindow.ui \
         scoreboardwidget.ui \
         logtablewidget.ui \
         historywidget.ui \
         historyitemwidget.ui
SOURCES += gameboardwidget.cpp \
           loginwindow.cpp \
           registerwindow.cpp \
           main.cpp \
           mainwindow.cpp \
           stonewidget.cpp \
           territorywidget.cpp \
           socket.cpp \
           gamewidget.cpp \
           menuwidget.cpp \
           playwidget.cpp \
           challengewindow.cpp \
           scoreboardwidget.cpp \
           logtablewidget.cpp \
           historywidget.cpp \
           historyitemwidget.cpp
