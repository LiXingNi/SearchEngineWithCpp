#ifndef SEARCH_SYSTEM_WORDDCT_H_
#define SEARCH_SYSTEM_WORDDCT_H_

#include "Base.h"

#include <unordered_map>
#include <fstream>

using std::fstream;
using std::unordered_map;


class WordDct
{
public:
	WordDct() = delete;
	WordDct(const string & filename) :file_name_(filename)
	{
		cout << "loading word dictionary" << endl;
		loadDct();
		cout << "loaded word dictionary" << endl;
	}
	

	int operator[](const string & word)const
	{
		auto iter = word_id_dct_.find(word);
		if (iter != word_id_dct_.end())
			return iter->second;
		else
		{
			return -1;
		}
	}

	string operator[](int id)const
	{
		auto iter = id_word_dct_.find(id);
		if (iter != id_word_dct_.end())
			return iter->second;
		else
		{
			return "";
		}
	}

	size_t size()const
	{
		return word_id_dct_.size();
	}


private:
	void loadDct()
	{
		fstream fobj(file_name_, std::ios::in);
		string line,word;
		int id;
		if (!fobj.good())
			cerr << "open failed" << endl;
		while (std::getline(fobj, line))
		{
			string::size_type loc = line.find_first_of('\t');
			word = line.substr(0, loc++);
			id = std::stoi((line.substr(loc)));
			word_id_dct_[word] = id;
			id_word_dct_[id] = word;
		}

		cout << id_word_dct_.size() << endl;

	}

	string file_name_;
	unordered_map<string, int> word_id_dct_;
	unordered_map<int, string> id_word_dct_;

};

#endif