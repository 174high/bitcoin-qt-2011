#include "headers.h"
#include "util.h"
#include <iostream>
#include "main.h"
#include"net.h"

int nBestHeight = -1;





// The message start string is designed to be unlikely to occur in normal data.
// The characters are rarely used upper ascii, not valid as UTF-8, and produce
// a large 4-byte int at any alignment.
char pchMessageStart[4] = { 0xf9, 0xbe, 0xb4, 0xd9 };


int main(void)
{


    if (!CreateThread(StartNode, NULL))
    {
        std::cout<<"error"<<std::endl; 
    }

    while(1);

}


