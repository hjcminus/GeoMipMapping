// GeoMipMapping.h

#pragma once

/*
================================================================================
GeoMipMapping
================================================================================
*/
struct height_field_s {
	float *						mHeights;
	int							mSizeX;
	int							mSizeY;
};

class GeoMipMapping {
public:
	GeoMipMapping();
	~GeoMipMapping();

	void						Build(const height_field_s &hf, float fovy, int view_width, int view_height, float max_pixel_error);
	void						Clear();

	void						Reshape(int view_width, int view_height);
	void						UpdateMaxPixelError(float max_pixel_error);

	void						Update(const camera_s &cam);
	const mesh_s &				GetRenderMesh() const;

private:

	struct block_s {
		bool					mVisible;
		int						mXIndex;		// block's x index
		int						mYIndex;		// block's y index
		vec3_t					mCenter;		// bounding box center
		int						mDisplayLevel;	// level 0 is finest
		float *					mErrorList;		// error in object space
		float *					mDSquareList;	// Dn
	};

	float						mFovy;
	float						mMaxPixelError;

	int							mLevels;		// level count
	float *						mHeights;
	int							mHeightsSizeX;
	int							mHeightsSizeY;
	vertex_pos_s *				mVertices;
	int							mVerticesCount;
	unsigned int *				mIndices;
	int							mIndicesCount;
	mesh_s						mRenderMesh;
	float *						mErrorBuffer;
	float *						mDSquareBuffer;
	block_s *					mBlocks;
	int							mBlocksXCount;
	int							mBlocksYCount;
	bool						mBufferFull;
	int							mTriangleCount;

	int							mViewWidth;
	int							mViewHeight;
	frustum_plane_s				mFrustumPlane;

	void						CalcBlockErrorList(block_s *block);
	void						BuildBlockDList();
	void						UpdateBlock(block_s *block, const camera_s &cam);
	void						SetupRenderMeshByBlock(block_s *block);

	void						SetupBottomEdgeNormally(block_s *block, unsigned int global_index_offset);
	void						SetupRightEdgeNormally(block_s *block, unsigned int global_index_offset);
	void						SetupTopEdgeNormally(block_s *block, unsigned int global_index_offset);
	void						SetupLeftEdgeNormally(block_s *block, unsigned int global_index_offset);

	void						SetupBottomEdgeFixGap(block_s *block, block_s *bottom_neighbor, unsigned int global_index_offset);
	void						SetupRightEdgeFixGap(block_s *block, block_s *right_neighbor, unsigned int global_index_offset);
	void						SetupTopEdgeFixGap(block_s *block, block_s *top_neighbor, unsigned int global_index_offset);
	void						SetupLeftEdgeFixGap(block_s *block, block_s *left_neighbor, unsigned int global_index_offset);
};

bool	CheckTerrainSize(int x_size, int y_size);
