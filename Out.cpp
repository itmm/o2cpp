#include "Out.h"

#include <iostream>

void Out_init_module() { }

void Out_WriteLn() { std::cout << "\n"; }
void Out_WriteInt(SYSTEM_INTEGER value) { std::cout << value; }
