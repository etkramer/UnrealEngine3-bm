/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/sw/tools/graphviz/license/source.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#ifndef         NEATO_H
#define         NEATO_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MODEL_SHORTPATH      0
#define MODEL_CIRCUIT        1
#define MODEL_SUBSET         2

#define MODE_KK          0
#define MODE_MAJOR       1

#define INIT_SELF        0
#define INIT_REGULAR     1
#define INIT_RANDOM      2

#include	"render.h"
#include	"pathplan.h"
#include	"neatoprocs.h"
#include	"adjust.h"

#endif /* NEATO_H */
