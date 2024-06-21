#include "Scanner.h"

static void init_module_imports() {
	Token_init_module();
}

SYSTEM_INTEGER Scanner_token;
void Scanner_init_module() {
	static bool already_run { false };
	if (already_run) { return; }
	already_run = true;
	init_module_imports();
	Scanner_token = Token_unknown;
}
