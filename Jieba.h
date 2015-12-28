#ifndef SEARCH_SYSTEM_JIEBA_H_
#define SEARCH_SYSTEM_JIEBA_H_

#include "Base.h"

class EZerr
{
public:
	EZerr(const string & s) :err_info(s){}

	const string& what()
	{
		return err_info;
	}

	string err_info;
};



class JieBa
{
private:
	void initModule()
	{
		try
		{
			Py_Initialize(); //初始化python
			check(Py_IsInitialized(), "initialized_error");
			PyRun_SimpleString("import  sys"); // 执行 python 中的短语句
			PyRun_SimpleString("sys.path.append('./')");

			PyObject * pName(nullptr), *pModule(nullptr);

			pName = PyString_FromString(file_name_.c_str());
			pModule = PyImport_Import(pName);
			check(pModule, "can not find file");
			pDct_ = PyModule_GetDict(pModule);
			check(pDct_, "pDct error");
		}
		catch (EZerr err)
		{
			cerr << err.what() << endl;
			system("pause");
			exit(-1);
		}
	}

protected:
	string file_name_;
	PyObject * pDct_, *pFunc_, *pArgs_;


	void check(bool flag, const string & err)const
	{
		if (!flag)
			throw EZerr(err);
	}

public:
	JieBa(const string & f = PYTHON_DIR) :file_name_(f), pDct_(nullptr), pFunc_(nullptr)
	{
		initModule();
	}

	virtual void cutForSearch(const string &, vector<pair<string,double>>&) = 0;  //提供接口
};



class JieBaBase : public JieBa
{
private:

	void initFunc()
	{
		pFunc_ = PyDict_GetItemString(pDct_, FUNC_NAME_BASE);
		check(PyCallable_Check(pFunc_), "pFunc init error");
	}

public:
	JieBaBase() :JieBa(){ initFunc(); }



	void cutForSearch(const string & sentence, vector<pair<string,double>> & result) override
	{
		check(pFunc_, "not init pFunc");

		pArgs_ = PyTuple_New(1);
		PyTuple_SetItem(pArgs_, 0, Py_BuildValue("s", sentence.c_str()));

		PyObject* pValue = NULL;  //定义返回值
		pValue = PyObject_CallObject(pFunc_, pArgs_); //获取返回值

		int sizes = int(PyList_Size(pValue));  //获取 list 的大小
		for (int i = 0; i != sizes; ++i)
		{
			string s;
			PyObject * ret = PyList_GetItem(pValue, i); //按索引获取返回值
			s = PyString_AsString(ret); //将返回的 python 字符串转为C风格字符串
			result.emplace_back(s,0);
		}
	}

};

class JieBaIDF : public JieBa
{
	void initFunc()
	{
		pFunc_ = PyDict_GetItemString(pDct_, FUNC_NAME_IDF);
		check(PyCallable_Check(pFunc_), "pFunc init error");
	}

public:
	JieBaIDF() :JieBa(){ initFunc(); }

	void cutForSearch(const string  & sentence, vector<pair<string, double>> & result) override
	{
		check(pFunc_, "not init pFunc");

		pArgs_ = PyTuple_New(1);
		PyTuple_SetItem(pArgs_, 0, Py_BuildValue("s", sentence.c_str()));

		PyObject* pValue = NULL;  //定义返回值
		pValue = PyObject_CallObject(pFunc_, pArgs_); //获取返回值

		int sizes = int(PyList_Size(pValue));  //获取 list 的大小

		//预先判断是否获取的 word与 idf 是成对的
		if (sizes % 2 != 0)
		{
			cerr << "python extract idf error" << endl;
			system("pause");
		}
		try
		{
			for (int i = 0; i < sizes; ++i)
			{

				PyObject * ret = PyList_GetItem(pValue, i); //按索引获取返回值
				PyObject * weight = PyList_GetItem(pValue, ++i);
				string word = PyString_AsString(ret); //将返回的 python 字符串转为C风格字符串
				double w = PyFloat_AsDouble(weight);
				//cout << word << " : " << w << endl;
				result.emplace_back(word, w);
			}
		}
		catch (...)
		{
			cout << sentence << " error in cutForSearch_idf" << endl;
			system("pause");
		}
	}

};
/*
class JieBa
{
public:
	JieBa() = delete;
	JieBa(BASE_SEARCH_ALGORITHM) :
		file_name_(PYTHON_DIR),
		pDct_(nullptr), pFunc_(nullptr),
		pArgs_(nullptr), pValues_(nullptr)
	{
		initModule();
		initFunc(BASE_SEARCH_ALGORITHM());
	}

	JieBa(IDF_SEARCH_ALGORITHM) :
		file_name_(PYTHON_DIR),
		pDct_(nullptr), pFunc_(nullptr),
		pArgs_(nullptr), pValues_(nullptr)
	{
		initModule();
		initFunc(IDF_SEARCH_ALGORITHM());
	}




	void initFunc(BASE_SEARCH_ALGORITHM)
	{
		pFunc_ = PyDict_GetItemString(pDct_, FUNC_NAME_BASE);
		check(PyCallable_Check(pFunc_), "pFunc init error");
	}

	void initFunc(IDF_SEARCH_ALGORITHM)
	{
		pFunc_ = PyDict_GetItemString(pDct_, FUNC_NAME_IDF);
		check(PyCallable_Check(pFunc_), "pFunc init error");
	}




	void cutForSearch(string  sentence, vector<string> & result)
	{
		check(pFunc_, "not init pFunc");

		pArgs_ = PyTuple_New(1);
		PyTuple_SetItem(pArgs_, 0, Py_BuildValue("s", sentence.c_str()));

		PyObject* pValue = NULL;  //定义返回值
		pValue = PyObject_CallObject(pFunc_, pArgs_); //获取返回值

		int sizes = int(PyList_Size(pValue));  //获取 list 的大小
		for (int i = 0; i != sizes; ++i)
		{
			string s;
			PyObject * ret = PyList_GetItem(pValue, i); //按索引获取返回值
			s = PyString_AsString(ret); //将返回的 python 字符串转为C风格字符串
			result.push_back(s);
		}
	}

	void cutForSearch_idf(string sentence, vector<pair<string,double>> & result)
	{
		check(pFunc_, "not init pFunc");

		pArgs_ = PyTuple_New(1);
		PyTuple_SetItem(pArgs_, 0, Py_BuildValue("s", sentence.c_str()));

		PyObject* pValue = NULL;  //定义返回值
		pValue = PyObject_CallObject(pFunc_, pArgs_); //获取返回值

		int sizes = int(PyList_Size(pValue));  //获取 list 的大小
		
		//预先判断是否获取的 word与 idf 是成对的
		if (sizes % 2 != 0)
		{
			cerr << "python extract idf error" << endl;
			system("pause");
		}
		try
		{
			for (int i = 0; i < sizes; ++i)
			{
				
				PyObject * ret = PyList_GetItem(pValue, i); //按索引获取返回值
				PyObject * weight = PyList_GetItem(pValue, ++i);
				string word = PyString_AsString(ret); //将返回的 python 字符串转为C风格字符串
				double w = PyFloat_AsDouble(weight);
				//cout << word << " : " << w << endl;
				result.emplace_back(word,w);
			}
		}
		catch (...)
		{
			cout << sentence << " error in cutForSearch_idf" << endl;
			system("pause");
		}
	}



private:
	string file_name_;
	PyObject* pDct_, *pFunc_, *pArgs_, *pValues_;
};

*/
#endif