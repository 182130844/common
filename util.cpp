#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include "util.h"

#ifdef _WIN32
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

namespace shadow {

#ifdef _WIN32
	static int gettimeofday(struct timeval* tp, void* tzp) {
		SYSTEMTIME st;
		GetLocalTime(&st);
		struct tm t;
		t.tm_year = st.wYear - 1900;
		t.tm_mon = st.wMonth - 1;
		t.tm_mday = st.wDay;
		t.tm_hour = st.wHour;
		t.tm_min = st.wMinute;
		t.tm_sec = st.wSecond;
		t.tm_isdst = -1;
		auto clock = mktime(&t);
		tp->tv_sec = (long)clock;
		tp->tv_usec = st.wMilliseconds * 1000;
		return 0;
	}

	static SOCKET s_ = INVALID_SOCKET;
#endif

	bool init_win_socket() {
#ifdef _WIN32
		WSADATA data;
		WORD wVersionRequested = MAKEWORD(2, 2);
		if (WSAStartup(wVersionRequested, &data) != 0) {
			return false;
		}

		if (s_ == INVALID_SOCKET) {
			s_ = socket(AF_INET, SOCK_STREAM, 0);
			return (s_ != INVALID_SOCKET);
		}
#endif
		return true;
	}

	bool clean_win_socket() {
#ifdef _WIN32
		if (s_ != INVALID_SOCKET) {
			closesocket(s_);
			s_ = INVALID_SOCKET;
		}
		return WSACleanup() == 0;
#endif
		return true;
	}

	int sleep(int64_t ms) {
		struct timeval tv;
		tv.tv_sec = long(ms / 1000);
		tv.tv_usec = (ms % 1000) * 1000;

#ifdef _WIN32
		fd_set fs;
		FD_ZERO(&fs);
		FD_SET(s_, &fs);
		return select(0, &fs, nullptr, nullptr, &tv);
#else
		return select(0, nullptr, nullptr, nullptr, &tv);
#endif
	}

	int64_t current_milliseconds() {
		struct timeval tp;
		gettimeofday(&tp, nullptr);
		int64_t ret = tp.tv_sec;
		return ret * 1000 + tp.tv_usec / 1000;
	}

	int64_t time_stoi(const char* st)
	{
		int64_t ret = 0;
		struct tm tm1;

		sscanf(st, "%4d-%2d-%2d %2d:%2d:%2d",
			&tm1.tm_year,
			&tm1.tm_mon,
			&tm1.tm_mday,
			&tm1.tm_hour,
			&tm1.tm_min,
			&tm1.tm_sec);
		tm1.tm_year -= 1900;
		tm1.tm_mon--;
		tm1.tm_isdst = -1;
		ret = mktime(&tm1);
		return ret;
	}

	int64_t date_stoi(const char* st) {
		int64_t ret = 0;
		struct tm tm1;

		sscanf(st, "%4d-%2d-%2d",
			&tm1.tm_year,
			&tm1.tm_mon,
			&tm1.tm_mday);
		tm1.tm_year -= 1900;
		tm1.tm_mon--;
		tm1.tm_isdst = -1;
		tm1.tm_hour = 0;
		tm1.tm_min = 0;
		tm1.tm_sec = 0;
		ret = mktime(&tm1);
		return ret;
	}

	void time_itos(int64_t t, char* out)
	{
		if (!out) return;
		tm t1 = *localtime(&t);
		int len = sprintf(out, "%04d-%02d-%02d %02d:%02d:%02d",
			t1.tm_year + 1900,
			t1.tm_mon + 1,
			t1.tm_mday,
			t1.tm_hour,
			t1.tm_min,
			t1.tm_sec);
		out[len] = 0;
	}

	void time_itos_d(int64_t t, char* out) {
		if (!out) return;
		tm t1 = *localtime(&t);
		int len = sprintf(out, "%04d-%02d-%02d 0:0:0",
			t1.tm_year + 1900,
			t1.tm_mon + 1,
			t1.tm_mday);
		out[len] = 0;
	}

	void time_itos_md(int64_t t, char* out) {
		if (!out) return;
		tm t1 = *localtime(&t);
		int len = sprintf(out, "%02d‘¬%02d»’",
			t1.tm_mon + 1,
			t1.tm_mday);
		out[len] = 0;
	}

	void time_itostamp(int64_t t, char* out) {
		if (!out) return;
		tm t1 = *localtime(&t);
		int len = sprintf(out, "%04d.%02d.%02d.%02d.%02d.%02d",
			t1.tm_year + 1900,
			t1.tm_mon + 1,
			t1.tm_mday,
			t1.tm_hour,
			t1.tm_min,
			t1.tm_sec);
		out[len] = 0;
	}


}