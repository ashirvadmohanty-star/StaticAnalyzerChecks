//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
//
//===----------------------------------------------------------------------===//

#include "APOneOneCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ExprCXX.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

class DynamicTypeVisitor
    : public RecursiveASTVisitor<DynamicTypeVisitor> {
public:
  explicit DynamicTypeVisitor(ASTContext *Context,
                              ClangTidyCheck *Check)
      : Context(Context), Check(Check) {}

  // 🔴 Detect virtual function calls
  bool VisitCXXMemberCallExpr(CXXMemberCallExpr *Call) {
    const auto *Method = Call->getMethodDecl();
    if (!Method)
      return true;

    if (Method->isVirtual()) {
      // Optional: restrict to calls on 'this'
      const Expr *Obj = Call->getImplicitObjectArgument();
      if (Obj && isa<CXXThisExpr>(Obj->IgnoreImpCasts())) {
        Check->diag(Call->getExprLoc(),
                    "virtual function call in constructor/destructor "
                    "uses dynamic type");
      }
    }
    return true;
  }

  // Detect dynamic_cast
  bool VisitCXXDynamicCastExpr(CXXDynamicCastExpr *Expr) {
    Check->diag(Expr->getExprLoc(),
                "dynamic_cast used in constructor/destructor "
                "uses dynamic type");
    return true;
  }

  //  Detect typeid(expr)
  bool VisitCXXTypeidExpr(CXXTypeidExpr *Expr) {
    if (!Expr->isTypeOperand()) { // typeid(expr), not typeid(type)
      Check->diag(Expr->getExprLoc(),
                  "typeid used on object in constructor/destructor "
                  "uses dynamic type");
    }
    return true;
  }

private:
  ASTContext *Context;
  ClangTidyCheck *Check;
};

} // namespace

// Register matchers
void APOneOneCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxConstructorDecl(isDefinition()).bind("ctor"),
      this);

  Finder->addMatcher(
      cxxDestructorDecl(isDefinition()).bind("dtor"),
      this);
}

//  Main check
void APOneOneCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
  const auto *Dtor = Result.Nodes.getNodeAs<CXXDestructorDecl>("dtor");

  const CXXMethodDecl *Method = Ctor ? Ctor : Dtor;
  if (!Method || !Method->hasBody())
    return;

  // Traverse body
  DynamicTypeVisitor Visitor(Result.Context, this);
  Visitor.TraverseStmt(Method->getBody());
}

} // namespace clang::tidy::hsc
