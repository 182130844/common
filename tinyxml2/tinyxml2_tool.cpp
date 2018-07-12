#include "tinyxml2_tool.h"
#ifdef _WIN32
#pragma warning(disable:4018)
#pragma warning(disable:4996)
#endif

bool tinyxml2_tool::get_int_attribute(XMLElement* pElm, const char* name, int& v)
{
	if(!pElm) return false;
	const char* pv = pElm->Attribute(name);
	if(!pv) return false;
    if (strlen(pv) == 0) return false;
	v = atoi(pv);
	return true;
}

bool tinyxml2_tool::get_str_attribute(XMLElement* pElm, const char* name, char* pszval, int nszlen)
{
	bool					ret = false;
	char*					pstrelm = NULL;

	if(pElm)
	{
		pstrelm = (char*)pElm->Attribute(name);
		if(!pstrelm)
			return false;
		if(nszlen > (int)strlen(pstrelm))
		{
			strcpy(pszval, pstrelm);
			ret = true;
		}
		else
		{
			memcpy(pszval, pstrelm, nszlen-1);
			pszval[nszlen-1] = 0;
			ret = true;
		}
	}

	return ret;
}

bool tinyxml2_tool::get_int64_attribute(XMLElement* pElm, const char* name, long long& v)
{
	if(!pElm) return false;
	const char* pv = pElm->Attribute(name);
	if(!pv) return false;
#ifdef _WIN32
    v = _atoi64(pv);
#else
    v = atol(pv);
#endif
	return true;
}

bool tinyxml2_tool::get_int_attribute(XMLElement* pElm, const char* name, long& v)
{
	if(!pElm) return false;
	const char* pv = pElm->Attribute(name);
	if(!pv) return false;
	v = atoi(pv);
	return true;
}

bool tinyxml2_tool::get_int_attribute_defaultvalue(XMLElement* pElm, const char* name, int& v, int DefaultValue)
{
	if(!pElm) return false;
	const char* pv = pElm->Attribute(name);
	if(!pv)
		v = DefaultValue;
	else
		v = atoi(pv);
	return true;

}

bool tinyxml2_tool::get_double_attribute(XMLElement* pElm, const char* name, double& v)
{
    if(!pElm) return false;
    const char* pv = pElm->Attribute(name);
    if(!pv)return false;
    v = atof(pv);
    return true;
}