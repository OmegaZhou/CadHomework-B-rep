#include "data_bind.h"

void bindPointToVertex(Point* p, Vertex* v)
{
	v->p = p;
}

void bindHoleType(Loop* l, LoopType type)
{
	l->type = type;
}
