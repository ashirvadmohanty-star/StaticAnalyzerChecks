//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "APOneTwoCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTContext.h"
#include "llvm/ADT/SmallPtrSet.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void APOneTwoCheck::registerMatchers(MatchFinder *Finder) {
 Finder->addMatcher(
     cxxConstructorDecl(isDefinition(), unless(isImplicit()), unless(isDefaulted()), unless(isDeleted())).bind("ctor"),
     this);
}
void APOneTwoCheck::check(const MatchFinder::MatchResult &Result) {
 const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
 if (!Ctor || Ctor->isDelegatingConstructor())
   return;
 const CXXRecordDecl *ClassDecl = Ctor->getParent();
 if (!ClassDecl || ClassDecl->isDependentContext())
   return;
 llvm::SmallPtrSet<const CXXRecordDecl *, 4> WrittenBases;
 for (const auto *Init : Ctor->inits()) {
   if (Init->isBaseInitializer() && Init->isWritten()) {
     if (const CXXRecordDecl *BaseDecl = Init->getBaseClass()->getAsCXXRecordDecl()) {
       WrittenBases.insert(BaseDecl->getCanonicalDecl());
     }
   }
 }
 bool ViolationFound = false;
 auto CheckBase = [&](const CXXBaseSpecifier &Base) {
   if (ViolationFound) return;
   const CXXRecordDecl *BaseRecord = Base.getType()->getAsCXXRecordDecl();
   if (!BaseRecord) return;
   if (WrittenBases.count(BaseRecord->getCanonicalDecl())) return;
   if (BaseRecord->isEmpty()) return;
   diag(Ctor->getLocation(), "Rule HSCAP.1.2: All constructors of a class should explicitly initialise all virtual and immediate base classes");
   ViolationFound = true;
 };
 for (const auto &Base : ClassDecl->bases()) {
   if (!Base.isVirtual())
     CheckBase(Base);
 }
 for (const auto &VBase : ClassDecl->vbases()) {
   CheckBase(VBase);
 }
}

} // namespace clang::tidy::hsc
