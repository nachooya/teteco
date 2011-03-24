HEADERS     = src/teteco.h   src/proxy.h   src/configuration_window.h   src/statistics_window.h   src/data_plot.h   src/documentwidget.h
SOURCES     = src/teteco.cpp src/proxy.cpp src/configuration_window.cpp src/statistics_window.cpp src/data_plot.cpp src/documentwidget.cpp src/main.cpp
FORMS       = src/teteco.ui  src/configuration_window.ui  src/statistics_window.ui
LIBS        += -lteteco -lqwt-qt4 -lpoppler-qt4
INCLUDEPATH += /usr/include/poppler/qt4/ /usr/include/qwt-qt4/
RESOURCES   += src/teteco.qrc

# install
target.path = teteco
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro
sources.path = src/
INSTALLS += target

