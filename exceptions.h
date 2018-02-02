
#pragma once
#include <exception>

/// Base class for all exceptions.
struct Exception : virtual std::exception
{
	Exception(std::string _message = std::string()) : m_message(std::move(_message)) {}
	const char* what() const noexcept override { return m_message.empty() ? std::exception::what() : m_message.c_str(); }

private:
	std::string m_message;
};

struct InterfaceNotSupported : virtual Exception { public: InterfaceNotSupported(std::string const& _f) : Exception("Interface " + _f + " not supported.") {} };
struct ExternalFunctionFailure : virtual Exception { public: ExternalFunctionFailure(std::string const& _f) : Exception("Function " + _f + "() failed.") {} };

#define DEV_THROW_EXCEPTION(x) throw(x) 
#define DEV_IGNORE_EXCEPTIONS(x) try { x; } catch (...) {}