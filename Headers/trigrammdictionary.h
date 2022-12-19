#ifndef TRIGRAMMDICTIONARY_H
#define TRIGRAMMDICTIONARY_H

#include <string>
#include <filesystem>
#include <vector>
#include <mutex>

// пара элементов из списка это сама триграмма и её колличество встреч в списке
using Dictionary = std::vector<std::pair<std::string, int>>;

class TrigrammDictionary
{
public:

    TrigrammDictionary();

    TrigrammDictionary(std::filesystem::path pathFile, int treadsCount=1);

    void printTop(int top);

    int size() const;

    
    // используем ассоциативный массив
    // функция, которая получаем путь к файлу и считывает триграммы
    bool fileRead(std::filesystem::path pathFile, int treadsCount=1);

    std::pair<std::string, int>& operator[](int index);

private:
    Dictionary trigrams_list;
    std::mutex mtx;

    int wordCount;

    void readText(std::vector<std::string>& words);

    bool isTrigramm(std::string word) const;

    // сортировать список триграмм по их значениям встреч в тексте
    void sort();

    // функция для преобразования слова к нижнему регистру
    // transformToLowerCase(string &)
    void transformToLowerCase(std::string& word);
};

#endif // TRIGRAMMDICTIONARY_H
