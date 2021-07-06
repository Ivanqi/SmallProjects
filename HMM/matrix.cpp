#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cmath>
#include <list>
#include "db.h"

using namespace std;

const int N = 4;            // 隐藏状态的数目
const int M = 5236;         // 汉字的个数
const double VALUE = 1.0;   // 平滑算法增加的值

// 定义字典对象
DB db("./data/db.txt");

/**
 * 模型训练，将频数转换为频率(加1平滑)
 */
void turingAdd(const int count[], double prob[], int len)
{
    double sum = 0.0;
    for (int i = 0; i < len; ++i) {
        sum += count[i];
    }

    if (sum == 0.0) {
        for (int i = 0; i < len; ++i) {
            prob[i] = 0.0;
        }
    } else {
        sum = sum + VALUE * len;
        for (int i = 0; i < len; ++i) {
            prob[i] = -log((count[i] + VALUE) / sum);   // 取对数
        }
    }
}

/**
 * 模型训练，将发射频率转换为频率(古德-图灵平滑)
 */
void turingGood(const int count[], double prob[], int len)
{
    map<int, list<int>> freq_map;       // key为词频，value为该词频对应的汉字列表
    map<int, list<int>>::iterator iter; // 迭代器
    int sum = 0;                        // 词频总和

    // 初始化freq_map
    for (int i = 0; i < len; i++) {
        int freq = count[i];
        sum += freq;

        iter = freq_map.find(freq);
        if (iter != freq_map.end()) {
            // 该词频已经存在，把当前词加入相应的list
            freq_map[freq].push_back(i);
        } else {
            // 该词频不存在，建立对应的汉字list
            list<int> lst;
            lst.push_back(i);
            freq_map[freq] = lst;
        }
    }

    // 若sum = 0，则结果初始化为0.0即可
    if (sum == 0) {
        for (int i = 0; i < len; i++) {
            prob[i] = 0.0;
        }
        return ;
    }

    // 数据平滑处理
    iter = freq_map.begin();
    while (iter != freq_map.end()) {
        double pr;  // 频率
        int freq = iter->first;
        int freqsize = iter->second.size();

        if (++iter != freq_map.end()) {
            int freq_2 = iter->first;
            if (freq_2 = freq + 1) {
                int freqsize_2 = iter->second.size();
                pr = ((1.0 + freq) * freqsize_2) / (sum * freqsize);
            } else {
                pr = 1.0 * freq / sum;
            }
        } else {
            pr = 1.0 * freq / sum;
        }

        // 计算结果
        list<int> lst = (--iter)->second;
        list<int>::iterator iter_in = lst.begin();
        while (iter_in != lst.end()) {
            int index = *iter_in;
            prob[index] = pr;
            ++iter_in;
        }

        // 准备下次迭代
        ++iter;
    }

    // 概率归一化
    double total = 0.0;
    for (int i = 0; i < len; i++) {
        total += prob[i];
    }

    for (int i = 0; i < len; i++) {
        prob[i] = -log((double)prob[i] / total);    // 取对数
    }
}


/*
 * 主函数，生成HMM模型的参数
 * 状态转移概率矩阵、初始状态概率矩阵、符号发射概率矩阵
 */
int main(int argc, char *argv[]) {
	if ( argc < 2) {
		cout << "Usage: " << argv[0] << " bmes_file !" << endl;
		exit(-1);
	}
 
	ifstream fin(argv[1]);
	if (!fin) {
		cerr << "Open input file " << argv[1] << "filed !" << endl;
		exit(-1);
	}
 
	int Pi[N] = {0};		//初始状态出现的次数
	int A[N][N] = {0};		//状态转移的次数
	int B[N][M] = {0};		//符号发射次数
 
	//抽取文件中的状态和观察值
	string line = "";			//存放每一行的内容
	int line_num = 0;			//句子编号

	while (getline(fin, line)) {
		line_num++;
		char state;			        //状态
		string cchar = "";		    //一个汉字
		int i, j, k;
		string::size_type pos = 0;	//当前处理位置

		if ((pos = line.find("/", pos + 1)) != string::npos) {
			//抽取句子的第一个状态
			state = line.at(pos + 1);
			i = db.getStateIndex(state);
			Pi[i]++;
			//抽取句子的第一个观察值
			cchar = line.substr(pos - 2, 2);
			k = db.getObservIndex(cchar);
			B[i][k]++;

			while ((pos = line.find("/", pos + 1)) != string::npos) {
				//抽取句子的其他状态
				state = line.at(pos + 1);
				j = db.getStateIndex(state);
				//Pi[j]++;
				A[i][j]++;
				//抽取句子的其他观察值
				cchar = line.substr(pos - 2, 2);
				k = db.getObservIndex(cchar);
				B[j][k]++;
				
				//准备下次迭代
				i = j;
			}
		}
	}

	fin.close();
 
	//打开输出流
	ofstream fout_1("Pi.mat");	//初始概率矩阵
	ofstream fout_2("A.mat");	//状态转移矩阵
	ofstream fout_3("B.mat");	//发射概率矩阵

	if (!(fout_1 && fout_2 && fout_3)) {
		cerr << "Create Matrix file failed !" << endl;
		return 1;
	}
 
	fout_1 << setprecision(8);
	fout_2 << setprecision(8);
	fout_3 << setprecision(8);
 
	//初始状态矩阵写入文件
	double arr_pi[N] = {0.0};
	//turingGood(Pi, arr_pi, N);

	turingAdd(Pi, arr_pi, N);
	for (int i = 0; i < N; i++) {
		fout_1 << arr_pi[i] << "\t";
	}

	fout_1 << endl;
 
	//状态转移矩阵写入文件
	double arr_a[N] = {0.0};
	for (int i = 0; i < N; i++) {
		//turingGood(A[i], arr_a, N);
		turingAdd(A[i], arr_a, N);
		for (int j = 0; j < N; j++) {
			fout_2 << arr_a[j] << "\t";
		}
		fout_2 << endl;
	}
	
	//发射概率矩阵写入文件
	double arr_b[M] = {0.0};
	for (int i = 0; i < N; i++) {
		//turingGood(B[i], arr_b, M);
		turingAdd(B[i], arr_b, M);
		for (int j = 0; j < M; j++) {
			fout_3 << arr_b[j] << "\t";
		}
		fout_3 << endl;
	}
 
	fout_1.close();
	fout_2.close();
	fout_3.close();
 
	return 0;
}