#include "display.h"
#include "ZWindow\ZWindow.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "../model_api.h"
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <set>
#include <iostream>
using namespace ZLib;
using glm::dmat4;
using glm::dvec4;
using glm::dvec3;
using glm::cross;
using glm::normalize;
using std::vector;
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min

#define EPSION 1e-4
glm::dvec3 getNormal(const dvec4& a0, const dvec4& b0, const dvec4& c0)
{
	dvec3 a(a0.x, a0.y, a0.z);
	dvec3 b(b0.x, b0.y, b0.z);
	dvec3 c(c0.x, c0.y, c0.z);
	return normalize(cross((b - a), (c - a)));
}

Real getZ(const dvec3& n,const dvec4& p, Real x, Real y)
{
	return p.z - (n.x * (x - p.x) + n.y * (y - p.y)) / n.z;
}

dvec4 operator*(const dmat4& m, const Point& p)
{
	return m * dvec4(p.x, p.y, p.z, 1.0);
}
bool isInside(const vector<vector<dvec4>>& v, Real x, Real y)
{
	int l = 0;
	int r = 0;
	for (int i = 0; i < v.size(); ++i) {
		for (int j = 0; j < v[i].size(); ++j) {
			
			auto& p0 = v[i][j];
			auto& p1 = v[i][(j + 1) % v[i].size()];
			Real x0 = p0.x, x1 = p1.x, y0 = p0.y, y1 = p1.y;
			if (y0 > y1) {
				std::swap(y0, y1);
				std::swap(x0, x1);
			}
			if (!(y >= y0 && y <= y1)) {
				continue;
			}
			if (y0 == y1) {
				if (x >= std::min(x0, x1) && x <= std::max(x0, x1)) {
					return true;
				} else {
					continue;
				}
			}
			dvec3 a(x1 - x, y1 - y, 0);
			dvec3 b(x0 - x, y0 - y, 0);
			auto n = cross(a, b);
			if (n.z > 0) {
				++l;
			} else {
				++r;
			}
		}
	}
	if (l % 2 && r % 2) {
		return true;
	}
	return false;
}
Display::Display(): buffer(128,128)
{
	depth_l = 128 * 128;
	depth = new Real[depth_l];
	colors.push_back(Red);
	colors.push_back(Green);
	colors.push_back(Blue);
	colors.push_back({ 255,255,0,255 });
	colors.push_back({ 255,0,255,255 });
	colors.push_back({ 0,255,255,255 });
}

Display::~Display()
{
	delete[] depth;
}

void Display::display(Solid* s, int w, int h, const dmat4& m)
{
	if (depth_l < w * h) {
		delete[] depth;
		depth_l = w * h;
		depth = new Real[depth_l];
	}
	buffer.resize(w, h);
	for (int i = 0; i < w; ++i) {
		for (int j = 0; j < h; ++j) {
			buffer.setColor(i, j, White);
		}
	}
	if (!s) {
		return;
	}
	for (int i = 0; i < depth_l; ++i) {
		depth[i] = 2;
	}
	auto f = s->first_face;
	int index = 0;
	while (f) {
		display(f, m, index % colors.size());
		++index;
		f = f->next;
	}
}

void Display::display(Face* f, const glm::dmat4& m, int ci)
{
	vector<vector<dvec4>> v;
	auto loop = f->first_loop;
	Real x0 = INFINITY, x1 = -INFINITY, y0 = INFINITY, y1 = -INFINITY;
	int w = buffer.getWidth();
	int h = buffer.getHeight();
	while (loop) {
		vector<dvec4> tmp;
		auto first = loop->first_e;
		auto e = first;
		do {
			e = e->next;
			auto p = m * (*(e->start_v->p));
			p /= p.w;
			tmp.push_back(p);
			x0 = std::min(x0, p.x);
			x1 = std::max(x1, p.x);
			y0 = std::min(y0, p.y);
			y1 = std::max(y1, p.y);
		} while (e != first);
		loop = loop->next;
		v.push_back(move(tmp));
	}
	auto n = getNormal(v[0][0], v[0][1], v[0][2]);
	if (std::abs(n.z) < 1e-4) {
		return;
	}

	int start_x = std::max(0, (int)((x0 + 1) * w / 2) - 1);
	int end_x = std::min(w - 1, (int)((x1 + 1) * w / 2) + 1);
	int start_y = std::max(0, (int)((y0 + 1) * h / 2) - 1);
	int end_y = std::min(w - 1, (int)((y1 + 1) * h / 2) + 1);
	for (int i = start_x; i <= end_x; ++i) {
		for (int j = start_y; j <= end_y; ++j) {
			Real x = 2.0 * i / w - 1.0;
			Real y = 2.0 * j / h - 1.0;
			int index = buffer.getIndex(i, j);
			Real z = getZ(n, v[0][0], x, y);
			if (z < depth[index]&&isInside(v,x,y)) {
				depth[index] = z;
				Color& color = colors[ci];
				Color c;
				Real k = -z * 0.5 + 0.5;
				c.a = 255;
				c.r = color.r * std::abs(n.z);
				c.g = color.g * std::abs(n.z);
				c.b = color.b * std::abs(n.z);
				buffer.setColor(i, j, c);
			}
		}
	}
}
Solid* s = nullptr;
dmat4 m(1.0);
dmat4 r(1.0);
Real l;
vector<Point*> p_vec;
void freeMemory()
{
	if (!s) {
		return;
	}
	for (auto p : p_vec) {
		delete[] p;
	}
	p_vec.clear();
	std::set<Vertex*> vset;
	std::set<Edge*> eset;
	std::set<HalfEdge*> heset;
	auto f = s->first_face;
	while (f) {
		auto l = f->first_loop;
		while (l) {
			auto start = l->first_e;
			auto e = start;
			do {
				vset.insert(e->start_v);
				heset.insert(e);
				eset.insert(e->edge);
			} while (e != start);
			auto tmp = l;
			l = l->next;
			delete tmp;
		}
		auto tmp = f;
		f = f->next;
		delete tmp;
	}
	for (auto k : vset) {
		delete k;
	}
	for (auto k : heset) {
		delete k;
	}
	for (auto k : eset) {
		delete k;
	}
	delete s;
}
void loadSolid(const ZLib::String& path)
{
	freeMemory();
	std::ifstream is;
	is.open(path, std::ios::binary | std::ios::in);
	std::string str;
	char command[10];
	while (std::getline(is, str)) {
		const char* buffer = str.c_str();
		int offset;
 		sscanf(buffer, "%s%n", command, &offset);

		if (std::strcmp(command, "face")==0) {
			int c;
			vector<Point> v;
			Point p;
			while (sscanf(buffer + offset, " %lf, %lf, %lf%n", &p.x, &p.y, &p.z, &c) == 3) {
				v.push_back(p);
				offset += c;
			}
			Point* tmp_p = new Point[v.size()];
			for (int i = 0; i < v.size(); ++i) {
				tmp_p[i] = v[i];
			}
			p_vec.push_back(tmp_p);
			s = createFace(tmp_p, v.size());
		} else if (std::strcmp(command, "ring")==0) {
			int c;
			vector<Point> v;
			Point p;
			while (sscanf(buffer + offset, "%lf, %lf, %lf%n", &p.x, &p.y, &p.z, &c) == 3) {
				v.push_back(p);
				offset += c;
			}
			Point* tmp_p = new Point[v.size()];
			for (int i = 0; i < v.size(); ++i) {
				tmp_p[i] = v[i];
			}
			p_vec.push_back(tmp_p);
			addRing(s, tmp_p, v.size());
		} else if (std::strcmp(command, "sweep")==0) {
			int c;
			Vec3 dir;
			Real d;
			sscanf(buffer + offset, "%lf,%lf,%lf%n", &dir.x, &dir.y, &dir.z, &c);
			Real lenth = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
			dir.x /= lenth;
			dir.y /= lenth;
			dir.z /= lenth;
			offset += c;
			sscanf(buffer + offset, "%lf", &d);
			sweep(s, dir, d);
		}
	}
	is.close();
}

void setM()
{
	if (!s) {
		return;
	}
	auto f = s->first_face;
	dvec3 center;
	Real x0 = INFINITY, x1 = -INFINITY, y0 = INFINITY, y1 = -INFINITY, z0 = INFINITY, z1 = -INFINITY;
	while (f) {
		auto loop = f->first_loop;
		while (loop) {
			auto start = loop->first_e;
			auto e = start;
			do {
				auto p = r * (*(e->start_v->p));
				x0 = std::min(x0, p.x);
				x1 = std::max(x1, p.x);
				y0 = std::min(y0, p.y);
				y1 = std::max(y1, p.y);
				z0 = std::min(z0, p.z);
				z1 = std::max(z1, p.z);
				e = e->next;
			} while (start != e);
			loop = loop->next;
		}
		f = f->next;
	}
	dvec3 box_l(x1 - x0, y1 - y0, z1 - z0);
	//Real l = glm::length(box_l);
	center = dvec3((x1 + x0) / 2, (y0 + y1) / 2, (z0 + z1) / 2);
	auto eye = center;
	eye.z += l / 2;
	m = glm::ortho(- l / 2, l / 2,  - l / 2,  l / 2, 0.0 , l) * glm::lookAt(eye, center, dvec3(0, 1, 0));
}

void addR(Real angle,const dvec3& axis)
{
	auto tmp = dmat4(1.0);
	r = glm::rotate(tmp, angle, axis) * r;
}

class DrawCallBack :public Callback
{
public:
	LRESULT operator()(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		if (message == WM_TIMER) {
			addR(1.0 / 180 * 3.1415926, dvec3(0, 1, 0));
			setM();
		}
		auto& window = ZWindow::getWindow(hwnd);
		auto& config = window.getConfig();
		display.display(s, config.width, config.height, m*r);
		window.draw(display.buffer, message);
		return 0;
	};
	Display display;
};
LRESULT KeyCallback(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	int times = 0xff & lparam;
	switch (wparam) {
	case 'A':
		addR(-times / 180.0 * 3.1415926, dvec3(0, 1, 0));
		break;
	case 'D':
		addR(times / 180.0 * 3.1415926, dvec3(0, 1, 0));
		break;
	case 'W':
		addR(-times / 180.0 * 3.1415926, dvec3(1, 0, 0));
		break;
	case 'S':
		addR(times / 180.0 * 3.1415926, dvec3(1, 0, 0));
		break;
	default:
		return 0;
		break;
	}
	setM();
	InvalidateRect(hwnd, NULL, false);
}
LRESULT LoadCallback(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	auto re = getFilePath(hwnd, true);
	if (!re.empty()) {
		loadSolid(re[0]);
		{
			auto f = s->first_face;
			Real x0 = INFINITY, x1 = -INFINITY, y0 = INFINITY, y1 = -INFINITY, z0 = INFINITY, z1 = -INFINITY;
			while (f) {
				auto loop = f->first_loop;
				while (loop) {
					auto start = loop->first_e;
					auto e = start;
					do {
						auto p = r * (*(e->start_v->p));
						x0 = std::min(x0, p.x);
						x1 = std::max(x1, p.x);
						y0 = std::min(y0, p.y);
						y1 = std::max(y1, p.y);
						z0 = std::min(z0, p.z);
						z1 = std::max(z1, p.z);
						e = e->next;
					} while (start != e);
					loop = loop->next;
				}
				f = f->next;
			}
			dvec3 box_l(x1 - x0, y1 - y0, z1 - z0);
			l = glm::length(box_l);
		}
		r = dmat4(1.0);
		setM();
		InvalidateRect(hwnd, NULL, false);
	}
	return 0;
}

class SaveCallback :public Callback
{
public:
	LRESULT operator()(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		auto& window = ZWindow::getWindow(hwnd);
		auto& config = window.getConfig();
		std::map<String, vector<String>> filter;
		filter[Z_STRING("图片文件(.bmp)")].push_back(Z_STRING("bmp"));
		auto re = getFilePath(hwnd, false, false, filter);
		if (!re.empty()) {
			callback->display.buffer.saveImage(re[0]);
		}
		
		return 0;
	};
	std::shared_ptr<DrawCallBack> callback;
};
void mainLoop()
{
	ZWindow& window = ZWindow::createWindow();
	auto& config = window.getConfig();
	config.window_name = Z_STRING("B_Rep");
	config.width = config.height = 720;
	//Point points[8] = { {0,0,0},{1,0,0},{1,0,1},{0,0,1},{0,1,0},{1,1,0},{0,1,1},{1,1,1} };
	//Point hole[8] = { {0.3,0,0.3},{0.6,0,0.3},{0.6,0,0.6},{0.3,0,0.6},{0.3,1,0.3},{0.6,1,0.3},{0.3,1,0.6},{0.6,1,0.6} };
	//s = createFace(points, 4);
	//addRing(s, hole, 4);
	//sweep(s, { 0,1,0 }, 1);
	//loadSolid(Z_STRING("solid.txt"));
	

	auto callback = std::make_shared<DrawCallBack>();
	auto save_callback = std::make_shared<SaveCallback>();
	save_callback->callback = callback;
	config.setDrawCallback(callback);
	config.setCallback(KeyCallback, WM_KEYDOWN);
	MenuInfo info("file", Z_STRING("选择文件"),MenuType::Click);
	MenuInfo info2("file2", Z_STRING("保存"), MenuType::Click);
	config.appendMenuItem(info, LoadCallback);
	config.appendMenuItem(info2, save_callback);
	window.start();
	//config.setTimerCallback(callback, DRAW_TIME_ID, 33);
	window.loop();
}
