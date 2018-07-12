#include <stdio.h>
#include <iostream>
#include "ini_file.h"

namespace shadow
{

#define LINE_BUFFER_SIZE 2048

	using std::ios;

	ini_file::ini_file(string&& file)
	{
		load_file(file);
	}

	ini_file::~ini_file()
	{
		close_file();
	}

	bool ini_file::load_file(const string& path) {

		close_file();

		path_ = path;

		fs_.open(path_, ios::in | ios::binary);

		if (!fs_.is_open()) return false;

		section sc;
		string comment;

		while (!fs_.eof()) {
			char buffer[LINE_BUFFER_SIZE] = { 0 };
			fs_.getline(buffer, sizeof(buffer), '\n');

			string line(buffer);
			if (line.empty()) {
				continue;
			}

			line.erase(0, line.find_first_not_of(' '));
			line.erase(line.find_last_not_of(' ') + 1);

			if (line[0] == '#') {
				comment += line + "\n";
				continue;
			}
			if (line[0] == '[') {
				auto it = line.find_first_of(']');
				if (it != string::npos) {

					string name = line.substr(1, it - 1);
					if (section_exists(name)) {
						printf("[%s] already exists\n", name.c_str());
						return false;
					}
					if (sc.name().empty()) {
					__clear:
						sc.name(name);
						sc.comment(comment);
						comment.clear();
					}
					else {
						add_section(sc);
						sc.clear();
						goto __clear;
					}
					continue;
				}
				else {
					printf("[section] can't find char ']'\n");
					return false;
				}
			}

			if (sc.name().empty()) {
				printf("[ini] no section");
				return false;
			}

			auto it = line.find_first_of('=');
			if (it == string::npos) {
				printf("[key-value] can't find char '='\n");
				return false;
			}
	
			sc[line.substr(0, it)] = line.substr(it + 1);
		}

		if (!sc.name().empty()) {
			sc.add_cmt(comment);
			add_section(sc);
		}

		return true;
	}
	
	section& ini_file::operator[](const string& name)
	{
		return data_[name];
	}
	
	bool ini_file::section_exists(const string& name)
	{
		auto it = data_.find(name);
		return (it != data_.end());
	}

	section ini_file::get_section(const string& name)
	{
		return data_[name];
	}

	bool ini_file::add_section(const section& sc)
	{
		if (section_exists(sc.name()))
			return false;

		data_[sc.name()] = sc;
		return true;
	}

	bool ini_file::add_section(const string& scname, const string& key, const string& value) {
		if (section_exists(scname))
			return false;

		shadow::section sc(scname);
		sc[key] = value;
		data_[scname] = sc;
		return true;
	}

	void ini_file::set_key_value(const string& scname, const string& key, const string& value) {
		data_[scname][key] = value;
	}

	bool ini_file::delete_section(const string& name) {
		auto it = data_.find(name);
		if (it == data_.end())
			return false;

		data_.erase(it);
		return true;
	}
	
	bool ini_file::save_to_file() {
		close_file();

		fs_.open(path_, ios::out | ios::binary | ios::trunc);
		if (!fs_.is_open()) return false;

		for (auto it = data_.begin(); it != data_.end(); it++) {
			fs_ << "\n";
			fs_ << it->second.comment();
			fs_ << "\n[";
			fs_ << it->second.name() + "]\n";
			for (auto sc : it->second) {
				fs_ << sc.first;
				fs_ << "=";
				fs_ << sc.second + "\n";
			}
		}

		close_file();
		return true;
	}

	void ini_file::close_file()
	{
		if (fs_.is_open()) {
			fs_.close();
		}
	}

}


