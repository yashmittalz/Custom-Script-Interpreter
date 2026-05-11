/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include "lexer.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Represents a single grammar rule (e.g., A -> B C)
struct Rule {
  string LHS;         // Left-hand side (e.g., "A")
  vector<string> RHS; // Right-hand side (e.g., ["B", "C"])
};

// Global variables to hold everything
vector<Rule> grammar;
vector<string> non_terminals; // Keep track of appearance order
vector<string> terminals;     // Keep track of appearance order
set<string> nullable_set;
unordered_map<string, set<string>> first_sets;
unordered_map<string, set<string>> follow_sets;
vector<string> universe; // Order of all symbols (terminals then non-terminals)

// Helper to check if a string is a non-terminal
bool isNonTerminal(string s) {
  for (const string &nt : non_terminals) {
    if (nt == s)
      return true;
  }
  return false;
}

// --- START OF NEW PARSER CODE ---

// Make lexer global so recursive functions can access it
LexicalAnalyzer lexer;
vector<string> all_ids_in_order;
set<string> seen_ids;
set<string> lhs_ids;

// Helper to trigger a syntax error
void syntax_error() {
  cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
  exit(1);
}

// Helper to expect a specific token
Token expect(TokenType expected_type) {
  Token t = lexer.GetToken();
  if (t.token_type != expected_type) {
    syntax_error();
  }
  if (t.token_type == ID) {
    if (seen_ids.find(t.lexeme) == seen_ids.end()) {
      all_ids_in_order.push_back(t.lexeme);
      seen_ids.insert(t.lexeme);
    }
  }
  return t;
}

// Function declarations for Recursive Descent Parsing
void parse_Grammar();
void parse_Rule_list();
void parse_Rule();
void parse_Right_hand_side(string lhs_name);
void parse_Id_list(vector<string> &rhs_symbols);

void parse_Id_list(vector<string> &rhs_symbols) {
  Token t = lexer.peek(1);
  if (t.token_type == ID) {
    Token id = expect(ID);
    rhs_symbols.push_back(id.lexeme);
    parse_Id_list(rhs_symbols);
  } else if (t.token_type == OR || t.token_type == STAR) {
    // Epsilon derivation. FOLLOW(Id-list) = { OR, STAR }
    return;
  } else {
    syntax_error();
  }
}

void parse_Right_hand_side(string lhs_name) {
  vector<string> rhs_symbols;
  parse_Id_list(rhs_symbols);

  // Successfully parsed a RHS, add it to our grammar data structure!
  grammar.push_back({lhs_name, rhs_symbols});

  Token t = lexer.peek(1);
  if (t.token_type == OR) {
    expect(OR);
    parse_Right_hand_side(lhs_name);
  } else if (t.token_type == STAR) {
    // FOLLOW(Right-hand-side) = { STAR }
    return;
  } else {
    syntax_error();
  }
}

void parse_Rule() {
  Token lhs = expect(ID);
  lhs_ids.insert(lhs.lexeme);

  expect(ARROW);
  parse_Right_hand_side(lhs.lexeme);
  expect(STAR);
}

void parse_Rule_list() {
  parse_Rule();
  Token t = lexer.peek(1);
  if (t.token_type == ID) {
    parse_Rule_list();
  } else if (t.token_type == HASH) {
    // FOLLOW(Rule-list) = { HASH }
    return;
  } else {
    syntax_error();
  }
}

void parse_Grammar() {
  parse_Rule_list();
  expect(HASH);

  // Check to make sure there is absolutely no garbage after the HASH
  Token t = lexer.GetToken();
  if (t.token_type != END_OF_FILE) {
    syntax_error();
  }
}

// read grammar
void ReadGrammar() {
  grammar.clear();
  non_terminals.clear();
  terminals.clear();
  all_ids_in_order.clear();
  seen_ids.clear();
  lhs_ids.clear();

  parse_Grammar(); // Start the recursive descent parser

  for (const string &s : all_ids_in_order) {
    if (lhs_ids.find(s) != lhs_ids.end()) {
      non_terminals.push_back(s);
    }
  }
}

// --- END OF NEW PARSER CODE ---

// Helper to identify terminals and populate universe
void Setup() {
  terminals.clear();
  set<string> seen_terminals;
  for (const auto &rule : grammar) {
    for (const string &sym : rule.RHS) {
      bool is_nt = false;
      for (const string &nt : non_terminals) {
        if (nt == sym) {
          is_nt = true;
          break;
        }
      }
      if (!is_nt) {
        if (seen_terminals.find(sym) == seen_terminals.end()) {
          terminals.push_back(sym);
          seen_terminals.insert(sym);
        }
      }
    }
  }
  // Re-filter terminals from all_ids_in_order to preserve appearance order
  vector<string> final_terminals;
  for (const string &s : all_ids_in_order) {
    if (lhs_ids.find(s) == lhs_ids.end()) {
      final_terminals.push_back(s);
    }
  }
  terminals = final_terminals;

  universe = terminals;
  for (const string &nt : non_terminals) {
    universe.push_back(nt);
  }
}

// Task 1: Identification
void Task1() {
  Setup();
  bool first = true;
  for (const string &s : terminals) {
    if (s.empty())
      continue;
    if (!first)
      cout << " ";
    cout << s;
    first = false;
  }
  for (const string &s : non_terminals) {
    if (s.empty())
      continue;
    if (!first)
      cout << " ";
    cout << s;
    first = false;
  }
}

// Helper to calculate nullable set internally
void CalculateNullable() {
  Setup();
  nullable_set.clear();
  bool changed = true;
  while (changed) {
    changed = false;
    for (const auto &rule : grammar) {
      if (nullable_set.find(rule.LHS) == nullable_set.end()) {
        bool all_nullable = true;
        for (const string &sym : rule.RHS) {
          if (nullable_set.find(sym) == nullable_set.end()) {
            all_nullable = false;
            break;
          }
        }
        if (all_nullable) {
          nullable_set.insert(rule.LHS);
          changed = true;
        }
      }
    }
  }
}

// Task 2: Nullable sets
void Task2() {
  CalculateNullable();
  cout << "Nullable = { ";
  bool first = true;
  for (const string &nt : non_terminals) {
    if (nullable_set.count(nt)) {
      if (!first)
        cout << ", ";
      cout << nt;
      first = false;
    }
  }
  if (first)
    cout << " ";
  cout << "}";
}

// Helper to calculate FIRST sets internally
void CalculateFirst() {
  CalculateNullable();
  first_sets.clear();
  for (const string &t : terminals) {
    first_sets[t].insert(t);
  }
  bool changed = true;
  while (changed) {
    changed = false;
    for (const auto &rule : grammar) {
      string lhs = rule.LHS;
      size_t before_size = first_sets[lhs].size();
      for (const string &sym : rule.RHS) {
        for (const string &f : first_sets[sym]) {
          first_sets[lhs].insert(f);
        }
        if (nullable_set.find(sym) == nullable_set.end())
          break;
      }
      if (first_sets[lhs].size() > before_size)
        changed = true;
    }
  }
}

// Task 3: FIRST sets
void Task3() {
  CalculateFirst();
  for (const string &nt : non_terminals) {
    cout << "FIRST(" << nt << ") = { ";
    bool first = true;
    for (const string &u : universe) {
      if (first_sets[nt].find(u) != first_sets[nt].end()) {
        if (!first)
          cout << ", ";
        cout << u;
        first = false;
      }
    }
    if (first)
      cout << "  ";
    cout << " }" << endl;
  }
}

// Task 4: FOLLOW sets
void Task4() {
  CalculateFirst();
  follow_sets.clear();
  if (!non_terminals.empty()) {
    follow_sets[non_terminals[0]].insert("$");
  }
  bool changed = true;
  while (changed) {
    changed = false;
    for (const auto &rule : grammar) {
      string lhs = rule.LHS;
      for (size_t i = 0; i < rule.RHS.size(); ++i) {
        string xi = rule.RHS[i];
        if (!isNonTerminal(xi))
          continue;
        size_t before_size = follow_sets[xi].size();
        bool all_nullable = true;
        for (size_t j = i + 1; j < rule.RHS.size(); ++j) {
          string xj = rule.RHS[j];
          for (const string &f : first_sets[xj]) {
            follow_sets[xi].insert(f);
          }
          if (nullable_set.find(xj) == nullable_set.end()) {
            all_nullable = false;
            break;
          }
        }
        if (all_nullable) {
          for (const string &f : follow_sets[lhs]) {
            follow_sets[xi].insert(f);
          }
        }
        if (follow_sets[xi].size() > before_size)
          changed = true;
      }
    }
  }
  for (const string &nt : non_terminals) {
    cout << "FOLLOW(" << nt << ") = { ";
    bool first = true;
    if (follow_sets[nt].find("$") != follow_sets[nt].end()) {
      cout << "$";
      first = false;
    }
    for (const string &u : universe) {
      if (u == "$")
        continue;
      if (follow_sets[nt].find(u) != follow_sets[nt].end()) {
        if (!first)
          cout << ", ";
        cout << u;
        first = false;
      }
    }
    if (first)
      cout << "  ";
    cout << " }" << endl;
  }
}

// Task 5: Left Factoring
void Task5() {
  vector<Rule> fact_grammar = grammar;
  unordered_map<string, int> new_nt_counts;
  bool overall_changed = true;
  while (overall_changed) {
    overall_changed = false;
    unordered_map<string, vector<int>> groups;
    for (int i = 0; i < (int)fact_grammar.size(); ++i) {
      groups[fact_grammar[i].LHS].push_back(i);
    }
    bool modified = false;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
      string lhs = it->first;
      const vector<int> &indices = it->second;
      if (indices.size() < 2)
        continue;
      vector<string> best_prefix;
      vector<int> sharing;
      for (size_t i = 0; i < indices.size(); ++i) {
        for (size_t j = i + 1; j < indices.size(); ++j) {
          vector<string> p;
          const auto &r1 = fact_grammar[indices[i]].RHS;
          const auto &r2 = fact_grammar[indices[j]].RHS;
          size_t k = 0;
          while (k < r1.size() && k < r2.size() && r1[k] == r2[k]) {
            p.push_back(r1[k]);
            k++;
          }
          if (p.empty())
            continue;
          vector<int> curr_sharing;
          for (int idx : indices) {
            const auto &r = fact_grammar[idx].RHS;
            if (r.size() >= p.size()) {
              bool match = true;
              for (size_t l = 0; l < p.size(); ++l)
                if (r[l] != p[l])
                  match = false;
              if (match)
                curr_sharing.push_back(idx);
            }
          }
          if (p.size() > best_prefix.size() ||
              (p.size() == best_prefix.size() && p < best_prefix)) {
            best_prefix = p;
            sharing = curr_sharing;
          }
        }
      }
      if (!best_prefix.empty()) {
        new_nt_counts[lhs]++;
        string new_nt = lhs + to_string(new_nt_counts[lhs]);
        vector<Rule> next;
        bool added_new = false;
        set<int> sharing_set(sharing.begin(), sharing.end());
        for (int i = 0; i < (int)fact_grammar.size(); ++i) {
          if (sharing_set.find(i) == sharing_set.end())
            next.push_back(fact_grammar[i]);
          else {
            if (!added_new) {
              Rule r;
              r.LHS = lhs;
              r.RHS = best_prefix;
              r.RHS.push_back(new_nt);
              next.push_back(r);
              added_new = true;
            }
            Rule rp;
            rp.LHS = new_nt;
            for (size_t k = best_prefix.size(); k < fact_grammar[i].RHS.size();
                 ++k)
              rp.RHS.push_back(fact_grammar[i].RHS[k]);
            next.push_back(rp);
          }
        }
        fact_grammar = next;
        modified = true;
        overall_changed = true;
        break;
      }
    }
    if (!modified)
      break;
  }
  auto cmp = [](const Rule &a, const Rule &b) {
    if (a.LHS != b.LHS)
      return a.LHS < b.LHS;
    return a.RHS < b.RHS;
  };
  sort(fact_grammar.begin(), fact_grammar.end(), cmp);
  for (const auto &r : fact_grammar) {
    cout << r.LHS << " -> ";
    for (const string &s : r.RHS)
      cout << s << " ";
    cout << "#" << endl;
  }
}

// Task 6: Eliminate Left Recursion
void Task6() {
  Setup();
  vector<Rule> curr = grammar;
  vector<string> nts = non_terminals;
  sort(nts.begin(), nts.end());
  unordered_map<string, int> counts;

  for (int i = 0; i < (int)nts.size(); ++i) {
    string ai = nts[i];
    for (int j = 0; j < i; ++j) {
      string aj = nts[j];
      vector<Rule> next;
      for (const auto &r : curr) {
        if (r.LHS == ai && !r.RHS.empty() && r.RHS[0] == aj) {
          vector<string> gamma(r.RHS.begin() + 1, r.RHS.end());
          for (const auto &ajr : curr) {
            if (ajr.LHS == aj) {
              Rule nr;
              nr.LHS = ai;
              nr.RHS = ajr.RHS;
              nr.RHS.insert(nr.RHS.end(), gamma.begin(), gamma.end());
              next.push_back(nr);
            }
          }
        } else
          next.push_back(r);
      }
      curr = next;
    }
    vector<Rule> alpha, beta, others;
    for (const auto &r : curr) {
      if (r.LHS == ai) {
        if (!r.RHS.empty() && r.RHS[0] == ai) {
          Rule ar;
          ar.LHS = ai;
          ar.RHS.assign(r.RHS.begin() + 1, r.RHS.end());
          alpha.push_back(ar);
        } else
          beta.push_back(r);
      } else
        others.push_back(r);
    }
    if (!alpha.empty()) {
      counts[ai]++;
      string ap = ai + to_string(counts[ai]);
      vector<Rule> nar;
      for (const auto &br : beta) {
        Rule r;
        r.LHS = ai;
        r.RHS = br.RHS;
        r.RHS.push_back(ap);
        nar.push_back(r);
      }
      for (const auto &ar : alpha) {
        Rule r;
        r.LHS = ap;
        r.RHS = ar.RHS;
        r.RHS.push_back(ap);
        nar.push_back(r);
      }
      // Always add epsilon for the new non-terminal
      Rule er;
      er.LHS = ap;
      nar.push_back(er);
      curr = others;
      curr.insert(curr.end(), nar.begin(), nar.end());
    }
  }
  auto cmp = [](const Rule &a, const Rule &b) {
    if (a.LHS != b.LHS)
      return a.LHS < b.LHS;
    return a.RHS < b.RHS;
  };
  sort(curr.begin(), curr.end(), cmp);
  for (const auto &r : curr) {
    cout << r.LHS << " -> ";
    for (const string &s : r.RHS)
      cout << s << " ";
    cout << "#" << endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 1;
  int task = atoi(argv[1]);
  ReadGrammar();
  switch (task) {
  case 1:
    Task1();
    break;
  case 2:
    Task2();
    break;
  case 3:
    Task3();
    break;
  case 4:
    Task4();
    break;
  case 5:
    Task5();
    break;
  case 6:
    Task6();
    break;
  }
  return 0;
}
