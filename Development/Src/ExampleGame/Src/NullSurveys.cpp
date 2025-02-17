#include "ExampleGame.h"

#if WITH_SURVEYS

void appSurveyHookInit(void *pDevice) {}
void appSurveyHookRender() {}
void appSurveyHookGetInput() {}
void appSurveyHookShow(const wchar_t* wcsQuestionID, const wchar_t* wcsContext) {}
bool appSurveyHookIsVisible() { return false; }

#endif // WITH_SURVEYS
