//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "APOneFourCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void APOneFourCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  Finder->addMatcher(
      cxxConstructorDecl(
          isDefinition(),
          unless(isImplicit())
      ).bind("ctor"),
      this);
}

void APOneFourCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
  if (!Ctor)
    return;

  const auto *Parent = Ctor->getParent();
  if (!Parent)
    return;

  // Track initialized fields
  llvm::SmallPtrSet<const FieldDecl *, 8> InitializedFields;

  for (const auto *Init : Ctor->inits()) {
    if (const auto *Field = Init->getMember()) {
      InitializedFields.insert(Field);
    }
  }

  for (const auto *Field : Parent->fields()) {


    // Already initialized in-class
    if (Field->hasInClassInitializer())
      continue;

    // Initialized in constructor
    if (InitializedFields.count(Field))
      continue;

    // Violation
    diag(Field->getLocation(),
         "data member '%0' is not initialized in constructor")
        << Field->getName();
  }
}

} // namespace clang::tidy::hsc
