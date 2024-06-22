MODULE Scanner;

    IMPORT T := Token;

    VAR
        token: INTEGER;

    PROCEDURE isDigit(ch: CHAR): BOOLEAN;
        RETURN ch >= "0" & ch <= "9"
    END isDigit;

BEGIN
    token := T.unknown
END Scanner.