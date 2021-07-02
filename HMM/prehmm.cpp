#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <map>
#include "util.h"

using namespace std;

/**
 * 函数功能
 *      将训练语料和测试语料中出现的汉字进行编码，将他们的对应关系存入文件
 *      格式: 汉字 - 编码,编码从0开始
 * 
 * 函数输入
 *      infile_1 训练语料文件名
 *      infile_2 训练语料文件名
 *      outfile 指定的输出文件名
 * 
 * 函数输出
 *      名为outfile的文件
 */
void makeDB(string infile_1, string infile_2, string outfile)
{
    // 读取输入文件
    ifstream fin_1(infile_1.c_str());
    ifstream fin_2(infile_2.c_str());

    if (!(fin_1 && fin_2)) {
        cerr << "makeDB: Open input file fail !" << endl;
        exit(-1);
    }

    // 打开输出文件
    ofstream fout(outfile.c_str());
    if (!fout) {
        cerr << "makeDB: Open output file fail!" << endl;
        exit(-1);
    }

    map<string, int> map_cchar;
    int id = -1;
    string line = "";
    string cchar = "";

    // 读取输入文件内容
    while (getline(fin_1, line)) {
        line = replace_all(line, "/", "");
        if (line.size() >= 3) {
            // 逐字读取
            for (int i = 0; i < line.size() - 1; i += 3) {
                cchar = line.substr(i, 3);
                if (map_cchar.find(cchar) == map_cchar.end()) {
                    ++id;
                    map_cchar[cchar] = id;
                }
            }
        }
    }

    while (getline(fin_2, line)) {
        line = replace_all(line, "/", "");
        if (line.size() >= 3) {
            // 逐字读取
            for (int i = 0; i < line.size() - 1; i += 3) {
                cchar = line.substr(i, 3);
                if (map_cchar.find(cchar) == map_cchar.end()) {
                    ++id;
                    map_cchar[cchar] = id;
                }
            }
        }
    }

    // 输出到文件
    map<string, int>::iterator iter;
    for (iter = map_cchar.begin(); iter != map_cchar.end(); ++iter) {
        fout << iter->first << " " << iter->second << endl;
    }

    fin_1.close();
    fin_2.close();
    fout.close();
}

/**
 * 函数功能:
 *      将训练语料每个汉字后面加入对应的BMES状态
 * 函数输入:
 *      infile 训练语料文件名
 *      outfile 指定的输出文件名
 * 
 * 函数输出: 名为outfile的文件
 */
void makeBMES(string infile, string outfile)
{
    ifstream fin(infile.c_str());
    ofstream fout(outfile.c_str());
    if (!(fin && fout)) {
        cerr << "makeBMES: Open file failed!" << endl;
        exit(-1);
    }

    string word_in = "";
    string word_out = "";
    string line_in = "";
    string line_out = "";

    while (getline(fin, line_in)) {
        if (line_in.size() > 1) {
            line_out.clear();
            line_in = replace_all(line_in, "/", " ");
            istringstream strstm(line_in);

            while (strstm >> word_in) {
                word_out.clear();
                if (word_in.size() % 3 != 0) {
                    cout << "单词不符合要求: " << word_in << endl;
                    continue;
                }

                int num = word_in.size() / 3;   // 单词中包含多少个汉字
                if (num == 0) {
                    continue;
                }

                if (num == 1) {
                    word_out = word_in;
                    word_out += "/S";
                } else {
                    // 复制单词中的第一个字
                    word_out.insert(word_out.size(), word_in, 0, 3);
                    word_out += "/B";
                    // 逐个复制单词中间的字
                    for (int i = 1; i < num - 1; i++) {
                        word_out.insert(word_out.size(), word_in, 3 * i, 3);
                        word_out += "/M";
                    }

                    // 复制单词中最后的汉字
                    word_out.insert(word_out.size(), word_in, 3 * num - 3, 3);
                    word_out += "/E";
                }

                line_out += word_out;
            }
            fout << line_out << endl;
        }
    }
}

// 主函数
int main(int argc, char *argv[]) {
    if (argc < 5) {
        cout << "Usage: " << argv[0] << " train_file test_file db_file bmes_file " << endl;
        exit(-1);
    }
    // 构造DB文件，输入训练语料、测试语料、输出文件名
    makeDB(argv[1], argv[2], argv[3]);

    // 构造BMES文件，输入训练语料、输出文件名
    makeBMES(argv[1], argv[4]);
} 