CONFIG += qt
#QT -= 

unix:LIBS += -ldb_cxx  -lcrypto   -lboost_filesystem -lboost_system

SOURCES += main.cpp \
           db.cpp \
           util.cpp

HEADERS += serialize.h \
           net.h  \
           util.h \
           uint256.h \
           db.h \
           main.h \
           bignum.h \
           strlcpy.h

#bitcoingui.h \
#          overviewpage.h

RESOURCES += \
#          bitcoin.qrc

FORMS += \   
#          forms/overviewpage.ui \

TRANSLATIONS = hellotr_la.ts
