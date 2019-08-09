#include "Device.h"
#include "Vertex.h"
#include "Transform.h"
#include "Light.h"
#include <Windows.h>
#include <math.h>


#define STATE_DRAW_LINE 0 //010
#define STATE_DRAW_COLOR 1 //100
#define STATE_DRAW_TEX 2 //001
#define STATE_DRAW_COLOR_GRAY 3

#define INTERP_NONE 0
#define INTERP_BILINEAR 1

#define CULL_MODE_NONE 0
#define CULL_MODE_BACK 1

#define LIGHT_MODE_OFF 0
#define LIGHT_MODE_ON  1

Device::Device() : transform(NULL), textures(NULL),framebuffer(NULL), zbuffer(NULL),
background(0), foreground(0), width(0), height(0), state(0), cullmode(0),lightmode(0)
{
}

void Device::init(int w, int h, uint32* fb, Transform* ts,int** tex,Light* l)
{
	width = w;
	height = h;

	// fb 为窗口显示缓存
	framebuffer = (uint32**)malloc(h * sizeof(uint32*));
	for (int y = 0; y < h; y++) {
		framebuffer[y] = fb + y * w;
	}

	zbuffer = (float*)malloc(w * h * sizeof(float));
	memset(zbuffer, 0, w * h * sizeof(float));

	background = 0xc0c0c0;
	foreground = 0xffffff;

	transform = ts;
	textures = tex;
	light = l;
}

void Device::clear()
{
	float inv_h = (float)1 / height;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int c = (float)0xc0 * inv_h * (height - y);
			framebuffer[y][x] = (c << 16 | c << 8 | c);

			zbuffer[y * width + x] = 1.f;
		}
	}
}
void Device::close()
{
	if (framebuffer != NULL) {
		free(framebuffer);
	}

	if (zbuffer != NULL) {
		free(zbuffer);
	}
}
void Device::setState(int s)
{
	state = s;
}

void Device::autoChangeState()
{
	state += 1;
}

//void Device::autoChangeInterp()
//{
//	if (interp == INTERP_NONE) {
//		interp = INTERP_BILINEAR;
//	}
//	else {
//		interp = INTERP_NONE;
//	}
//}
//

//
void Device::autoChangeCullMode()
{
	if (cullmode == CULL_MODE_NONE) {
		cullmode = CULL_MODE_BACK;
	}
	else {
		cullmode = CULL_MODE_NONE;
	}
}

void Device::autoChangeLightMode() {
	if (lightmode == LIGHT_MODE_OFF) {
		lightmode = LIGHT_MODE_ON;
	}
	else {
		lightmode = LIGHT_MODE_OFF;
	}
}
//}
//
int GetRealState(int s)
{
	s = s % 4;
	if (s == 0) {
		return STATE_DRAW_LINE;
	}
	else if (s == 1) {
		//100 | 010 = 110 = 6
		return STATE_DRAW_COLOR;
	}
	else if (s == 2) {
		//100 | 001 = 101 = 5
		return STATE_DRAW_TEX;
	}
	else if (s == 3) {
		return STATE_DRAW_COLOR_GRAY;
	}
	return 0;
}

void Device::drawPoint(const Vector& p,const Vector& normal,const Texcoord& tc,const Color& color)
{
	int y = (int)p.y;
	int x = (int)p.x;

	if (y < 0 || y >= height) return;
	if (x < 0 || x >= width) return;

	if (zbuffer[y * width + x] < p.z) return;

	int fcolor = 0;

	int s = GetRealState(state);

	if (s== STATE_DRAW_COLOR||s==STATE_DRAW_COLOR_GRAY) {
		int r = color.r * 255;
		int g = color.g * 255;
		int b = color.b * 255;

		fcolor = (r << 16) | (g << 8) | b;
	}
	else if(s==STATE_DRAW_TEX ){
		int *tex = textures[0];

		Color tex_color;
		if (interp == INTERP_NONE) {
			int i = int(tc.u * 100) * 100 + int(tc.v * 100);
			i = i >= 10000 ? 10000 - 1 : i;
			int c = tex[i];
			float inv = (float)1 / 255;
			tex_color = { (c >> 16) * inv, (c >> 8 & 0xff) * inv, (c & 0xff) * inv };
		}
		else {
			//tex_color = BilinearInterp(tex, tc.u, tc.v);
		}
		if (lightmode == LIGHT_MODE_ON) {
			// light
			float n_dot_l = VectorDotProduct(light->direction, normal);

			// 默认环境光
			Color ambient = { 1.0f, 1.0f, 1.0f };
			float intensity = 0.3;

			// 环境光的影响
			ambient.r *= intensity * tex_color.r;
			ambient.g *= intensity * tex_color.g;
			ambient.b *= intensity * tex_color.b;

			// blend
			Color diffuse = { 0.f, 0.f, 0.f };
			if (n_dot_l > 0) {
				diffuse.r = tex_color.r * n_dot_l;
				diffuse.g = tex_color.g * n_dot_l;
				diffuse.b = tex_color.b * n_dot_l;
			}

			ambient.r += diffuse.r;
			ambient.g += diffuse.g;
			ambient.b += diffuse.b;

			ambient.r = ambient.r > 1.f ? 1.f : ambient.r;
			ambient.g = ambient.g > 1.f ? 1.f : ambient.g;
			ambient.b = ambient.b > 1.f ? 1.f : ambient.b;

			fcolor = (int(ambient.r * 255) << 16 | int(ambient.g * 255) << 8 | int(ambient.b * 255));
		}
		else {
			fcolor = (int(tex_color.r * 255) << 16 | int(tex_color.g * 255) << 8 | int(tex_color.b * 255));
		}
		
	}
	framebuffer[y][x] = fcolor;
	zbuffer[y * width + x] = p.z;
}

void Device::drawLine(const Vector& p1, const Vector& p2)
{
	float inv = (float)1 / 255;
	Color color = { (foreground >> 16) * inv, (foreground >> 8 & 0xff) * inv, (foreground & 0xff) * inv };
	Texcoord tex = { 0.f, 0.f };
	Vector normal = { 0.f, 0.f, 0.f };

	int x1, y1, x2, y2;
	x1 = p1.x;
	y1 = p1.y;
	x2 = p2.x;
	y2 = p2.y;

	float y, x;
	y = p1.y;
	x = p1.x;

	if (x1 == x2 && y1 == y2) {
		drawPoint(p1, normal, tex,color);
	}
	else if (x1 == x2) {
		drawPoint(p1, normal, tex,color);

		int inc = (y1 < y2) ? 1 : -1;
		while (1) {
			y += inc;
			if (inc == 1 && y >= y2) break;
			if (inc == -1 && y <= y2) break;
			Vector p = { x, y, 0.f, 1.f };
			drawPoint(p, normal, tex,color);
		}

		drawPoint(p2,normal, tex, color);
	}
	else if (y1 == y2) {
		drawPoint(p1, normal, tex, color);

		int inc = (x1 < x2) ? 1 : -1;
		while (1) {
			x += inc;
			if (inc == 1 && x >= x2) break;
			if (inc == -1 && x <= x2) break;
			Vector p = { x, y, 0.f, 1.f };
			drawPoint(p, normal, tex,color);
		}

		drawPoint(p2, normal, tex,color);
	}
	else {
		drawPoint(p1,normal, tex,color);

		float t = (float)abs(x2 - x1) / abs(y2 - y1);
		int xinc = (p1.x < p2.x) ? 1 : -1;
		int yinc = (p1.y < p2.y) ? 1 : -1;
		while (1) {
			y += yinc;
			if (yinc == 1 && y >= y2) break;
			if (yinc == -1 && y <= y2) break;
			x += t * xinc;
			drawPoint({ x,y,0.f,1.f },normal, tex, color);
		}

		drawPoint(p2,normal, tex,color);
	}
}

int getTriangleInterp(const Vector& v1, const Vector& v2, const Vector& v3, const Vector& p, float *u, float *v)
{
	float a, b, c;

	float nv1 = ((v1.y - v2.y)*v3.x + (v2.x - v1.x)*v3.y + v1.x*v2.y - v2.x*v1.y);
	float nv2 = ((v1.y - v3.y)*v2.x + (v3.x - v1.x)*v2.y + v1.x*v3.y - v3.x*v1.y);

	if (nv1 == 0.f || nv2 == 0.f) {
		*u = 1.f; *v = 0.f;
		return 1;
	}

	c = ((v1.y - v2.y)*p.x + (v2.x - v1.x)*p.y + v1.x*v2.y - v2.x*v1.y) / nv1;
	if (c < 0 || c > 1) return 0;
	b = ((v1.y - v3.y)*p.x + (v3.x - v1.x)*p.y + v1.x*v3.y - v3.x*v1.y) / nv2;
	if (b < 0 || b > 1) return 0;
	a = 1 - b - c;
	if (a < 0 || a > 1) return 0;

	*u = a; *v = b;
	return 1;
}

//同号的点在三角形中，不同号的不在三角形中，为0的则在线上
int getTriangleInterp_2(const Vector& v1, const Vector& v2, const Vector& v3, const Vector& p)
{
	Vector P1, P2, P3;
	VectorSub(P1, p, v1);
	VectorSub(P2, p, v2);
	VectorSub(P3, p, v3);

	float a, b, c;

	a = P1.x*P2.y - P2.x*P1.y;
	b = P2.x*P3.y - P3.x*P2.y;
	c = P3.x*P1.y - P1.x*P3.y;
	
	if (a == 0 || b == 0 || c == 0)
		return 1;
	else if ((a > 0 && b > 0 && c > 0) || (a < 0 && b < 0 && c < 0))
		return 1;
	else
		return 0;
}

void VertexRhwInit(Vertex& v)
{
	v.rhw = 1 / v.pos.w;
	v.color.r *= v.rhw;
	v.color.g *= v.rhw;
	v.color.b *= v.rhw;
	v.tex.u *= v.rhw;
	v.tex.v *= v.rhw;
	v.normal.x *= v.rhw;
	v.normal.y *= v.rhw;
	v.normal.z *= v.rhw;
}

void Device::drawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	int s = GetRealState(state);
	if (s == STATE_DRAW_LINE) {
		Vector c1, c2, c3;
		Vector n1, n2, n3;

		transform->apply(c1, v1.pos);
		transform->apply(c2, v2.pos);
		transform->apply(c3, v3.pos);

		//检查是否越界
		if (transform->checkCvv(c1)) return;
		if (transform->checkCvv(c2)) return;
		if (transform->checkCvv(c3)) return;

		//if (cullmode == CULL_MODE_BACK) {
		//	Vector s1, s2, pn;
		//	VectorSub(s1, c2, c1);
		//	VectorSub(s2, c3, c2);
		//	VectorCrossProduct(pn, s2, s1);
		//	VectorNormalize(pn);
		//	if (transform->checkBackCulling(pn)) return;
		//}

		Vector p1, p2, p3, min, max;

		transform->homogenize(p1, c1);
		transform->homogenize(p2, c2);
		transform->homogenize(p3, c3);

		if (cullmode == CULL_MODE_BACK) {
			Vector s1, s2, pn;
			VectorSub(s1, p2, p1);
			VectorSub(s2, p3, p2);
			VectorCrossProduct(pn, s2, s1);
			VectorNormalize(pn);
			if (transform->checkBackCulling(pn)) return;
		}

		drawLine(p1, p2);
		drawLine(p1, p3);
		drawLine(p2, p3);
	}
	else if (s == STATE_DRAW_COLOR) {
		Vector c1, c2, c3;
		Vector n1, n2, n3;

		transform->apply(c1, v1.pos);
		transform->apply(c2, v2.pos);
		transform->apply(c3, v3.pos);

		//检查是否越界
		if (transform->checkCvv(c1)) return;
		if (transform->checkCvv(c2)) return;
		if (transform->checkCvv(c3)) return;

		Vector p1, p2, p3, min, max;

		transform->homogenize(p1, c1);
		transform->homogenize(p2, c2);
		transform->homogenize(p3, c3);

		if (cullmode == CULL_MODE_BACK) {
			Vector s1, s2, pn;
			VectorSub(s1, p2, p1);
			VectorSub(s2, p3, p2);
			VectorCrossProduct(pn, s2, s1);
			VectorNormalize(pn);
			if (transform->checkBackCulling(pn)) return;
		}

		min = p1; max = p1;

		if (p2.x < min.x) min.x = p2.x;
		if (p3.x < min.x) min.x = p3.x;
		if (p2.y < min.y) min.y = p2.y;
		if (p3.y < min.y) min.y = p3.y;

		if (p2.x > max.x) max.x = p2.x;
		if (p3.x > max.x) max.x = p3.x;
		if (p2.y > max.y) max.y = p2.y;
		if (p3.y > max.y) max.y = p3.y;

		Vertex t1 = v1, t2 = v2, t3 = v3;
		t1.pos = p1; t2.pos = p2; t3.pos = p3;
		t1.pos.w = c1.w; t2.pos.w = c2.w; t3.pos.w = c3.w;

		VertexRhwInit(t1);
		VertexRhwInit(t2);
		VertexRhwInit(t3);

		for (int j = min.y; j < max.y; j++) {
			for (int i = min.x; i < max.x; i++) {
				Vector p = { i,j,0.f,1.f };
				float c1, c2, c3;
				if (getTriangleInterp(p1, p2, p3, p, &c1, &c2)) {
					c3 = 1.f - c1 - c2;

					// Rectification for Perspective
					float w = 1 / (c1 * t1.rhw + c2 * t2.rhw + c3 * t3.rhw);

					float r = t1.color.r * c1 + t2.color.r * c2 + t3.color.r * c3;
					float g = t1.color.g * c1 + t2.color.g * c2 + t3.color.g * c3;
					float b = t1.color.b * c1 + t2.color.b * c2 + t3.color.b * c3;
				
					
					Vector normal = { 0, 0, 0 };

					float z = t1.pos.z * t1.rhw * c1 + t2.pos.z * t2.rhw * c2 + t3.pos.z * t3.rhw * c3;
					p.z = w * z;

					drawPoint(p, normal,{ 0, 0 } ,{ w * r, w * g, w * b });
				}
			}
		}
	}
	else if (s == STATE_DRAW_TEX) {
		Vector c1, c2, c3;
		Vector n1, n2, n3;

		//透视投影变换
		transform->apply(c1, v1.pos);
		transform->apply(c2, v2.pos);
		transform->apply(c3, v3.pos);

		//检查是否越界
		if (transform->checkCvv(c1)) return;
		if (transform->checkCvv(c2)) return;
		if (transform->checkCvv(c3)) return;

		transform->applyMV(n1, v1.normal);
		transform->applyMV(n2, v2.normal);
		transform->applyMV(n3, v3.normal);

		Vector p1, p2, p3, min, max;

		//到显示器坐标
		transform->homogenize(p1, c1);
		transform->homogenize(p2, c2);
		transform->homogenize(p3, c3);

		if (cullmode == CULL_MODE_BACK) {
			Vector s1, s2, pn;
			VectorSub(s1, p2, p1);
			VectorSub(s2, p3, p2);
			VectorCrossProduct(pn, s2, s1);
			VectorNormalize(pn);
			if (transform->checkBackCulling(pn)) return;
		}
		min = p1; max = p1;

		if (p2.x < min.x) min.x = p2.x;
		if (p3.x < min.x) min.x = p3.x;
		if (p2.y < min.y) min.y = p2.y;
		if (p3.y < min.y) min.y = p3.y;

		if (p2.x > max.x) max.x = p2.x;
		if (p3.x > max.x) max.x = p3.x;
		if (p2.y > max.y) max.y = p2.y;
		if (p3.y > max.y) max.y = p3.y;

		Vertex t1 = v1, t2 = v2, t3 = v3;
		t1.pos = p1; t2.pos = p2; t3.pos = p3;
		t1.normal = n1; t2.normal = n2; t3.normal = n3;
		t1.pos.w = c1.w; t2.pos.w = c2.w; t3.pos.w = c3.w;

		VertexRhwInit(t1);
		VertexRhwInit(t2);
		VertexRhwInit(t3);

		for (int j = min.y; j < max.y; j++) {
			for (int i = min.x; i < max.x; i++) {
				Vector p = { i,j,0.f,1.f };
				float c1, c2, c3;
				if (getTriangleInterp(p1, p2, p3, p, &c1, &c2)) {
					c3 = 1.f - c1 - c2;

					// Rectification for Perspective
					float w = 1 / (c1 * t1.rhw + c2 * t2.rhw + c3 * t3.rhw);

					float r = t1.color.r * c1 + t2.color.r * c2 + t3.color.r * c3;
					float g = t1.color.g * c1 + t2.color.g * c2 + t3.color.g * c3;
					float b = t1.color.b * c1 + t2.color.b * c2 + t3.color.b * c3;

					float u = t1.tex.u * c1 + t2.tex.u * c2 + t3.tex.u * c3;
					float v = t1.tex.v * c1 + t2.tex.v * c2 + t3.tex.v * c3;

					float nx = t1.normal.x * c1 + t2.normal.x * c2 + t3.normal.x * c3;
					float ny = t1.normal.y * c1 + t2.normal.y * c2 + t3.normal.y * c3;
					float nz = t1.normal.z * c1 + t2.normal.z * c2 + t3.normal.z * c3;
					Vector normal = { w*nx, w*ny, w*nz };
					VectorNormalize(normal);

					float z = t1.pos.z * t1.rhw * c1 + t2.pos.z * t2.rhw * c2 + t3.pos.z * t3.rhw * c3;
					p.z = w * z;

					drawPoint(p, normal, { w*u, w*v }, { w * r, w * g, w * b });
				}
			}
		}
	}
	else if (s == STATE_DRAW_COLOR_GRAY) {
		Vector c1, c2, c3;
		Vector n1, n2, n3;

		transform->apply(c1, v1.pos);
		transform->apply(c2, v2.pos);
		transform->apply(c3, v3.pos);

		//检查是否越界
		if (transform->checkCvv(c1)) return;
		if (transform->checkCvv(c2)) return;
		if (transform->checkCvv(c3)) return;

		Vector p1, p2, p3, min, max;

		transform->homogenize(p1, c1);
		transform->homogenize(p2, c2);
		transform->homogenize(p3, c3);

		if (cullmode == CULL_MODE_BACK) {
			Vector s1, s2, pn;
			VectorSub(s1, p2, p1);
			VectorSub(s2, p3, p2);
			VectorCrossProduct(pn, s2, s1);
			VectorNormalize(pn);
			if (transform->checkBackCulling(pn)) return;
		}

		min = p1; max = p1;

		if (p2.x < min.x) min.x = p2.x;
		if (p3.x < min.x) min.x = p3.x;
		if (p2.y < min.y) min.y = p2.y;
		if (p3.y < min.y) min.y = p3.y;

		if (p2.x > max.x) max.x = p2.x;
		if (p3.x > max.x) max.x = p3.x;
		if (p2.y > max.y) max.y = p2.y;
		if (p3.y > max.y) max.y = p3.y;

		Vertex t1 = v1, t2 = v2, t3 = v3;
		t1.pos = p1; t2.pos = p2; t3.pos = p3;
		t1.pos.w = c1.w; t2.pos.w = c2.w; t3.pos.w = c3.w;

		VertexRhwInit(t1);
		VertexRhwInit(t2);
		VertexRhwInit(t3);

		for (int j = min.y; j < max.y; j++) {
			for (int i = min.x; i < max.x; i++) {
				Vector p = { i,j,0.f,1.f };
				float c1, c2, c3;
				if (getTriangleInterp(p1, p2, p3, p, &c1, &c2)) {
					c3 = 1.f - c1 - c2;
					float r, g, b;
					if ((c1 == 1.f) && (c2 = 0.f)) {
						r = 1.0f;
						g = 0.0f;
						b = 0.0f;
					}
					else
					{
						r = 0.5f;
						g = 0.5f;
						b = 0.5f;
					}
					// Rectification for Perspective
					float w = 1 / (c1 * t1.rhw + c2 * t2.rhw + c3 * t3.rhw);

					Vector normal = { 0, 0, 0 };

					float z = t1.pos.z * t1.rhw * c1 + t2.pos.z * t2.rhw * c2 + t3.pos.z * t3.rhw * c3;
					p.z = w * z;

					drawPoint(p, normal, { 0, 0 }, {  r,  g,  b });
				}
			}
		}
	}
}

//实验新算法
//void Device::drawTriangle_2(const Vertex& v1, const Vertex& v2, const Vertex& v3)
//{
//	Vector c1, c2, c3;
//	Vector n1, n2, n3;
//
//	transform->apply(c1, v1.pos);
//	transform->apply(c2, v2.pos);
//	transform->apply(c3, v3.pos);
//
//	//检查是否越界
//	if (transform->checkCvv(c1)) return;
//	if (transform->checkCvv(c2)) return;
//	if (transform->checkCvv(c3)) return;
//
//	transform->applyMV(n1, v1.normal);
//	transform->applyMV(n2, v2.normal);
//	transform->applyMV(n3, v3.normal);
//
//	Vector p1, p2, p3, min, max;
//
//	transform->homogenize(p1, c1);
//	transform->homogenize(p2, c2);
//	transform->homogenize(p3, c3);
//
//	if (cullmode == CULL_MODE_BACK) {
//		Vector s1, s2, pn;
//		VectorSub(s1, p2, p1);
//		VectorSub(s2, p3, p2);
//		VectorCrossProduct(pn, s2, s1);
//		VectorNormalize(pn);
//		if (transform->checkBackCulling(pn)) return;
//	}
//
//	int s = GetRealState(state);
//
//	if (!(s&STATE_DRAW_LINE)) {
//		min = p1; max = p1;
//
//		if (p2.x < min.x) min.x = p2.x;
//		if (p3.x < min.x) min.x = p3.x;
//		if (p2.y < min.y) min.y = p2.y;
//		if (p3.y < min.y) min.y = p3.y;
//
//		if (p2.x > max.x) max.x = p2.x;
//		if (p3.x > max.x) max.x = p3.x;
//		if (p2.y > max.y) max.y = p2.y;
//		if (p3.y > max.y) max.y = p3.y;
//
//		Vertex t1 = v1, t2 = v2, t3 = v3;
//		t1.pos = p1; t2.pos = p2; t3.pos = p3;
//		t1.normal = n1; t2.normal = n2; t3.normal = n3;
//		t1.pos.w = c1.w; t2.pos.w = c2.w; t3.pos.w = c3.w;
//
//		VertexRhwInit(t1);
//		VertexRhwInit(t2);
//		VertexRhwInit(t3);
//
//		for (int j = min.y; j < max.y; j++) {
//			for (int i = min.x; i < max.x; i++) {
//				Vector p = { i,j,0.f,1.f };
//				float c1=0.25, c2=0.25, c3;
//				if (getTriangleInterp_2(p1, p2, p3, p)) {
//					c3 = 1.f - c1 - c2;
//
//					// Rectification for Perspective
//					float w = 1 / (c1 * t1.rhw + c2 * t2.rhw + c3 * t3.rhw);
//
//					float r = t1.color.r * c1 + t2.color.r * c2 + t3.color.r * c3;
//					float g = t1.color.g * c1 + t2.color.g * c2 + t3.color.g * c3;
//					float b = t1.color.b * c1 + t2.color.b * c2 + t3.color.b * c3;
//
//					Vector normal = { 0.f, 0.f, 0.f };
//
//					float z = t1.pos.z * t1.rhw * c1 + t2.pos.z * t2.rhw * c2 + t3.pos.z * t3.rhw * c3;
//					p.z = w * z;
//
//					drawPoint(p, normal, { w * r, w * g, w * b });
//				}
//			}
//		}
//	}
//	else
//	{
//		drawLine(p1, p2);
//		drawLine(p1, p3);
//		drawLine(p2, p3);
//	}
//}

void Device::drawLine(const Vertex& v1, const Vertex& v2) {
	Vector c1, c2;
	Vector n1, n2;

	transform->apply(c1, v1.pos);
	transform->apply(c2, v2.pos);

	//检查是否越界
	if (transform->checkCvv(c1)) return;
	if (transform->checkCvv(c2)) return;

	transform->applyMV(n1, v1.normal);
	transform->applyMV(n2, v2.normal);

	Vector p1, p2, min, max;

	transform->homogenize(p1, c1);
	transform->homogenize(p2, c2);

	int s = GetRealState(state);

	drawLine(p1, p2);
}