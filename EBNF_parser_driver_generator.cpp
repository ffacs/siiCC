#include "LALR_table_generator.h"
#include "LALR_parser_generator.h"
#include <fstream>

using namespace siicc::LALR;

int main() {
  Grammar BNF;
  
  auto b_blank = NewBlank();
  auto t_terminator = NewTerminator("terminator", "terminator");
  auto t_nonterminator = NewTerminator("nonterminator", "nonterminator");
  auto t_nonterminator_repeated = NewTerminator("nonterminator_repreated", "nonterminator_repeated");
  auto t_nonterminator_optional = NewTerminator("nonterminator_optional", "nonterminator_optional");
  auto t_nonterminator_one_more = NewTerminator("nonterminator_one_more", "nonterminator_one_more");
  auto t_or = NewTerminator("or", "|");  
  auto t_equals = NewTerminator("equals", "::=");
  auto t_semicolon = NewTerminator("semicolon", ";");
  auto e_ed = NewTerminator("end", "$");

  auto n_productions = NewNonTerminator("Productions");
  auto n_production = NewNonTerminator("Production");
  auto n_bodies = NewNonTerminator("Production_Bodies");
  auto n_body = NewNonTerminator("Production_Body");
  auto n_items = NewNonTerminator("Production_Items");
  auto n_item = NewNonTerminator("Production_Item");
  
  BNF.start_ = n_productions;
  BNF.end_ = e_ed;

  BNF.nonterminators_ = {
    n_productions, n_production, n_bodies, n_body, n_items, n_item
  };
  
  BNF.terminators_ = {
    t_terminator, t_nonterminator, t_nonterminator_one_more, t_nonterminator_optional, t_nonterminator_repeated, t_or, t_semicolon, t_equals, e_ed, b_blank
  };
  
  BNF.blank_ = b_blank;
  BNF.productions_ = {
      std::make_shared<Production>(n_productions, TokenPtrVec{n_production}),
      std::make_shared<Production>(n_productions, TokenPtrVec{n_production, n_productions}),

      std::make_shared<Production>(n_production, TokenPtrVec{t_nonterminator, t_equals, n_bodies, t_semicolon}),

      std::make_shared<Production>(n_bodies, TokenPtrVec{n_body}),
      std::make_shared<Production>(n_bodies, TokenPtrVec{n_body, t_or, n_bodies}),

      std::make_shared<Production>(n_body, TokenPtrVec{n_items}),

      std::make_shared<Production>(n_items, TokenPtrVec{n_item}),
      std::make_shared<Production>(n_items, TokenPtrVec{n_item, n_items}),
      
      std::make_shared<Production>(n_item, TokenPtrVec{t_terminator}),
      std::make_shared<Production>(n_item, TokenPtrVec{t_nonterminator}),
      std::make_shared<Production>(n_item, TokenPtrVec{t_nonterminator_one_more}),
      std::make_shared<Production>(n_item, TokenPtrVec{t_nonterminator_repeated}),
      std::make_shared<Production>(n_item, TokenPtrVec{t_nonterminator_optional}),
  };

  LALRTableGenerator t_generator(BNF);
  t_generator.GenerateLALRTable();

  LALRParserGenerator p_generator(
      t_generator.MoveAction(), t_generator.MoveReduce(),
      t_generator.MoveClosures(), t_generator.MoveGrammar());
  
  std::string header_name = "siicc_EBNF.h";
  std::string cpp_name = "siicc_EBNF.cpp";
  std::ofstream header_file(header_name);
  std::ofstream cpp_file(cpp_name);

  p_generator.OutputHeader(header_file);
  p_generator.OutputCpp(header_name, cpp_file);
}