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
        threads.push_back(std::thread(&TrigrammDictionary::createTrigrammList, this, std::ref(words)));
    }

    // основной поток ожидает выполнения созданных потоков (в предыдущем цикле)
    for (int i = 0; i < treadsCount; i++)
    {
        // указываем основному потоку, чтобы он дождался работы дочерних потоков,
        // которые занимаются созданием списка триграмм
        threads[i].join();
    }
    sort();

    return true;
}

std::pair<std::string, int>& TrigrammDictionary::operator[](int index)
{
    return trigrams_list[index];
}

void TrigrammDictionary::createTrigrammList(std::vector<std::string>& words)
{
    std::string word;

    while(true)
    {
        mtxWordCount.lock();

        // текущий поток проверяет, не закончились ли слова в списке
        if (wordCount >= words.size())
        {
            mtxWordCount.unlock();
            // и завершает словю работу
            return;
        }

        // запоминаем текущее слово и увеличиваем глобальную переменную-счетчик на единицу (чтобы следующий поток взял другое слово из списка)
        int i = wordCount++;
        mtxWordCount.unlock();

        // приводим к нижнему регистру текущее слово, чтобы в словаре не было повторений слов
        transformToLowerCase(words[i]);

        // проверяем что слово состоит из 3 букв
        if(isTrigramm(words[i]))
        {
            bool wordExist = false;

            // проверим есть ли слово (триграмма) в словаре (списке триграмм)
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

void TrigrammDictionary::sort()
{
    if (trigrams_list.size() < 2)
        return;

    for (int i = 0; i < trigrams_list.size() - 1; i++)
    {
        int current = i;
        int max_index = current;

        for (int j = current + 1; j < trigrams_list.size(); j++)
        {
            if (trigrams_list[j].second > trigrams_list[max_index].second)
                max_index = j;
        }

        if (max_index != current)
            std::swap(trigrams_list[max_index], trigrams_list[current]);
    }
}

void TrigrammDictionary::transformToLowerCase(std::string& word)
{
    //пройтись по каждому символу слова и преобразовать символ
    for(char& symbol: word)
    {
        if (symbol >= 65 && symbol <= 90)
        {
            symbol += 32;
        }
    }
}
