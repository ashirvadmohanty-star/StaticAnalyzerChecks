//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AITwoTwoCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AITwoTwoCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
    cStyleCastExpr(unless(hasDestinationType(voidType()))).bind("c-cast"),
    this);

  Finder->addMatcher(
    cxxFunctionalCastExpr().bind("func-cast"),
    this);
}

void AITwoTwoCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto *MatchedCCast = Result.Nodes.getNodeAs<CStyleCastExpr>("c-cast")) {
    diag(MatchedCCast->getBeginLoc(), "C-style casts shall not be used");
    return;
  }

  if (const auto *MatchedFuncCast = Result.Nodes.getNodeAs<CXXFunctionalCastExpr>("func-cast")) {
    if (MatchedFuncCast->getCastKind() == CK_ConstructorConversion)
        return;
    if (MatchedFuncCast->isListInitialization())
        return;

    diag(MatchedFuncCast->getBeginLoc(), "Functional notation casts shall not be used");
    return;
  }
}
} // namespace clang::tidy::hsc
