// Demo.cpp

#include "Precompiled.h"

/*
================================================================================
Demo
================================================================================
*/
Demo::Demo():
	mDrawFlags(DRAW_SKYDOME | DRAW_CLOUD | DRAW_TERRAIN_SOLID),
	mMoveSpeed(10.0f),
	mMaxPixelError(DEFAILT_PIXEL_ERROR),
	mShowCursor(true),
	mLButtonDown(false),
	mPreviousMouseX(0),
	mPreviousMouseY(0),
	mViewCX(1),
	mViewCY(1),
	mSkipMouseMoveOnce(false)
{
	memset(mKeyStates, 0, sizeof(mKeyStates));
}

Demo::~Demo() {
	// do nothing
}

bool Demo::Init(d3d11_ctx_s *ctx, const char_t *res_dir, int test_case) {
	if (!mRenderer.Init(ctx, res_dir, test_case)) {
		return false;
	}

	if (!mTerrain.Init(res_dir, test_case, mMaxPixelError)) {
		return false;
	}

	// set viewer to terrain center
	vec3_t init_view_pos;
	mTerrain.GetCenter(init_view_pos);
	init_view_pos[2] += 4.0f;
	mViewer.InitPos(init_view_pos);

	return true;
}

void Demo::Shutdown() {
	mTerrain.Shutdown();
	mRenderer.Shutdown();
}

void Demo::Resize(int width, int height) {
	mViewCX = max(width, 1);
	mViewCY = max(height, 1);
	mRenderer.Reshape(width, height);
	mTerrain.Reshape(width, height);
}

void Demo::Frame(const input_s *inputs, int count, float delta_seconds) {
	ProcessInput(inputs, count, delta_seconds);

	// update terrain
	const camera_s & cam = mViewer.GetCamera();

	double t1 = Sys_GetRelativeTime();
	mTerrain.Update(cam);
	double t2 = Sys_GetRelativeTime();

	demo_info_s info;
	info.mUpdateTimeElapsed = t2 - t1;
	info.mMaxPixelError = mMaxPixelError;

	// draw
	mRenderer.UpdateRenderTerrain(mTerrain.GetRenderMesh());
	mRenderer.Draw(delta_seconds, mDrawFlags, cam, info);
}

void Demo::ProcessInput(const input_s *inputs, int count, float delta_seconds) {
	Viewer::movement_delta_s movement_delta = { 0.0f };

	for (int i = 0; i < count; ++i) {
		const input_s * input = inputs + i;

		if (input->mType == IN_LBUTTON_DOWN) {
			mLButtonDown = true;

			mPreviousMouseX = input->mParam1;
			mPreviousMouseY = input->mParam2;
		}
		else if (input->mType == IN_LBUTTON_UP) {
			mLButtonDown = false;

			mPreviousMouseX = input->mParam1;
			mPreviousMouseY = input->mParam2;
		}
		else if (input->mType == IN_MOUSE_MOVE) {
			int mouse_delta_x = input->mParam1 - mPreviousMouseX;
			int mouse_delta_y = input->mParam2 - mPreviousMouseY;

			mPreviousMouseX = input->mParam1;
			mPreviousMouseY = input->mParam2;

			if (mShowCursor) {
				if (mLButtonDown) {
					movement_delta.mDeltaYaw += -mouse_delta_x * MOUSE_SENSITIVITY;
					movement_delta.mDeltaPitch += -mouse_delta_y * MOUSE_SENSITIVITY;
				}
			}

		}
		else if (input->mType == IN_KEY_DOWN) {
			int key = input->mParam1;

			mKeyStates[key].mDown = true;

			// update display flags (first key down)
			if (mKeyStates[VK_F2].mDown && mKeyStates[VK_F2].mRepeat == 0) {
				ToggleFlags(mDrawFlags, DRAW_TERRAIN_SOLID);
			}
			else if (mKeyStates[VK_F3].mDown && mKeyStates[VK_F3].mRepeat == 0) {
				ToggleFlags(mDrawFlags, DRAW_TERRAIN_WIREFRAME);
			}
			else if (mKeyStates[VK_F4].mDown && mKeyStates[VK_F4].mRepeat == 0) {
				ToggleFlags(mDrawFlags, DRAW_SKYDOME);
			}
			else if (mKeyStates[VK_F5].mDown && mKeyStates[VK_F5].mRepeat == 0) {
				ToggleFlags(mDrawFlags, DRAW_CLOUD);
			}
			else if (mKeyStates[VK_SPACE].mDown && mKeyStates[VK_SPACE].mRepeat == 0) {
				mShowCursor = !mShowCursor;
				ShowCursor(mShowCursor);

				if (!mShowCursor) {
					mSkipMouseMoveOnce = true;
				}
			}
			else if (mKeyStates[VK_UP].mDown && mKeyStates[VK_UP].mRepeat == 0) {
				mMaxPixelError += 1.0f;
				if (mMaxPixelError > MAX_PIXEL_ERROR) {
					mMaxPixelError = MAX_PIXEL_ERROR;
				}
				mTerrain.UpdateMaxPixelError(mMaxPixelError);
			}
			else if (mKeyStates[VK_DOWN].mDown && mKeyStates[VK_DOWN].mRepeat == 0) {
				mMaxPixelError -= 1.0f;
				if (mMaxPixelError < MIN_PIXEL_ERROR) {
					mMaxPixelError = MIN_PIXEL_ERROR;
				}
				mTerrain.UpdateMaxPixelError(mMaxPixelError);
			}

			mKeyStates[key].mRepeat++;
		}
		else if (input->mType == IN_KEY_UP) {
			int key = input->mParam1;

			mKeyStates[key].mDown = false;
			mKeyStates[key].mRepeat = 0;
		}
	}

	// update move speed
	if (mKeyStates[VK_PRIOR].mDown) {
		mMoveSpeed += 1.0f;
		if (mMoveSpeed > MAX_MOVE_SPEED) {
			mMoveSpeed = MAX_MOVE_SPEED;
		}
	}
	else if (mKeyStates[VK_NEXT].mDown) {
		mMoveSpeed -= 1.0f;
		if (mMoveSpeed < MIN_MOVE_SPEED) {
			mMoveSpeed = MIN_MOVE_SPEED;
		}
	}

	if (!mShowCursor) {
		if (!mSkipMouseMoveOnce) {
			POINT pt;
			DX_GetClientCursorPos(pt);

			int mouse_delta_x = pt.x - mViewCX / 2;
			int mouse_delta_y = pt.y - mViewCY / 2;

			movement_delta.mDeltaYaw = -mouse_delta_x * MOUSE_SENSITIVITY;
			movement_delta.mDeltaPitch = -mouse_delta_y * MOUSE_SENSITIVITY;
		}
		else {
			mSkipMouseMoveOnce = false;
		}

		DX_MoveCursorToClientCenter();
	}

	// update movement
	if (mKeyStates['W'].mDown) {
		movement_delta.mMoveForward = 1.0f;
	}

	if (mKeyStates['S'].mDown) {
		movement_delta.mMoveForward = -1.0f;
	}

	if (mKeyStates['A'].mDown) {
		movement_delta.mMoveRight = -1.0f;
	}

	if (mKeyStates['D'].mDown) {
		movement_delta.mMoveRight = 1.0f;
	}

	if (mKeyStates['Q'].mDown) {
		movement_delta.mMoveUp = 1.0f;
	}

	if (mKeyStates['Z'].mDown) {
		movement_delta.mMoveUp = -1.0f;
	}

	movement_delta.mMoveSpeed = mMoveSpeed;
	movement_delta.mDeltaSeconds = delta_seconds;

	mViewer.Update(movement_delta);
}
