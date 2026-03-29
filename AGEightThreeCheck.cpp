//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AGEightThreeCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTContext.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AGEightThreeCheck::registerMatchers(MatchFinder *Finder) {
 Finder->addMatcher(
     binaryOperator(
         hasOperatorName("="),
         hasLHS(ignoringParenImpCasts(declRefExpr(to(varDecl().bind("lhs"))))),
         hasRHS(ignoringParenImpCasts(anyOf(
             unaryOperator(
                 hasOperatorName("&"),
                 hasUnaryOperand(ignoringParenImpCasts(declRefExpr(to(varDecl(hasLocalStorage()).bind("rhs")))))
             ),
             declRefExpr(to(varDecl(hasLocalStorage(), hasType(arrayType())).bind("rhs")))
         )))
     ).bind("assign"),
     this);
}
void AGEightThreeCheck::check(const MatchFinder::MatchResult &Result) {
 const auto *Assign = Result.Nodes.getNodeAs<BinaryOperator>("assign");
 const auto *LHS = Result.Nodes.getNodeAs<VarDecl>("lhs");
 const auto *RHS = Result.Nodes.getNodeAs<VarDecl>("rhs");
 if (!Assign || !LHS || !RHS)
   return;
 if (!LHS->hasLocalStorage()) {
   diag(Assign->getOperatorLoc(), "Rule HSCAG.8.3: An assignment operator shall not assign the address of an automatic-storage object to an object with greater lifetime");
   return;
 }
 if (isa<ParmVarDecl>(LHS) && !isa<ParmVarDecl>(RHS)) {
   diag(Assign->getOperatorLoc(), "Rule HSCAG.8.3: An assignment operator shall not assign the address of an automatic-storage object to an object with greater lifetime");
   return;
 }
 ASTContext &Context = *Result.Context;
 auto GetScopeStmt = [&Context](const VarDecl *VD) -> const Stmt * {
   DynTypedNodeList Parents = Context.getParents(*VD);
   while (!Parents.empty()) {
     const Stmt *S = Parents[0].get<Stmt>();
     if (S && (isa<CompoundStmt>(S) || isa<ForStmt>(S) || isa<WhileStmt>(S) ||
               isa<CXXForRangeStmt>(S) || isa<IfStmt>(S) || isa<SwitchStmt>(S))) {
       return S;
     }
     Parents = Context.getParents(Parents[0]);
   }
   return nullptr;
 };
 const Stmt *LHSScope = GetScopeStmt(LHS);
 const Stmt *RHSScope = GetScopeStmt(RHS);
 if (LHSScope != RHSScope) {
   diag(Assign->getOperatorLoc(), "Rule HSCAG.8.3: An assignment operator shall not assign the address of an automatic-storage object to an object with greater lifetime");
 }
}

} // namespace clang::tidy::hsc
