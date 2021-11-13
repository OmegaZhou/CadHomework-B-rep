#include "model_api.h"
#include "euler_operator.h"
#include "data_bind.h"
#include <vector>
Solid* createFace(Point* p, int n)
{
    Solid* s;
    Face* f;
    Loop* l;
    Vertex* start;
    Vertex* last;
    Vertex* now;
    mvfs(start, l, f, s);
    bindPointToVertex(p, start);
    last = start;
    for (int i = 1; i < n; ++i) {
        mev(last, now, l);
        bindPointToVertex(p + i, now);
        last = now;
    }
    Loop* nl;
    mef(last, start, nl, f, l);
    return s;
}

void addRing(Solid* s, Point* p, int n)
{
    auto l = s->first_face->first_loop;
    auto v = l->first_e->start_v;
    Vertex* start, * last, * now;
    mev(v, start, l);
    bindPointToVertex(p, start);
    last = start;
    Loop* nl;
    kemr(v, start, l, nl);
    for (int i = 1; i < n; ++i) {
        mev(last, now, nl);
        bindPointToVertex(p + i, now);
        last = now;
    }
    Loop* nnl;
    Face* nf;
    mef(last, start, nnl, nf, nl);
    bindHoleType(nl, Hole);
}

void sweepLoop(Loop* bottom_l, Face*& top, Loop* now_l, const Vec3& dir, Real d)
{
    auto e = bottom_l->first_e;
    auto st_e = e;
    std::vector<Vertex*> vec;
    do {
        Point* np = new Point();
        Point* p = e->start_v->p;
        np->x = p->x + dir.x * d;
        np->y = p->y + dir.y * d;
        np->z = p->z + dir.z * d;
        Vertex* v;
        mev(e->start_v, v, now_l);
        bindPointToVertex(np, v);
        vec.push_back(v);
        e = e->next;
    } while (st_e != e);
    Loop* last = now_l;
    for (int i = 0; i < vec.size(); ++i) {
        auto e_v = vec[(i + 1) % vec.size()];
        mef(vec[i], e_v, now_l, top, last);
        last = now_l;
    }
}

void sweep(Solid* s, const Vec3& dir, Real d)
{
    Face* top;
    Face* f = s->first_face;
    Loop* l = f->first_loop;
    f = f->next;
    sweepLoop(l, top, f->first_loop, dir, d);
    l = l->next;
    f = f->next;
    while (l) {
        Face* t;
        sweepLoop(l, t, f->first_loop, dir, d);
        bindHoleType(t->first_loop, l->type);
        kfmrh(t, top);
        l = l->next;
        f = f->next;
    }

}

