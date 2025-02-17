#ifndef CIRCO_H
#define CIRCO_H

#include "render.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void circo_layout(Agraph_t *g);
extern void circoLayout(Agraph_t *g);
extern void circo_cleanup(Agraph_t* g);
extern void circo_nodesize(node_t* n, boolean flip);
extern void circo_init_graph(graph_t *g);

#ifdef __cplusplus
}
#endif

#endif
