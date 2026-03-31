//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AATwoFourCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AATwoFourCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  Finder->addMatcher(
      functionDecl(
          isDefinition(),
          unless(isMain()),
          anyOf(
              isStaticStorageClass(),
              hasDeclContext(namespaceDecl(isAnonymous()))
          )
      ).bind("func"),
      this);
}

void AATwoFourCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (!Func)
    return;

  // Ignore compiler-generated or invalid locations
  if (Func->isImplicit() || !Func->getLocation().isValid())
    return;

  // Check if function is referenced
  if (Func->isUsed())
    return;

  // Report unused limited-visibility function
  diag(Func->getLocation(),
       "function '%0' has limited visibility but is never used")
      << Func->getName();
}

} // namespace clang::tidy::hsc
