#ifndef SEARCH_SYSTEM_DATA_ORG_H_
#define SEARCH_SYSTEM_DATA_ORG_H_

#include "Base.h"



class DataOrg
{
	string file_name_;
	unordered_map < int, string> title_info_;
	unordered_map <int, string> url_info_;
public:
	DataOrg(const string & str) :file_name_(str)
	{
		cout << "loading data org" << endl;
		loadDataOrg();
		cout << "loaded data org : " << title_info_.size() << endl;
	}

	string getTitle(int doc_id)
	{
		if (title_info_.find(doc_id) != title_info_.end())
			return title_info_[doc_id];
		else
			return string();
	}

	string getUrl(int doc_id)
	{
		if (url_info_.find(doc_id) != url_info_.end())
			return url_info_[doc_id];
		else
			return string();
	}

private:
	void loadDataOrg()
	{
		fstream f_obj(file_name_, std::ios::in);
		string line;
		while (std::getline(f_obj,line))
		{
			int doc_id;
			string title;
			string url;
			parseLine(line, doc_id, title,url);
			title_info_[doc_id] = title;
			url_info_[doc_id] = url;
		}
	}


	void parseLine(const string & line, int & id, string & title,string & url)
	{
		size_t loc1 = line.find_first_of('\t');
		id = std::stoi(line.substr(0, loc1));
		size_t loc2 = line.find_last_of('\t');
		title = line.substr(loc1 + 1, loc2 - loc1 - 1);
		url = line.substr(loc2 + 1);

	}
};

#endif