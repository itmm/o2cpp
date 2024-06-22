MODULE Hello;

    IMPORT Out;

    PROCEDURE isDigit(ch: CHAR): BOOLEAN;
        RETURN ch >= "0" & ch <= "9"
    END isDigit;

BEGIN
    IF 3 < 4 THEN
        Out.WriteInt(42)
    ELSE
        Out.WriteInt(-42)
    END;
    Out.WriteLn
END Hello.