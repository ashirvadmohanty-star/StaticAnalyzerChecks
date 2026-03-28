//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AREightOneCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AREightOneCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
          functionDecl(
            isExplicitTemplateSpecialization()
          ).bind("spec"), 
          this);
}

void AREightOneCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *MatchedDecl = Result.Nodes.getNodeAs<FunctionDecl>("spec");

  if (!MatchedDecl)
      return;

  diag(MatchedDecl->getBeginLoc(), "Function templates shall not be explicitly specialized");
}

} // namespace clang::tidy::hsc
