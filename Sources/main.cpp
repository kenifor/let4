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

    // прочитать файл
    // сделать список слов из триграмм (слов с тремя символами)
    // если слово встретилось впервые, тогда добавить в список с счетчиком 1
    // если слово встречалась ранее, тогда найти его в списке и увеличить его счетчик на 1
    // когда файл прочитан, тогда нужно отсортировать получившийся список слов по убыванию их счетчиков
    // закрыть файл

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
