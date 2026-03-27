//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AITwoFiveCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AITwoFiveCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
            cxxReinterpretCastExpr().bind("reinterpretCast"),
            this);
}

static bool isAllowedTargetType(const QualType Target, ASTContext &Ctx) {
    if (Target->isVoidType() || Target->isCharType() || 
            (Target->isUnsignedIntegerType() && Ctx.getTypeSize(Target) == 8))
        return true;

    if (const auto *ET = Target->getAs<EnumType>()) {
        if (ET->getDecl()->getName() == "byte" && ET->getDecl()->getQualifiedNameAsString() == "std::byte")
            return true;
    }

    if (Target->isIntegerType()) {
        const auto *BT = Target->getAs<BuiltinType>();
        if (BT && (BT->getKind() == BuiltinType::ULongLong ||
                    BT->getKind() == BuiltinType::UInt128 ||
                    BT->getKind() == BuiltinType::ULong ||
                    (Ctx.getTypeSize(Target) >= Ctx.getTypeSize(Ctx.VoidPtrTy))))
            return true;
    }
    return false;
}

static bool isAllowedPointerTarget(const QualType Target, ASTContext &Ctx) {
    QualType Pointee = Target->getPointeeType();
    if (Pointee.isNull())
        return false;

    Pointee = Pointee.getCanonicalType().getUnqualifiedType();

    if (Pointee->isVoidType() || Pointee->isCharType() 
            || (Pointee->isUnsignedIntegerType() && Ctx.getTypeSize(Pointee) == 8))
        return true;

    if (const auto *ET = Pointee->getAs<EnumType>()) {
        if (ET->getDecl()->getName() == "byte" &&
                ET->getDecl()->getQualifiedNameAsString() == "std::byte")
            return true;
    }

    return false;
}

void AITwoFiveCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Cast = Result.Nodes.getNodeAs<CXXReinterpretCastExpr>("reinterpretCast");
  if (!Cast)
      return;

  QualType TargetType = Cast->getType().getCanonicalType();
  QualType SourceType = Cast->getSubExpr()->getType().getCanonicalType();

  if (TargetType->isPointerType() && isAllowedPointerTarget(TargetType, *Result.Context))
      return;

  if (TargetType->isIntegerType() && SourceType->isPointerType() && isAllowedTargetType(TargetType, *Result.Context))
      return;

  diag(Cast->getBeginLoc(), "reinterpret_cast shall not be used between unrelated types") << FixItHint();
}

} // namespace clang::tidy::hsc
