#pragma once

// 设备渲染
#include "Config.h"

class Transform;
struct Vertex;
struct Vector;
struct Color;
struct Texcoord;
struct Light;

class Device
{
public:
	Device();

	void init(int w, int h, uint32* fb, Transform* ts,int **tex,Light* light);
	void clear();
	void close();

	void setState(int s);
	void autoChangeState();
	void autoChangeInterp();
	void autoChangeCullMode();
	void autoChangeLightMode();
	//v2_带颜色
	void drawPoint(const Vector& p,const Vector& normal, const Texcoord& tc,const Color& color);
	void drawLine(const Vector& p1, const Vector& p2);
	void drawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3);
	void drawLine(const Vertex& v1, const Vertex& v2);

	void drawTriangle_2(const Vertex& v1, const Vertex& v2, const Vertex& v3);

private:
	Transform* transform;
	Light* light;
	int** textures;
	uint32 ** framebuffer;
	float * zbuffer;
	uint32 background; // 背景颜色
	uint32 foreground; // 线框颜色
	int width;
	int height;
	int state;
	int interp;
	int cullmode;
	int lightmode;
};