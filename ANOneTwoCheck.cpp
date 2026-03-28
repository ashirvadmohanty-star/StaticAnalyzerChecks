//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ANOneTwoCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/SmallPtrSet.h"
using namespace clang::ast_matchers;
namespace clang::tidy::hsc {
void ANOneTwoCheck::registerMatchers(MatchFinder *Finder) {
 Finder->addMatcher(
     cxxRecordDecl(isDefinition(), hasAnyBase(cxxBaseSpecifier())).bind("class"),
     this);
}
void ANOneTwoCheck::check(const MatchFinder::MatchResult &Result) {
 const auto *MatchedDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("class");
 if (!MatchedDecl || !MatchedDecl->hasDefinition() || MatchedDecl->getNumVBases() == 0)
   return;
 llvm::SmallPtrSet<const CXXRecordDecl *, 4> VirtualBases;
 for (const auto &VBase : MatchedDecl->vbases()) {
   if (const auto *BaseDecl = VBase.getType()->getAsCXXRecordDecl())
     VirtualBases.insert(BaseDecl->getCanonicalDecl());
 }
 CXXBasePaths Paths;
 bool MixedInheritance = MatchedDecl->lookupInBases(
     [&VirtualBases](const CXXBaseSpecifier *Specifier, CXXBasePath &Path) {
       if (Specifier->isVirtual())
         return false;
       if (const auto *BaseDecl = Specifier->getType()->getAsCXXRecordDecl()) {
         if (VirtualBases.count(BaseDecl->getCanonicalDecl()))
           return true;
       }
       return false;
     },
     Paths);
 if (MixedInheritance) {
   diag(MatchedDecl->getLocation(), "Rule HSCAN.1.2: An accessible base class shall not be both virtual and non-virtual in the same hierarchy");
 }
}
} // namespace clang::tidy::hsc

