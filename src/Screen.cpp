#include "Screen.h"
#include <windows.h>
#include <tchar.h>

int ScreenKeys[512]; // ��¼����״̬
int ScreenUpKeys[512]; // ��¼�����Ƿ񱻷ſ�
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
	//������
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)WndProc, 0, 0, 0, NULL, NULL, NULL, NULL, _T("SCREEN") };

	//// -height ��ʾ top-down
	//BITMAPINFO�ṹ������Windows�豸�޹�λͼ��DIB���Ķ�������ɫ��Ϣ
	BITMAPINFO bi = { {sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB, width * height * 4, 0, 0, 0, 0} };
	
	//rect���������洢һ�����ο�����Ͻ����ꡢ��Ⱥ͸߶ȡ�
	RECT rect = { 0, 0, width, height };

	this->close();

	//���嵱����򿪻��ػ�ĳ�����ڸ���Ĵ���ʱ����䴰�ڿͻ�������ɫ����ʽ
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	
	//��ʶע������DLL��Ӧ�ó���ʵ��
	wc.hInstance = GetModuleHandle(NULL);

	//����������Ĵ�������ʾ��ָ��, IDC_ARROW ��׼�ļ�ͷ
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	//ע�ᴰ�����
	if (!RegisterClass(&wc)) return -1;

	//��������
	wndHandle = CreateWindow(_T("SCREEN"), title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (wndHandle == NULL) return -2;
    
	//��ô��ڵ�ͼ���豸������
	HDC hDC = GetDC(wndHandle);
	//����һ����ָ���豸���ݵ��ڴ��豸�����Ļ���
	wndDc = CreateCompatibleDC(hDC);
	
	ReleaseDC(wndHandle, hDC);
	//Ӧ�ó������ֱ��д��ġ����豸�޹ص�λͼ��DIB���� 
	wndHb = CreateDIBSection(wndDc, &bi, DIB_RGB_COLORS, &wndFramebuffer, 0, 0);
	if (wndHb == NULL) return -3;

	//��һ������(λͼ�����ʡ���ˢ��)ѡ��ָ�����豸�������µĶ������ͬһ���͵��϶���
	wndOb = (HBITMAP)SelectObject(wndDc, wndHb);
	this->width = width;
	this->height = height;

	//�ú�����������ͻ����εĴ�С��������Ҫ�Ĵ��ھ��εĴ�С��
	AdjustWindowRect(&rect, GetWindowLong(wndHandle, GWL_STYLE), 0);
	int wx = rect.right - rect.left;
	int wy = rect.bottom - rect.top;
	//GetSystemMetrics���Ի����Ļ�ֱ��ʣ���͸�
	int sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	int sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	
	//�ı䴰��λ�ú�״̬
	SetWindowPos(wndHandle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	//����������ָ�����ڵ��߳����õ�ǰ̨�����Ҽ���ô��ڡ�
	//��������ת��ô��ڣ���Ϊ�û��ĸ��ֿ��ӵļǺš�ϵͳ������ǰ̨���ڵ��̷߳����Ȩ���Ը��������̡߳�
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