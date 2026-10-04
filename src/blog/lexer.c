#include <blog/lexer.h>
#include <blog/error.h>

// the trick explained in more detail here:
// https://github.com/Maqi-x/cfw/blob/main/src/ft.c
#undef isspace
#define isspace(c) \
    isspace((uchar)c)

static inline bool is_at_end(const BlLexer* lexer) {
    return lexer->pos >= lexer->input.len;
}

static inline char peek(const BlLexer* lexer) {
    if (lexer->pos + 0 >= lexer->input.len) return '\0';
    return lexer->input.data[lexer->pos + 0];
}
static inline char peek2(const BlLexer* lexer) {
    if (lexer->pos + 1 >= lexer->input.len) return '\0';
    return lexer->input.data[lexer->pos + 1];
}
static inline char peek3(const BlLexer* lexer) {
    if (lexer->pos + 2 >= lexer->input.len) return '\0';
    return lexer->input.data[lexer->pos + 2];
}

static inline bool check2(const BlLexer* lexer, char c1, char c2) {
    return peek(lexer) == c1 && peek2(lexer) == c2;
}
static inline bool check3(const BlLexer* lexer, char c1, char c2, char c3) {
    return peek(lexer) == c1 && peek2(lexer) == c2 && peek3(lexer) == c3;
}

static inline char advance(BlLexer* lexer) {
    char c = lexer->input.data[lexer->pos++];

    // TODO: maybe we should support CRLF.
    //       the project is currently posix only anyway
    //       so maybe it's not worth wasting time.
    if (c == '\n') {
        lexer->line++;
        lexer->col = 0;
    } else {
        lexer->col++;
    }

    return c; //return c;
}

static void skip_spaces(BlLexer* lexer) {
    while (!is_at_end(lexer)) {
        if (isspace(peek(lexer))) advance(lexer);
        else break;
    }
}

static bool is_blank_line(const BlLexer* lexer) {
    uint i = lexer->pos;
    while (i < lexer->input.len && lexer->input.data[i] != '\n') {
        if (!isspace(lexer->input.data[i++])) return false;
    }
    return true;
}

static StringView take_line(BlLexer* lexer) {
    uint start = lexer->pos;
    while (!is_at_end(lexer) && peek(lexer) != '\n')
        advance(lexer);

    return sv_slice(lexer->input, start, lexer->pos);
}

static BlToken make_token_ex(
    const BlLexer* lexer, BlTokenType type, usize unescaped_len,
    uint start, uint line, uint col
) {
    return (BlToken) {
        .type   = type,
        .lexeme = sv_slice(lexer->input, start, lexer->pos),
        .unescaped_len = unescaped_len,
        .line   = line,
        .col    = col,
    };
}

static BlToken make_token(const BlLexer* lexer, BlTokenType type, uint start) {
    return make_token_ex(lexer, type, lexer->pos - start, start, lexer->line, lexer->col);
}

// the helpers.
static BlToken
    lex_attr(BlLexer* lexer),
    lex_text(BlLexer* lexer),
    lex_code(BlLexer* lexer, BlTokenType type, const char* name),
    lex_line_token(BlLexer* lexer, BlTokenType type, uint skip);

void bl_lexer_init(BlLexer* lexer, StringView input) {
    //memset(lexer, 0, sizeof(BlLexer));
    *lexer = (BlLexer) { 0 };

    lexer->input = input;
    lexer->header_done = false;
}

BlToken bl_lexer_next(BlLexer* lexer) {
    if (is_at_end(lexer)) {
        goto eof;
    }

    // we actually want to imitate these semi-weird markdown line breaks rules
    // a single \n is a soft break or whatever is it called. we only insert a
    // hard break token on \n, blank line and then another \n
    if (peek(lexer) == '\n') {
        uint line = lexer->line;
        advance(lexer); // '\n'

        bool is_an_actual_break = false;
        while (!is_at_end(lexer) && is_blank_line(lexer)) {
            while (!is_at_end(lexer) && peek(lexer) != '\n')
                advance(lexer);

            if (!is_at_end(lexer)) advance(lexer);
            is_an_actual_break = true;
        }

        if (is_an_actual_break) {
            lexer->header_done = true;
            return (BlToken) {
                .type = BL_TT_HARDBREAK, .line = line,
            };
        }

        if (is_at_end(lexer)) goto eof;

        return (BlToken) {
            .type = BL_TT_SOFTBREAK, .line = line,
        };
    }

    uint start = lexer->pos;

    // :: comment
    if (check2(lexer, ':', ':'))
        return lex_line_token(lexer, BL_TT_COMMENT, 2);

    // :attr:
    if (peek(lexer) == ':' && !lexer->header_done)
        return lex_attr(lexer);

    // == title
    if (check2(lexer, '=', '='))
        return lex_line_token(lexer, BL_TT_H1, 2);

    // --- subtitle
    if (check3(lexer, '-', '-', '-'))
        return lex_line_token(lexer, BL_TT_H2, 3);

    // ```code```
    if (check3(lexer, '`', '`', '`'))
        return lex_code(lexer, BL_TT_CODE_BLOCK, "");

    // `code`
    if (peek(lexer) == '`')
        return lex_code(lexer, BL_TT_CODE_INLINE, "inline ");

    // *bold*
    if (peek(lexer) == '*')
        return advance(lexer), make_token(lexer, BL_TT_BOLD, start);

    // /italic/
    if (peek(lexer) == '/')
        return advance(lexer), make_token(lexer, BL_TT_ITALIC, start);

    return lex_text(lexer);

eof:
    return (BlToken) {
        .type = BL_TT_EOF, .lexeme = SV_NULL, .line = lexer->line,
    };
}

static BlToken lex_attr(BlLexer* lexer) {
    uint line = lexer->line;
    uint col = lexer->col;
    advance(lexer); // ':'

    uint name_start = lexer->pos;
    while (!is_at_end(lexer) && peek(lexer) != ':' &&
           peek(lexer) != '\n')
        advance(lexer);

    if (is_at_end(lexer) || peek(lexer) != ':')
        bl_error("%u:%u: unterminated attribute name", line, col);

    StringView name = sv_slice(lexer->input, name_start, lexer->pos);
    advance(lexer); // ':'

    return (BlToken) {
        .type = BL_TT_ATTR, .lexeme = name, .line = line,
    };
}

// for headers and comments (and maybe quotes/lists in the future)
static BlToken lex_line_token(BlLexer* lexer, BlTokenType type, uint skip) {
    lexer->header_done = true;
    uint line = lexer->line;

    // TODO: maybe add some helper like advance_by(lexer, n)
    while (skip--) advance(lexer);
    skip_spaces(lexer);

    StringView text = take_line(lexer);

    return (BlToken) {
        .type = type,
        .lexeme = text,
        .line = line,
    };
}

static BlToken lex_code(
    BlLexer* lexer, BlTokenType type, const char* name
) {
    uint line = lexer->line;
    uint col = lexer->col;

    bool block = type == BL_TT_CODE_BLOCK;
    if (block) advance(lexer), advance(lexer), advance(lexer);
    else advance(lexer);

    uint content_start = lexer->pos;
    while (!is_at_end(lexer)) {
        bool terminates = block
            ? check3(lexer, '`', '`', '`')
            : peek(lexer) == '`';

        if (terminates) {
            StringView content = sv_slice(lexer->input, content_start, lexer->pos);

            if (block) advance(lexer), advance(lexer), advance(lexer);
            else advance(lexer);

            return (BlToken) {
                .type   = type,
                .lexeme = content,
                .line   = line,
                .col    = col,
            };
        }
        advance(lexer);
    }

    bl_error("%u:%u: unterminated %scode block", line, col, name);
}

static BlToken lex_text(BlLexer* lexer) {
    uint line  = lexer->line;
    uint col   = lexer->col;
    uint start = lexer->pos;

    usize unescaped_len = 0;
    while (!is_at_end(lexer)) {
        if (peek(lexer) == '\\') {
            advance(lexer);
            if (is_at_end(lexer)) {
                bl_error("%u:%u: expected a character after \\", lexer->line, lexer->col);
            }

            advance(lexer);
            unescaped_len++;
            continue;
        }

        bool is_special =
            peek(lexer) == '\n'
         || peek(lexer) == '*'
         || peek(lexer) == '/'
         || peek(lexer) == '`';

        if (is_special) break;
        advance(lexer);
        unescaped_len++;
    }
    return make_token_ex(lexer, BL_TT_TEXT, unescaped_len, start, line, col);
}
