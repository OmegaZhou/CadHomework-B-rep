#pragma once
#include "entity.h"

template<class T>
void circle_insert(T* loc, T* v)
{
	if (loc) {
		v->next = loc->next;
		v->pre = loc;
		loc->next = v;
		if (v->next->pre == loc) {
			v->next->pre = v;
		}
	} else {
		v->next = v->pre = v;
	}
	
}
template<class T>
void insert(T* loc, T* v)
{
	if (loc) {
		v->next = loc->next;
		v->pre = loc;
		loc->next = v;
	} else {
		v->next = v->pre = nullptr;
	}

}
void mvfs(Vertex*& v, Loop*& l, Face*& f, Solid*& s);
void mev(Vertex* old_v, Vertex*& new_v, Loop* l);
void mef(Vertex* old_v, Vertex* new_v, Loop*& new_l, Face*& f, Loop* old_l); 
void kemr(Vertex* s_v, Vertex* e_v, Loop* old_l, Loop*& new_l);
void kfmrh(Face* old_f, Face* new_face);