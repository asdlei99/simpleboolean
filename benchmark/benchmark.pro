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

INCLUDEPATH += ./
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

SOURCES += simpleboolean/meshoperator.cpp
HEADERS += simpleboolean/meshoperator.h

SOURCES += simpleboolean/util.cpp
HEADERS += simpleboolean/util.h

SOURCES += simpleboolean/triangulate.cpp
HEADERS += simpleboolean/triangulate.h

SOURCES += simpleboolean/axisalignedboundingboxtree.cpp
HEADERS += simpleboolean/axisalignedboundingboxtree.h

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

SOURCES += cgalmesh.cpp
HEADERS += cgalmesh.h

SOURCES += positionkey.cpp
HEADERS += positionkey.h

SOURCES += misc.cpp
HEADERS += misc.h

macx {
	GMP_LIBNAME = gmp
	MPFR_LIBNAME = mpfr
	CGAL_LIBNAME = cgal
	BOOST_INCLUDEDIR = /usr/local/opt/boost/include
	CGAL_INCLUDEDIR = /usr/local/opt/cgal/include
	CGAL_BUILDINCLUDEDIR = /usr/local/opt/cgal/include
	CGAL_LIBDIR = /usr/local/opt/cgal/lib
	GMP_INCLUDEDIR = /usr/local/opt/gmp/include
	GMP_LIBDIR = /usr/local/opt/gmp/lib
	MPFR_INCLUDEDIR = /usr/local/opt/mpfr/include
	MPFR_LIBDIR = /usr/local/opt/mpfr/lib
}

INCLUDEPATH += $$BOOST_INCLUDEDIR

INCLUDEPATH += $$GMP_INCLUDEDIR
LIBS += -L$$GMP_LIBDIR -l$$GMP_LIBNAME

INCLUDEPATH += $$MPFR_INCLUDEDIR
LIBS += -L$$MPFR_LIBDIR -l$$MPFR_LIBNAME

INCLUDEPATH += $$CGAL_INCLUDEDIR
INCLUDEPATH += $$CGAL_BUILDINCLUDEDIR
LIBS += -L$$CGAL_LIBDIR -l$$CGAL_LIBNAME

QMAKE_CXXFLAGS += -std=c++11

target.path = ./
INSTALLS += target
