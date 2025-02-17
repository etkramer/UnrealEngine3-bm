#ifndef _INC_UNSURVEYS
#define _INC_UNSURVEYS

#if WITH_SURVEYS

void appSurveyHookInit(void *pDevice);
void appSurveyHookRender();
void appSurveyHookGetInput();
void appSurveyHookShow(const wchar_t* wcsQuestionID, const wchar_t* wcsContext);
bool appSurveyHookIsVisible();

#else

inline void appSurveyHookInit(void *pDevice) {}
inline void appSurveyHookRender() {}
inline void appSurveyHookGetInput() {}
inline void appSurveyHookShow(const wchar_t* wcsQuestionID, const wchar_t* wcsContext) {}
inline bool appSurveyHookIsVisible() { return false; }

#endif

#endif
