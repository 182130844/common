
#pragma once
#include "tinyxml2.h"
using namespace tinyxml2;

class tinyxml2_tool
{
public:
	static bool get_int_attribute(XMLElement* pElm, const char* name, int& v);
	static bool get_str_attribute(XMLElement* pElm, const char* name, char* pszval, int nszlen);
	static bool get_int64_attribute(XMLElement* pElm, const char* name, long long& v);
	static bool get_int_attribute(XMLElement* pElm, const char* name, long& v);
	static bool get_int_attribute_defaultvalue(XMLElement* pElm, const char* name, int& v, int DefaultValue);
    static bool get_double_attribute(XMLElement* pElm, const char* name, double& v);
};
