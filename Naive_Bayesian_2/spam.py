from numpy import *
import feedparser

'''获取所有单词的集合:返回不含重复元素的单词列表'''
def createVocabList(dataSet):
    vocabSet = set([])
    for document in dataSet:
        vocabSet = vocabSet | set(document)     # 操作符 | 用于求两个集合的并集
    return list(vocabSet)

'''词集模型构建数据矩阵'''
def setOfWords2Vec(vocabList, inputSet):
    # 创建一个和词汇表等长的向量，并将其元素都设置为0
    returnVec = [0] * len(vocabList)
    # 遍历文档中的所有单词，如果出现了词汇表中的单词，则将输出的文档向量中的对应值设为1
    for word in inputSet:
        if word in vocabList:
            returnVec[vocabList.index(word)] = 1
        else:
            print("单词: %s 不在词汇表之中!" % word)
    return returnVec

'''
朴素贝叶斯分类函数

将乘法转换为加法
乘法：P(C|F1F2...Fn) = P(F1F2...Fn|C)P(C)/P(F1F2...Fn)
加法：P(F1|C)*P(F2|C)....P(Fn|C)P(C) -> log(P(F1|C))+log(P(F2|C))+....+log(P(Fn|C))+log(P(C))

vec2Classify: 
    待测数据[0,1,1,1,1...]，即要分类的向量
p0Vec: 
    类别0，即正常文档的[log(P(F1|C0)),log(P(F2|C0)),log(P(F3|C0)),log(P(
    F4|C0)),log(P(F5|C0))....]列表

p1Vec: 
    类别1，即侮辱性文档的[log(P(F1|C1)),log(P(F2|C1)),log(P(F3|C1)),log(P(
    F4|C1)),log(P(F5|C1))....]列表

pClass1: 
    类别1，侮辱性文件的出现概率

返回：类别1 or 0
'''
def classifyNB(vec2Classify, p0Vec, p1Vec, pClass1):
    # 计算公式 log(P(F1|C))+log(P(F2|C))+....+log(P(Fn|C))+log(P(C))
    # 使用 NumPy 数组来计算两个向量相乘的结果，这里的相乘是指对应元素相乘，即先将两个向量中的第一个元素相乘，然后将第2个元素相乘，以此类推。这里的 vec2Classify * p1Vec 的意思就是将每个词与其
    p1 = sum(vec2Classify * p1Vec) + log(pClass1)
    p0 = sum(vec2Classify * p0Vec) + log(1.0 - pClass1)
    if p1 > p0:
        return 1
    else:
        return 0

'''训练数据优化版本'''
def trainNB0(trainMatrix, trainCategory):

    numTrainDocs = len(trainMatrix) # 总文件数
    numWords = len(trainMatrix[0])  # 总单词数

    pAbusive = sum(trainCategory) / float(numTrainDocs) # 侮辱性文件的出现概率

    # 构造单词出现次数列表,p0Num 正常的统计,p1Num 侮辱的统计
    # 避免单词列表中的任何一个单词为0，而导致最后的乘积为0，所以将每个单词的出现次数初始化为 1
    p0Num = ones(numWords)  # [0,0......]->[1,1,1,1,1.....],ones初始化1的矩阵
    p1Num = ones(numWords)

    # 整个数据集单词出现总数，2.0根据样本实际调查结果调整分母的值（2主要是避免分母为0，当然值可以调整）
    # p0Denom 正常的统计
    # p1Denom 侮辱的统计
    p0Denom = 2.0
    p1Denom = 2.0

    for i in range(numTrainDocs):
        if trainCategory[i] == 1:
            p1Num += trainMatrix[i]         # 累加辱骂词的频率
            p1Denom += sum(trainMatrix[i])  # 对每篇文章的辱骂的频次，进行统计汇总
        else:
            p0Num += trainMatrix[i]
            p0Denom += sum(trainMatrix[i])
    
    '''
    类别1，即侮辱性文档的[log(P(F1|C1)),log(P(F2|C1)),log(P(F3|C1)),log(P(F4|C1)),log(P(F5|C1))....]列表
    取对数避免下溢出或浮点舍入出错
    '''
    p1Vect = log(p1Num / p1Denom)

    '''
    类别0，即正常文档的[log(P(F1|C0)),log(P(F2|C0)),log(P(F3|C0)),log(P(F4|C0)),log(P(F5|C0))....]列表
    '''
    p0Vect = log(p0Num / p0Denom)
    return p0Vect, p1Vect, pAbusive


'''---------------项目案例2: 使用朴素贝叶斯过滤垃圾邮件----------------'''
'''接收一个大字符串并将其解析为字符串列表'''
def textParse(bigString):
    import re
    # 使用正则表达式来切分句子，其中分隔符是除单词、数字处的任意字符串
    listOfTokens = re.split(r'\W+', bigString)
    return [tok.lower() for tok in listOfTokens if len(tok) > 2]

'''读取文本'''
def testParseTest():
    print(textParse(open('./email/ham/1.txt').read()), '\n')

'''对贝叶斯垃圾邮件分类器进行自动化处理'''
def spamTest():
    docList = [];classList = [];fullText = []   # 文档列表、类别列表、文本特征
    for i in range(1, 26):                      # 总共25个文档
        # 切分，解析数据，并归类为1类别
        wordList = textParse(open('./email/spam/%d.txt' % i, "rb").read().decode('GBK', 'ignore'))
        docList.append(wordList)
        classList.append(1)
        # # 切分，解析数据，并归类为0类别
        wordList = textParse(open('./email/ham/%d.txt' % i, "rb").read().decode('GBK', 'ignore'))
        docList.append(wordList)
        classList.append(0)
        fullText.extend(wordList)

    # 创建词汇表
    vocabList = createVocabList(docList)
    trainingSet = list(range(50))   # 词汇表文档索引
    testSet = []

    # # 随机取10个邮件用来测试
    for i in range(10):
        # random.uniform(x, y) 随机生成一个范围为 x - y 的实数
        randIndex = int(random.uniform(0, len(trainingSet)))
        testSet.append(trainingSet[randIndex])                  # 随机抽取测试样本
        del(trainingSet[randIndex])                             # 训练集中删除选择为测试集的文档

    trainMat = [];trainClasses = []                             # 训练集合训练标签
    for docIndex in trainingSet:
        trainMat.append(setOfWords2Vec(vocabList, docList[docIndex]))
        trainClasses.append(classList[docIndex])
    
    p0V, p1V, pSpam = trainNB0(array(trainMat), array(trainClasses))
    errorCount = 0

    for docIndex in testSet:
        wordVector = setOfWords2Vec(vocabList, docList[docIndex])
        if classifyNB(array(wordVector), p0V, p1V, pSpam) != classList[docIndex]:
            errorCount += 1
    
    print('the errorCount is: ', errorCount)
    print('the testSet length is :', len(testSet))
    print('the error rate is :', float(errorCount)/len(testSet))

if __name__ == "__main__":
    # 正则切词
    bigString = 'This book is the best book on python or M.L. I have ever laid eyes upon.'
    textParse(bigString)
    testParseTest()
    print('\n')
    
    spamTest()