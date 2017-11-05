// Main.cpp

#include "Precompiled.h"
#include <conio.h>

/*
================================================================================
Callback
================================================================================
*/

static d3d11_ctx_s *	gD3D11Ctx;
static input_s			gInputs[256];
static int				gInputsCount;
static Demo				gDemo;

static void OnReshape(int width, int height) {
	gDemo.Resize(width, height);
}

static void OnMouse(int button, int state, int x, int y) {
	if (gInputsCount < 256) {
		if (button == DX_LEFT_BUTTON) {
			input_s * input = gInputs + gInputsCount;
			input->mType = state == DX_DOWN ? IN_LBUTTON_DOWN : IN_LBUTTON_UP;
			input->mParam1 = x;
			input->mParam2 = y;
			gInputsCount++;
		}
	}
}

static void OnMotion(int x, int y) {
	if (gInputsCount < 256) {
		input_s * input = gInputs + gInputsCount;
		input->mType = IN_MOUSE_MOVE;
		input->mParam1 = x;
		input->mParam2 = y;
		gInputsCount++;
	}
}

static void OnKeyboard(int key) {
	if (gInputsCount < 256) {
		input_s * input = gInputs + gInputsCount;
		input->mType = IN_KEY_DOWN;
		input->mParam1 = key;
		gInputsCount++;
	}
}

static void OnKeyboardUp(int key) {
	if (gInputsCount < 256) {
		input_s * input = gInputs + gInputsCount;
		input->mType = IN_KEY_UP;
		input->mParam1 = key;
		gInputsCount++;
	}
}

static void OnIdle(int delta_ms) {
	gDemo.Frame(gInputs, gInputsCount, delta_ms * 0.001f);
	gInputsCount = 0;
}

static void OnClose() {
	gDemo.Shutdown();
}

int main(int argc, char **argv) {
#ifdef _DEBUG // detect memory leak
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Sys_InitTimer();

	printf("F2: toggle draw terrain\n");
	printf("F3: toggle draw wireframe mode\n");
	printf("F4: toggle draw skydome\n");
	printf("Page Up: increase move speed (max: %d)\n", (int)MAX_MOVE_SPEED);
	printf("Page Down: decrease move speed (min: %d)\n", (int)MIN_MOVE_SPEED);
	printf("Arrow Up: increase pixel error (max: %d)\n", (int)MAX_PIXEL_ERROR);
	printf("Arrow Down: decrease pixel error (min: %d)\n\n", (int)MIN_PIXEL_ERROR);
	printf("Please select a test case (1: Grand Canyon (4K * 2k), 2: Puget Sound (4k * 4k): ");

	int user_input = 1;
	scanf_s("%d", &user_input);
	if (user_input < 1) {
		user_input = 1;
	}

	if (user_input > 2) {
		user_input = 2;
	}

	int test_case = user_input - 1; // convert to array index

	// extract res path
	char_t res_dir[MAX_PATH_];

#ifdef _UNICODE
	char_t exe_filename[MAX_PATH_];
	Str_FrAnsi(argv[0], exe_filename, MAX_PATH_);
	Str_ExtractExeDir(exe_filename, res_dir, MAX_PATH_);
#else
	Str_ExtractExeDir(argv[0], res_dir, MAX_PATH_);
#endif
	Str_ExtractDirSelf(res_dir);
	Str_ExtractDirSelf(res_dir);
	Str_ExtractDirSelf(res_dir);
	Str_ExtractDirSelf(res_dir);
	strcat_s_(res_dir, TXT_("\\res"));

	if (!DX_CreateWindow(VIEWPORT_CX, VIEWPORT_CY, L"GeoMipMapping", &gD3D11Ctx)) {
		return 1;
	}

	printf("loading data ...\n");

	if (!gDemo.Init(gD3D11Ctx, res_dir, test_case)) {
		gDemo.Shutdown();

		gD3D11Ctx->mDeviceContext->Release();
		gD3D11Ctx->mDevice->Release();

		_getch();	// pause. display error message
		return 1;
	}

	printf("done.\n");

	DX_ReshapeFunc(OnReshape);
	DX_MouseFunc(OnMouse);
	DX_MotionFunc(OnMotion);
	DX_KeyboardFunc(OnKeyboard);
	DX_KeyboardUpFunc(OnKeyboardUp);
	DX_IdleFunc(OnIdle);
	DX_CloseFunc(OnClose);

	DX_MainLoop();

	return 0;
}
