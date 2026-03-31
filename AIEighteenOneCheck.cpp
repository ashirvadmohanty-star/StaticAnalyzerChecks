//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AIEighteenOneCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AIEighteenOneCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  
  // ===============================================
  // Case 1: Union member copy within same object
  // ===============================================
  Finder->addMatcher(binaryOperator(isAssignmentOperator(),hasLHS(memberExpr(hasObjectExpression(expr().bind("unionBase")),hasDeclaration(fieldDecl(hasParent(cxxRecordDecl(isUnion()))).bind("lhsField")))),hasRHS(memberExpr(hasObjectExpression(expr().bind("unionBase")),hasDeclaration(fieldDecl(hasParent(cxxRecordDecl(isUnion()))).bind("rhsField"))))).bind("unionAssign"),this);

  // ===============================================
  // Case 2: memcpy with potentially overlapping args
  // ===============================================
  Finder->addMatcher(
  callExpr(
    callee(functionDecl(
      hasAnyName("memcpy", "std::memcpy")
    )),
    argumentCountIs(3),
    hasArgument(0, expr().bind("dest")),
    hasArgument(1, expr().bind("src")),
    hasArgument(2, expr().bind("size"))
  ).bind("memcpyCall"),
  this);

}

static bool getBaseAndOffset(const Expr *E,
                            const Expr *&Base,
                            int64_t &Offset) {
  E = E->IgnoreParenImpCasts();

  // Case: &arr[i]
  if (const auto *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() == UO_AddrOf) {
      if (const auto *ASE = dyn_cast<ArraySubscriptExpr>(UO->getSubExpr())) {
        Base = ASE->getBase()->IgnoreParenImpCasts();

        if (const auto *Idx =
                dyn_cast<IntegerLiteral>(ASE->getIdx())) {
          Offset = Idx->getValue().getSExtValue();
          return true;
        }
      }
    }
  }

  // Case: ptr + N
  if (const auto *BO = dyn_cast<BinaryOperator>(E)) {
    if (BO->getOpcode() == BO_Add) {
      Base = BO->getLHS()->IgnoreParenImpCasts();

      if (const auto *Idx =
              dyn_cast<IntegerLiteral>(BO->getRHS())) {
        Offset = Idx->getValue().getSExtValue();
        return true;
      }
    }
  }

  // Case: plain ptr
  Base = E;
  Offset = 0;
  return true;
}

void AIEighteenOneCheck::check(const MatchFinder::MatchResult &Result) {

 // -----------------------------------------------
  // Union member copy diagnosis
  // -----------------------------------------------
  if (const auto *BO =
    Result.Nodes.getNodeAs<BinaryOperator>("unionAssign")) {

    const auto *LHSField =
        Result.Nodes.getNodeAs<FieldDecl>("lhsField");
    const auto *RHSField =
        Result.Nodes.getNodeAs<FieldDecl>("rhsField");

    if (LHSField != RHSField) {
      diag(BO->getOperatorLoc(),
           "copying between members of the same union object has undefined "
           "behaviour (overlapping subobject copy)")
          << BO->getSourceRange();
    }
    return;
  }

  // -----------------------------------------------
  // memcpy overlap diagnosis
  // -----------------------------------------------
  if (const auto *Call =
        Result.Nodes.getNodeAs<CallExpr>("memcpyCall")) {

  const Expr *Dest = Result.Nodes.getNodeAs<Expr>("dest");
  const Expr *Src  = Result.Nodes.getNodeAs<Expr>("src");

  if (!Dest || !Src)
    return;

  const Expr *Base1 = nullptr;
  const Expr *Base2 = nullptr;
  int64_t Off1 = 0, Off2 = 0;

  if (!getBaseAndOffset(Dest, Base1, Off1) ||
      !getBaseAndOffset(Src, Base2, Off2))
    return;

  // 🔥 Only proceed if SAME base object
  if (!Expr::isSameComparisonOperand(Base1, Base2))
    return;

  // Get size (best effort)
  int64_t Size = 1; // fallback
  if (const auto *SizeExpr =
          Result.Nodes.getNodeAs<Expr>("size")) {
    if (const auto *IL =
            dyn_cast<IntegerLiteral>(SizeExpr->IgnoreParenImpCasts())) {
      Size = IL->getValue().getSExtValue();
    }
  }

  // 🔥 Overlap condition
  if (std::abs(Off1 - Off2) < Size) {
    diag(Call->getBeginLoc(),
         "std::memcpy source and destination overlap; "
         "use std::memmove instead")
        << Call->getSourceRange();
  }
}
}


} // namespace clang::tidy::hsc
