import jieba
import jieba.analyse


def seg(s):
    s = unicode(s.decode('gbk'))
    res = jieba.cut(s)
    lis = list(res)
    res = []
    for obj in lis:
        res.append(obj.encode('gbk'))
    return res


def idfSeg(s):
    s = unicode(s.decode('gbk'))
    res = jieba.analyse.extract_tags(s,withWeight = True)
    lis = list(res)
    res = []
    for obj in lis:
        res.append(obj[0].encode('gbk'))
        res.append(obj[1])
    return res
