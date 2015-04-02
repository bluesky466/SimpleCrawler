#include "Crawler.h"

char* CrawlerBase::RemoveSpaceAndNewlines(char* str)
{
	long length = strlen(str);
	long replace = 0;
	long cur = 0;

	while(cur<=length)
	{
		if(str[cur]!=' ' && str[cur]!='\n')
			str[replace++] = str[cur];

		cur++;
	}
	
	return str;
}

char* CrawlerBase::ParseANode(char* html,NodeInfo* pInfo)
{
	pInfo->type = NT_UNKNOWN;
	pInfo->text.clear();
	pInfo->host.clear();
	pInfo->resource.clear();

	if(html==nullptr || *(html+1)!='a')
		return nullptr;
	
	char* leftEnd = strstr(html,">");
	char* rightBegin = strstr(html+1,"</a>");

	if(leftEnd==nullptr || rightBegin==nullptr || leftEnd>rightBegin)
		return nullptr;

	//提取href,如果没有链接的url,其实这个a标签没有意义,就直接return了
	char* href = strstr(html,"href=\"");
	if(href!=nullptr && href<rightBegin)
	{
		char* urlBegin = href + sizeof("href=\"")-1; //sizeof减去1就是strlen
		char* urlEnd = strstr(urlBegin, "\"");

		string url = "";
		if(urlEnd!=nullptr && urlEnd<leftEnd && urlEnd-urlBegin>0 && *urlBegin!='#')
		{
			for(int i = 0 ; i<urlEnd - urlBegin ; i++)
				url += *(urlBegin+i);
			
			if(!ParseURL(url,&(pInfo->host),&(pInfo->resource)))
				return nullptr;
		}
		else
		{
			return rightBegin + sizeof("</a>");
		}
	}
	else
		return rightBegin + sizeof("</a>");
	
	//提取html文本
	for(int i = 1 ; i<rightBegin - leftEnd ; i++)
		pInfo->text += *(leftEnd+i);

	pInfo->type = NT_A;

	int size = rightBegin - html + sizeof("</a>");

	return rightBegin + sizeof("</a>");
}

char* CrawlerBase::ParseImgNode(char* html,NodeInfo* pInfo)
{
	pInfo->type = NT_UNKNOWN;
	pInfo->text.clear();
	pInfo->host.clear();
	pInfo->resource.clear();

	if(html==nullptr || 0!=strncmp(html,"<img",sizeof("<img")-1)) //sizeof减去1就是strlen
		return nullptr;

	char* end = strstr(html,">");
	if(end==nullptr)
		return nullptr;

	//提取src,如果没有链接的url,其实这个img标签没有意义,就直接return了(虽然一般都会有)
	char* src = strstr(html,"src=\"");
	if(src!=nullptr && src<end)
	{
		char* urlBegin = src + sizeof("src=\"")-1; //sizeof减去1就是strlen
		char* urlEnd = strstr(urlBegin, "\"");

		if(urlEnd!=nullptr && urlEnd<end && urlEnd-urlBegin>0)
		{
			string url;
			for(int i = 0 ; i<urlEnd - urlBegin ; i++)
				url += *(urlBegin+i);

			if(!ParseURL(url,&(pInfo->host),&(pInfo->resource)))
				return nullptr;
		}
		else
		{
			return end + 1;
		}
	}
	else
	{
		return end + 1;
	}

	//提取alt,如果没有
	char* alt = strstr(html,"alt=\"");
	if(alt!=nullptr && alt<end)
	{
		char* urlBegin = alt + sizeof("alt=\"")-1; //sizeof减去1就是strlen
		char* urlEnd = strstr(urlBegin, "\"");

		if(urlEnd!=nullptr && urlEnd<end && urlEnd-urlBegin>0)
		{
			for(int i = 0 ; i<urlEnd - urlBegin ; i++)
				pInfo->text += *(urlBegin+i);
		}
	}

	pInfo->type = NT_IMG;

	return end + 1;
}

string CrawlerBase::ToFileName(string url)
{  
	string fileName;  
	fileName.resize(url.size());  
	int k=0;  
	for( int i=0; i<(int)url.length(); i++)
	{  
		char ch = url[i];  
		if( ch!='\\' && ch!='/' && ch!=':' && ch!='*' && ch!='?' && ch!='"' && ch!='<' && ch!='>' && ch!='|')  
			fileName[k++]=ch;  
	}  

	return fileName.substr(0,k);  
}

string CrawlerBase::UTF8ToGBK(const std::string& strUTF8)  
{  
    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);  
    unsigned short * wszGBK = new unsigned short[len + 1];  
    memset(wszGBK, 0, len * 2 + 2);  
    MultiByteToWideChar(CP_UTF8, 0, (LPCCH)strUTF8.c_str(), -1, (LPWSTR)wszGBK, len);  
  
    len = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)wszGBK, -1, NULL, 0, NULL, NULL);  
    char *szGBK = new char[len + 1];  
    memset(szGBK, 0, len + 1);  
    WideCharToMultiByte(CP_ACP,0, (LPCWCH)wszGBK, -1, szGBK, len, NULL, NULL); 

    std::string strTemp(szGBK);  
    delete[]szGBK;  
    delete[]wszGBK;  
    return strTemp;  
}

bool CrawlerBase::ParseURL(const string & url,string* pHost,string* pResource)
{
    if(strlen(url.c_str()) > 2000) 
        return false;  

	int pos = 0;

	//跳过前面的"http:// , http://, "//""
	if(0 == strncmp(url.c_str(),"http://",sizeof("http://") - 1))
		pos += sizeof("http://") - 1;

	else if(0 == strncmp(url.c_str(),"https://",sizeof("https://") - 1))
		pos += sizeof("https://") - 1;

	else if(0 == strncmp(url.c_str(),"//",sizeof("//") - 1))
		pos += sizeof("//") - 1;

	bool isAddSlash = false;
	const char* pSlash = strstr(url.c_str()+pos,"/");
	if(pSlash  == nullptr)
	{
		if(0 == strncmp(url.c_str(),"www.",sizeof("www.")-1))
			isAddSlash = true;
		else
			return false;
	}
	else if(pSlash-url.c_str()>99) //主机名不超过99个字符，因为下面的host数字大小为100
		return false;

    char host[100] = "";  
    char resource[2000] = "";  
	sscanf( url.c_str()+pos, "%[^/]%s", host, resource );  
    *pHost = host;  

	if(isAddSlash)
		*pResource="/";
	else
	  *pResource = resource;
	

    return true;  
}  
  
bool CrawlerBase::GetHttpResponse(const NodeInfo & info,char** ppBuffer,long bufferSize,long* pBytesRead)
{  
	if(info.host.length()==0)
		return false;

    //建立socket  
    struct hostent * hp= gethostbyname( info.host.c_str() );  
    if(hp==NULL)
	{
		cout<<"Can not find host address: "<<info.host<<"\n"<<endl;  
        return false;
	}
  
    SOCKET sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    if( sock == -1 || sock == -2 )
	{
		cout<<"Can not create sock\n"<<endl; 
        return false;
	}
  
    //建立服务器地址  
    SOCKADDR_IN sa;  
    sa.sin_family = AF_INET;  
    sa.sin_port = htons( 80 );  

    memcpy( &sa.sin_addr, hp->h_addr, 4 );  
  
	cout<<"connect: "<<info.host<<endl;  

    //建立连接  
    if( 0!= connect( sock, (SOCKADDR*)&sa, sizeof(sa) ) )
	{
		cout<<"Can not connect: "<<info.host<<"\n"<<endl; 
        closesocket(sock);  
        return false;  
    };  
  
    //准备发送数据  
    string request = "GET " + info.resource + " HTTP/1.1\r\nHost:" + info.host + "\r\nConnection:Close\r\n\r\n";  
  
    //发送数据  
    if(SOCKET_ERROR == send(sock, request.c_str(),request.size(), 0))
	{
        cout<<"send error\n"<<endl;
        closesocket( sock );  
        return false;  
    }  

    *pBytesRead = 0;  
    int ret = 1;  

	cout<<"recv resource : "<<info.resource<<endl;  
    while(ret > 0)
	{  
        ret = recv(sock, (*ppBuffer) + (*pBytesRead), bufferSize-(*pBytesRead)-1, 0);  
        
        if(ret > 0)
            *pBytesRead += ret;  
  
        if(*pBytesRead>=bufferSize-1)
		{
			cout<<"recv data is more then bufferSize:"<<bufferSize<<endl;
			return false;
		}
    }
  
    (*ppBuffer)[(*pBytesRead)] = '\0';  
    closesocket( sock );  

	cout<<"\n";

    return true;  
}


/*
	SimpleCrawler
*/
bool SimpleCrawler::isInVisibleVec(const NodeInfo& node)
{
	for(auto i = m_visibleList.begin() ; i!=m_visibleList.end() ; i++)
		if(node.host==i->host && node.text==i->text)
			return true;

	return false;
}

void SimpleCrawler::removeNodeByHost(string host)
{
	for(auto i = m_visibleList.begin() ; i!=m_visibleList.end() ; i++)
		if(host==i->host)
			i = m_visibleList.erase(i);
}


char* SimpleCrawler::readFile(const char* fn)
{
	ifstream in(fn,ios::_Nocreate);

	if(!in.is_open())
		return 0;

	//获得文件总长度
	in.seekg (0, ios::end);
	long size = in.tellg();
	in.seekg(0, ios::beg); 

	//读取文件
	char* buffer = new char[size];
	in.read(buffer,size);

	in.close(); 

	return buffer;
}

void SimpleCrawler::scanHtml(char* html)
{
	NodeInfo info;

	char* nodeBegin = strstr(html,"<");

	while(nodeBegin!=nullptr)
	{
		char *aNext,*imgNext;
		if(aNext = ParseANode(nodeBegin,&info))
		{
			if(!isInVisibleVec(info))
			{
				m_visibleList.push_back(info);
				m_workList.push_back(info);
				m_outputA<<UTF8ToGBK(info.text)<<":\nhost:  "<<info.host<<"\nres:  "<<info.resource<<"\n"<<endl;
			}
		}

		if(imgNext = ParseImgNode(nodeBegin,&info))
		{
			if(!isInVisibleVec(info))
			{
				m_outputImg<<UTF8ToGBK(info.text)<<":\nhost:  "<<info.host<<"\nres:  "<<info.resource<<"\n"<<endl;
			}
		}

		nodeBegin = strstr(nodeBegin+1,"<");
	}
}

void SimpleCrawler::saveHtml(const string& fnHtml,const char* html)
{
	ofstream out("html/"+ToFileName(fnHtml)+".txt");
	out<<html;
	out.close();
}

void SimpleCrawler::Run()
{
	CreateDirectory( L"html",NULL );
	
	WSADATA wsaData;  
    if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0 )
	{  
        return;  
    }

	//ScanHtml(RemoveSpaceAndNewlines(ReadFile("test.txt")),"test");

	NodeInfo info;
	info.text = "";
	ParseURL("www.baidu.com/",&info.host,&info.resource);
	info.type = NT_A;
	m_workList.push_back(info);
	long len;
	char* buffer = new char[DEFAULT_PAGE_BUF_SIZE];

	while(!m_workList.empty())
	{
		NodeInfo node = m_workList.back();
		m_workList.pop_back();

		if(GetHttpResponse(node,&buffer,DEFAULT_PAGE_BUF_SIZE,&len))
		{
			saveHtml(node.host+node.resource,buffer);
			scanHtml(RemoveSpaceAndNewlines(buffer));
		}
	}

	delete[] buffer;

	WSACleanup();  
	
	m_outputA.close();
	m_outputImg.close();
}