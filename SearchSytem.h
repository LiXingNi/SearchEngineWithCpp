#ifndef SEARCH_SYSTEM_SEARCY_SYSTEM_H_
#define SEARCH_SYSTEM_SEARCH_SYSTEM_H_

#include "Base.h"
#include "BackTable.h"
#include "WordDct.h"
#include "Jieba.h"
#include "DataOrg.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <bitset>

class CountScoreBase;
class CountScoreIdf;

template<class T1, class T2>
struct HeapPred
{
	bool operator()(const pair<T1, T2> & a, const pair<T1, T2>& b)
	{
		return a.second > b.second;
	}
};

class SearchSystem
{
	friend class CountScoreBase;
	friend class CountScoreIdf;
protected:
	DataOrg data_org_obj_;
	BackTable back_table_obj_;
	WordDct word_dct_obj_;

	void print_result(vector<pair<int, double>>& result)
	{
		for (vector<pair<int, double>>::iterator it = result.begin(); it != result.end(); ++it)
		{
			cout << data_org_obj_.getTitle(it->first) << "	：	" << it->second << endl;
		}
	}


public:
	SearchSystem(SEARCH_SEG_MODE) :
		data_org_obj_(DATA_DIR),back_table_obj_(BACK_TABLE_DIR_SEARCH_MODE),word_dct_obj_(WORD_DCT_DIR_SEARCH_MODE){}

	SearchSystem(ACCURATE_SEG_MODE) :
		data_org_obj_(DATA_DIR), back_table_obj_(BACK_TABLE_DIR_ACCURATE_MODE), word_dct_obj_(WORD_DCT_DIR_ACCUTATE_MODE){}

	virtual void oneSearch(const string &, vector<pair<int, double>>& = vector<pair<int,double>>(),bool print = true) = 0;
};


class SearchSystemBase : public SearchSystem
{
	shared_ptr<JieBa> jieba_;

	void searchAlgorithm(vector<pair<string,double>> & word_sequence, vector<pair<int, double>> & heap_base)
	{
		typedef int DocId;
		typedef int Score;
		typedef vector<TableObj>::iterator ZipIter;
		typedef list<pair<ZipIter, ZipIter>>::iterator ContainerIter;
		HeapPred<int, int> heap_pred;
		const int stop_word_threshold = 100000;
		const int result_num_threshold = 10;
		list< pair<ZipIter, ZipIter> > zip_container;

		int score_threshold = word_sequence.size();



		//add useful sequence pointer(header pointer)
		for (auto word : word_sequence)
		{
			vector<TableObj>& hit_docs = back_table_obj_[word_dct_obj_[word.first]];
			zip_container.emplace_back(hit_docs.begin(), hit_docs.end());
		}

		//清空没有找到的关键字。没有找到的关键字中存的迭代器对为空
		for (auto iter = zip_container.begin(); iter != zip_container.end();)
		{
			if (iter->first == iter->second)
				iter = zip_container.erase(iter);
			else
				++iter;
		}


		//若堆的大小小于最小堆，或堆中分数最低的 doc 的分数已经达到上限
		while (zip_container.size() > 0)
		{
			ContainerIter min_iter = std::min_element(zip_container.begin(), zip_container.end(), [](const pair<ZipIter, ZipIter> & a, const pair<ZipIter, ZipIter> &  b){return a.first->doc_id_ <= b.first->doc_id_; });
			int min_score = 0;
			int min_val = min_iter->first->doc_id_;

			//递增最小doc_id 所在的倒排拉链的指针，若某个倒排拉链结束，则将其删除
			for (ContainerIter iter = zip_container.begin(); iter != zip_container.end();)
			{
				if (iter->first->doc_id_ == min_val)
				{
					++min_score;
					if ((++(iter->first)) == iter->second)
						iter = zip_container.erase(iter);
					else
						++iter;
				}
				else
					++iter;
			}

			//若堆中的数量小于目标数量，则直接入堆
			if (heap_base.size() < result_num_threshold)
			{

				heap_base.emplace_back(min_val, min_score);
				std::push_heap(heap_base.begin(), heap_base.end(), heap_pred);
			}
			//若堆中的数量已满，则需要判断后入堆，此处必须使用最小堆，STL默认是最大堆，需要使用可调用对象来进行调整
			else
			{
				if (heap_base.begin()->second < min_score)
				{
					std::pop_heap(heap_base.begin(), heap_base.end(), heap_pred);
					heap_base.pop_back();
					heap_base.emplace_back(min_val, min_score);
					std::push_heap(heap_base.begin(), heap_base.end(), heap_pred);
				}
			}

		}

		std::sort_heap(heap_base.begin(), heap_base.end(), heap_pred);

	}

public:
	SearchSystemBase(SEARCH_SEG_MODE) :SearchSystem(SEARCH_SEG_MODE()), jieba_(new JieBaBase()){}
	SearchSystemBase(ACCURATE_SEG_MODE) :SearchSystem(ACCURATE_SEG_MODE()), jieba_(new JieBaBase()){}

	void oneSearch(const string & line, vector<pair<int, double>>& sequence = vector<pair<int, double>>(), bool print = true) override
	{

		string lines(line);
		std::for_each(lines.begin(), lines.end(), [](char & c){if (std::isalpha(c) && !std::islower(c)) c = std::tolower(c); });
		vector<pair<string, double>> word_sequence;

		jieba_->cutForSearch(lines, word_sequence);
		searchAlgorithm(word_sequence, sequence);

		if (print)
			print_result(sequence);
	}
};


class SearchSystemIdf : public SearchSystem
{
	struct ZipIdfObj
	{
		typedef vector<TableObj>::iterator Iter;
		Iter beg_iter_;
		Iter end_iter_;
		double idf_score_;

		ZipIdfObj(Iter first, Iter last, double idf) :beg_iter_(first), end_iter_(last), idf_score_(idf){}
	};

private:
	shared_ptr<JieBa> jieba_;

	void searchAlgorithm(vector<pair<string, double>> & word_sequence, vector<pair<int, double>> & heap_base)
	{
		typedef int DocId;
		typedef double Score;
		typedef vector<TableObj>::iterator ZipIter;
		typedef list<ZipIdfObj>::iterator ContainerIter;
		HeapPred<int, double> heap_pred;
		const int stop_word_threshold = 100000;
		const int result_num_threshold = 10;
		list< ZipIdfObj > zip_container;


		int score_threshold = word_sequence.size();



		//add useful sequence pointer(header pointer)
		for (auto word : word_sequence)
		{
			vector<TableObj>& hit_docs = back_table_obj_[word_dct_obj_[word.first]];
			zip_container.emplace_back(hit_docs.begin(), hit_docs.end(), word.second);
		}

		//清空没有找到的关键字。没有找到的关键字中存的迭代器对为空
		for (auto iter = zip_container.begin(); iter != zip_container.end();)
		{
			if (iter->beg_iter_ == iter->end_iter_)
				iter = zip_container.erase(iter);
			else
				++iter;
		}


		//若堆的大小小于最小堆，或堆中分数最低的 doc 的分数已经达到上限
		while (zip_container.size() > 0)
		{
			ContainerIter min_iter = std::min_element(zip_container.begin(), zip_container.end(), [](const ZipIdfObj & a, const ZipIdfObj &  b){return a.beg_iter_->doc_id_ < b.beg_iter_->doc_id_; });
			double weight = 0;
			int min_val = min_iter->beg_iter_->doc_id_;

			//递增最小doc_id 所在的倒排拉链的指针，若某个倒排拉链结束，则将其删除
			for (ContainerIter iter = zip_container.begin(); iter != zip_container.end();)
			{
				if (iter->beg_iter_->doc_id_ == min_val)
				{
					weight += iter->idf_score_;
					if ((++(iter->beg_iter_)) == iter->end_iter_)
						iter = zip_container.erase(iter);
					else
						++iter;
				}
				else
					++iter;
			}

			//若堆中的数量小于目标数量，则直接入堆
			if (heap_base.size() < result_num_threshold)
			{

				heap_base.emplace_back(min_val, weight);
				std::push_heap(heap_base.begin(), heap_base.end(), heap_pred);
			}
			//若堆中的数量已满，则需要判断后入堆，此处必须使用最小堆，STL默认是最大堆，需要使用可调用对象来进行调整
			else
			{
				if (heap_base.begin()->second < weight)
				{
					std::pop_heap(heap_base.begin(), heap_base.end(), heap_pred);
					heap_base.pop_back();
					heap_base.emplace_back(min_val, weight);
					std::push_heap(heap_base.begin(), heap_base.end(), heap_pred);
				}
			}

		}

		std::sort_heap(heap_base.begin(), heap_base.end(), heap_pred);
	}
	
public:
	SearchSystemIdf(SEARCH_SEG_MODE) :SearchSystem(SEARCH_SEG_MODE()), jieba_(new JieBaIDF()){}
	SearchSystemIdf(ACCURATE_SEG_MODE) : SearchSystem(ACCURATE_SEG_MODE()), jieba_(new JieBaIDF()){}

	void oneSearch(const string & line, vector<pair<int, double>>& sequence = vector<pair<int, double>>(), bool print = true) override
	{
		string lines(line);
		std::for_each(lines.begin(), lines.end(), [](char & c){if (std::isalpha(c) && !std::islower(c)) c = std::tolower(c); });

		vector<pair<string, double>> word_idf_sequence;
		jieba_->cutForSearch(lines, word_idf_sequence);

		searchAlgorithm(word_idf_sequence, sequence);

		if (print)
			print_result(sequence);

	}
};


/*
class SearchSystem
{
	friend CountScore;
	BackTable back_table_obj_;
	WordDct word_dct_obj_;
	JieBa jieba;
	DataOrg data_org_obj_;

	//void intersectSet(vector<TableObj> & vec1, vector<TableObj>& vec2)
	//{
	//	auto it = std::set_intersection(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), vec1.begin(), [](const TableObj & a, const TableObj & b){return a.doc_id_ < b.doc_id_; });
	//	vec1.erase(it, vec1.end());
	//}
	//
	//void allHits(const vector<string> & word_sequence,vector<TableObj> &total_result)
	//{
	//	vector<TableObj> result;
	//	if (word_sequence.size() == 0)
	//		return;
	//	result = back_table_obj_[word_dct_obj_[word_sequence[0]]]; //init first word
	//
	//	for (vector <string>::const_iterator it = word_sequence.begin() + 1; it != word_sequence.end(); ++it)
	//	{
	//		intersectSet(result, back_table_obj_[word_dct_obj_[*it]]);
	//	}
	//	
	//	for (auto obj : result)
	//	{
	//
	//			total_result.push_back(obj);
	//
	//
	//	}
	//
	//}
	//
	//void secondHits(const vector<string >& word_sequence,int key_word_num,vector<TableObj> & total_result)
	//{
	//	vector<TableObj> temp;
	//	allHits(vector<string>(word_sequence.begin(), word_sequence.begin() + key_word_num), temp);
	//	for (auto obj : temp)
	//		total_result.push_back(obj);
	//}

	template<class T1,class T2>
	struct HeapPred
	{
		bool operator()(const pair<T1, T2> & a, const pair<T1, T2>& b)
		{
			return a.second > b.second;
		}
	};

	struct ZipIdfObj
	{
		typedef vector<TableObj>::iterator Iter;
		Iter beg_iter_;
		Iter end_iter_;
		double idf_score_;

		ZipIdfObj(Iter first, Iter last, double idf) :beg_iter_(first), end_iter_(last), idf_score_(idf){}
	};

	void searchAlgorithm(vector<string> & word_sequence,vector<pair<int,int>> & heap_base = vector<pair<int,int>>())
	{
		typedef int DocId;
		typedef int Score;
		typedef vector<TableObj>::iterator ZipIter;
		typedef list<pair<ZipIter, ZipIter>>::iterator ContainerIter;
		HeapPred<int,int> heap_pred;
		const int stop_word_threshold = 100000;
		const int result_num_threshold = 10;
		list< pair<ZipIter, ZipIter> > zip_container;
		//vector< pair<DocId, Score>> heap_base;

		int score_threshold = word_sequence.size();



		//add useful sequence pointer(header pointer)
		for (auto word : word_sequence)
		{
			//cout << word << "	:	";
			vector<TableObj>& hit_docs = back_table_obj_[word_dct_obj_[word]];
			//if (hit_docs.size() > stop_word_threshold)
			//{
			//	cout << "ignore word : " << word << endl;
			//	continue;
			//}
			//cout << hit_docs.size() << endl;
			zip_container.emplace_back(hit_docs.begin(), hit_docs.end());
		}

		//清空没有找到的关键字。没有找到的关键字中存的迭代器对为空
		for (auto iter = zip_container.begin(); iter != zip_container.end();)
		{
			if (iter->first == iter->second)
				iter = zip_container.erase(iter);
			else
				++iter;
		}


		//若堆的大小小于最小堆，或堆中分数最低的 doc 的分数已经达到上限
		while (zip_container.size() > 0)
		{
			ContainerIter min_iter = std::min_element(zip_container.begin(), zip_container.end(), [](const pair<ZipIter, ZipIter> & a, const pair<ZipIter, ZipIter> &  b){return a.first->doc_id_ <= b.first->doc_id_; });
			int min_score = 0;
			int min_val = min_iter->first->doc_id_;

			//递增最小doc_id 所在的倒排拉链的指针，若某个倒排拉链结束，则将其删除
			for (ContainerIter iter = zip_container.begin(); iter != zip_container.end();)
			{
				if (iter->first->doc_id_ == min_val)
				{
					++min_score;
					if ((++(iter->first)) == iter->second)
						iter = zip_container.erase(iter);
					else
						++iter;
				}
				else
					++iter;
			}

			//若堆中的数量小于目标数量，则直接入堆
			if (heap_base.size() < result_num_threshold)
			{

				heap_base.emplace_back(min_val, min_score);
				std::push_heap(heap_base.begin(), heap_base.end(), heap_pred);
			}
			//若堆中的数量已满，则需要判断后入堆，此处必须使用最小堆，STL默认是最大堆，需要使用可调用对象来进行调整
			else
			{
				if (heap_base.begin()->second < min_score)
				{
					std::pop_heap(heap_base.begin(), heap_base.end(), heap_pred);
					heap_base.pop_back();
					heap_base.emplace_back(min_val, min_score);
					std::push_heap(heap_base.begin(), heap_base.end(), heap_pred);
				}
			}

		}

		std::sort_heap(heap_base.begin(), heap_base.end(), heap_pred);

		//for (vector<pair<int, int>>::iterator it = heap_base.begin(); it != heap_base.end(); ++it)
		//{
		//	cout << data_org_obj_.getTitle(it->first) << "	：	" << it->second << endl;
		//}

	}

	void searchAlgorithm(vector<pair<string,double>> & word_sequence, vector<pair<int, double>> & heap_base = vector<pair<int, double>>())
	{
		typedef int DocId;
		typedef double Score;
		typedef vector<TableObj>::iterator ZipIter;
		typedef list<ZipIdfObj>::iterator ContainerIter;
		HeapPred<int,double> heap_pred;
		const int stop_word_threshold = 100000;
		const int result_num_threshold = 10;
		list< ZipIdfObj > zip_container;
		//vector< pair<DocId, Score>> heap_base;

		int score_threshold = word_sequence.size();



		//add useful sequence pointer(header pointer)
		for (auto word : word_sequence)
		{
			vector<TableObj>& hit_docs = back_table_obj_[word_dct_obj_[word.first]];
			zip_container.emplace_back(hit_docs.begin(),hit_docs.end(),word.second);
		}

		//清空没有找到的关键字。没有找到的关键字中存的迭代器对为空
		for (auto iter = zip_container.begin(); iter != zip_container.end();)
		{
			if (iter->beg_iter_ == iter->end_iter_)
				iter = zip_container.erase(iter);
			else
				++iter;
		}


		//若堆的大小小于最小堆，或堆中分数最低的 doc 的分数已经达到上限
		while (zip_container.size() > 0)
		{
			ContainerIter min_iter = std::min_element(zip_container.begin(), zip_container.end(), [](const ZipIdfObj & a, const ZipIdfObj &  b){return a.beg_iter_->doc_id_ < b.beg_iter_->doc_id_; });
			double weight = 0;
			int min_val = min_iter->beg_iter_->doc_id_;

			//递增最小doc_id 所在的倒排拉链的指针，若某个倒排拉链结束，则将其删除
			for (ContainerIter iter = zip_container.begin(); iter != zip_container.end();)
			{
				if (iter->beg_iter_->doc_id_ == min_val)
				{
					weight += iter->idf_score_;
					if ((++(iter->beg_iter_)) == iter->end_iter_)
						iter = zip_container.erase(iter);
					else
						++iter;
				}
				else
					++iter;
			}

			//若堆中的数量小于目标数量，则直接入堆
			if (heap_base.size() < result_num_threshold)
			{

				heap_base.emplace_back(min_val, weight);
				std::push_heap(heap_base.begin(), heap_base.end(), heap_pred);
			}
			//若堆中的数量已满，则需要判断后入堆，此处必须使用最小堆，STL默认是最大堆，需要使用可调用对象来进行调整
			else
			{
				if (heap_base.begin()->second < weight)
				{
					std::pop_heap(heap_base.begin(), heap_base.end(), heap_pred);
					heap_base.pop_back();
					heap_base.emplace_back(min_val, weight);
					std::push_heap(heap_base.begin(), heap_base.end(), heap_pred);
				}
			}

		}

		std::sort_heap(heap_base.begin(), heap_base.end(), heap_pred);

		//for (auto it = heap_base.begin(); it != heap_base.end(); ++it)
		//{
		//	cout << data_org_obj_.getTitle(it->first) << "	：	" << it->second << endl;
		//}

	}


	void normalSearch(vector<string> & word_sequence, vector<pair<int, int>>& sequence)
	{
		unordered_map<int, int> m_count;
		
		for (auto word : word_sequence)
		{
			int word_id = word_dct_obj_[word];
			vector<TableObj> & vec = back_table_obj_[word_id];
			if (vec.size() > 10000)
				continue;
			if (word_id != -1 && vec.size() != 0)
			{
				for (auto obj : vec)
				{
					++m_count[obj.doc_id_];
				}
			}
			else
			{
				cout << "can not find word : " << word << endl;
			}
		}
		
		int size = m_count.size();
		size = size <= 30 ? size : 30;
		
		sequence.resize(size);
		std::partial_sort_copy(m_count.begin(), m_count.end(), sequence.begin(), sequence.end(), [](const pair<const int, int>& a1, const pair<const int, int> & a2)
		{
			return a1.second > a2.second;
		});
		
		
		for (auto obj : sequence)
		{
			cout << obj.first << " : " << obj.second << " : " << data_org_obj_.getTitle(obj.first) << endl;
		}
	}


public:
	SearchSystem(const string & func_name = FUNC_NAME_BASE, const string & py_file = PYTHON_DIR, const string & word_file = "data/cutData/wordDct_gbk_cut.lst", const string & table_file = "data/cutData/backTable_u_cut.txt", const string & data_org = "data/dataOrg_gbk.lst")
		:back_table_obj_(table_file), word_dct_obj_(word_file), data_org_obj_(data_org), jieba(py_file, func_name){}



	void oneSearch(const string & line, vector<pair<int, int>>& sequence = vector<pair<int,int>>())
	{

		string lines(line);
		std::for_each(lines.begin(), lines.end(), [](char & c){if (std::isalpha(c) && !std::islower(c)) c = std::tolower(c); });
		vector<string> word_sequence;

		jieba.cutForSearch(lines, word_sequence);
		searchAlgorithm(word_sequence, sequence);

		
		//const int result_num = 10;
		//doc_log = 0;

		//vector<TableObj> total_result;
		//allHits(word_sequence, total_result);

		//while (total_result.size() < result_num)
		//{
		//	double percent_num = (1 - static_cast<double>(total_result.size()) / static_cast<double>(result_num)) * word_sequence.size();
		//	int num = static_cast<int>(percent_num);
		//	if (num == 0)
		//		break;
		//	secondHits(word_sequence, num, total_result);
		//}
		////
		////for (auto obj : total_result)
		////{
		////	cout << data_org_obj_.getTitle[obj.doc_id_] << endl;
		////}
		//
	}


	void oneSearch(const string & line, vector<pair<int, double>>& sequence)
	{
		string lines(line);
		std::for_each(lines.begin(), lines.end(), [](char & c){if (std::isalpha(c) && !std::islower(c)) c = std::tolower(c); });

		vector<pair<string, double>> word_idf_sequence;
		jieba.cutForSearch_idf(lines, word_idf_sequence);

		//std::for_each(word_idf_sequence.begin(), word_idf_sequence.end(), [](const pair<string, double>& obj){cout << obj.first << " : " << obj.second << " "; });

		searchAlgorithm(word_idf_sequence, sequence);

		//cout << endl;
		//std::for_each(sequence.begin(), sequence.end(), [](const pair<int, double>& obj){cout << obj.first << " : " << obj.second << endl; });
	}
};

*/

#endif