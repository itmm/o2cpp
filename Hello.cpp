#include "Hello.h"

static void init_module_imports() {
	static bool already_run { false };
	if (already_run) { return; }
	already_run = true;
;	Out_init_module();
}

void Hello_init_module() {
	init_module_imports();
	Out_WriteInt(42);
	Out_WriteLn();
}
