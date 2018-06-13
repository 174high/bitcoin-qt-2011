CONFIG += qt
#QT -= 

unix:LIBS += -ldb_cxx -lcrypto -lboost_filesystem -lboost_system

SOURCES += main.cpp \
#           bitcoingui.cpp \
#           overviewpage.cpp      

HEADERS += serialize.h \
           uint256.h \
           main.h \
           bignum.h \
           util.h

#bitcoingui.h \
#          overviewpage.h

RESOURCES += \
#          bitcoin.qrc

FORMS += \   
#          forms/overviewpage.ui \

TRANSLATIONS = hellotr_la.ts
