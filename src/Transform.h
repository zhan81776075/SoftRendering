#pragma once

#pragma once

// ����任

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
	Matrix world; //��������任
	Matrix view;  //���������任
	Matrix projection; //ͶӰ�任
	Matrix transform;  //transform = world * view * projection
	float width, height; // screen
};
