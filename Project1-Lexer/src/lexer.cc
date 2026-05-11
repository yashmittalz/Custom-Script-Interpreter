/*
 * Copyright (C) Rida Bazzi
 *
 * Do not share this file with anyone
 */
#include <cctype>
#include <iostream>
#include <istream>
#include <string>
#include <vector>

#include "inputbuf.h"
#include "lexer.h"

using namespace std;

string reserved[] = {
    "END_OF_FILE", "IF",        "WHILE",   "DO",        "THEN",      "PRINT",
    "PLUS",        "MINUS",     "DIV",     "MULT",      "EQUAL",     "COLON",
    "COMMA",       "SEMICOLON", "LBRAC",   "RBRAC",     "LPAREN",    "RPAREN",
    "NOTEQUAL",    "GREATER",   "LESS",    "LTEQ",      "GTEQ",      "DOT",
    "NUM",         "ID",        "REALNUM", "BASE08NUM", "BASE16NUM", "ERROR"};

#define KEYWORDS_COUNT 5
string keyword[] = {"IF", "WHILE", "DO", "THEN", "PRINT"};

void Token::Print() {
  cout << "{" << this->lexeme << " , " << reserved[(int)this->token_type]
       << " , " << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer() {
  this->line_no = 1;
  tmp.lexeme = "";
  tmp.line_no = 1;
  tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace() {
  char c;
  bool space_encountered = false;

  input.GetChar(c);
  line_no += (c == '\n');

  while (!input.EndOfInput() && isspace(c)) {
    space_encountered = true;
    input.GetChar(c);
    line_no += (c == '\n');
  }

  if (!input.EndOfInput()) {
    input.UngetChar(c);
  }
  return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s) {
  for (int i = 0; i < KEYWORDS_COUNT; i++) {
    if (s == keyword[i]) {
      return true;
    }
  }
  return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s) {
  for (int i = 0; i < KEYWORDS_COUNT; i++) {
    if (s == keyword[i]) {
      return (TokenType)(i + 1);
    }
  }
  return ERROR;
}

Token LexicalAnalyzer::ScanNumber() {
  char c;
  string lex = "";
  input.GetChar(c);

  // 1. Consume the NUM prefix: ((pdigit digit*) + 0)
  if (isdigit(c)) {
    if (c == '0') {
      lex = "0";
    } else {
      while (!input.EndOfInput() && isdigit(c)) {
        lex += c;
        input.GetChar(c);
      }
      if (!input.EndOfInput())
        input.UngetChar(c);
    }
  }

  // 2. Peek ahead for BASE or REALNUM
  char c1 = ' ', c2 = ' ', c3 = ' ';
  input.GetChar(c1);

  // --- Check for BASE ---
  if (c1 == 'x') {
    input.GetChar(c2);
    input.GetChar(c3);
    if ((c2 == '0' && c3 == '8') ||
        (c2 == '1' && c3 == '1' && false /*placeholder*/)) {
      // Logic for x08
      bool allEight = true;
      for (char b : lex)
        if (b < '0' || b > '7')
          allEight = false;
      if (allEight && c2 == '0' && c3 == '8') {
        tmp.lexeme = lex + "x08";
        tmp.token_type = BASE08NUM;
        return tmp;
      }
    } else if (c2 == '1' && c3 == '6') {
      // Logic for x16
      tmp.lexeme = lex + "x16";
      tmp.token_type = BASE16NUM;
      return tmp;
    }
    // Not a valid base, put them back in REVERSE order
    input.UngetChar(c3);
    input.UngetChar(c2);
    input.UngetChar(c1);
  }
  // --- Check for REALNUM ---
  else if (c1 == '.') {
    char next;
    input.GetChar(next);
    if (isdigit(next)) {
      lex += ".";
      while (!input.EndOfInput() && isdigit(next)) {
        lex += next;
        input.GetChar(next);
      }
      if (!input.EndOfInput())
        input.UngetChar(next);
      tmp.lexeme = lex;
      tmp.token_type = REALNUM;
      return tmp;
    }
    input.UngetChar(next);
    input.UngetChar(c1);
  } else {
    input.UngetChar(c1);
  }

  // 3. Fallback to NUM
  tmp.lexeme = lex;
  tmp.token_type = NUM;
  return tmp;
}

Token LexicalAnalyzer::ScanIdOrKeyword() {
  char c;
  input.GetChar(c);

  if (isalpha(c)) {
    tmp.lexeme = "";
    while (!input.EndOfInput() && isalnum(c)) {
      tmp.lexeme += c;
      input.GetChar(c);
    }
    if (!input.EndOfInput()) {
      input.UngetChar(c);
    }
    tmp.line_no = line_no;

    // --- NEW LOGIC FOR BASE16NUM STARTING WITH A-F ---
    // Check if the lexeme ends in "x16"
    if (tmp.lexeme.length() > 3 &&
        tmp.lexeme.substr(tmp.lexeme.length() - 3) == "x16") {

      // The prefix (everything before x16) must be valid pdigit16 + digit16*
      string prefix = tmp.lexeme.substr(0, tmp.lexeme.length() - 3);
      bool validBase16 = true;

      // Check if prefix matches ((pdigit16 digit16*) + 0)
      if (prefix == "0") {
        validBase16 = true;
      } else {
        // pdigit16 check (starts with 1-9 or A-F)
        if (!((prefix[0] >= '1' && prefix[0] <= '9') ||
              (prefix[0] >= 'A' && prefix[0] <= 'F'))) {
          validBase16 = false;
        } else {
          // digit16 check (rest are 0-9 or A-F)
          for (size_t i = 1; i < prefix.length(); i++) {
            if (!isxdigit(prefix[i])) { // isxdigit checks 0-9, a-f, A-F
              validBase16 = false;
              break;
            }
          }
        }
      }

      if (validBase16) {
        tmp.token_type = BASE16NUM;
        return tmp;
      }
    }

    if (IsKeyword(tmp.lexeme))
      tmp.token_type = FindKeywordIndex(tmp.lexeme);
    else
      tmp.token_type = ID;
  } else {
    if (!input.EndOfInput()) {
      input.UngetChar(c);
    }
    tmp.lexeme = "";
    tmp.token_type = ERROR;
  }
  return tmp;
}

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok) {
  tokens.push_back(tok);
  ;
  return tok.token_type;
}

Token LexicalAnalyzer::GetToken() {
  char c;

  // if there are tokens that were previously
  // stored due to UngetToken(), pop a token and
  // return it without reading from input
  if (!tokens.empty()) {
    tmp = tokens.back();
    tokens.pop_back();
    return tmp;
  }

  SkipSpace();
  tmp.lexeme = "";
  tmp.line_no = line_no;
  input.GetChar(c);
  switch (c) {
  case '.':
    tmp.token_type = DOT;
    return tmp;
  case '+':
    tmp.token_type = PLUS;
    return tmp;
  case '-':
    tmp.token_type = MINUS;
    return tmp;
  case '/':
    tmp.token_type = DIV;
    return tmp;
  case '*':
    tmp.token_type = MULT;
    return tmp;
  case '=':
    tmp.token_type = EQUAL;
    return tmp;
  case ':':
    tmp.token_type = COLON;
    return tmp;
  case ',':
    tmp.token_type = COMMA;
    return tmp;
  case ';':
    tmp.token_type = SEMICOLON;
    return tmp;
  case '[':
    tmp.token_type = LBRAC;
    return tmp;
  case ']':
    tmp.token_type = RBRAC;
    return tmp;
  case '(':
    tmp.token_type = LPAREN;
    return tmp;
  case ')':
    tmp.token_type = RPAREN;
    return tmp;
  case '<':
    input.GetChar(c);
    if (c == '=') {
      tmp.token_type = LTEQ;
    } else if (c == '>') {
      tmp.token_type = NOTEQUAL;
    } else {
      if (!input.EndOfInput()) {
        input.UngetChar(c);
      }
      tmp.token_type = LESS;
    }
    return tmp;
  case '>':
    input.GetChar(c);
    if (c == '=') {
      tmp.token_type = GTEQ;
    } else {
      if (!input.EndOfInput()) {
        input.UngetChar(c);
      }
      tmp.token_type = GREATER;
    }
    return tmp;
  default:
    if (isdigit(c)) {
      string full_lexeme = "";
      full_lexeme += c;
      char next;

      // Peek ahead to see if this is a BASE16NUM containing letters
      // like 1AB1x16. We collect all alphanumeric chars first.
      while (!input.EndOfInput()) {
        input.GetChar(next);
        if (isalnum(next)) {
          full_lexeme += next;
        } else {
          input.UngetChar(next);
          break;
        }
      }

      // CASE 1: Is the combined string a valid BASE16NUM?
      bool validBase16 = false;
      if (full_lexeme.length() > 3 &&
          full_lexeme.substr(full_lexeme.length() - 3) == "x16") {
        string prefix = full_lexeme.substr(0, full_lexeme.length() - 3);
        validBase16 = true;
        if (prefix == "0") {
          validBase16 = true;
        } else {
          // Check pdigit16 (start)
          // We know prefix[0] is digit because 'c' was isdigit.
          // But if it is '0' and length > 1 (e.g. 01x16), that's invalid.
          if (prefix[0] == '0') {
            validBase16 = false;
          } else {
            // digit16 check (rest are 0-9 or A-F)
            for (size_t i = 0; i < prefix.length(); i++) {
              if (!isxdigit(prefix[i])) {
                validBase16 = false;
                break;
              }
            }
          }
        }
      }

      if (validBase16) {
        tmp.lexeme = full_lexeme;
        tmp.token_type = BASE16NUM;
        return tmp;
      }

      // CASE 2: If it's not a BASE16NUM, put the extra chars back
      // and let the original ScanNumber handle it as a NUM or REALNUM.
      if (full_lexeme.length() > 1) {
        input.UngetString(full_lexeme.substr(1));
      }
      input.UngetChar(c);
      return ScanNumber();
    } else if (isalpha(c)) {
      input.UngetChar(c);
      return ScanIdOrKeyword();
    } else if (input.EndOfInput())
      tmp.token_type = END_OF_FILE;
    else
      tmp.token_type = ERROR;

    return tmp;
  }
}

int main() {
  LexicalAnalyzer lexer;
  Token token;

  token = lexer.GetToken();
  token.Print();
  while (token.token_type != END_OF_FILE) {
    token = lexer.GetToken();
    token.Print();
  }
}
