#pragma once

/*

     SHADOW 
     9/26 2017 
	 SHENZHEN

*/


#include <string>
#include <unordered_map>
using std::unordered_map;
using std::string;


class section : public unordered_map<string, string>
{
public:
	section() = default;
	section(string sc) {
		_name = sc;
	}
	void name(string name) {
		_name = name;
	}
	string name() const { return _name; }
private:
	string _name;
};

class ini_file
{
public:
	ini_file(string&& file) {
		load_file(file);
	}
	~ini_file() = default;

	bool load_file(string file);

	section operator[] (string name);

	bool section_exists(string name);

	section get_section(string name);

	bool add_section(string name, string keyvalue = "");
	void set_value(string sc, string key, string value);
	void add_key(string sc, string key, string value = "");
	bool delete_section(string name);

private:
	string _file_path;
	unordered_map<string, section>  _data;
};