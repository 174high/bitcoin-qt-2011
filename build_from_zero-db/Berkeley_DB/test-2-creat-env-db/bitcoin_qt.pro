CONFIG += qt
#QT -= 

unix:LIBS += -ldb_cxx  -lboost_filesystem -lboost_system

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
