#pragma once

class connection_base
{
public:
	virtual unsigned int init(const char* charset = "utf8mb4") = 0;
	virtual bool         open(const char* host, const char* username, const char* pwd, const char* dbname, unsigned short port) = 0;
	virtual int          select_db(const char* db) = 0;
};
