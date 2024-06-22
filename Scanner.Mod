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

BEGIN
    token := T.unknown
END Scanner.