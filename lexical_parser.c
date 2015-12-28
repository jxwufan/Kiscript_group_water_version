//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#include "lexical_parser.h"

#include <string.h>

gboolean text_is_match(gchar *input, gchar *text) {
    return g_strcmp0(input, text) == 0;
}

gboolean char_is_first(gchar *input, gchar char0) {
    return input[0] == char0;
}

void consume_char(gchar **input_p) {
    // Always use g_utf8_next_char for safety.
    *input_p = g_utf8_next_char(*input_p);
}

gboolean match_char(gchar **input_p, gchar char0) {
    if (char_is_first(*input_p, char0)) {
        consume_char(input_p);
        return TRUE;
    } else {
        return FALSE;
    }
}

gboolean match_not_char(gchar **input_p, gchar char0) {
    if (!char_is_first(*input_p, char0)) {
        consume_char(input_p);
        return TRUE;
    } else {
        return FALSE;
    }
}

gboolean match_not_char2(gchar **input_p, gchar char0, gchar char1) {
    if (!char_is_first(*input_p, char0) && !char_is_first(*input_p, char1)) {
        consume_char(input_p);
        return TRUE;
    } else {
        return FALSE;
    }
}

gboolean match_chars(gchar **input_p, gchar *chars) {
    for (gchar *char_p = chars; *char_p; ++char_p) {
        if (match_char(input_p, *char_p)) {
            return TRUE;
        }
    }
    return FALSE;
}

gboolean text_is_first(gchar *input, gchar *text) {
    return g_str_has_prefix(input, text);
}

void consume_text(gchar **input_p, gchar *text) {
    *input_p += strlen(text);
}

gboolean match_text(gchar **input_p, gchar *text) {
    if (text_is_first(*input_p, text)) {
        consume_text(input_p, text);
        return TRUE;
    } else {
        return FALSE;
    }
}

#define try_match_and_return_token(input_p, token_id, match) \
    { \
        char *input_old = *(input_p); \
        if ((match)) { \
            return token_new_strndup_no_data((token_id), input_old, \
                                             *(input_p)); \
        } \
    }

#define try_match_char_and_return_token(input_p, token_id, token_char) \
    try_match_and_return_token((input_p), (token_id), \
                               match_char((input_p), (token_char)))

#define try_match_text_and_return_token(input_p, token_id, token_text) \
    try_match_and_return_token((input_p), (token_id), \
                               match_text((input_p), (token_text)))


/*
 * InputElementDiv ::
 *     WhiteSpace
 *     LineTerminator
 *     Comment
 *     Token
 *     DivPunctuator
 */
token_t *input_element_div(gchar **input_p) {
    white_space(input_p);
    line_terminator(input_p);
    comment(input_p);
    token(input_p);
    div_punctuator(input_p);
}

static gboolean match_usp(gchar **input_p) {
    if (g_unichar_isspace(g_utf8_get_char(*input_p))) {
        *input_p = g_utf8_next_char(*input_p);
        return TRUE;
    } else {
        return FALSE;
    }
}

// TODO
#define CHARACTER_TAB_CHAR
#define CHARACTER_VT_CHAR
#define CHARACTER_FF_CHAR
#define CHARACTER_SP_CHAR
#define CHARACTER_NBSP_CHAR
#define CHARACTER_BOM_CHAR

/*
 * WhiteSpace ::
 *     <TAB>
 *     <VT>
 *     <FF>
 *     <SP>
 *     <NBSP>
 *     <BOM>
 *     <USP>
 */
token_t *white_space(gchar **input_p) {

    // <TAB>
    try_match_char_and_return_token(input_p, TOKEN_LEXICAL_WHITE_SPACE, '\u0009')
    // <VT>
    try_match_char_and_return_token(input_p, TOKEN_LEXICAL_WHITE_SPACE, '\u000B')
    // <FF>
    try_match_char_and_return_token(input_p, TOKEN_LEXICAL_WHITE_SPACE, '\u000C')
    // <SP>
    try_match_char_and_return_token(input_p, TOKEN_LEXICAL_WHITE_SPACE, '\u0020')
    // <NBSP>
    try_match_char_and_return_token(input_p, TOKEN_LEXICAL_WHITE_SPACE, '\u00A0')
    // <BOM>
    try_match_text_and_return_token(input_p, TOKEN_LEXICAL_WHITE_SPACE, "\uFEFF")
    // <USP>
    try_match_and_return_token(input_p, TOKEN_LEXICAL_WHITE_SPACE, match_usp(input_p))

    // TODO: Error!
    return NULL;
}

#define CHARACTER_LF_CHAR '\u000A'
#define CHARACTER_CR_CHAR '\u000D'
#define CHARACTER_LS_TEXT "\u2028"
#define CHARACTER_PS_TEXT "\u2029"
#define TEXT_CR_LF "\u000D\u000A"

gboolean line_terminator_is_first(gchar *input) {
    return char_is_first(input, CHARACTER_LF_CHAR)
           || char_is_first(input, CHARACTER_CR_CHAR)
           || text_is_first(input, CHARACTER_LS_TEXT)
           || text_is_first(input, CHARACTER_PS_TEXT);
}

/*
 * LineTerminator ::
 *     <LF>
 *     <CR>
 *     <LS>
 *     <PS>
 */
token_t *line_terminator(gchar **input_p) {

    // <LF>
    try_match_char_and_return_token(input_p, TOKEN_LEXICAL_LINE_TERMINATOR,
                                    CHARACTER_LF_CHAR)
    // <CR>
    try_match_char_and_return_token(input_p, TOKEN_LEXICAL_LINE_TERMINATOR,
                                    CHARACTER_CR_CHAR)
    // <LS>
    try_match_text_and_return_token(input_p, TOKEN_LEXICAL_LINE_TERMINATOR,
                                    CHARACTER_LS_TEXT)
    // <PS>
    try_match_text_and_return_token(input_p, TOKEN_LEXICAL_LINE_TERMINATOR,
                                    CHARACTER_PS_TEXT)

    // TODO: Error!
    return NULL;
}

/*
 * LineTerminatorSequence ::
 *     <LF>
 *     <CR> [lookahead ∉ <LF> ]
 *     <LS>
 *     <PS>
 *     <CR> <LF>
 */
token_t *line_terminator_sequence(gchar **input_p) {

    // <LF>
    try_match_char_and_return_token(input_p,
                                    TOKEN_LEXICAL_LINE_TERMINATOR_SEQUENCE,
                                    CHARACTER_LF_CHAR)
    // <CR> <LF>
    // NOTE: Promoted over <CR> for the lookahead of <CR>.
    try_match_text_and_return_token(input_p,
                                    TOKEN_LEXICAL_LINE_TERMINATOR_SEQUENCE,
                                    TEXT_CR_LF)
    // <CR>
    try_match_char_and_return_token(input_p,
                                    TOKEN_LEXICAL_LINE_TERMINATOR_SEQUENCE,
                                    CHARACTER_CR_CHAR)
    // <LS>
    try_match_text_and_return_token(input_p,
                                    TOKEN_LEXICAL_LINE_TERMINATOR_SEQUENCE,
                                    CHARACTER_LS_TEXT)
    // <PS>
    try_match_text_and_return_token(input_p,
                                    TOKEN_LEXICAL_LINE_TERMINATOR_SEQUENCE,
                                    CHARACTER_PS_TEXT)

    // TODO: Error!
    return NULL;
}

gboolean comment_is_first(gchar *input) {
    return multi_line_comment_is_first(input)
           || single_line_comment_is_first(input);
}

/*
 * Comment ::
 *     MultiLineComment
 *     SingleLineComment
 */
token_t *comment(gchar **input_p) {

    // MultiLineComment
    if (multi_line_comment_is_first(*input_p)) {
        // TODO: Wrap as children!
        return multi_line_comment(input_p);
    }
    // SingleLineComment
    if (single_line_comment_is_first(*input_p)) {
        // TODO: Wrap as children!
        return single_line_comment(input_p);
    }

    // TODO: Error!
    return NULL;
}

gboolean multi_line_comment_is_first(gchar *input) {
    return g_str_has_prefix(input, "/*");
}

static gboolean match_post_asterisk_comment_chars(gchar **input_p);

static gboolean match_multi_line_comment_chars(gchar **input_p) {
    if (match_not_char(input_p, '*')) {
        // MultiLineNotAsteriskChar MultiLineCommentChars(opt)
        match_multi_line_comment_chars(input_p);
        return TRUE;
    // LOOKAHEAD!
    } else if (!text_is_first(*input_p, "*/")) {
        // * PostAsteriskCommentChars(opt)
        consume_char(input_p);
        match_post_asterisk_comment_chars(input_p);
        return TRUE;
    }
    return FALSE;
}

static gboolean match_post_asterisk_comment_chars(gchar **input_p) {
    if (match_not_char2(input_p, '/', '*')) {
        // MultiLineNotForwardSlashOrAsteriskChar MultiLineCommentChars(opt)
        match_multi_line_comment_chars(input_p);
        return TRUE;
    // LOOKAHEAD!
    } else if (!text_is_first(*input_p, "*/")) {
        // * PostAsteriskCommentChars(opt)
        consume_char(input_p);
        match_post_asterisk_comment_chars(input_p);
        return TRUE;
    }
    return FALSE;
}

/*
 * MultiLineComment ::
 *     /* MultiLineCommentChars(opt) */                                       /*
 * MultiLineCommentChars ::
 *     MultiLineNotAsteriskChar MultiLineCommentChars(opt)
 *     * PostAsteriskCommentChars(opt)
 * PostAsteriskCommentChars ::
 *     MultiLineNotForwardSlashOrAsteriskChar MultiLineCommentChars(opt)
 *     * PostAsteriskCommentChars(opt)
 * MultiLineNotAsteriskChar ::
 *     SourceCharacter but not *
 * MultiLineNotForwardSlashOrAsteriskChar ::
 *     SourceCharacter but not one of / or *
 */
token_t *multi_line_comment(gchar **input_p) {

    gchar *input_old = *input_p;

    // /* MultiLineCommentChars(opt) */
    if (match_text(input_p, "/*")) {
        match_multi_line_comment_chars(input_p);
        if (match_text(input_p, "*/")) {
            return token_new_strndup_no_data(TOKEN_LEXICAL_MULTI_LINE_COMMENT,
                                             input_old, *input_p);
        }
    }

    // TODO: Error!
    return NULL;
}

gboolean single_line_comment_is_first(gchar *input) {
    return g_str_has_prefix(input, "//");
}

static gboolean match_single_line_comment_char(gchar **input_p) {
    if (!line_terminator_is_first(*input_p)) {
        consume_char(input_p);
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * SingleLineComment ::
 *     // SingleLineCommentChar*
 * SingleLineCommentChar ::
 *     SourceCharacter but not LineTerminator
 */
token_t *single_line_comment(gchar **input_p) {

    gchar *input_old = *input_p;

    // // SingleLineCommentChar*
    if (match_text(input_p, "//")) {
        while (match_single_line_comment_char(input_p)) {}
        return token_new_strndup_no_data(TOKEN_LEXICAL_SINGLE_LINE_COMMENT,
                                         input_old, *input_p);
    }

    // TODO: Error!
    return NULL;
}

/*
 * Token ::
 *     IdentifierName
 *     Punctuator
 *     NumericLiteral
 *     StringLiteral
 */
token_t *token(gchar **input_p) {

    // TODO

    // TODO: Error!
    return NULL;
}

static char *KEYWORDS[] = {
        "break",
        "do",
        "instanceof",
        "typeof",
        "case",
        "else",
        "new",
        "var",
        "catch",
        "finally",
        "return",
        "void",
        "continue",
        "for",
        "switch",
        "while",
        "debugger",
        "function",
        "this",
        "with",
        "default",
        "if",
        "throw",
        "delete",
        "in",
        "try"
};

static gboolean keyword_is_match(gchar *input) {
    for (gsize i = 0; i < G_N_ELEMENTS(KEYWORDS); ++i) {
        if (text_is_match(input, KEYWORDS[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

static char *FUTURE_RESERVED_WORDS[] = {
        "class",
        "enum",
        "extends",
        "super",
        "const",
        "export",
        "import",
        // Strict mode
        "implements",
        "let",
        "private",
        "public",
        "yield",
        "interface",
        "package",
        "protected",
        "static"
};

static gboolean future_reserved_word_is_match(gchar *input) {
    for (gsize i = 0; i < G_N_ELEMENTS(FUTURE_RESERVED_WORDS); ++i) {
        if (text_is_match(input, FUTURE_RESERVED_WORDS[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean reserved_word_is_match(gchar *input) {
    return keyword_is_match(input) || future_reserved_word_is_match(input)
           || null_literal_is_match(input) || boolean_literal_is_match(input);
}

/*
 * Identifier ::
 *     IdentifierName but not ReservedWord
 */
token_t *identifier(gchar **input_p) {

    token_t *identifier_name_token = identifier_name(input_p);
    if (!reserved_word_is_match(identifier_name_token->text)) {
        // TODO: Wrap child.
        return identifier_name_token;
    }

    // TODO: Error!
    return NULL;
}

static gboolean unicode_letter_is_first(gchar *input) {
    switch (g_unichar_type(g_utf8_get_char(input))) {
        case G_UNICODE_UPPERCASE_LETTER:
        case G_UNICODE_LOWERCASE_LETTER:
        case G_UNICODE_TITLECASE_LETTER:
        case G_UNICODE_MODIFIER_LETTER:
        case G_UNICODE_OTHER_LETTER:
        case G_UNICODE_LETTER_NUMBER:
            return TRUE;
        default:
            return FALSE;
    }
}

/*
 * UnicodeLetter ::
 *     any character in the Unicode categories “Uppercase letter (Lu)”,
 *             “Lowercase letter (Ll)”, “Titlecase letter (Lt)”,
 *             “Modifier letter (Lm)”, “Other letter (Lo)”, or
 *             “Letter number (Nl)”.
 */
static gboolean match_unicode_letter(gchar **input_p) {
    if (unicode_letter_is_first(*input_p)) {
        consume_char(input_p);
        return TRUE;
    } else {
        return FALSE;
    }
}

static gboolean identifier_start_is_first(gchar *input) {
    return unicode_letter_is_first(input) || char_is_first(input, '$')
           || char_is_first(input, '_') || char_is_first(input, '\\');
}

/**
 * IdentifierStart ::
 *     UnicodeLetter
 *     $
 *     _
 *     \ UnicodeEscapeSequence
 */
static gboolean match_identifier_start(gchar **input_p) {
    gboolean matched = match_unicode_letter(input_p) || match_char(input_p, '$')
                       || match_char(input_p, '_');
    if (matched) {
        return TRUE;
    } else {
        // \ UnicodeEscapeSequence
        gchar *input_old = *input_p;
        if (match_char(input_p, '\\')
            && match_unicode_escape_sequence(input_p)) {
            return TRUE;
        } else {
            // BACKTRACK!
            *input_p = input_old;
            return FALSE;
        }
    }
}

/*
 * UnicodeCombiningMark ::
 *     any character in the Unicode categories “Non-spacing mark (Mn)” or
 *     “Combining spacing mark (Mc)”
 */
static gboolean match_unicode_combining_mark(gchar **input_p) {
    switch (g_unichar_type(g_utf8_get_char(*input_p))) {
        case G_UNICODE_NON_SPACING_MARK:
        case G_UNICODE_SPACING_MARK:
            consume_char(input_p);
            return TRUE;
        default:
            return FALSE;
    }
}

/*
 * UnicodeDigit ::
 *     any character in the Unicode category “Decimal number (Nd)”
 */
static gboolean match_unicode_digit(gchar **input_p) {
    if (g_unichar_type(g_utf8_get_char(*input_p)) == G_UNICODE_DECIMAL_NUMBER) {
        consume_char(input_p);
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * UnicodeConnectorPunctuation ::
 *     any character in the Unicode category “Connector punctuation (Pc)”
 */
static gboolean match_unicode_connector_punctuation(gchar **input_p) {
    if (g_unichar_type(g_utf8_get_char(*input_p)) ==
            G_UNICODE_CONNECT_PUNCTUATION) {
        consume_char(input_p);
        return TRUE;
    } else {
        return FALSE;
    }
}

#define CHARACTER_ZWNJ_TEXT "\u200C"
#define CHARACTER_ZWJ_TEXT "\u200D"

/*
 * IdentifierPart ::
 *     IdentifierStart
 *     UnicodeCombiningMark
 *     UnicodeDigit
 *     UnicodeConnectorPunctuation
 *     <ZWNJ>
 *     <ZWJ>
 */
static gboolean match_identifier_part(gchar **input_p) {
    return match_identifier_start(input_p)
           || match_unicode_combining_mark(input_p)
           || match_unicode_digit(input_p)
           || match_unicode_connector_punctuation(input_p)
           || match_text(input_p, CHARACTER_ZWNJ_TEXT)
           || match_text(input_p, CHARACTER_ZWJ_TEXT);
}

/*
 * IdentifierName ::
 *     IdentifierStart
 *     IdentifierName IdentifierPart
 */
token_t *identifier_name(gchar **input_p) {

    gchar *input_old = *input_p;

    // IdentifierStart IdentifierPart*
    if (match_identifier_start(input_p)) {
        while (match_identifier_part(input_p)) {}
        return token_new_strndup_no_data(TOKEN_LEXICAL_IDENTIFIER_NAME,
                                         input_old, *input_p);
    }

    // TODO: Error!
    return NULL;
}

static char *PUNCTUATORS[] = {
        "{",
        "}",
        "(",
        ")",
        "[",
        "]",
        ".",
        ";",
        ",",
        "<",
        ">",
        "<=",
        ">=",
        "==",
        "!=",
        "===",
        "!==",
        "",
        "+",
        "-",
        "*",
        "%",
        "++",
        "--",
        "<<",
        ">>",
        ">>>",
        "&",
        "|",
        "^",
        "!",
        "~",
        "&&",
        "||",
        "?",
        ":",
        "=",
        "+=",
        "-=",
        "*=",
        "%=",
        "<<=",
        ">>=",
        ">>>=",
        "&=",
        "|=",
        "^="
};

gboolean punctuator_is_first(gchar *input) {
    for (gsize i = 0; i < G_N_ELEMENTS(FUTURE_RESERVED_WORDS); ++i) {
        if (text_is_first(input, FUTURE_RESERVED_WORDS[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

token_t *punctuator(gchar **input_p) {

    for (gsize i = 0; i < G_N_ELEMENTS(PUNCTUATORS); ++i) {
        try_match_text_and_return_token(input_p, TOKEN_LEXICAL_PUNCTUATOR,
                                        PUNCTUATORS[i])
    }

    // TODO: Error!
    return NULL;
}

static char *DIV_PUNCTUATORS[] = {
        "|=",
        "^="
};

gboolean div_punctuator_is_first(gchar *input) {
    for (gsize i = 0; i < G_N_ELEMENTS(DIV_PUNCTUATORS); ++i) {
        if (text_is_first(input, DIV_PUNCTUATORS[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

token_t *div_punctuator(gchar **input_p) {

    for (gsize i = 0; i < G_N_ELEMENTS(DIV_PUNCTUATORS); ++i) {
        try_match_text_and_return_token(input_p, TOKEN_LEXICAL_DIV_PUNCTUATOR,
                                        DIV_PUNCTUATORS[i])
    }

    // TODO: Error!
    return NULL;
}

#define TEXT_NULL "null"

/*
 * NullLiteral ::
 *     null
 */
token_t *null_literal(gchar **input_p) {
    // TODO
}

gboolean null_literal_is_match(gchar *input) {
    return text_is_match(input, TEXT_NULL);
}

#define TEXT_TRUE "true"
#define TEXT_FALSE "false"

/*
 * BooleanLiteral ::
 *     true
 *     false
 */
token_t *boolean_literal(gchar **input_p) {
    // TODO
}

gboolean boolean_literal_is_match(gchar *input) {
    return text_is_match(input, TEXT_TRUE) || text_is_match(input, TEXT_FALSE);
}

/*
 * NumericLiteral ::
 *     DecimalLiteral
 *     HexIntegerLiteral
 * The SourceCharacter immediately following a NumericLiteral must not be an
 * IdentifierStart or DecimalDigit.
 */
token_t *numeric_literal(gchar **input_p) {

    if (binary_integer_is_first(*input_p)) {
        // TODO: Wrap child
        return hex_integer_literal(input_p);
    }
    if (hex_integer_is_first(*input_p)) {
        // TODO: Wrap child
        return hex_integer_literal(input_p);
    }

    // TODO: Error!
    return NULL;
}

/*
 * DecimalLiteral ::
 *     DecimalIntegerLiteral . DecimalDigits(opt) ExponentPart(opt)
 *     . DecimalDigits ExponentPart(opt)
 *     DecimalIntegerLiteral ExponentPart(opt)
 * ExponentPart ::
 *     ExponentIndicator SignedInteger
 * ExponentIndicator :: one of
 *     e E
 * SignedInteger ::
 *     DecimalDigits
 *     + DecimalDigits
 *     - DecimalDigits
 */
token_t *decimal_literal(gchar **input_p) {
    
}

static gint64 *integer_new(gint64 value) {
    gint64 *value_p = g_new(gint64, 1);
    *value_p = value;
    return value_p;
}

static gint64 *integer_parse(gchar *input, gchar *input_end, guint base) {
    gchar *digits = g_strndup(input, input_end - input);
    gint64 *value_p = integer_new(g_ascii_strtoll(digits, NULL, base));
    g_free(digits);
    return value_p;
}

#define DECIMAL_DIGITS "0123456789"

gboolean match_decimal_digit(gchar **input_p) {
    return match_chars(input_p, DECIMAL_DIGITS);
}

/*
 * NONSTANDARD:
 *
 * Allows 0+ as a DecimalIntegerLiteral.
 *
 * DecimalIntegerLiteral ::
 *     DecimalDigit+
 *
 * STANDARD:
 *
 * DecimalIntegerLiteral ::
 *     0
 *     NonZeroDigit DecimalDigit*
 * DecimalDigit :: one of
 *     0 1 2 3 4 5 6 7 8 9
 * NonZeroDigit :: one of
 *     1 2 3 4 5 6 7 8 9
 */
token_t *decimal_integer_literal(gchar **input_p) {

    gchar *input_old = *input_p;
    if (match_decimal_digit(input_p)) {
        while (match_decimal_digit(input_p)) {}
        return token_new_strndup(TOKEN_LEXICAL_DECIMAL_INTEGER_LITERAL,
                                 input_old, *input_p,
                                 integer_parse(input_old, *input_p, 10),
                                 NULL);
    }

    // TODO: Error!
    return NULL;
}

#define TEXT_BINARY_INTEGER_PREFIX_1 "0b"
#define TEXT_BINARY_INTEGER_PREFIX_2 "0B"

gboolean binary_integer_is_first(gchar *input) {
    return text_is_first(input, TEXT_BINARY_INTEGER_PREFIX_1)
           || text_is_first(input, TEXT_BINARY_INTEGER_PREFIX_2);
}

#define BINARY_DIGITS "01"

static gboolean match_binary_digit(gchar **input_p) {
    return match_chars(input_p, BINARY_DIGITS);
}

/*
 * BinaryIntegerLiteral ::
 *     0b BinaryDigit+
 *     0B BinaryDigit+
 * BinaryDigit :: one of
 *     0 1
 */
token_t *binary_integer_literal(gchar **input_p) {

    gchar *input_old = *input_p;
    if (match_text(input_p, TEXT_BINARY_INTEGER_PREFIX_1)
        || match_text(input_p, TEXT_BINARY_INTEGER_PREFIX_2)) {
        gchar *input_digit_start = *input_p;
        if (match_binary_digit(input_p)) {
            while (match_binary_digit(input_p)) {}
            return token_new_strndup(TOKEN_LEXICAL_BINARY_INTEGER_LITERAL,
                                     input_old, *input_p,
                                     integer_parse(input_digit_start, *input_p,
                                                   2), NULL);
        }
    }

    // TODO: Error!
    return NULL;
}

#define TEXT_OCTAL_INTEGER_PREFIX_1 "0o"
#define TEXT_OCTAL_INTEGER_PREFIX_2 "0O"

gboolean octal_integer_is_first(gchar *input) {
    return text_is_first(input, TEXT_OCTAL_INTEGER_PREFIX_1)
           || text_is_first(input, TEXT_OCTAL_INTEGER_PREFIX_2);
}

#define OCTAL_DIGITS "01234567"

static gboolean match_octal_digit(gchar **input_p) {
    return match_chars(input_p, OCTAL_DIGITS);
}

/*
 * OctalIntegerLiteral ::
 *     0x OctalDigit+
 *     0X OctalDigit+
 * OctalDigit :: one of
 *     0 1 2 3 4 5 6 7
 */
token_t *octal_integer_literal(gchar **input_p) {

    gchar *input_old = *input_p;
    if (match_text(input_p, TEXT_OCTAL_INTEGER_PREFIX_1)
        || match_text(input_p, TEXT_OCTAL_INTEGER_PREFIX_2)) {
        gchar *input_digit_start = *input_p;
        if (match_octal_digit(input_p)) {
            while (match_octal_digit(input_p)) {}
            return token_new_strndup(TOKEN_LEXICAL_OCTAL_INTEGER_LITERAL,
                                     input_old, *input_p,
                                     integer_parse(input_digit_start, *input_p,
                                                   8), NULL);
        }
    }

    // TODO: Error!
    return NULL;
}

#define TEXT_HEX_INTEGER_PREFIX_1 "0x"
#define TEXT_HEX_INTEGER_PREFIX_2 "0X"

gboolean hex_integer_is_first(gchar *input) {
    return text_is_first(input, TEXT_HEX_INTEGER_PREFIX_1)
           || text_is_first(input, TEXT_HEX_INTEGER_PREFIX_2);
}

#define HEX_DIGITS "0123456789abcdefABCDEF"

static gboolean match_hex_digit(gchar **input_p) {
    return match_chars(input_p, HEX_DIGITS);
}

/*
 * HexIntegerLiteral ::
 *     0x HexDigit+
 *     0X HexDigit+
 * HexDigit :: one of
 *     0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F
 */
token_t *hex_integer_literal(gchar **input_p) {

    gchar *input_old = *input_p;
    if (match_text(input_p, TEXT_HEX_INTEGER_PREFIX_1)
        || match_text(input_p, TEXT_HEX_INTEGER_PREFIX_2)) {
        gchar *input_digit_start = *input_p;
        if (match_hex_digit(input_p)) {
            while (match_hex_digit(input_p)) {}
            return token_new_strndup(TOKEN_LEXICAL_HEX_INTEGER_LITERAL,
                                     input_old, *input_p,
                                     integer_parse(input_digit_start, *input_p,
                                                   16), NULL);
        }
    }

    // TODO: Error!
    return NULL;
}

gboolean match_unicode_escape_sequence(gchar **input_p) {
    // TODO
    return FALSE;
}
