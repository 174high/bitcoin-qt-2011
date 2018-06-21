CONFIG += qt
#QT -= 
INCLUDEPATH +=cryptopp


unix:LIBS +=  -ldb_cxx -lcrypto   -lboost_filesystem -lboost_system -lboost_thread 

SOURCES += main.cpp \
           net.cpp \ 
           util.cpp \       
           cryptopp/sha.cpp \
           cryptopp/cpu.cpp

HEADERS += serialize.h \
           net.h  \
           util.h \
           uint256.h \
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
   

RESOURCES += \
#          bitcoin.qrc

FORMS += \   
#          forms/overviewpage.ui \

TRANSLATIONS = hellotr_la.ts
