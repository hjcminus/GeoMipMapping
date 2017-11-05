// DXApp.cpp

#include "DXApp.h"
#include <mmsyscom.h>

#pragma comment(lib, "winmm.lib")

/*
================================================================================
function pointers
================================================================================
*/

static reshape_func_t		gReshapeFunc;
static mouse_func_t			gMouseFunc;
static motion_func_t		gMotionFunc;
static keyboard_func_t		gKeyboardFunc;
static keyboard_func_t		gKeyboardUpFunc;
static void_func_t			gDisplayFunc;
static idle_func_t			gIdleFunc;
static void_func_t			gCloseFunc;
static d3d11_ctx_s			gCtx;
static RECT					gClientRect;

static void OnLButtonDown(int x, int y) {
	if (gMouseFunc) {
		gMouseFunc(DX_LEFT_BUTTON, DX_DOWN, x, y);
	}
}

static void OnLButtonUp(int x, int y) {
	if (gMouseFunc) {
		gMouseFunc(DX_LEFT_BUTTON, DX_UP, x, y);
	}
}

static void OnMouseMove(int x, int y) {
	if (gMotionFunc) {
		gMotionFunc(x, y);
	}
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;

	switch (message) {
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		OnLButtonDown((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		OnLButtonUp((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		OnMouseMove((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (gKeyboardFunc) {
			gKeyboardFunc((int)wParam);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (gKeyboardUpFunc) {
			gKeyboardUpFunc((int)wParam);
		}
		break;
	case WM_SIZE:
		if (gReshapeFunc) {
			gReshapeFunc((int)LOWORD(lParam), (int)HIWORD(lParam));
		}
		break;
	case WM_MOVE:
		::GetClientRect(hWnd, &gClientRect);
		break;
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);

		if (gDisplayFunc) {
			gDisplayFunc();
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool DX_CreateWindow(int width, int height, const wchar_t *title, d3d11_ctx_s **ctx) {
	HINSTANCE hInst = GetModuleHandle(NULL);

	// register demo class
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	// set up RegisterClass struct
	wcex.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(LONG_PTR);
	wcex.hInstance = hInst;
	//wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_APP));
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"DX";
	wcex.hIconSm = NULL; // no small icon for now

	// register the window class
	RegisterClassEx(&wcex);

	DWORD window_style = WS_OVERLAPPEDWINDOW;
	DWORD ex_style = WS_EX_CLIENTEDGE;

	RECT wndRect;
	wndRect.left = 0;
	wndRect.top = 0;
	wndRect.right = width;
	wndRect.bottom = height;

	AdjustWindowRectEx(&wndRect, window_style, FALSE, ex_style);

	int window_cx = wndRect.right - wndRect.left;
	int window_cy = wndRect.bottom - wndRect.top;

	int screen_cx = GetSystemMetrics(SM_CXSCREEN);
	int screen_cy = GetSystemMetrics(SM_CYSCREEN);

	int pos_x = (screen_cx - window_cx) >> 1;
	int pos_y = (screen_cy - window_cy) >> 1;

	gCtx.mWnd = CreateWindowEx(ex_style, L"DX", title, window_style, pos_x, pos_y, window_cx, window_cy, nullptr, nullptr, hInst, nullptr);

	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL supported_feature_level;
	UINT create_flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#ifdef _DEBUG
	create_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	if (DX_FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_flags,
		feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION,
		&gCtx.mDevice, &supported_feature_level, &gCtx.mDeviceContext))) {
		return false;
	}

	*ctx = &gCtx;

	return true;
}

void DX_ReshapeFunc(reshape_func_t func) {
	gReshapeFunc = func;
}

void DX_MouseFunc(mouse_func_t func) {
	gMouseFunc = func;
}

void DX_MotionFunc(motion_func_t func) {
	gMotionFunc = func;
}

void DX_KeyboardFunc(keyboard_func_t func) {
	gKeyboardFunc = func;
}

void DX_KeyboardUpFunc(keyboard_func_t func) {
	gKeyboardUpFunc = func;
}

void DX_DisplayFunc(void_func_t func) {
	gDisplayFunc = func;
}

void DX_IdleFunc(idle_func_t func) {
	gIdleFunc = func;
}

void DX_CloseFunc(void_func_t func) {
	gCloseFunc = func;
}

// utils
void DX_GetClientCursorPos(POINT &pt) {
	::GetCursorPos(&pt);
	::ScreenToClient(gCtx.mWnd, &pt);
}

void DX_MoveCursorToClientCenter() {
	POINT pt;
	pt.x = (gClientRect.left + gClientRect.right) / 2;
	pt.y = (gClientRect.top + gClientRect.bottom) / 2;
	::ClientToScreen(gCtx.mWnd, &pt);
	::SetCursorPos(pt.x, pt.y);
}

void DX_MainLoop() {
	ShowWindow(gCtx.mWnd, SW_SHOW);
	UpdateWindow(gCtx.mWnd);

	::GetClientRect(gCtx.mWnd, &gClientRect);

	timeBeginPeriod(1);
	DWORD previous = timeGetTime();
	DWORD frame_ms = 1000 / DX_FPS;

	while (true) {
		MSG msg;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (WM_QUIT == msg.message) {
				goto QUIT;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DWORD current = timeGetTime();
		DWORD delta = current - previous;

		if (delta >= frame_ms) {
			if (gIdleFunc) {
				gIdleFunc(delta);
			}
			previous = current;
		}
	}

QUIT:

	timeEndPeriod(1);

	if (gCloseFunc) {
		gCloseFunc();
	}

	gCtx.mDeviceContext->Release();
	gCtx.mDevice->Release();
}

bool DX_CreateSwapChain(DXGI_SWAP_CHAIN_DESC *scd, IDXGISwapChain **ppSwapChain) {
	// create the swap chain
	IDXGIDevice *	dxgiDevice = nullptr;
	IDXGIAdapter *	dxgiAdapter = nullptr;
	IDXGIFactory *	dxgiFactory = nullptr;

	bool ok = false;

	do {

		if (DX_FAILED(gCtx.mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice))) {
			break;
		}
		DX_SET_DEBUG_NAME(dxgiDevice, "[CreateSwapChain]dxgiDevice");

		if (DX_FAILED(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter))) {
			break;
		}
		DX_SET_DEBUG_NAME(dxgiAdapter, "[CreateSwapChain]dxgiAdapter");

		if (DX_FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory))) {
			break;
		}
		DX_SET_DEBUG_NAME(dxgiFactory, "[CreateSwapChain]dxgiFactory");

		if (DX_FAILED(dxgiFactory->CreateSwapChain(gCtx.mDevice, scd, ppSwapChain))) {
			break;
		}

		ok = true;

	} while (0);

	if (dxgiFactory) {
		dxgiFactory->Release();
	}

	if (dxgiAdapter) {
		dxgiAdapter->Release();
	}

	if (dxgiDevice) {
		dxgiDevice->Release();
	}

	return ok;
}

bool DX_CreateRenderTargetView(IDXGISwapChain *pSwapChain, ID3D11RenderTargetView **ppRenderTargetView) {
	// create the render target view
	ID3D11Texture2D * back_buffer = nullptr;

	bool ok = false;

	do {

		if (DX_FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer)))) {
			break;
		}

		if (DX_FAILED(gCtx.mDevice->CreateRenderTargetView(back_buffer, NULL, ppRenderTargetView))) {
			break;
		}

		ok = true;

	} while (0);

	if (back_buffer) {
		back_buffer->Release();
	}

	return ok;
}

bool DX_CreateTexture2D(ID3D11Device *dev, int width, int height, DXGI_FORMAT fmt, UINT bind_flags, const void *data, ID3D11Texture2D **ppTexture2D) {
	D3D11_TEXTURE2D_DESC td;
	memset(&td, 0, sizeof(td));

	td.Width = width;
	td.Height = height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = fmt;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = bind_flags;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	if (data) {
		D3D11_SUBRESOURCE_DATA sd;
		memset(&sd, 0, sizeof(sd));

		sd.pSysMem = data;
		sd.SysMemPitch = width * 4;
		sd.SysMemSlicePitch = width * height * 4;

		if (DX_FAILED(dev->CreateTexture2D(&td, &sd, ppTexture2D))) {
			return false;
		}
	}
	else {
		if (DX_FAILED(dev->CreateTexture2D(&td, nullptr, ppTexture2D))) {
			return false;
		}
	}

	return true;
}

bool DX_CreateBuffer(ID3D11Device *dev, size_t size, UINT bind_flags, const void *data, ID3D11Buffer **ppBuffer) {
	D3D11_BUFFER_DESC bd;

	bd.ByteWidth = (UINT)size;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = bind_flags;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA rd;

	rd.pSysMem = data;
	rd.SysMemPitch = 0;
	rd.SysMemSlicePitch = 0;

	if (data) {
		D3D11_SUBRESOURCE_DATA sd;
		memset(&sd, 0, sizeof(sd));

		sd.pSysMem = data;
		sd.SysMemPitch = 0;
		sd.SysMemSlicePitch = 0;

		if (DX_FAILED(dev->CreateBuffer(&bd, &sd, ppBuffer))) {
			return false;
		}
	}
	else {
		if (DX_FAILED(dev->CreateBuffer(&bd, nullptr, ppBuffer))) {
			return false;
		}
	}

	return true;
}

bool DX_Failed(HRESULT hr, const char_t *file, int line) {
	switch (hr) {
	case D3D11_ERROR_FILE_NOT_FOUND:
		Sys_Error(file, line, TXT_("D3D11_ERROR_FILE_NOT_FOUND\n"));
		return true;
	case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
		Sys_Error(file, line, TXT_("D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS\n"));
		return true;
	case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
		Sys_Error(file, line, TXT_("D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS\n"));
		return true;
	case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
		Sys_Error(file, line, TXT_("D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD\n"));
		return true;
	case DXGI_ERROR_INVALID_CALL: // old: D3DERR_INVALIDCAL
		Sys_Error(file, line, TXT_("DXGI_ERROR_INVALID_CALL\n"));
		return true;
	case DXGI_ERROR_WAS_STILL_DRAWING: // old: D3DERR_WASSTILLDRAWING
		Sys_Error(file, line, TXT_("DXGI_ERROR_WAS_STILL_DRAWING\n"));
		return true;
	case E_FAIL:
		Sys_Error(file, line, TXT_("E_FAIL\n"));
		return true;
	case E_INVALIDARG:
		Sys_Error(file, line, TXT_("E_INVALIDARG\n"));
		return true;
	case E_OUTOFMEMORY:
		Sys_Error(file, line, TXT_("E_OUTOFMEMORY\n"));
		return true;
	case E_NOTIMPL:
		Sys_Error(file, line, TXT_("E_NOTIMPL\n"));
		return true;
	case S_FALSE:
		Sys_Error(file, line, TXT_("S_FALSE\n"));
		return true;
	case S_OK:
		return false;
	}

	if (FAILED(hr)) {
		Sys_Error(file, line, TXT_("D3D Error: UNKNOWN\n"));
		return true;
	}

	return true;
}
