
#include "CountScore.h"

//#define DEBUGSEARCH
#define DEBUGCOUNTSCORE


int main()
{
#ifdef DEBUGSEARCH
	
	string line;
	SearchSystemIdf search_system((SEARCH_SEG_MODE()));
 	while (cout << "input sentence" << endl,getline(cin, line))
	{
		vector<pair<int, double>> flag;
		search_system.oneSearch(line);
		
	}
#endif
#ifdef DEBUGCOUNTSCORE
	CountScoreIdf count((SEARCH_SEG_MODE()));
	count.count_score();
#endif
	system("pause");
}


