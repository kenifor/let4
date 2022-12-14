#include "trigrammdictionary.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <functional> // for std::ref

TrigrammDictionary::TrigrammDictionary()
{

}

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

    // open file (read mode)
    std::ifstream file;
    file.open(pathFile, std::ios_base::in);

    // check file opening
    if (file.is_open() == false)
    {
        return false;
    }

    std::string word;
    std::vector<std::string> words;

    while (file.eof() == false)
    {
        file >> word;

        // if word is empty then dont add to the vector of words
        if (word == "")
            continue;

        words.push_back(word);
    }
    file.close();

    if (words.empty())
    {
        return false;
    }

    // использовать потоки, чтобы считывать слова из файла

    // create the list (vector) of threads using smart pointer
    std::vector<std::unique_ptr<std::thread>> threads;

    for (int i = 0; i < treadsCount; i++)
    {
        // create new thread and run it with function readText
        threads.push_back(std::make_unique<std::thread>(&TrigrammDictionary::readText, this, std::ref(words)));
    }

    for (int i = 0; i < treadsCount; i++)
    {
        // main thread must wait other threas
        threads[i]->join();
    }

    sort();

    return true;
}

std::pair<std::string, int>& TrigrammDictionary::operator[](int index)
{
    return trigrams_list[index];
}

void TrigrammDictionary::readText(std::vector<std::string>& words)
{
    std::string word;

    while(true)
    {
        mtx.lock();
        if (wordCount >= words.size())
        {
            mtx.unlock();
            return;
        }

        int i = wordCount++; // запоминаем текущее слово и увеличиваем глобальную переменную-счетчик на единицу (чтобы следующий поток взял другое слово из списка)
        mtx.unlock();

        transformToLowerCase(words[i]); // приводим к нижнему регистру

        // check word consist three letters
        if(isTrigramm(words[i]))
        {
            // данный параметр (флаг) говорит о том, что в словаре нет рассматриваемой триграммы (words[i])
            bool wordExist = false;

            // проверить есть ли слово (триграмма) в словаре (списке триграмм)
            for(auto& trigramma: trigrams_list)
            {
                mtx.lock();
                // если есть, тогда увеличить его число встреч на один
                if(words[i] == trigramma.first)
                {
                    trigramma.second += 1;
                    wordExist = true;
                    mtx.unlock();
                    break;
                }
                mtx.unlock();
            }

            // если триграммы нет в словаре, тогда добавить это слово с числом встреч 1
            if (wordExist == false)
            {
                mtx.lock();
                // чтобы добавить пару (std::pair) в вектор, нужно std::make_pair(fileWord, 1)
                // слово fileWord встретилось впервые, то есть 1 (один раз)
                trigrams_list.push_back(std::make_pair(words[i], 1));
                mtx.unlock();
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
