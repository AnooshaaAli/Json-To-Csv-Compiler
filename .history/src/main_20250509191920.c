pair: 
    STRING COLON value {
        // Check for duplicate keys in object
        if (isSymbolPresent($1)) {
            yyerror("Duplicate key found in object");
            YYABORT;
        }
        if (!addSymbol($1)) {
            yyerror("Memory allocation error while adding symbol");
            YYABORT;
        }
        $$ = createStrNode("pair", $1);
        addChild($$, $3);
    }
;