/*
 * Copyright (C) Rida Bazzi
 *
 * Do not share this file with anyone
 */
#ifndef __LEXER__H__
#define __LEXER__H__

#include <string>
#include <vector>

#include "inputbuf.h"

// ------- token types -------------------

typedef enum {
  END_OF_FILE = 0,
  IF,
  WHILE,
  DO,
  THEN,
  PRINT,
  PLUS,
  MINUS,
  DIV,
  MULT,
  EQUAL,
  COLON,
  COMMA,
  SEMICOLON,
  LBRAC,
  RBRAC,
  LPAREN,
  RPAREN,
  NOTEQUAL,
  GREATER,
  LESS,
  LTEQ,
  GTEQ,
  DOT,
  NUM,
  ID,
  REALNUM,
  BASE08NUM,
  BASE16NUM,
  ERROR
} TokenType;

class Token {
public:
  void Print();

  std::string lexeme;
  TokenType token_type;
  int line_no;
};

class LexicalAnalyzer {
public:
  Token GetToken();
  TokenType UngetToken(Token);
  LexicalAnalyzer();

private:
  std::vector<Token> tokens;
  int line_no;
  Token tmp;
  InputBuffer input;

  bool SkipSpace();
  bool IsKeyword(std::string);
  TokenType FindKeywordIndex(std::string);
  Token ScanIdOrKeyword();
  Token ScanNumber();
};

#endif //__LEXER__H__
