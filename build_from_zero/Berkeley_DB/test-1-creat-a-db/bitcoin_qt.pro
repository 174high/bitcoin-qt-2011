CONFIG += qt
#QT -= 

unix:LIBS += -ldb_cxx

SOURCES += main.cpp \
#           bitcoingui.cpp \
#           overviewpage.cpp      

HEADERS += 
#bitcoingui.h \
#          overviewpage.h

RESOURCES += \
#          bitcoin.qrc

FORMS += \   
#          forms/overviewpage.ui \

TRANSLATIONS = hellotr_la.ts
