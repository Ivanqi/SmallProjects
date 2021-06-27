#ifndef VITERBI_H
#define VITERBI_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include "hmm.h"
#include "db.h"

using namespace std;

HMM hmm("./data/Pi.mat", "./data/A.mat", "./data/B.mat");   // 初始化HMM模型
DB db("./data/db.txt"); // 初始化字典

// Viterbi算法进行分词
string viterbi(string str_in)
{
    string str_out = "";
    if (str_in.size() == 0) {
        return str_out;
    }

    // 分配矩阵空间
    int row = str_in.size() / 2;    // 输入句子中的汉字个数

    double **delta = new double *[row];
    for (int i = 0; i < row; i++) {
        delta[i] = new double[N]();
    }

    int **path = new int *[row];
    for (int i = 0; i < row; i++) {
        path[i] = new int[N]();
    }

    // 中间变量
    string cchar = "";  // 存放汉字
    int max_path = -1;
    
    double val = 0.0;
    double max_val = 0.0;

    // 初始化矩阵，给delta和path矩阵的第一行赋初值
    cchar = str_in.substr(0, 2);
    int cchar_num = db.getObservIndex(cchar);
    for (int i = 0; i < N; i++) {
        delta[0][i] = hmm.Pi[i] + hmm.B[i][cchar_num];  // 对数
        path[0][i] = -1;
    }

    // 给delta和path的后续行赋值(对数)
    for (int t = 1; t < row; t++) {
        cchar = str_in.substr(2*t, 2);
		cchar_num = db.getObservIndex(cchar);
		for (int j = 0; j < N; j++) {
			max_val = 100000.0;
			//max_path = -1;
			max_path = 0;
			for (int i = 0; i < N; i++) {
				val = delta[t-1][i] + hmm.A[i][j];
				if(val < max_val){
					max_val = val;
					max_path = i;
				}
			}

            delta[t][j] = max_val + hmm.B[j][cchar_num];
			path[t][j] = max_path;
        }
    }

    //找delta矩阵最后一行的最大值
	max_val = 100000.0;
	//max_path = -1;
	max_path = 0;
	for (int i = 0; i < N; i++) {
		if(delta[row - 1][i] < max_val){
			max_val = delta[row-1][i];
			max_path = i;
		}
	}

    // 从max_path出发,回溯得到最可能的路径
    stack<int> path_st;
    path_st.push(max_path);
    
    for (int i = row - 1; i > 0; i--) {
        max_path = path[i][max_path];
        path_st.push(max_path);
    }

    // 释放二维数组
    for (int i = 0; i < row; i++) {
        delete []delta[i];
        delete []path[i];
    }

    delete [] delta;
    delete [] path;

    // 根据标记好的状态序列分词
    int pos = 0;
	int index = -1;

    while (!path_st.empty()) {
        index = path_st.top();
        path_st.pop();
        str_out.insert(str_out.size(), str_in, pos, 2);

        if (index == 2 || index == 3) {
            // 状态为E或S
            str_out.append("/");
        }

        pos += 2;
    }
}

#endif