CONFIG += qt
#QT -= 
INCLUDEPATH +=cryptopp

unix:LIBS += -ldb_cxx -lcrypto -lboost_filesystem -lboost_system

DEPENDPATH +=cryptopp 

SOURCES += main.cpp \
           cryptopp/sha.cpp \
           cryptopp/cpu.cpp 
#           bitcoingui.cpp \
#           overviewpage.cpp      

HEADERS += serialize.h \
           uint256.h \
           key.h \
           bignum.h \
           base58.h \
           cryptopp/stdcpp.h \
           cryptopp/smartptr.h \
           cryptopp/simple.h \
           cryptopp/sha.h \
           cryptopp/secblock.h \
           cryptopp/pch.h \
           cryptopp/misc.h \
           cryptopp/iterhash.h \
           cryptopp/cryptlib.h \
           cryptopp/cpu.h \
           cryptopp/config.h 

#bitcoingui.h \
#          overviewpage.h

RESOURCES += \
#          bitcoin.qrc

FORMS += \   
#          forms/overviewpage.ui \

TRANSLATIONS = hellotr_la.ts
