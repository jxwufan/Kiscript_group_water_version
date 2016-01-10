# KiScript

ECMAScript interpreter, written purely in C, kept simple.

## Specification

Based on ECMAScript 5.0, with reference to ECMAScript 6.0, and some customization.

## Grammar Variation

- UTF-8 instead of UTF-16.
- No `/regex/` literal.
- No `Keyword` as `IdentifierName`.
- No `.1` or `1.`.
- `\0` is always `NUL` in `StringLiteral`.
- No automatic semicolon insertion, so no restriction on no line terminator.
- No `in`.
- For each for `iterable` with `for ( : )`.
- No `EmptyStatement`.
- `Block` instead of `Statement` for `if` / `for` / `while` / `do while` body.
