#ifndef SEARCHSYSTEM_BACKTABLE_H_
#define SEARCHSYSTEM_BACKTABLE_H_

#include "Base.h"
#include <fstream>
#include <utility>
#include <map>
#include <unordered_map>
#include <memory>
#include <streambuf>
#include <sstream>
#include <list>

using std::shared_ptr;
using std::ifstream;
using std::istreambuf_iterator;
using std::make_shared;
using std::make_pair;
using std::pair;
using std::list;
using std::map;
using std::unordered_map;

struct TableObj
{
	int doc_id_;
	int hit_times_;
	//string hit_list;
	TableObj(int i, int j) :doc_id_(i), hit_times_(j){}
};

class BackTable
{
public:
	BackTable(const string & str) :file_name_(str)
	{
		cout << "loading back table" << endl;
		loadBackTable();
		cout << "loaded back table" << endl;
	}


	void changeFileName(const string & str)
	{
		file_name_ = str;
		loadBackTable();
	}

	vector<TableObj> & operator[](int word)
	{
		if (back_table_.find(word) != back_table_.end())
			return back_table_[word];
		else
		{
			return null_vec_;
		}
	}

private:


	void loadBackTable()
	{
		ifstream fobj(file_name_);
		if (!fobj.good())
		{
			cerr << "open backTable error" << endl;
			system("pause");
			exit(1);
		}
		std::stringstream sstrm;
		sstrm << fobj.rdbuf();
		string line;
		vector<int> vec(3);
		int cnt = 0;
		while (std::getline(sstrm, line))
		{
			parseLine(line,vec,3);
			back_table_[vec[0]].push_back(TableObj(vec[1], vec[2]));
			if (cnt ++ % 10000 == 0)
				cout << cnt << endl;
		}
		cout << "ok : " << cnt << endl;
		cout << back_table_.size() << endl;
	}

	void parseLine(const string & line, vector<int>& vec, int times)
	{
		vec.clear();
		string::const_iterator fir = line.begin();
		for (string::const_iterator it = line.begin(); times != 0 && it != line.end(); ++it)
		{
			if (*it != '\t')
				continue;

			vec.push_back(stoi(string(fir, it)));
			fir = it + 1;
			--times;
		}
	}

	string file_name_;
	vector<TableObj> null_vec_;
	unordered_map<int, vector<TableObj>> back_table_;
};

#endif