#include "Out.h"

#include <iostream>

void Out_WriteLn() { std::cout << "\n"; }
void Out_WriteInt(SYSTEM_INTEGER value) { std::cout << value; }

void Out_init_module() {
	static bool already_run { false };
	if (already_run) { return; }
	already_run = true;
}

