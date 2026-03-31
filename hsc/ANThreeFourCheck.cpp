//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ANThreeFourCheck.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void ANThreeFourCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.

  Finder->addMatcher(
      binaryOperator(
          hasAnyOperatorName("==", "!="),
          hasLHS(expr().bind("lhs")),
          hasRHS(expr().bind("rhs"))
      ).bind("cmp"),
      this);

}


static bool isNullPtr(const Expr *E) {
  E = E->IgnoreParenImpCasts();
  return isa<CXXNullPtrLiteralExpr>(E);
}

static bool isPointerToMemberFunction(QualType T,
                                      const CXXRecordDecl **OutClass = nullptr) {
  if (const auto *MPT = T->getAs<MemberPointerType>()) {
    if (MPT->isMemberFunctionPointer()) {
      if (OutClass)
        *OutClass = MPT->getMostRecentCXXRecordDecl();
      return true;
    }
  }
  return false;
}

static bool hasVirtualWithSameSignature(const CXXRecordDecl *RD,
                                        const FunctionProtoType *Sig) {
  for (const auto *M : RD->methods()) {
    if (M->isVirtual() &&
        M->getType()->getAs<FunctionProtoType>() == Sig) {
      return true;
    }
  }
  return false;
}

static const CXXMethodDecl* getMethodDeclFromExpr(const Expr *E) {
  E = E->IgnoreParenImpCasts();

  if (const auto *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() == UO_AddrOf) {
      if (const auto *DRE = dyn_cast<DeclRefExpr>(UO->getSubExpr())) {
        return dyn_cast<CXXMethodDecl>(DRE->getDecl());
      }
    }
  }

  return nullptr;
}

void ANThreeFourCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.

  const auto *Op =
      Result.Nodes.getNodeAs<BinaryOperator>("cmp");
  const auto *LHS =
      Result.Nodes.getNodeAs<Expr>("lhs");
  const auto *RHS =
      Result.Nodes.getNodeAs<Expr>("rhs");

  if (!Op || !LHS || !RHS)
    return;

  // nullptr comparisons are always allowed
  if (isNullPtr(LHS) || isNullPtr(RHS))
    return;
  
  // 🔥 2. FIX: handle direct member function comparisons
  const CXXMethodDecl *LMethod = getMethodDeclFromExpr(LHS);
  const CXXMethodDecl *RMethod = getMethodDeclFromExpr(RHS);

  if (LMethod && RMethod) {
    // both non-virtual → OK
    if (!LMethod->isVirtual() && !RMethod->isVirtual())
      return;

    // same function → OK
    if (LMethod == RMethod)
      return;
  }
  

  const CXXRecordDecl *LClass = nullptr;
  const CXXRecordDecl *RClass = nullptr;

  bool LIsPMF = isPointerToMemberFunction(LHS->getType(), &LClass);
  bool RIsPMF = isPointerToMemberFunction(RHS->getType(), &RClass);

  if (!LIsPMF && !RIsPMF)
    return;

  const CXXRecordDecl *TargetClass = LClass ? LClass : RClass;
  if (!TargetClass)
    return;

  // Incomplete class → non-compliant
  if (!TargetClass->hasDefinition()) {
    diag(Op->getBeginLoc(),
         "comparison of pointer-to-member functions of an incomplete class "
         "shall only be against nullptr")
        << Op->getSourceRange();
    return;
  }

  // Extract function signature
  QualType PMType = LIsPMF ? LHS->getType() : RHS->getType();
  const auto *MPT = PMType->getAs<MemberPointerType>();
  const auto *FPT = MPT->getPointeeType()->getAs<FunctionProtoType>();

  if (!FPT)
    return;

  // Check for virtual function ambiguity
  if (hasVirtualWithSameSignature(TargetClass, FPT)) {
    diag(Op->getBeginLoc(),
         "comparison of a potentially virtual pointer to member function "
         "shall only be against nullptr")
        << Op->getSourceRange();
  }

}

} // namespace clang::tidy::hsc
