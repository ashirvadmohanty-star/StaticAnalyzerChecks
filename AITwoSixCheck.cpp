//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AITwoSixCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AITwoSixCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(explicitCastExpr().bind("cast"), this);
}

void AITwoSixCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Cast = Result.Nodes.getNodeAs<ExplicitCastExpr>("cast");
  if (!Cast)
      return;

  if (isa<CXXConstCastExpr>(Cast))
      return;

  QualType DestType = Cast->getTypeAsWritten();
  QualType SrcType = Cast->getSubExpr()->getType();

  if (!DestType->isPointerType())
      return;

  if (SrcType->isIntegerType() || SrcType->isEnumeralType() || SrcType->isVoidPointerType()) {
    diag(Cast->getBeginLoc(), 
            "An object with integral, enumerated, or pointer to void type shall not be cast to a pointer type");
  }
}

} // namespace clang::tidy::hsc
