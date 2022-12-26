#include "trigrammdictionary.h"

#include <time.h>
#include <iostream>


int main(int argc, char** argv)
{
    // check command line arguments: must be 1 arg
    if (argc < 2)
    {
        std::cout << "Must be input file with words!" << std::endl;
        return 1;
    }

    // TIMER
    clock_t start, stop;

    // SYNC
    int sync = 1; // one thread
    TrigrammDictionary tri;
    // start timer
    start = clock();
    tri.fileRead(argv[1], sync);
    // stop timer
    stop = clock();
    tri.printTop(10);
    // print timer for sync search
    std::cout << "SYNC TIME: " << ((stop - start) / (float)CLOCKS_PER_SEC) << std::endl;

    // ASYNC
    int async = 4; // four threads
    TrigrammDictionary tri2;
    // start timer
    start = clock();
    tri2.fileRead(argv[1], sync);
    // stop timer
    stop = clock();
    tri2.printTop(10);
    // print timer for async search
    std::cout << "ASYNC TIME: " << ((stop - start) / (float)CLOCKS_PER_SEC) << std::endl;

    return 0;
}
