//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_APONETHREECHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_APONETHREECHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::hsc {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// https://clang.llvm.org/extra/clang-tidy/checks/hsc/a-p-one-three.html
class APOneThreeCheck : public ClangTidyCheck {
public:
  APOneThreeCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override {
    return LangOpts.CPlusPlus;
  }
};

} // namespace clang::tidy::hsc

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_APONETHREECHECK_H
