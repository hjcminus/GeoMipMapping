// Terrain.cpp

#include "Precompiled.h"

/*
================================================================================
Terrain
================================================================================
*/
Terrain::Terrain() {
	mCenter[0] = mCenter[1] = mCenter[2] = 0.0f;
}

Terrain::~Terrain() {
	// do nothing
}

bool Terrain::Init(const char_t *res_dir, int test_case, float max_pixel_error) {
	char_t full_filename[MAX_PATH_];
	sprintf_s_(full_filename, TXT_("%s\\terrain\\%s"), res_dir, TEST_DATA[test_case].mHeightMap);

	image_s heights = { 0 };

	if (ReadPNG(full_filename, &heights)) {
		if (CheckTerrainSize(heights.mCX, heights.mCY)) {
			height_field_s hf;
			hf.mHeights = (float*)malloc(sizeof(float) * heights.mCX * heights.mCY);
			hf.mSizeX = heights.mCX;
			hf.mSizeY = heights.mCY;

			if (heights.mFormat == IMG_FORMAT_2BYTE_GRAY) {
				for (int y = 0; y < heights.mCY; ++y) {
					unsigned short * src_row = (unsigned short*)heights.mData + y * heights.mCX;
					float * dst_row = hf.mHeights + (heights.mCY - y - 1) * heights.mCX; // png scanlines are ordered from top to bottom, revert y
					for (int x = 0; x < heights.mCX; ++x) {
						float f = (float)src_row[x] * TEST_DATA[test_case].mElevationScale;
						dst_row[x] = f;
					}
				}
			}
			else if (heights.mFormat == IMG_FORMAT_1BYTE_RGB) {
				for (int y = 0; y < heights.mCY; ++y) {
					unsigned char * src_row = (unsigned char*)heights.mData + y * heights.mCX * 3;
					float * dst_row = hf.mHeights + (heights.mCY - y - 1) * heights.mCX; // png scanlines are ordered from top to bottom, revert y
					for (int x = 0; x < heights.mCX; ++x) {
						unsigned char * src_pixel = src_row + x * 3;
						float f = (float)src_pixel[0] * TEST_DATA[test_case].mElevationScale;
						dst_row[x] = f;
					}
				}
			}
			else {
				printf("unsupported terrain height format\n");
				free(hf.mHeights);
				FreeImg(&heights);
				return false;
			}

			mGeoMipmapping.Build(hf, FOVY, VIEWPORT_CX, VIEWPORT_CY, max_pixel_error);

			mCenter[0] = (float)(heights.mCX / 2);
			mCenter[1] = (float)(heights.mCY / 2);
			mCenter[2] = hf.mHeights[(heights.mCY / 2) * heights.mCX + heights.mCX / 2];

			free(hf.mHeights);
			FreeImg(&heights);

			return true;
		}
		else {
			printf("wrong terrain size\n");
			FreeImg(&heights);
			return false;
		}
	}
	else {
		return false;
	}
}

void Terrain::Shutdown() {
	mGeoMipmapping.Clear();
}

void Terrain::GetCenter(vec3_t center) const {
	Vec3Copy(mCenter, center);
}

void Terrain::Reshape(int view_width, int view_height) {
	mGeoMipmapping.Reshape(view_width, view_height);
}

void Terrain::UpdateMaxPixelError(float max_pixel_error) {
	mGeoMipmapping.UpdateMaxPixelError(max_pixel_error);
}

void Terrain::Update(const camera_s &cam) {
	mGeoMipmapping.Update(cam);
}

const mesh_s & Terrain::GetRenderMesh() const {
	return mGeoMipmapping.GetRenderMesh();
}
