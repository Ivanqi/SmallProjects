#include <cmath>
#include <string>
#include <iostream>
#include "dictionary_2.h"

const short MaxWordLength = 20;     // 词典中最大词的长度
const short Separator = '/';        // 词界标记

Dictionary word_dict;               // 初始化一个词典

/**
 * 类定义： 候选词结构
 */
class Candidate
{
    public:
        short pos;                 // 候选词在输入串的起点
        short length;              // 输入串的长度
        short bestPrev;            // 最佳前趋词的序号
        float fee;                 // 候选词的费用
        float sumFee;              // 候选词路径上的累计费用
        string word;               // 候选词
        int freq;                  // 候选词的频数(不能用short，否则有可能溢出)
};

/**
 * 函数功能: 取出字符串中的全部候选词
 * 函数输入: 字符串的引用
 * 函数输出: 该字符中含有的所有的存在与词典中的词(或者单字，单字可以在词典中不存在)
 */
vector<Candidate> getTmpWords(const string &s)
{
    int freq = 0;                 // 词典中词的频率
    short n = s.length();         // 字符串的长度
    string word = "";             // 存放候选词
    Candidate cand;               // 存放候选词属性
    vector<Candidate> vec_cd;     // 候选词队列

    // 以每个汉字为起点
    for (short i = 0; i < n; i += 2) {
        // 词的长度为 1 ~ MaxWordLength / 2 个汉字
        for (short len = 2; len <= MaxWordLength; len += 2) {
            word = s.substr(i, len);
            freq = word_dict.findWord(word);    // 去词典中查找出现频率
            if (len > 2 && freq == -1) {
                // 若不止一字且词表中找不到则不予登录
                continue;
            }

            if (freq == -1) {
                // 如果为单字词，且词表中找不到
                freq = 0;
            }

            cand.pos = i;           // 该候选词在汉字串中的起点
            cand.length = len;      // 该候选词的长度
            cand.word = word;
            cand.fee = -log((double)(freq * 1 + 1) / word_dict.freq_all);  // 该候选词的费用
            cand.sumFee = 0.0f;     // 该候选词的累计费用置初值
            cand.freq = freq;
            // 将获取的候选词加入队列
            vec_cd.push_back(cand);
        }
    }

    return vec_cd;
}

/**
 * 函数功能: 获取最佳前趋词序号
 * 函数输入: 候选词列表的引用
 * 函数输出: 无
 */
void getPrew(vector<Candidate> &vec_cd)
{
    short min_id = -1;                       // 最佳前趋词编号
    short j = -1;
    short size = (short) vec_cd.size();     // 计算队列长度

    for (short i = 0; i < size; i++) {
        if (vec_cd[i].pos == 0) {
            // 如果候选词是汉字串中的首词
            vec_cd[i].bestPrev = -1;            // 无前趋词
            vec_cd[i].sumFee = vec_cd[i].fee;   // 累计费用为该词本身费用
        } else {
            // 如果候选词不是汉字串中的首词
            min_id = -1;                        // 初始化最佳前趋向编号
            j = i - 1;                          // 从当前对象向左找

            while (j >= 0) {
                // 向左寻找所遇到的所有前趋词
                if (vec_cd[j].pos + vec_cd[j].length == vec_cd[i].pos) {
                    if (min_id == -1 || vec_cd[j].sumFee < vec_cd[min_id].sumFee) {
                        min_id = j;
                    }
                }
                --j;
            }

            vec_cd[i].bestPrev = min_id;       // 登记最佳前趋编号
            vec_cd[i].sumFee = vec_cd[i].fee + vec_cd[min_id].sumFee;   // 登记最小累计费用
        }
    }
}

/**
 * 函数功能: 最大概率法分词
 * 函数输入: 待切分的字符串
 * 函数输出: 切分好的字符串
 */
string segmentSentence_MP(string s1)
{
    short len = s1.length();
    short min_id = -1;      // 最小费用路径的终点词的序号

    // 取出s1中的全部候选词
    vector<Candidate> vec_cd = getTmpWords(s1);

    // 获取最佳前趋词序号，当前词最小累计费用
    getPrew(vec_cd);

    // 确定最小费用路径的终点词的序号
    short n = (short)vec_cd.size();
    for (short i = 0; i < n; i++) {
        if (vec_cd[i].pos + vec_cd[i].length == len) {
            // 如果当前词是s1的尾词
            if (min_id == -1 || vec_cd[i].sumFee < vec_cd[min_id].sumFee) {
                // 如果是第一个遇到的尾词，或者是当前尾词的最小累计费用小于已经遇到过的任一尾词的最小累计费用，则将其序号赋给min_id
                min_id = i;
            }
        }
    }

    // 构造输出串
    string s2 = "";     // 输出串初始化
    for (short i = min_id; i >= 0; i = vec_cd[i].bestPrev) {
        // 注意: 是先取后面的词
        s2 = s1.substr(vec_cd[i].pos, vec_cd[i].length) + Separator + s2;
    }

    return s2;
}

/**
 * 函数功能: 对字符串用最大匹配算法(正向)处理
 * 函数输入: 汉字字符串
 * 函数输出: 分好词的字符串
 */
string segmentSentence_1(string s1)
{
    string s2 = ""; // 用s2存放分词结果

    while (!s1.empty()) {
        int len = s1.length();      // 取输入串长度
        if (len > MaxWordLength) {
            len = MaxWordLength;    // 只在最大词长范围内进行处理
        }

        string w = s1.substr(0, len);
        int n = word_dict.findWord(w);  // 在词典中查找相应的词

        while (len > 2 && n == -1) {
            len -= 2;                   // 从候选词右边减掉一个汉字，将剩下的部分作为候选词
            w = s1.substr(0, len);
            n = word_dict.findWord(w);
        }

        s2 = s2 + w + Separator;
        s1 = s1.substr(w.length(), s1.length() - w.length());
    }

    return s2;
}

/**
 * 函数功能: 对字符串用最大匹配算法(逆向)处理
 * 函数输入: 汉字字符串
 * 函数输出: 分好词的字符串
 */
string segmentSentence_2(string s1)
{
    string s2 = "";                 // 用s2存放分词结果

    while (!s1.empty()) {
        int len = s1.length();      // 取输入串长度
        if (len > MaxWordLength) {
            len = MaxWordLength;    // 只在最大词长范围内进行处理
        }

        string w = s1.substr(s1.length() - len, len);
        int n = word_dict.findWord(w);  // 在词典中查找相应的词

        while (len > 2 && n == -1) {
            len -= 2;                   // 从候选词左边减掉一个汉字，将剩下的部分作为候选词
            w = s1.substr(s1.length() - len, len);
            n = word_dict.findWord(w);
        }

        w = w + Separator;
        s2 = w + s2;
        s1 = s1.substr(0, s1.length() - len);
    }

    return s2;
}