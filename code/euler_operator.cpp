#include "euler_operator.h"
static void createEdge(HalfEdge*& e1, HalfEdge*& e2, Vertex* s, Vertex* e)
{
	e1 = new HalfEdge();
	e2 = new HalfEdge();

	Edge* edge = new Edge();
	edge->pre = edge->next = nullptr;
	edge->e[0] = e1;
	edge->e[1] = e2;

	e1->start_v = e2->end_v = s;
	e1->end_v = e2->start_v = e;
	e1->pre = e1->next = e1;
	e2->pre = e2->next = e2;
	e1->edge = e2->edge = edge;

	e->edge = e1;
}
void mvfs(Vertex*& v, Loop*& l, Face*& f, Solid*& s)
{
	s = new Solid();
	v = new Vertex();
	l = new Loop();
	f = new Face();
	s->first_face = f;
	s->pre = s->next = nullptr;

	f->first_loop = l;
	f->pre = f->next = nullptr;
	f->solid = s;

	l->face = f;
	l->first_e = nullptr;
	l->pre = l->next = nullptr;
}

void mev(Vertex* old_v, Vertex*& new_v, Loop* l)
{
	new_v = new Vertex();
	HalfEdge* e1,* e2;
	createEdge(e1, e2, old_v, new_v);

	e1->loop = e2->loop = l;
	if (!(l->first_e)) {
		l->first_e = e1;
	} else {
		auto p = l->first_e;
		while (p->end_v != old_v) {
			p = p->next;
		}
		circle_insert(p, e1);
	}
	circle_insert(e1, e2);
}

void mef(Vertex* s_v, Vertex* e_v, Loop*& l, Face*& f, Loop* old_l)
{
	l = new Loop();
	f = new Face();
	l->pre = l->next = nullptr;
	l->face = f;
	f->pre = f->next = nullptr;
	f->first_loop = l;
	f->solid = old_l->face->solid;
	auto f_p = f->solid->first_face;
	while (f_p->next) {
		f_p = f_p->next;
	}
	insert(f_p, f);

	auto s_e = old_l->first_e;
	while (s_e) {
		if (s_e->start_v == e_v) {
			break;
		}
		s_e = s_e->next;
	}

	HalfEdge* e1, * e2;
	createEdge(e1, e2, s_v, e_v);
	e1->loop = e2->loop = old_l;
	auto e_p = old_l->first_e;
	while (e_p->end_v != s_v) {
		e_p = e_p->next;
	}
	circle_insert(e_p, e1);
	circle_insert(e1, e2);
	
	
	auto e_e = e1;
	s_e->pre->next = e_e->next;
	e_e->next->pre = s_e->pre;
	auto new_first_e = e2;

	old_l->first_e = s_e;
	s_e->pre = e_e;
	e_e->next = s_e;
	l->first_e = new_first_e;
	auto start = new_first_e;
	do {
		new_first_e->loop = l;
		new_first_e = new_first_e->next;
	} while (start != new_first_e);
}

void kemr(Vertex* s_v, Vertex* e_v, Loop* old_l, Loop*& new_l)
{
	new_l = new Loop();
	new_l->face = old_l->face;
	{
		auto p = old_l->face->first_loop;
		while (p->next) {
			p = p->next;
		}
		insert(p, new_l);
	}
	HalfEdge* e1 = old_l->first_e;
	for (;;) {
		if (e1->start_v == s_v && e1->end_v == e_v) {
			break;
		}
		e1 = e1->next;
	}
	HalfEdge* e2 = e1;
	for (;;) {
		if (e2->start_v == e_v && e2->end_v == s_v) {
			break;
		}
		e2 = e2->next;
	}
	e1->pre->next = e2->next;
	e2->next->pre = e1->pre;
	e1->next->pre = e2->pre;
	e2->pre->next = e1->next;
	new_l->first_e = e1->next;
	if (new_l->first_e == e1 || new_l->first_e == e2) {
		new_l->first_e = nullptr;
	}
	{
		auto p = new_l->first_e;
		while (p&&p != e2) {
			p->loop = new_l;
			p = p->next;
		}
	}
	

	auto edge = e1->edge;
	delete edge;
	delete e1;
	delete e2;
	
}

void kfmrh(Face* old_f, Face* new_face)
{
	auto p = new_face->first_loop;
	while (p->next) {
		p = p->next;
	}
	insert(p, old_f->first_loop);
	if (old_f->pre) {
		old_f->pre->next = old_f ->next;
	}
	if (old_f->next) {
		old_f->next->pre = old_f->pre;
	}
	delete old_f;
}
