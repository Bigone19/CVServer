QT       += core sql concurrent network gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        fileTransferRecevicer.cpp \
        server.cpp \
        yoloDetector.cpp \
        yoloUtils.cpp \
        resTransferSender.cpp \
        fileUtils.cpp

HEADERS += \
        config.h \
        fileTransferRecevicer.h \
        server.h \
        yoloDetector.h \
        yoloConfig.h \
        yoloUtils.h \
        resTransferSender.h \
        fileUtils.h

TRANSLATIONS += \
    server_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

#INCLUDEPATH += ../libtorch/include \
#               ../libtorch/include/torch/csrc/api/include

#DEPENDPATH += ../libtorch/include \
#              ../libtorch/include/torch/csrc/api/include

#LIBS += -L../libtorch/lib -lc10 \#-lc10
#        -lclog -lcpuinfo \
#        -llibprotoc \
#        -ltorch \#-ltorch -ltorch_cuda
#        -ltorch_cpu
#
INCLUDEPATH+=../opencv/build/include \
             ../opencv/build/include/opencv \
             ../opencv/build/include/opencv2 \
             ../Microsoft.ML.OnnxRuntime.1.10.0/build/native/include
DEPENDPATH+=../opencv/build/include \
            ../opencv/build/include/opencv \
            ../opencv/build/include/opencv2 \
            ../Microsoft.ML.OnnxRuntime.1.10.0/build/native/include
LIBS += -L../opencv/build/x64/vc15/lib \
        -lopencv_world452d \
        -lopencv_world452\
        -L../Microsoft.ML.OnnxRuntime.1.10.0/runtimes/win-x64/native \
        -lonnxruntime

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    server.ui
