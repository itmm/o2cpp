#include "Token.h"

static void init_module_imports() {
}

void Token_init_module() {
	static bool already_run { false };
	if (already_run) { return; }
	already_run = true;
	init_module_imports();
}
