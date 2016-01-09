//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#include "lexical_parser.h"

#include <errno.h>

#include "lexical_parser_utils.h"

// NOTE: All text arrays are sorted reversely for matching longer text first.

gboolean lexical_parse_normalize_input(gchar **input_p) {
    gchar *normalized_input = g_utf8_normalize(*input_p, -1,
                                               G_NORMALIZE_DEFAULT);
    if (normalized_input) {
        g_free(*input_p);
        *input_p = normalized_input;
        return TRUE;
    } else {
        return FALSE;
    }
}

static gboolean lexical_parse_is_token_ignored(token_t *token) {
    return token->id == TOKEN_LEXICAL_WHITE_SPACE
           || token->id == TOKEN_LEXICAL_LINE_TERMINATOR
           || token->id == TOKEN_LEXICAL_SINGLE_LINE_COMMENT
           || token->id == TOKEN_LEXICAL_MULTI_LINE_COMMENT;
}

GPtrArray *lexical_parse(gchar *input, token_t **error_p) {
    GPtrArray *token_list = token_list_new();
    while (*input) {
        token_t *token = lexical_token(&input);
        if (error_is_error(token)) {
            token_list_free(&token_list);
            *error_p = token;
            return NULL;
        }
        if (lexical_parse_is_token_ignored(token)) {
            token_free(&token);
        } else {
            g_ptr_array_add(token_list, token);
        }
    }
    return token_list;
}

/*
 * NONSTANDARD:
 * LexicalToken ::
 *     Identifier
 *     Keyword
 *     FutureReservedWord
 *     NullLiteral
 *     BooleanLiteral
 *     NumericLiteral
 *     StringLiteral
 *     //Punctuator
 *     WhiteSpace
 *     LineTerminator
 *     SingleLineComment
 *     MultiLineComment
 *     // NOTE: Punctuator is moved down for disambiguating with Comment.
 *     Punctuator
 */

token_t *lexical_token(gchar **input_p) {

    tokenize_and_return_if_is_first(input_p, keyword)
    tokenize_and_return_if_is_first(input_p, future_reserved_word)
    tokenize_and_return_if_is_first(input_p, null_literal)
    tokenize_and_return_if_is_first(input_p, boolean_literal)
    // NOTE:
    // {keyword,future_reserved_word,null_literal,boolean_literal}_is_first
    // checks identifier_part_is_first().
    tokenize_and_return_if_is_first(input_p, identifier)
    tokenize_and_return_if_is_first(input_p, numeric_literal)
    tokenize_and_return_if_is_first(input_p, string_literal)
    tokenize_and_return_if_is_first(input_p, white_space)
    tokenize_and_return_if_is_first(input_p, line_terminator)
    tokenize_and_return_if_is_first(input_p, single_line_comment)
    tokenize_and_return_if_is_first(input_p, multi_line_comment)
    tokenize_and_return_if_is_first(input_p, punctuator)

    return error_new_lexical(ERROR_LEXICAL_LEXICAL_TOKEN, *input_p);
}

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

static gboolean usp_is_first(gchar *input) {
    return g_unichar_isspace(g_utf8_get_char(input));
}

static gboolean usp_match(gchar **input_p) {
    if (usp_is_first(*input_p)) {
        char_consume(input_p);
        return TRUE;
    } else {
        return FALSE;
    }
}

#define CHARACTER_TAB_CHAR '\x09'
#define CHARACTER_VT_CHAR '\x0B'
#define CHARACTER_FF_CHAR '\x0C'
#define CHARACTER_SP_CHAR '\x20'
// TODO: NO_BREAK SPACE in UTF-8 is 0xC2 0xA0, while the C code is '\u00a0',
// according to http://www.fileformat.info/info/unicode/char/00a0/index.htm .
// Will this give the correct behavior?
#define CHARACTER_NBSP_TEXT "\xC2\xA0"
#define CHARACTER_BOM_TEXT "\uFEFF"

gboolean white_space_is_first(gchar *input) {
    return char_is_first(input, CHARACTER_TAB_CHAR)
           || char_is_first(input, CHARACTER_VT_CHAR)
           || char_is_first(input, CHARACTER_FF_CHAR)
           || char_is_first(input, CHARACTER_SP_CHAR)
           || text_is_first(input, CHARACTER_NBSP_TEXT)
           || text_is_first(input, CHARACTER_BOM_TEXT)
           || usp_is_first(input);
}

token_t *white_space(gchar **input_p) {

    return_token_if_match_char(input_p, CHARACTER_TAB_CHAR,
                               TOKEN_LEXICAL_WHITE_SPACE)
    return_token_if_match_char(input_p, CHARACTER_VT_CHAR,
                               TOKEN_LEXICAL_WHITE_SPACE)
    return_token_if_match_char(input_p, CHARACTER_FF_CHAR,
                               TOKEN_LEXICAL_WHITE_SPACE)
    return_token_if_match_char(input_p, CHARACTER_SP_CHAR,
                               TOKEN_LEXICAL_WHITE_SPACE)
    return_token_if_match_text(input_p, CHARACTER_NBSP_TEXT,
                               TOKEN_LEXICAL_WHITE_SPACE)
    return_token_if_match_text(input_p, CHARACTER_BOM_TEXT,
                               TOKEN_LEXICAL_WHITE_SPACE)
    return_token_if_match(input_p, usp_match(input_p),
                          TOKEN_LEXICAL_WHITE_SPACE)

    return error_new_lexical(ERROR_LEXICAL_WHITE_SPACE, *input_p);
}

/*
 * LineTerminator ::
 *     <LF>
 *     <CR>
 *     <LS>
 *     <PS>
 */

#define CHARACTER_LF_CHAR '\x0A'
#define CHARACTER_CR_CHAR '\x0D'
#define CHARACTER_LS_TEXT "\u2028"
#define CHARACTER_PS_TEXT "\u2029"

gboolean line_terminator_is_first(gchar *input) {
    return char_is_first(input, CHARACTER_LF_CHAR)
           || char_is_first(input, CHARACTER_CR_CHAR)
           || text_is_first(input, CHARACTER_LS_TEXT)
           || text_is_first(input, CHARACTER_PS_TEXT);
}

token_t *line_terminator(gchar **input_p) {

    return_token_if_match_char(input_p, CHARACTER_LF_CHAR,
                               TOKEN_LEXICAL_LINE_TERMINATOR)
    return_token_if_match_char(input_p, CHARACTER_CR_CHAR,
                               TOKEN_LEXICAL_LINE_TERMINATOR)
    return_token_if_match_text(input_p, CHARACTER_LS_TEXT,
                               TOKEN_LEXICAL_LINE_TERMINATOR)
    return_token_if_match_text(input_p, CHARACTER_PS_TEXT,
                               TOKEN_LEXICAL_LINE_TERMINATOR)

    return error_new_lexical(ERROR_LEXICAL_LINE_TERMINATOR, *input_p);
}

/*
 * LineTerminatorSequence ::
 *     <LF>
 *     <CR> [lookahead ∉ <LF> ]
 *     <LS>
 *     <PS>
 *     <CR> <LF>
 */

#define TEXT_CR_LF "\x0D\x0A"

static gboolean line_terminator_sequence_match(gchar **input_p) {
    return char_match(input_p, CHARACTER_LF_CHAR)
           // NOTE: <CR> <LF> is Promoted over <CR> for being prefix of <CR>.
           || text_match(input_p, TEXT_CR_LF)
           || char_match(input_p, CHARACTER_CR_CHAR)
           || text_match(input_p, CHARACTER_LS_TEXT)
           || text_match(input_p, CHARACTER_PS_TEXT);
}

/*
 * MultiLineComment ::
 *     / * MultiLineCommentChars? * /
 * MultiLineCommentChars ::
 *     MultiLineNotAsteriskChar MultiLineCommentChars?
 *     * PostAsteriskCommentChars?
 * PostAsteriskCommentChars ::
 *     MultiLineNotForwardSlashOrAsteriskChar MultiLineCommentChars?
 *     * PostAsteriskCommentChars?
 * MultiLineNotAsteriskChar ::
 *     SourceCharacter but not *
 * MultiLineNotForwardSlashOrAsteriskChar ::
 *     SourceCharacter but not one of / or *
 */

gboolean multi_line_comment_is_first(gchar *input) {
    return text_is_first(input, "/*");
}

static gboolean post_asterisk_comment_chars_match_save(gchar **input_p,
                                                       GString *buffer);

static gboolean multi_line_comment_chars_match_save(gchar **input_p,
                                                    GString *buffer) {
    if (char_match_not_save(input_p, '*', buffer)) {
        // MultiLineNotAsteriskChar MultiLineCommentChars?
        multi_line_comment_chars_match_save(input_p, buffer);
        return TRUE;
    // LOOKAHEAD!
    } else if (!text_is_first(*input_p, "*/")) {
        // * PostAsteriskCommentChars?
        char_consume_save(input_p, buffer);
        post_asterisk_comment_chars_match_save(input_p, buffer);
        return TRUE;
    }
    return FALSE;
}

static gboolean post_asterisk_comment_chars_match_save(gchar **input_p,
                                                       GString *buffer) {
    if (char_match_not_2_save(input_p, '/', '*', buffer)) {
        // MultiLineNotForwardSlashOrAsteriskChar MultiLineCommentChars?
        multi_line_comment_chars_match_save(input_p, buffer);
        return TRUE;
    // LOOKAHEAD!
    } else if (!text_is_first(*input_p, "*/")) {
        // * PostAsteriskCommentChars?
        char_consume_save(input_p, buffer);
        post_asterisk_comment_chars_match_save(input_p, buffer);
        return TRUE;
    }
    return FALSE;
}

token_t *multi_line_comment(gchar **input_p) {

    GString *buffer = g_string_new(NULL);
    if (text_match_save(input_p, "/*", buffer)) {
        multi_line_comment_chars_match_save(input_p, buffer);
        if (text_match_save(input_p, "*/", buffer)) {
            return token_new_gstring(TOKEN_LEXICAL_MULTI_LINE_COMMENT, buffer);
        }
    }

    g_string_free(buffer, TRUE);
    return error_new_lexical(ERROR_LEXICAL_MULTI_LINE_COMMENT, *input_p);
}

DEFINE_TOKEN_IS_TOKEN_FUNC(multi_line_comment, TOKEN_LEXICAL_MULTI_LINE_COMMENT)

DEFINE_TOKEN_GET_VALUE_FUNC(multi_line_comment, GString)

/*
 * SingleLineComment ::
 *     // SingleLineCommentChar*
 * SingleLineCommentChar ::
 *     SourceCharacter but not LineTerminator
 */

gboolean single_line_comment_is_first(gchar *input) {
    return text_is_first(input, "//");
}

static gboolean single_line_comment_char_match_save(gchar **input_p,
                                                    GString *buffer) {
    if (!line_terminator_is_first(*input_p)) {
        char_consume_save(input_p, buffer);
        return TRUE;
    } else {
        return FALSE;
    }
}

static DEFINE_MATCH_ANY_SAVE_FUNC(single_line_comment_char_match_save,
                                  single_line_comment_char_match_any_save)

token_t *single_line_comment(gchar **input_p) {

    GString *buffer = g_string_new(NULL);
    if (text_match_save(input_p, "//", buffer)) {
        single_line_comment_char_match_any_save(input_p, buffer);
        return token_new_gstring(TOKEN_LEXICAL_SINGLE_LINE_COMMENT, buffer);
    }

    g_string_free(buffer, TRUE);
    return error_new_lexical(ERROR_LEXICAL_SINGLE_LINE_COMMENT, *input_p);
}

DEFINE_TOKEN_IS_TOKEN_FUNC(single_line_comment,
                           TOKEN_LEXICAL_SINGLE_LINE_COMMENT)

DEFINE_TOKEN_GET_VALUE_FUNC(single_line_comment, GString)

/*
 * UnicodeLetter ::
 *     any character in the Unicode categories “Uppercase letter (Lu)”,
 *             “Lowercase letter (Ll)”, “Titlecase letter (Lt)”,
 *             “Modifier letter (Lm)”, “Other letter (Lo)”, or
 *             “Letter number (Nl)”.
 */

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

static gboolean unicode_letter_match_save(gchar **input_p, GString *buffer) {
    if (unicode_letter_is_first(*input_p)) {
        char_consume_save(input_p, buffer);
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * IdentifierStart ::
 *     UnicodeLetter
 *     $
 *     _
 *     \ UnicodeEscapeSequence
 */

static gboolean identifier_start_is_first(gchar *input) {
    return unicode_letter_is_first(input) || char_is_first(input, '$')
           || char_is_first(input, '_') || char_is_first(input, '\\');
}

static gboolean identifier_start_match_save_value(gchar **input_p,
                                                  GString *buffer) {
    gboolean matched = unicode_letter_match_save(input_p, buffer)
                       || char_match_save(input_p, '$', buffer)
                       || char_match_save(input_p, '_', buffer);
    if (matched) {
        return TRUE;
    } else {
        // \ UnicodeEscapeSequence
        gchar *input_old = *input_p;
        if (char_match(input_p, '\\')
            && unicode_escape_sequence_match_save_value(input_p, buffer)) {
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

static gboolean unicode_combining_mark_is_first(gchar *input_p) {
    switch (g_unichar_type(g_utf8_get_char(input_p))) {
        case G_UNICODE_NON_SPACING_MARK:
        case G_UNICODE_SPACING_MARK:
            return TRUE;
        default:
            return FALSE;
    }
}

static gboolean unicode_combining_mark_match_save(gchar **input_p,
                                                  GString *buffer) {
    if (unicode_combining_mark_is_first(*input_p)) {
        char_consume_save(input_p, buffer);
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * UnicodeDigit ::
 *     any character in the Unicode category “Decimal number (Nd)”
 */

static gboolean unicode_digit_is_first(gchar *input) {
    if (g_unichar_type(g_utf8_get_char(input)) == G_UNICODE_DECIMAL_NUMBER) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static gboolean unicode_digit_match_save(gchar **input_p, GString *buffer) {
    if (unicode_digit_is_first(*input_p)) {
        char_consume_save(input_p, buffer);
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * UnicodeConnectorPunctuation ::
 *     any character in the Unicode category “Connector punctuation (Pc)”
 */

static gboolean unicode_connector_punctuation_is_first(gchar *input) {
    if (g_unichar_type(g_utf8_get_char(input))
        == G_UNICODE_CONNECT_PUNCTUATION) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static gboolean unicode_connector_punctuation_match_save(gchar **input_p,
                                                         GString *buffer) {
    if (unicode_connector_punctuation_is_first(*input_p)) {
        char_consume_save(input_p, buffer);
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * IdentifierPart ::
 *     IdentifierStart
 *     UnicodeCombiningMark
 *     UnicodeDigit
 *     UnicodeConnectorPunctuation
 *     <ZWNJ>
 *     <ZWJ>
 */

#define CHARACTER_ZWNJ_TEXT "\u200C"
#define CHARACTER_ZWJ_TEXT "\u200D"

gboolean identifier_part_is_first(gchar *input) {
    return identifier_start_is_first(input)
           || unicode_combining_mark_is_first(input)
           || unicode_digit_is_first(input)
           || unicode_connector_punctuation_is_first(input)
           || text_is_first(input, CHARACTER_ZWNJ_TEXT)
           || text_is_first(input, CHARACTER_ZWJ_TEXT);
}

static gboolean identifier_part_match_save_value(gchar **input_p,
                                                 GString *buffer) {
    return identifier_start_match_save_value(input_p, buffer)
           || unicode_combining_mark_match_save(input_p, buffer)
           || unicode_digit_match_save(input_p, buffer)
           || unicode_connector_punctuation_match_save(input_p, buffer)
           || text_match_save(input_p, CHARACTER_ZWNJ_TEXT, buffer)
           || text_match_save(input_p, CHARACTER_ZWJ_TEXT, buffer);
}

static DEFINE_MATCH_ANY_SAVE_FUNC(identifier_part_match_save_value,
                                  identifier_part_match_any_save_value)

/*
 * NONSTANDARD:
 * IdentifierName is merged into Identifier
 * Identifier ::
 *     IdentifierStart IdentifierPart*
 * Identifier is not ReservedWord
 */

gboolean identifier_is_first(gchar *input) {
    return identifier_start_is_first(input);
}

token_t *identifier(gchar **input_p) {

    GString *string = g_string_new(NULL);
    if (identifier_start_match_save_value(input_p, string)) {
        identifier_part_match_any_save_value(input_p, string);
        return token_new_gstring(TOKEN_LEXICAL_IDENTIFIER, string);
    }

    g_string_free(string, TRUE);
    return error_new_lexical(ERROR_LEXICAL_IDENTIFIER, *input_p);
}

DEFINE_TOKEN_IS_TOKEN_FUNC(identifier, TOKEN_LEXICAL_IDENTIFIER)

DEFINE_TOKEN_GET_VALUE_FUNC(identifier, GString)

static char *KEYWORD_ID_NAMES[] = {
        "KEYWORD_WITH",
        "KEYWORD_WHILE",
        "KEYWORD_VOID",
        "KEYWORD_VAR",
        "KEYWORD_TYPEOF",
        "KEYWORD_TRY",
        "KEYWORD_THROW",
        "KEYWORD_THIS",
        "KEYWORD_SWITCH",
        "KEYWORD_RETURN",
        "KEYWORD_NEW",
        "KEYWORD_INSTANCEOF",
        "KEYWORD_IN",
        "KEYWORD_IF",
        "KEYWORD_FUNCTION",
        "KEYWORD_FOR",
        "KEYWORD_FINALLY",
        "KEYWORD_ELSE",
        "KEYWORD_DO",
        "KEYWORD_DELETE",
        "KEYWORD_DEFAULT",
        "KEYWORD_DEBUGGER",
        "KEYWORD_CONTINUE",
        "KEYWORD_CATCH",
        "KEYWORD_CASE",
        "KEYWORD_BREAK",
        "KEYWORD_GET",
        "KEYWORD_SET"
};

DEFINE_ID_GET_NAME_FUNC(keyword_id, KEYWORD_ID_NAMES)

static char *KEYWORDS[] = {
        "with",
        "while",
        "void",
        "var",
        "typeof",
        "try",
        "throw",
        "this",
        "switch",
        "return",
        "new",
        "instanceof",
        "in",
        "if",
        "function",
        "for",
        "finally",
        "else",
        "do",
        "delete",
        "default",
        "debugger",
        "continue",
        "catch",
        "case",
        "break",
        "get",
        "set"
};

gboolean keyword_is_first(gchar *input) {
    return text_array_match(&input, KEYWORDS, G_N_ELEMENTS(KEYWORDS))
           && !identifier_part_is_first(input);
}

static DEFINE_PRIMITIVE_NEW_FUNC(keyword_id)

static DEFINE_PRIMITIVE_CLONE_FUNC_FUNC(keyword_id)

static DEFINE_ID_TO_STRING_FUNC_FUNC(keyword_id)

token_t *keyword(gchar **input_p) {

    gsize index;
    if (text_array_match_save_index(input_p, KEYWORDS, G_N_ELEMENTS(KEYWORDS),
                                    &index)) {
        return token_new(TOKEN_LEXICAL_KEYWORD,
                         keyword_id_new((keyword_id_t) index),
                         keyword_id_clone_func, NULL,
                         keyword_id_to_string_func);
    }

    return error_new_lexical(ERROR_LEXICAL_KEYWORD, *input_p);
}

DEFINE_TOKEN_IS_TOKEN_FUNC(keyword, TOKEN_LEXICAL_KEYWORD)

DEFINE_TOKEN_GET_ID_FUNC(keyword)

DEFINE_TOKEN_IS_TOKEN_WITH_ID_FUNC(keyword)

// Strict mode

static char *FUTURE_RESERVED_WORD_NAMES[] = {
        "FUTURE_RESERVED_WORD_YIELD",
        "FUTURE_RESERVED_WORD_SUPER",
        "FUTURE_RESERVED_WORD_STATIC",
        "FUTURE_RESERVED_WORD_PUBLIC",
        "FUTURE_RESERVED_WORD_PROTECTED",
        "FUTURE_RESERVED_WORD_PRIVATE",
        "FUTURE_RESERVED_WORD_PACKAGE",
        "FUTURE_RESERVED_WORD_LET",
        "FUTURE_RESERVED_WORD_INTERFACE",
        "FUTURE_RESERVED_WORD_IMPORT",
        "FUTURE_RESERVED_WORD_IMPLEMENTS",
        "FUTURE_RESERVED_WORD_EXTENDS",
        "FUTURE_RESERVED_WORD_EXPORT",
        "FUTURE_RESERVED_WORD_ENUM",
        "FUTURE_RESERVED_WORD_CONST",
        "FUTURE_RESERVED_WORD_CLASS"
};

DEFINE_ID_GET_NAME_FUNC(future_reserved_word_id, FUTURE_RESERVED_WORD_NAMES)

static char *FUTURE_RESERVED_WORDS[] = {
        "yield",
        "super",
        "static",
        "public",
        "protected",
        "private",
        "package",
        "let",
        "interface",
        "import",
        "implements",
        "extends",
        "export",
        "enum",
        "const",
        "class"
};

gboolean future_reserved_word_is_first(gchar *input) {
    return text_array_match(&input, FUTURE_RESERVED_WORDS,
                            G_N_ELEMENTS(FUTURE_RESERVED_WORDS))
           && !identifier_part_is_first(input);
}

static DEFINE_PRIMITIVE_NEW_FUNC(future_reserved_word_id)

static DEFINE_PRIMITIVE_CLONE_FUNC_FUNC(future_reserved_word_id)

static DEFINE_ID_TO_STRING_FUNC_FUNC(future_reserved_word_id)

token_t *future_reserved_word(gchar **input_p) {

    gsize index;
    if (text_array_match_save_index(input_p, FUTURE_RESERVED_WORDS,
                                    G_N_ELEMENTS(FUTURE_RESERVED_WORDS),
                                    &index)) {
        return token_new(TOKEN_LEXICAL_FUTURE_RESERVED_WORD,
                         future_reserved_word_id_new(
                                 (future_reserved_word_id_t) index),
                         future_reserved_word_id_clone_func, NULL,
                         future_reserved_word_id_to_string_func);
    }

    return error_new_lexical(ERROR_LEXICAL_FUTURE_RESERVED_WORD, *input_p);
}

DEFINE_TOKEN_IS_TOKEN_FUNC(future_reserved_word,
                           TOKEN_LEXICAL_FUTURE_RESERVED_WORD)

DEFINE_TOKEN_GET_ID_FUNC(future_reserved_word)

DEFINE_TOKEN_IS_TOKEN_WITH_ID_FUNC(future_reserved_word)

static char *PUNCTUATOR_ID_NAMES[] = {
        "PUNCTUATOR_INCREMENT",
        "PUNCTUATOR_ADD_ASSIGNMENT",
        "PUNCTUATOR_PLUS",
        "PUNCTUATOR_MODULO_ASSIGNMENT",
        "PUNCTUATOR_PERCENT",
        "PUNCTUATOR_LOGICAL_AND",
        "PUNCTUATOR_BITWISE_AND_ASSIGNMENT",
        "PUNCTUATOR_AMPERSAND",
        "PUNCTUATOR_MULTIPLY_ASSIGNMENT",
        "PUNCTUATOR_ASTERISK",
        "PUNCTUATOR_CURLY_BRACE_RIGHT",
        "PUNCTUATOR_CURLY_BRACE_LEFT",
        "PUNCTUATOR_SQUARE_BRACKET_RIGHT",
        "PUNCTUATOR_SQUARE_BRACKET_LEFT",
        "PUNCTUATOR_PARENTHESIS_RIGHT",
        "PUNCTUATOR_PARENTHESIS_LEFT",
        "PUNCTUATOR_DOT",
        "PUNCTUATOR_DIVIDE_ASSIGNMENT",
        "PUNCTUATOR_SLASH",
        "PUNCTUATOR_QUESTION_MARK",
        "PUNCTUATOR_STRICT_DOES_NOT_EQUAL",
        "PUNCTUATOR_DOES_NOT_EQUAL",
        "PUNCTUATOR_EXCLAMATION",
        "PUNCTUATOR_COLON",
        "PUNCTUATOR_SEMICOLON",
        "PUNCTUATOR_COMMA",
        "PUNCTUATOR_DECREMENT",
        "PUNCTUATOR_SUBTRACT_ASSIGNMENT",
        "PUNCTUATOR_MINUS",
        "PUNCTUATOR_LOGICAL_OR",
        "PUNCTUATOR_BITWISE_OR_ASSIGNMENT",
        "PUNCTUATOR_VERTICAL_BAR",
        "PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_ASSIGNMENT",
        "PUNCTUATOR_UNSIGNED_RIGHT_SHIFT",
        "PUNCTUATOR_SIGNED_RIGHT_SHIFT_ASSIGNMENT",
        "PUNCTUATOR_SIGNED_RIGHT_SHIFT",
        "PUNCTUATOR_GREATER_THAN_OR_EQUAL",
        "PUNCTUATOR_ANGLE_BRACKET_RIGHT",
        "PUNCTUATOR_STRICT_EQUALS",
        "PUNCTUATOR_EQUALS",
        "PUNCTUATOR_EQUALS_SIGN",
        "PUNCTUATOR_LESS_THAN_OR_EQUAL",
        "PUNCTUATOR_LEFT_SHIFT_ASSIGNMENT",
        "PUNCTUATOR_LEFT_SHIFT",
        "PUNCTUATOR_ANGLE_BRACKET_LEFT",
        "PUNCTUATOR_TILDE",
        "PUNCTUATOR_BITWISE_XOR_ASSIGNMENT",
        "PUNCTUATOR_CARET"
};

DEFINE_ID_GET_NAME_FUNC(punctuator_id, PUNCTUATOR_ID_NAMES)

static char *PUNCTUATORS[] = {
        "++",
        "+=",
        "+",
        "%=",
        "%",
        "&&",
        "&=",
        "&",
        "*=",
        "*",
        "}",
        "{",
        "]",
        "[",
        ")",
        "(",
        ".",
        "/=",
        "/",
        "?",
        "!==",
        "!=",
        "!",
        ":",
        ";",
        ",",
        "--",
        "-=",
        "-",
        "||",
        "|=",
        "|",
        ">>>=",
        ">>>",
        ">>=",
        ">>",
        ">=",
        ">",
        "===",
        "==",
        "=",
        "<=",
        "<<=",
        "<<",
        "<",
        "~",
        "^=",
        "^"
};

gboolean punctuator_is_first(gchar *input) {
    return text_array_is_first(input, PUNCTUATORS, G_N_ELEMENTS(PUNCTUATORS));
}

static DEFINE_PRIMITIVE_NEW_FUNC(punctuator_id)

static DEFINE_PRIMITIVE_CLONE_FUNC_FUNC(punctuator_id)

static DEFINE_ID_TO_STRING_FUNC_FUNC(punctuator_id)

token_t *punctuator(gchar **input_p) {

    gsize index;
    if (text_array_match_save_index(input_p, PUNCTUATORS,
                                    G_N_ELEMENTS(PUNCTUATORS), &index)) {
        return token_new(TOKEN_LEXICAL_PUNCTUATOR,
                         punctuator_id_new((punctuator_id_t) index),
                         punctuator_id_clone_func, NULL,
                         punctuator_id_to_string_func);
    }

    return error_new_lexical(ERROR_LEXICAL_PUNCTUATOR, *input_p);
}

DEFINE_TOKEN_IS_TOKEN_FUNC(punctuator, TOKEN_LEXICAL_PUNCTUATOR)

DEFINE_TOKEN_GET_ID_FUNC(punctuator)

DEFINE_TOKEN_IS_TOKEN_WITH_ID_FUNC(punctuator)

/*
 * NullLiteral ::
 *     null
 */

#define TEXT_NULL "null"

gboolean null_literal_is_first(gchar *input) {
    return text_match(&input, TEXT_NULL) && !identifier_part_is_first(input);
}

token_t *null_literal(gchar **input_p) {

    return_token_if_match_text(input_p, TEXT_NULL, TOKEN_LEXICAL_NULL_LITERAL)

    return error_new_lexical(ERROR_LEXICAL_NULL_LITERAL, *input_p);
}

/*
 * BooleanLiteral ::
 *     true
 *     false
 */

#define TEXT_TRUE "true"
#define TEXT_FALSE "false"

gboolean boolean_literal_is_first(gchar *input) {
    return (text_match(&input, TEXT_TRUE) || text_match(&input, TEXT_FALSE))
           && !identifier_part_is_first(input);
}

static DEFINE_PRIMITIVE_NEW_FUNC_WITH_TYPE(boolean, gboolean)

static DEFINE_PRIMITIVE_CLONE_FUNC_FUNC_WITH_TYPE(boolean, gboolean)

static GString *boolean_to_string_func(gpointer _boolean) {
    gboolean *boolean = _boolean;
    return g_string_new(boolean ? "true" : "false");
}

token_t *boolean_literal(gchar **input_p) {

    if (text_match(input_p, TEXT_TRUE)) {
        return token_new(TOKEN_LEXICAL_BOOLEAN_LITERAL, boolean_new(TRUE),
                         boolean_clone_func, NULL, boolean_to_string_func);
    } else if (text_match(input_p, TEXT_FALSE)) {
        return token_new(TOKEN_LEXICAL_BOOLEAN_LITERAL, boolean_new(FALSE),
                         boolean_clone_func, NULL, boolean_to_string_func);
    }

    return error_new_lexical(ERROR_LEXICAL_BOOLEAN_LITERAL, *input_p);
}

DEFINE_TOKEN_IS_TOKEN_FUNC(boolean_literal, TOKEN_LEXICAL_BOOLEAN_LITERAL)

DEFINE_TOKEN_GET_VALUE_FUNC(boolean_literal, gboolean)

/*
 * NONSTANDARD:
 * Doesn't allow ".1", or "1.".
 * DecimalLiteral ::
 *     DecimalDigit+ (. DecimalDigit+)? (e|E (+|-)? DecimalDigit+)?
 *
 * STANDARD:
 * DecimalLiteral ::
 *     (0|NonZeroDigit DecimalDigit*) (. DecimalDigit*)? ExponentPart?
 *     . DecimalDigit+ ExponentPart?
 * DecimalDigit :: one of
 *     0 1 2 3 4 5 6 7 8 9
 * NonZeroDigit :: one of
 *     1 2 3 4 5 6 7 8 9
 * ExponentPart ::
 *     ExponentIndicator SignedInteger
 * ExponentIndicator ::
 *     e|E
 * SignedInteger ::
 *     DecimalDigits
 *     + DecimalDigits
 *     - DecimalDigits
 */

static gboolean decimal_digit_is_first(gchar *input) {
    return chars_is_first(input, "0123456789");
}

static gboolean decimal_digit_match(gchar **input_p) {
    return chars_match(input_p, "0123456789");
}

static DEFINE_MATCH_MULTIPLE_FUNC(decimal_digit_match)

DEFINE_PRIMITIVE_NEW_FUNC_WITH_TYPE(number, gdouble)

static gdouble *number_new_parse(gchar *input, gchar *input_end) {
    gchar *text = g_strndup(input, input_end - input);
    gdouble *value_p = number_new(g_ascii_strtod(text, NULL));
    g_free(text);
    if (errno) {
        // TODO: Does the library overflow/underflow handling comply with
        // standard?
    }
    return value_p;
}

static gdouble *number_new_parse_integer(gchar *input, gchar *input_end,
                                         guint base) {
    gchar *text = g_strndup(input, input_end - input);
    gdouble *value_p = number_new(g_ascii_strtoll(text, NULL, base));
    g_free(text);
    if (errno) {
        // TODO: Does the library overflow/underflow handling comply with
        // standard?
        errno = 0;
    }
    return value_p;
}

DEFINE_PRIMITIVE_CLONE_FUNC_FUNC_WITH_TYPE(number, gdouble)

static GString *number_to_string_func(gpointer _number) {
    gdouble *number = _number;
    GString *result = g_string_new(NULL);
    g_string_printf(result, "%f", *number);
    return result;
}

static token_t *decimal_literal(gchar **input_p) {

    do {
        gchar *input_old = *input_p;
        // DecimalDigit+
        if (decimal_digit_match_multiple(input_p)) {
            // (. DecimalDigit+)?
            if (char_match(input_p, '.')) {
                if (!decimal_digit_match_multiple(input_p)) {
                    break;
                }
            }
            // (e|E (+|-)? DecimalDigit+)?
            if (chars_match(input_p, "eE")) {
                chars_match(input_p, "+-");
                if (!decimal_digit_match_multiple(input_p)) {
                    break;
                }
            }
            return token_new(TOKEN_LEXICAL_NUMERIC_LITERAL,
                             number_new_parse(input_old, *input_p),
                             number_clone_func, NULL, number_to_string_func);
        }
    } while (FALSE);

    return error_new_lexical(ERROR_LEXICAL_NUMERIC_LITERAL_DECIMAL, *input_p);
}

/*
 * BinaryIntegerLiteral ::
 *     0b BinaryDigit+
 *     0B BinaryDigit+
 * BinaryDigit :: one of
 *     0 1
 */

#define TEXT_BINARY_INTEGER_PREFIX_1 "0b"
#define TEXT_BINARY_INTEGER_PREFIX_2 "0B"

static gboolean binary_integer_is_first(gchar *input) {
    return text_is_first(input, TEXT_BINARY_INTEGER_PREFIX_1)
           || text_is_first(input, TEXT_BINARY_INTEGER_PREFIX_2);
}

static gboolean binary_digit_match(gchar **input_p) {
    return chars_match(input_p, "01");
}

static DEFINE_MATCH_MULTIPLE_FUNC(binary_digit_match)

static token_t *binary_integer_literal(gchar **input_p) {

    if (text_match(input_p, TEXT_BINARY_INTEGER_PREFIX_1)
        || text_match(input_p, TEXT_BINARY_INTEGER_PREFIX_2)) {
        gchar *input_old = *input_p;
        if (binary_digit_match_multiple(input_p)) {
            return token_new(TOKEN_LEXICAL_NUMERIC_LITERAL,
                             number_new_parse_integer(input_old, *input_p, 2),
                             number_clone_func, NULL, number_to_string_func);
        }
    }

    return error_new_lexical(ERROR_LEXICAL_NUMERIC_LITERAL_BINARY, *input_p);
}

/*
 * OctalIntegerLiteral ::
 *     0x OctalDigit+
 *     0X OctalDigit+
 * OctalDigit :: one of
 *     0 1 2 3 4 5 6 7
 */

#define TEXT_OCTAL_INTEGER_PREFIX_1 "0o"
#define TEXT_OCTAL_INTEGER_PREFIX_2 "0O"

static gboolean octal_integer_is_first(gchar *input) {
    return text_is_first(input, TEXT_OCTAL_INTEGER_PREFIX_1)
           || text_is_first(input, TEXT_OCTAL_INTEGER_PREFIX_2);
}

static gboolean octal_digit_match(gchar **input_p) {
    return chars_match(input_p, "01234567");
}

static DEFINE_MATCH_MULTIPLE_FUNC(octal_digit_match)

static token_t *octal_integer_literal(gchar **input_p) {

    if (text_match(input_p, TEXT_OCTAL_INTEGER_PREFIX_1)
        || text_match(input_p, TEXT_OCTAL_INTEGER_PREFIX_2)) {
        gchar *input_old = *input_p;
        if (octal_digit_match_multiple(input_p)) {
            return token_new(TOKEN_LEXICAL_NUMERIC_LITERAL,
                             number_new_parse_integer(input_old, *input_p, 8),
                             number_clone_func, NULL, number_to_string_func);
        }
    }

    return error_new_lexical(ERROR_LEXICAL_NUMERIC_LITERAL_OCTAL, *input_p);
}

/*
 * HexIntegerLiteral ::
 *     0x HexDigit+
 *     0X HexDigit+
 * HexDigit :: one of
 *     0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F
 */

#define TEXT_HEX_INTEGER_PREFIX_1 "0x"
#define TEXT_HEX_INTEGER_PREFIX_2 "0X"

static gboolean hex_integer_is_first(gchar *input) {
    return text_is_first(input, TEXT_HEX_INTEGER_PREFIX_1)
           || text_is_first(input, TEXT_HEX_INTEGER_PREFIX_2);
}

#define CHARACTERS_HEX_DIGITS_CHARS "0123456789abcdefABCDEF"

static gboolean hex_digit_match(gchar **input_p) {
    return chars_match(input_p, CHARACTERS_HEX_DIGITS_CHARS);
}

static gboolean hex_digit_match_save_char(gchar **input_p, gunichar *char_p) {
    return chars_match_save_char(input_p, CHARACTERS_HEX_DIGITS_CHARS, char_p);
}

static guint hex_digit_to_value(gunichar char0) {

    g_assert(chars_is_match(char0, CHARACTERS_HEX_DIGITS_CHARS));
    if (char0 >= '0' && char0 <= '9') {
        return (guint) char0 - '0';
    }
    if (char0 >= 'a' && char0 <= 'f') {
        return (guint) char0 - 'a' + 10;
    }
    if (char0 >= 'A' && char0 <= 'F') {
        return (guint) char0 - 'A' + 10;
    }

    g_error("Illegal hex digit char: %c", char0);
}

static gboolean hex_digit_match_save_value(gchar **input_p, guint *value_p) {
    gunichar char0;
    if (hex_digit_match_save_char(input_p, &char0)) {
        *value_p = hex_digit_to_value(char0);
        return TRUE;
    } else {
        return FALSE;
    }
}

static DEFINE_MATCH_MULTIPLE_FUNC(hex_digit_match)

static token_t *hex_integer_literal(gchar **input_p) {

    if (text_match(input_p, TEXT_HEX_INTEGER_PREFIX_1)
        || text_match(input_p, TEXT_HEX_INTEGER_PREFIX_2)) {
        gchar *input_old = *input_p;
        if (hex_digit_match_multiple(input_p)) {
            return token_new(TOKEN_LEXICAL_NUMERIC_LITERAL,
                             number_new_parse_integer(input_old, *input_p, 16),
                             number_clone_func, NULL, number_to_string_func);
        }
    }

    return error_new_lexical(ERROR_LEXICAL_NUMERIC_LITERAL_HEX, *input_p);
}

/*
 * NumericLiteral ::
 *     DecimalLiteral
 *     BinaryIntegerLiteral
 *     OctalIntegerLiteral
 *     HexIntegerLiteral
 * The SourceCharacter immediately following a NumericLiteral must not be an
 * IdentifierStart or DecimalDigit.
 */

gboolean numeric_literal_is_first(gchar *input) {
    // NOTE: Includes BinaryIntegerLiteral, OctalIntegerLiteral and
    // HexIntegerLiteral.
    return decimal_digit_is_first(input);
}

token_t *numeric_literal(gchar **input_p) {

    token_t *token;
    if (binary_integer_is_first(*input_p)) {
        token = binary_integer_literal(input_p);
    } else if (octal_integer_is_first(*input_p)) {
        token = octal_integer_literal(input_p);
    } else if (hex_integer_is_first(*input_p)) {
        token = hex_integer_literal(input_p);
    } else {
        token = decimal_literal(input_p);
    }
    if (!identifier_is_first(*input_p)) {
        return token;
    }

    return error_new_lexical(
            ERROR_LEXICAL_NUMERIC_LITERAL_FOLLOWED_BY_IDENTIFIER, *input_p);
}

DEFINE_TOKEN_IS_TOKEN_FUNC(numeric_literal, TOKEN_LEXICAL_NUMERIC_LITERAL)

DEFINE_TOKEN_GET_VALUE_FUNC(numeric_literal, gdouble)

/*
 * StringLiteral ::
 *     " DoubleStringCharacter* "
 *     ' SingleStringCharacter* '
 * DoubleStringCharacter ::
 *     SourceCharacter but not one of " or \ or LineTerminator
 *     \ EscapeSequence
 *     \ LineTerminatorSequence
 * SingleStringCharacter ::
 *     SourceCharacter but not one of ' or \ or LineTerminator
 *     \ EscapeSequence
 *     \ LineTerminatorSequence
 * EscapeSequence ::
 *     CharacterEscapeSequence
 * // NONSTANDARD
 *     0
 * // STANDARD
 *     0 [lookahead ∉ DecimalDigit]
 *     x HexDigit HexDigit
 *     u HexDigit HexDigit HexDigit HexDigit
 * CharacterEscapeSequence ::
 *     SingleEscapeCharacter
 *     NonEscapeCharacter
 * SingleEscapeCharacter :: one of
 *     ' " \ b f n r t v
 * NonEscapeCharacter ::
 *     SourceCharacter but not one of EscapeCharacter or LineTerminator
 * EscapeCharacter ::
 *     SingleEscapeCharacter
 * // NONSTANDARD
 *     0
 * // STANDARD
 *     DecimalDigit
 *     x
 *     u
 */

static gboolean hex_escape_sequence_match_save_value(gchar **input_p,
                                                     GString *buffer) {

    gchar *input_old = *input_p;
    guint digit[2];
    if (char_match(input_p, 'x')
        && hex_digit_match_save_value(input_p, &digit[0])
        && hex_digit_match_save_value(input_p, &digit[1])) {
        gunichar char0 = 16 * digit[0] + digit[1];
        g_string_append_unichar(buffer, char0);
        return TRUE;
    }

    // BACKTRACK!
    *input_p = input_old;
    return FALSE;
}

gboolean unicode_escape_sequence_match_save_value(gchar **input_p,
                                                  GString *buffer) {

    gchar *input_old = *input_p;
    guint digit[4];
    if (char_match(input_p, 'u')
        && hex_digit_match_save_value(input_p, &digit[0])
        && hex_digit_match_save_value(input_p, &digit[1])
        && hex_digit_match_save_value(input_p, &digit[2])
        && hex_digit_match_save_value(input_p, &digit[3])) {
        gunichar char0 = 4096 * digit[0] + 256 * digit[1] + 16 * digit[2]
                         + digit[3];
        g_string_append_unichar(buffer, char0);
        return TRUE;
    }

    // BACKTRACK!
    *input_p = input_old;
    return FALSE;
}

static gboolean string_character_match_save(gchar **input_p, gchar quote,
                                            GString *buffer) {
    if (char_is_first(*input_p, quote) || line_terminator_is_first(*input_p)) {
        return FALSE;
    }
    // \ LineTerminatorSequence
    // \ EscapeSequence
    if (char_match(input_p, '\\')) {
        // LineTerminatorSequence
        if (line_terminator_sequence_match(input_p)) {
            return TRUE;
        } else {
            // SingleEscapeCharacter
            // 0
            // HexEscapeSequence
            // UnicodeEscapeSequence
            if (char_match_save(input_p, '\'', buffer)
                || char_match_save(input_p, '"', buffer)
                || char_match_save(input_p, '\\', buffer)
                || char_match_save_other(input_p, 'b', buffer, '\x08')
                || char_match_save_other(input_p, 'f', buffer, '\x0C')
                || char_match_save_other(input_p, 'n', buffer, '\x0A')
                || char_match_save_other(input_p, 'r', buffer, '\x0D')
                || char_match_save_other(input_p, 't', buffer, '\x09')
                || char_match_save_other(input_p, 'v', buffer, '\x0B')
                || char_match_save_other(input_p, '0', buffer, '\0')
                || hex_escape_sequence_match_save_value(input_p, buffer)
                || unicode_escape_sequence_match_save_value(input_p, buffer)) {
                return TRUE;
            // NonEscapeCharacter
            } else {
                char_consume_save(input_p, buffer);
                return TRUE;
            }
        }
    } else {
        // SourceCharacter but not one of `quote` or \ or LineTerminator
        char_consume_save(input_p, buffer);
        return TRUE;
    }
}

static void string_character_match_any_save(gchar **input_p, gchar quote,
                                            GString *buffer) {
    while (string_character_match_save(input_p, quote, buffer)) {}
}

gboolean string_literal_is_first(gchar *input) {
    return char_is_first(input, '"') || char_is_first(input, '\'');
}

token_t *string_literal(gchar **input_p) {

    do {
        gchar quote;
        if (char_match(input_p, '\'')) {
            quote = '\'';
        } else if (char_match(input_p, '"')) {
            quote = '"';
        } else {
            break;
        }
        GString *string = g_string_new(NULL);
        string_character_match_any_save(input_p, quote, string);
        if (char_match(input_p, quote)) {
            return token_new_gstring(TOKEN_LEXICAL_STRING_LITERAL, string);
        }
        g_string_free(string, TRUE);
    } while (FALSE);

    return error_new_lexical(ERROR_LEXICAL_STRING_LITERAL, *input_p);
}

DEFINE_TOKEN_IS_TOKEN_FUNC(string_literal, TOKEN_LEXICAL_STRING_LITERAL)

DEFINE_TOKEN_GET_VALUE_FUNC(string_literal, GString)
