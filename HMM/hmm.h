#ifndef HMM_H
#define HMM_H

#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
using namespace std;

const int N = 4;
const int M = 5236;

// 定义HMM模型
class HMM
{
    public:
        int n;          // 状态数目
        int m;          // 可能的观察符号数目
        double A[N][N]; // 状态转移概率矩阵
        double B[N][M]; // 符号发射概率矩阵
        double Pi[N];   // 初始状态概率
        HMM();
        HMM(string f1, string f2, string f3);
};

// 无参数构造函数
HMM::HMM()
{

}

// 有参构造函数
HMM::HMM(string f1, string f2, string f3)
{
    ifstream fin_1(f1.c_str());
	ifstream fin_2(f2.c_str());
	ifstream fin_3(f3.c_str());
	if(!(fin_1 && fin_2 && fin_3)){
		exit(-1);
	}

    string line = "";
    string word = "";

    //读取Pi
	getline(fin_1, line);
	istringstream strstm_1(line);
	for (int i = 0; i < N; i++) {
		strstm_1 >> word;
		Pi[i] = atof(word.c_str());
	}

    //读取A
	for(int i = 0; i < N; i++){
		getline(fin_2, line);
		istringstream strstm_2(line);
		for(int j = 0; j < N; j++){
			strstm_2 >> word;
			A[i][j] = atof(word.c_str());
		}
	}

    // 读取B
    for (int i = 0; i < N; i++) {
        getline(fin_3, line);
		istringstream strstm_3(line);
		for (int j = 0; j < M; j++) {
			strstm_3 >> word;
			B[i][j] = atof(word.c_str());
		}
    }

    fin_1.close();
	fin_2.close();
	fin_3.close();
}

#endif