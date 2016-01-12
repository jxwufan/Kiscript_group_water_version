//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#include "expression_parser.h"

#include "function_parser.h"
#include "lexical_parser.h"
#include "syntactic_parser_utils.h"
#include "parser.h"

/*
 * AST:
 * PrimaryExpression = Keyword(this)|Identifier|NullLiteral|BooleanLiteral
 *         |NumericLiteral|StringLiteral|ArrayLiteral|ObjectLiteral|Expression
 *
 * GRAMMAR:
 * PrimaryExpression :
 *     this
 *     Identifier
 *     Literal : NullLiteral|BooleanLiteral|NumericLiteral|StringLiteral
 *     ArrayLiteral
 *     ObjectLiteral
 *     ( Expression )
 */

gboolean primary_expression_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_THIS)
           || token_is_first_id(input, position, TOKEN_LEXICAL_IDENTIFIER)
           || token_is_first_id(input, position, TOKEN_LEXICAL_NULL_LITERAL)
           || token_is_first_id(input, position, TOKEN_LEXICAL_BOOLEAN_LITERAL)
           || token_is_first_id(input, position, TOKEN_LEXICAL_NUMERIC_LITERAL)
           || token_is_first_id(input, position, TOKEN_LEXICAL_STRING_LITERAL)
           || array_literal_is_first(input, position)
           || object_literal_is_first(input, position)
           || token_is_first_punctuator(input, position,
                                        PUNCTUATOR_PARENTHESIS_LEFT);
}

token_t *primary_expression(GPtrArray *input, gsize *position_p) {

    token_t *token;
    if (token_match_keyword_clone(input, position_p, KEYWORD_THIS, &token)
        || token_match_id_clone(input, position_p, TOKEN_LEXICAL_IDENTIFIER,
                                &token)
        || token_match_id_clone(input, position_p, TOKEN_LEXICAL_NULL_LITERAL,
                                &token)
        || token_match_id_clone(input, position_p,
                                TOKEN_LEXICAL_BOOLEAN_LITERAL, &token)
        || token_match_id_clone(input, position_p,
                                TOKEN_LEXICAL_NUMERIC_LITERAL, &token)
        || token_match_id_clone(input, position_p, TOKEN_LEXICAL_STRING_LITERAL,
                                &token)) {
        return token;
    }

    tokenize_and_return_if_is_first(input, position_p, array_literal)

    tokenize_and_return_if_is_first(input, position_p, object_literal)

    if (token_match_punctuator(input, position_p,
                               PUNCTUATOR_PARENTHESIS_LEFT)) {
        token_t *expression_token;
        tokenize_or_return_error(input, position_p, expression,
                                 expression_token)
        match_punctuator_or_free_and_return_error(input, position_p,
                PUNCTUATOR_PARENTHESIS_RIGHT, expression_token,
                ERROR_EXPRESSION_PRIMARY_EXPRESSION_PARENTHESIS_LEFT)
        return expression_token;
    }

    return error_new_syntactic(ERROR_EXPRESSION_PRIMARY_EXPRESSION,
                               *position_p);
}

/*
 * AST:
 * ArrayLiteral - AssignmentExpression*
 *
 * GRAMMAR:
 * NONSTANDARD: Disallow multiple or trailing comma. Anyway JSON disallows this.
 * ArrayLiteral :
 *     [ (AssignmentExpression (, AssignmentExpression)*)? ]
 * STANDARD:
 * ArrayLiteral :
 *     [ ElementList? ,* ]
 * ElementList :
 *     ,* AssignmentExpression (,+ AssignmentExpression)*
 */

gboolean array_literal_is_first(GPtrArray *input, gsize position) {
    return token_is_first_punctuator(input, position,
                                     PUNCTUATOR_SQUARE_BRACKET_LEFT);
}

token_t *array_literal(GPtrArray *input, gsize *position_p) {

    match_punctuator_or_return_error(input, position_p,
            PUNCTUATOR_SQUARE_BRACKET_LEFT,
            ERROR_EXPRESSION_ARRAY_LITERAL_SQUARE_BRACKET_LEFT)

    token_t *array_literal_token = token_new_no_data(
            TOKEN_EXPRESSION_ARRAY_LITERAL);

    gboolean first = TRUE;
    while (!token_match_punctuator(input, position_p,
                                   PUNCTUATOR_SQUARE_BRACKET_RIGHT)) {

        if (first) {
            first = FALSE;
        } else {
            match_punctuator_or_free_and_return_error(input, position_p,
                    PUNCTUATOR_COMMA, array_literal_token,
                   ERROR_EXPRESSION_ARRAY_LITERAL_COMMA_OR_SQUARE_BRACKET_RIGHT)
        }

        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, assignment_expression, array_literal_token)
    }

    return array_literal_token;
}

/*
 * AST:
 * ObjectLiteral - PropertyAssignment*
 *
 * GRAMMAR:
 * ObjectLiteral :
 *     { (PropertyAssignment (, PropertyAssignment)*)? }
 */

gboolean object_literal_is_first(GPtrArray *input, gsize position) {
    return token_is_first_punctuator(input, position,
                                     PUNCTUATOR_CURLY_BRACE_LEFT);
}

token_t *object_literal(GPtrArray *input, gsize *position_p) {

    match_punctuator_or_return_error(input, position_p,
            PUNCTUATOR_CURLY_BRACE_LEFT,
            ERROR_EXPRESSION_OBJECT_LITERAL_CURLY_BRACE_LEFT)

    token_t *object_literal_token = token_new_no_data(
            TOKEN_EXPRESSION_OBJECT_LITERAL);

    gboolean first = TRUE;
    while (!token_match_punctuator(input, position_p,
                                   PUNCTUATOR_CURLY_BRACE_RIGHT)) {

        if (first) {
            first = FALSE;
        } else {
            match_punctuator_or_free_and_return_error(input, position_p,
                    PUNCTUATOR_COMMA, object_literal_token,
                    ERROR_EXPRESSION_OBJECT_LITERAL_COMMA_OR_CURLY_BRACE_RIGHT)
        }

        tokenize_and_add_child_or_free_parent_and_return_error(input,
                    position_p, property_assignment, object_literal_token)
    }

    return object_literal_token;
}

/*
 * AST:
 * PropertyAssignment = PropertyAssignmentPlain|PropertyAssignmentGet
 *     |PropertyAssignmentSet
 *
 * GRAMMAR:
 * PropertyAssignment :
 *     (PropertyAssignmentPlain|PropertyAssignmentGet|PropertyAssignmentSet)
 */

token_t *property_assignment(GPtrArray *input, gsize *position_p) {

    tokenize_and_return_if_is_first(input, position_p,
                                    property_assignment_plain)
    tokenize_and_return_if_is_first(input, position_p,
                                    property_assignment_get)
    tokenize_and_return_if_is_first(input, position_p,
                                    property_assignment_set)

    return error_new_syntactic(ERROR_EXPRESSION_PROPERTY_ASSIGNMENT,
                               *position_p);
}

/*
 * AST:
 * PropertyAssignmentPlain = PropertyName AssignmentExpression
 *
 * GRAMMAR:
 * PropertyAssignmentPlain :
 *     PropertyName : AssignmentExpression
 */

gboolean property_assignment_plain_is_first(GPtrArray *input, gsize position) {
    return property_name_is_first(input, position);
}

token_t *property_assignment_plain(GPtrArray *input, gsize *position_p) {

    token_t *property_assignment_plain_token = token_new_no_data(
            TOKEN_EXPRESSION_PROPERTY_ASSIGNMENT_PLAIN);

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            property_name, property_assignment_plain_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_COLON, property_assignment_plain_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_PLAIN_COLON)

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            assignment_expression, property_assignment_plain_token)

    return property_assignment_plain_token;
}

/*
 * AST:
 * PropertyAssignmentGet = PropertyName FunctionBody
 *
 * GRAMMAR:
 * PropertyAssignmentGet :
 *     get PropertyName ( ) { FunctionBody }
 */

gboolean property_assignment_get_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_GET);
}

token_t *property_assignment_get(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_GET,
                                  ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_GET);

    token_t *property_assignment_get_token = token_new_no_data(
            TOKEN_EXPRESSION_PROPERTY_ASSIGNMENT_GET);

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            property_name, property_assignment_get_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_LEFT, property_assignment_get_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_PARENTHESIS_LEFT)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_RIGHT, property_assignment_get_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_PARENTHESIS_RIGHT)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_CURLY_BRACE_LEFT, property_assignment_get_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_CURLY_BRACE_LEFT)

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            property_name, property_assignment_get_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_CURLY_BRACE_RIGHT, property_assignment_get_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_GET_CURLY_BRACE_RIGHT)

    return property_assignment_get_token;
}

/*
 * AST:
 * PropertyAssignmentSet = PropertyName Identifier FunctionBody
 *
 * GRAMMAR:
 * PropertyAssignmentSet :
 *     set PropertyName ( Identifier ) { FunctionBody }
 */

gboolean property_assignment_set_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_SET);
}

token_t *property_assignment_set(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_SET,
                                  ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_SET);

    token_t *property_assignment_get_token = token_new_no_data(
            TOKEN_EXPRESSION_PROPERTY_ASSIGNMENT_SET);

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            property_name, property_assignment_get_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_LEFT, property_assignment_get_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_PARENTHESIS_LEFT)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_RIGHT, property_assignment_get_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_PARENTHESIS_RIGHT)

    match_token_id_clone_and_add_child_or_free_parent_and_return_error(input,
            position_p, TOKEN_LEXICAL_IDENTIFIER, property_assignment_get_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_IDENTIFIER)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_CURLY_BRACE_LEFT, property_assignment_get_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_CURLY_BRACE_LEFT)

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            property_name, property_assignment_get_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_CURLY_BRACE_RIGHT, property_assignment_get_token,
            ERROR_EXPRESSION_PROPERTY_ASSIGNMENT_SET_CURLY_BRACE_RIGHT)

    return property_assignment_get_token;
}

/*
 * AST:
 * PropertyName = Identifier|StringLiteral|NumericLiteral|AssignmentExpression
 *
 * GRAMMAR:
 * PropertyName :
 *     Identifier
 *     StringLiteral
 *     NumericLiteral
 *     [ AssignmentExpression ]
 */

gboolean property_name_is_first(GPtrArray *input, gsize position) {
    return token_is_first_id(input, position, TOKEN_LEXICAL_IDENTIFIER)
           || token_is_first_id(input, position, TOKEN_LEXICAL_STRING_LITERAL)
           || token_is_first_id(input, position, TOKEN_LEXICAL_NUMERIC_LITERAL)
           || token_is_first_punctuator(input, position,
                                        PUNCTUATOR_SQUARE_BRACKET_LEFT);
}

token_t *property_name(GPtrArray *input, gsize *position_p) {

    match_token_id_clone_and_return_if_is_first(input, position_p,
                                                TOKEN_LEXICAL_IDENTIFIER)
    match_token_id_clone_and_return_if_is_first(input, position_p,
                                                TOKEN_LEXICAL_STRING_LITERAL)
    match_token_id_clone_and_return_if_is_first(input, position_p,
                                                TOKEN_LEXICAL_NUMERIC_LITERAL);

    if (token_match_punctuator(input, position_p,
                               PUNCTUATOR_SQUARE_BRACKET_LEFT)) {
        token_t *assignment_expression_token;
        tokenize_or_return_error(input, position_p, assignment_expression,
                                 assignment_expression_token);
        match_punctuator_or_free_and_return_error(input, position_p,
                PUNCTUATOR_SQUARE_BRACKET_RIGHT, assignment_expression_token,
                ERROR_EXPRESSION_PROPERTY_NAME_SQUARE_BRACKET_RIGHT)
        return assignment_expression_token;
    }

    return error_new_syntactic(ERROR_EXPRESSION_PROPERTY_NAME, *position_p);
}

/*
 * AST:
 * CallableExpression = PrimaryExpression|FunctionExpression|NewExpression
 *     |PropertyAccessor|CallExpression
 * PropertyAccessor - CallableExpression (Identifier|Expression)
 * CallExpression - CallableExpression ArgumentList
 *
 * GRAMMAR:
 * NonLeftRecursiveCallableExpression :
 *     PrimaryExpression
 *     FunctionExpression
 *     NewExpression
 * CallableExpression :
 *     NonLeftRecursiveCallableExpression
 *     PropertyAccessor
 *     CallExpression
 * PropertyAccessor :
 *     CallableExpression . Identifier
 *     CallableExpression [ Expression ]
 * CallExpression :
 *     CallableExpression ArgumentList
 */

static gboolean non_left_recursive_callable_expression_is_first(
        GPtrArray *input, gsize position) {
    return primary_expression_is_first(input, position)
           || function_expression_is_first(input, position)
           || new_expression_is_first(input, position);
}

static token_t *non_left_recursive_callable_expression(GPtrArray *input,
                                                       gsize *position_p) {

    tokenize_and_return_if_is_first(input, position_p, primary_expression)
    tokenize_and_return_if_is_first(input, position_p, function_expression)
    tokenize_and_return_if_is_first(input, position_p, new_expression)

    return error_new_syntactic(
            ERROR_EXPRESSION_CALLABLE_EXPRESSION_CALLABLE_EXPRESSION,
            *position_p);
}

gboolean callable_expression_is_first(GPtrArray *input, gsize position) {
    return non_left_recursive_callable_expression_is_first(input, position);
}

token_t *callable_expression(GPtrArray *input, gsize *position_p) {

    token_t *callable_expression_token;
    tokenize_or_return_error(input, position_p,
                             non_left_recursive_callable_expression,
                             callable_expression_token)

    // LEFT_RECURSION: lookahead with the power of recursive-descent parsing.
    while (TRUE) {

        if (token_match_punctuator(input, position_p, PUNCTUATOR_DOT)) {

            token_t *property_accessor_token = token_new_no_data(
                    TOKEN_EXPRESSION_PROPERTY_ACCESSOR);
            token_add_child(property_accessor_token, callable_expression_token);
            match_token_id_clone_and_add_child_or_free_parent_and_return_error(
                    input, position_p, TOKEN_LEXICAL_IDENTIFIER,
                    property_accessor_token,
                    ERROR_EXPRESSION_PROPERTY_ACCESSOR_IDENTIFIER)
            callable_expression_token = property_accessor_token;

        } else if (token_match_punctuator(input, position_p,
                                   PUNCTUATOR_SQUARE_BRACKET_LEFT)) {

            token_t *property_accessor_token = token_new_no_data(
                    TOKEN_EXPRESSION_PROPERTY_ACCESSOR);
            token_add_child(property_accessor_token, callable_expression_token);
            tokenize_and_add_child_or_free_parent_and_return_error(input,
                    position_p, expression, property_accessor_token);
            match_punctuator_or_free_and_return_error(input, position_p,
                    PUNCTUATOR_SQUARE_BRACKET_RIGHT, property_accessor_token,
                    ERROR_EXPRESSION_PROPERTY_ACCESSOR_SQUARE_BRACKET_RIGHT)
            callable_expression_token = property_accessor_token;

        } else if (argument_list_is_first(input, *position_p)) {

            // NOTE: CallExpression is shared with the implementation of
            // NewExpression. See the comment there.
            token_t *call_expression_token = token_new_no_data(
                    TOKEN_EXPRESSION_CALL_EXPRESSION);
            token_add_child(call_expression_token, callable_expression_token);
            tokenize_and_add_child_or_free_parent_and_return_error(input,
                    position_p, argument_list, call_expression_token)
            callable_expression_token = call_expression_token;

        } else {
            break;
        }
    }

    return callable_expression_token;
}

/*
 * AST:
 * ArgumentList - AssignmentExpression*
 *
 * GRAMMAR:
 * ArgumentList :
 *     ( (AssignmentExpression (, AssignmentExpression)*)? )
 */

gboolean argument_list_is_first(GPtrArray *input, gsize position) {
    return token_is_first_punctuator(input, position,
                                     PUNCTUATOR_PARENTHESIS_LEFT);
}

token_t *argument_list(GPtrArray *input, gsize *position_p) {

    match_punctuator_or_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_LEFT,
            ERROR_EXPRESSION_ARGUMENT_LIST_PARENTHESIS_LEFT)

    token_t *argument_list_token = token_new_no_data(
            TOKEN_EXPRESSION_ARGUMENT_LIST);

    gboolean first = TRUE;
    while (!token_match_punctuator(input, position_p,
                                   PUNCTUATOR_PARENTHESIS_RIGHT)) {

        if (first) {
            first = FALSE;
        } else {
            match_punctuator_or_free_and_return_error(input, position_p,
                    PUNCTUATOR_COMMA, argument_list_token,
                    ERROR_EXPRESSION_ARGUMENT_LIST_COMMA_OR_PARENTHESIS_RIGHT)
        }

        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, assignment_expression, argument_list_token)
    }

    return argument_list_token;
}

/**
 * AST:
 * NewExpression - CallableExpression ArgumentList
 *
 * GRAMMAR:
 * // NONSTANDARD: Disallow NewExpression without ArgumentList.
 * NewExpression :
 *     new CallableExpression ArgumentList
 *
 */
gboolean new_expression_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_NEW);
}

token_t *new_expression(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_NEW,
                                  ERROR_EXPRESSION_NEW_EXPRESSION_NEW)

    token_t *callable_expression_token;
    tokenize_or_return_error(input, position_p, callable_expression,
                             callable_expression_token)

    // HACK: CallExpression is CallableExpression, so we cannot disambiguate the
    // matching between CallExpression and NewExpression. We simply take the
    // must-be CallExpression and turn it into a NewExpression here.
    if (callable_expression_token->id != TOKEN_EXPRESSION_CALL_EXPRESSION) {
        return error_new_syntactic(
                ERROR_EXPRESSION_NEW_EXPRESSION_CALL_EXPRESSION, *position_p);
    }
    callable_expression_token->id = TOKEN_EXPRESSION_NEW_EXPRESSION;

    return callable_expression_token;
}

/*
 * NOTE: LeftHandSideExpression here is only a syntactic symbol, and the real
 * check is performed at execution time.
 *
 * AST:
 * LeftHandSideExpression = CallableExpression
 *
 * GRAMMAR:
 * LeftHandSideExpression :
 *     CallableExpression
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(left_hand_side_expression, callable_expression)

token_t *left_hand_side_expression(GPtrArray *input, gsize *position_p) {
    return callable_expression(input, position_p);
}

/*
 * AST:
 * PostfixExpression = LeftHandSideExpression
 * PostfixExpression - LeftHandSideExpression (Punctuator(++)|Punctuator(--))
 *
 * GRAMMAR:
 * NONSTANDARD: Allow LineTerminator.
 * PostfixExpression :
 *     LeftHandSideExpression
 *     LeftHandSideExpression ++
 *     LeftHandSideExpression --
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(postfix_expression, left_hand_side_expression)

token_t *postfix_expression(GPtrArray *input, gsize *position_p) {

    token_t *left_hand_side_expression_token;
    tokenize_or_return_error(input, position_p, left_hand_side_expression,
                             left_hand_side_expression_token)

    token_t *operator_token;
    if (token_match_punctuator_clone(input, position_p, PUNCTUATOR_INCREMENT,
                                     &operator_token)
        || token_match_punctuator_clone(input, position_p, PUNCTUATOR_DECREMENT,
                                        &operator_token)) {

        token_t *postfix_expression_token = token_new_no_data(
                TOKEN_EXPRESSION_POSTFIX_EXPRESSION);
        token_add_child(postfix_expression_token,
                        left_hand_side_expression_token);
        token_add_child(postfix_expression_token, operator_token);
        return postfix_expression_token;

    } else {
        return left_hand_side_expression_token;
    }
}

/*
 * AST:
 * UnaryExpression = PostfixExpression
 * UnaryExpression - (delete|void|typeof|++|--|+|-|~|!) UnaryExpression
 *
 * GRAMMAR:
 * UnaryExpression :
 *     PostfixExpression
 *     delete UnaryExpression
 *     void UnaryExpression
 *     typeof UnaryExpression
 *     ++ UnaryExpression
 *     -- UnaryExpression
 *     + UnaryExpression
 *     - UnaryExpression
 *     ~ UnaryExpression
 *     ! UnaryExpression
 */

gboolean unary_expression_is_first(GPtrArray *input, gsize position) {
    return postfix_expression_is_first(input, position)
           || token_is_first_keyword(input, position, KEYWORD_DELETE)
           || token_is_first_keyword(input, position, KEYWORD_VOID)
           || token_is_first_keyword(input, position, KEYWORD_TYPEOF)
           || token_is_first_punctuator(input, position, PUNCTUATOR_INCREMENT)
           || token_is_first_punctuator(input, position, PUNCTUATOR_DECREMENT)
           || token_is_first_punctuator(input, position, PUNCTUATOR_PLUS)
           || token_is_first_punctuator(input, position, PUNCTUATOR_MINUS)
           || token_is_first_punctuator(input, position, PUNCTUATOR_TILDE)
           || token_is_first_punctuator(input, position,
                                        PUNCTUATOR_EXCLAMATION);
}

token_t *unary_expression(GPtrArray *input, gsize *position_p) {

    token_t *operator_token;
    if (token_match_keyword_clone(input, position_p, KEYWORD_DELETE,
                                  &operator_token)
        || token_match_keyword_clone(input, position_p, KEYWORD_VOID,
                                     &operator_token)
        || token_match_keyword_clone(input, position_p, KEYWORD_TYPEOF,
                                     &operator_token)
        || token_match_punctuator_clone(input, position_p, PUNCTUATOR_INCREMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p, PUNCTUATOR_DECREMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p, PUNCTUATOR_PLUS,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p, PUNCTUATOR_MINUS,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p, PUNCTUATOR_TILDE,
                                        &operator_token)
        ||token_match_punctuator_clone(input, position_p,
                                       PUNCTUATOR_EXCLAMATION, &operator_token)
            ) {

        token_t *unary_expression_token = token_new_no_data(
                TOKEN_EXPRESSION_UNARY_EXPRESSION);
        token_add_child(unary_expression_token, operator_token);
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, unary_expression, unary_expression_token)
        return unary_expression_token;

    } else {
        return postfix_expression(input, position_p);
    }
}

/*
 * AST:
 * MultiplicativeExpression = UnaryExpression
 * MultiplicativeExpression - MultiplicativeExpression (*|/|%) UnaryExpression
 *
 * GRAMMAR:
 * MultiplicativeExpression :
 *     UnaryExpression
 *     MultiplicativeExpression * UnaryExpression
 *     MultiplicativeExpression / UnaryExpression
 *     MultiplicativeExpression % UnaryExpression
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(multiplicative_expression, unary_expression)

token_t *multiplicative_expression(GPtrArray *input, gsize *position_p) {

    token_t *unary_or_multiplicative_expression_token;
    tokenize_or_return_error(input, position_p, unary_expression,
                             unary_or_multiplicative_expression_token)

    // LEFT_RECURSION
    token_t *operator_token;
    while (token_match_punctuator_clone(input, position_p, PUNCTUATOR_ASTERISK,
                                        &operator_token)
           || token_match_punctuator_clone(input, position_p, PUNCTUATOR_SLASH,
                                           &operator_token)
           || token_match_punctuator_clone(input, position_p,
                                           PUNCTUATOR_PERCENT,
                                           &operator_token)) {

        token_t *multiplicative_expression_token = token_new_no_data(
                TOKEN_EXPRESSION_MULTIPLICATIVE_EXPRESSION);
        token_add_child(multiplicative_expression_token,
                        unary_or_multiplicative_expression_token);
        token_add_child(multiplicative_expression_token, operator_token);
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, unary_expression, multiplicative_expression_token)
        unary_or_multiplicative_expression_token =
                multiplicative_expression_token;
    }

    return unary_or_multiplicative_expression_token;
}

/*
 * AST:
 * AdditiveExpression = MultiplicativeExpression
 * AdditiveExpression - AdditiveExpression (+|-) MultiplicativeExpression
 *
 * GRAMMAR:
 * AdditiveExpression :
 *     MultiplicativeExpression
 *     AdditiveExpression + MultiplicativeExpression
 *     AdditiveExpression - MultiplicativeExpression
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(additive_expression, multiplicative_expression)

token_t *additive_expression(GPtrArray *input, gsize *position_p) {

    token_t *multiplicative_or_additive_expression_token;
    tokenize_or_return_error(input, position_p, multiplicative_expression,
                             multiplicative_or_additive_expression_token)

    // LEFT_RECURSION
    token_t *operator_token;
    while (token_match_punctuator_clone(input, position_p, PUNCTUATOR_PLUS,
                                        &operator_token)
           || token_match_punctuator_clone(input, position_p, PUNCTUATOR_MINUS,
                                           &operator_token)) {

        token_t *additive_expression_token = token_new_no_data(
                TOKEN_EXPRESSION_ADDITIVE_EXPRESSION);
        token_add_child(additive_expression_token,
                        multiplicative_or_additive_expression_token);
        token_add_child(additive_expression_token, operator_token);
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, multiplicative_expression,
                additive_expression_token)
        multiplicative_or_additive_expression_token = additive_expression_token;
    }

    return multiplicative_or_additive_expression_token;
}

/*
 * AST:
 * ShiftExpression = AdditiveExpression
 * ShiftExpression - ShiftExpression (<<|>>|>>>) AdditiveExpression
 *
 * GRAMMAR:
 * ShiftExpression :
 *     AdditiveExpression
 *     ShiftExpression << AdditiveExpression
 *     ShiftExpression >> AdditiveExpression
 *     ShiftExpression >>> AdditiveExpression
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(shift_expression, additive_expression)

token_t *shift_expression(GPtrArray *input, gsize *position_p) {

    token_t *additive_or_shift_expression_token;
    tokenize_or_return_error(input, position_p, additive_expression,
                             additive_or_shift_expression_token)

    // LEFT_RECURSION
    token_t *operator_token;
    while (token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_LEFT_SHIFT, &operator_token)
           || token_match_punctuator_clone(input, position_p,
                                           PUNCTUATOR_SIGNED_RIGHT_SHIFT,
                                           &operator_token)
           || token_match_punctuator_clone(input, position_p,
                                           PUNCTUATOR_UNSIGNED_RIGHT_SHIFT,
                                           &operator_token)) {

        token_t *shift_expression_token = token_new_no_data(
                TOKEN_EXPRESSION_SHIFT_EXPRESSION);
        token_add_child(shift_expression_token,
                        additive_or_shift_expression_token);
        token_add_child(shift_expression_token, operator_token);
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, additive_expression, shift_expression_token)
        additive_or_shift_expression_token = shift_expression_token;
    }

    return additive_or_shift_expression_token;
}

/*
 * AST:
 * RelationalExpression = ShiftExpression
 * RelationalExpression - RelationalExpression (<|>|<=|>=|instanceof)
 *         ShiftExpression
 *
 * GRAMMAR:
 * // NONSTANDARD: No in.
 * RelationalExpression :
 *     ShiftExpression
 *     RelationalExpression < ShiftExpression
 *     RelationalExpression > ShiftExpression
 *     RelationalExpression <= ShiftExpression
 *     RelationalExpression >= ShiftExpression
 *     RelationalExpression instanceof ShiftExpression
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(relational_expression, shift_expression)

token_t *relational_expression(GPtrArray *input, gsize *position_p) {

    token_t *shift_or_relational_expression_token;
    tokenize_or_return_error(input, position_p, shift_expression,
                             shift_or_relational_expression_token)

    // LEFT_RECURSION
    token_t *operator_token;
    while (token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_ANGLE_BRACKET_LEFT,
                                        &operator_token)
           || token_match_punctuator_clone(input, position_p,
                                           PUNCTUATOR_ANGLE_BRACKET_RIGHT,
                                           &operator_token)
           || token_match_punctuator_clone(input, position_p,
                                           PUNCTUATOR_LESS_THAN_OR_EQUAL,
                                           &operator_token)
           || token_match_punctuator_clone(input, position_p,
                                           PUNCTUATOR_GREATER_THAN_OR_EQUAL,
                                           &operator_token)
           || token_match_keyword_clone(input, position_p, KEYWORD_INSTANCEOF,
                                        &operator_token)) {

        token_t *relational_expression_token = token_new_no_data(
                TOKEN_EXPRESSION_RELATIONAL_EXPRESSION);
        token_add_child(relational_expression_token,
                        shift_or_relational_expression_token);
        token_add_child(relational_expression_token, operator_token);
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, shift_expression, relational_expression_token)
        shift_or_relational_expression_token = relational_expression_token;
    }

    return shift_or_relational_expression_token;
}

/*
 * AST:
 * EqualityExpression = RelationalExpression
 * EqualityExpression - EqualityExpression (==|!=|===|!==) RelationalExpression
 *
 * GRAMMAR:
 * EqualityExpression :
 *     RelationalExpression
 *     EqualityExpression == RelationalExpression
 *     EqualityExpression != RelationalExpression
 *     EqualityExpression === RelationalExpression
 *     EqualityExpression !== RelationalExpression
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(equality_expression, relational_expression)

token_t *equality_expression(GPtrArray *input, gsize *position_p) {

    token_t *relational_or_equality_expression_token;
    tokenize_or_return_error(input, position_p, relational_expression,
                             relational_or_equality_expression_token)

    // LEFT_RECURSION
    token_t *operator_token;
    while (token_match_punctuator_clone(input, position_p, PUNCTUATOR_EQUALS,
                                        &operator_token)
           || token_match_punctuator_clone(input, position_p,
                                           PUNCTUATOR_DOES_NOT_EQUAL,
                                           &operator_token)
           || token_match_punctuator_clone(input, position_p,
                                           PUNCTUATOR_STRICT_EQUALS,
                                           &operator_token)
           || token_match_punctuator_clone(input, position_p,
                                           PUNCTUATOR_STRICT_DOES_NOT_EQUAL,
                                           &operator_token)) {

        token_t *equality_expression_token = token_new_no_data(
                TOKEN_EXPRESSION_EQUALITY_EXPRESSION);
        token_add_child(equality_expression_token,
                        relational_or_equality_expression_token);
        token_add_child(equality_expression_token, operator_token);
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, relational_expression, equality_expression_token)
        relational_or_equality_expression_token = equality_expression_token;
    }

    return relational_or_equality_expression_token;
}

#define DEFINE_SIMPLE_EXPRESSION_TOKEN(name, parent_name, operator, id) \
    DEFINE_DELEGATED_TOKEN_IS_FIRST(name, parent_name) \
     \
    token_t *name(GPtrArray *input, gsize *position_p) { \
     \
        token_t *parent_name##_or_##name##_token; \
        tokenize_or_return_error(input, position_p, parent_name, \
                parent_name##_or_##name##_token) \
         \
        while (token_match_punctuator(input, position_p, (operator))) { \
            token_t *name##_token = token_new_no_data((id)); \
            token_add_child(name##_token, parent_name##_or_##name##_token); \
            tokenize_and_add_child_or_free_parent_and_return_error(input, \
                    position_p, (parent_name), name##_token) \
            parent_name##_or_##name##_token = name##_token; \
        } \
         \
        return parent_name##_or_##name##_token; \
    }

/*
 * AST:
 * BitwiseANDExpression = EqualityExpression
 * BitwiseANDExpression - BitwiseANDExpression EqualityExpression
 *
 * GRAMMAR:
 * BitwiseANDExpression :
 *     EqualityExpression
 *     BitwiseANDExpression & EqualityExpression
 */

DEFINE_SIMPLE_EXPRESSION_TOKEN(bitwise_and_expression, equality_expression,
                               PUNCTUATOR_AMPERSAND,
                               TOKEN_EXPRESSION_BITWISE_AND_EXPRESSION)

/*
 * AST:
 * BitwiseXORExpression = BitwiseANDExpression
 * BitwiseXORExpression - BitwiseXORExpression BitwiseANDExpression
 *
 * GRAMMAR:
 * BitwiseXORExpression :
 *     BitwiseANDExpression
 *     BitwiseXORExpression ^ BitwiseANDExpression
 */

DEFINE_SIMPLE_EXPRESSION_TOKEN(bitwise_xor_expression, bitwise_and_expression,
                               PUNCTUATOR_CARET,
                               TOKEN_EXPRESSION_BITWISE_XOR_EXPRESSION)

/*
 * AST:
 * BitwiseORExpression = BitwiseXORExpression
 * BitwiseORExpression - BitwiseORExpression BitwiseXORExpression
 *
 * GRAMMAR:
 * BitwiseORExpression :
 *     BitwiseXORExpression
 *     BitwiseORExpression | BitwiseXORExpression
 */

DEFINE_SIMPLE_EXPRESSION_TOKEN(bitwise_or_expression, bitwise_xor_expression,
                               PUNCTUATOR_VERTICAL_BAR,
                               TOKEN_EXPRESSION_BITWISE_OR_EXPRESSION)

/*
 * AST:
 * LogicalANDExpression = BitwiseORExpression
 * LogicalANDExpression - LogicalANDExpression BitwiseORExpression
 *
 * GRAMMAR:
 * LogicalANDExpression :
 *     BitwiseORExpression
 *     LogicalANDExpression && BitwiseORExpression
 */

DEFINE_SIMPLE_EXPRESSION_TOKEN(logical_and_expression, bitwise_or_expression,
                               PUNCTUATOR_LOGICAL_AND,
                               TOKEN_EXPRESSION_LOGICAL_AND_EXPRESSION)

/*
 * AST:
 * LogicalORExpression = LogicalANDExpression
 * LogicalORExpression - LogicalORExpression LogicalANDExpression
 *
 * GRAMMAR:
 * LogicalORExpression :
 *     LogicalANDExpression
 *     LogicalORExpression || LogicalANDExpression
 */

DEFINE_SIMPLE_EXPRESSION_TOKEN(logical_or_expression, logical_and_expression,
                               PUNCTUATOR_LOGICAL_OR,
                               TOKEN_EXPRESSION_LOGICAL_OR_EXPRESSION)

/*
 * AST:
 * ConditionalExpression = LogicalORExpression
 * ConditionalExpression - LogicalORExpression AssignmentExpression
 *         AssignmentExpression
 *
 * GRAMMAR:
 * ConditionalExpression :
 *     LogicalORExpression
 *     LogicalORExpression ? AssignmentExpression : AssignmentExpression
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(conditional_expression, logical_or_expression)

token_t *conditional_expression(GPtrArray *input, gsize *position_p) {

    token_t *logical_or_expression_token;
    tokenize_or_return_error(input, position_p, logical_or_expression,
                             logical_or_expression_token)

    if (token_match_punctuator(input, position_p, PUNCTUATOR_QUESTION_MARK)) {

        token_t *conditional_expression_token = token_new_no_data(
                TOKEN_EXPRESSION_CONDITIONAL_EXPRESSION);
        token_add_child(conditional_expression_token,
                        logical_or_expression_token);
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, assignment_expression, conditional_expression_token)
        match_punctuator_or_free_and_return_error(input, position_p,
                PUNCTUATOR_COLON, conditional_expression_token,
                ERROR_EXPRESSION_CONDITIONAL_EXPRESSION_COLON)
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, assignment_expression, conditional_expression_token)
        return conditional_expression_token;

    } else {
        return logical_or_expression_token;
    }
}

/*
 * AST:
 * AssignmentExpression = ConditionalExpression
 * AssignmentExpression - LeftHandSideExpression
 *         (=|*=|/=|%=|+=|-=|<<=|>>=|>>>=|&=|^=||=) AssignmentExpression
 *
 * GRAMMAR:
 * AssignmentExpression :
 *     ConditionalExpression
 *     LeftHandSideExpression (=|*=|/=|%=|+=|-=|<<=|>>=|>>>=|&=|^=||=)
 *             AssignmentExpression
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(assignment_expression, conditional_expression)

token_t *assignment_expression(GPtrArray *input, gsize *position_p) {

    token_t *conditional_expression_token;
    tokenize_or_return_error(input, position_p, conditional_expression,
                             conditional_expression_token)

    token_t *operator_token;
    if (token_match_punctuator_clone(input, position_p, PUNCTUATOR_EQUALS_SIGN,
                                     &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_MULTIPLY_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_DIVIDE_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_MODULO_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_ADD_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_SUBTRACT_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_LEFT_SHIFT_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                       PUNCTUATOR_SIGNED_RIGHT_SHIFT_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                     PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_BITWISE_AND_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_BITWISE_XOR_ASSIGNMENT,
                                        &operator_token)
        || token_match_punctuator_clone(input, position_p,
                                        PUNCTUATOR_BITWISE_OR_ASSIGNMENT,
                                        &operator_token)) {

        // NOTE: LeftHandSideExpression not checked here because by the rules in
        // spec, LeftHandSideExpression = NewExpression = MemberExpression
        // = PrimaryExpression = Expression = ... = ConditionalExpression, which
        // means it can always be of type ConditionalExpression.

        token_t *assignment_expression_token = token_new_no_data(
                TOKEN_EXPRESSION_ASSIGNMENT_EXPRESSION);
        token_add_child(assignment_expression_token,
                        conditional_expression_token);
        token_add_child(assignment_expression_token, operator_token);
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, assignment_expression, assignment_expression_token)
        return assignment_expression_token;

    } else {
        return conditional_expression_token;
    }
}

/*
 * AST:
 * Expression = AssignmentExpression
 * Expression - AssignmentExpression{2,}
 *
 * GRAMMAR:
 * Expression:
 *     AssignmentExpression (, AssignmentExpression)*
 */

DEFINE_DELEGATED_TOKEN_IS_FIRST(expression, assignment_expression)

token_t *expression(GPtrArray *input, gsize *position_p) {

    token_t *assignment_expression_token;
    tokenize_or_return_error(input, position_p, assignment_expression,
                             assignment_expression_token)

    if (token_match_punctuator(input, position_p, PUNCTUATOR_COMMA)) {

        token_t *expression_token = token_new_no_data(
                TOKEN_EXPRESSION_EXPRESSION);
        do {
            tokenize_and_add_child_or_free_parent_and_return_error(input,
                    position_p, assignment_expression, expression_token)
        } while (token_match_punctuator(input, position_p, PUNCTUATOR_COMMA));
        return expression_token;

    } else {
        return assignment_expression_token;
    }
}
