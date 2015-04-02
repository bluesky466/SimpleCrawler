#ifndef _CRAWLER_H_
#define _CRAWLER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include "winsock2.h"
#include <windows.h> 

#pragma comment(lib, "ws2_32.lib")   
using namespace std;

enum NodeType
{
	NT_UNKNOWN, ///<未知
	NT_A,   ///<a标签
	NT_IMG  ///<img标签
};

struct NodeInfo
{
	NodeType type;
	string   host;     ///<主机名
	string   resource; ///<资源路径
	string   text;     ///<描述文本,如果是a标签,保存html文本,如果是img标签,保存alt属性文本
};

class CrawlerBase
{
public:
	///消除html中的空格和换行符
	char* RemoveSpaceAndNewlines(char* str);

	///解析a标签
	char* ParseANode(char* html,NodeInfo* pInfo);

	///解析img标签
	char* ParseImgNode(char* html,NodeInfo* pInfo);

	///从url生成文件名,其实只是去掉一些不能在文件名中使用的字符
	string ToFileName(string url);

	///utf-8转gbk,windows下调试输出用到
	string UTF8ToGBK(const std::string& strUTF8);

	///解析URL，解析出主机名，资源名  
	bool ParseURL(const string& url,string* pHost,string* pResource);

	///使用Get请求，得到响应  
	bool GetHttpResponse(const NodeInfo & info,char** ppBuffer,long bufferSize,long* pBytesRead);
};


#define DEFAULT_PAGE_BUF_SIZE 1024000

class SimpleCrawler : public CrawlerBase
{
public:
	SimpleCrawler():
		m_outputA("outputA.txt"),
		m_outputImg("outputImg.txt")
	{
	}

	void Run();

private:
	list<NodeInfo> m_visibleList;
	list<NodeInfo> m_workList;
	ofstream m_outputA;
	ofstream m_outputImg;
	
	bool isMatch(const string& text) {return true;}

	char* readFile(const char* fn);
	void  removeNodeByHost(string host);
	bool  isInVisibleVec(const NodeInfo& node);
	void  saveHtml(const string& fnHtml,const char* html);
	void  scanHtml(char* html);
};
#endif