HEADERS     = interface.h   configuration_window.h   statistics_window.h   data_plot.h
SOURCES     = interface.cpp configuration_window.cpp statistics_window.cpp data_plot.cpp main.cpp
FORMS       = interface.ui  configuration_window.ui  statistics_window.ui
LIBS        += -L../src -L./qwt-5.2.1/lib -lteteco -lqwt5
INCLUDEPATH += ../src ./qwt-5.2.1/src ../libs/win/portaudio/include ../libs/win/speex-1.2rc1/include
RESOURCES   += interface.qrc
QMAKE_CXXFLAGS += -D__WINDOWS__

# include(./basicgraph/basicgraph.pri)
include(./qwt-5.2.1/qwtconfig.pri)

# install
target.path = teteco
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro
sources.path = .
INSTALLS += target

