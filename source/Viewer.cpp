// Viewer.cpp

#include "Precompiled.h"

/*
================================================================================
Viewer
================================================================================
*/
#define START_FORWARD	XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
#define START_RIGHT		XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)
#define START_UP		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)

Viewer::Viewer():
	mYaw(0.0f),
	mPitch(0.0f)
{
	Vec3Set(mCamera.mPos, 0.0f, 0.0f, 0.0f);
	mCamera.mFovy = FOVY;
	mCamera.mZNear = NEAR_CLIP_PLANE;
	mCamera.mZFar = FAR_CLIP_PLANE;
	UpdateCameraOrientation();
}

Viewer::~Viewer() {
	// do nothing
}

void Viewer::InitPos(const vec3_t pos) {
	Vec3Copy(pos, mCamera.mPos);
	UpdateCameraOrientation();
}

void Viewer::Update(const movement_delta_s &delta) {
	mYaw += delta.mDeltaYaw;
	mPitch += delta.mDeltaPitch;
	UpdateCameraOrientation();

	vec3_t delta_forward, delta_right, delta_up;
	Vec3Mul(mCameraForward, delta.mMoveForward * delta.mMoveSpeed * delta.mDeltaSeconds, delta_forward);
	Vec3Mul(mCameraRight, delta.mMoveRight * delta.mMoveSpeed * delta.mDeltaSeconds, delta_right);

	static const vec3_t UP = { 0.0f, 0.0f, 1.0f }; // Z direction
	Vec3Mul(UP, delta.mMoveUp * delta.mMoveSpeed * delta.mDeltaSeconds, delta_up);

	vec3_t total_delta;
	Vec3Add(delta_forward, delta_right, total_delta);
	Vec3Add(total_delta, delta_up, total_delta);

	Vec3Add(mCamera.mPos, total_delta, mCamera.mPos);
	Vec3Add(mCamera.mPos, mCameraForward, mCamera.mTarget);
}

void Viewer::UpdateCameraOrientation() {
	// clamp rotation
	mYaw = ClampYaw(mYaw);
	mPitch = ClampPitch(mPitch);

	// update rotation
	XMMATRIX yaw_matrix = XMMatrixRotationZ(Deg2Rad(mYaw));

	XMVECTOR camera_forward = XMVector4Transform(START_FORWARD, yaw_matrix);
	XMVECTOR camera_right = XMVector4Transform(START_RIGHT, yaw_matrix);

	XMMATRIX pitch_matrix = XMMatrixRotationAxis(camera_right, Deg2Rad(mPitch));

	camera_forward = XMVector4Transform(camera_forward, pitch_matrix);
	XMVECTOR camera_up = XMVector4Transform(START_UP, pitch_matrix);

	camera_forward = XMVector4Normalize(camera_forward);
	camera_right = XMVector4Normalize(camera_right);
	camera_up = XMVector4Normalize(camera_up);

	XMVECTORToVec3(camera_forward, mCameraForward);
	XMVECTORToVec3(camera_right, mCameraRight);
	XMVECTORToVec3(camera_up, mCamera.mUp);
	
	Vec3Add(mCamera.mPos, mCameraForward, mCamera.mTarget);
}

const camera_s & Viewer::GetCamera() const {
	return mCamera;
}
