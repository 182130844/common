
/*

     shadow_yuan@qq.com 
     shenzhen 9/26 2017 

*/

#pragma once
#include <string>
#include <unordered_map>
#include <fstream>

namespace shadow {

	using std::string;
	using std::unordered_map;

	class section : public unordered_map<string, string>
	{
	public:
		section() = default;
		section(const string& sc) :name_(sc) {}
		void name(string& name) {
			name_ = name;
		}
		string name() const { return name_; }
		string comment() const { return comment_; }
		void comment(string& cmt) {
			comment_ = cmt;
		}
		void add_cmt(string& cmt) {
			comment_ += cmt;
		}
		void clear() {
			name_.clear();
			comment_.clear();
			unordered_map<string, string>::clear();
		}
	private:
		string name_;
		string comment_;
	};

	class ini_file
	{
	public:
	
		ini_file() = default;
		~ini_file();

		ini_file(string&& file);

		bool load_file(const string& path);

		section& operator[] (const string& name);

		bool section_exists(const string& name);

		section get_section(const string& name);

		bool add_section(const section& sc);

		bool add_section(const string& scname, const string& key, const string& value = "");

		/*
		 *  1.section不存在时添加section
		 *  2.value为空时，表示清空key的value值
		 */
		void set_key_value(const string& scname, const string& key, const string& value = "");
		
		bool delete_section(const string& name);

		bool save_to_file();

		void close_file();

	private:

		string path_;
		unordered_map<string, section>  data_;
		std::fstream fs_;
	};

}
