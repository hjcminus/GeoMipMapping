// Viewer.h

#pragma once

/*
================================================================================
Viewer
================================================================================
*/
class Viewer {
public:

	struct movement_delta_s {
		float					mDeltaYaw;
		float					mDeltaPitch;
		float					mMoveForward;
		float					mMoveRight;
		float					mMoveUp;
		float					mMoveSpeed;
		float					mDeltaSeconds;
	};

	Viewer();
	~Viewer();

	void						InitPos(const vec3_t pos);
	void						Update(const movement_delta_s &delta);
	const camera_s &			GetCamera() const;

private:

	camera_s					mCamera;

	float						mYaw;
	float						mPitch;
	vec3_t						mCameraForward;
	vec3_t						mCameraRight;

	void						UpdateCameraOrientation();
};

