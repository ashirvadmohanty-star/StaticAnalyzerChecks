//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ANThreeTwoCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void ANThreeTwoCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
   Finder->addMatcher(
      cxxMethodDecl(
          isOverride()
      ).bind("method"),
      this);
}

void ANThreeTwoCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Method = Result.Nodes.getNodeAs<CXXMethodDecl>("method");
  if (!Method || Method->isImplicit())
    return;

  const auto &SM = *Result.SourceManager;
  const auto &LangOpts = Result.Context->getLangOpts();

  // Iterate over all overridden base methods
  for (const auto *BaseMethod : Method->overridden_methods()) {

    unsigned NumParams = Method->getNumParams();
    if (NumParams != BaseMethod->getNumParams())
      continue;

    for (unsigned i = 0; i < NumParams; ++i) {
      const ParmVarDecl *DerivedParam = Method->getParamDecl(i);
      const ParmVarDecl *BaseParam    = BaseMethod->getParamDecl(i);

      const Expr *DerivedDefault = DerivedParam->getDefaultArg();
      const Expr *BaseDefault    = BaseParam->getDefaultArg();

      // Case 1: One has default, other doesn't
      if ((DerivedDefault && !BaseDefault) ||
          (!DerivedDefault && BaseDefault)) {
        diag(DerivedParam->getLocation(),
             "default argument in overriding function differs from base declaration");
        continue;
      }

      // Case 2: Both have defaults → compare them
      if (DerivedDefault && BaseDefault) {
        std::string DerivedText =
            Lexer::getSourceText(
                CharSourceRange::getTokenRange(DerivedDefault->getSourceRange()),
                SM, LangOpts).str();

        std::string BaseText =
            Lexer::getSourceText(
                CharSourceRange::getTokenRange(BaseDefault->getSourceRange()),
                SM, LangOpts).str();

        if (DerivedText != BaseText) {
          diag(DerivedParam->getLocation(),
               "default argument '%0' differs from base version '%1'")
              << DerivedText << BaseText;
        }
      }
    }
  }
}

} // namespace clang::tidy::hsc
