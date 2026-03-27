//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AAOneTwoCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AAOneTwoCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
          callExpr(
              callee(functionDecl(unless(returns(voidType())))), 
              hasParent(compoundStmt()), 
              unless(hasParent(explicitCastExpr(hasDestinationType(voidType())))), 
              unless(cxxOperatorCallExpr())
          ).bind("unused-return"),
          this
  );
}

void AAOneTwoCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *MatchedCall = Result.Nodes.getNodeAs<CallExpr>("unused-return");
  
  if (!MatchedCall)
      return;

  auto Diag = diag(MatchedCall->getBeginLoc(), 
          "[HSCAA.1.2] The value returned by this function is ignored; "
          "use the value or cast to 'void' to clarify intent");

  Diag << FixItHint::CreateInsertion(MatchedCall->getBeginLoc(), "(void)");
}

} // namespace clang::tidy::hsc
