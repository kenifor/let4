#include "trigrammdictionary.h"
#include <gtest/gtest.h>

TEST(TrigrammDictionary, checkEmptyFileOpening){
    TrigrammDictionary dict;
    bool readingStatus = dict.fileRead("D:\\qt\\let_4\\Tests\\testODIN.txt");

    ASSERT_EQ(readingStatus, false);

}

TEST(TrigrammDictionary, checkFileWithoutTrigrams){
    TrigrammDictionary dict;
    dict.fileRead("D:\\qt\\let_4\\Tests\\testDVA.txt");

    ASSERT_EQ(dict.size(), 0);
}

TEST(TrigrammDictionary, checkNePustoiFile){
    TrigrammDictionary dict;
    bool readingStatus = dict.fileRead("D:\\qt\\let_4\\Tests\\testTRI.txt");

    ASSERT_EQ(readingStatus, true);
}

TEST(TrigrammDictionary, checkFileWithOdnaTrigramma){
    TrigrammDictionary dict;
    dict.fileRead("D:\\qt\\let_4\\Tests\\testTRI.txt");

    ASSERT_EQ(dict.size(), 1);
}

TEST(TrigrammDictionary, DveTrigramsWithDifferentRegistor){
    TrigrammDictionary dict;
    dict.fileRead("D:\\qt\\let_4\\Tests\\testCHETURY.txt");

    ASSERT_EQ(dict.size(), 1);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
