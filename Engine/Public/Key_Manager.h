#pragma once
#include "Base.h"
BEGIN(Engine)

enum class KEY_STATE
{
	NONE,// 이전에도 눌리지 않았고, 현재도 눌리지 않은.
	DOWN, // 막 누른 시점.
	PRESSING, // 누르고 있는.
	UP, // 막 뗀 시점.
};

enum class MOUSE_KEY { LB, RB, MB, LAST, };
enum class MOUSE_MOVE { X, Y, Z, LAST, };

enum class KEY // 자체적으로 지원 할 키 종류
{
	LEFT,
	RIGHT,
	UP,
	DOWN,
	NUM1,
	NUM2,
	NUM3,
	NUM4,
	NUM5,
	NUM6,
	NUM7,
	NUM8,
	NUM9,
	NUM0,
	NUMPAD8,
	NUMPAD4,
	NUMPAD5,
	NUMPAD6,
	NUMPAD2,
	Q,
	W,
	E,
	R,
	T,
	Y,
	U,
	H,
	J,
	K,
	M,
	I,
	O,
	P,
	A,
	S,
	D,
	F,
	G,
	Z,
	X,
	C,
	V,
	B,
	ALT,
	CTRL,
	LSHIFT,
	SPACE,
	ENTER,
	ESC,
	TAB,
	LAST, // 끝을 알리는.
};

// 키 정보를 담을 구조체
struct tKeyInfo
{
	KEY_STATE   eState; // 키의 상태값
	bool		bPrevPush;	// 이전 프레임에서 눌렸는지 여부.
};

// 키 매니저
class CKey_Manager final : public CBase
{
private:
	CKey_Manager();
	virtual ~CKey_Manager() = default;
public:
	HRESULT Initialize(HINSTANCE _hInst, HWND _hWnd);
	void Update();
public:
	// Get
	const KEY_STATE& GetKeyState(KEY _eKEY) // 키입력 동기화 o 
	{
		return m_vecKey[(int)_eKEY].eState;
	}
	const KEY_STATE& GetMouseKeyState(MOUSE_KEY _eMouse)
	{
		return m_arrMouseKey[(int)_eMouse].eState;
	}
	_long	Get_DIMouseMove(MOUSE_MOVE eMouseMove)
	{
		return *(((_long*)&m_tMouseState) + (int)eMouseMove);
	}

	POINT	Get_MousePos()
	{
		POINT tPos;
		GetCursorPos(&tPos);
		ScreenToClient(m_hWnd, &tPos);

		return tPos;
	}

	
	_bool		Check_Cursor_Position(RECT _tRect);
private:

private:
	vector<tKeyInfo> m_vecKey; // 키 인포 타입을 저장 할 가변배열. // 벡터의 인덱스번호가 해당 키값.
private:
	LPDIRECTINPUT8			m_pInputSDK = nullptr;
	LPDIRECTINPUTDEVICE8	m_pKeyBoard = nullptr;
	LPDIRECTINPUTDEVICE8	m_pMouse = nullptr;
	_byte					m_byKeyState[256];		// 키보드에 있는 모든 키값을 저장하기 위한 변수

	
	tKeyInfo				m_arrMouseKey[(int)MOUSE_KEY::LAST];
	DIMOUSESTATE			m_tMouseState;

	HWND			m_hWnd;
	HINSTANCE		m_hIns;

public:
	static CKey_Manager* Create(HINSTANCE _hInst, HWND _hWnd);
	virtual void		Free();
}; 

END
