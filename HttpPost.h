#pragma once
#include <Windows.h>
#include <WinInet.h>
#pragma comment(lib, "WinInet.lib")

#include <string>
using std::string;
using std::wstring;

class HttpPost {

public:

	static bool PostData(const wstring& strUrl, const wstring& strData, wstring& strResponse);
	static bool GetData(const wstring& strUrl, wstring& strResponse);
	static bool PostData(const string& strUrl, const string& strData, string& strResponse);
	static bool GetData(const string& strUrl, string& strResponse);
	
	static wstring UTF8ToWide(const string &utf8);
	static string WTA(const wstring& str);
	static wstring ATW(const string& str);

private:

	static bool ParseURL(const wstring& strUrl, INTERNET_PORT& port, wstring& strScheme, wstring& strHost, wstring& strPath);
	static bool ReadResponse(HINTERNET hInternet, wstring& strResponse);
};

