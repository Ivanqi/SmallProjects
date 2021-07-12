#include <iostream>
#include <string>
#include <list>
using namespace std;

int main() {

    double pi[3] = {0.2, 0.4, 0.4};
    double C[3][3] = {0.5, 0.2, 0.3, 0.3, 0.5, 0.2, 0.3, 0.5};
    double E[3][2] = {0.5, 0.5, 0.4, 0.6, 0.7, 0.3};

    string output[4] = {"R", "W", "R", "W"};
    int state[3] = {1, 2, 3};
    int row = 3;
    int column = 3;

    // 开辟数组空间
    double **delta = new double*[row];
    int **path = new int*[row];

    int i, j, k;
    for (i = 0; i < 3; i++) {
        delta[i] = new double[3];
        path[i] = new int[3];
    }

    // 将输出状态转为数组
    int outnum[4];
    for (i = 0; i < row; i++) {
        if (output[i] == "R") {
            outnum[i] = 0;
        } else if (output[i] == "W") {
            outnum[i] = 1;
        }
    }

    // 初始化
    for (i = 0; i < column; i++) {
        path[i][0] = 0;
        delta[i][0] = pi[i] * E[i][0];
        cout << delta[i][0] << endl;
    }

    cout << "----------------" << endl;

    // 递归
    for (j = 1; j < row; j++) {             // 序列遍历，矩阵列遍历
        for (k = 0; k < column; k++) {      // 状态遍历
            double pro = 0;
            int sta = 0;

            for (i = 0; i < column; i++) {  // 矩阵行遍历
                double temp = delta[i][j - 1] * C[i][k] * E[k][outnum[j]];  // delta[i][j-1]*转移概率*发射概率
                cout << delta[i][j - 1] << " " << E[k][outnum[j]] << "\t";
                cout << "temp: " << temp << "\t";
                
                if (temp > pro) {
                    pro = temp;
                    sta = i;    // 记录路径信息
                }
            }
            cout << endl;

            delta[k][j] = pro;
            path[k][j] = sta;
        }
    }

    cout << "----------------" << endl;


    // 终止，找到概率最大值
    double max = 0;
    int sta = 0;
    for (i = 0; i < column; i++) {
        if (delta[i][row - 1] > max) {
            max = delta[i][row - 1];
            sta = i;
        }
    }

    // 回溯，找到最大路径及其对应的状态值
    list<int> listPath;
    listPath.push_back(sta + 1);

    for (j = row - 1; j > 0; j--) {
        sta = path[sta][j];
        listPath.push_back(sta + 1);
    }

    // 输出
    cout << "max probability: " << max << endl;
    list<int>::iterator itepath;

    for (itepath = listPath.begin(); itepath != listPath.end(); itepath++) {
        cout << *itepath << " ";
    }

    return 0;
}

