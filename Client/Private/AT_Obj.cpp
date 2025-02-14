#include "stdafx.h"
#include "AT_Obj.h"
#include "GameInstance.h"
#include "AT_PartAnimObj.h"


#include <iostream>
using namespace std;


CAT_Obj::CAT_Obj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:
	CGameObject(pDevice, pContext)
{

}

CAT_Obj::CAT_Obj(const CAT_Obj& Prototype)
	:
	CGameObject(Prototype)
{

}

HRESULT CAT_Obj::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAT_Obj::Initialize(void* pArg)
{
	if (pArg)
	{
		AT_OBJ_DESC* pDesc = static_cast<AT_OBJ_DESC*>(pArg);
		m_strModelProtoName = pDesc->strModelProtoName;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_PartObjects()))
		return E_FAIL;

	m_pTransformCom->Scaling(1.f, 1.f, 1.f);
	return S_OK;
}

void CAT_Obj::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

// 예외 처리 함수
float CleanFloat(_float fVal) 
{
	const static _float fEpsilon = 1e-6f; // 임계값
	return (fabs(fVal) < fEpsilon) ? 0.0f : fVal;
}

void CalculateQuaternionAndRotation(_float3 vRight, _float3 vUp, _float3 vLook)
{
	_matrix matRotation = XMMatrixIdentity();
	matRotation.r[0] = XMLoadFloat3(&vRight);
	matRotation.r[1] = XMLoadFloat3(&vUp);
	matRotation.r[2] = XMLoadFloat3(&vLook);

	// 쿼터니언 계산
	_vector vQuaternion = XMQuaternionRotationMatrix(matRotation);

	// 쿼터니언 출력
	_float4 q;
	XMStoreFloat4(&q, vQuaternion);
	std::cout << "Quaternion: (" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")\n";

	// Euler 각도로 변환
	XMFLOAT3 euler;
	euler.x = atan2f(2.0f * (q.w * q.x + q.y * q.z), 1.0f - 2.0f * (q.x * q.x + q.y * q.y)); // Pitch
	euler.y = asinf(2.0f * (q.w * q.y - q.z * q.x));                                        // Yaw
	euler.z = atan2f(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.y * q.y + q.z * q.z)); // Roll
	std::cout << "Euler Angles: Pitch: " << euler.x << ", Yaw: " << euler.y << ", Roll: " << euler.z << "\n";

}


void CAT_Obj::Update(_float fTimeDelta)
{
    // 루트 본의 애니메이션 행렬 계산을 보장하기 위해 먼저 실행
    __super::Update(fTimeDelta);

    if (m_bFollowRootBone)
    {
        CAT_PartAnimObj* pPartObj = static_cast<CAT_PartAnimObj*>(m_vecChildren[0]);
        if (!pPartObj) return;

        _matrix FixMatrix = XMMatrixIdentity();
        FixMatrix = XMMatrixScaling(-1.0f, 1.0f, -1.0f); // Right와 Look 축 반전

        // 1. 이전 루트 본 변환 행렬 저장 및 초기화 처리
        static _matrix MatPrevRootBoneTransfomationMatrix = XMMatrixIdentity();
        static _vector vPlayerRotationQuaternion = XMQuaternionIdentity(); // 플레이어 각 축별 누적 회전양
        static _bool bIsFirstFrame = true; // 첫 프레임인지 확인
        static _vector vPrevPos = XMVectorZero(); // 이전 이동량

        // 250122
        // 첫 프레임일 경우 초기화
        if (bIsFirstFrame)
        {
            cout << "애니매이션 첫 프레임 " << endl;
            cout << endl;
            MatPrevRootBoneTransfomationMatrix = pPartObj->Get_RootBoneCombinedTransfomationMatrix();
            MatPrevRootBoneTransfomationMatrix = MatPrevRootBoneTransfomationMatrix * FixMatrix;
            bIsFirstFrame = false;
        }

        // 루트 본의 변환 행렬 계산
        _matrix MatRootBoneTransfomationMatrix = pPartObj->Get_RootBoneCombinedTransfomationMatrix();
        MatRootBoneTransfomationMatrix = MatRootBoneTransfomationMatrix * FixMatrix;


        // 각 축 및 위치 정보
        _vector vRight = MatRootBoneTransfomationMatrix.r[0];
        _vector vUp = MatRootBoneTransfomationMatrix.r[1];
        _vector vLook = MatRootBoneTransfomationMatrix.r[2];
        _vector vPos = MatRootBoneTransfomationMatrix.r[3];

        {
            vRight.m128_f32[0] = CleanFloat(vRight.m128_f32[0]);
            vRight.m128_f32[1] = CleanFloat(vRight.m128_f32[1]);
            vRight.m128_f32[2] = CleanFloat(vRight.m128_f32[2]);

            vUp.m128_f32[0] = CleanFloat(vUp.m128_f32[0]);
            vUp.m128_f32[1] = CleanFloat(vUp.m128_f32[1]);
            vUp.m128_f32[2] = CleanFloat(vUp.m128_f32[2]);

            vLook.m128_f32[0] = CleanFloat(vLook.m128_f32[0]);
            vLook.m128_f32[1] = CleanFloat(vLook.m128_f32[1]);
            vLook.m128_f32[2] = CleanFloat(vLook.m128_f32[2]);
        }

        cout << "Right : " << vRight.m128_f32[0] << ' ' << vRight.m128_f32[1] << ' ' << vRight.m128_f32[2] << endl;
        cout << "Up : " << vUp.m128_f32[0] << ' ' << vUp.m128_f32[1] << ' ' << vUp.m128_f32[2] << endl;
        cout << "Look : " << vLook.m128_f32[0] << ' ' << vLook.m128_f32[1] << ' ' << vLook.m128_f32[2] << endl;
        cout << endl;

        // Get_Transform()->Set_State(CTransform::STATE_RIGHT, vRight);
        // Get_Transform()->Set_State(CTransform::STATE_UP, vUp);
        // Get_Transform()->Set_State(CTransform::STATE_LOOK, vLook);

        // 현재 루트 본 행렬과 이전 행렬로부터 쿼터니언을 계산
        _vector vCurQuaternion = XMQuaternionRotationMatrix(MatRootBoneTransfomationMatrix);
        _vector vPrevQuaternion = XMQuaternionRotationMatrix(MatPrevRootBoneTransfomationMatrix);

        // 회전값 차이 계산
        _vector vDeltaRotation = XMQuaternionMultiply(vCurQuaternion, XMQuaternionInverse(vPrevQuaternion));

        // 기존 누적 쿼터니언에 변화량을 곱하여 누적
        vPlayerRotationQuaternion = XMQuaternionMultiply(vPlayerRotationQuaternion, vDeltaRotation);
        vPlayerRotationQuaternion = XMQuaternionNormalize(vPlayerRotationQuaternion);

        // 누적 회전을 쿼터니언으로 변환
        _matrix MatPlayerRotation = XMMatrixRotationQuaternion(vPlayerRotationQuaternion);

        // 회전값 적용
        vRight = MatPlayerRotation.r[0];
        vUp = MatPlayerRotation.r[1];
        vLook = MatPlayerRotation.r[2];

        // 6. Transform 업데이트
        //Get_Transform()->Set_State(CTransform::STATE_RIGHT, vRight);
        //Get_Transform()->Set_State(CTransform::STATE_UP, vUp);
        //Get_Transform()->Set_State(CTransform::STATE_LOOK, vLook);

        // 8. 이전 루트 본 변환 행렬 업데이트
        MatPrevRootBoneTransfomationMatrix = MatRootBoneTransfomationMatrix;

        // 애니메이션 초기화
        if (pPartObj->Get_Anim_CurTrackPos() == 0.f)
        {
            vPrevPos = vPos;
            bIsFirstFrame = true;
        }

        // 이동 벡터 계산
        _vector vMoveDelta = vPos - vPrevPos;
        _float fMoveAmount = XMVectorGetX(XMVector3Length(vMoveDelta));
        vPrevPos = vPos;

        // 4. 이동 방향을 회전된 벡터를 기준으로 이동
        _vector vCurPos = Get_Transform()->Get_State(CTransform::STATE_POSITION);

        // 현재 Transform의 위치를 기준으로 이동량을 직접 더함
        _vector vPlayerLook = XMVector3Normalize(Get_Transform()->Get_State(CTransform::STATE_LOOK));
        vCurPos += vPlayerLook * fMoveAmount; // Look 방향으로 이동량 추가

        // 최종 위치 업데이트
        Get_Transform()->Set_State(CTransform::STATE_POSITION, vCurPos);
    }
}


void CAT_Obj::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CAT_Obj::Render()
{
	return S_OK;
}

void CAT_Obj::Add_PartAnimObject(CAT_PartAnimObj* pPartAnimObject)
{
	if (pPartAnimObject)
		Add_Child(pPartAnimObject);
}

HRESULT CAT_Obj::Ready_Components()
{
	return S_OK;
}

HRESULT CAT_Obj::Ready_PartObjects()
{
	CAT_PartAnimObj::AT_PARTANIMOBJ_DESC Desc{};

	Desc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	Desc.strModelProtoName = m_strModelProtoName;

	Add_Child(static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, LEVEL_ANIMTOOL, TEXT("Prototype_GameObject_AT_PartAnimObj"), &Desc)));

	if (nullptr == m_vecChildren.back())
		return E_FAIL;

	return S_OK;
}

CAT_Obj* CAT_Obj::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAT_Obj* pInstance = new CAT_Obj(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAT_Obj");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAT_Obj::Clone(void* pArg)
{
	CAT_Obj* pInstance = new CAT_Obj(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAT_Obj");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAT_Obj::Free()
{
	__super::Free();
}
