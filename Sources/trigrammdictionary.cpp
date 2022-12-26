#include "trigrammdictionary.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <functional> // for std::ref

// пустой конструктор, чтобы была возможность создать объект класса TrigrammDictionary без создания словаря,
// а словарь можно будет создать позднее, вызвав функцию fileRead()
TrigrammDictionary::TrigrammDictionary()
{

}

// дуструктор существует по умолчанию

// конструктор с параметрами, который при создании объекта класса создает и словарь триграмм
TrigrammDictionary::TrigrammDictionary(std::filesystem::path pathFile, int treadsCount)
{
    fileRead(pathFile, treadsCount);
}

void TrigrammDictionary::printTop(int top)
{
    for (int i = 0; i < top; i++)
    {
        std::cout << "TOP " << i+1 << " - " << trigrams_list[i].first << " = " << trigrams_list[i].second << std::endl;
    }
}

int TrigrammDictionary::size() const
{
    return trigrams_list.size();
}

bool TrigrammDictionary::fileRead(std::filesystem::path pathFile, int treadsCount)
{
    // переменная, по которой потоки обращаются к словам в списке слов (потоки не должны прочитывать слова по несколько раз)
     wordCount = 0;

    // открываем файл на чтение, чтобы считать список слов
    std::ifstream file;
    file.open(pathFile, std::ios_base::in);

    // проверяем файл на открытие
    if (file.is_open() == false)
    {
        return false;
    }

    std::string word;
    std::vector<std::string> words;

    file >> word;

    // если файл пустой,тогда функция завершает работу
    if (word == "")
        return false;

    while (file.eof() == false)
    {
        words.push_back(word);
        file >> word;
    }

    file.close();

    if (words.empty())
    {
       return false;
    }

    // использовать потоки, чтобы считывать слова из файла

    // создаём пустой список потоков
    std::vector<std::thread> threads;

    // создаем определенное количество потоков
    for (int i = 0; i < treadsCount; i++)
    {
        // создаеём новый поток и запускаем с функцией createTrigrammList
        // при создании потока в скобках передается три параметра
        // 1 - указатель на функцию createTrigrammList() класса TrigrammDictionary
        // 2 - уазатель на текущий объект класса TrigrammDictionary
        // 3 - ссылку (адрес) на список слов (без копирования)
        threads.push_back(std::thread(&TrigrammDictionary::createTrigrammList, this, std::ref(words)));
    }

    // основной поток ожидает выполнения созданных потоков (в предыдущем цикле)
    for (int i = 0; i < treadsCount; i++)
    {
        // указываем основному потоку, чтобы он дождался работы дочерних потоков,
        // которые занимаются созданием списка триграмм
        threads[i].join();
    }

    // сортируем список триграмм по частоте появления триграмм
    sort();

    return true;
}

// функция (перегруженный оператор квадратные скобки) возвращает пару (триграмма и количество ее встречь) из списка триграмм
// по ссылке, то есть то, что уже существует (без копирования)
std::pair<std::string, int>& TrigrammDictionary::operator[](int index)
{
    return trigrams_list[index];
}

// функция получает сырой список всех слов и записывает их в объект класса TrigrammDictionary
// эта функция вызывается несколько раз (один раз в каждом созданном потоке)
void TrigrammDictionary::createTrigrammList(std::vector<std::string>& words)
{
    std::string word;

    while(true)
    {
        // переменная wordCount (количество слов) - общая для всех потоков
        // Эта переменная сигнализирует всем потокам, сколько слов обработано из сырого списка слов (words)
        mtxWordCount.lock();

        // текущий поток проверяет, не закончились ли слова в списке
        if (wordCount >= words.size())
        {
            // Если глобальная переменная-счетчик достигла размера списка слов, то это означает, что список слов
            // закончен. И текущему потоку нечего больше обрабатывать.
            // Поток разблокирует работу остальных потоков
            mtxWordCount.unlock();
            // и завершает словю работу
            return;
        }

        // запоминаем текущее слово и увеличиваем глобальную переменную-счетчик на единицу (чтобы следующий поток взял другое слово из списка)
        int i = wordCount++;
        mtxWordCount.unlock();

        // приводим к нижнему регистру текущее слово, чтобы в словаре не было повторений слов
        transformToLowerCase(words[i]);

        // check word consist three letters
        if(isTrigramm(words[i]))
        {
            // данный параметр (флаг) говорит о том, что в словаре нет рассматриваемой триграммы (words[i])
            bool wordExist = false;

            // проверить есть ли слово (триграмма) в словаре (списке триграмм)
            for(auto& trigramma: trigrams_list)
            {
                mtxTrigList.lock();
                // если есть, тогда увеличить его число встреч на один
                if(words[i] == trigramma.first)
                {
                    trigramma.second += 1;
                    wordExist = true;
                    mtxTrigList.unlock();
                    break;
                }
                mtxTrigList.unlock();
            }

            // если триграммы нет в словаре, тогда добавить это слово с числом встреч 1
            if (wordExist == false)
            {
                mtxTrigList.lock();
                // чтобы добавить пару (std::pair) в вектор, нужно std::make_pair(fileWord, 1)
                // слово fileWord встретилось впервые, то есть 1 (один раз)
                trigrams_list.push_back(std::make_pair(words[i], 1));
                mtxTrigList.unlock();
            }
        }
    }
}

bool TrigrammDictionary::isTrigramm(std::string word) const
{
    if (word.size() == 3)
        return true;
    else
        return false;
}

// функция сортирует список триграмм по частоте появления триграмм
// т.е. та триграмма, которая чаще всего встречается в тексте - она будет первая в списке триграмм
void TrigrammDictionary::sort()
{
    // если размер массива задан как нуль или один,тогда сортировать нечего
    if (trigrams_list.size() < 2)
        return;

    /* Пройтись по всему массиву от первого до предпоследнего элемента.
     * Последний элемент будет стоять на своём месте под конец цикла.
     * */
    for (int i = 0; i < trigrams_list.size() - 1; i++)
    {
        int current = i;
        int max_index = current;

        // Пройтись по массиву от следующего элемента(current+1) до последнего элемента массива.
        for (int j = current + 1; j < trigrams_list.size(); j++)
        {
            // Если текущий элемент(j) больше, чем текущий максимальный(max_index), то нужно запомнить его индекс.
            if (trigrams_list[j].second > trigrams_list[max_index].second)
                max_index = j;
        }

        // После сравнения нужно поменять местами текущий элемент(current) и максимальный элемент(min_index).
        // Только если это не оказался один и тот же элемент.
        // переставляются местами целые пары (слово-количество встреч в тексте)
        if (max_index != current)
            std::swap(trigrams_list[max_index], trigrams_list[current]);
    }
}

void TrigrammDictionary::transformToLowerCase(std::string& word)
{
    //пройтись по каждому символу слова и преобразовать символ
    for(char& symbol: word)
    {
        // преобразуем только те символы, которые в верхнем регистре лат. алфавита
        // числа 65 и 90 это символы A и Z из таблицы ASCII
        if (symbol >= 65 && symbol <= 90)
        {
            // добавляем к символу 32, чтобы из верхнего регистра получить нижний по таблице ASCII
            symbol += 32;
        }
    }
}
