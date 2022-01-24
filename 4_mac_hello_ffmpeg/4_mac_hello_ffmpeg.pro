QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mypushbutton.cpp

HEADERS += \
    mainwindow.h \
    mypushbutton.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



# mac:表示mac环境
# windows使用win32表示

# ffmpeg头文件
#mac:INCLUDEPATH += $${FFMPEG_HOME}/include

## ffmpeg库文件
#mac:LIBS += -L $${FFMPEG_HOME}/lib \
#        -lavcodec \
#        -lavdevice \
#        -lavfilter \
#        -lavformat \
#        -lavutil \
#        -lpostproc \
#        -lswscale \
#        -lswresample \
#        -lavresample
mac:{

    FFMPEG_HOME = /opt/homebrew/Cellar/ffmpeg/4.4.1_3
    message($${FFMPEG_HOME})

    INCLUDEPATH += $${FFMPEG_HOME}/include

    # ffmpeg库文件
    LIBS += -L $${FFMPEG_HOME}/lib \
            -lavcodec \
            -lavdevice \
            -lavfilter \
            -lavformat \
            -lavutil \
            -lpostproc \
            -lswscale \
            -lswresample \
            -lavresample
}
