//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AATwoOneCheck.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AATwoOneCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      varDecl(
          hasLocalStorage(),
          unless(isExpansionInSystemHeader())
      ).bind("var"),
      this);
}

void AATwoOneCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *VD = Result.Nodes.getNodeAs<VarDecl>("var");
  if (!VD)
    return;

  // Skip unnamed variables
  if (!VD->getIdentifier())
    return;

  // Skip variables explicitly marked maybe_unused
  if (VD->hasAttr<clang::UnusedAttr>())
    return;

  // If variable is used, it's fine
  if (VD->isUsed())
    return;

  // Check for RAII-style usage (non-trivial ctor/dtor)
  const QualType QT = VD->getType();
  if (const auto *RT = QT->getAsCXXRecordDecl()) {
    if (RT->hasNonTrivialDestructor()) {
      // Considered "used" due to side effects
      return;
    }
  }

  // Report diagnostic
  diag(VD->getLocation(),
       "variable '%0' with limited visibility is never used")
      << VD->getName();

  // Optional fix-it: suggest [[maybe_unused]]
  diag(VD->getLocation(),
       "consider marking it as [[maybe_unused]] if intentional",
       DiagnosticIDs::Note);
}

} // namespace clang::tidy::hsc
