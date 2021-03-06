//==============================================================================
namespace ACCCodeTokeniserHelpers
{
    static const char* const keywords2Char[] =
    {
        "do", "if",
        nullptr
    };

    static const char* const keywords3Char[] =
    {
        "for", "int", "log", "net", "not", "str",
        nullptr
    };

    static const char* const keywords4Char[] =
    {
        "bool", "case", "else", "goto", "void",
        "OPEN",
        nullptr
    };

    static const char* const keywords5Char[] =
    {
        "break", "const", "DEATH", "ENTER", "event",
        "print", "until", "while", "world",
        nullptr
    };

    static const char* const keywords6Char[] =
    {
        "define", "global", "import", "PICKUP", "return",
        "RETURN", "script", "static", "strcpy", "switch",
        nullptr
    };

    static const char* const keywords7Char[] =
    {
        "default", "include", "library", "RESPAWN",
        "restart", "special", "suspend",
        nullptr
    };

    static const char* const keywords8Char[] =
    {
        "continue", "function", "strparam",
        nullptr
    };

    static const char* const keywords9Char[] =
    {
        "libdefine", "LIGHTNING", "nocompact", "printbold",
        "REDRETURN", "terminate", "UNLOADING", "wadauthor",
        nullptr
    };

    static const char* const keywords10Char[] =
    {
        "BLUERETURN", "clientside", "DISCONNECT", "hudmessage",
        nullptr
    };

    static const char* const keywordsOther[] =
    {
        "nowadauthor", "WHITERETURN", "encryptstrings",
        "hudmessagebold", "acs_executewait", "createtranslation",
        "acs_namedexecutewait",
        nullptr
    };
}

//==============================================================================
ACCCodeTokeniser::ACCCodeTokeniser()
{
}

//==============================================================================
CodeEditorComponent::ColourScheme ACCCodeTokeniser::getDefaultColourScheme()
{
    const CodeEditorComponent::ColourScheme::TokenType tokenTypes[] =
    {
        { "Error",              Colour (163, 21, 21) },
        { "Comment",            Colours::green },
        { "Keyword",            Colours::blue },
        { "Operator",           Colours::black },
        { "Identifier",         Colours::black },
        { "Integer",            Colours::black },
        { "Float",              Colours::black },
        { "String",             Colour (163, 21, 21) },
        { "Bracket",            Colours::black },
        { "Punctuation",        Colours::black },
        { "Preprocessor Text",  Colours::black }
    };

    Array<CodeEditorComponent::ColourScheme::TokenType> types (tokenTypes, numElementsInArray (tokenTypes));

    CodeEditorComponent::ColourScheme colourScheme;
    colourScheme.types.swapWith (types);

    return colourScheme;
}

bool ACCCodeTokeniser::isReservedKeyword (const String& token) noexcept
{
    using namespace ACCCodeTokeniserHelpers;

    const char* const* k = nullptr;

    const int tokenLength = token.length();
    switch (tokenLength)
    {
        case 2:     k = keywords2Char; break;
        case 3:     k = keywords3Char; break;
        case 4:     k = keywords4Char; break;
        case 5:     k = keywords5Char; break;
        case 6:     k = keywords6Char; break;
        case 7:     k = keywords7Char; break;
        case 8:     k = keywords8Char; break;
        case 9:     k = keywords9Char; break;
        case 10:    k = keywords10Char; break;

        default:
        {
            if (tokenLength < shortestKeywordLength
                || tokenLength > longestKeywordLength)
            {
                return false;
            }

            k = keywordsOther;
        }
        break;
    }

    for (int i = 0; k[i] != 0; ++i)
        if (token == k[i])
            return true;

    return false;
}

bool ACCCodeTokeniser::isIdentifierStart (juce_wchar c) noexcept
{
    return CharacterFunctions::isLetter (c);
}

bool ACCCodeTokeniser::isIdentifierBody (juce_wchar c) noexcept
{
    return CharacterFunctions::isLetterOrDigit (c);
}

ACCCodeTokeniser::TokenType ACCCodeTokeniser::parseIdentifier (CodeDocument::Iterator& source)
{
    int tokenLength = 0;
    String::CharPointerType::CharType possibleIdentifier[100];
    String::CharPointerType possible (possibleIdentifier);

    while (isIdentifierBody (source.peekNextChar()))
    {
        const juce_wchar c = source.nextChar();

        if (tokenLength < longestKeywordLength)
            possible.write (c);

        ++tokenLength;
    }

    if (tokenLength >= shortestKeywordLength && tokenLength <= longestKeywordLength)
    {
        possible.writeNull();

        if (isReservedKeyword (possibleIdentifier))
            return keywordTokenType;
    }

    return identifierTokenType;
}

ACCCodeTokeniser::TokenType ACCCodeTokeniser::parseNumber (CodeDocument::Iterator& source)
{
    const CodeDocument::Iterator original (source);

    if (CppTokeniserFunctions::parseFloatLiteral (source))
        return floatTokenType;

    source = original;

    if (CppTokeniserFunctions::parseHexLiteral (source))
        return integerTokenType;

    source = original;

    if (CppTokeniserFunctions::parseOctalLiteral (source)) 
        return integerTokenType;

    source = original;

    if (CppTokeniserFunctions::parseDecimalLiteral (source))
        return integerTokenType;

    source = original;

    return errorTokenType;
}

int ACCCodeTokeniser::readNextToken (CodeDocument::Iterator& source)
{
    source.skipWhitespace();

    const juce_wchar firstChar = source.peekNextChar();

    switch (firstChar)
    {
        case 0:
        break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
        {
            const int result = parseNumber (source);

            if (result == errorTokenType)
            {
                source.skip();

                if (firstChar == '.')
                    return punctuationTokenType;
            }

            return result;
        }

        case ',':
        case ';':
        case ':':
            source.skip();
            return punctuationTokenType;

        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
            source.skip();
            return bracketTokenType;

        case '"':
        case '\'':
            CppTokeniserFunctions::skipQuotedString (source);
            return stringTokenType;

        case '+':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, '+', '=');
            return operatorTokenType;

        case '-':
        {
            source.skip();
            const int result = parseNumber (source);

            if (result == errorTokenType)
            {
                CppTokeniserFunctions::skipIfNextCharMatches (source, '-', '=');
                return operatorTokenType;
            }

            return result;
        }

        case '*':
        case '%':
        case '=':
        case '!':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, '=');
            return operatorTokenType;

        case '/':
        {
            source.skip();
            const juce_wchar nextChar = source.peekNextChar();

            if (nextChar == '/')
            {
                source.skipToEndOfLine();
                return commentTokenType;
            }

            if (nextChar == '*')
            {
                source.skip();
                CppTokeniserFunctions::skipComment (source);
                return commentTokenType;
            }

            if (nextChar == '=')
                source.skip();

            return operatorTokenType;
        }

        case '?':
        case '~':
            source.skip();
            return operatorTokenType;

        case '<':
        case '>':
        case '|':
        case '&':
        case '^':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, firstChar);
            CppTokeniserFunctions::skipIfNextCharMatches (source, '=');
            return operatorTokenType;

        case '#':
            CppTokeniserFunctions::skipPreprocessorLine (source);
            return preprocessorTokenType;

        default:
            if (isIdentifierStart (firstChar))
                return parseIdentifier (source);

            source.skip();
        break;
    }

    return errorTokenType;
}