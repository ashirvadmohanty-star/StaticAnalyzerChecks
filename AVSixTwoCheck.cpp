//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AVSixTwoCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AVSixTwoCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxNewExpr().bind("new"), this);
  Finder->addMatcher(cxxDeleteExpr().bind("delete"), this);
  Finder->addMatcher(
          callExpr(callee(functionDecl(hasAnyName("::malloc", "::calloc", "::realloc", "::aligned_alloc", "::free"))))
            .bind("c-alloc"),
          this);
  Finder->addMatcher(
          cxxMemberCallExpr(
              callee(
                  cxxMethodDecl(
                      hasAnyName("allocate", "deallocate"), 
                      ofClass(hasDeclContext(namespaceDecl(hasName("std"))))
                  )
              )
          ).bind("std-alloc"),
          this);
  Finder->addMatcher(
          cxxMemberCallExpr(callee(cxxMethodDecl(hasName("release"),
                      ofClass(hasName("::std::unique_ptr")))))
            .bind("unique-release"),
          this);
}

void AVSixTwoCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto *New = Result.Nodes.getNodeAs<CXXNewExpr>("new")) {
    if (New->getNumPlacementArgs() == 0) {
        diag(New->getBeginLoc(), "Any non-placement form of new shall not be used");
    }
    return;
  }

  if (const auto *Delete = Result.Nodes.getNodeAs<CXXDeleteExpr>("delete")) {
    diag(Delete->getBeginLoc(), "Any non-placement form of delete shall not be used");
    return;
  }

  if (const auto *CAlloc = Result.Nodes.getNodeAs<CallExpr>("c-alloc")) {
    diag(CAlloc->getBeginLoc(), "malloc, calloc, realloc, aligned_alloc, and free shall not be used");
    return;
  }

  if (const auto *StdAlloc = Result.Nodes.getNodeAs<CXXMemberCallExpr>("std-alloc")) {
    diag(StdAlloc->getBeginLoc(), "std allocate or deallocate member functions shall not be used");
    return;
  }

  if (const auto *UniqueRelease = Result.Nodes.getNodeAs<CXXMemberCallExpr>("unique-release")) {
    diag(UniqueRelease->getBeginLoc(), "std::unique_ptr::release shall not be used");
    return;
  }
}

} // namespace clang::tidy::hsc
