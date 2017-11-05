// GeoMipMapping.cpp

#include "Precompiled.h"

/*
================================================================================
GeoMipMapping
================================================================================
*/
#define	BLOCK_SIZE	16	// tune this: 16, 32
static const float BLOCK_RADIUS = sqrtf(BLOCK_SIZE * BLOCK_SIZE) * 0.75f; // * 0.75f scale up a bit

bool CheckTerrainSize(int x_size, int y_size) {
	if ((x_size - 1) < BLOCK_SIZE || (y_size - 1) < BLOCK_SIZE) {
		return false;
	}

	if (x_size > MAX_TERRAIN_SIZE || y_size > MAX_TERRAIN_SIZE) {
		return false;
	}

	if (((x_size - 1) % BLOCK_SIZE) != 0 || ((y_size - 1) % BLOCK_SIZE) != 0) { // must divide exactly by BLOCK_SIZE
		return false;
	}
	
	return true;
}

GeoMipMapping::GeoMipMapping() :
	mFovy(FOVY),
	mMaxPixelError(DEFAILT_PIXEL_ERROR),
	mLevels(0),
	mHeights(nullptr),
	mHeightsSizeX(1),
	mHeightsSizeY(1),
	mVertices(nullptr),
	mVerticesCount(0),
	mIndices(nullptr),
	mIndicesCount(0),
	mErrorBuffer(nullptr),
	mDSquareBuffer(nullptr),
	mBlocks(nullptr),
	mBlocksXCount(0),
	mBlocksYCount(0),
	mBufferFull(false),
	mViewWidth(1),
	mViewHeight(1),
	mTriangleCount(0)
{
	mRenderMesh.mVertices = nullptr;
	mRenderMesh.mVerticesCount = 0;
	mRenderMesh.mIndices = nullptr;
	mRenderMesh.mIndicesCount = 0;
}

GeoMipMapping::~GeoMipMapping() {
	// do nothing
}

void GeoMipMapping::Build(const height_field_s &hf, float fovy, int view_width, int view_height, float max_pixel_error) {
	mFovy = fovy;
	mViewWidth = view_width;
	mViewHeight = view_height;
	mMaxPixelError = max_pixel_error;

	mHeights = (float*)malloc(sizeof(float) * hf.mSizeX * hf.mSizeY);
	memcpy(mHeights, hf.mHeights, sizeof(float) * hf.mSizeX * hf.mSizeY);
	mHeightsSizeX = hf.mSizeX;
	mHeightsSizeY = hf.mSizeY;

	mVertices = (vertex_pos_s*)malloc(sizeof(vertex_pos_s) * MAX_DRAW_VERT_COUNT);
	mIndices = (unsigned int*)malloc(sizeof(unsigned int) * MAX_DRAW_IDX_COUNT);
	mVerticesCount = 0;
	mIndicesCount = 0;

	mLevels = 1;
	int block_size = BLOCK_SIZE;
	while (block_size >= 2) {
		block_size >>= 1;
		mLevels++;
	}

	mBlocksXCount = (mHeightsSizeX - 1) / BLOCK_SIZE;
	mBlocksYCount = (mHeightsSizeY - 1) / BLOCK_SIZE;
	int block_count = mBlocksXCount * mBlocksYCount;

	mBlocks = (block_s*)malloc(sizeof(block_s) * block_count);
	mDSquareBuffer = (float*)malloc(sizeof(float) * block_count * (mLevels - 1));
	mErrorBuffer = (float*)malloc(sizeof(float) * block_count * (mLevels - 1));

	for (int block_y = 0; block_y < mBlocksYCount; ++block_y) {
		for (int block_x = 0; block_x < mBlocksXCount; ++block_x) {
			block_s * block = mBlocks + block_y * mBlocksXCount + block_x;

			int center_x = block_x * BLOCK_SIZE + BLOCK_SIZE / 2;
			int center_y = block_y * BLOCK_SIZE + BLOCK_SIZE / 2;

			// find min z, max z in current block
			float min_z =  99999999.0f;
			float max_z = -99999999.0f;
			for (int y = block_y * BLOCK_SIZE; y <= (block_y + 1) * BLOCK_SIZE; ++y) {
				for (int x = block_x * BLOCK_SIZE; x <= (block_x + 1) * BLOCK_SIZE; ++x) {
					float z = mHeights[y * mHeightsSizeX + x];

					if (z > max_z) {
						max_z = z;
					}

					if (z < min_z) {
						min_z = z;
					}
				}
			}

			float center_z = (min_z + max_z) * 0.5f; // block bouding box center

			block->mXIndex = block_x;
			block->mYIndex = block_y;
			Vec3Set(block->mCenter, (float)center_x, (float)center_y, center_z);
			block->mDisplayLevel = 0;
			block->mDSquareList = mDSquareBuffer + (block_y * mBlocksXCount + block_x) * (mLevels - 1);
			block->mErrorList = mErrorBuffer + (block_y * mBlocksXCount + block_x) * (mLevels - 1);

			CalcBlockErrorList(block);
		}
	}

	BuildBlockDList();
}

void GeoMipMapping::Clear() {
	SAFE_FREE(mDSquareBuffer);
	SAFE_FREE(mErrorBuffer);
	SAFE_FREE(mBlocks);
	SAFE_FREE(mIndices);
	SAFE_FREE(mVertices);
	SAFE_FREE(mHeights);
}

void GeoMipMapping::Reshape(int view_width, int view_height) {
	mViewWidth = view_width;
	mViewHeight = view_height;
	BuildBlockDList();
}

void GeoMipMapping::UpdateMaxPixelError(float max_pixel_error) {
	mMaxPixelError = max_pixel_error;
	BuildBlockDList();
}

void GeoMipMapping::Update(const camera_s &cam) {
	mFrustumPlane.Setup(mViewWidth, mViewHeight, cam);

	mTriangleCount = 0;
	// setup display level
	for (int i = 0; i < mBlocksXCount * mBlocksYCount; ++i) {
		block_s * block = mBlocks + i;

		UpdateBlock(mBlocks + i, cam);

		if (mFrustumPlane.CullHorizontalCircle(block->mCenter, BLOCK_RADIUS)) {
			block->mVisible = false;
		}
		else {
			block->mVisible = true;

			int display_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;
			mTriangleCount += display_quad_dim * display_quad_dim * 2;
		}
	}

	mVerticesCount = 0;
	mIndicesCount = 0;

	// setup draw vertices and indices
	mBufferFull = false;
	for (int i = 0; i < mBlocksXCount * mBlocksYCount; ++i) {
		block_s * block = mBlocks + i;

		if (mBufferFull) { // run out of vertex or index buffer
			break;
		}

		if (block->mVisible) {
			SetupRenderMeshByBlock(block);
		}		
	}

	mRenderMesh.mVertices = mVertices;
	mRenderMesh.mVerticesCount = mVerticesCount;
	mRenderMesh.mIndices = mIndices;
	mRenderMesh.mIndicesCount = mIndicesCount;
	mRenderMesh.mTriangleCount = mTriangleCount;
}

const mesh_s & GeoMipMapping::GetRenderMesh() const {
	return mRenderMesh;
}

/*

'.' are eliminated vertices
'+' are keep vertices

+ -- . -- + -- . -- +    row 4
| \       | \       |
.    .    .    .    .    row 3
|     \   |     \   |
+ -- . -- + -- . -- +    row 2
| \       | \       |
.    .    .    .    .    row 1
|     \   |     \   | 
+ -- . -- + -- . -- +    row 0

*/

// make sure higher level has larger error
#define MIN_LEVEL_ERROR_TIMES	1.5f

void GeoMipMapping::CalcBlockErrorList(block_s *block) {
	int vert_start_x = block->mXIndex * BLOCK_SIZE;
	int vert_start_y = block->mYIndex * BLOCK_SIZE;

	float prior_level_max_error = 0.0f;
	for (int i = 1; i < mLevels; ++i) {
		float current_level_max_error = 0.0f;

		int step = 1 << i;
		int half_step = step / 2;

		// even rows
		for (int y = vert_start_y; y <= vert_start_y + BLOCK_SIZE; y += step) {

			for (int x = vert_start_x + half_step; x <= vert_start_x + BLOCK_SIZE - half_step; x += step) {

				float height = mHeights[y * mHeightsSizeX + x];
				float height_lf = mHeights[y * mHeightsSizeX + x - half_step];
				float height_rt = mHeights[y * mHeightsSizeX + x + half_step];
				float simplified_height = (height_lf + height_rt) * 0.5f;
				float error = fabsf(height - simplified_height);

				if (error > current_level_max_error) {
					current_level_max_error = error;
				}
			}
		}

		// odd rows
		for (int y = vert_start_y + half_step; y <= vert_start_y + BLOCK_SIZE - half_step; y += step) {

			for (int x = vert_start_x; x <= vert_start_x + BLOCK_SIZE; x += step) {

				float height = mHeights[y * mHeightsSizeX + x];
				float height_tp = mHeights[(y + half_step) * mHeightsSizeX + x];
				float height_bt = mHeights[(y - half_step) * mHeightsSizeX + x];
				float simplified_height = (height_tp + height_bt) * 0.5f;
				float error = fabsf(height - simplified_height);

				if (error > current_level_max_error) {
					current_level_max_error = error;
				}
			}
		}
		
		// center
		for (int y = vert_start_y + half_step; y <= vert_start_y + BLOCK_SIZE - half_step; y += step) {

			for (int x = vert_start_x + half_step; x <= vert_start_x + BLOCK_SIZE - half_step; x += step) {

				float height = mHeights[y * mHeightsSizeX + x];
				float height_tp_lf = mHeights[(y + half_step) * mHeightsSizeX + x - half_step];
				float height_bt_rt = mHeights[(y - half_step) * mHeightsSizeX + x + half_step];
				float simplified_height = (height_tp_lf + height_bt_rt) * 0.5f;
				float error = fabsf(height - simplified_height);

				if (error > current_level_max_error) {
					current_level_max_error = error;
				}
			}

		}

		if (current_level_max_error < prior_level_max_error * MIN_LEVEL_ERROR_TIMES) {
			current_level_max_error = prior_level_max_error * MIN_LEVEL_ERROR_TIMES;
		}

		block->mErrorList[i - 1] = current_level_max_error;
		prior_level_max_error = current_level_max_error;
	}
}

void GeoMipMapping::BuildBlockDList() {
	float t = NEAR_CLIP_PLANE * tanf(Deg2Rad(mFovy * 0.5f));
	float A = NEAR_CLIP_PLANE / t;
	float T = 2.0f * mMaxPixelError / mViewHeight;
	float C = A / T;

	for (int i = 0; i < mBlocksXCount * mBlocksYCount; ++i) {
		block_s * block = mBlocks + i;

		for (int j = 1; j < mLevels; ++j) {
			float err = block->mErrorList[j - 1];
			float d = (err * err) * (C * C); // squared
			block->mDSquareList[j - 1] = d;
		}
	}

#ifdef _DEBUG

	/* // dump error and d list
	FILE * f = nullptr;
	fopen_s(&f, "errorlist.txt", "w");
	for (int i = 0; i < mBlocksXCount * mBlocksYCount; ++i) {
		block_s * block = mBlocks + i;

		fprintf_s(f, "%03d-%03d ", block->mXIndex, block->mYIndex);

		for (int j = 1; j < mLevels; ++j) {
			float err = block->mErrorList[j - 1];
			float d = block->mDSquareList[j - 1];

			fprintf_s(f, "(%12.4f,%12.4f) ", err, d);
		}

		fprintf_s(f, "\n");
	}
	fclose(f);
	//*/

#endif
}

void GeoMipMapping::UpdateBlock(block_s *block, const camera_s &cam) {
	vec3_t horizontal_delta_to_view;

	horizontal_delta_to_view[0] = cam.mPos[0] - block->mCenter[0];
	horizontal_delta_to_view[1] = cam.mPos[1] - block->mCenter[1];
	horizontal_delta_to_view[2] = 0.0f; // omit vertical difference

	float d_square = Vec3SquareLength(horizontal_delta_to_view);

	block->mDisplayLevel = 0;
	for (int i = 1; i < mLevels; ++i) {
		if (d_square > block->mDSquareList[i - 1]) {
			block->mDisplayLevel = i;
		}
		else {
			break;
		}
	}
}

void GeoMipMapping::SetupRenderMeshByBlock(block_s *block) {
	// check free buffer capacity

	int display_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;
	int need_vertices = Square(display_quad_dim + 1);
	if ((need_vertices + mVerticesCount) > MAX_DRAW_VERT_COUNT) {
		mBufferFull = true;
		return;
	}

	int need_max_indices = (display_quad_dim * (display_quad_dim + 1) * 2) + display_quad_dim; /* plus restart index */

	need_max_indices += (display_quad_dim / 2) * 4; // possible edge restart index

	if ((need_max_indices + mIndicesCount) > MAX_DRAW_IDX_COUNT) {
		mBufferFull = true;
		return;
	}

	int vert_start_x = block->mXIndex * BLOCK_SIZE;
	int vert_start_y = block->mYIndex * BLOCK_SIZE;
	int vert_step = 1 << block->mDisplayLevel;

	unsigned global_index_offset = (unsigned)mVerticesCount;

	// setup vertices
	for (int y = vert_start_y; y <= vert_start_y + BLOCK_SIZE; y += vert_step) {
		for (int x = vert_start_x; x <= vert_start_x + BLOCK_SIZE; x += vert_step) {
			Vec3Set(mVertices[mVerticesCount++].mPosition, (float)x, (float)y, mHeights[y * mHeightsSizeX + x]);
		}
	}

	// setup indices
	if (block->mDisplayLevel == mLevels - 1) {
		mIndices[mIndicesCount++] = RESTART_INDEX;

		mIndices[mIndicesCount++] = 2 + global_index_offset;
		mIndices[mIndicesCount++] = 0 + global_index_offset;
		mIndices[mIndicesCount++] = 3 + global_index_offset;
		mIndices[mIndicesCount++] = 1 + global_index_offset;

		return;
	}

	/*
	
	topology, front face is counter-clock wise
	________________________________________________
	|    /|    /|    /|    /|    /|    /|    /|    /|
	|   / |   / |   / |   / |   / |   / |   / |   / |
	|  /  |  /  |  /  |  /  |  /  |  /  |  /  |  /  |
	| /   | /   | /   | /   | /   | /   | /   | /   |
	|/____|/____|/____|/____|/____|/____|/____|/____|

	*/
	for (int row = 1; row < display_quad_dim - 1; ++row) {
		mIndices[mIndicesCount++] = RESTART_INDEX;
		
		for (int col = 1; col < display_quad_dim; ++col) {
			int local_idx0 = (row + 1) * (display_quad_dim + 1) + col;
			int local_idx1 = local_idx0 - display_quad_dim - 1;

			mIndices[mIndicesCount++] = local_idx0 + global_index_offset;
			mIndices[mIndicesCount++] = local_idx1 + global_index_offset;
		}
	}

	// check neighbors

	// bottom
	if (block->mYIndex > 0) { // has bottom neighbor
		block_s * bottom_neighbor = mBlocks + (block->mYIndex - 1) * mBlocksXCount + block->mXIndex;
		if (bottom_neighbor->mDisplayLevel > block->mDisplayLevel) {
			SetupBottomEdgeFixGap(block, bottom_neighbor, global_index_offset);
		}
		else {
			SetupBottomEdgeNormally(block, global_index_offset);
		}
	}
	else {
		SetupBottomEdgeNormally(block, global_index_offset);
	}

	// right
	if (block->mXIndex < mBlocksXCount - 1) {
		block_s * right_neighbor = mBlocks + block->mYIndex * mBlocksXCount + block->mXIndex + 1;
		if (right_neighbor->mDisplayLevel > block->mDisplayLevel) {
			SetupRightEdgeFixGap(block, right_neighbor, global_index_offset);
		}
		else {
			SetupRightEdgeNormally(block, global_index_offset);
		}
	}
	else {
		SetupRightEdgeNormally(block, global_index_offset);
	}

	// top
	if (block->mYIndex < mBlocksYCount - 1) {
		block_s * top_neighbor = mBlocks + (block->mYIndex + 1) * mBlocksXCount + block->mXIndex;
		if (top_neighbor->mDisplayLevel > block->mDisplayLevel) {
			SetupTopEdgeFixGap(block, top_neighbor, global_index_offset);
		}
		else {
			SetupTopEdgeNormally(block, global_index_offset);
		}
	}
	else {
		SetupTopEdgeNormally(block, global_index_offset);
	}

	// left
	if (block->mXIndex > 0) {
		block_s * left_neighbor = mBlocks + block->mYIndex * mBlocksXCount + block->mXIndex - 1;
		if (left_neighbor->mDisplayLevel > block->mDisplayLevel) {
			SetupLeftEdgeFixGap(block, left_neighbor, global_index_offset);
		}
		else {
			SetupLeftEdgeNormally(block, global_index_offset);
		}
	}
	else {
		SetupLeftEdgeNormally(block, global_index_offset);
	}
}

/*

bottom edge topology

<------
  ______
 /|\ | \|\ 
/_|_\|_\|_\

*/
void GeoMipMapping::SetupBottomEdgeNormally(block_s *block, unsigned int global_index_offset) {
	int display_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;

	mIndices[mIndicesCount++] = RESTART_INDEX;

	mIndices[mIndicesCount++] = display_quad_dim + global_index_offset;
	for (int col = display_quad_dim - 1; col >= 1; --col) {
		int local_idx0 = display_quad_dim + 1 + col;
		int local_idx1 = local_idx0 - display_quad_dim - 1;

		mIndices[mIndicesCount++] = local_idx0 + global_index_offset;
		mIndices[mIndicesCount++] = local_idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = 0 + global_index_offset;
}

/*

right edge topology

 |
 |
\|/

  /|
 /_|
| /|
|/_|
| /|
|/_|
 \ |
  \|

*/
void GeoMipMapping::SetupRightEdgeNormally(block_s *block, unsigned int global_index_offset) {
	int display_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;

	mIndices[mIndicesCount++] = RESTART_INDEX;

	int last_local_idx = Square(display_quad_dim + 1) - 1;

	mIndices[mIndicesCount++] = last_local_idx + global_index_offset;
	for (int row = display_quad_dim - 1; row >= 1; --row) {
		int local_idx0 = (display_quad_dim + 1) * (row + 1) - 2;
		int local_idx1 = local_idx0 + 1;

		mIndices[mIndicesCount++] = local_idx0 + global_index_offset;
		mIndices[mIndicesCount++] = local_idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = display_quad_dim + global_index_offset;
}

/*

top edge topology

------>

___________
\ |\ |\ | /
 \|_\|_\|/

*/
void GeoMipMapping::SetupTopEdgeNormally(block_s *block, unsigned int global_index_offset) {
	int display_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;

	mIndices[mIndicesCount++] = RESTART_INDEX;

	mIndices[mIndicesCount++] = display_quad_dim * (display_quad_dim + 1) + global_index_offset;
	for (int col = 1; col < display_quad_dim; ++col) {
		int local_idx0 = (display_quad_dim - 1) * (display_quad_dim + 1) + col;
		int local_idx1 = local_idx0 + display_quad_dim + 1;

		mIndices[mIndicesCount++] = local_idx0 + global_index_offset;
		mIndices[mIndicesCount++] = local_idx1 + global_index_offset;
	}
	int last_local_idx = Square(display_quad_dim + 1) - 1;
	mIndices[mIndicesCount++] = last_local_idx + global_index_offset;
}

/*

left edge topology
 
/|\
 |
 |

|\
|_\
| /|
|/_|
| /|
|/_|
| /
|/

*/

void GeoMipMapping::SetupLeftEdgeNormally(block_s *block, unsigned int global_index_offset) {
	int display_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;

	mIndices[mIndicesCount++] = RESTART_INDEX;
	mIndices[mIndicesCount++] = 0 + global_index_offset;
	for (int row = 1; row < display_quad_dim; ++row) {
		int local_idx0 = (display_quad_dim + 1) * row + 1;
		int local_idx1 = local_idx0 - 1;

		mIndices[mIndicesCount++] = local_idx0 + global_index_offset;
		mIndices[mIndicesCount++] = local_idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = display_quad_dim * (display_quad_dim + 1) + global_index_offset;
}

// fix gaps

void GeoMipMapping::SetupBottomEdgeFixGap(block_s *block, block_s *bottom_neighbor, unsigned int global_index_offset) {
	int block_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;
	int neighbor_quad_dim = BLOCK_SIZE >> bottom_neighbor->mDisplayLevel;

	int t = block_quad_dim / neighbor_quad_dim;

	int x_max, x_min;

	bool ret = false;

	// right corner
	x_max = neighbor_quad_dim * t;
	x_min = x_max - t;

	mIndices[mIndicesCount++] = RESTART_INDEX;
	for (int x = x_max; x > x_min; --x) {
		if (x - 1 <= 0) {
			ret = true;
			break;
		}

		int idx0 = x_max;
		int idx1 = (x - 1) + block_quad_dim + 1;
		mIndices[mIndicesCount++] = idx0 + global_index_offset;
		mIndices[mIndicesCount++] = idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = x_min + global_index_offset;

	if (ret) {
		return;
	}

	// left corner
	x_max = t;
	x_min = 0;

	mIndices[mIndicesCount++] = RESTART_INDEX;
	for (int x = x_max; x > x_min; --x) {
		int idx0 = x_max;
		int idx1 = x + block_quad_dim + 1;
		mIndices[mIndicesCount++] = idx0 + global_index_offset;
		mIndices[mIndicesCount++] = idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = x_min + global_index_offset;

	// bottom edge
	for (int i = neighbor_quad_dim - 2; i >= 1; --i) {
		x_max = (i + 1) * t;
		x_min = x_max - t;

		mIndices[mIndicesCount++] = RESTART_INDEX;
		for (int x = x_max; x >= x_min; --x) {
			int idx0 = x_max;
			int idx1 = x + block_quad_dim + 1;
			mIndices[mIndicesCount++] = idx0 + global_index_offset;
			mIndices[mIndicesCount++] = idx1 + global_index_offset;
		}
		mIndices[mIndicesCount++] = x_min + global_index_offset;
	}
}

void GeoMipMapping::SetupRightEdgeFixGap(block_s *block, block_s *right_neighbor, unsigned int global_index_offset) {
	int block_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;
	int neighbor_quad_dim = BLOCK_SIZE >> right_neighbor->mDisplayLevel;

	int t = block_quad_dim / neighbor_quad_dim;

	int y_max, y_min;
	bool ret = false;

	// top corner
	y_max = neighbor_quad_dim * t;
	y_min = y_max - t;

	mIndices[mIndicesCount++] = RESTART_INDEX;
	for (int y = y_max; y > y_min; --y) {
		if (y - 1 == 0) {
			ret = true;
			break;
		}

		int idx0 = y_max * (block_quad_dim + 1) + (block_quad_dim + 1) - 1;
		int idx1 = (y - 1) * (block_quad_dim + 1) + (block_quad_dim + 1) - 1 - 1;
		mIndices[mIndicesCount++] = idx0 + global_index_offset;
		mIndices[mIndicesCount++] = idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = y_min * (block_quad_dim + 1) + (block_quad_dim + 1) - 1 + global_index_offset;

	if (ret) {
		return;
	}

	// bottom corner
	y_max = t;
	y_min = y_max - t;

	mIndices[mIndicesCount++] = RESTART_INDEX;
	for (int y = y_max; y > y_min; --y) {
		int idx0 = y_max * (block_quad_dim + 1) + (block_quad_dim + 1) - 1;
		int idx1 = y * (block_quad_dim + 1) + (block_quad_dim + 1) - 1 - 1;
		mIndices[mIndicesCount++] = idx0 + global_index_offset;
		mIndices[mIndicesCount++] = idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = y_min * (block_quad_dim + 1) + (block_quad_dim + 1) - 1 + global_index_offset;

	// right edge
	for (int i = neighbor_quad_dim - 2; i >= 1; --i) {
		y_max = (i + 1) * t;
		y_min = y_max - t;

		mIndices[mIndicesCount++] = RESTART_INDEX;
		for (int y = y_max; y >= y_min; --y) {
			int idx0 = (y_max + 1) * (block_quad_dim + 1) - 1;
			int idx1 = (y + 1) * (block_quad_dim + 1) - 1 - 1;
			mIndices[mIndicesCount++] = idx0 + global_index_offset;
			mIndices[mIndicesCount++] = idx1 + global_index_offset;
		}
		mIndices[mIndicesCount++] = (y_min + 1) * (block_quad_dim + 1) - 1 + global_index_offset;
	}
}

void GeoMipMapping::SetupTopEdgeFixGap(block_s *block, block_s *top_neighbor, unsigned int global_index_offset) {
	int block_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;
	int neighbor_quad_dim = BLOCK_SIZE >> top_neighbor->mDisplayLevel;

	int t = block_quad_dim / neighbor_quad_dim;

	int x_max, x_min;
	bool ret = false;

	// left corner
	x_min = 0;
	x_max = x_min + t;

	mIndices[mIndicesCount++] = RESTART_INDEX;
	for (int x = x_min; x < x_max; ++x) {
		if (x + 1 >= neighbor_quad_dim * t) {
			ret = true;
			break;
		}

		int idx0 = x_min + block_quad_dim * (block_quad_dim + 1);
		int idx1 = (x + 1) + block_quad_dim * (block_quad_dim + 1) - block_quad_dim - 1;
		mIndices[mIndicesCount++] = idx0 + global_index_offset;
		mIndices[mIndicesCount++] = idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = x_max + block_quad_dim * (block_quad_dim + 1) + global_index_offset;

	if (ret) {
		return;
	}

	// right corner
	x_min = (neighbor_quad_dim - 1) * t;
	x_max = x_min + t;

	mIndices[mIndicesCount++] = RESTART_INDEX;
	for (int x = x_min; x < x_max; ++x) {
		int idx0 = x_min + block_quad_dim * (block_quad_dim + 1);
		int idx1 = x + block_quad_dim * (block_quad_dim + 1) - block_quad_dim - 1;
		mIndices[mIndicesCount++] = idx0 + global_index_offset;
		mIndices[mIndicesCount++] = idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = x_max + block_quad_dim * (block_quad_dim + 1) + global_index_offset;

	// top edge
	for (int i = 1; i <= neighbor_quad_dim - 2; ++i) {
		x_min = i * t;
		x_max = x_min + t;

		mIndices[mIndicesCount++] = RESTART_INDEX;
		for (int x = x_min; x <= x_max; ++x) {
			int idx0 = x_min + block_quad_dim * (block_quad_dim + 1);
			int idx1 = x + block_quad_dim * (block_quad_dim + 1) - block_quad_dim - 1;
			mIndices[mIndicesCount++] = idx0 + global_index_offset;
			mIndices[mIndicesCount++] = idx1 + global_index_offset;
		}
		mIndices[mIndicesCount++] = x_max + block_quad_dim * (block_quad_dim + 1) + global_index_offset;
	}
}

void GeoMipMapping::SetupLeftEdgeFixGap(block_s *block, block_s *left_neighbor, unsigned int global_index_offset) {
	int block_quad_dim = BLOCK_SIZE >> block->mDisplayLevel;
	int neighbor_quad_dim = BLOCK_SIZE >> left_neighbor->mDisplayLevel;

	int t = block_quad_dim / neighbor_quad_dim;

	int y_max, y_min;

	bool ret = false;

	// bottom corner
	y_min = 0;
	y_max = t;

	mIndices[mIndicesCount++] = RESTART_INDEX;
	for (int y = y_min; y < y_max; ++y) {
		if (y + 1 >= neighbor_quad_dim * t) {
			ret = true;
			break;
		}

		int idx1 = (y + 1) * (block_quad_dim + 1) + 1;
		int idx0 = y_min * (block_quad_dim + 1);
		mIndices[mIndicesCount++] = idx0 + global_index_offset;
		mIndices[mIndicesCount++] = idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = y_max * (block_quad_dim + 1) + global_index_offset;

	if (ret) {
		return;
	}

	// top corner
	y_min = (neighbor_quad_dim - 1) * t;
	y_max = y_min + t;

	mIndices[mIndicesCount++] = RESTART_INDEX;
	for (int y = y_min; y < y_max; ++y) {
		int idx1 = y * (block_quad_dim + 1) + 1;
		int idx0 = y_min * (block_quad_dim + 1);
		mIndices[mIndicesCount++] = idx0 + global_index_offset;
		mIndices[mIndicesCount++] = idx1 + global_index_offset;
	}
	mIndices[mIndicesCount++] = y_max * (block_quad_dim + 1) + global_index_offset;

	// left edge
	for (int i = 1; i <= neighbor_quad_dim - 2; ++i) {
		y_min = i * t;
		y_max = y_min + t;

		mIndices[mIndicesCount++] = RESTART_INDEX;
		for (int y = y_min; y <= y_max; ++y) {
			int idx1 = y * (block_quad_dim + 1) + 1;
			int idx0 = y_min * (block_quad_dim + 1);
			mIndices[mIndicesCount++] = idx0 + global_index_offset;
			mIndices[mIndicesCount++] = idx1 + global_index_offset;
		}
		mIndices[mIndicesCount++] = y_max * (block_quad_dim + 1) + global_index_offset;
	}
}
