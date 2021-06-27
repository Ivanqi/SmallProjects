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

#endif