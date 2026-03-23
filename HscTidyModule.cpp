#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "AAOneTwoCheck.h"
#include "AATwoFourCheck.h"
#include "AATwoOneCheck.h"
#include "AATwoThreeCheck.h"
#include "AATwoTwoCheck.h"
#include "AGEightFourCheck.h"
#include "AGEightThreeCheck.h"
#include "AGFourThreeCheck.h"
#include "AGFourTwoCheck.h"
#include "AIEighteenOneCheck.h"
#include "AITwoFiveCheck.h"
#include "AITwoOneCheck.h"
#include "AITwoSixCheck.h"
#include "AITwoThreeCheck.h"
#include "AITwoTwoCheck.h"
#include "AJFourOneCheck.h"
#include "ANOneTwoCheck.h"
#include "ANThreeFourCheck.h"
#include "ANThreeOneCheck.h"
#include "ANThreeTwoCheck.h"
#include "APOneFourCheck.h"
#include "APOneOneCheck.h"
#include "APOneThreeCheck.h"
#include "APOneTwoCheck.h"
#include "APZeroTwoCheck.h"
#include "AREightOneCheck.h"
#include "ASFourOneCheck.h"
#include "ASOneOneCheck.h"
#include "AVSixThreeCheck.h"
#include "AVSixTwoCheck.h"
#include "BCSixThreeCheck.h"
[span_0](start_span)#include "UnusedReturnCheck.h" // Rule HSCAA.1.2[span_0](end_span)
[span_1](start_span)#include "UnusedVariableCheck.h" // Rule HSCAA.2.1[span_1](end_span)
namespace clang::tidy {
namespace hsc {
class HSCModule : public ClangTidyModule {
public:
 void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
   // Mapping the guideline strings to your C++ classes
    CheckFactories.registerCheck<AAOneTwoCheck>(
        "hsc-a-a-one-two");
    CheckFactories.registerCheck<AATwoFourCheck>(
        "hsc-a-a-two-four");
    CheckFactories.registerCheck<AATwoOneCheck>(
        "hsc-a-a-two-one");
    CheckFactories.registerCheck<AATwoThreeCheck>(
        "hsc-a-a-two-three");
    CheckFactories.registerCheck<AATwoTwoCheck>(
        "hsc-a-a-two-two");
    CheckFactories.registerCheck<AGEightFourCheck>(
        "hsc-a-g-eight-four");
    CheckFactories.registerCheck<AGEightThreeCheck>(
        "hsc-a-g-eight-three");
    CheckFactories.registerCheck<AGFourThreeCheck>(
        "hsc-a-g-four-three");
    CheckFactories.registerCheck<AGFourTwoCheck>(
        "hsc-a-g-four-two");
    CheckFactories.registerCheck<AIEighteenOneCheck>(
        "hsc-a-i-eighteen-one");
    CheckFactories.registerCheck<AITwoFiveCheck>(
        "hsc-a-i-two-five");
    CheckFactories.registerCheck<AITwoOneCheck>(
        "hsc-a-i-two-one");
    CheckFactories.registerCheck<AITwoSixCheck>(
        "hsc-a-i-two-six");
    CheckFactories.registerCheck<AITwoThreeCheck>(
        "hsc-a-i-two-three");
    CheckFactories.registerCheck<AITwoTwoCheck>(
        "hsc-a-i-two-two");
    CheckFactories.registerCheck<AJFourOneCheck>(
        "hsc-a-j-four-one");
    CheckFactories.registerCheck<ANOneTwoCheck>(
        "hsc-a-n-one-two");
    CheckFactories.registerCheck<ANThreeFourCheck>(
        "hsc-a-n-three-four");
    CheckFactories.registerCheck<ANThreeOneCheck>(
        "hsc-a-n-three-one");
    CheckFactories.registerCheck<ANThreeTwoCheck>(
        "hsc-a-n-three-two");
    CheckFactories.registerCheck<APOneFourCheck>(
        "hsc-a-p-one-four");
    CheckFactories.registerCheck<APOneOneCheck>(
        "hsc-a-p-one-one");
    CheckFactories.registerCheck<APOneThreeCheck>(
        "hsc-a-p-one-three");
    CheckFactories.registerCheck<APOneTwoCheck>(
        "hsc-a-p-one-two");
    CheckFactories.registerCheck<APZeroTwoCheck>(
        "hsc-a-p-zero-two");
    CheckFactories.registerCheck<AREightOneCheck>(
        "hsc-a-r-eight-one");
    CheckFactories.registerCheck<ASFourOneCheck>(
        "hsc-a-s-four-one");
    CheckFactories.registerCheck<ASOneOneCheck>(
        "hsc-a-s-one-one");
    CheckFactories.registerCheck<AVSixThreeCheck>(
        "hsc-a-v-six-three");
    CheckFactories.registerCheck<AVSixTwoCheck>(
        "hsc-a-v-six-two");
    CheckFactories.registerCheck<BCSixThreeCheck>(
        "hsc-b-c-six-three");
   CheckFactories.registerCheck<UnusedReturnCheck>(
       "hsc-unused-return-value");
   CheckFactories.registerCheck<UnusedVariableCheck>(
       "hsc-unused-variable");
 }
};
// Register the module with the Clang-Tidy engine
static ClangTidyModuleRegistry::Add<HSCModule>
   X("hsc-module", "Adds checks for HSC guidelines.");
} // namespace hsc
// Anchor to ensure the module is linked into the clang-tidy binary
volatile int HSCModuleAnchorSource = 0;
} // namespace clang::tidy
