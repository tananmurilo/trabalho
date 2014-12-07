QT       += core
QT       -= gui

TARGET = OpenCV-Ex1
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

win32 {
    INCLUDEPATH += D:/OpenCV-MinGW/install/include

    LIBS += -LD:/OpenCV-MinGW/lib \
        -llibopencv_core249 \
        -llibopencv_highgui249 \
        -llibopencv_imgproc249 \
        -llibopencv_features2d249 \
        -llibopencv_calib3d249 \
        -llibopencv_ml249 \
        -llibopencv_features2d249
}

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv

}
