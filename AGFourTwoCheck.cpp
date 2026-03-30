//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AGFourTwoCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AGFourTwoCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  Finder->addMatcher(
    cxxMethodDecl(
        isDefinition(),
        ofClass(cxxRecordDecl(isDerivedFrom(cxxRecordDecl())))
    ).bind("method"),
    this);
}

void AGFourTwoCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Method = Result.Nodes.getNodeAs<CXXMethodDecl>("method");
  if (!Method || Method->isImplicit())
    return;

  const auto *Derived = Method->getParent();
  if (!Derived)
    return;

  // Skip constructors/destructors/operators (optional)
  if (isa<CXXConstructorDecl>(Method) || isa<CXXDestructorDecl>(Method))
    return;

  for (const auto &Base : Derived->bases()) {
    const auto *BaseDecl = Base.getType()->getAsCXXRecordDecl();
    if (!BaseDecl)
      continue;

    for (const auto *BaseMethod : BaseDecl->methods()) {
      // Same name?
      if (BaseMethod->getName() != Method->getName())
        continue;

      // If it properly overrides → OK
      if (Method->size_overridden_methods() > 0)
        return;

      // If signatures differ → hiding case
      if (!Method->getType().getCanonicalType()
              .getTypePtr()
              ->isFunctionProtoType() ||
          !BaseMethod->getType().getCanonicalType()
              .getTypePtr()
              ->isFunctionProtoType()) {
        continue;
      }

      // Compare signatures (basic check)
      if (Method->getType() != BaseMethod->getType()) {
        diag(Method->getLocation(),
             "method '%0' in derived class hides overloaded method from base class")
            << Method->getName();
        return;
      }
    }
  }
}

} // namespace clang::tidy::hsc
