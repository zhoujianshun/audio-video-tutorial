QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    yuvplayer.cpp

HEADERS += \
    mainwindow.h \
    yuvplayer.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

macx {
    FFMPEG_HOME = /usr/local/ffmpeg
    SDL_HOME = /opt/homebrew/Cellar/sdl2/2.0.18
#    QMAKE_INFO_PLIST += Info.plist
    message($${SDL_HOME})
}

win32 {
   # FFMPEG_HOME =
}

INCLUDEPATH += $${FFMPEG_HOME}/include

# ffmpeg库文件
LIBS += -L$${FFMPEG_HOME}/lib \
    -lavutil \

INCLUDEPATH += $${SDL_HOME}/include

# ffmpeg库文件
LIBS += -L$${SDL_HOME}/lib \
    -lSDL2 \
