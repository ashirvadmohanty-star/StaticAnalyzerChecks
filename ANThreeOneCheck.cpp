//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ANThreeOneCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {
void ANThreeOneCheck::registerMatchers(MatchFinder *Finder) {
 Finder->addMatcher(cxxMethodDecl(unless(isImplicit())).bind("method"), this);
}
void ANThreeOneCheck::check(const MatchFinder::MatchResult &Result) {
 const auto *Method = Result.Nodes.getNodeAs<CXXMethodDecl>("method");
 if (!Method)
   return;
 bool IsVirtualAsWritten = Method->isVirtualAsWritten();
 bool HasOverride = Method->hasAttr<OverrideAttr>();
 bool HasFinal = Method->hasAttr<FinalAttr>();
 bool IsOverriding = Method->size_overridden_methods() > 0;
 if (!IsVirtualAsWritten && !IsOverriding && !HasOverride && !HasFinal)
   return;
 if (IsOverriding) {
   if (IsVirtualAsWritten) {
     diag(Method->getBeginLoc(), "Rule HSCAN.3.1: 'virtual' specifier shall not be used on overriding functions");
   }
   if (HasOverride && HasFinal) {
     diag(Method->getBeginLoc(), "Rule HSCAN.3.1: 'override' and 'final' specifiers shall not be used together");
   } else if (!HasOverride && !HasFinal) {
     diag(Method->getBeginLoc(), "Rule HSCAN.3.1: overriding function is missing 'override' or 'final' specifier");
   }
 } else {
   if (IsVirtualAsWritten && HasFinal) {
     diag(Method->getBeginLoc(), "Rule HSCAN.3.1: 'virtual' and 'final' specifiers shall not be used together");
   }
 }
}
} // namespace clang::tidy::hsc
