#pragma once
#include "entity.h"

Solid* createFace(Point* p, int n);
// must only have one face
void addRing(Solid* s, Point* p, int n);
void sweep(Solid* f, const Vec3& dir, Real d);