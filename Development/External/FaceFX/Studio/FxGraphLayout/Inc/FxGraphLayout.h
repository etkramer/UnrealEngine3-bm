//------------------------------------------------------------------------------
// The DLL wrapper for the graph layout algorithm.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxGraphLayout_H__
#define FxGraphLayout_H__

#define FX_GRAPH_LAYOUT_API __declspec( dllexport )

#include "FxFaceGraph.h"

extern "C"
{

// Runs the layout algorithm on the face graph.
FX_GRAPH_LAYOUT_API void DoLayout( OC3Ent::Face::FxFaceGraph* pFaceGraph );

}

#endif
