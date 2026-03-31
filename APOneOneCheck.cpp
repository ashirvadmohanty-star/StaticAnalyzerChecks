//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "APOneOneCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void APOneOneCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  
   // --- Virtual calls inside constructor ---
  Finder->addMatcher(
      cxxConstructorDecl(
          isDefinition(),
          hasDescendant(
              cxxMemberCallExpr(
                  callee(cxxMethodDecl(isVirtual()))
              ).bind("virtualCall")
          )
      ),
      this);

  // --- Virtual calls inside destructor ---
  Finder->addMatcher(
      cxxDestructorDecl(
          isDefinition(),
          hasDescendant(
              cxxMemberCallExpr(
                  callee(cxxMethodDecl(isVirtual()))
              ).bind("virtualCall")
          )
      ),
      this);

  // --- dynamic_cast inside constructor ---
  Finder->addMatcher(
      cxxDynamicCastExpr(
          hasAncestor(cxxConstructorDecl())
      ).bind("dynCast"),
      this);

  // --- dynamic_cast inside destructor ---
  Finder->addMatcher(
      cxxDynamicCastExpr(
          hasAncestor(cxxDestructorDecl())
      ).bind("dynCast"),
      this);
}

void APOneOneCheck::check(const MatchFinder::MatchResult &Result) {
if (const auto *Call =
          Result.Nodes.getNodeAs<CXXMemberCallExpr>("virtualCall")) {

    if (const auto *Method = Call->getMethodDecl()) {
      if (Method->isVirtual()) {
        diag(Call->getExprLoc(),
             "virtual function call inside constructor/destructor "
             "uses incomplete dynamic type");
      }
    }
  }

  if (const auto *Dyn =
          Result.Nodes.getNodeAs<CXXDynamicCastExpr>("dynCast")) {

    diag(Dyn->getExprLoc(),
         "dynamic_cast used inside constructor/destructor");
  }

  //  Handle typeid safely here
  if (const auto *Stmt = Result.Nodes.getNodeAs<IfStmt>("stmt")) {
    if (const auto *TypeId = llvm::dyn_cast<CXXTypeidExpr>(Stmt)) {
      diag(TypeId->getExprLoc(),
           "typeid used inside constructor/destructor");
    }
  }
}

} // namespace clang::tidy::hsc
