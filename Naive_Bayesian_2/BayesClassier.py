from numpy import *
import time, nltk
from decimal import Decimal
import jieba.posseg as pseg

'''
描述: 朴素贝叶斯新闻分类
思路:
    训练数据采集
    数据读取和清洗
    数据分析和特征选择
    数据向量化
    朴素贝叶斯模型训练
    分类算法实现
优点:
    训练简单，分类器行之有效，可以处理多类别问题
缺点:
    强制的独立假设问题，新词比较敏感
关键点:
    训练数据的选择、特征的选择、模型训练方式
改进点:
    准备数据，数据样本类间均衡问题
    分析数据，结合数据分布（结合NLTK等）和特点，对文本词性进行特征选择。
    模型训练，文本分类用主题模型（LDA）来向量化文本，还可以将文本向量用PCA进行一次特征降维，然后再训练模型
'''


# 分词去停用词, 创建数据集
import jieba, itertools

'''创建数据集和类标签'''
def loadDataSet():
    docList = []; classList = []; fullText = [] # 文档列表、类别列表、文本特征
    dirList = ['C3-Art','C4-Literature','C5-Education','C6-Philosophy','C7-History']

    for j in range(5):
        for i in range(1, 11):  # 总共10个文档
            # 切分，解析数据，并归类为1类别
            wordList = textParse(open('./fudan/%s/%d.txt' % (dirlist[j], i), "rb", encoding='UTF-8').read().decode('GBK', 'ignore'))
            docList.append(wordList)
            fullText.extend(wordList)