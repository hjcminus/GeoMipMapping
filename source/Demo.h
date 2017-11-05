// Demo.h

#pragma once

/*
================================================================================
Demo
================================================================================
*/
class Demo {
public:

	Demo();
	~Demo();

	bool						Init(d3d11_ctx_s *ctx, const char_t *res_dir, int test_case);
	void						Shutdown();

	void						Resize(int width, int height);
	void						Frame(const input_s *inputs, int count, float delta_seconds);

private:

	struct key_state_s {
		bool					mDown;
		int						mRepeat;
	};

	// control variables
	unsigned					mDrawFlags;
	float						mMoveSpeed;
	float						mMaxPixelError;
	bool						mShowCursor;

	// input states
	bool						mLButtonDown;
	int							mPreviousMouseX;
	int							mPreviousMouseY;
	key_state_s					mKeyStates[256];

	int							mViewCX;
	int							mViewCY;
	bool						mSkipMouseMoveOnce;

	// sub objects
	Viewer						mViewer;
	Terrain						mTerrain;
	Renderer					mRenderer;

	void						ProcessInput(const input_s *inputs, int count, float delta_seconds);
};
