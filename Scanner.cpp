#include "Scanner.h"

static void init_module_imports() {
	Token_init_module();
}

SYSTEM_INTEGER Scanner_token;
auto Scanner_isDigit(SYSTEM_CHAR Scanner_ch) -> SYSTEM_BOOLEAN {
	return Scanner_ch >= Oberon_String { "0" } && Scanner_ch <= Oberon_String { "9" };
}
auto Scanner_isLetter(SYSTEM_CHAR Scanner_ch) -> SYSTEM_BOOLEAN {
	return (Scanner_ch >= Oberon_String { "a" } && Scanner_ch <= Oberon_String { "z" }) || (Scanner_ch >= Oberon_String { "A" } && Scanner_ch <= Oberon_String { "Z" });
}
auto Scanner_isWhitespace(SYSTEM_CHAR Scanner_ch) -> SYSTEM_BOOLEAN {
	return Scanner_ch == Oberon_String { " " } || Scanner_ch == '\x09' || Scanner_ch == '\x0C' || Scanner_ch == '\x0B' || Scanner_ch == '\x0A' || Scanner_ch == '\x0D';
}
void Scanner_init_module() {
	static bool already_run { false };
	if (already_run) { return; }
	already_run = true;
	init_module_imports();
	Scanner_token = Token_unknown;
}
