实现了简单的搜索，其中分为两个模式：
	一个根据命中次数对 doc 进行排序，称为 Base_algorithm
	一个根据 idf 对 doc 进行排序，称为 idf_algorithm

采用了两种分词模式：
	一个 cutForSearch ,该分词存在冗余：“今天天气真好”,结果：“今天天气”，“今天”，“天天”,... 称为：Search_mode
	一个 cut ，该分词为精确分词，不存在冗余：“今天天气真好”，结果：“今天天气”，“真好” 称为：Accurate_mode

搜索类：
	基类：
		SearchSystem
	派生类：
		SearchSystemBase, 对应 Base_algorithm,定义了两个构造函数 SearchSystemBase(ACCURATE_SEG_MODE),SearchSystemBase(Accurate_mode)
		
		SearchSystemIdf, 对应 Idf_algorithm,同样定义了针对两种分词模式的构造函数
	
	该继承体系中的对象通过如下语句调用搜索：
		obj.oneSearch(sentence);
	
统计性能类：
	基类：
		CountScore , 采用{3,5,10}条的 NDCG 来衡量搜索算法的性能
	派生类：
		CountScoreBase, 对应统计 Base_algorithm 算法的性能，同样提供针对两种分词模式的构造函数
		CountScoreIdf, 对应统计 Idf_algorithm 算法的性能，同样提供针对两种分词模式的构造函数

	该继承体系中的对象通过如下语句调用测试搜索性能：
		obj.count_score();
		 