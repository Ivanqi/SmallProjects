#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <iomanip>

using namespace std;

class CDictionary
{
    private:
        string strline;                  // 读取词典的每一行
        string word;                    // 保存每个词
        map<string, int> wordhash;      // 用于读取词典后的哈希
        map<string, int>::iterator worditer;
        typedef pair<string, int> sipair;

    public:
        CDictionary();                  // 将词典文件读入并构造一个哈希字典
        ~CDictionary();
        int FindWord(string w);         // 在哈希词典中查找词
};

// 将词典文件读入并构造一个哈希词典
CDictionary::CDictionary()
{
    ifstream infile("./data/wordlexicon.txt");     // 打开词典
    if (!infile.is_open()) {            // 打开词典失败则退出程序
        cerr << "Unable to open input file: " << "wordlexicon" << " -- bailing out!" << endl;
        exit(-1);
    }

    while (getline(infile, strline, '\n')) {    // 读入词典的每一行并将其添加入哈希中
        istringstream istr(strline);
        istr >> word;                           // 读入每行第一个词
        wordhash.insert(sipair(word, 1));       // 插入哈希中
    }
}

CDictionary::~CDictionary()
{

}

// 在哈希词典中查找词，若找到，则返回，否则返回
int CDictionary::FindWord(string w)
{
    if (wordhash.find(w) != wordhash.end()) {
        return 1;
    } else {
        return 0;
    }
}

#endif