// DXApp.h

#pragma once

#include <d3d11.h>
#include "Shared.h"

struct d3d11_ctx_s {
	HWND						mWnd;
	ID3D11Device *				mDevice;
	ID3D11DeviceContext *		mDeviceContext;
};

#define DX_LEFT_BUTTON	1

#define	DX_DOWN			1
#define DX_UP			2

#define DX_FPS			60

typedef void(*reshape_func_t)(int width, int height);
typedef void(*mouse_func_t)(int button, int state, int x, int y);
typedef void(*motion_func_t)(int x, int y);
typedef void(*keyboard_func_t)(int key);
typedef void(*idle_func_t)(int delta_ms);
typedef void(*void_func_t)();

bool	DX_CreateWindow(int width, int height, const wchar_t *title, d3d11_ctx_s **ctx);
void	DX_ReshapeFunc(reshape_func_t func);
void	DX_MouseFunc(mouse_func_t func);
void	DX_MotionFunc(motion_func_t func);
void	DX_KeyboardFunc(keyboard_func_t func);
void	DX_KeyboardUpFunc(keyboard_func_t func);
void	DX_DisplayFunc(void_func_t func);
void	DX_IdleFunc(idle_func_t func);
void	DX_CloseFunc(void_func_t func);
void	DX_MainLoop();

// utils
void	DX_GetClientCursorPos(POINT &pt);
void	DX_MoveCursorToClientCenter();

bool	DX_CreateSwapChain(DXGI_SWAP_CHAIN_DESC *scd, IDXGISwapChain **ppSwapChain);
bool	DX_CreateRenderTargetView(IDXGISwapChain *pSwapChain, ID3D11RenderTargetView **ppRenderTargetView);
bool	DX_CreateTexture2D(ID3D11Device *dev, int width, int height, DXGI_FORMAT fmt, UINT bind_flags, const void *data, ID3D11Texture2D **ppTexture2D);
bool	DX_CreateBuffer(ID3D11Device *dev, size_t size, UINT bind_flags, const void *data, ID3D11Buffer **ppBuffer);

bool	DX_Failed(HRESULT hr, const char_t *file, int line);
#define DX_FAILED(hr)	DX_Failed(hr, __SRC_FILE__, __LINE__)
#define DX_SAFE_RELEASE(obj)	do { if (obj) { obj->Release();  obj = nullptr; } } while (0)

#ifdef _DEBUG

# define DX_SET_DEBUG_NAME(obj, name)	\
do {	\
	const GUID WKPDID_D3DDebugObjectName_ = { 0x429b8c22, 0x9188, 0x4b0c, 0x87, 0x42, 0xac, 0xb0, 0xbf, 0x85, 0xc2, 0x00 };	\
	obj->SetPrivateData(WKPDID_D3DDebugObjectName_, (UINT)strlen(name), name);	\
} while (0)

#else

# define DX_SET_DEBUG_NAME(obj, name)	(void)0

#endif // !_DEBUG
