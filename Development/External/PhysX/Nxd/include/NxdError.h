#ifndef NXD_ERROR_H
#define NXD_ERROR_H

#if _MSC_VER < 1400
#define _snprintf_s _snprintf
#endif

#ifdef __CELLOS_LV2__
#define _snprintf snprintf
#endif

void NxdReportError(const char *z, const char *file, unsigned int line);
void NxdReportWarning(const char *z, const char *file, unsigned int line);

#endif
