#pragma once

//
//  常用函数
//

namespace shadow {

	// only works on the Windows platform
	bool init_win_socket();
	bool clean_win_socket();

	int sleep(int64_t ms);

	int64_t current_milliseconds();

	int64_t time_stoi(const char* st);
	int64_t date_stoi(const char* st);
	void time_itos(int64_t t, char* out);
	void time_itos_d(int64_t t, char* out);
	void time_itos_md(int64_t t, char* out);
	void time_itostamp(int64_t t, char* out);
}