//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ASOneOneCheck.h"
#include "clang/AST/Expr.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void ASOneOneCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxThrowExpr().bind("throwExpr"),
      this);
}

void ASOneOneCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Throw = Result.Nodes.getNodeAs<CXXThrowExpr>("throwExpr");
  if (!Throw)
    return;

  const Expr *SubExpr = Throw->getSubExpr();

  // Skip rethrow (throw;)
  if (!SubExpr)
    return;

  QualType QT = SubExpr->getType();

  // Check if thrown type is pointer
  if (!QT->isPointerType())
    return;

  diag(Throw->getExprLoc(),
       "exception object shall not have pointer type");
}

} // namespace clang::tidy::hsc
