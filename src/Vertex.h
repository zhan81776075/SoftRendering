#pragma once

#include "math.h"

struct Color { float r; float g; float b; };
struct Texcoord { float u; float v; };

struct Vertex
{
	//空间坐标
	Vector pos;
	//颜色
	Color color;

	//纹理坐标
	Texcoord tex;
	
	//法向量
	Vector normal;
	float rhw; // 透视矫正
};