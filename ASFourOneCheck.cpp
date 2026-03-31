//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ASFourOneCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void ASFourOneCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      functionDecl(
          anyOf(
              cxxDestructorDecl(),
              cxxConstructorDecl(isMoveConstructor()),
              cxxMethodDecl(isMoveAssignmentOperator())
          ),
          unless(isNoThrow())
      ).bind("func"),
      this);
}

void ASFourOneCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *FD = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (!FD)
    return;

  // Avoid implicit functions (compiler-generated)
  if (FD->isImplicit())
    return;

  // Get readable function kind
  std::string FuncType = "function";

  if (isa<CXXDestructorDecl>(FD))
    FuncType = "destructor";
  else if (const auto *Ctor = dyn_cast<CXXConstructorDecl>(FD)) {
    if (Ctor->isMoveConstructor())
      FuncType = "move constructor";
  } else if (const auto *Method = dyn_cast<CXXMethodDecl>(FD)) {
    if (Method->isMoveAssignmentOperator())
      FuncType = "move assignment operator";
  }

  diag(FD->getLocation(),
       "%0 should be marked noexcept")
      << FuncType;

  // Optional FixIt (basic)
  if (FD->getTypeSourceInfo()) {
    SourceLocation EndLoc = FD->getEndLoc();

    diag(FD->getLocation(),
         "add 'noexcept' specifier",
         DiagnosticIDs::Note)
        << FixItHint::CreateInsertion(EndLoc, " noexcept");
  }
}

} // namespace clang::tidy::hsc
