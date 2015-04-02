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
	NT_UNKNOWN, ///<δ֪
	NT_A,   ///<a��ǩ
	NT_IMG  ///<img��ǩ
};

struct NodeInfo
{
	NodeType type;
	string   host;     ///<������
	string   resource; ///<��Դ·��
	string   text;     ///<�����ı�,�����a��ǩ,����html�ı�,�����img��ǩ,����alt�����ı�
};

class CrawlerBase
{
public:
	///����html�еĿո�ͻ��з�
	char* RemoveSpaceAndNewlines(char* str);

	///����a��ǩ
	char* ParseANode(char* html,NodeInfo* pInfo);

	///����img��ǩ
	char* ParseImgNode(char* html,NodeInfo* pInfo);

	///��url�����ļ���,��ʵֻ��ȥ��һЩ�������ļ�����ʹ�õ��ַ�
	string ToFileName(string url);

	///utf-8תgbk,windows�µ�������õ�
	string UTF8ToGBK(const std::string& strUTF8);

	///����URL������������������Դ��  
	bool ParseURL(const string& url,string* pHost,string* pResource);

	///ʹ��Get���󣬵õ���Ӧ  
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