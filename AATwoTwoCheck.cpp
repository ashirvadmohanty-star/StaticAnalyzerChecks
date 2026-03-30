//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AATwoTwoCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AATwoTwoCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  Finder->addMatcher(
      parmVarDecl(
          hasIdentifier(),                        // only named params
          unless(isExpansionInSystemHeader())
      ).bind("param"),
      this);
}

void AATwoTwoCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  const auto *Param = Result.Nodes.getNodeAs<ParmVarDecl>("param");
  if (!Param)
    return;

  // Skip implicit parameters
  if (Param->isImplicit())
    return;

  // Skip parameters without names (just in case)
  if (!Param->getIdentifier())
    return;

  // Skip intentionally unused parameters
  if (Param->hasAttr<UnusedAttr>())
    return;

  // Core check: is it used?
  if (!Param->isUsed()) {
    diag(Param->getLocation(),
         "function parameter '%0' is not used")
        << Param->getName();
  }
}

} // namespace clang::tidy::hsc
