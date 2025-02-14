#include "Key_Manager.h"

int g_ArrDK[(int)KEY::LAST] = {
    DIK_LEFT,
    DIK_RIGHT,
    DIK_UP,
    DIK_DOWN,
    DIK_1,
    DIK_2,
    DIK_3,
    DIK_4,
    DIK_5,
    DIK_6,
    DIK_7,
    DIK_8,
    DIK_9,
    DIK_0,
    DIK_NUMPAD8,
    DIK_NUMPAD4,
    DIK_NUMPAD5,
    DIK_NUMPAD6,
    DIK_NUMPAD2,
    DIK_Q,
    DIK_W,
    DIK_E,
    DIK_R,
    DIK_T,
    DIK_Y,
    DIK_U, // 
    DIK_H, // 추가
    DIK_J, // 추가
    DIK_K, // 추가
    DIK_M, // 추가
    DIK_I,
    DIK_O,
    DIK_P,
    DIK_A,
    DIK_S,
    DIK_D,
    DIK_F,
    DIK_G,
    DIK_Z,
    DIK_X,
    DIK_C,
    DIK_V,
    DIK_B,
    DIK_LALT,     // ALT,
    DIK_LCONTROL, // CTRL,
    DIK_LSHIFT,   // LSHIFT,
    DIK_SPACE,    // SPACE,
    DIK_RETURN,   // ENTER,
    DIK_ESCAPE,   // ESC,
    DIK_TAB       // TAB,


    // LAST // 끝을 알리는.
};

CKey_Manager::CKey_Manager() 
    : m_vecKey{} 
    , m_pInputSDK(nullptr)
    , m_pKeyBoard(nullptr)
    , m_pMouse(nullptr)
    , m_byKeyState{}
    , m_arrMouseKey{}
    , m_tMouseState{}
{
}

HRESULT CKey_Manager::Initialize(HINSTANCE hInst, HWND hWnd)
{

    m_hWnd = hWnd;
    m_hIns = hInst;
    // DInput 컴객체를 생성하는 함수
    if (FAILED(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pInputSDK, NULL)))
        return E_FAIL;

    // 키보드 객체 생성
    if (FAILED(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyBoard, nullptr)))
        return E_FAIL;

    // 생성된 키보드 객체의 대한 정보를 컴 객체에게 전달하는 함수
    m_pKeyBoard->SetDataFormat(&c_dfDIKeyboard);

    // 장치에 대한 독점권을 설정해주는 함수, (클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수)
    m_pKeyBoard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

    // 장치에 대한 access 버전을 받아오는 함수
    m_pKeyBoard->Acquire();

    // 마우스 객체 생성
    if (FAILED(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
        return E_FAIL;


    // 생성된 마우스 객체의 대한 정보를 컴 객체에게 전달하는 함수
    m_pMouse->SetDataFormat(&c_dfDIMouse);

    // 장치에 대한 독점권을 설정해주는 함수, 클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수
    m_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

    // 장치에 대한 access 버전을 받아오는 함수
    m_pMouse->Acquire();

    // 키의 상태값을 저장하는 배열 초기화
    for (int i = 0; i < (int)KEY::LAST; ++i)
    {
        m_vecKey.push_back(tKeyInfo{ KEY_STATE::NONE, false });
    }

    for (int i = 0; i < (int)MOUSE_KEY::LAST; ++i)
    {
        m_arrMouseKey[i] = { tKeyInfo{ KEY_STATE::NONE, false } };
    }
    return S_OK;
}

void CKey_Manager::Update()
{
    // Direct Input StateCheck
    m_pKeyBoard->GetDeviceState(256, m_byKeyState);
    m_pMouse->GetDeviceState(sizeof(m_tMouseState), &m_tMouseState);

    HWND hWnd = GetFocus(); // 현재 포커싱 중인 윈도우핸들값을 알려준다 >> 포커싱 된 윈도우가 없으면, 0이 나온다 id(0)

    if (nullptr != hWnd) // 윈도우가 포커싱 중일 때
    {
        for (int i = 0; i < (int)KEY::LAST; ++i)
        {
            // 현재 키가 눌려있다.
            if (m_byKeyState[g_ArrDK[i]] & 0x80)
            {
                if (m_vecKey[i].bPrevPush)
                {
                    // 이전에도 눌려있었다.
                    m_vecKey[i].eState = KEY_STATE::PRESSING;
                }
                else
                {
                    // 이전에는 눌려있지 않았다.
                    m_vecKey[i].eState = KEY_STATE::DOWN;
                }
                // 프리브푸쉬를 트루로 바꿔줘야지. 다음 프레임때 상태를 체크 가능.
                m_vecKey[i].bPrevPush = true;
            }
            // 키가 안눌려있다.
            else
            {
                if (m_vecKey[i].bPrevPush)
                {
                    // 이전에 눌려있었다.
                    m_vecKey[i].eState = KEY_STATE::UP;
                }
                else
                {
                    // 이전에도 안눌려있었다.
                    m_vecKey[i].eState = KEY_STATE::NONE;
                }
                m_vecKey[i].bPrevPush = false;
            }
        }

        // Mouse
        for (int i = 0; i < (int)MOUSE_KEY::LAST; ++i)
        {
            if (m_tMouseState.rgbButtons[i]) // 마우스가 현재 눌려있다.(클릭체크)
            {
                if (m_arrMouseKey[i].bPrevPush) // 이전에도 눌려있었다.
                {
                    m_arrMouseKey[i].eState = KEY_STATE::PRESSING;
                }
                else // 이전에는 눌려있지 않았다.
                {
                    m_arrMouseKey[i].eState = KEY_STATE::DOWN;
                }
                m_arrMouseKey[i].bPrevPush = true;
            }
            else // 마우스가 현재 눌려있지 않다.
            {
                if (m_arrMouseKey[i].bPrevPush) // 이전에는 눌려있었다.
                {
                    m_arrMouseKey[i].eState = KEY_STATE::UP;
                }
                else // 이전에도 눌려있지 않았다.
                {
                    m_arrMouseKey[i].eState = KEY_STATE::NONE;
                }
                m_arrMouseKey[i].bPrevPush = false;
            }
        }


    }
    else // 윈도우 포커싱 해제상태
    {
        // 키보드
        for (int i = 0; i < (int)KEY::LAST; ++i)
        {
            m_vecKey[i].bPrevPush = false;

            if (m_vecKey[i].eState == KEY_STATE::DOWN || m_vecKey[i].eState == KEY_STATE::PRESSING)
            {
                m_vecKey[i].eState = KEY_STATE::UP;
            }
            else if (m_vecKey[i].eState == KEY_STATE::UP)
            {
                m_vecKey[i].eState = KEY_STATE::NONE;
            }
        }

        // 마우스 
        for (int i = 0; i < (int)MOUSE_KEY::LAST; ++i)
        {
            m_arrMouseKey[i].bPrevPush = false;

            if (m_arrMouseKey[i].eState == KEY_STATE::DOWN || m_arrMouseKey[i].eState == KEY_STATE::PRESSING)
            {
                m_arrMouseKey[i].eState = KEY_STATE::UP;
            }
            else if (m_arrMouseKey[i].eState == KEY_STATE::UP)
            {
                m_arrMouseKey[i].eState = KEY_STATE::NONE;
            }
        }
    }
}

_bool CKey_Manager::Check_Cursor_Position(RECT _tRect)
{
    POINT tPos = Get_MousePos();

    tPos.x -= 640.f;
    tPos.y -= 360.f;
    tPos.y *= -1;

    if (tPos.x >= _tRect.left && tPos.x <= _tRect.right &&
        tPos.y >= _tRect.bottom && tPos.y <= _tRect.top)
    {
        return true;
    }

    return false;
}

CKey_Manager* CKey_Manager::Create(HINSTANCE _hInst, HWND _hWnd)
{
    CKey_Manager* pInstance = new CKey_Manager();

    if (FAILED(pInstance->Initialize(_hInst, _hWnd)))
    {
        MSG_BOX("Failed to Created : CKey_Manager");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKey_Manager::Free()
{
    Safe_Release(m_pKeyBoard);
    Safe_Release(m_pMouse);
    Safe_Release(m_pInputSDK);

    __super::Free();
}
