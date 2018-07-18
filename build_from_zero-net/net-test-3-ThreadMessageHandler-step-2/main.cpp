#include "headers.h"
#include "util.h"
#include <iostream>
#include "main.h"
#include "net.h"
#include "db.h"

using namespace std; 

CCriticalSection cs_main;

int nBestHeight = -1;



bool ProcessMessages(CNode* pfrom)
{
    CDataStream& vRecv = pfrom->vRecv;

#ifdef DEBUG_NODE&&DEBUG_MAIN
    std::cout<<__FUNCTION__<<std::endl; 
    std::cout<<"1: vRecv size= "<<vRecv.size()<<std::endl; 
    std::cout<<"2: vRecv empty= "<<vRecv.empty()<<std::endl; 
#endif 

    if (vRecv.empty())
        return true;

#ifdef DEBUG_NODE&&DEBUG_MAIN
    std::cout<<"3: vRecv size= "<<vRecv.size()<<std::endl;
#endif

    //if (fDebug)
    //    printf("ProcessMessages(%u bytes)\n", vRecv.size());

    //
    // Message format
    //  (4) message start
    //  (12) command
    //  (4) size
    //  (4) checksum
    //  (x) data
    //

    loop
    {
        // Scan for message start
        CDataStream::iterator pstart = search(vRecv.begin(), vRecv.end(), BEGIN(pchMessageStart), END(pchMessageStart));

#ifdef DEBUG_NODE&&DEBUG_MAIN
//        std::cout<<" CMessageHeader()  "<<CMessageHeader()<<std::endl;
#endif        
        int nHeaderSize = vRecv.GetSerializeSize(CMessageHeader());
#ifdef DEBUG_NODE&&DEBUG_MAIN
//	std::cout<<" nHeaderSize= "<<nHeaderSize<<std::endl;
#endif

        if (vRecv.end() - pstart < nHeaderSize)
        {
            if (vRecv.size() > nHeaderSize)
            {
                printf("\n\nPROCESSMESSAGE MESSAGESTART NOT FOUND\n\n");
                vRecv.erase(vRecv.begin(), vRecv.end() - nHeaderSize);
            }
            break;
        }

#ifdef DEBUG_NODE&&DEBUG_MAIN
//    std::cout<<" pstart - vRecv.begin()= "<<(pstart - vRecv.begin())<<std::endl;
#endif

        if (pstart - vRecv.begin() > 0)
            printf("\n\nPROCESSMESSAGE SKIPPED %d BYTES\n\n", pstart - vRecv.begin());
        vRecv.erase(vRecv.begin(), pstart);

        // Read header
        vector<char> vHeaderSave(vRecv.begin(), vRecv.begin() + nHeaderSize);
        CMessageHeader hdr;
        vRecv >> hdr;
        if (!hdr.IsValid())
        {
            printf("\n\nPROCESSMESSAGE: ERRORS IN HEADER %s\n\n\n", hdr.GetCommand().c_str());
            continue;
        }
        string strCommand = hdr.GetCommand();

        // Message size
        unsigned int nMessageSize = hdr.nMessageSize;
        if (nMessageSize > MAX_SIZE)
        {
            printf("ProcessMessage(%s, %u bytes) : nMessageSize > MAX_SIZE\n", strCommand.c_str(), nMessageSize);
            continue;
        }
        if (nMessageSize > vRecv.size())
        {
            // Rewind and wait for rest of message
            vRecv.insert(vRecv.begin(), vHeaderSave.begin(), vHeaderSave.end());
            break;
        }

        // Checksum
        if (vRecv.GetVersion() >= 209)
        {
            uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
            unsigned int nChecksum = 0;
            memcpy(&nChecksum, &hash, sizeof(nChecksum));
            if (nChecksum != hdr.nChecksum)
            {
                printf("ProcessMessage(%s, %u bytes) : CHECKSUM ERROR nChecksum=%08x hdr.nChecksum=%08x\n",
                       strCommand.c_str(), nMessageSize, nChecksum, hdr.nChecksum);
                continue;
            }
	    else
	    {
		 printf("ProcessMessage(%s, %u bytes) : CHECKSUM correct nChecksum=%08x hdr.nChecksum=%08x\n",
                       strCommand.c_str(), nMessageSize, nChecksum, hdr.nChecksum);
	    }
        }

        // Copy message to its own buffer
        CDataStream vMsg(vRecv.begin(), vRecv.begin() + nMessageSize, vRecv.nType, vRecv.nVersion);
        vRecv.ignore(nMessageSize);

        // Process message
        bool fRet = false;
        try
        {
            CRITICAL_BLOCK(cs_main)
//                fRet = ProcessMessage(pfrom, strCommand, vMsg);
            if (fShutdown)
                return true;
        }
        catch (std::ios_base::failure& e)
        {
            if (strstr(e.what(), "end of data"))
            {
                // Allow exceptions from underlength message on vRecv
                printf("ProcessMessage(%s, %u bytes) : Exception '%s' caught, normally caused by a message being shorter than its stated length\n", strCommand.c_str(), nMessageSize, e.what());
            }
            else if (strstr(e.what(), "size too large"))
            {
                // Allow exceptions from overlong size
                printf("ProcessMessage(%s, %u bytes) : Exception '%s' caught\n", strCommand.c_str(), nMessageSize, e.what());
            }
            else
            {
                PrintExceptionContinue(&e, "ProcessMessage()");
            }
        }
        catch (std::exception& e) {
            PrintExceptionContinue(&e, "ProcessMessage()");
        } catch (...) {
            PrintExceptionContinue(NULL, "ProcessMessage()");
        }

        if (!fRet)
            printf("ProcessMessage(%s, %u bytes) FAILED\n", strCommand.c_str(), nMessageSize);

    }

    vRecv.Compact();
    return true;
}



// The message start string is designed to be unlikely to occur in normal data.
// The characters are rarely used upper ascii, not valid as UTF-8, and produce
// a large 4-byte int at any alignment.
char pchMessageStart[4] = { 0xf9, 0xbe, 0xb4, 0xd9 };


int main(void)
{

    if (!LoadAddresses()) ;

    if (!CreateThread(StartNode, NULL))
    {
        std::cout<<"error"<<std::endl; 
    }

    while(1);

}


