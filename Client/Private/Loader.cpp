#include "stdafx.h"
#include "..\Public\Loader.h"

#include "GameInstance.h"

#include "Particle_Explosion.h"
#include "Particle_Snow.h"
#include "Camera_Free.h"
#include "Body_Player.h"
#include "BackGround.h"
#include "Terrain.h"
#include "Monster.h"
#include "Player.h"
#include "Weapon.h"
#include "Effect.h"
#include "Sky.h"
#include "VIBuffer_Square.h"
#include "VIBuffer_Line.h"

// --------------------------------------------------------
// AnimTool Headers
// --------------------------------------------------------
#include "AT_Cam.h"
#include "AT_Obj.h"
#include "AT_PartAnimObj.h"
#include "AT_Terrain.h"
#include "AT_Square.h"
#include "AT_Mouse.h"
#include "AT_Stair.h"
#include "AT_Ray.h"


CLoader::CLoader(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);

}

//typedef unsigned(__stdcall* _beginthreadex_proc_type)(void*);
_uint APIENTRY LoadingMain(void* pArg)
{	
	CLoader*	pLoader = static_cast<CLoader*>(pArg);

	if (FAILED(pLoader->Loading()))
		return 1;

	return 0;
}

HRESULT CLoader::Initialize(LEVELID eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;

	InitializeCriticalSection(&m_Critical_Section);

	/* 내 코드를 읽어나갈수 있는 하나의 스레드를 생성한다. */
	/* 생성한 스레드가 호출해야할 진입점함수를 지정해준다. */
	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, LoadingMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Loading()
{
	EnterCriticalSection(&m_Critical_Section);

	CoInitializeEx( nullptr, 0);

	HRESULT		hr = {};

	switch (m_eNextLevelID)
	{
	case LEVEL_LOGO:
		hr = Loading_Level_Logo();
		break;
	case LEVEL_GAMEPLAY:
		hr = Loading_Level_GamePlay();
		break;
	case LEVEL_ANIMTOOL:
		hr = Loading_Level_AnimTool();
		break;
	}

	if (FAILED(hr))
		return E_FAIL;

	LeaveCriticalSection(&m_Critical_Section);

	return S_OK;
}

#ifdef _DEBUG

void CLoader::Show_Debug()
{
	SetWindowText(g_hWnd, m_szLoadingText);
}

#endif

HRESULT CLoader::Loading_Level_Logo()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐를 로딩중입니다."));
	/* For.Prototype_Component_Texture_Logo */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_LOGO, TEXT("Prototype_Component_Texture_Logo"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/resources/Textures/Default%d.jpg"), 2))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("사운드를 로딩중입니다."));

	lstrcpy(m_szLoadingText, TEXT("쉐이더를 로딩중입니다."));
	
	lstrcpy(m_szLoadingText, TEXT("모델(을)를 로딩중입니다."));

	lstrcpy(m_szLoadingText, TEXT("객체원형(을)를 로딩중입니다."));
	/* For.Prototype_GameObject_BackGround */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_LOGO, TEXT("Prototype_GameObject_BackGround"),
		CBackGround::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_Level_GamePlay()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐를 로딩중입니다."));
	/* For.Prototype_Component_Texture_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/resources/Textures/Terrain/Tile%d.dds"), 2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Terrain_Mask */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Mask"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/resources/Textures/Terrain/Mask.dds"), 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Terrain_Brush */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Brush"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/resources/Textures/Terrain/Brush.png"), 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Sky */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Sky"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/resources/Textures/SkyBox/Sky_%d.dds"), 4))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Snow */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Snow"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/resources/Textures/Snow/Snow.png"), 1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Effect */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Effect"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/resources/Textures/Explosion/Explosion%d.png"), 90))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("사운드를 로딩중입니다."));
		

	lstrcpy(m_szLoadingText, TEXT("쉐이더를 로딩중입니다."));
	/* For.Prototype_Component_Shader_VtxNorTex*/
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxNorTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/shaderFiles/Shader_VtxNorTex.hlsl"), VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/shaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxAnimMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/shaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxCube */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxCube"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/shaderFiles/Shader_VtxCube.hlsl"), VTXCUBE::Elements, VTXCUBE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxRectInstance */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxRectInstance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/shaderFiles/Shader_VtxRectInstance.hlsl"), VTXRECTINSTANCE::Elements, VTXRECTINSTANCE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPointInstance */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxPointInstance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/shaderFiles/Shader_VtxPointInstance.hlsl"), VTXPOINTINSTANCE::Elements, VTXPOINTINSTANCE::iNumElements))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("콜라이더(을)를 로딩중입니다."));

	/* For.Prototype_Component_Collider_AABB */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_AABB"),
		CCollider::Create(m_pDevice, m_pContext, CCollider::TYPE_AABB))))
		return E_FAIL;

	/* For.Prototype_Component_Collider_OBB */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"),
		CCollider::Create(m_pDevice, m_pContext, CCollider::TYPE_OBB))))
		return E_FAIL;

	/* For.Prototype_Component_Collider_Sphere */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_Sphere"),
		CCollider::Create(m_pDevice, m_pContext, CCollider::TYPE_SPHERE))))
		return E_FAIL;

	


	 
	 
	 lstrcpy(m_szLoadingText, TEXT("모델(을)를 로딩중입니다."));
	 /* For.Prototype_Component_VIBuffer_Terrain */
	 if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Terrain"),
	 	CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../Bin/resources/Textures/Terrain/Height.bmp")))))
	 	return E_FAIL;
	 
	 /* For.Prototype_Component_VIBuffer_Cube */
	 if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Cube"),
	 	CVIBuffer_Cube::Create(m_pDevice, m_pContext))))
	 	return E_FAIL;
	 
	 
	 
	 
	 /* For.Prototype_Component_VIBuffer_Particle_Snow */
	 CVIBuffer_Particle_Instance::PARTICLE_DESC			SnowDesc{};
	 
	 SnowDesc.iNumInstances = 2000;
	 SnowDesc.vCenter = _float3(64.f, 20.f, 64.f);
	 SnowDesc.vRange = _float3(128.f, 1.f, 128.f);
	 SnowDesc.vSpeed = _float2(4.f, 10.f);
	 SnowDesc.vLifeTime = _float2(2.f, 4.f);
	 SnowDesc.vSize = _float2(0.2f, 0.5f);
	 /*Desc.vPivot = _float3();*/
	 SnowDesc.isLoop = true;
	 
	 if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Particle_Snow"),
	 	CVIBuffer_Particle_Rect::Create(m_pDevice, m_pContext, SnowDesc))))
	 	return E_FAIL;
	 
	 /* For.Prototype_Component_VIBuffer_Particle_Explosion */
	 CVIBuffer_Particle_Instance::PARTICLE_DESC			ExplosionDesc{};
	 
	 ExplosionDesc.iNumInstances = 700;
	 ExplosionDesc.vCenter = _float3(0.f, 0.f, 0.f);
	 ExplosionDesc.vRange = _float3(1.f, 1.f, 1.f);
	 ExplosionDesc.vSpeed = _float2(4.f, 10.f);
	 ExplosionDesc.vLifeTime = _float2(0.3f, 1.f);
	 ExplosionDesc.vSize = _float2(0.2f, 0.5f);
	 ExplosionDesc.vPivot = _float3(0.f, -1.f, 0.f);
	 ExplosionDesc.isLoop = true;
	 
	 if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Particle_Explosion"),
	 	CVIBuffer_Particle_Point::Create(m_pDevice, m_pContext, ExplosionDesc))))
	 	return E_FAIL;
	 
	 
	 _matrix		PreTransformMatrix = XMMatrixIdentity();
	 
	  /* For.Prototype_Component_Model_Fiona */
	  PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));
	  
	  if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Fiona"),
	  	CModel::Create(m_pDevice, m_pContext, MODEL_TYPE::TYPE_ANIM, "../Bin/resources/Models/Fiona/Fiona.fbx", PreTransformMatrix))))
	  	return E_FAIL;
	 
	 /* For.Prototype_Component_Model_ForkLift */
	 PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	 
	 if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_ForkLift"),
	 	CModel::Create(m_pDevice, m_pContext, MODEL_TYPE::TYPE_NONANIM, "../Bin/resources/Models/ForkLift/ForkLift.fbx", PreTransformMatrix))))
	 	return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("네비게이션원형(을)를 로딩중입니다."));
	/* For.Prototype_Component_Navigation */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Navigation"),
		CNavigation::Create(m_pDevice, m_pContext, TEXT("../Bin/DataFiles/Navigation.dat")))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("객체원형(을)를 로딩중입니다."));

	/* For.Prototype_GameObject_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Terrain"),
		CTerrain::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	/* For.Prototype_GameObject_Camera_Free */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Monster */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Monster"),
		CMonster::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Player */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Body_Player */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Body_Player"),
		CBody_Player::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Weapon */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Weapon"),
		CWeapon::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Sky */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Sky"),
		CSky::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Particle_Snow */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Particle_Snow"),
		CParticle_Snow::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Particle_Explosion */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Particle_Explosion"),
		CParticle_Explosion::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Effect */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect"),
		CEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_Level_AnimTool()
{
	_int iLevelID = LEVEL_ANIMTOOL;

	lstrcpy(m_szLoadingText, TEXT("AnimToolCamera를 로딩중입니다..."));
	{
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_GameObject_AnimToolCamera"), CAnimTool_Camera::Create(m_pDevice, m_pContext))))
			return E_FAIL;
	}

	lstrcpy(m_szLoadingText, TEXT("AnimTool 쉐이더 로딩중입니다..."));
	{
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
			CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Shader_VtxMesh"),
			CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
			return E_FAIL;

		// Square 쉐이더 로드
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Shader_VtxPosNorTex"), CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosNorTex.hlsl"), VTX_POSNORTEX::Elements, VTX_POSNORTEX::iNumElements))))
			return E_FAIL;

		// Ray 쉐이더 로드
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Shader_VtxPosColor"), CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosColor.hlsl"), VTX_POSCOLOR::Elements, VTX_POSCOLOR::iNumElements))))
			return E_FAIL;
	}

	lstrcpy(m_szLoadingText, TEXT("AnimTool 버퍼 로딩중입니다..."));
	{
		// 마우스 버퍼 (다용도)
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_VIBuffer_Square"), CVIBuffer_Square::Create(m_pDevice, m_pContext))))
			return E_FAIL;

		// Ray 버퍼
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_VIBuffer_Line"), CVIBuffer_Line::Create(m_pDevice, m_pContext))))
			return E_FAIL;
	}

	lstrcpy(m_szLoadingText, TEXT("AnimTool 텍스쳐 로딩중입니다..."));
	{
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Texture_Test"),
			CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/resources/Textures/Logo/Logo.png")))))
			return E_FAIL;
	}

	lstrcpy(m_szLoadingText, TEXT("AnimTool 모델 로딩중입니다..."));
	{
		_matrix		PreTransformMatrix = XMMatrixIdentity();

		///* For.Prototype_Component_Model_Fiona */
		//PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));

		//if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Model_Fiona"), CModel::Create(m_pDevice, m_pContext, CModel::TYPE_ANIM, "../Bin/resources/Models/Fiona/Fiona.fbx", PreTransformMatrix))))
		//	return E_FAIL;

		if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Model_AT_Terrain"), CModel::Create(m_pDevice, m_pContext, MODEL_TYPE::TYPE_NONANIM, "../Bin/Resources/Models/AT_Terrain/Boss_Stage.fbx", PreTransformMatrix))))
			return E_FAIL;

		// if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Model_AT_Terrain_Test"), CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, "../Bin/Resources/Models/AT_TestTerrain/source/Modern Slim Laptop.fbx", PreTransformMatrix)))) return E_FAIL;

		//if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Model_AT_Terrain_Test"), CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, "../Bin/Resources/Models/AT_TestTerrain/Ramps_Export.fbx", PreTransformMatrix))))
		//	return E_FAIL;

		//if (m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Model_AT_Stair"), CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, "../Bin/Resources/Models/AT_Stair/s1/Stairs.fbx", PreTransformMatrix)) != S_OK)
		//	return E_FAIL;


	/*	if (m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Model_AT_Stair"), CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, "../Bin/Resources/Models/AT_Stair/s2/Stairs.fbx", PreTransformMatrix)) != S_OK)
			return E_FAIL;*/



		if (m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Model_AT_Stair"), CModel::Create(m_pDevice, m_pContext, MODEL_TYPE::TYPE_NONANIM, "../Bin/Resources/Models/AT_Stair/s3/EscadariaTriangulate.fbx", PreTransformMatrix)) != S_OK)
			return E_FAIL;
	}

	lstrcpy(m_szLoadingText, TEXT("AnimTool 오브젝트 로딩중입니다..."));
	{
		// AnimTool Object
		{
			if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_GameObject_AT_Obj"), CAT_Obj::Create(m_pDevice, m_pContext))))
				return E_FAIL;

			if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_GameObject_AT_PartAnimObj"), CAT_PartAnimObj::Create(m_pDevice, m_pContext))))
				return E_FAIL;

			if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_GameObject_AT_Terrain"), CAT_Terrain::Create(m_pDevice, m_pContext))))
				return E_FAIL;

			if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_GameObject_AT_Square"), CAT_Square::Create(m_pDevice, m_pContext))))
				return E_FAIL;

			if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_GameObject_AT_Mouse"), CAT_Mouse::Create(m_pDevice, m_pContext))))
				return E_FAIL;

			if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_GameObject_AT_Stair"), CAT_Stair::Create(m_pDevice, m_pContext))))
				return E_FAIL;

			if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_GameObject_AT_Ray"), CAT_Ray::Create(m_pDevice, m_pContext))))
				return E_FAIL;
		}
	}

	lstrcpy(m_szLoadingText, TEXT("콜라이더(을)를 로딩중입니다."));

	/* For.Prototype_Component_Collider_AABB */
	if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Collider_AABB"),
		CCollider::Create(m_pDevice, m_pContext, CCollider::TYPE_AABB))))
		return E_FAIL;

	/* For.Prototype_Component_Collider_OBB */
	if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Collider_OBB"),
		CCollider::Create(m_pDevice, m_pContext, CCollider::TYPE_OBB))))
		return E_FAIL;

	/* For.Prototype_Component_Collider_Sphere */
	if (FAILED(m_pGameInstance->Add_Prototype(iLevelID, TEXT("Prototype_Component_Collider_Sphere"),
		CCollider::Create(m_pDevice, m_pContext, CCollider::TYPE_SPHERE))))
		return E_FAIL;


	lstrcpy(m_szLoadingText, TEXT("AnimTool 로딩 완료 | 'Spacebar' + 'A'"));

	m_isFinished = true;
	return S_OK;
}

CLoader * CLoader::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, LEVELID eNextLevelID)
{
	CLoader*	pInstance = new CLoader(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX("Failed to Created : CLoader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoader::Free()
{
	__super::Free();

	WaitForSingleObject(m_hThread, INFINITE);

	DeleteObject(m_hThread);

	DeleteCriticalSection(&m_Critical_Section);

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);


}
