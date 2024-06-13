#pragma once

#include "LALR_common.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <optional>
#include <queue>

class LALRTableGenerator {
public:
  LALRTableGenerator(const Grammar &grammar);

  void GenerateLALRTable();

  void PrintLALRTable();

private:
  TokenPtrVec GetNextTokens(const ClosurePtr &closure);

  std::pair<bool, ClosurePtr> GetNextClosureFor(const TokenPtr &token,
                                                const ClosurePtr &closure);

  std::pair<bool, ClosurePtr> GetClosure(KernelItemVec &&kernel_items);

  const TokenPtrSet &getFollowOf(const TokenPtr &token);

  const TokenPtrSet &GetFirstOf(const TokenPtr &token);

private:
  std::vector<ClosurePtr> closures_;
  Grammar grammar_;
  std::map<TokenPtr, TokenPtrSet> follow_of_;
  std::map<TokenPtr, TokenPtrSet> first_of_;
  std::map<ClosurePtr, std::map<TokenPtr, ClosurePtr>> action_;
  std::map<ClosurePtr, std::map<TokenPtr, ProductionPtr>> reduce_;
};