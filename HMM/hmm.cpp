#include <cstdlib>
#include <vector>
#include <iomanip>
#include <map>
#include <algorithm>
#include <sys/stat.h>
#include <sys/time.h>
#include "util.h"
#include "viterbi.h"


const long MaxCount = 5000; // 需要切分的最大句子数量，若该值大于文件中. 实际的句子数量，以实际句子数量为准

// 获取当前时间(ms)
long getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// 获取文件大小
unsigned long getFileSize(string file_path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    
    if (stat(file_path.c_str(), &statbuff) < 0) {
        return filesize;
    } else {
        filesize = statbuff.st_size;
    }

    return filesize;
}

/**
 * 函数功能: 计算切分标记的位置
 * 函数输入: 
 *      1. strline_in未进行切分的汉字字符串
 *      2. strline_right 进行切分后的汉字字符串
 * 函数输出
 *      vector， 其中存放了strline_in中哪些位置放置了分词标记
 *      注意: vector中不包含最后标记的位置，但是包含位置0
 */
vector<int> getPos(string strline_right, string strline_in)
{
	int pos_1 = 0;
	int pos_2 = -1;
	int pos_3 = 0;
	string word = "";
	vector<int> vec;
 
	int length = strline_right.length();
	while (pos_2 < length) {
		//前面的分词标记
		pos_1 = pos_2;
		
		//后面的分词标记
		pos_2 = strline_right.find('/', pos_1 + 1);
 
		if (pos_2 > pos_1) {
			//将两个分词标记之间的单词取出
			word  = strline_right.substr(pos_1 + 1, pos_2 - pos_1 - 1);
			//根据单词去输入序列中查出出现的位置
			pos_3 = strline_in.find(word, pos_3);
			//将位置存入数组
			vec.push_back(pos_3);
			pos_3 = pos_3 + word.size();
		} else {
			break;
		}
	}
	
	return vec;
}

/**
 * 获取标准切分和程序切分的结果
 */

string getString(string word, int pos, vector<int> vec_right)
{
	char ss[1000];
	int i = 0;
	int k = 0;

	if (vec_right.size() == 0) {
		return word;
	}

	while (vec_right[i] < pos) {
		i++;
	}

	for (int j = 0; j < word.size(); j++) {
		if (j == vec_right[i] - pos) {
			if (j != 0) {
				ss[k] = '/';
				++k;
			}
			++i;
		}
		ss[k] = word[j];
		++k;
	}
	ss[k] = '\0';
	string word_str = ss;
 
	return word_str;
}

/*
 * 函数功能：获取单个句子切分的结果统计
 * 函数输入：1.vec_right 正确的分词标记位置集合
 *           2.vec_out   函数切分得到的分词标记位置集合
 * 函数输出：返回一个veceor，含有4个元素，分别为：
 *          切分正确、组合型歧义、未登录词、交集型歧义的数量
 *
 */

vector<int> getCount_2(string strline, vector<int> vec_right, vector<int> vec_out, vector<string> &vec_err)
{
	vector<int> vec(4, 0);	//存放计算结果
	//建立map
	map<int, int> map_result;
	for (int i = 0; i < vec_right.size(); i++) {
		map_result[vec_right[i]] += 1;
	}

	for (int i = 0; i < vec_out.size(); i++) {
		map_result[vec_out[i]] += 2;
	}
 
	//统计map中的信息
	//若value=1，只在vec_right中
	//若value=2，只在vec_out中
	//若value=3，在vec_right和vec_out中都有
	map<int, int>::iterator p_pre, p_cur;
	int count_value_1 = 0;
	int count_value_2 = 0;
	int count_value_3 = 0;
	p_pre = map_result.begin();
	p_cur = map_result.begin();
    
	while (p_cur != map_result.end()) {
        
		while (p_cur != map_result.end() && p_cur -> second == 3) {
			p_pre = p_cur;
			++count_value_3;	//切分正确的数目
			++p_cur;		//迭代器后移
		}
		
		while (p_cur != map_result.end() && p_cur -> second != 3) {
			if(p_cur -> second == 1){
				++count_value_1;
			}else if(p_cur -> second == 2){
				++count_value_2;
			}
			++p_cur;
		}
		
		//确定切分错误的字符串
		if (p_cur == map_result.end() && p_cur == (++p_pre)) {
			continue;
		}

		int pos_1 = p_pre -> first;
		int pos_2 = p_cur -> first; 
		string word = strline.substr(pos_1, pos_2 - pos_1);	//切分错误的单词
		string word_right = getString(word, pos_1, vec_right);	//正确的切分方式
		string word_out = getString(word, pos_1, vec_out);	//得到的切分方式
 
		string str_err = "";
		//不同的错误类型		
		if (count_value_1 > 0 && count_value_2 == 0) {
			str_err = "  组合型歧义： " + word + "    正确切分： " + word_right + "    错误切分： " + word_out;
			vec_err.push_back(str_err);
			cout << str_err << endl;
			vec[1] += count_value_1;		
		} else if(count_value_1 == 0 && count_value_2 > 0) {
			str_err = "  未登录词语： " + word + "    正确切分： " + word_right + "    错误切分： " + word_out;
			vec_err.push_back(str_err);
			cout << str_err << endl;
			vec[2] += count_value_2;
		} else if(count_value_1 > 0 && count_value_2 > 0) {
			str_err = "  交集型歧义： " + word + "    正确切分： " + word_right + "    错误切分： " + word_out;
			vec_err.push_back(str_err);
			cout << str_err << endl;
			vec[3] += count_value_2;	
		}
 
		//计数器复位
		count_value_1 = 0;
		count_value_2 = 0;
	}
 
	vec[0] += count_value_3;	
 
	return vec;
}

/**
 * 主函数：进行分词并统计分词结果
 */

int main(int argc, char *argv[]) {

	if (argc < 3) {
		cout << "Usage: " << argv[0] << " test_file result_file" << endl;
		exit(-1);
	}
 
	long time_1 = getCurrentTime();
	
	string strline_right;	//输入语料：用作标准分词结果
	string strline_in;	//去掉分词标记的语料（用作分词的输入）
	string strline_out_1;	//隐马尔科夫模型分词完毕的语料
	
	ifstream fin(argv[1]);	//打开输入文件
	if (!fin) {
		cout << "Unable to open input file !" << argv[1] << endl;
		exit(-1);
	}
 
	ofstream fout(argv[2]);	//确定输出文件
	if (!fout) {
		cout << "Unable to open output file !" << endl;
		exit(-1);
	}
	
	long count = 0;			//句子编号
	long count_1 = 0;		//隐马尔科夫模型切分完全正确的句子总数
	long count_right_all = 0;	//准确的切分总数
	long count_out_1_all = 0;	//隐马尔科夫模型切分总数
	long count_out_1_right_all = 0;	//隐马尔科夫模型切分正确总数
	long count_out_1_fail_1_all = 0;//隐马尔科夫模型（组合型歧义）
	long count_out_1_fail_2_all = 0;//隐马尔科夫模型（未登录词语）
	long count_out_1_fail_3_all = 0;//隐马尔科夫模型（交集型歧义）
 
 
	vector<string> vec_err_1;	//隐马尔科夫模型切分错误的词
 
	while (getline(fin, strline_right, '\n') && count < MaxCount) {
		if (strline_right.length() > 1) {
			
			//去掉分词标记
			strline_in = strline_right;
			strline_in = replace_all(strline_in, "/", "");
 
			//隐马尔科夫模型分词
			strline_out_1 = strline_right;
			istringstream strstm(strline_in);
			string sentence;
			string result;
			string line_out;
			while(strstm >> sentence){
				result = viterbi(sentence);
				line_out += result;
			}
			strline_out_1 = line_out;
 
			//输出分词结果
			count++;
			cout << "----------------------------------------------" << endl;
			cout << "句子编号：" << count << endl;
			cout << endl;
			cout << "待分词的句子长度: " << strline_in.length() << "  句子：" << endl;
			cout << strline_in << endl;
			cout << endl;
			cout << "标准比对结果长度: " << strline_right.length() << "  句子：" << endl;
			cout << strline_right << endl;
			cout << endl;
			cout << "隐马尔科夫模型分词长度: " << strline_out_1.length() << "  句子：" << endl;
			cout << strline_out_1 << endl;
			cout << endl;
			
 
			//输出分词结果的数字序列表示
			vector<int> vec_right = getPos(strline_right, strline_in);
			vector<int> vec_out_1 = getPos(strline_out_1, strline_in);
			
			cout << "标准结果：" << endl;
			for (int i = 0; i < vec_right.size(); i++) {
				cout << setw(4) << vec_right[i];
			}

			cout << endl;
 
			cout << "隐马尔科夫模型分词结果：" << endl;
			for (int i = 0; i < vec_out_1.size(); i++) {
				cout << setw(4) << vec_out_1[i];
			}
			cout << endl;
			
 
			//输出匹配的错误列表
			cout << endl;
			if (vec_right == vec_out_1) {
				cout << "隐马尔科夫模型分词完全正确！" << endl;
				count_1++;
			} else {
				cout << "隐马尔科夫模型分词错误列表：" << endl;
			}

			vector<int> vec_count_1 = getCount_2(strline_in, vec_right, vec_out_1, vec_err_1);
 
						
			//准确的切分数量
			int count_right = vec_right.size();
			//切分得到的数量
			int count_out_1 = vec_out_1.size();			
			//切分正确的数量
			int count_out_1_right = vec_count_1[0];
			cout << "切分得到：" << count_out_1 << endl;
			cout << "切分正确：" << count_out_1_right << endl;			
 
			cout << "隐马尔科夫模型：" << endl;	
			cout << "  组合型歧义：" << vec_count_1[1] << endl;
			cout << "  未登录词语：" << vec_count_1[2] << endl;
			cout << "  交集型歧义：" << vec_count_1[3] << endl;
			
			
			count_right_all += count_right;
			count_out_1_all += count_out_1;
			count_out_1_right_all += count_out_1_right;
			count_out_1_fail_1_all += vec_count_1[1];
			count_out_1_fail_2_all += vec_count_1[2];
			count_out_1_fail_3_all += vec_count_1[3];
			
			
		}
	}
	
	long time_2 = getCurrentTime();
	unsigned long file_size = getFileSize("test.txt");
 
 
	//打印错误的切分内容	
	cout << endl;
	cout << "---------------------------------" << endl;
	cout << "错误样例（已排序）：" << endl;
 
 
	//对错误切分内容进行排序并掉重复的
	sort(vec_err_1.begin(), vec_err_1.end());
	
	vector<string>::iterator end_unique_1 = unique(vec_err_1.begin(), vec_err_1.end());
	
 
	int num_1 = end_unique_1 - vec_err_1.begin();
	
 
	cout << "----------------------------------" << endl;
	cout << "隐马尔科夫模型切分错误数量：" << num_1 << endl;
	for (int i = 0; i < num_1; i++) {
		cout << vec_err_1[i] << endl;
	}
	cout << endl;
 
	
 
	//计算准确率和召回率
	double kk_1 = (double)count_out_1_right_all / count_out_1_all;	//隐马尔科夫模型准确率
	double kk_2 = (double)count_out_1_right_all / count_right_all;	//隐马尔科夫模型召回率
	
 
	//集中输出结果
	cout << endl;
	cout << "---------------------------------" << endl;
	cout << "分词消耗时间：" << time_2 - time_1 << "ms" << endl;
	cout << "测试文件大小：" << file_size/1024 << " KB" << endl;
	cout << "分词速度为：  " << (double)file_size*1000/((time_2 - time_1)*1024) << " KB/s" << endl;
 
	
 
	cout << endl;
	cout << "句子总数：" << count << endl;
	
	cout << "隐马尔科夫模型切分完全正确的句子数目： " << count_1 << "\t （ " << (double)count_1*100/count << " % ）" << endl;
	
	cout << endl;
 
	cout << "准确的切分总数：" << count_right_all << endl;		//准确的切分总数
	cout << "隐马尔科夫模型切分总数：" << count_out_1_all << endl;		//隐马尔科夫模型切分总数	
	cout << "隐马尔科夫模型切分正确总数：" << count_out_1_right_all << endl;	//隐马尔科夫模型切分正确总数
	
 
	cout << endl;
	cout << "隐马尔科夫模型：" << endl;
	long count_out_1_fail_all = count_out_1_fail_1_all + count_out_1_fail_2_all + count_out_1_fail_3_all;	
	cout << "  组合型歧义：" << count_out_1_fail_1_all << "\t ( " << (double)count_out_1_fail_1_all*100/count_out_1_fail_all << " % )" << endl;
	cout << "  未登录词语：" << count_out_1_fail_2_all << "\t ( " << (double)count_out_1_fail_2_all*100/count_out_1_fail_all << " % )" << endl;
	cout << "  交集型歧义：" << count_out_1_fail_3_all << "\t ( " << (double)count_out_1_fail_3_all*100/count_out_1_fail_all << " % )" << endl;
	
	cout << endl;		
	cout << "统计结果：" << endl;
	cout << "隐马尔科夫模型    准确率：" << kk_1*100 << "%  \t召回率：" << kk_2*100 << "%" << endl;
	
 
	return 0;
}