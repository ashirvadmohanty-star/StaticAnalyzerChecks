#include "AATwoThreeCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/SmallPtrSet.h"
using namespace clang;
using namespace clang::ast_matchers;
namespace clang::tidy::hsc {
namespace {
// A local visitor to find real usages in the file and ignore self-references
class UsageVisitor : public RecursiveASTVisitor<UsageVisitor> {
 llvm::SmallPtrSetImpl<const NamedDecl*> &ValidUsages;
 const Decl *CurrentContext = nullptr;
public:
 UsageVisitor(llvm::SmallPtrSetImpl<const NamedDecl*> &Usages) : ValidUsages(Usages) {}
 // Keep track of what function/class we are currently inside
 bool TraverseDecl(Decl *D) {
   const Decl *Prev = CurrentContext;
   if (D) CurrentContext = D;
   bool Result = RecursiveASTVisitor<UsageVisitor>::TraverseDecl(D);
   CurrentContext = Prev;
   return Result;
 }
 // Check if a usage is happening inside the class itself or its own methods
 bool isSelfReference(const NamedDecl *UsedDecl) {
   const Decl *D = CurrentContext;
   while (D) {
     if (D->getCanonicalDecl() == UsedDecl) return true;
     if (const auto *Method = dyn_cast<CXXMethodDecl>(D)) {
       if (Method->getParent() && Method->getParent()->getCanonicalDecl() == UsedDecl)
         return true;
     }
     const DeclContext *DC = D->getDeclContext();
     D = DC ? dyn_cast<Decl>(DC) : nullptr;
   }
   return false;
 }
 bool VisitTypeLoc(TypeLoc TL) {
   if (TL.isNull() || TL.getType().isNull()) return true;
   if (const TagDecl *TD = TL.getType()->getAsTagDecl()) {
     const NamedDecl *ND = cast<NamedDecl>(TD->getCanonicalDecl());
     if (!isSelfReference(ND)) ValidUsages.insert(ND);
   } else if (const TypedefType *TT = TL.getType()->getAs<TypedefType>()) {
     const NamedDecl *ND = cast<NamedDecl>(TT->getDecl()->getCanonicalDecl());
     if (!isSelfReference(ND)) ValidUsages.insert(ND);
   }
   return true;
 }
 bool VisitDeclRefExpr(DeclRefExpr *E) {
   if (!E->getDecl()) return true;
   const NamedDecl *ND = cast<NamedDecl>(E->getDecl()->getCanonicalDecl());
   if (!isSelfReference(ND)) ValidUsages.insert(ND);
   return true;
 }
};
} // namespace
void AATwoThreeCheck::registerMatchers(MatchFinder *Finder) {
 // Grab the whole Translation Unit once
 Finder->addMatcher(translationUnitDecl().bind("tu"), this);
}
void AATwoThreeCheck::check(const MatchFinder::MatchResult &Result) {
 const auto *TU = Result.Nodes.getNodeAs<TranslationUnitDecl>("tu");
 if (!TU) return;
 auto LimitedVisibility = anyOf(
     hasAncestor(namespaceDecl(isAnonymous())),
     hasAncestor(functionDecl())
 );
 // 1. Find the target declarations
 auto TypeMatcher = namedDecl(
     anyOf(typeAliasDecl(), typedefDecl(),
           cxxRecordDecl(isDefinition(), unless(isLambda())),
           enumDecl(isDefinition())),
     LimitedVisibility,
     unless(isImplicit()),
     unless(hasAttr(attr::Unused))
 ).bind("type");
 auto DeclMatches = match(findAll(TypeMatcher), *TU, *Result.Context);
 // 2. Scan the file for usages using our visitor
 llvm::SmallPtrSet<const NamedDecl*, 16> ValidUsages;
 UsageVisitor Visitor(ValidUsages);
 Visitor.TraverseDecl(const_cast<TranslationUnitDecl*>(TU));
 // 3. Flag unused types
 for (const auto &M : DeclMatches) {
   const auto *TD = M.getNodeAs<NamedDecl>("type");
   if (!TD) continue;
   const NamedDecl *Canonical = cast<NamedDecl>(TD->getCanonicalDecl());
   if (!ValidUsages.count(Canonical)) {
     diag(TD->getBeginLoc(), "Rule HSCAA.2.3: Types with limited visibility should be used at least once");
   }
 }
}
} // namespace clang::tidy::hsc
