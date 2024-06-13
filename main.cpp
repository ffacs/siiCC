#include "LALR_table_generator.h"

int main() {
  Grammar simple;

  auto b_blank = std::make_shared<Token>(Token::Type::BLANK, "blank");

  auto t_star = std::make_shared<Token>(Token::Type::Terminator, "*");
  auto t_id = std::make_shared<Token>(Token::Type::Terminator, "id");
  auto t_equal = std::make_shared<Token>(Token::Type::Terminator, "=");

  auto n_S =
      std::make_shared<Token>(Token::Type::Nonterminator, "S");
  auto n_L =
      std::make_shared<Token>(Token::Type::Nonterminator, "L");
  auto n_R = std::make_shared<Token>(Token::Type::Nonterminator, "R");

  auto e_ed = std::make_shared<Token>(Token::Type::END, "$");

  simple.start_ = n_S;
  simple.end_ = e_ed;

  simple.nonterminators_ = {n_S, n_L, n_R};
  simple.terminators_ = {t_star, t_id, t_equal, e_ed};

  simple.blank_ = b_blank;

  ProductionPtrVec productions = {
      std::make_shared<Production>(n_S, TokenPtrVec{n_L, t_equal, n_R}),
      std::make_shared<Production>(n_S, TokenPtrVec{n_R}),
      std::make_shared<Production>(n_L, TokenPtrVec{t_star, n_R}),
      std::make_shared<Production>(n_L, TokenPtrVec{t_id}),
      std::make_shared<Production>(n_R, TokenPtrVec{n_L}),
  };

  simple.productions_ = productions;

  LALRTableGenerator generator(simple);
  generator.GenerateLALRTable();
  generator.PrintLALRTable();
}