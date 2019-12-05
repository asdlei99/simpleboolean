TARGET = benchmark
TEMPLATE = app

QT += widgets
CONFIG += release
DEFINES += NDEBUG

macx {
	QMAKE_CXXFLAGS_RELEASE -= -O
	QMAKE_CXXFLAGS_RELEASE -= -O1
	QMAKE_CXXFLAGS_RELEASE -= -O2

	QMAKE_CXXFLAGS_RELEASE += -O3

	QMAKE_CXXFLAGS += -std=c++11
}

unix:!macx {
	QMAKE_CXXFLAGS_RELEASE -= -O
	QMAKE_CXXFLAGS_RELEASE -= -O1
	QMAKE_CXXFLAGS_RELEASE -= -O2

	QMAKE_CXXFLAGS_RELEASE += -O3

	QMAKE_CXXFLAGS += -std=c++11
}

win32 {
	QMAKE_CXXFLAGS += /O2
}

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

SOURCES += main.cpp

QMAKE_CXXFLAGS += -std=c++11

target.path = ./
INSTALLS += target
