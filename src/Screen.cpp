#include "Screen.h"
#include <windows.h>
#include <tchar.h>

int ScreenKeys[512]; // 记录键盘状态
int ScreenUpKeys[512]; // 记录键盘是否被放开
int Exit = 0;

static LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:Exit = 1; break;
	case WM_KEYDOWN:ScreenKeys[wParam & 511] = 1; break;
	case WM_KEYUP:ScreenKeys[wParam & 511] = 0; ScreenUpKeys[wParam & 511] = 1; break;
	default:return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

int Screen::init(int width, int height, LPCTSTR title)
{
	//窗口类
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)WndProc, 0, 0, 0, NULL, NULL, NULL, NULL, _T("SCREEN") };

	//// -height 表示 top-down
	//BITMAPINFO结构定义了Windows设备无关位图（DIB）的度量和颜色信息
	BITMAPINFO bi = { {sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB, width * height * 4, 0, 0, 0, 0} };
	
	//rect对象用来存储一个矩形框的左上角坐标、宽度和高度。
	RECT rect = { 0, 0, width, height };

	this->close();

	//定义当程序打开或重画某个属于该类的窗口时，填充窗口客户区的颜色和样式
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	
	//标识注册该类的DLL或应用程序实例
	wc.hInstance = GetModuleHandle(NULL);

	//属于鼠标该类的窗口里显示的指针, IDC_ARROW 标准的箭头
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	//注册窗口类别
	if (!RegisterClass(&wc)) return -1;

	//创建窗口
	wndHandle = CreateWindow(_T("SCREEN"), title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (wndHandle == NULL) return -2;
    
	//获得窗口的图像设备表述表
	HDC hDC = GetDC(wndHandle);
	//创建一个与指定设备兼容的内存设备上下文环境
	wndDc = CreateCompatibleDC(hDC);
	
	ReleaseDC(wndHandle, hDC);
	//应用程序可以直接写入的、与设备无关的位图（DIB）。 
	wndHb = CreateDIBSection(wndDc, &bi, DIB_RGB_COLORS, &wndFramebuffer, 0, 0);
	if (wndHb == NULL) return -3;

	//把一个对象(位图、画笔、画刷等)选入指定的设备描述表。新的对象代替同一类型的老对象。
	wndOb = (HBITMAP)SelectObject(wndDc, wndHb);
	this->width = width;
	this->height = height;

	//该函数依据所需客户矩形的大小，计算需要的窗口矩形的大小。
	AdjustWindowRect(&rect, GetWindowLong(wndHandle, GWL_STYLE), 0);
	int wx = rect.right - rect.left;
	int wy = rect.bottom - rect.top;
	//GetSystemMetrics可以获得屏幕分辨率，宽和高
	int sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	int sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	
	//改变窗口位置和状态
	SetWindowPos(wndHandle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	//函数将创建指定窗口的线程设置到前台，并且激活该窗口。
	//键盘输入转向该窗口，并为用户改各种可视的记号。系统给创建前台窗口的线程分配的权限稍高于其他线程。
	SetForegroundWindow(wndHandle);

	ShowWindow(wndHandle, SW_NORMAL);
	dispatch();

	memset(wndFramebuffer, 0, width * height * 4);
	memset(ScreenKeys, 0, sizeof(int) * 512);
	return 0;
}

void Screen::update()
{
	HDC hDC = GetDC(wndHandle);
	BitBlt(hDC, 0, 0, width, height, wndDc, 0, 0, SRCCOPY);
	ReleaseDC(wndHandle, hDC);

	dispatch();
}

void Screen::dispatch()
{
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void Screen::close()
{
	if (wndDc) {
		if (wndOb) {
			SelectObject(wndDc, wndOb);
			wndOb = NULL;
		}
		DeleteDC(wndDc);
		wndDc = NULL;
	}

	if (wndHb) {
		DeleteObject(wndHb);
		wndHb = NULL;
	}

	if (wndHandle) {
		CloseWindow(wndHandle);
		wndHandle = NULL;
	}
}

int Screen::isKeyPressed(int key)
{
	return ScreenKeys[key & 511];
}

int Screen::getKeyUpEvent(int key)
{
	int r = ScreenUpKeys[key];
	ScreenUpKeys[key] = 0;
	return r;
}

int Screen::isExit()
{
	return Exit;
}

LPVOID Screen::getFrameBuffer()
{
	return wndFramebuffer;
}