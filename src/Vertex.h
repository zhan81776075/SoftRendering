#pragma once

#include "math.h"

struct Color { float r; float g; float b; };
struct Texcoord { float u; float v; };

struct Vertex
{
	//�ռ�����
	Vector pos;
	//��ɫ
	Color color;

	//��������
	Texcoord tex;
	
	//������
	Vector normal;
	float rhw; // ͸�ӽ���
};