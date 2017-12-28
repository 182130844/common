#include <vector>
using std::vector;

#include "HttpPost.h"


static wchar_t g_UserAgent[] = L"Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)";

class AutoInternetHandle {
public:
	explicit AutoInternetHandle(HINTERNET handle) : handle_(handle) {}
	~AutoInternetHandle() {
		if (handle_) {
			InternetCloseHandle(handle_);
		}
	}

	HINTERNET get() { return handle_; }

private:
	HINTERNET handle_;
};

bool HttpPost::ParseURL(const wstring& strUrl, INTERNET_PORT& port, wstring& strScheme, wstring& strHost, wstring& strPath)
{
	wchar_t scheme[16], host[256], path[256];
	URL_COMPONENTS components;
	memset(&components, 0, sizeof(components));
	components.dwStructSize = sizeof(components);
	components.lpszScheme = scheme;
	components.dwSchemeLength = sizeof(scheme) / sizeof(scheme[0]);
	components.lpszHostName = host;
	components.dwHostNameLength = sizeof(host) / sizeof(host[0]);
	components.lpszUrlPath = path;
	components.dwUrlPathLength = sizeof(path) / sizeof(path[0]);

	if (!InternetCrackUrlW(strUrl.c_str(), static_cast<DWORD>(strUrl.size()), 0, &components))
		return false;

	strScheme = scheme;
	strHost = host;
	strPath = path;
	port = components.nPort;

	return true;
}

bool HttpPost::PostData(const wstring& strUrl, const wstring& strData, wstring& strResponse)
{
	INTERNET_PORT port = 0;

	wstring scheme, host, path;

	wstring strAccept = L"Accept: */*";
	wstring strHeader = L"Content-Type: application/x-www-form-urlencoded";

	if (!HttpPost::ParseURL(strUrl, port, scheme, host, path))
		return false;
	
	AutoInternetHandle internet(InternetOpenW(
		g_UserAgent, 
		INTERNET_OPEN_TYPE_PRECONFIG, 
		NULL, 
		NULL, 
		0));

	if (!internet.get())
		return false;

	AutoInternetHandle connection(InternetConnectW(
		internet.get(), 
		host.c_str(), 
		port, 
		NULL, 
		NULL, 
		INTERNET_SERVICE_HTTP, 
		0, 
		NULL));

	if (!connection.get())
		return false;

	AutoInternetHandle request(HttpOpenRequestW(
		connection.get(),
		L"POST",
		path.c_str(),
		NULL,    // If this parameter is NULL, the function uses HTTP/1.1 as the version
		NULL,    // referer
		NULL,    // agent type
		INTERNET_FLAG_DONT_CACHE,
		(DWORD_PTR)strAccept.c_str()));

	if (!request.get())
		return false;


	if (!HttpSendRequest(
		request.get(), 
		strHeader.c_str(),
		strHeader.length(),
		(LPVOID)strData.c_str(), //  提交ascii 字符
		strData.length()))
		return false;

	wstring strRead;
	if (!HttpPost::ReadResponse(request.get(), strRead))
		return false;
	
	strResponse = strRead.c_str();
	return true;
}


bool HttpPost::ReadResponse(HINTERNET hInternet, wstring& strResponse) {
	bool has_content_length_header = false;
	char content_length[32];
	DWORD content_length_size = sizeof(content_length);
	DWORD claimed_size = 0;
	string response_body;

	if (HttpQueryInfo(hInternet, HTTP_QUERY_CONTENT_LENGTH,
		static_cast<LPVOID>(&content_length),
		&content_length_size, 0)) {
		has_content_length_header = true;
		claimed_size = strtol(content_length, NULL, 10);
		response_body.reserve(claimed_size);
	}


	DWORD bytes_available;
	DWORD total_read = 0;
	BOOL return_code;

	while (((return_code = InternetQueryDataAvailable(hInternet, &bytes_available,
		0, 0)) != 0) && bytes_available > 0) {

		vector<char> response_buffer(bytes_available);
		DWORD size_read;

		return_code = InternetReadFile(hInternet,
			&response_buffer[0],
			bytes_available, &size_read);

		if (return_code && size_read > 0) {
			total_read += size_read;
			response_body.append(&response_buffer[0], size_read);
		}
		else {
			break;
		}
	}

	bool succeeded = return_code && (!has_content_length_header ||
		(total_read == claimed_size));
	if (succeeded) {
		strResponse = UTF8ToWide(response_body);
	}

	return succeeded;
}

bool HttpPost::GetData(const wstring& strUrl, wstring& strResponse)
{
	INTERNET_PORT port = 0;

	wstring scheme, host, path;

	wstring strAccept = L"Accept: */*";
	wstring strHeader = L"Content-Type: application/x-www-form-urlencoded";

	if (!HttpPost::ParseURL(strUrl, port, scheme, host, path))
		return false;
	
	AutoInternetHandle internet(InternetOpen(
		g_UserAgent, 
		INTERNET_OPEN_TYPE_PRECONFIG, 
		NULL, 
		NULL, 
		0));

	if (!internet.get())
		return false;
	
	AutoInternetHandle request(InternetOpenUrl(
		internet.get(),
		strUrl.c_str(),
		NULL,
		0,
		INTERNET_FLAG_TRANSFER_ASCII,
		0));

	if (!request.get())
		return false;

	wstring strContent;
	if (!HttpPost::ReadResponse(request.get(), strContent))
		return false;

	strResponse = strContent.c_str();
	return true;
}

bool HttpPost::PostData(const string& strUrl, const string& strData, string& strResponse) {
	wstring response;
	wstring url = ATW(strUrl);
	wstring data = ATW(strData);
	if (PostData(url, data, response)) {
		strResponse = WTA(response);
		return true;
	}
	return false;
}

bool HttpPost::GetData(const string& strUrl, string& strResponse) {
	wstring response;
	wstring url = ATW(strUrl);
	if (GetData(url, response)) {
		strResponse = WTA(response);
		return true;
	}
	return false;
}

wstring HttpPost::UTF8ToWide(const string &utf8) {
	if (utf8.length() == 0) {
		return wstring();
	}

	// compute the length of the buffer we'll need
	int charcount = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);

	if (charcount == 0) {
		return wstring();
	}

	// convert
	wchar_t* buf = new wchar_t[charcount];
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, buf, charcount);
	wstring result(buf);
	delete[] buf;
	return result;
}

string HttpPost::WTA(const wstring& str)
{
	string ret;

	if (str.empty())
		return ret;

	int nlen = WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
	ret.resize(nlen);
	WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, (char*)ret.c_str(), nlen, NULL, NULL);
	return ret;
}

wstring HttpPost::ATW(const string& str)
{
	wstring ret;
	if (str.empty())
		return ret;

	int nlen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	ret.resize(nlen);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (wchar_t*)ret.c_str(), nlen);
	return ret;
}