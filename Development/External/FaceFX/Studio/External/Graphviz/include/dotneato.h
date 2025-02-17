#ifndef DOTNEATO_H
#define DOTNEATO_H

/* FIXME - these shouldn't be needed */
#define ENABLE_CODEGENS 1
#define GD_RENDER 1

#include <dot.h>
#include <neato.h>
#include <circle.h>
#include <fdp.h>
#include <circo.h>

#ifdef __cplusplus
extern "C" {
#endif

extern GVC_t *gvContext();

#ifdef __cplusplus
}
#endif

#endif
