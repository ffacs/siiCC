#pragma once

#include "LALR_common.h"

typedef std::map<ClosurePtr, std::map<TokenPtr, ClosurePtr>> ActionType;
typedef std::map<ClosurePtr, std::map<TokenPtr, ProductionPtr>> ReduceType;

class LALRParserGenerator {
public:
  LALRParserGenerator(ActionType &&action, ReduceType &&reduce, std::vector<ClosurePtr>&& closures, GrammarPtr grammar);
  
  void OutputHeader(std::ostream& os);
  void OutputCpp(const std::string& header_name, std::ostream& os);

private:

  GrammarPtr grammar_;
  ActionType action_;
  ReduceType reduce_;
  std::vector<ClosurePtr> closures_;
};