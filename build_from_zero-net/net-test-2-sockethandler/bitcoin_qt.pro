CONFIG += qt
#QT -= 
DEFINES += QT_GUI DEBUG_NET DEBUG_IRC
INCLUDEPATH +=cryptopp


unix:LIBS +=  -ldb_cxx -lcrypto   -lboost_filesystem -lboost_system -lboost_thread 

SOURCES += main.cpp \
           irc.cpp \
           net.cpp \ 
           db.cpp \
           util.cpp \       
           cryptopp/sha.cpp \
           cryptopp/cpu.cpp

HEADERS += serialize.h \
           headers.h \
           irc.h  \
           net.h  \
           db.h   \
           util.h \
           main.h \
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
