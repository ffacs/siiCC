#include "LALR_table_generator.h"
#include "LALR_parser_generator.h"
#include <fstream>

int main() {
  Grammar simple;

  auto b_blank = NewTerminator(Token::Type::BLANK, "blank", "blank");

  auto t_star = NewTerminator(Token::Type::Terminator, "star", "*");
  auto t_id = NewTerminator(Token::Type::Terminator, "id", "id");
  auto t_equal = NewTerminator(Token::Type::Terminator, "equal", "=");
  auto e_ed = NewTerminator(Token::Type::Terminator, "end", "$");

  auto n_S =
    NewNonTerminator(Token::Type::Nonterminator, "S");
  auto n_L =
    NewNonTerminator(Token::Type::Nonterminator, "L");
  auto n_R = NewNonTerminator(Token::Type::Nonterminator, "R");

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

  LALRTableGenerator t_generator(simple);
  t_generator.GenerateLALRTable();
//  t_generator.PrintLALRTable();

  LALRParserGenerator p_generator(
      t_generator.MoveAction(), t_generator.MoveReduce(),
      t_generator.MoveClosures(), t_generator.MoveGrammar());
  
  std::string header_name = "siicc_header.h";
  std::string cpp_name = "siicc_cpp.cpp";
  std::ofstream header_file(header_name);
  std::ofstream cpp_file(cpp_name);

  p_generator.OutputHeader(header_file);
  p_generator.OutputCpp(header_name, cpp_file);
}