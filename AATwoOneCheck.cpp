//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AATwoOneCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AATwoOneCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  Finder->addMatcher( varDecl(
        hasLocalStorage(),        // local variables
        unless(isExpansionInSystemHeader())
    ).bind("var"), this);
}

void AATwoOneCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  const auto *VD = Result.Nodes.getNodeAs<VarDecl>("var");
  if (!VD)
    return;

  // Skip implicit/generated variables
  if (VD->isImplicit())
    return;

  // Skip parameters (optional)
  if (isa<ParmVarDecl>(VD))
    return;

  // Check if unused
  if (!VD->isUsed()) {
    diag(VD->getLocation(),
         "variable '%0' with limited visibility is not used")
        << VD->getName();
  }
}

} // namespace clang::tidy::hsc
