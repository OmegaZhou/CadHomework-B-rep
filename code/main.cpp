#include "data_bind.h"
#include "euler_operator.h"
#include "model_api.h"
#include "Display\display.h"
#include <cstdio>
void printLoop(Loop* loop)
{
	auto p = loop->first_e;
	if (loop->type == Unhole) {
		printf("Loop: ");
	} else {
		printf("Hole: ");
	}
	do {
		auto& v = *(p->start_v->p);
		printf("(%lf, %lf, %lf) ", v.x, v.y, v.z);
		p = p->next;
	} while (p != loop->first_e);
	printf("\n");
}

void printSolid(Solid* s)
{
	auto f = s->first_face;
	int i = 0;
	while (f) {
		auto l = f->first_loop;
		printf("Face %d\n", i);
		while (l) {
			printLoop(l);
			l = l->next;
		}
		printf("\n");
		f = f->next;
		++i;
	}
}
int main()
{

	mainLoop();
	//Point points[8] = { {0,0,0},{1,0,0},{1,0,1},{0,0,1},{0,1,0},{1,1,0},{0,1,1},{1,1,1} };
	//Point hole[8] = { {0.3,0,0.3},{0.6,0,0.3},{0.6,0,0.6},{0.3,0,0.6},{0.3,1,0.3},{0.6,1,0.3},{0.3,1,0.6},{0.6,1,0.6} };
	//Solid* s;
	//Loop* l[6];
	//Face* f[6];
	//Vertex* v[8];
	//s = createFace(points, 4);
	//addRing(s, hole, 4);
	//sweep(s, { 0,1,0 }, 1);

	//mvfs(v[0], *l, *f, s);
	//bindPointToVertex(points, v[0]);

	//mev(v[0], v[1], *l);
	//bindPointToVertex(points + 1, v[1]);
	//
	//mev(v[1], v[2], *l);
	//bindPointToVertex(points + 3, v[2]);
	//
	//mev(v[2], v[3], *l);
	//bindPointToVertex(points + 2, v[3]);
	//
	//
	//mef(v[3], v[0], l[1], f[1], l[0]);
	////printLoop(l[1]);

	//mev(v[0], v[4], l[1]);
	//bindPointToVertex(points+4, v[4]);

	//mev(v[1], v[5], l[1]);
	//bindPointToVertex(points + 5, v[5]);

	//mev(v[2], v[6], l[1]);
	//bindPointToVertex(points + 7, v[6]);

	//mev(v[3], v[7], l[1]);
	//bindPointToVertex(points + 6, v[7]);

	//mef(v[4], v[5], l[2], f[2], l[1]);
	//mef(v[5], v[6], l[3], f[3], l[2]);
	//mef(v[6], v[7], l[4], f[4], l[3]);
	////printLoop(l[4]);
	//mef(v[7], v[4], l[5], f[5], l[4]);

	//Loop* hl[6];
	//Face* hf[6];
	//Vertex* hv[8];
	//mev(v[0], hv[0], l[0]);
	//bindPointToVertex(hole, hv[0]);
	//kemr(v[0], hv[0], l[0], hl[0]);
	//bindHoleType(hl[0], Hole);

	//mev(hv[0], hv[1], *hl);
	//bindPointToVertex(hole + 1, hv[1]);

	//mev(hv[1], hv[2], *hl);
	//bindPointToVertex(hole + 3, hv[2]);

	//mev(hv[2], hv[3], *hl);
	//bindPointToVertex(hole + 2, hv[3]);


	//mef(hv[3], hv[0], hl[1], hf[1], hl[0]);
	////printLoop(l[1]);

	//mev(hv[0], hv[4], hl[1]);
	//bindPointToVertex(hole + 4, hv[4]);

	//mev(hv[1], hv[5], hl[1]);
	//bindPointToVertex(hole + 5, hv[5]);

	//mev(hv[2], hv[6], hl[1]);
	//bindPointToVertex(hole + 7, hv[6]);

	//mev(hv[3], hv[7], hl[1]);
	//bindPointToVertex(hole + 6, hv[7]);

	//mef(hv[4], hv[5], hl[2], hf[2], hl[1]);
	//mef(hv[5], hv[6], hl[3], hf[3], hl[2]);
	//mef(hv[6], hv[7], hl[4], hf[4], hl[3]);
	////printLoop(l[4]);
	//mef(hv[7], hv[4], hl[5], hf[5], hl[4]);
	//bindHoleType(hl[5], Hole);
	//kfmrh(hf[5], f[5]);
	

	//printSolid(s);
	return 0;
}