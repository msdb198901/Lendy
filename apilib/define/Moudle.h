#ifndef MOUDLE_H
#define MOUDLE_H

#include "Define.h"
#include <string>
#include <wtypes.h>

typedef struct GGUID {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];

	bool operator==(GGUID t)
	{
		return (
			((unsigned long *)&this->Data1)[0] == ((unsigned long *)&t)[0] &&
			((unsigned long *)&this->Data1)[1] == ((unsigned long *)&t)[1] &&
			((unsigned long *)&this->Data1)[2] == ((unsigned long *)&t)[2] &&
			((unsigned long *)&this->Data1)[3] == ((unsigned long *)&t)[3]);
	}

	bool operator!= (GGUID t)
	{
		return !(*this == t);
	}
} GGUID;

static GGUID IID_IUnknownEx = { 0xa5813c2f, 0x6c9d, 0x4c33, { 0x88, 0x7f, 0x7, 0x99, 0xba, 0x1f, 0x9c, 0xd8 } };

struct IUnknownEx
{
	virtual void Release() = 0;
	virtual void *QueryInterface(GGUID uuid) = 0;
};

//内部接口查询
#define QUERY_INTERFACE(interface_, uuid_)		\
if (uuid_==IID_##interface_)					\
{												\
	return static_cast<interface_*>(this);		\
}

#define QUERY_INTERFACE_IUNKNOWNEX(baseinterface_, uuid_)	\
if (uuid_==IID_IUnknownEx)									\
{															\
	return static_cast<IUnknownEx*>(static_cast<baseinterface_*>(this));	\
}

//外部接口查询
#define QUERY_ME_INTERFACE(interface_)					((interface_*)QueryInterface(IID_##interface_))
#define QUERY_OBJECT_INTERFACE(object, interface_)		((interface_*)object.QueryInterface(IID_##interface_))
#define QUERY_OBJECT_PTR_INTERFACE(p, interface_)		((p==nullptr)?nullptr:((interface_*)p->QueryInterface(IID_##interface_)))

//组件模板
template <typename IDLLInterface>
class CMoudleHelper
{
public:
	typedef void* (MoudleCreateProc)(GGUID uuid);

public:
	GGUID			m_uuid;

public:
	std::string		m_strCreateProc;	//创建函数
	std::string		m_strDLLName;		//组件名字

public:
	void*			m_pDLLInstance;
	IDLLInterface*	m_pDLLInterface;

public:
	CMoudleHelper(GGUID uuid) :
		m_uuid(uuid),
		m_pDLLInstance(nullptr),
		m_pDLLInterface(nullptr)
	{
	}

	CMoudleHelper(GGUID uuid, std::string strDLLName, std::string strCreateProc) :
		m_uuid(uuid), 
		m_strDLLName(strDLLName),
		m_strCreateProc(strCreateProc),
		m_pDLLInstance(nullptr), 
		m_pDLLInterface(nullptr)
	{
		
	}

	virtual ~CMoudleHelper() 
	{
		CloseInstance();
	}

public:
	//释放组件
	bool CloseInstance()
	{
		if (m_pDLLInterface)
		{
			m_pDLLInterface->Release();
			m_pDLLInterface = nullptr;
		}

		if (m_pDLLInstance)
		{
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
			FreeLibrary(static_cast<HINSTANCE>(m_pDLLInstance));
#else
			dlclose(m_pDLLInstance);
#endif
			m_pDLLInstance = nullptr;
		}
		return true;
	}

	//创建函数
	bool CreateInstance()
	{
		CloseInstance();

		try
		{
			assert(!m_strDLLName.empty() && !m_strCreateProc.empty());

			MoudleCreateProc* CreateProc = nullptr;

#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS

			decltype(auto) FuncVtoHPoint = [this]()
			{
				return static_cast<HINSTANCE>(m_pDLLInstance);
			};

			m_pDLLInstance = LoadLibrary(m_strDLLName.c_str());
			if (m_pDLLInstance == nullptr)
			{
				int i = GetLastError();
				FreeLibrary(FuncVtoHPoint());
				return false;
			}
			CreateProc = (MoudleCreateProc*)GetProcAddress(FuncVtoHPoint(), m_strCreateProc.c_str());
#else
			m_pDLLInstance = dlopen(file, RTLD_NOW);
			if (m_pDLLInstance == nullptr)
			{
				return false;
			}
			CreateProc = (MoudleCreateProc*)dlsym(m_pDLLInstance, m_strCreateProc.c_str());
#endif 
			m_pDLLInterface = (IDLLInterface*)CreateProc(m_uuid);
			if (m_pDLLInterface == nullptr)
			{
				return false;
			}
		}
		catch (const std::exception&)
		{
			return false;
		}
		return true;
	}

	//创建信息
	void SetMoudleDLLCreate(std::string strDLLName, std::string strCreateProc)
	{
		m_strDLLName = std::move(strDLLName);
		m_strCreateProc = std::move(strCreateProc);
	}

public:
	IDLLInterface* operator->() const
	{
		return m_pDLLInterface;
	}
	IDLLInterface* GetDLLInterface() const
	{
		return m_pDLLInterface;
	}
};

//组件创建函数
#define DECLARE_CREATE_MODULE(ObjectType)					\
extern "C" LENDY_COMMON_API void* Create##ObjectType(GGUID uuid)		\
{																		\
	C##ObjectType *pObjectType = nullptr;								\
	try																	\
	{																	\
		pObjectType = new C##ObjectType();								\
		if (pObjectType == nullptr) throw "创建失败";					\
		void *pObject = pObjectType->QueryInterface(uuid);				\
		if (pObject == nullptr) throw "查询失败";						\
		return pObject;													\
	}																	\
	catch (const std::exception& ){  }									\
	PDELETE(pObjectType);												\
	return nullptr;														\
}	

//组件辅助类宏
#define DECLARE_MOUDLE_DYNAMIC(ObjectType)																	\
class C##ObjectType##Helper : public CMoudleHelper<I##ObjectType>											\
{																											\
public:																										\
	C##ObjectType##Helper() : CMoudleHelper<I##ObjectType>(IID_I##ObjectType) { }							\
};
																																			
#define DECLARE_MOUDLE_HELPER(ObjectType, DllName, CreateFunctionName)	\
class C##ObjectType##Helper : public CMoudleHelper<I##ObjectType>		\
{																		\
public:																	\
	C##ObjectType##Helper() : CMoudleHelper<I##ObjectType>(IID_I##ObjectType, \
		DllName, CreateFunctionName) { }\
};

#endif