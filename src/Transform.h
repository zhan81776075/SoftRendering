#pragma once

#pragma once

// 坐标变换

#include "math.h"

class Transform
{
public:
	void init(int width, int height);
	void update();
	void apply(Vector& b, const Vector& a);
	void applyMV(Vector& b, const Vector& a);
	void homogenize(Vector& b, const Vector& a);
	int checkCvv(const Vector& v);
	int checkBackCulling(const Vector& v);

	void setWorld(const Matrix& m);
	void setView(const Matrix& m);

private:
	Matrix world; //世界坐标变换
	Matrix view;  //摄像机坐标变换
	Matrix projection; //投影变换
	Matrix transform;  //transform = world * view * projection
	float width, height; // screen
};
