HEADERS     = src/teteco.h   src/proxy.h   src/configuration_window.h   src/statistics_window.h   src/data_plot.h   src/documentwidget.h
SOURCES     = src/teteco.cpp src/proxy.cpp src/configuration_window.cpp src/statistics_window.cpp src/data_plot.cpp src/documentwidget.cpp src/main.cpp
FORMS       = src/teteco.ui  src/configuration_window.ui  src/statistics_window.ui
LIBS        += $$(PREFIX)/lib/teteco.a \
			   #$$(PREFIX)/lib/libqwt.a \
			   $$(PREFIX)/lib/libqwtd.a \
			   #../libs/poppler-0.16.3/fofi/.libs/libfofi.a \
			   #../libs/poppler-0.16.3/goo/.libs/libgoo.a \
			   #../libs/poppler-0.16.3/splash/.libs/libsplash.a \
			   #../libs/poppler-0.16.3/poppler/.libs/libpoppler-arthur.a \
			   $$(PREFIX)/lib/libpoppler-qt4.a \
			   $$(PREFIX)/lib/libpoppler.a \
			   $$(PREFIX)/lib/libfreetype.a
			   
			   
INCLUDEPATH += $$(PREFIX)/include
RESOURCES   += src/teteco.qrc
RC_FILE     = teteco.rc
QMAKE_CXXFLAGS += -D__WINDOWS__ -DQT_NO_DEBUG -U_WIN32 -UPOPPLER_QT4_EXPORT

##QTPLUGIN     += xml
QT += xml

CONFIG += static
static {

    CONFIG += static
    QTPLUGIN +=

    DEFINES += STATIC 
    message("Static build.")
}
win32 {
	QMAKE_LFLAGS += -static-libgcc -static-libstdc++ -static
}

# include(./qwt-5.2.1/qwtconfig.pri)

# install
target.path = teteco
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro
sources.path = .
INSTALLS += target

