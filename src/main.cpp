#include <Windows.h>

#include <time.h>

#include "Light.h"
#include "Screen.h"
#include "Device.h"
#include "Transform.h"
#include "Config.h"
#include "Vertex.h"
#include<tchar.h>
#include<stdio.h>

//设置快捷键
#define VK_Z 0x5A
#define VK_X 0x58
#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C

//设置窗口大小
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

//初始化窗口
Screen* screen = NULL;
//初始化渲染流水线
Device* device = NULL;
//初始化变换
Transform* transform = NULL;


int *textures[3] = { 0,0,0 };

//使用控制台进行调试输出
void InitConsoleWindow()
{
	int nCrt;
	FILE* fp;
	if (AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
	}
}

void DrawLine()
{
	Vector v1 = { 0.f, 0.f, 0.f, 1.f };
	Vector v2 = { 799.f, 599.f, 0.f, 1.f };
	device->drawLine(v1, v2);
	Vector v3 = { 0.f,599.f,0.f,1.f };
	Vector v4 = { 799.f,0.f,0.f,1.f };
	device->drawLine(v3, v4);
}

void DrawLine(const Vertex& v1, const Vertex& v2) {
	Vertex t1 = v1, t2 = v2;
	device->drawLine(t1, t2);
}

void DrawPlane(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4)
{
	// 必须重新制定一下纹理，不然就乱掉了
	Vertex t1 = v1, t2 = v2, t3 = v3, t4 = v4;
	t1.tex.u = 0.f; t2.tex.u = 1.f; t3.tex.u = 1.f; t4.tex.u = 0.f;
	t1.tex.v = 0.f; t2.tex.v = 0.f; t3.tex.v = 1.f; t4.tex.v = 1.f;

	device->drawTriangle(t1, t2, t3);
	device->drawTriangle(t3, t4, t1);
}

int* CreateTexture()
{
	int *tex = (int*)malloc(100 * 100 * sizeof(int));

	int color[10][10];

	int c = 0x11eeee;
	for (int i = 0; i < 10; i++) {
		int c1 = c;
		for (int j = 0; j < 10; j++) {
			color[i][j] = c1;
			c1 = c1 == 0xffffff ? 0x11eeee : 0xffffff;
		}
		c = c == 0xffffff ? 0x11eeee : 0xffffff;
	}

	// 100 x 100
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			tex[i * 100 + j] = color[i / 10][j / 10];
		}
	}

	return tex;
}

void DrawBox(float theta)
{
	Matrix mm;
	MatrixSetRotate(mm, 0.f, 1.0f, 0.f, theta);

	Matrix tm;
	MatrixSetTranslate(tm, 3.0f, 3.f, 3.f);

	Matrix m;
	MatrixMul(m, mm, tm);

	transform->setWorld(m);
	transform->update();


	Vertex vs[8] = {
	{{-1.f,  1.f, -1.f, 1.f}, {1.0f, 0.0f, 0.0f}, {0.f, 0.f}, {-1.f,  1.f, -1.f, 0.f},  1.f },
	{{-1.f,  1.f,  1.f, 1.f}, {0.0f, 1.0f, 0.0f}, {0.f, 1.f}, {-1.f,  1.f,  1.f, 0.f}, 1.f },
	{{ 1.f,  1.f,  1.f, 1.f}, {0.0f, 0.0f, 1.0f}, {1.f, 1.f}, { 1.f,  1.f,  1.f, 0.f}, 1.f },
	{{ 1.f,  1.f, -1.f, 1.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}, { 1.f,  1.f, -1.f, 0.f}, 1.f },

	{{-1.f, -1.f, -1.f, 1.f}, {0.0f, 0.0f, 1.0f}, {1.f, 0.f}, {-1.f, -1.f, -1.f, 0.f}, 1.f },
	{{-1.f, -1.f,  1.f, 1.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}, {-1.f, -1.f,  1.f, 0.f}, 1.f },
	{{ 1.f, -1.f,  1.f, 1.f}, {1.0f, 0.0f, 0.0f}, {1.f, 1.f}, { 1.f, -1.f,  1.f, 0.f}, 1.f },
	{{ 1.f, -1.f, -1.f, 1.f}, {0.0f, 1.0f, 0.0f}, {1.f, 1.f}, { 1.f, -1.f, -1.f, 0.f}, 1.f },
	};

	DrawPlane(vs[0], vs[1], vs[2], vs[3]);
	DrawPlane(vs[7], vs[4], vs[0], vs[3]);
	DrawPlane(vs[2], vs[6], vs[7], vs[3]);
	DrawPlane(vs[5], vs[1], vs[0], vs[4]);
	DrawPlane(vs[2], vs[1], vs[5], vs[6]);
	DrawPlane(vs[6], vs[5], vs[4], vs[7]);
}

void DrawCoordinateSystem(float theta)
{
	Matrix mm;
	MatrixSetRotate(mm, 0.f, 1.0f, 0.f, theta);

	Matrix tm;
	MatrixSetTranslate(tm, 0.f, 0.f, 0.f);

	Matrix m;
	MatrixMul(m, mm, tm);

	transform->setWorld(m);
	transform->update();

	Vertex vs[4] = {
	{{0.f,  0.f, 0.f, 1.f}, {1.0f, 0.0f, 0.0f}, {0.f, 0.f}, {-1.f,  1.f, -1.f, 0.f},  1.f },
	{{1.f,  0.f,  0.f, 1.f}, {0.0f, 1.0f, 0.0f}, {0.f, 1.f}, {-1.f,  1.f,  1.f, 0.f}, 1.f },
	{{ 0.f,  1.f,  0.f, 1.f}, {0.0f, 0.0f, 1.0f}, {1.f, 1.f}, { 1.f,  1.f,  1.f, 0.f}, 1.f },
	{{ 0.f,  0.f, 1.f, 1.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}, { 1.f,  1.f, -1.f, 0.f}, 1.f },
	};

	device->drawLine(vs[0], vs[1]);
	device->drawLine(vs[0], vs[2]);
	device->drawLine(vs[0], vs[3]);
}


void DrawBox_2(float theta)
{
	Matrix mm;
	MatrixSetRotate(mm, 0.f, 1.0f, 0.f, theta);

	Matrix tm;
	MatrixSetTranslate(tm, 0.0f, 0.f, 0.f);

	Matrix m;
	MatrixMul(m, mm, tm);

	transform->setWorld(m);
	transform->update();


	Vertex vs[8] = {
	{{-1.f,  1.f, -1.f, 1.f}, {1.0f, 0.0f, 0.0f}, {0.f, 0.f}, {-1.f,  1.f, -1.f, 0.f},  1.f },
	{{-1.f,  1.f,  1.f, 1.f}, {0.0f, 1.0f, 0.0f}, {0.f, 1.f}, {-1.f,  1.f,  1.f, 0.f}, 1.f },
	{{ 1.f,  1.f,  1.f, 1.f}, {0.0f, 0.0f, 1.0f}, {1.f, 1.f}, { 1.f,  1.f,  1.f, 0.f}, 1.f },
	{{ 1.f,  1.f, -1.f, 1.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}, { 1.f,  1.f, -1.f, 0.f}, 1.f },

	{{-1.f, -1.f, -1.f, 1.f}, {0.0f, 0.0f, 1.0f}, {1.f, 0.f}, {-1.f, -1.f, -1.f, 0.f}, 1.f },
	{{-1.f, -1.f,  1.f, 1.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}, {-1.f, -1.f,  1.f, 0.f}, 1.f },
	{{ 1.f, -1.f,  1.f, 1.f}, {1.0f, 0.0f, 0.0f}, {1.f, 1.f}, { 1.f, -1.f,  1.f, 0.f}, 1.f },
	{{ 1.f, -1.f, -1.f, 1.f}, {0.0f, 1.0f, 0.0f}, {1.f, 1.f}, { 1.f, -1.f, -1.f, 0.f}, 1.f },
	};

	DrawPlane(vs[0], vs[1], vs[2], vs[3]);
	//DrawPlane(vs[3], vs[2], vs[1], vs[0]);
	DrawPlane(vs[7], vs[4], vs[0], vs[3]);
	DrawPlane(vs[2], vs[6], vs[7], vs[3]);
	DrawPlane(vs[5], vs[1], vs[0], vs[4]);
	DrawPlane(vs[2], vs[1], vs[5], vs[6]);
	DrawPlane(vs[6], vs[5], vs[4], vs[7]);
}

void DrawBox_3(float x_theta, float y_theta, float z_theta)
{
	Matrix mm_x;
	Matrix mm_y;
	Matrix mm_z;
	MatrixSetRotate_X(mm_x, x_theta);
	MatrixSetRotate_Y(mm_y, y_theta);
	MatrixSetRotate_Z(mm_z, z_theta);
	
	Matrix mm;

	MatrixMul(mm, mm_x, mm_y);
	MatrixMul(mm, mm, mm_z);

	Matrix tm;
	MatrixSetTranslate(tm, 0.0f, 0.f, 0.f);

	Matrix m;
	MatrixMul(m, mm, tm);

	transform->setWorld(m);
	transform->update();


	Vertex vs[8] = {
	{{-1.f,  1.f, -1.f, 1.f}, {1.0f, 0.0f, 0.0f}, {0.f, 0.f}, {-1.f,  1.f, -1.f, 0.f},  1.f },
	{{-1.f,  1.f,  1.f, 1.f}, {0.0f, 1.0f, 0.0f}, {0.f, 1.f}, {-1.f,  1.f,  1.f, 0.f}, 1.f },
	{{ 1.f,  1.f,  1.f, 1.f}, {0.0f, 0.0f, 1.0f}, {1.f, 1.f}, { 1.f,  1.f,  1.f, 0.f}, 1.f },
	{{ 1.f,  1.f, -1.f, 1.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}, { 1.f,  1.f, -1.f, 0.f}, 1.f },

	{{-1.f, -1.f, -1.f, 1.f}, {0.0f, 0.0f, 1.0f}, {1.f, 0.f}, {-1.f, -1.f, -1.f, 0.f}, 1.f },
	{{-1.f, -1.f,  1.f, 1.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}, {-1.f, -1.f,  1.f, 0.f}, 1.f },
	{{ 1.f, -1.f,  1.f, 1.f}, {1.0f, 0.0f, 0.0f}, {1.f, 1.f}, { 1.f, -1.f,  1.f, 0.f}, 1.f },
	{{ 1.f, -1.f, -1.f, 1.f}, {0.0f, 1.0f, 0.0f}, {1.f, 1.f}, { 1.f, -1.f, -1.f, 0.f}, 1.f },
	};

	DrawPlane(vs[0], vs[1], vs[2], vs[3]);
	//DrawPlane(vs[3], vs[2], vs[1], vs[0]);
	DrawPlane(vs[7], vs[4], vs[0], vs[3]);
	DrawPlane(vs[2], vs[6], vs[7], vs[3]);
	DrawPlane(vs[5], vs[1], vs[0], vs[4]);
	DrawPlane(vs[2], vs[1], vs[5], vs[6]);
	DrawPlane(vs[6], vs[5], vs[4], vs[7]);
}

//画平面
void DrawPlane(float theta)
{
	Matrix m;
	MatrixSetRotate(m, 1.f, 0.0f, 0.f, theta);
	transform->setWorld(m);
	transform->update();

	Vertex vs[4] = {
		{{-1.f, 0.f, 0.f, 1.f}, {1.f, 0.f, 0.f}, {0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, 1.f },
		{{-1.f, 0.f, 10.f, 1.f}, {0.f, 0.5f, 0.f}, {0.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, 1.f },
		{{ 1.f, 0.f, 10.f, 1.f}, {0.f, 0.5f, 0.f}, {1.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, 1.f },
		{{ 1.f, 0.f, 0.f, 1.f}, {1.f, 0.f, 0.f}, {0.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, 1.f },
	};

	DrawPlane(vs[0], vs[1], vs[2], vs[3]);
}

struct Face {
	int i1, i2, i3;
};

char *ReadFile(const char* file)
{
	FILE *pFile = fopen(file, "rb");
	if (!pFile) {
		return NULL;
	}

	char *pBuf;
	//int fseek(FILE *stream, long int offset, int whence) 设置流 stream 的文件位置为给定的偏移 offset，
	//参数 offset 意味着从给定的 whence 位置查找的字节数。
	fseek(pFile, 0, SEEK_END);
	
	//long int ftell(FILE *stream) 返回给定流 stream 的当前文件位置。
	int len = ftell(pFile);
	pBuf = new char[len + 1];
	
	//void rewind(FILE *stream) 设置文件位置为给定流 stream 的文件的开头。
	rewind(pFile);

	//size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) 从给定流 stream 读取数据到 ptr 所指向的数组中。
	fread(pBuf, 1, len, pFile);
	pBuf[len] = '\0';
	fclose(pFile);
	return pBuf;
}

//读取网格
int LoadMesh(const char *file, Vertex*& pVertexs, int& vsize, Face*& pFaces, int& fsize)
{
	char* pFile;
	pFile = ReadFile(file);
	if (!pFile) {
		return 0;
	}

	char* pSrc;
	pSrc = pFile;
}

void SetCamera(float x, float y, float z)
{
	Vector eye = { x, y, z, 1.f }, at = { 0.f, 0.f, 0.f, 1.f }, up = { 0.f, 1.f, 0.f, 1.f };
	Matrix m;
	MatrixSetLookAt(m, eye, at, up);
	transform->setView(m);
	transform->update();
}

void TransformLight(Light& light, float theta)
{
	Matrix m;
	MatrixSetRotate(m, 0.f, 1.0f, 0.f, theta);
	transform->setWorld(m);
	transform->update();

	transform->applyMV(light.direction, { -0.3f, 1.f, -0.3f, 0.f });
	VectorNormalize(light.direction);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	InitConsoleWindow();
	srand((unsigned)time(NULL));

	screen = new Screen();
	int ret = screen->init(WINDOW_WIDTH, WINDOW_HEIGHT, _T("SoftRendering"));
	if (ret < 0) {
		printf("screen init failed(%d)!\n", ret);
		exit(ret);
	}

	transform = new Transform();
	transform->init(WINDOW_WIDTH, WINDOW_HEIGHT);

	textures[0] = CreateTexture();

	Light light = { {-1.f, 1.f, -1.f, 0.f}, {0.5f, 0.5f, 0.5f} };

	uint32* wfb = (uint32*)(screen->getFrameBuffer());
	device = new Device();
	device->init(WINDOW_WIDTH, WINDOW_HEIGHT, wfb, transform,textures,&light);
	device->setState(0);

	//int vsize, fsize;
	//Vertex* pVertexs;
	//Face* pFaces;
	//LoadMesh("models/cow.obj", pVertexs, vsize, pFaces, fsize);

	float theta = 0.f;
	float dist = 5.f;
	
	//direct 旋转
	float X_theta = 0.f;
	float Y_theta = 0.f;
	float Z_theta = 0.f;

	//add
	float w = 0.0f;
	float h = 0.0f;

	float light_theta = 1.f;
	while (!screen->isExit()) {
		device->clear();
		screen->dispatch();
		//SetCamera(3.f, 3.f, dist);
		SetCamera(w, h, dist);

		light_theta += 0.05f;
		TransformLight(light, light_theta);

		if (screen->isKeyPressed(VK_UP))
			dist -= 0.05f;
		if (screen->isKeyPressed(VK_DOWN))
			dist += 0.05f;

		if (screen->isKeyPressed(VK_LEFT))
			theta += 0.01f;
		if (screen->isKeyPressed(VK_RIGHT))
			theta -= 0.01f;

		//add by zhang 上下平移
		if (screen->isKeyPressed(VK_W))
			h += 0.1f;
		if (screen->isKeyPressed(VK_S))
			h -= 0.1f;
		if (screen->isKeyPressed(VK_A))
			w += 0.1f;
		if (screen->isKeyPressed(VK_D))
			w -= 0.1f;
		
		//绕坐标轴旋转
		if (screen->isKeyPressed(VK_I))
			X_theta += 0.01f;
		if (screen->isKeyPressed(VK_K))
			X_theta -= 0.01f;
		if (screen->isKeyPressed(VK_J))
			Y_theta += 0.01f;
		if (screen->isKeyPressed(VK_L))
			Y_theta -= 0.01f;

		//改变状态
		if (screen->getKeyUpEvent(VK_SPACE))
			device->autoChangeState();

		//透视
		if (screen->getKeyUpEvent(VK_Z)) // K key
			device->autoChangeCullMode();

		//开关灯
		if (screen->getKeyUpEvent(VK_X)) // K key
			device->autoChangeLightMode();

		DrawBox_3(X_theta, Y_theta, Z_theta);
		//DrawLine();
		//DrawCoordinateSystem(theta);
		//DrawPlane(theta);

		screen->dispatch();
		screen->update();
		Sleep(1);
	}
	device->close();
	screen->close();

	for (int i = 0; i < 3; i++) {
		if (textures[i]) {
			delete textures[i];
		}
	}

	delete transform;
	delete device;
	delete screen;

	return 0;
}