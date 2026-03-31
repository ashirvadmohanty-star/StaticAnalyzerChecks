//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "APOneThreeCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void APOneThreeCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  // Match constructors callable with a single argument and not explicit
  Finder->addMatcher(
      cxxConstructorDecl(
          unless(isExplicit()),
          unless(isDeleted()),
          ofClass(recordDecl().bind("class")),
          anyOf(
              parameterCountIs(1),
              allOf(
                  parameterCountIs(2),
                  hasParameter(1, parmVarDecl(hasDefaultArgument()))
              )
          )
      ).bind("ctor"),
      this);

  // Match conversion operators that are not explicit
  Finder->addMatcher(
      cxxConversionDecl(
          unless(isExplicit()),
          unless(isDeleted())
      ).bind("conv"),
      this);
  
}

void APOneThreeCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  //Constructor case
  if (const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor")) {
    diag(Ctor->getLocation(),
         "single-argument constructor should be marked explicit")
        << FixItHint::CreateInsertion(Ctor->getLocation(), "explicit ");
  }

  // Conversion operator case
  if (const auto *Conv = Result.Nodes.getNodeAs<CXXConversionDecl>("conv")) {
    diag(Conv->getLocation(),
         "conversion operator should be marked explicit")
        << FixItHint::CreateInsertion(Conv->getLocation(), "explicit ");
  }
}

} // namespace clang::tidy::hsc
