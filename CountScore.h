#ifndef SEARCH_SYSTEM_COUNT_SCORE_H_
#define SEARCH_SYSTEM_COUNT_SCORE_H_

#include "SearchSytem.h"
#include <set>
using std::set;

#define DEBUG

class CountScore
{
protected:
	fstream data_obj_;
	vector<int> times_;

	void splitLine(string & line, string & key_word, string & url, int & score)
	{
		string::size_type loc1 = line.find_first_of('\t');
		key_word = line.substr(0, loc1);
		string::size_type loc2 = line.find_last_of('\t');
		url = line.substr(loc1 + 1, loc2 - loc1 - 1);
		string last = line.substr(loc2 + 1);
		if (!last.empty())
			score = stoi(last);
		else
			score = 0;
	}

	double countBDGC(map<string, int> & score_record, int count_num)
	{
		vector<pair<string, int>> score_keep(count_num);
		std::partial_sort_copy(score_record.begin(), score_record.end(), score_keep.begin(), score_keep.end(), [](const pair<const string, int>& a1, const pair<const string &, int> &a2)
		{return a1.second >= a2.second; });

		double score = 0;
		int cnt = 1;

		for (vector<pair<string, int>>::iterator it = score_keep.begin(); cnt <= count_num && it != score_keep.end(); ++it, ++cnt)
		{
#ifdef DEBUG
			//cout << "*score: " << it->second << " , loc: " << cnt << endl;
#endif
			score += countDGC(it->second, cnt);
		}

		return score;
	}

	double countDGC(int score, int loc)
	{
		double demonimator = std::log2(loc);
		if (demonimator != 0)
			return score / demonimator;
		else
			return score;
	}

	
public:

	CountScore() :data_obj_(DOC_SCORE_DIR){ times_ = { 3, 5, 10 }; }

	virtual void count_score() = 0;
};

class CountScoreBase : public CountScore
{
	shared_ptr<SearchSystem> search_ptr_;
	fstream output_obj_;

	//对每个关键进行计算
	void countOneWordIdf(const string & key_word, map<string, int> & score_record, double * average_score)
	{
		vector<pair<int, double>> sequence;
		search_ptr_->oneSearch(key_word, sequence,false);

		vector<int> doc_sequence;
		std::for_each(sequence.begin(), sequence.end(), [&doc_sequence](const pair<int, double> & obj){doc_sequence.push_back(obj.first); });

		output_obj_ << "key word: " << key_word << endl;
		//cout << key_word << endl;
		for (auto num : times_)
		{
			double temp_score = counter(doc_sequence, score_record, num);
			if (temp_score != -1)
				*average_score += temp_score;
			++average_score;
		}
		output_obj_ << endl;

	}

	double counter(vector<int> & sequence, map<string, int> & score_record, int count_num)
	{
		size_t cnt(1);
		double res(0);
		for (vector<int>::iterator it = sequence.begin(); cnt <= count_num && it != sequence.end(); ++cnt, ++it)
		{
			//cout << "doc_id : " << it->first << endl;
			string url = search_ptr_->data_org_obj_.getUrl(*it);
			int doc_score;
			if (score_record.find(url) != score_record.end())
				doc_score = score_record[url];
			else
				doc_score = 0;
			res += countDGC(doc_score, cnt);
#ifdef DEBUG
			//cout << "score: " << doc_score << " , loc: " << cnt << endl;
#endif
		}

		double best_res = countBDGC(score_record, count_num);

		if (best_res == 0)
			return -1;

		res = res / best_res;

		output_obj_ << res << endl;
		//cout << " score: " << res << endl;
		return res;
	}

public:
	CountScoreBase(SEARCH_SEG_MODE) :
		search_ptr_(new SearchSystemBase(SEARCH_SEG_MODE())), output_obj_(OUTPUT_SCORE_DIR_SEARCH_MODE_BASE,std::ios::out){}

	CountScoreBase(ACCURATE_SEG_MODE) :
		search_ptr_(new SearchSystemBase(ACCURATE_SEG_MODE())), output_obj_(OUTPUT_SCORE_DIR_ACCURATE_MODE_BASE, std::ios::out){}

	void count_score() override
	{
		string line;
		string prev_key_word;
		string key_word;
		string url;
		int score = 0;

		int average_count = 0;
		double average_score[3] = { 0.0, 0.0, 0.0 };

		map<string, int> score_record;
		vector<pair<int, int>> sequence;

		while (getline(data_obj_, line))
		{
			splitLine(line, key_word, url, score);
			if (prev_key_word.empty() || key_word == prev_key_word)
			{
				if (prev_key_word.empty())
					prev_key_word = key_word;
				score_record[url] = score;
				continue;
			}

			countOneWordIdf(prev_key_word, score_record, average_score);
			++average_count;

			prev_key_word = key_word;
			sequence.clear();
			score_record.clear();
		}
		countOneWordIdf(key_word, score_record, average_score);  //处理最后一行数据
		++average_count;

		output_obj_ << "total_score" << endl;
		for (double score : average_score)
		{
			output_obj_ << score / double(average_count) << endl;
		}
	}
};

class CountScoreIdf : public CountScore
{
	shared_ptr<SearchSystem> search_ptr_;
	fstream output_obj_;


	//对每个关键进行计算
	void countOneWord(const string & key_word, map<string, int> & score_record, double * average_score)
	{
		vector<pair<int, double>> sequence;
		search_ptr_->oneSearch(key_word, sequence, false);

		vector<int> doc_sequence;
		std::for_each(sequence.begin(), sequence.end(), [&doc_sequence](const pair<int,double> & obj){doc_sequence.push_back(obj.first); });

		output_obj_ << "key word: " << key_word << endl;
		//cout << key_word << endl;
		for (auto num : times_)
		{
			double temp_score = counter(doc_sequence, score_record, num);
			if (temp_score != -1)
				*average_score += temp_score;
			++average_score;
		}
		output_obj_ << endl;

	}

	double counter(vector<int> & sequence, map<string, int> & score_record, int count_num)
	{
		size_t cnt(1);
		double res(0);
		for (vector<int>::iterator it = sequence.begin(); cnt <= count_num && it != sequence.end(); ++cnt, ++it)
		{
			//cout << "doc_id : " << it->first << endl;
			string url = search_ptr_->data_org_obj_.getUrl(*it);
			int doc_score;
			if (score_record.find(url) != score_record.end())
				doc_score = score_record[url];
			else
				doc_score = 0;
			res += countDGC(doc_score, cnt);
#ifdef DEBUG
			//cout << "score: " << doc_score << " , loc: " << cnt << endl;
#endif
		}

		double best_res = countBDGC(score_record, count_num);

		if (best_res == 0)
			return -1;

		res = res / best_res;

		output_obj_ << res << endl;
		//cout << " score: " << res << endl;
		return res;
	}

public:
	CountScoreIdf(SEARCH_SEG_MODE) :
		search_ptr_(new SearchSystemIdf(SEARCH_SEG_MODE())), output_obj_(OUTPUT_SCORE_DIR_SEARCH_MODE_IDF, std::ios::out){}

	CountScoreIdf(ACCURATE_SEG_MODE) :
		search_ptr_(new SearchSystemIdf(ACCURATE_SEG_MODE())), output_obj_(OUTPUT_SCORE_DIR_ACCURATE_MODE_IDF, std::ios::out){}

	void count_score() override
	{
		string line;
		string prev_key_word;
		string key_word;
		string url;
		int score = 0;

		int average_count = 0;
		double average_score[3] = { 0.0, 0.0, 0.0 };

		map<string, int> score_record;
		vector<pair<int, int>> sequence;

		while (getline(data_obj_, line))
		{
			splitLine(line, key_word, url, score);
			if (prev_key_word.empty() || key_word == prev_key_word)
			{
				if (prev_key_word.empty())
					prev_key_word = key_word;
				score_record[url] = score;
				continue;
			}

			countOneWord(prev_key_word, score_record, average_score);
			++average_count;

			prev_key_word = key_word;
			sequence.clear();
			score_record.clear();
		}
		countOneWord(key_word, score_record, average_score);  //处理最后一行数据
		++average_count;

		output_obj_ << "total_score" << endl;
		for (double score : average_score)
		{
			output_obj_ << score / double(average_count) << endl;
		}
	}
};

/*
class CountScore
{
	shared_ptr<SearchSystem> search_base_ptr_;
	shared_ptr<SearchSystem> search_idf_ptr_
	fstream f_obj_;
	fstream output_base_obj_;
	fstream output_idf_obj_;
	vector<int> times_;




public:
	CountScore(const string & func_name_,const string & filename = DOC_SCORE_DIR) :output_base_obj_(OUTPUT_SCORE_DIR_SEARCH_MODE, std::ios::out), f_obj_(filename), search_obj_(func_name_){ times_ = { 3, 5, 10 }; }

	CountScore(SEARCH_SEG_MODE) :
		output_base_obj_(OUTPUT_SCORE_DIR_SEARCH_MODE_BASE, std::ios::out),
		output_idf_obj_(OUTPUT_SCORE_DIR_SEARCH_MODE_IDF, std::ios::out),
		search_base_ptr_(new SearchSystemBase(SEARCH_SEG_MODE())),
		search_idf_ptr_(new SearchSystemIdf(SEARCH_SEG_MODE()))
	{
		
	}

	CountScore(ACCURATE_SEG_MODE):
		output_base_obj_(OUTPUT_SCORE_DIR_ACCURATE_MODE_BASE, std::ios::out),
		output_idf_obj_(OUTPUT_SCORE_DIR_ACCURATE_MODE_IDF, std::ios::out),
		search_base_ptr_(new SearchSystemBase(SEARCH_SEG_MODE())),
		search_idf_ptr_(new SearchSystemIdf(SEARCH_SEG_MODE()))

	void countScore()
	{
		string line;
		string prev_key_word;
		string key_word;
		string url;
		int score = 0;

		int average_count = 0;
		double average_score[3] = {0.0,0.0,0.0};

		map<string, int> score_record;
		vector<pair<int, int>> sequence;
 
		while (getline(f_obj_, line))
		{
			splitLine(line, key_word, url, score);
			if (prev_key_word.empty() || key_word == prev_key_word)
			{
				if (prev_key_word.empty())
					prev_key_word = key_word;
				score_record[url] = score;
				continue;
			}
			
			countOneWordIdf(prev_key_word, score_record,average_score);
			++average_count;

			prev_key_word = key_word;
			sequence.clear();
			score_record.clear();
		}
		countOneWordIdf(key_word, score_record,average_score);  //处理最后一行数据
		++average_count;

		output_obj_ << "total_score" << endl;
		for (double score : average_score)
		{
			output_obj_ << score / double(average_count) << endl;
		}

	}

	//对每个关键进行计算
	void countOneWordIdf(const string & key_word, map<string, int> & score_record, double * average_score)
	{
		vector<pair<int, double>> sequence;
		search_obj_.oneSearch(key_word, sequence);

		vector<int> doc_sequence;
		std::for_each(sequence.begin(), sequence.end(), [&doc_sequence](const pair<int, double> & obj){doc_sequence.push_back(obj.first); });

		output_obj_ << "key word: " << key_word << endl;
		//cout << key_word << endl;
		for (auto num : times_)
		{
			double temp_score = counter(doc_sequence, score_record, num);
			if (temp_score != -1)
				*average_score += temp_score;
			++average_score;
		}
		output_obj_ << endl;

	}

	//对每个关键进行计算
	void countOneWord(const string & key_word, map<string, int> & score_record,double * average_score)
	{
		vector<pair<int, int>> sequence;
		search_obj_.oneSearch(key_word, sequence);

		vector<int> doc_sequence;
		std::for_each(sequence.begin(), sequence.end(), [&doc_sequence](const pair<int, int> & obj){doc_sequence.push_back(obj.first); });

		output_obj_ << "key word: " << key_word << endl;
		//cout << key_word << endl;
		for (auto num : times_)
		{
			double temp_score = counter(doc_sequence, score_record, num);
			if (temp_score != -1)
				*average_score += temp_score;
			++average_score;
		}
		output_obj_ << endl;

	}

	double counter(vector<int> & sequence, map<string, int> & score_record, int count_num)
	{
		size_t cnt(1);
		double res(0);
		for (vector<int>::iterator it = sequence.begin(); cnt <= count_num && it != sequence.end(); ++cnt, ++it)
		{
			//cout << "doc_id : " << it->first << endl;
			string url = search_obj_.data_org_obj_.getUrl(*it);
			int doc_score;
			if (score_record.find(url) != score_record.end())
				doc_score = score_record[url];
			else
				doc_score = 0;
			res += countDGC(doc_score, cnt);
#ifdef DEBUG
			//cout << "score: " << doc_score << " , loc: " << cnt << endl;
#endif
		}

		double best_res = countBDGC(score_record,count_num);

		if (best_res == 0)
			return -1;
		
		res = res / best_res;

		output_obj_ << res << endl;
		//cout << " score: " << res << endl;
		return res;
	}




};
*/

#endif