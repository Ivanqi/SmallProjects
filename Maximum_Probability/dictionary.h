#ifndef DICTIONARY_H
#define DICTIONARY_H
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <cstdlib>
using namespace std;

// 词头文件


// 词典定义，用于最大概率分词
class Dictionary
{
    private:
        string strline;             // 保存每行内容
        string word;                // 保存一个词语
        map<string, int> word_map;  // 词典，用map表示

    public:
        long size;                  // 词典规模
        long freq_all;              // 词典总数
        long arr_1[20];
        double arr_2[20];
        Dictionary();
        ~Dictionary();              // 构造函数，初始化词典
        int findWord(string word);  // 在词典中查找特定的词语
};

Dictionary::Dictionary()
{
    freq_all = 0;
    for (int i = 0; i < 20; i++) {
        arr_1[i] = 0;
        arr_2[i] = 0.0;
    }

    // 读取词典文件
    fstream fin("./data/dict.txt");
    if (!fin) {
        cerr << "open file error !" << endl;
        exit(-1);
    }

    // 将每个词加入集合
    while (getline(fin, strline, '\n')) {
        std::size_t found = strline.find(" ");
        istringstream istr(strline.substr(0, found));
        istr >> word;           // 从流中读取单词
        ++word_map[word];
        ++arr_1[word.size()];
        ++freq_all;
    }

    fin.close();

    // 初始化词典大小
    size = word_map.size();
    for (int i = 0; i < 20; i++) {
        arr_2[i] = (double) arr_1[i] / freq_all;
    }
}

Dictionary::~Dictionary()
{

}

int Dictionary::findWord(string word)
{
    map<string, int>::iterator p_cur = word_map.find(word);
    if (p_cur != word_map.end()) {
        return p_cur->second;
    } else {
        return -1;
    }
}
#endif