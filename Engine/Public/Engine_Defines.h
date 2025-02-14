#pragma once

#pragma warning (disable : 4251)
#pragma warning(disable : 5208)

#include <d3d11.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
using namespace DirectX;

#include "Effects11\d3dx11effect.h"
#include "DirectXTK\DDSTextureLoader.h"
#include "DirectXTK\WICTextureLoader.h"
#include "DirectXTK\ScreenGrab.h"
#include "DirectXTK\PrimitiveBatch.h"
#include "DirectXTK\VertexTypes.h"
#include "DirectXTK\Effects.h"

#include "DirectXTK\SpriteBatch.h"
#include "DirectXTK\SpriteFont.h"

#include "assimp\scene.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\version.h"

#include <d3dcompiler.h>

namespace Engine
{
	enum class PROTOTYPE { PROTO_GAMEOBJ, PROTO_COMPONENT };

	enum MOUSEKEYSTATE { DIM_LB, DIM_RB, DIM_MB, DIM_END };
	enum MOUSEMOVESTATE { DIMS_X, DIMS_Y, DIMS_Z, DIMS_END };
	enum ABLE_STATE {ENABLE, DISABLE, NONEABLESTATE};
	//const unsigned int g_iMaxWidth = 2560;
	//const unsigned int g_iMaxHeight = 1440;
	const unsigned int g_iMaxWidth = 1280;
	const unsigned int g_iMaxHeight = 720;
	/*const unsigned int g_iMaxWidth = 16384;
	const unsigned int g_iMaxHeight = 9216;*/
	//const unsigned int g_iMaxWidth = 8192;
	//const unsigned int g_iMaxHeight = 4608;

	// 기본 애니매이션 재생, 루트 모션(위치는 원점 고정, 회전은 유지), 항등(위치, 회전 모두 항등, 원점에서 회전없이 재생), RootBone데이터를 따라가는 타입
	enum class ANIM_PLAY_TYPE { ANIM_DEFAULT, ANIM_ROOT_MOTION, ANIM_IDENTITY, ANIM_FOLLOW_ROOT_BONE, };

	// client
	// enum class ANIM_BLEND_GROUP_TYPE { NONE, LOWER_BODY, UPPER_BODY, ARM_LEFT, ARM_RIGHT, HEAD, TOTAL, BLEND_END }; // 250124

	enum class ANIM_BLEND_GROUP_TYPE { NONE, LOWER_BODY, UPPER_BODY, ARM_LEFT, ARM_RIGHT, HEAD, TOTAL, LOWERUPPER, UPPERLOWER, BLEND_END }; // 250210

	enum MODEL_TYPE { TYPE_NONANIM, TYPE_ANIM, TYPE_END };

	// animtool

	using Abgt = ANIM_BLEND_GROUP_TYPE;
	using Abgti = std::underlying_type_t<Abgt>;  // enum class의 기본형 (_int 대체)

	constexpr Abgti						iLower = static_cast<Abgti>(ANIM_BLEND_GROUP_TYPE::LOWER_BODY);
	constexpr Abgti						iTotal = static_cast<Abgti>(ANIM_BLEND_GROUP_TYPE::TOTAL);
	constexpr Abgti						low = static_cast<Abgti>(ANIM_BLEND_GROUP_TYPE::LOWER_BODY);
	constexpr Abgti						tot = static_cast<Abgti>(ANIM_BLEND_GROUP_TYPE::TOTAL);
	constexpr Abgti						NON = -1;
}

#include <functional>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <algorithm>
using namespace std;

#include "Engine_Typedef.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Function.h"

using namespace Engine;

#ifdef _DEBUG

#pragma region ImGui
#define IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#define IMAPP_IMPL
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#pragma endregion

#pragma region ImGuizmo
#include "ImGui/ImGuizmo/GraphEditor.h"
#include "ImGui/ImGuizmo/ImCurveEdit.h"
#include "ImGui/ImGuizmo/ImGradient.h"
#include "ImGui/ImGuizmo/ImGuizmo.h"
#include "ImGui/ImGuizmo/ImSequencer.h"
#include "ImGui/ImGuizmo/ImZoomSlider.h"
#pragma endregion

#pragma region ImNodes
#include "ImGui/ImNodes/imnodes.h"
#include "ImGui/ImNodes/imnodes_internal.h"
#pragma endregion

#include <stdexcept>


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 



#endif

#endif // _DEBUG

