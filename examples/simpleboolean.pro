TARGET = simpleboolean
TEMPLATE = app

QT += widgets opengl
CONFIG += debug

INCLUDEPATH += ../
INCLUDEPATH += ../thirdparty/moller97
INCLUDEPATH += ../thirdparty/poly2tri/poly2tri

VPATH += ../

SOURCES += simpleboolean/meshdatatype.cpp
HEADERS += simpleboolean/meshdatatype.h

SOURCES += simpleboolean/axisalignedboundingbox.cpp
HEADERS += simpleboolean/axisalignedboundingbox.h

SOURCES += simpleboolean/retriangulation.cpp
HEADERS += simpleboolean/retriangulation.h

SOURCES += simpleboolean/edgeloop.cpp
HEADERS += simpleboolean/edgeloop.h

SOURCES += simpleboolean/subsurface.cpp
HEADERS += simpleboolean/subsurface.h

SOURCES += simpleboolean/subblock.cpp
HEADERS += simpleboolean/subblock.h

SOURCES += simpleboolean/distinguish.cpp
HEADERS += simpleboolean/distinguish.h

SOURCES += simpleboolean/meshcombiner.cpp
HEADERS += simpleboolean/meshcombiner.h

SOURCES += simpleboolean/util.cpp
HEADERS += simpleboolean/util.h

SOURCES += simpleboolean/triangulate.cpp
HEADERS += simpleboolean/triangulate.h

SOURCES += thirdparty/moller97/tritri_isectline.cpp
HEADERS += thirdparty/moller97/tritri_isectline.h

SOURCES += thirdparty/poly2tri/poly2tri/sweep/advancing_front.cc
HEADERS += thirdparty/poly2tri/poly2tri/sweep/advancing_front.h

SOURCES += thirdparty/poly2tri/poly2tri/sweep/cdt.cc
HEADERS += thirdparty/poly2tri/poly2tri/sweep/cdt.h

SOURCES += thirdparty/poly2tri/poly2tri/sweep/sweep_context.cc
HEADERS += thirdparty/poly2tri/poly2tri/sweep/sweep_context.h

SOURCES += thirdparty/poly2tri/poly2tri/sweep/sweep.cc
HEADERS += thirdparty/poly2tri/poly2tri/sweep/sweep.h

SOURCES += thirdparty/poly2tri/poly2tri/common/shapes.cc
HEADERS += thirdparty/poly2tri/poly2tri/common/shapes.h

HEADERS += thirdparty/poly2tri/poly2tri/common/utils.h

HEADERS += thirdparty/poly2tri/poly2tri/poly2tri.h

SOURCES += glwidget.cpp
HEADERS += glwidget.h

SOURCES += testmesh.cpp
HEADERS += testmesh.h

SOURCES += model.cpp
HEADERS += model.h

SOURCES += mainwindow.cpp
HEADERS += mainwindow.h

SOURCES += main.cpp

QMAKE_CXXFLAGS += -std=c++11

target.path = ./
INSTALLS += target
