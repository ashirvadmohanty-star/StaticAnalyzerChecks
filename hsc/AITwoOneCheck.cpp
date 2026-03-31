//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AITwoOneCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AITwoOneCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  
  // Match reinterpret_cast or static_cast expressions
  Finder->addMatcher(explicitCastExpr(unless(cxxDynamicCastExpr()),hasSourceExpression(expr().bind("srcExpr"))).bind("badCast"),this);

}

void AITwoOneCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.

	const auto *Cast =
    	Result.Nodes.getNodeAs<ExplicitCastExpr>("badCast");
	const auto *SrcExpr =
    	Result.Nodes.getNodeAs<Expr>("srcExpr");

	if (!Cast || !SrcExpr)
    	return;

	const QualType SrcType = SrcExpr->getType();
	const QualType DestType = Cast->getTypeAsWritten();

	const CXXRecordDecl *SrcRecord =
      SrcType->getPointeeCXXRecordDecl();
	const CXXRecordDecl *DestRecord =
      DestType->getPointeeCXXRecordDecl();

	if (!SrcRecord || !DestRecord)
    	return;

  // Must be polymorphic base
	if (!SrcRecord->isPolymorphic())
    	return;

  // Must actually be a base → derived relationship
	if (!DestRecord->isDerivedFrom(SrcRecord))
    	return;

  // Check for virtual inheritance
	bool HasVirtualBase = false;
  	for (const auto &Base : DestRecord->bases()) {
    	if (Base.isVirtual() &&
        	Base.getType()->getAsCXXRecordDecl() == SrcRecord) {
      		HasVirtualBase = true;
      		break;
    	}
  	}

	if (!HasVirtualBase)
    	return;

  // Reject everything except dynamic_cast
	diag(Cast->getBeginLoc(),
       "casting from a virtual base class to a derived class shall be done "
       "only using dynamic_cast")
      << Cast->getSourceRange();
}



} // namespace clang::tidy::hsc
