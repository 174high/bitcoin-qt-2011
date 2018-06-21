CONFIG += qt
#QT -= 

unix:LIBS += -ldb_cxx  -lboost_filesystem -lboost_system -lboost_thread 

SOURCES += main.cpp \
           net.cpp \ 
           util.cpp        
#           bitcoingui.cpp \
#           overviewpage.cpp      

HEADERS += serialize.h \
           net.h  \
           util.h \
           uint256.h \
   
#bitcoingui.h \
#          overviewpage.h

RESOURCES += \
#          bitcoin.qrc

FORMS += \   
#          forms/overviewpage.ui \

TRANSLATIONS = hellotr_la.ts
