#include <utility>

#pragma once

using SYSTEM_INTEGER = int;
using SYSTEM_REAL = double;
using SYSTEM_CHAR = char;
using SYSTEM_BOOLEAN = bool;

class Oberon_String {
	private:
		const char* str_;

	public:
		explicit Oberon_String(const char* str): str_ { str } { }
		operator const char*() const { return str_; }
		operator char() const { return str_[0]; }
};
