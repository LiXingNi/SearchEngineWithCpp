#ifndef SEARCH_SYSTEM_BASE_H_
#define SEARCH_SYSTEM_BASE_H_

#include <Python.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <unordered_map>

using std::vector;
using std::cin;
using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::unordered_map;
using std::fstream;
using std::ifstream;

#define DOC_SCORE_DIR	"data/qu.label"
#define PYTHON_DIR		"pytest"
#define DATA_DIR		"data/dataOrg_gbk.lst"


#define OUTPUT_SCORE_DIR_ACCURATE_MODE_BASE	"data/cutData/score_base.lst"
#define OUTPUT_SCORE_DIR_ACCURATE_MODE_IDF	"data/cutData/score_idf.lst"
#define WORD_DCT_DIR_ACCUTATE_MODE			"data/cutData/wordDct_gbk_cut.lst"
#define BACK_TABLE_DIR_ACCURATE_MODE		"data/cutData/backTable_u_cut.txt"

#define OUTPUT_SCORE_DIR_SEARCH_MODE_BASE	"data/score_base.lst"
#define OUTPUT_SCORE_DIR_SEARCH_MODE_IDF	"data/socre_idf.lst"
#define WORD_DCT_DIR_SEARCH_MODE			"data/wordDct_gbk.lst"
#define BACK_TABLE_DIR_SEARCH_MODE			"data/backTable.lst"


#define FUNC_NAME_IDF "idfSeg"
#define FUNC_NAME_BASE "seg"

struct ACCURATE_SEG_MODE
{
	ACCURATE_SEG_MODE() = default;
};

struct SEARCH_SEG_MODE{};

struct BASE_SEARCH_ALGORITHM{};

struct IDF_SEARCH_ALGORITHM{};

#endif