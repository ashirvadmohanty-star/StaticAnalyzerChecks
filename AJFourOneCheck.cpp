//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AJFourOneCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AJFourOneCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  Finder->addMatcher(ifStmt().bind("ifStmt"), this);
}

void AJFourOneCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
    const auto *If = Result.Nodes.getNodeAs<IfStmt>("ifStmt");
  if (!If)
    return;

  // Walk to the last else-if in the chain
  const IfStmt *Current = If;

  while (true) {
    const Stmt *Else = Current->getElse();

    // No else → violation
    if (!Else) {
      diag(Current->getIfLoc(),
           "if-else-if chain should be terminated with a final else statement");
      return;
    }

    // If else is another if → it's an else-if, keep walking
    if (const auto *ElseIf = dyn_cast<IfStmt>(Else)) {
      Current = ElseIf;
    } else {
      // Found final else → valid
      return;
    }
  }
}

} // namespace clang::tidy::hsc
