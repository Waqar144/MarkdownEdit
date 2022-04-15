#include "highlighter.h"

#include <QDebug>
#include <algorithm>
#include <QTextDocument>


Highliter::Highliter(QTextDocument *doc)
    : QSyntaxHighlighter(doc),
    _language(CodeXML)
{
    initFormats();
    _formats = HighliterTheme::theme(Monokai);
}

void Highliter::initFormats() {
    /****************************************
     * Formats for syntax highlighting
     ***************************************/

    QTextCharFormat format = QTextCharFormat();

    _formats[Token::CodeBlock] = format;
    format = QTextCharFormat();

    format.setForeground(QColor("#F92672"));
    _formats[Token::CodeKeyWord] = format;
    format = QTextCharFormat();

    format.setForeground(QColor("#a39b4e"));
    _formats[Token::CodeString] = format;
    format = QTextCharFormat();

    format.setForeground(QColor("#75715E"));
    _formats[Token::CodeComment] = format;
    format = QTextCharFormat();

    format.setForeground(QColor("#54aebf"));
    _formats[Token::CodeType] = format;

    format = QTextCharFormat();
    format.setForeground(QColor("#db8744"));
    _formats[Token::CodeOther] = format;

    format = QTextCharFormat();
    format.setForeground(QColor("#AE81FF"));
    _formats[Token::CodeNumLiteral] = format;

    format = QTextCharFormat();
    format.setForeground(QColor("#018a0f"));
    _formats[Token::CodeBuiltIn] = format;
}

void Highliter::highlightBlock(const QString &text)
{
    if (currentBlock() == document()->firstBlock()) {
        setCurrentBlockState(_language);
    } else {
        previousBlockState() == _language ?
            setCurrentBlockState(_language) :
            setCurrentBlockState(_language + 1);
    }

    highlightSyntax(text);
}

/**
 * @brief Does the code syntax highlighting
 * @param text
 */
void Highliter::highlightSyntax(const QString &text)
{
    if (text.isEmpty()) return;

    const auto textLen = text.length();

    QChar comment;
    bool isCSS = false;
    bool isYAML = false;
    bool isMake = false;
    bool isAsm = false;
    bool isSQL = false;

    switch (currentBlockState()) {
    case CodeXML :
        xmlHighlighter(text);
        return;
    default:
        break;
    }

    // keep the default code block format
    // this statement is very slow
    // TODO: do this formatting when necessary instead of
    // applying it to the whole block in the beginning
    setFormat(0, textLen, _formats[CodeBlock]);

    const QTextCharFormat &formatComment = _formats[CodeComment];


    for (int i = 0; i < textLen; ++i) {

        if (currentBlockState() % 2 != 0) goto Comment;

        while (i < textLen && !text[i].isLetter()) {
            if (text[i].isSpace()) {
                ++i;
                //make sure we don't cross the bound
                if (i == textLen) return;
                if (text[i].isLetter()) break;
                else continue;
            }
            //inline comment
            if (comment.isNull() && text[i] == QLatin1Char('/')) {
                if((i+1) < textLen){
                    if(text[i+1] == QLatin1Char('/')) {
                        setFormat(i, textLen, formatComment);
                        return;
                    } else if(text[i+1] == QLatin1Char('*')) {
                    Comment:
                        int next = text.indexOf(QLatin1String("*/"));
                        if (next == -1) {
                            //we didn't find a comment end.
                            //Check if we are already in a comment block
                            if (currentBlockState() % 2 == 0)
                                setCurrentBlockState(currentBlockState() + 1);
                            setFormat(i, textLen,  formatComment);
                            return;
                        } else {
                            //we found a comment end
                            //mark this block as code if it was previously comment
                            //first check if the comment ended on the same line
                            //if modulo 2 is not equal to zero, it means we are in a comment
                            //-1 will set this block's state as language
                            if (currentBlockState() % 2 != 0) {
                                setCurrentBlockState(currentBlockState() - 1);
                            }
                            next += 2;
                            setFormat(i, next - i,  formatComment);
                            i = next;
                            if (i >= textLen) return;
                        }
                    }
                }
            } else if (isSQL && comment.isNull() && text[i] == QLatin1Char('-')) {
                if((i+1) < textLen){
                    if(text[i+1] == QLatin1Char('-')) {
                        setFormat(i, textLen, formatComment);
                        return;
                    }
                }
            } else if (text[i] == comment) {
                setFormat(i, textLen, formatComment);
                i = textLen;
                //integer literal
            } else if (text[i].isNumber()) {
                i = highlightNumericLiterals(text, i);
                //string literals
            } else if (text[i] == QLatin1Char('\"')) {
                i = highlightStringLiterals('\"', text, i);
            }  else if (text[i] == QLatin1Char('\'')) {
                i = highlightStringLiterals('\'', text, i);
            }
            if (i >= textLen) {
                break;
            }
            ++i;
        }

        const int pos = i;

        if (i == textLen || !text[i].isLetter()) continue;

        /************************************************
         next letter is usually a space, in that case
         going forward is useless, so continue;
         We can ++i here and go to the beginning of the next word
         so that the next formatter can check for formatting but this will
         cause problems in case the next word is also of 'Type' or the current
         type(keyword/builtin). We can work around it and reset the value of i
         in the beginning of the loop to the word's first letter but I am not
         sure about its efficiency yet.
         ************************************************/
        if (i == textLen || !text[i].isLetter()) continue;

        //we were unable to find any match, lets skip this word
        if (pos == i) {
            int count = i;
            while (count < textLen) {
                if (!text[count].isLetter()) break;
                ++count;
            }
            i = count;
        }
    }

    if (isCSS) cssHighlighter(text);
    if (isYAML) ymlHighlighter(text);
    if (isMake) makeHighlighter(text);
    if (isAsm)  asmHighlighter(text);
}

/**
 * @brief Highlight string literals in code
 * @param strType str type i.e., ' or "
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the string
 */
int Highliter::highlightStringLiterals(const QChar strType, const QString &text, int i) {
    setFormat(i, 1,  _formats[CodeString]);
    ++i;

    while (i < text.length()) {
        //look for string end
        //make sure it's not an escape seq
        if (text.at(i) == strType && text.at(i-1) != QLatin1Char('\\')) {
            setFormat(i, 1,  _formats[CodeString]);
            ++i;
            break;
        }
        //look for escape sequence
        if (text.at(i) == QLatin1Char('\\') && (i+1) < text.length()) {
            int len = 0;
            switch(text.at(i+1).toLatin1()) {
            case 'a':
            case 'b':
            case 'e':
            case 'f':
            case 'n':
            case 'r':
            case 't':
            case 'v':
            case '\'':
            case '"':
            case '\\':
            case '\?':
                //2 because we have to highlight \ as well as the following char
                len = 2;
                break;
            //octal esc sequence \123
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            {
                if (i + 4 <= text.length()) {
                    bool isCurrentOctal = true;
                    if (!isOctal(text.at(i+2).toLatin1())) {
                        isCurrentOctal = false;
                        break;
                    }
                    if (!isOctal(text.at(i+3).toLatin1())) {
                        isCurrentOctal = false;
                        break;
                    }
                    len = isCurrentOctal ? 4 : 0;
                }
                break;
            }
            //hex numbers \xFA
            case 'x':
            {
                if (i + 3 <= text.length()) {
                    bool isCurrentHex = true;
                    if (!isHex(text.at(i+2).toLatin1())) {
                        isCurrentHex = false;
                        break;
                    }
                    if (!isHex(text.at(i+3).toLatin1())) {
                        isCurrentHex = false;
                        break;
                    }
                    len = isCurrentHex ? 4 : 0;
                }
                break;
            }
            //TODO: implement unicode code point escaping
            default:
                break;
            }

            //if len is zero, that means this wasn't an esc seq
            //increment i so that we skip this backslash
            if (len == 0) {
                setFormat(i, 1,  _formats[CodeString]);
                ++i;
                continue;
            }

            setFormat(i, len, _formats[CodeNumLiteral]);
            i += len;
            continue;
        }
        setFormat(i, 1,  _formats[CodeString]);
        ++i;
    }
    return i;
}

/**
 * @brief Highlight number literals in code
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the number
 */
int Highliter::highlightNumericLiterals(const QString &text, int i)
{
    bool isPreAllowed = false;
    if (i == 0) isPreAllowed = true;
    else {
        //these values are allowed before a number
        switch(text.at(i - 1).toLatin1()) {
        //css number
        case ':':
            if (currentBlockState() == CodeCSS)
                isPreAllowed = true;
            break;
        case '$':
            if (currentBlockState() == CodeAsm)
                isPreAllowed = true;
            break;
        case '[':
        case '(':
        case '{':
        case ' ':
        case ',':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '<':
        case '>':
            isPreAllowed = true;
            break;
        }
    }

    if (!isPreAllowed) return i;

    const int start = i;

    if ((i+1) >= text.length()) {
        setFormat(i, 1, _formats[CodeNumLiteral]);
        return ++i;
    }

    ++i;
    //hex numbers highlighting (only if there's a preceding zero)
    if (text.at(i) == QChar('x') && text.at(i - 1) == QChar('0'))
        ++i;

    while (i < text.length()) {
        if (!text.at(i).isNumber() && text.at(i) != QChar('.') &&
            text.at(i) != QChar('e')) //exponent
            break;
        ++i;
    }

    bool isPostAllowed = false;
    if (i == text.length()) {
        //cant have e at the end
        if (text.at(i - 1) != QChar('e'))
            isPostAllowed = true;
    } else {
        //these values are allowed after a number
        switch(text.at(i).toLatin1()) {
        case ']':
        case ')':
        case '}':
        case ' ':
        case ',':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '>':
        case '<':
        case ';':
            isPostAllowed = true;
            break;
        // for 100u, 1.0F
        case 'p':
            if (currentBlockState() == CodeCSS)
                if (i + 1 < text.length() && text.at(i+1) == QChar('x')) {
                    if (i + 2 == text.length() || !text.at(i+2).isLetterOrNumber())
                        isPostAllowed = true;
                }
            break;
        case 'e':
            if (currentBlockState() == CodeCSS)
                if (i + 1 < text.length() && text.at(i+1) == QChar('m')) {
                    if (i + 2 == text.length() || !text.at(i+2).isLetterOrNumber())
                        isPostAllowed = true;
                }
            break;
        case 'u':
        case 'l':
        case 'f':
        case 'U':
        case 'L':
        case 'F':
            if (i + 1 == text.length() || !text.at(i+1).isLetterOrNumber()) {
                isPostAllowed = true;
                ++i;
            }
            break;
        }
    }
    if (isPostAllowed) {
        int end = i;
        setFormat(start, end - start, _formats[CodeNumLiteral]);
    }
    //decrement so that the index is at the last number, not after it
    return --i;
}

/**
 * @brief The YAML highlighter
 * @param text
 * @details This function post processes a line after the main syntax
 * highlighter has run for additional highlighting. It does these things
 *
 * If the current line is a comment, skip it
 *
 * Highlight all the words that have a colon after them as 'keyword' except:
 * If the word is a string, skip it.
 * If the colon is in between a path, skip it (C:\)
 *
 * Once the colon is found, the function will skip every character except 'h'
 *
 * If an h letter is found, check the next 4/5 letters for http/https and
 * highlight them as a link (underlined)
 */
void Highliter::ymlHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();
    bool colonNotFound = false;

    //if this is a comment don't do anything and just return
    if (text.trimmed().at(0) == QLatin1Char('#'))
        return;

    for (int i = 0; i < textLen; ++i) {
        if (!text.at(i).isLetter()) continue;

        if (colonNotFound && text.at(i) != QLatin1Char('h')) continue;

        //we found a string literal, skip it
        if (i != 0 && (text.at(i-1) == QLatin1Char('"') || text.at(i-1) == QLatin1Char('\''))) {
            const int next = text.indexOf(text.at(i-1), i);
            if (next == -1) break;
            i = next;
            continue;
        }

        const int colon = text.indexOf(QLatin1Char(':'), i);

        //if colon isn't found, we set this true
        if (colon == -1) colonNotFound = true;

        if (!colonNotFound) {
            //if the line ends here, format and return
            if (colon+1 == textLen) {
                setFormat(i, colon - i, _formats[CodeKeyWord]);
                return;
            } else {
                //colon is found, check if it isn't some path or something else
                if (!(text.at(colon+1) == QLatin1Char('\\') && text.at(colon+1) == QLatin1Char('/'))) {
                    setFormat(i, colon - i, _formats[CodeKeyWord]);
                }
            }
        }

        //underlined links
        if (text.at(i) == QLatin1Char('h')) {
            if (strMidRef(text, i, 5) == QLatin1String("https") ||
                strMidRef(text, i, 4) == QLatin1String("http")) {
                int space = text.indexOf(QChar(' '), i);
                if (space == -1) space = textLen;
                QTextCharFormat f = _formats[CodeString];
                f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                setFormat(i, space - i, f);
                i = space;
            }
        }
    }
}

void Highliter::cssHighlighter(const QString &text)
{
    if (text.isEmpty()) return;
    const auto textLen = text.length();
    for (int i = 0; i<textLen; ++i) {
        if (text[i] == QLatin1Char('.') || text[i] == QLatin1Char('#')) {
            if (i+1 >= textLen) return;
            if (text[i + 1].isSpace() || text[i+1].isNumber()) continue;
            int space = text.indexOf(QLatin1Char(' '), i);
            if (space < 0) {
                space = text.indexOf('{');
                if (space < 0) {
                    space = textLen;
                }
            }
            setFormat(i, space - i, _formats[CodeKeyWord]);
            i = space;
        } else if (text[i] == QLatin1Char('c')) {
            if (strMidRef(text, i, 5) == QLatin1String("color")) {
                i += 5;
                int colon = text.indexOf(QLatin1Char(':'), i);
                if (colon < 0) continue;
                i = colon;
                i++;
                while(i < textLen) {
                    if (!text[i].isSpace()) break;
                    i++;
                }
                int semicolon = text.indexOf(QLatin1Char(';'));
                if (semicolon < 0) semicolon = textLen;
                const QString color = text.mid(i, semicolon-i);
                QTextCharFormat f = _formats[CodeBlock];
                QColor c(color);
                if (color.startsWith(QLatin1String("rgb"))) {
                    int t = text.indexOf(QLatin1Char('('), i);
                    int rPos = text.indexOf(QLatin1Char(','), t);
                    int gPos = text.indexOf(QLatin1Char(','), rPos+1);
                    int bPos = text.indexOf(QLatin1Char(')'), gPos);
                    if (rPos > -1 && gPos > -1 && bPos > -1) {
                        const auto r = strMidRef(text, t+1, rPos - (t+1));
                        const auto g = strMidRef(text, rPos+1, gPos - (rPos + 1));
                        const auto b = strMidRef(text, gPos+1, bPos - (gPos+1));
                        c.setRgb(r.toInt(), g.toInt(), b.toInt());
                    } else {
                        c = _formats[CodeBlock].background().color();
                    }
                }

                if (!c.isValid()) {
                    continue;
                }

                int lightness{};
                QColor foreground;
                //really dark
                if (c.lightness() <= 20) {
                    foreground = Qt::white;
                } else if (c.lightness() > 20 && c.lightness() <= 51){
                    foreground = QColor("#ccc");
                } else if (c.lightness() > 51 && c.lightness() <= 78){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 78 && c.lightness() <= 110){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 127) {
                    lightness = c.lightness() + 100;
                    foreground = c.darker(lightness);
                }
                else {
                    lightness = c.lightness() + 100;
                    foreground = c.lighter(lightness);
                }

                f.setBackground(c);
                f.setForeground(foreground);
                setFormat(i, semicolon - i, QTextCharFormat()); //clear prev format
                setFormat(i, semicolon - i, f);
                i = semicolon;
            }
        }
    }
}


void Highliter::xmlHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();

    setFormat(0, textLen, _formats[CodeBlock]);

    for (int i = 0; i < textLen; ++i) {
        if (text[i] == QLatin1Char('<') && text[i+1] != QLatin1Char('!')) {
            const int found = text.indexOf(QLatin1Char('>'), i);
            if (found > 0) {
                ++i;
                if (text[i] == QLatin1Char('/')) ++i;
                setFormat(i, found - i, _formats[CodeKeyWord]);
            }
        }

        if (text[i] == QLatin1Char('=')) {
            int lastSpace = text.lastIndexOf(QLatin1Char(' '), i);
            if (lastSpace == i-1) lastSpace = text.lastIndexOf(QLatin1Char(' '), i-2);
            if (lastSpace > 0) {
                setFormat(lastSpace, i - lastSpace, _formats[CodeBuiltIn]);
            }
        }

        if (text[i] == QLatin1Char('\"')) {
            const int pos = i;
            int cnt = 1;
            ++i;
            //bound check
            if ( (i+1) >= textLen) return;
            while (i < textLen) {
                if (text[i] == QLatin1Char('\"')) {
                    ++cnt;
                    ++i;
                    break;
                }
                ++i; ++cnt;
                //bound check
                if ( (i+1) >= textLen) {
                    ++cnt;
                    break;
                }
            }
            setFormat(pos, cnt, _formats[CodeString]);
        }
    }
}

void Highliter::makeHighlighter(const QString &text)
{
    int colonPos = text.indexOf(QLatin1Char(':'));
    if (colonPos == -1)
        return;
    setFormat(0, colonPos, _formats[Token::CodeBuiltIn]);
}

/**
 * @brief highlight inline labels such as 'func()' in "call func()"
 * @param text
 */
void Highliter::highlightInlineAsmLabels(const QString &text)
{
#define Q(s) QStringLiteral(s)
    static const QString jumps[27] = {
        //0 - 19
        Q("jmp"), Q("je"), Q("jne"), Q("jz"), Q("jnz"), Q("ja"), Q("jb"), Q("jg"), Q("jge"), Q("jae"), Q("jl"), Q("jle"),
        Q("jbe"), Q("jo"), Q("jno"), Q("js"), Q("jns"), Q("jcxz"), Q("jecxz"), Q("jrcxz"),
        //20 - 24
        Q("loop"), Q("loope"), Q("loopne"), Q("loopz"), Q("loopnz"),
        //25 - 26
        Q("call"), Q("callq")
    };
#undef Q

    auto format = _formats[Token::CodeBuiltIn];
    format.setFontUnderline(true);

    const QString trimmed = text.trimmed();
    int start = -1;
    int end = -1;
    char c{};
    if (!trimmed.isEmpty())
        c = trimmed.at(0).toLatin1();
    if (c == 'j') {
        start = 0; end = 20;
    } else if (c == 'c') {
        start = 25; end = 27;
    } else if (c == 'l') {
        start = 20; end = 25;
    } else {
        return;
    }

    auto skipSpaces = [&text](int& j){
        while (text.at(j).isSpace()) j++;
        return j;
    };

    for (int i = start; i < end; ++i) {
        if (trimmed.startsWith(jumps[i])) {
            int j = 0;
            skipSpaces(j);
            j = j + jumps[i].length() + 1;
            skipSpaces(j);
            int len = text.length() - j;
            setFormat(j, len, format);
        }
    }
}

void Highliter::asmHighlighter(const QString& text)
{
    highlightInlineAsmLabels(text);
    //label highlighting
    //examples:
    //L1:
    //LFB1:           # local func begin
    //
    //following e.gs are not a label
    //mov %eax, Count::count(%rip)
    //.string ": #%s"

    //look for the last occurence of a colon
    int colonPos = text.lastIndexOf(QLatin1Char(':'));
    if (colonPos == -1)
        return;
    //check if this colon is in a comment maybe?
    bool isComment = text.lastIndexOf('#', colonPos) != -1;
    if (isComment) {
        int commentPos = text.lastIndexOf('#', colonPos);
        colonPos = text.lastIndexOf(':', commentPos);
    }

    auto format = _formats[Token::CodeBuiltIn];
    format.setFontUnderline(true);

    if (colonPos >= text.length() - 1) {
        setFormat(0, colonPos, format);
    }

    int i = 0;
    bool isLabel = true;
    for (i = colonPos + 1; i < text.length(); ++i) {
        if (!text.at(i).isSpace()) {
            isLabel = false;
            break;
        }
    }

    if (!isLabel && i < text.length() && text.at(i) == QLatin1Char('#'))
        setFormat(0, colonPos, format);
}

static QHash<Highliter::Token, QTextCharFormat> formats()
{
    QHash<Highliter::Token, QTextCharFormat> _formats;

    QTextCharFormat defaultFormat = QTextCharFormat();

    _formats[Highliter::Token::CodeBlock] = defaultFormat;
    _formats[Highliter::Token::CodeKeyWord] = defaultFormat;
    _formats[Highliter::Token::CodeString] = defaultFormat;
    _formats[Highliter::Token::CodeComment] = defaultFormat;
    _formats[Highliter::Token::CodeType] = defaultFormat;
    _formats[Highliter::Token::CodeOther] = defaultFormat;
    _formats[Highliter::Token::CodeNumLiteral] = defaultFormat;
    _formats[Highliter::Token::CodeBuiltIn] = defaultFormat;

    return _formats;
}

static QHash<Highliter::Token, QTextCharFormat> monokai()
{
    QHash<Highliter::Token, QTextCharFormat> _formats = formats();

    _formats[Highliter::Token::CodeBlock].setForeground(QColor(227, 226, 214));
    _formats[Highliter::Token::CodeKeyWord].setForeground(QColor(249, 38, 114));
    _formats[Highliter::Token::CodeString].setForeground(QColor(230, 219, 116));
    _formats[Highliter::Token::CodeComment].setForeground(QColor(117, 113, 94));
    _formats[Highliter::Token::CodeType].setForeground(QColor(102, 217, 239));
    _formats[Highliter::Token::CodeOther].setForeground(QColor(249, 38, 114));
    _formats[Highliter::Token::CodeNumLiteral].setForeground(QColor(174, 129, 255));
    _formats[Highliter::Token::CodeBuiltIn].setForeground(QColor(166, 226, 46));

    return _formats;
}

QHash<Highliter::Token, QTextCharFormat>
HighliterTheme::theme(Highliter::Themes theme) {
    switch (theme) {
    case Highliter::Themes::Monokai:
        return monokai();
    default:
        return {};
    }
}
