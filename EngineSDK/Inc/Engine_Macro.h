#ifndef Engine_Macro_h__
#define Engine_Macro_h__

namespace Engine
{
#ifndef			MSG_BOX
#define			MSG_BOX(_message)			MessageBox(NULL, TEXT(_message), L"System Message", MB_OK)
#endif

#define			BEGIN(NAMESPACE)		namespace NAMESPACE {
#define			END						}

#define			USING(NAMESPACE)	using namespace NAMESPACE;

#ifdef	ENGINE_EXPORTS
#define ENGINE_DLL		_declspec(dllexport)
#else
#define ENGINE_DLL		_declspec(dllimport)
#endif	

#define	CONVERTTOSTRING(STRNAME)		m_pGameInstance->ConvertToString(STRNAME)
#define	CONVERTTOWSTRING(STRNAME)		m_pGameInstance->ConvertToWString(STRNAME)

#define NO_COPY(CLASSNAME)											\
			private:												\
			CLASSNAME(const CLASSNAME&) = delete;					\
			CLASSNAME& operator = (const CLASSNAME&) = delete;		

#define DECLARE_SINGLETON(CLASSNAME)								\
			NO_COPY(CLASSNAME)										\
			private:												\
			static CLASSNAME*	m_pInstance;						\
			public:													\
			static CLASSNAME*	GetInstance( void );				\
			static unsigned int DestroyInstance( void );			

#define IMPLEMENT_SINGLETON(CLASSNAME)								\
			CLASSNAME*	CLASSNAME::m_pInstance = nullptr;			\
			CLASSNAME*	CLASSNAME::GetInstance( void )	{			\
			if(nullptr == m_pInstance) {							\
				m_pInstance = new CLASSNAME;						\
				}													\
				return m_pInstance;									\
			}														\
			unsigned int CLASSNAME::DestroyInstance( void ) {		\
				unsigned int iRefCnt = {0};							\
				if (nullptr != m_pInstance) {						\
				iRefCnt = m_pInstance->Release();					\
				if(0 == iRefCnt)									\
					m_pInstance = nullptr;							\
				}													\
				return iRefCnt;										\
			}
}

#endif // Engine_Macro_h__


// KeyManager
#define KEY_CHECK(key, state)   m_pGameInstance->GetKeyState(key) == state
	// 실제 사용은 아래 4개
#define KEY_PRESSING(key)       KEY_CHECK(key, KEY_STATE::PRESSING)
#define KEY_DOWN(key)           KEY_CHECK(key, KEY_STATE::DOWN)
#define KEY_UP(key)				KEY_CHECK(key, KEY_STATE::UP)
#define KEY_NONE(key)           KEY_CHECK(key, KEY_STATE::NONE)

#define MOUSE_CHECK(key, state)	m_pGameInstance->GetMouseKeyState(key) == state
	// 실제 사용은 아래 4개
#define MOUSE_PRESSING(key)		MOUSE_CHECK(key, KEY_STATE::PRESSING)
#define MOUSE_DOWN(key)			MOUSE_CHECK(key, KEY_STATE::DOWN)
#define MOUSE_UP(key)			MOUSE_CHECK(key, KEY_STATE::UP)
#define MOUSE_NONE(key)			MOUSE_CHECK(key, KEY_STATE::NONE)
	// 마우스 움직임.
#define MOUSE_MOVE(key)			m_pGameInstance->Get_DIMouseMove(key)
