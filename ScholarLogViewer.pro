QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
win32{
RC_ICONS = logo.ico
}

macx{
TARGET = ScholarlogViewer
ICON = logo.icns
QMAKE_INFO_PLIST = Info.plist

SOURCES += \
    mac/macminimizewidget.mm

HEADERS += \
    mac/macminimizewidget.h

LIBS += -framework CoreServices
LIBS += -framework Foundation
LIBS += -framework CoreFoundation
LIBS += -framework Carbon
LIBS += -lobjc
LIBS += -framework AppKit
DEFINES += OBJC_OLD_DISPATCH_PROTOTYPES

}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    fileprocessing/actionlogprocessor.cpp \
    fileprocessing/androidlogprocessor.cpp \
    fileprocessing/ioslogprocessor.cpp \
    fileprocessing/programlogprocessor.cpp \
    fileprocessing/qcommondelegate.cpp \
    imaskwidget.cpp \
    main.cpp \
    scholarlogviewer.cpp \
    usertablemodel.cpp \
    waitingwidget.cpp

HEADERS += \
    CommonHelper.h \
    UtilityDefine.h \
    fileprocessing/actionlogprocessor.h \
    fileprocessing/androidlogprocessor.h \
    fileprocessing/baselogprocessor.h \
    fileprocessing/ioslogprocessor.h \
    fileprocessing/programlogprocessor.h \
    fileprocessing/qcommondelegate.h \
    imaskwidget.h \
    scholarlogviewer.h \
    usertablemodel.h \
    waitingwidget.h

FORMS += \
    imaskwidget.ui \
    scholarlogviewer.ui \
    waitingwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
