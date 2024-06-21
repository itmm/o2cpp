#include "Hello.h"

static void init_module_imports() {
	Out_init_module();
}

void Hello_init_module() {
	init_module_imports();
	Out_WriteInt(42);
	Out_WriteLn();
}
