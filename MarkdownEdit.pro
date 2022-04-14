QT       += core gui svg printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051208    # disables all the APIs deprecated before Qt 5.12.8


INCLUDEPATH += \
    src/ \
    3rdparty/md4c/src


SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/about.cpp \
    src/parser.cpp \
    src/settings.cpp \
    3rdparty/md4c/src/entity.c \
    3rdparty/md4c/src/md4c.c \
    3rdparty/md4c/src/md4c-html.c

HEADERS += \
    src/mainwindow.h \
    src/about.h \
    src/parser.h \
    src/settings.h \
    3rdparty/md4c/src/entity.h \
    3rdparty/md4c/src/md4c.h \
    3rdparty/md4c/src/md4c-html.h

FORMS += \
    ui/mainwindow.ui \
    ui/settings.ui

TRANSLATIONS += \
    translations/MarkdownEdit_de.ts

include(3rdparty/qmarkdowntextedit/qmarkdowntextedit.pri)
include(3rdparty/QSourceHighlite/QSourceHighlite.pri)
include(3rdparty/qtspell/qtspell.pri)

CONFIG += lrelease
CONFIG += embed_translations
CONFIG -= qtquickcompiler

VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Only show qDebug() messages in debug mode
CONFIG(release, debug | release): DEFINES += QT_NO_DEBUG_OUTPUT


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ressources/ressources.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
