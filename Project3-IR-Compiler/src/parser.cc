#include "compiler.h"
#include "lexer.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

// Global lexer object
LexicalAnalyzer lexer;

map<string, int> location_table;
int next_available_location = 0;

// Helper function to map a variable name to an index in the mem[] array
int getLocation(string name) {
  if (location_table.find(name) == location_table.end()) {
    location_table[name] = next_available_location;
    next_available_location++;
  }
  return location_table[name];
}

// Helper function to map a hardcoded number to an index in the mem[] array
int getNumLocation(string num_str) {
  if (location_table.find(num_str) == location_table.end()) {
    location_table[num_str] = next_available_location;
    mem[next_available_location] = stoi(num_str);
    next_available_location++;
  }
  return location_table[num_str];
}

// Function declarations so C++ knows they exist
void parse_var_section();
struct InstructionNode *parse_body();
void parse_inputs();
struct InstructionNode *parse_stmt();
struct InstructionNode *parse_assign_stmt();
struct InstructionNode *parse_for();
struct InstructionNode *parse_switch();

// --- THE ENTRY POINT ---
// The instructor's main() calls this function.
// You must build the linked list here and return the first node.
struct InstructionNode *parse_generate_intermediate_representation() {
  parse_var_section();
  struct InstructionNode *program_head = parse_body();
  parse_inputs();

  return program_head;
}

// Read variables from the top
void parse_var_section() {
  Token t = lexer.GetToken();
  while (t.token_type != SEMICOLON && t.token_type != END_OF_FILE) {
    if (t.token_type == ID) {
      getLocation(t.lexeme);
    }
    t = lexer.GetToken();
  }
}

// Read inputs from the bottom
void parse_inputs() {
  Token t = lexer.GetToken();
  while (t.token_type != END_OF_FILE) {
    if (t.token_type == NUM) {
      inputs.push_back(stoi(t.lexeme));
    }
    t = lexer.GetToken();
  }
}

struct InstructionNode *parse_assign_stmt() {
  Token t = lexer.GetToken(); // Should be ID
  struct InstructionNode *inst = new InstructionNode;
  inst->type = ASSIGN;
  inst->assign_inst.left_hand_side_index = getLocation(t.lexeme);

  lexer.GetToken(); // Consume EQUAL

  Token op1 = lexer.GetToken();
  inst->assign_inst.operand1_index = (op1.token_type == ID)
                                         ? getLocation(op1.lexeme)
                                         : getNumLocation(op1.lexeme);

  Token op_bin = lexer.GetToken();
  if (op_bin.token_type == SEMICOLON) {
    inst->assign_inst.op = OPERATOR_NONE;
  } else {
    if (op_bin.token_type == PLUS)
      inst->assign_inst.op = OPERATOR_PLUS;
    else if (op_bin.token_type == MINUS)
      inst->assign_inst.op = OPERATOR_MINUS;
    else if (op_bin.token_type == MULT)
      inst->assign_inst.op = OPERATOR_MULT;
    else if (op_bin.token_type == DIV)
      inst->assign_inst.op = OPERATOR_DIV;

    Token op2 = lexer.GetToken();
    inst->assign_inst.operand2_index = (op2.token_type == ID)
                                           ? getLocation(op2.lexeme)
                                           : getNumLocation(op2.lexeme);
    lexer.GetToken(); // Consume SEMICOLON
  }
  inst->next = nullptr;
  return inst;
}

struct InstructionNode *parse_for() {
  lexer.GetToken(); // Consume FOR
  lexer.GetToken(); // Consume LPAREN

  struct InstructionNode *assign_1 = parse_assign_stmt();

  struct InstructionNode *cjmp = new InstructionNode;
  cjmp->type = CJMP;

  Token op1 = lexer.GetToken();
  cjmp->cjmp_inst.operand1_index = (op1.token_type == ID)
                                       ? getLocation(op1.lexeme)
                                       : getNumLocation(op1.lexeme);

  Token cond = lexer.GetToken();
  if (cond.token_type == GREATER)
    cjmp->cjmp_inst.condition_op = CONDITION_GREATER;
  else if (cond.token_type == LESS)
    cjmp->cjmp_inst.condition_op = CONDITION_LESS;
  else if (cond.token_type == NOTEQUAL)
    cjmp->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

  Token op2 = lexer.GetToken();
  cjmp->cjmp_inst.operand2_index = (op2.token_type == ID)
                                       ? getLocation(op2.lexeme)
                                       : getNumLocation(op2.lexeme);

  lexer.GetToken(); // Consume SEMICOLON

  struct InstructionNode *assign_2 = parse_assign_stmt();

  lexer.GetToken(); // Consume RPAREN

  struct InstructionNode *body = parse_body();

  struct InstructionNode *a1_end = assign_1;
  while (a1_end->next != nullptr)
    a1_end = a1_end->next;
  a1_end->next = cjmp;

  cjmp->next = body;

  struct InstructionNode *curr = body;
  if (curr == nullptr) {
    cjmp->next = assign_2;
  } else {
    while (curr->next != nullptr)
      curr = curr->next;
    curr->next = assign_2;
  }

  struct InstructionNode *a2_end = assign_2;
  while (a2_end->next != nullptr)
    a2_end = a2_end->next;

  struct InstructionNode *jmp = new InstructionNode;
  jmp->type = JMP;
  jmp->jmp_inst.target = cjmp;

  struct InstructionNode *noop = new InstructionNode;
  noop->type = NOOP;
  noop->next = nullptr;

  jmp->next = noop;
  a2_end->next = jmp;

  cjmp->cjmp_inst.target = noop;

  return assign_1;
}

struct InstructionNode *parse_switch() {
  lexer.GetToken();                   // Parse SWITCH
  Token var_token = lexer.GetToken(); // Parse ID
  int switch_var_loc = getLocation(var_token.lexeme);

  lexer.GetToken(); // Parse LBRACE

  struct InstructionNode *end_switch_node = new InstructionNode;
  end_switch_node->type = NOOP;
  end_switch_node->next = nullptr;

  struct InstructionNode *first_cjmp = nullptr;
  struct InstructionNode *prev_cjmp = nullptr;

  while (lexer.peek(1).token_type == CASE) {
    lexer.GetToken();                 // Consume CASE
    Token num_tok = lexer.GetToken(); // Consume NUM
    lexer.GetToken();                 // Consume COLON

    struct InstructionNode *body = parse_body();

    struct InstructionNode *jmp_to_end = new InstructionNode;
    jmp_to_end->type = JMP;
    jmp_to_end->jmp_inst.target = end_switch_node;
    jmp_to_end->next = end_switch_node;

    struct InstructionNode *curr = body;
    if (curr == nullptr) {
      body = jmp_to_end;
    } else {
      while (curr->next != nullptr) {
        curr = curr->next;
      }
      curr->next = jmp_to_end;
    }

    struct InstructionNode *cjmp = new InstructionNode;
    cjmp->type = CJMP;
    cjmp->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    cjmp->cjmp_inst.operand1_index = switch_var_loc;
    cjmp->cjmp_inst.operand2_index = getNumLocation(num_tok.lexeme);
    cjmp->cjmp_inst.target = body;
    cjmp->next = nullptr;

    if (first_cjmp == nullptr) {
      first_cjmp = cjmp;
    }
    if (prev_cjmp != nullptr) {
      prev_cjmp->next = cjmp;
    }
    prev_cjmp = cjmp;
  }

  if (lexer.peek(1).token_type == DEFAULT) {
    lexer.GetToken(); // Consume DEFAULT
    lexer.GetToken(); // Consume COLON

    struct InstructionNode *def_body = parse_body();

    struct InstructionNode *curr = def_body;
    if (curr == nullptr) {
      def_body = end_switch_node;
    } else {
      while (curr->next != nullptr)
        curr = curr->next;
      curr->next = end_switch_node;
    }

    if (prev_cjmp != nullptr) {
      prev_cjmp->next = def_body;
    } else {
      first_cjmp = def_body;
    }
  } else {
    if (prev_cjmp != nullptr) {
      prev_cjmp->next = end_switch_node;
    } else {
      first_cjmp = end_switch_node;
    }
  }

  lexer.GetToken(); // Parse RBRACE
  return first_cjmp;
}

struct InstructionNode *parse_stmt() {
  Token t = lexer.GetToken();

  if (t.token_type == OUTPUT) {
    struct InstructionNode *inst = new InstructionNode;
    inst->type = OUT;
    Token var_token = lexer.GetToken();
    inst->output_inst.var_index = getLocation(var_token.lexeme);
    lexer.GetToken(); // Consume SEMICOLON
    inst->next = nullptr;
    return inst;
  }

  if (t.token_type == INPUT) {
    struct InstructionNode *inst = new InstructionNode;
    inst->type = IN;
    Token var_token = lexer.GetToken();
    inst->input_inst.var_index = getLocation(var_token.lexeme);
    lexer.GetToken(); // Consume SEMICOLON
    inst->next = nullptr;
    return inst;
  }

  if (t.token_type == ID) {
    lexer.UngetToken(1);
    return parse_assign_stmt();
  }

  if (t.token_type == FOR) {
    lexer.UngetToken(1);
    return parse_for();
  }

  if (t.token_type == SWITCH) {
    lexer.UngetToken(1);
    return parse_switch();
  }

  if (t.token_type == IF) {
    struct InstructionNode *inst = new InstructionNode;
    inst->type = CJMP;

    Token op1 = lexer.GetToken();
    inst->cjmp_inst.operand1_index = (op1.token_type == ID)
                                         ? getLocation(op1.lexeme)
                                         : getNumLocation(op1.lexeme);

    Token cond = lexer.GetToken();
    if (cond.token_type == GREATER)
      inst->cjmp_inst.condition_op = CONDITION_GREATER;
    else if (cond.token_type == LESS)
      inst->cjmp_inst.condition_op = CONDITION_LESS;
    else if (cond.token_type == NOTEQUAL)
      inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    Token op2 = lexer.GetToken();
    inst->cjmp_inst.operand2_index = (op2.token_type == ID)
                                         ? getLocation(op2.lexeme)
                                         : getNumLocation(op2.lexeme);

    struct InstructionNode *body = parse_body();
    inst->next = body;

    struct InstructionNode *noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = nullptr;

    inst->cjmp_inst.target = noop;

    struct InstructionNode *curr = body;
    if (curr == nullptr) {
      inst->next = noop;
    } else {
      while (curr->next != nullptr) {
        curr = curr->next;
      }
      curr->next = noop;
    }

    return inst;
  }

  if (t.token_type == WHILE) {
    struct InstructionNode *inst = new InstructionNode;
    inst->type = CJMP;

    Token op1 = lexer.GetToken();
    inst->cjmp_inst.operand1_index = (op1.token_type == ID)
                                         ? getLocation(op1.lexeme)
                                         : getNumLocation(op1.lexeme);

    Token cond = lexer.GetToken();
    if (cond.token_type == GREATER)
      inst->cjmp_inst.condition_op = CONDITION_GREATER;
    else if (cond.token_type == LESS)
      inst->cjmp_inst.condition_op = CONDITION_LESS;
    else if (cond.token_type == NOTEQUAL)
      inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    Token op2 = lexer.GetToken();
    inst->cjmp_inst.operand2_index = (op2.token_type == ID)
                                         ? getLocation(op2.lexeme)
                                         : getNumLocation(op2.lexeme);

    struct InstructionNode *body = parse_body();
    inst->next = body;

    struct InstructionNode *noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = nullptr;

    inst->cjmp_inst.target = noop;

    struct InstructionNode *jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->jmp_inst.target = inst;
    jmp->next = noop;

    struct InstructionNode *curr = body;
    if (curr == nullptr) {
      inst->next = jmp;
    } else {
      while (curr->next != nullptr) {
        curr = curr->next;
      }
      curr->next = jmp;
    }

    return inst;
  }

  return nullptr;
}

struct InstructionNode *parse_body() {
  struct InstructionNode *head = nullptr;
  struct InstructionNode *tail = nullptr;

  Token t = lexer.GetToken(); // Consume LBRACE

  Token next_t = lexer.peek(1);
  while (next_t.token_type != RBRACE && next_t.token_type != END_OF_FILE) {
    struct InstructionNode *stmt = parse_stmt();
    if (stmt == nullptr)
      break;

    if (head == nullptr) {
      head = stmt;
      tail = stmt;
    } else {
      tail->next = stmt;
      tail = stmt;
    }
    while (tail->next != nullptr) {
      tail = tail->next;
    }
    next_t = lexer.peek(1);
  }

  lexer.GetToken(); // Consume RBRACE
  return head;
}
