#pragma once
#include "../entity.h"
#include "ImageTool\ImageBuffer.hpp"
#include "glm\glm.hpp"
#include <vector>
class Display
{
public:
	Display();
	~Display();
	ZLib::ImageBuffer buffer;
	Real* depth;
	int depth_l;
	std::vector<ZLib::Color> colors;
	void display(Solid* s, int w, int h, const glm::dmat4& m);
	void display(Face* f, const glm::dmat4& m, int i);
};
void mainLoop();