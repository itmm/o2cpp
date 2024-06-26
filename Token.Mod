MODULE Token;

    CONST
        unknown* = 0; eof* = 1; identifier* = 2; integerLiteral* = 3;
        floatLiteral* = 4; stringLiteral* = 5; charLiteral* = 6; plus* = 7;
        minus* = 8; star* = 9; slash* = 10; leftParenthesis* = 11;
        rightParenthesis* = 12; semicolon* = 13; period* = 14; comma* = 15;
        colon* = 16; assign* = 17; equals* = 18; bar* = 19; notEquals* = 20;
        leftBracket* = 21; rightBracket* = 22; ptr* = 23; andop* = 24;
        notop* = 25; leftBrace* = 26; rightBrace* = 27; less* = 28;
        lessOrEqual* = 29; greater* = 30; greaterOrEqual* = 31; range* = 32;
        kwARRAY* = 33; kwBEGIN* = 34; kwBY* = 35; kwCASE* = 36; kwCONST* = 37;
        kwDIV* = 38; kwDO* = 39; kwEND* = 40; kwELSE* = 41; kwELSIF* = 42;
        kwFALSE* = 43; kwFOR* = 44; kwIF* = 45; kwIMPORT* = 46; kwIN* = 47;
        kwIS* = 48; kwMOD* = 49; kwMODULE* = 50; kwNIL* = 51; kwOF* = 52;
        kwOR* = 53; kwPOINTER* = 54; kwPROCEDURE* = 55; kwRECORD* = 56;
        kwREPEAT* = 57; kwRETURN* = 58; kwTHEN* = 59; kwTO* = 60; kwTRUE* = 61;
        kwTYPE* = 62; kwUNTIL* = 63; kwVAR* = 64; kwWHILE* = 65;

END Token.