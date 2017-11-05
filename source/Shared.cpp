// Shared.cpp

#include "Precompiled.h"
#include "png.h"

/*
================================================================================
Math
================================================================================
*/

void Vec3Copy(const vec3_t in, vec3_t out) {
	memcpy(out, in, sizeof(vec3_t));
}

void Vec3Set(vec3_t v, float x, float y, float z) {
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void Vec3Add(const vec3_t a, const vec3_t b, vec3_t out) {
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}

void Vec3Sub(const vec3_t a, const vec3_t b, vec3_t out) {
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}

float Vec3Dot(const vec3_t a, const vec3_t b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float Vec3Normalize(vec3_t v) {
	float len = Vec3Length(v);
	if (len > 0.0f) {
		float i = 1.0f / len;
		v[0] *= i;
		v[1] *= i;
		v[2] *= i;
	}

	return len;
}

float Vec3Length(const vec3_t v) {
	return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float Vec3SquareLength(const vec3_t v) {
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

void Vec3Mul(const vec3_t a, float f, vec3_t out) {
	out[0] = a[0] * f;
	out[1] = a[1] * f;
	out[2] = a[2] * f;
}

float Deg2Rad(float deg) {
	return deg * PI / 180.0f;
}

float Rad2Deg(float rad) {
	return rad * 180.0f / PI;
}

XMVECTOR Vec3ToXMVECTOR(const vec3_t v, float w) {
	return XMVectorSet(v[0], v[1], v[2], w);
}

void XMVECTORToVec3(const XMVECTOR &in, vec3_t out) {
	out[0] = in.m128_f32[0];
	out[1] = in.m128_f32[1];
	out[2] = in.m128_f32[2];
}

float ClampYaw(float yaw) {
	while (yaw < 0.0f) {
		yaw += 360.0f;
	}

	while (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	return yaw;
}

float ClampPitch(float pitch) {
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}

	return pitch;
}

void Vec4Copy(const vec4_t in, vec4_t out) {
	memcpy(out, in, sizeof(vec4_t));
}

void Vec4Set(vec4_t v, float x, float y, float z, float w) {
	v[0] = x;
	v[1] = y;
	v[2] = z;
	v[3] = w;
}

void Vec4Sub(const vec4_t a, const vec4_t b, vec4_t out) {
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
	out[3] = a[3] - b[3];
}

void Vec4MulAdd(const vec4_t a, float f, const vec4_t b, vec4_t out) {
	out[0] = a[0] * f + b[0];
	out[1] = a[1] * f + b[1];
	out[2] = a[2] * f + b[2];
	out[3] = a[3] * f + b[3];
}

void frustum_plane_s::Setup(int viewport_width, int viewport_height, const camera_s &cam) {
	float tan1 = tanf(Deg2Rad(cam.mFovy * 0.5f));
	float d = (float)viewport_height * 0.5f / tan1;
	float tan2 = (float)viewport_width * 0.5f / d;
	float fovx = Rad2Deg(atanf(tan2)) * 2.0f;

	vec3_t forward;
	Vec3Sub(cam.mTarget, cam.mPos, forward);
	Vec3Normalize(forward);

	XMVECTOR up = Vec3ToXMVECTOR(cam.mUp, 0.0f);
	XMVECTOR f = Vec3ToXMVECTOR(forward, 0.0f);
	XMMATRIX m;

	float deg = -(90.0f - fovx * 0.5f);
	m = DirectX::XMMatrixRotationAxis(up, Deg2Rad(deg));
	XMVECTOR n = DirectX::XMVector4Transform(f, m);
	mLeftPlane[0] = n.m128_f32[0];
	mLeftPlane[1] = n.m128_f32[1];
	mLeftPlane[2] = n.m128_f32[2];
	mLeftPlane[3] = -Vec3Dot(cam.mPos, mLeftPlane);
	
	deg = (90.0f - fovx * 0.5f);
	m = DirectX::XMMatrixRotationAxis(up, Deg2Rad(deg));
	n = DirectX::XMVector4Transform(f, m);
	mRightPlane[0] = n.m128_f32[0];
	mRightPlane[1] = n.m128_f32[1];
	mRightPlane[2] = n.m128_f32[2];
	mRightPlane[3] = -Vec3Dot(cam.mPos, mRightPlane);
}

bool frustum_plane_s::CullHorizontalCircle(const vec3_t &center, float radius) const {
	vec3_t temp;

	memcpy(temp, mLeftPlane, sizeof(float) * 3);
	float dist = Vec3Dot(temp, center) + mLeftPlane[3];
	if (dist < -radius) {
		return true;
	}

	memcpy(temp, mRightPlane, sizeof(float) * 3);
	dist = Vec3Dot(temp, center) + mRightPlane[3];
	if (dist < -radius) {
		return true;
	}

	return false;
}

/*
================================================================================
Demo
================================================================================
*/
const test_data_s TEST_DATA[2] = {
	// Grand Canyon
	// horizontal: 60 meters / inter-pixel spacing, vertical: 10  meters / pixel unit
	{ TXT_("gcanyon_height_4k2k.png"), TXT_("gcanyon_color_4k2k.png"), 14.0f / 60.0f }, // exaggerate the elevation by 40% to make the demo more challenging.

																						// Puget Sound
																						// horizontal: 40 meters / inter-pixel spacing, vertical: 0.1 meters / pixel unit
	{ TXT_("ps_height_4k.png"), TXT_("ps_texture_4k.png"), 0.1f / 40.0f }
};

void ToggleFlags(unsigned int &flags, unsigned int f) {
	if (flags & f) {
		flags &= ~f;
	}
	else {
		flags |= f;
	}
}

/*
================================================================================
Log
================================================================================
*/
void Sys_Error(const char_t *file, int line, const char_t *fmt, ...) {
	char_t buffer[4096];

	sprintf_s_(buffer, TXT_("file: %s, line: %d, msg: "), file, line);
	int len = (int)strlen_(buffer);

	va_list argptr;
	va_start(argptr, fmt);
	vsprintf_s_(buffer + len, 4096 - len, fmt, argptr);
	va_end(argptr);

	fprintf_s_(stderr, TXT_("%s\n"), buffer);
}

/*
================================================================================
File
================================================================================
*/
static int FileSize(FILE * f) {
	int pos = ftell(f);
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, pos, SEEK_SET);

	return size;
}

int	File_LoadBinary(const char_t * filename, char *buffer, int buffer_size) {
	int read = 0;

	FILE * f = nullptr;
	if (fopen_s_(&f, filename, TXT_("rb")) == 0) {
		int size = FileSize(f);
		if (size <= buffer_size) {
			read = (int)fread(buffer, 1, (size_t)size, f);
		}
		fclose(f);
	}

	return read;
}

/*
================================================================================
Timer
================================================================================
*/
static double	gSecondsPerCycle = 1.0;
static __int64	gBaseCycle = 0;

void Sys_InitTimer() {
	__int64 f;
	QueryPerformanceFrequency((LARGE_INTEGER*)&f); //call QueryPerformanceFrequency only once, this value would not change during machine running
	gSecondsPerCycle = 1.0 / f;

	QueryPerformanceCounter((LARGE_INTEGER*)&gBaseCycle);
}

double Sys_GetRelativeTime() {
	__int64 c;
	QueryPerformanceCounter((LARGE_INTEGER*)&c);
	return (c - gBaseCycle) * gSecondsPerCycle;
}

/*
================================================================================
String
================================================================================
*/
static char_t * FindDoubleDots(char_t *s) {
	char_t * pc = strstr_(s, TXT_("..\\"));
	if (!pc) {
		pc = strstr_(s, TXT_("../"));
	}
	return pc;
}

static char_t * RFindSlash(char_t *s) {
	char_t * pc = strrchr_(s, TXT_('\\'));
	if (!pc) {
		pc = strrchr_(s, TXT_('/'));
	}
	return pc;
}

void Str_EraseDoubleDots(char_t *s) {
	char_t sl[2][MAX_PATH_];

	strcpy_s_(sl[0], s);
	strcpy_s_(sl[1], s);

	int srcidx = 0;
	int dstidx = 1 - srcidx;

	char_t * pc = FindDoubleDots(sl[srcidx]);
	while (pc) {
		*pc = 0;

		char_t * pc1 = RFindSlash(sl[srcidx]);
		if (pc1) {
			*pc1 = 0;

			char_t * pc2 = RFindSlash(sl[srcidx]);
			if (pc2) {
				pc2[1] = 0;
				strcpy_s_(sl[dstidx], sl[srcidx]);
				strcat_s_(sl[dstidx], pc + 3);

				srcidx = 1 - srcidx;
				dstidx = 1 - srcidx;

				pc = FindDoubleDots(sl[srcidx]);
			}
			else {
				break;
			}

		}
		else {
			break;
		}
	}

	strcpy_s_(s, MAX_PATH_, sl[srcidx]);
}

void Str_ExtractDirSelf(char_t *s) {
	char_t * pc = strrchr_(s, TXT_('\\'));
	if (!pc) {
		pc = strrchr_(s, '/');
	}

	if (pc) {
		*pc = 0;
	}
}

void Str_ExtractExeDir(const char_t *exe, char_t *dir, int dir_size) {
	strcpy_s_(dir, dir_size, exe);
	Str_EraseDoubleDots(dir);
	Str_ExtractDirSelf(dir);
}

int Str_ToAnsi(const char_t *src, char * dst, int dst_chars) {
#ifdef _UNICODE

	if (!src) {
		if (dst && dst_chars > 0) {
			dst[0] = 0;
		}
		return 0;
	}

	int wide_chars = (int)strlen_(src);
	int need_chars = WideCharToMultiByte(CP_ACP, 0, src, wide_chars, nullptr, 0, nullptr, nullptr); // returns the number of characters
	if (!dst) {
		return need_chars;
	}

	if (dst_chars < need_chars + 1) {
		dst[0] = 0;
		SYS_ERROR(TXT_("ansi buffer too small\n"));
		return 0;
	}

	int translen = WideCharToMultiByte(CP_ACP, 0, src, wide_chars, dst, dst_chars, nullptr, nullptr);
	dst[translen] = 0;

	return translen;

#else

	int src_chars = (int)strlen_(src);
	if (dst_chars < src_chars + 1) {
		dst[0] = 0;
		SYS_ERROR(TXT_("ansi buffer too small\n");
		return 0;
	}

	strcpy_s_(dst, dst_chars, src);

	return src_chars;

#endif
}

int	Str_FrAnsi(const char *src, char_t * dst, int dst_chars) {
#ifdef _UNICODE

	if (!src) {
		if (dst && dst_chars > 0) {
			dst[0] = 0;
		}
		return 0;
	}


	int ansi_chars = (int)strlen(src);
	int need_chars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, ansi_chars, nullptr, 0); // returns the number of characters
	if (!dst) {
		return need_chars;
	}

	if (dst_chars < need_chars + 1) {
		dst[0] = 0;
		SYS_ERROR(TXT_("wide buffer too small\n"));
		return 0;
	}

	int translen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, ansi_chars, dst, dst_chars);
	dst[translen] = 0;

	return translen;

#else

	int src_chars = (int)wcslen(src);
	if (dst_chars < src_chars + 1) {
		dst[0] = 0;
		SYS_ERROR(TXT_("wide buffer too small\n");
		return 0;
	}

	wcscpy_s(dst, dst_chars, src);

	return src_chars;
	
#endif
}

/*
================================================================================
Image
================================================================================
*/
bool ReadPNG(const char_t *filename, image_s *image) {
	char ansi_filename[MAX_PATH_];
	Str_ToAnsi(filename, ansi_filename, MAX_PATH_);

	png_image image_png;
	memset(&image_png, 0, sizeof(image_png));
	image_png.version = PNG_IMAGE_VERSION;

	if (png_image_begin_read_from_file(&image_png, ansi_filename)) {
		size_t pixel_size = 0;

		// check file format
		switch (image_png.format) {
		case PNG_FORMAT_GRAY:
			image->mFormat = IMG_FORMAT_1BYTE_GRAY;
			pixel_size = 1;
			break;
		case PNG_FORMAT_LINEAR_Y:
			image->mFormat = IMG_FORMAT_2BYTE_GRAY;
			pixel_size = 2;
			break;
		case PNG_FORMAT_RGB:
			image->mFormat = IMG_FORMAT_1BYTE_RGB;
			pixel_size = 3;
			break;
		case PNG_FORMAT_RGBA:
			image->mFormat = IMG_FORMAT_1BYTE_RGBA;
			pixel_size = 4;
			break;
		default:
			SYS_ERROR(L"unsupported format\n");
			return false;
		}

		size_t size = PNG_IMAGE_SIZE(image_png); // get image size
		png_bytep buffer_png = (png_bytep)malloc(size);

		if (png_image_finish_read(&image_png, NULL/*background*/, buffer_png, 0/*row_stride*/, nullptr/*colormap*/)) {

			image->mCX = image_png.width;
			image->mCY = image_png.height;
			image->mData = malloc(image_png.height * image_png.width * pixel_size);
			memcpy(image->mData, buffer_png, image_png.height * image_png.width * pixel_size);

			png_image_free(&image_png);
			free(buffer_png);

			return true;
		}
		else {
			png_image_free(&image_png);
			free(buffer_png);

			SYS_ERROR(L"png_image_finish_read failed\n");
			return false;
		}
	}
	else {
		SYS_ERROR(L"png_image_begin_read_from_file failed\n");
		return false;
	}
}

bool SavePNG(const char_t *filename, const image_s *image) {
	if (!image->mData || image->mCX < 1 || image->mCY < 1) {
		SYS_ERROR(L"bad data\n");
		return false;
	}

	png_uint_32 dst_format = 0;
	size_t pixel_size = 0;

	switch (image->mFormat) {
	case IMG_FORMAT_1BYTE_GRAY:
		dst_format = PNG_FORMAT_GRAY;
		pixel_size = 1;
		break;
	case IMG_FORMAT_2BYTE_GRAY:
		dst_format = PNG_FORMAT_LINEAR_Y;
		pixel_size = 2;
		break;
	case IMG_FORMAT_1BYTE_RGB:
		dst_format = PNG_FORMAT_RGB;
		pixel_size = 3;
		break;
	case IMG_FORMAT_1BYTE_RGBA:
		dst_format = PNG_FORMAT_RGBA;
		pixel_size = 4;
		break;
	default:
		SYS_ERROR(L"unsupported format\n");
		return false;
	}

	png_image image_png;
	memset(&image_png, 0, sizeof(image_png));
	image_png.version = PNG_IMAGE_VERSION;
	image_png.width = image->mCX;
	image_png.height = image->mCY;
	image_png.format = dst_format;


	size_t size = PNG_IMAGE_SIZE(image_png); // get image size
	png_bytep buffer_png = (png_bytep)malloc(size);

	/*
	// convert height direction
	for (int h = 0; h < image->mCY; ++h) {
		const char * src_line = (const char *)image->mData + h * image->mCX * pixel_size;
		png_bytep dst_line = buffer_png + (image->mCY - h - 1) * image->mCX * pixel_size;
		memcpy(dst_line, src_line, image->mCX * pixel_size);
	}
	*/
	memcpy(buffer_png, image->mData, image->mCY * image->mCX * pixel_size);

	// save to png file
	char ansi_filename[MAX_PATH_];
	Str_ToAnsi(filename, ansi_filename, MAX_PATH_);
	if (png_image_write_to_file(&image_png, ansi_filename, 0, buffer_png, PNG_IMAGE_ROW_STRIDE(image_png), nullptr/*colormap*/)) {
		png_image_free(&image_png);
		free(buffer_png);
		return true;
	}
	else {
		SYS_ERROR(L"png_image_write_to_file failed\n");
		free(buffer_png);
		return false;
	}
}

void FreeImg(image_s *image) {
	if (image->mData) {
		free(image->mData);
		image->mData = nullptr;
		image->mCX = image->mCY = 0;
	}
}
