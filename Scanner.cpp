#include "Scanner.h"

static void init_module_imports() {
	static bool already_run { false };
	if (already_run) { return; }
	already_run = true;
	Token_init_module();
}

SYSTEM_INTEGER Scanner_token;
void Scanner_init_module() {
	init_module_imports();
}
