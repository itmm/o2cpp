#pragma once

#include "SYSTEM.h"

#include "Token.h"

extern SYSTEM_INTEGER Scanner_token;
auto Scanner_isDigit(SYSTEM_CHAR Scanner_ch) -> SYSTEM_BOOLEAN;
auto Scanner_isLetter(SYSTEM_CHAR Scanner_ch) -> SYSTEM_BOOLEAN;
void Scanner_init_module();
