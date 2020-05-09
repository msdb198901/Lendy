#ifndef MACRO_H
#define MACRO_H

#define LENDY_PLATFORM_WINDOWS 0
#define LENDY_PLATFORM_UNIX    1
#define LENDY_PLATFORM_APPLE   2
#define LENDY_PLATFORM_INTEL   3

// must be first (win 64 also define _WIN32)
#if defined( _WIN64 )
#  define LENDY_PLATFORM LENDY_PLATFORM_WINDOWS
#elif defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define LENDY_PLATFORM LENDY_PLATFORM_WINDOWS
#elif defined( __APPLE_CC__ )
#  define LENDY_PLATFORM LENDY_PLATFORM_APPLE
#elif defined( __INTEL_COMPILER )
#  define LENDY_PLATFORM LENDY_PLATFORM_INTEL
#else
#  define LENDY_PLATFORM LENDY_PLATFORM_UNIX
#endif

#define LENDY_COMPILER_MICROSOFT 0
#define LENDY_COMPILER_GNU       1
#define LENDY_COMPILER_BORLAND   2
#define LENDY_COMPILER_INTEL     3

#ifdef _MSC_VER
#  define LENDY_COMPILER LENDY_COMPILER_MICROSOFT
#elif defined( __BORLANDC__ )
#  define LENDY_COMPILER LENDY_COMPILER_BORLAND
#elif defined( __INTEL_COMPILER )
#  define LENDY_COMPILER LENDY_COMPILER_INTEL
#elif defined( __GNUC__ )
#  define LENDY_COMPILER LENDY_COMPILER_GNU
#  define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#  error "FATAL ERROR: Unknown compiler."
#endif

#ifdef LENDY_API_USE_DYNAMIC_LIBS
#  if LENDY_COMPILER == LENDY_COMPILER_MICROSOFT
#    define LENDY_API_EXPORT __declspec(dllexport)
#    define LENDY_API_IMPORT __declspec(dllimport)
#  elif LENDY_COMPILER == LENDY_COMPILER_GNU
#    define LENDY_API_EXPORT __attribute__((visibility("default")))
#    define LENDY_API_IMPORT
#  else
#    error compiler not supported!
#  endif
#else
#  define LENDY_API_EXPORT
#  define LENDY_API_IMPORT
#endif

#ifdef LENDY_API_EXPORT_COMMON
#  define LENDY_COMMON_API LENDY_API_EXPORT
#else
#  define LENDY_COMMON_API LENDY_API_IMPORT
#endif

#ifdef LENDY_API_EXPORT_DATABASE
#  define LENDY_DATABASE_API LENDY_API_EXPORT
#else
#  define LENDY_DATABASE_API LENDY_API_IMPORT
#endif

#ifdef LENDY_API_EXPORT_SHARED
#  define LENDY_SHARED_API LENDY_API_EXPORT
#else
#  define LENDY_SHARED_API LENDY_API_IMPORT
#endif

#ifdef LENDY_API_EXPORT_GAME
#  define LENDY_GAME_API LENDY_API_EXPORT
#else
#  define LENDY_GAME_API LENDY_API_IMPORT
#endif


#define ISNULL(a) (a)==nullptr 
#define ISEMPTY(a) (a)=='\0'
#define PDELETE(a) if((a)){ delete (a); (a) = nullptr; }
#define ADELETE(a) if((a)){ delete[] (a); (a) = nullptr; }
#define ADELETE(a) if((a)){ delete[] (a); (a) = nullptr; }

#define ARR_LEN(a) (sizeof((a))/sizeof((a[0])))

#if LENDY_COMPILER == LENDY_COMPILER_MICROSOFT
#define EXPORT_BEGIN __pragma(warning(push)) __pragma(warning(disable:4251))
#define EXPORT_END __pragma(warning(pop))
#else
#define EXPORT_BEGIN
#define EXPORT_END
#endif

//½ûÖ¹¿½±´¸´ÖÆ
#define DELETE_COPY_ASSIGN(ClassName)						\
public:												\
ClassName(const ClassName&) = delete;				\
ClassName& operator=(const ClassName&) = delete;

#define LENDY_SYNTHESIZE(varType, varName, ...) \
varType m_##varName __VA_ARGS__; \
public : \
inline varType& get_##varName(void) {return m_##varName;}\
inline void set_##varName(varType var) { m_##varName = var;}

#define LENDY_SYNTHESIZE_READONLY(varType, varName, ...) \
varType m_##varName __VA_ARGS__; \
public : \
inline varType& get_##varName(void) {return m_##varName;}\

#define LENDY_SYNTHESIZE_REG(varType, varName, ...) \
varType m_##varName __VA_ARGS__; \
public : \
inline varType& get_##varName(void) {return m_##varName;}\
inline void register_##varName(varType var) { m_##varName = var;}

//linuxºê
#if LENDY_PLATFORM == LENDY_PLATFORM_UNIX

#define __max(a,b) (((a) > (b)) ? (a) : (b))
#define __min(a,b) (((a) < (b)) ? (a) : (b))

#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==nullptr

#define LOWORD(l)           ((uint16)(((uint64)(l)) & 0xffff))

#ifndef SOCKET
#	define SOCKET int
#endif

#ifndef SOCKET_ERROR
#   define SOCKET_ERROR -1
#endif

#ifndef INVALID_SOCKET
#   define INVALID_SOCKET (SOCKET)(~0)
#endif

#ifndef TRUE
#	define TRUE 1
#endif

#ifndef FALSE
#	define FALSE 0
#endif

#endif

//Õë¶Ôlinux
#if (__cplusplus >= 201402L) || (defined(_MSC_VER) && _MSC_VER >= 1800)
#define LENDY_COMPILER_14
#endif

#define SZFMTD "%" PRIuPTR

//Ç¿ÖÆ¹Ø±Õ
#define LENDY_SOCKET_FORCE_CLOSE	

#endif