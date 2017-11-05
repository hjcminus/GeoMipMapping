// Shared.h

#pragma once

#include <stdio.h>

#include <DirectXMath.h>
using namespace DirectX;

/*
================================================================================
Memory
================================================================================
*/

#ifdef _DEBUG
# include <crtdbg.h>
# define	NEW__				new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
# define	NEW__				new
#endif

#define MAX_PATH_				260

#ifdef _UNICODE

typedef wchar_t					char_t;

# define _L(x)					__L(x)
# define __L(x)					L ## x
# define __SRC_FILE__			_L(__FILE__)

# define TXT_(x)				L ## x

# define strlen_				wcslen
# define strstr_				wcsstr
# define strrchr_				wcsrchr
# define strcpy_s_				wcscpy_s
# define strcat_s_				wcscat_s
# define fopen_s_				_wfopen_s
# define sprintf_s_				swprintf_s
# define vsprintf_s_			vswprintf_s
# define fprintf_s_				fwprintf_s

#else

typedef char					char_t;

# define __SRC_FILE__			__FILE__

# define TXT_(x)				x

# define strlen_				strlen
# define strstr_				strstr	
# define strrchr_				strrchr
# define strcpy_s_				strcpy_s
# define strcat_s_				strcat_s
# define fopen_s_				fopen_s
# define sprintf_s_				sprintf_s
# define vsprintf_s_			vsprintf_s
# define fprintf_s_				fprintf_s

#endif

#define SAFE_FREE(p)			do { if (p) { free(p); p = nullptr; } } while (0)

/*
================================================================================
Math
================================================================================
*/

typedef float					vec2_t[2];
typedef float					vec3_t[3];
typedef float					vec4_t[4];

#define PI		3.14159265358979323846f

template<class T>
T			Square(T v) { return v * v; }
void		Vec3Copy(const vec3_t in, vec3_t out);
void		Vec3Set(vec3_t v, float x, float y, float z);
void		Vec3Add(const vec3_t a, const vec3_t b, vec3_t out);
void		Vec3Sub(const vec3_t a, const vec3_t b, vec3_t out);
float		Vec3Dot(const vec3_t a, const vec3_t b);
float		Vec3Normalize(vec3_t v);
float		Vec3Length(const vec3_t v);
float		Vec3SquareLength(const vec3_t v);
void		Vec3Mul(const vec3_t a, float f, vec3_t out);
float		Deg2Rad(float deg);
float		Rad2Deg(float rad);
XMVECTOR	Vec3ToXMVECTOR(const vec3_t v, float w);
void		XMVECTORToVec3(const XMVECTOR &in, vec3_t out);
float		ClampYaw(float yaw);
float		ClampPitch(float pitch);

void		Vec4Copy(const vec4_t in, vec4_t out);
void		Vec4Set(vec4_t v, float x, float y, float z, float w);
void		Vec4Sub(const vec4_t a, const vec4_t b, vec4_t out);
void		Vec4MulAdd(const vec4_t a, float f, const vec4_t b, vec4_t out);

#define FOVY					70.0f
#define NEAR_CLIP_PLANE			1.0f
#define FAR_CLIP_PLANE			8192.0f
#define VIEWPORT_CX				800
#define VIEWPORT_CY				450

struct camera_s {
	vec3_t						mPos;
	vec3_t						mTarget;
	vec3_t						mUp;
	float						mFovy;
	float						mZNear;
	float						mZFar;
};

struct frustum_plane_s {
	vec4_t						mLeftPlane;
	vec4_t						mRightPlane;

	void						Setup(int viewport_width, int viewport_height, const camera_s &cam);
	bool						CullHorizontalCircle(const vec3_t &center, float radius) const;
};

/*
================================================================================
Vertex Format
================================================================================
*/
struct vertex_pos_s {
	vec3_t						mPosition;
};

struct vertex_pos_clr_s {
	vec3_t						mPosition;
	vec4_t						mColor;
};

struct vertex_pos_texcoord_s {
	vec3_t						mPosition;
	vec2_t						mTexCoord;
};

#define							MAX_DRAW_VERT_COUNT	(1024 * 1024 * 2 * 3) // tune this
#define							MAX_DRAW_IDX_COUNT	(1024 * 1024 * 2 * 3) // tune this
#define							RESTART_INDEX		0xffffffff
#define							MAX_PRINT_TEXT_LEN	1024

/*
================================================================================
Terrain Mesh
================================================================================
*/
struct mesh_s {
	vertex_pos_s *				mVertices;
	int							mVerticesCount;
	unsigned int *				mIndices;
	int							mIndicesCount;
	int							mTriangleCount;
};

/*
================================================================================
Demo
================================================================================
*/
#define MAX_TERRAIN_SIZE		4097

// pixel error control, tune this using arrow up / arrow down key
#define DEFAILT_PIXEL_ERROR		2.0f	
#define MIN_PIXEL_ERROR			1.0f
#define MAX_PIXEL_ERROR			4.0f

// viewer movement speed, tune this using page up / page down key
#define	MIN_MOVE_SPEED			1.0f
#define MAX_MOVE_SPEED			100.0f

// viewer rotation
#define MOUSE_SENSITIVITY		0.2f

struct demo_info_s {
	double						mUpdateTimeElapsed;
	float						mMaxPixelError;
};

struct test_data_s {
	const char_t *				mHeightMap;
	const char_t *				mColorMap;
	float						mElevationScale;
};

extern const test_data_s 		TEST_DATA[2];

void ToggleFlags(unsigned int &flags, unsigned int f);

/*
================================================================================
Log
================================================================================
*/
#define	SYS_ERROR(fmt, ...)		Sys_Error(__SRC_FILE__, __LINE__, fmt, __VA_ARGS__)

void	Sys_Error(const char_t *file, int line, const char_t *fmt, ...);

/*
================================================================================
String
================================================================================
*/
void	Str_ExtractDirSelf(char_t *s);
void	Str_ExtractExeDir(const char_t *exe, char_t *dir, int dir_size);
int		Str_ToAnsi(const char_t *src, char * dst, int dst_chars);
int		Str_FrAnsi(const char *src, char_t * dst, int dst_chars);

/*
================================================================================
File
================================================================================
*/
int		File_LoadBinary(const char_t * filename, char *buffer, int buffer_size);

/*
================================================================================
Timer
================================================================================
*/
void	Sys_InitTimer();
double	Sys_GetRelativeTime();	// seconds

/*
================================================================================
Image
================================================================================
*/

#define IMG_FORMAT_1BYTE_GRAY	1
#define IMG_FORMAT_2BYTE_GRAY	2
#define IMG_FORMAT_1BYTE_RGB	3
#define IMG_FORMAT_1BYTE_RGBA	4

struct image_s {
	int							mFormat;
	int							mCX;
	int							mCY;
	void *						mData;
};

bool	ReadPNG(const char_t *filename, image_s *image);
bool	SavePNG(const char_t *filename, const image_s *image);
void	FreeImg(image_s *image);

/*
================================================================================
Input
================================================================================
*/
#define IN_LBUTTON_DOWN			1
#define IN_LBUTTON_UP			2
#define IN_MOUSE_MOVE			3
#define IN_KEY_DOWN				4
#define IN_KEY_UP				5

struct input_s {
	int							mType;
	int							mParam1;
	int							mParam2;
};
