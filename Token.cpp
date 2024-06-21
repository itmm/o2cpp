#include "Token.h"

static void init_module_imports() {
	static bool already_run { false };
	if (already_run) { return; }
	already_run = true;
}

void Token_init_module() {
	init_module_imports();
}
