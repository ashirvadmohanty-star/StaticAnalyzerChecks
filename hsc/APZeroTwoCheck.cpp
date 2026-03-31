//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "APZeroTwoCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/StmtVisitor.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {


namespace {

// Detect comparison between `this` and parameter address
class SelfAssignmentGuardVisitor
    : public ConstStmtVisitor<SelfAssignmentGuardVisitor, bool> {
public:
  bool VisitIfStmt(const IfStmt *If) {
    return containsThisOtherComparison(If->getCond());
  }

  bool VisitCompoundStmt(const CompoundStmt *CS) {
    for (const Stmt *S : CS->body())
      if (Visit(S))
        return true;
    return false;
  }

  bool VisitStmt(const Stmt *) { return false; }

private:
  bool containsThisOtherComparison(const Expr *E) {
    if (!E)
      return false;

    E = E->IgnoreParenImpCasts();

    if (const auto *BO = dyn_cast<BinaryOperator>(E)) {
      if (!BO->isComparisonOp())
        return false;

      const Expr *L = BO->getLHS()->IgnoreParenImpCasts();
      const Expr *R = BO->getRHS()->IgnoreParenImpCasts();

      return (isThisExpr(L) && isAddressOfParam(R)) ||
             (isThisExpr(R) && isAddressOfParam(L));
    }
    return false;
  }

  static bool isThisExpr(const Expr *E) {
    return isa<CXXThisExpr>(E);
  }

  static bool isAddressOfParam(const Expr *E) {
    if (const auto *UO = dyn_cast<UnaryOperator>(E))
      return UO->getOpcode() == UO_AddrOf;

    if (const auto *Call = dyn_cast<CallExpr>(E))
      if (const auto *FD = Call->getDirectCallee())
        return FD->getName() == "addressof";

    return false;
  }
};

} // namespace


void APZeroTwoCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  
  Finder->addMatcher(
      cxxMethodDecl(
          isUserProvided(),
          isCopyAssignmentOperator()
      ).bind("assign"),
      this);

  Finder->addMatcher(
      cxxMethodDecl(
          isUserProvided(),
          isMoveAssignmentOperator()
      ).bind("assign"),
      this);

}

void APZeroTwoCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.

  const auto *Method =
      Result.Nodes.getNodeAs<CXXMethodDecl>("assign");

  if (!Method || !Method->hasBody())
    return;

  const Stmt *Body = Method->getBody();
  SelfAssignmentGuardVisitor Visitor;

  // If a guard exists, we consider it compliant
  if (Visitor.Visit(Body))
    return;

  // Heuristic suppression: copy-and-swap pattern
  for (const Stmt *S : Body->children()) {
    if (const auto *DS = dyn_cast<DeclStmt>(S)) {
      for (const Decl *D : DS->decls())
        if (const auto *VD = dyn_cast<VarDecl>(D))
          if (VD->getType()->getAsCXXRecordDecl() ==
              Method->getParent()) {
            return; // likely copy-and-swap
          }
    }
  }

  diag(Method->getLocation(),
       "user-provided assignment operator does not appear to handle "
       "self-assignment")
      << Method->getSourceRange();

}

} // namespace clang::tidy::hsc
