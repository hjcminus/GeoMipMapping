// Terrain.h

#pragma once

/*
================================================================================
Terrain
================================================================================
*/
class Terrain {
public:
	Terrain();
	~Terrain();

	bool						Init(const char_t *res_dir, int test_case, float max_pixel_error);
	void						Shutdown();

	void						GetCenter(vec3_t center) const;

	void						Reshape(int view_width, int view_height);
	void						UpdateMaxPixelError(float max_pixel_error);

	// update terrain geometry each frame
	void						Update(const camera_s &cam);
	const mesh_s &				GetRenderMesh() const;

private:

	vec3_t						mCenter;
	GeoMipMapping				mGeoMipmapping;
};
