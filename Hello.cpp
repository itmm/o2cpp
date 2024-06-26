#include "Hello.h"

static void init_module_imports() {
	Out_init_module();
}

auto Hello_isDigit(SYSTEM_CHAR Hello_ch) -> SYSTEM_BOOLEAN {
	return Hello_ch >= Oberon_String { "0" } && Hello_ch <= Oberon_String { "9" };
}
void Hello_init_module() {
	static bool already_run { false };
	if (already_run) { return; }
	already_run = true;
	init_module_imports();
	if (3 < 4) {
		Out_WriteInt(42);
	} else {
		Out_WriteInt(-42);
	}
	Out_WriteLn();
}
