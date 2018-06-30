#include "util.h"
#include <iostream>
#include"net.h"


int main(void)
{


    if (!CreateThread(StartNode, NULL))
    {
        std::cout<<"error"<<std::endl; 
    }

    while(1);

}


