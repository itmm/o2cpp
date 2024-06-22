MODULE Scanner;

    IMPORT T := Token;

    VAR
        token: INTEGER;

    PROCEDURE isDigit(ch: CHAR): BOOLEAN;
        RETURN ch >= "0" & ch <= "9"
    END isDigit;

    PROCEDURE isLetter(ch: CHAR): BOOLEAN;
        RETURN (ch >= "a" & ch <= "z") OR (ch >= "A" & ch <= "Z")
    END isLetter;

    PROCEDURE isWhitespace(ch: CHAR): BOOLEAN;
        RETURN ch = " " OR ch = 09X OR ch = 0CX OR ch = 0BX OR
            ch = 0AX OR ch = 0DX
    END isWhitespace;

BEGIN
    token := T.unknown
END Scanner.