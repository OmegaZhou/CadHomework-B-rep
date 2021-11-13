#pragma once
typedef double Real;
struct Edge;
struct HalfEdge;
struct Loop;
struct Face;
struct Solid;

enum LoopType
{
	Unhole, Hole
};

struct Point
{
	Real x;
	Real y;
	Real z;
};
typedef Point Vec3;
struct  Vertex
{
	Point* p;
	Vertex* pre;
	Vertex* next;
	HalfEdge* edge;
};

struct HalfEdge
{
	Vertex* start_v;
	Vertex* end_v;
	HalfEdge* pre;
	HalfEdge* next;
	Edge* edge;
	Loop* loop;
};

struct  Edge
{
	HalfEdge* e[2];
	Edge* pre;
	Edge* next;
};

struct Loop
{
	HalfEdge* first_e;
	Loop* pre;
	Loop* next;
	Face* face;
	LoopType type;
};

struct Face
{
	Loop* first_loop;
	Face* pre;
	Face* next;
	Solid* solid;
};

struct Solid
{
	Solid* pre;
	Solid* next;
	Face* first_face;
};