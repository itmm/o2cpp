MODULE Hello;

    IMPORT Out;

BEGIN
    IF 3 < 4 THEN
        Out.WriteInt(42)
    ELSE
        Out.WriteInt(-42)
    END;
    Out.WriteLn
END Hello.