#include "ini.h"
#include <windows.h>


bool ini_file::load_file(string file)
{
	_file_path = file;

	CHAR buf[40960] = { 0 };
	DWORD bytes = GetPrivateProfileSectionNamesA(buf, sizeof(buf), file.c_str());

	CHAR* ptr = &buf[bytes];
	DWORD sz = sizeof(buf) - bytes;

	_data.clear();

	for (DWORD i = 0; i < bytes;) {
		
		if (buf[i] == 0)
			break;

		string name(&buf[i]);
		printf("[%s]\n", name.c_str());
		i += name.length() + 1;

		DWORD copy_size = GetPrivateProfileStringA(name.c_str(), NULL, NULL, ptr, sz, file.c_str());
		
		section sc(name);
		for (DWORD j = 0; j < copy_size;) {
			if (ptr[j] == 0)
				break;

			string key(&ptr[j]);
			CHAR value[4096] = { 0 };
			GetPrivateProfileStringA(name.c_str(), key.c_str(), NULL, value, sizeof(value), file.c_str());
			printf("%s=%s\n", key.c_str(), value);
			j += key.length() + 1;

			sc[key] = value;
		}
		_data[name] = sc;
	}

	return true;
}

section ini_file::operator[](string name)
{	
	return _data[name];
}

bool ini_file::section_exists(string name)
{
	auto it = _data.find(name);
	return (it != _data.end());
}

section ini_file::get_section(string name)
{
	return _data[name];
}

bool ini_file::add_section(string name, string keyvalue/*= ""*/) {
	if (section_exists(name))
		return false;

	section sc(name);
	_data[name] = sc;

	WritePrivateProfileSectionA(name.c_str(), keyvalue.c_str(), _file_path.c_str());
	return true;
}

void ini_file::set_value(string sc, string key, string value) {
	auto& it = _data[sc];
	it[key] = value;
	WritePrivateProfileStringA(sc.c_str(), key.c_str(), value.c_str(), _file_path.c_str());
}

void ini_file::add_key(string sc, string key, string value /*= ""*/) {
	set_value(sc, key, value);
}

bool ini_file::delete_section(string name) {
	auto it = _data.find(name);
	if (it == _data.end())
		return false;

	_data.erase(it);
	WritePrivateProfileSectionA(name.c_str(), NULL, _file_path.c_str());
	return true;
}

