/**\file*********************************************************************
 *                                                                     \brief
 *  NTL compiler configuration
 *
 ****************************************************************************
 */
#ifndef NTL__BASECONF
#define NTL__BASECONF
#pragma once


/************************************************************************/
/* Supported compilers                                                  */
/************************************************************************/

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
// MS VC++
  #if _MSC_VER < 1400
  # error upgrade your compiler at least to VC8 (14.00)
  #endif
#elif defined(__BCPLUSPLUS__)
// Borland C++
  #if __BCPLUSPLUS__ < 0x600
  # error upgrade your compiler at least to BCB 2009 (12.00)
  #endif
#elif defined(__INTEL_COMPILER)
// Intel C++
  #ifndef __ICL
  # define __ICL __INTEL_COMPILER
  #endif
#elif defined(__GNUC__)
// GCC
  #if !(__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 2)
  # error upgrade GCC at least to the GCC 4.2
  #endif
#elif defined(__clang__)
// Clang
  #if !(__clang_major__ >= 3 || __clang_major__ == 2 && __clang_minor__ >= 9)
  # error upgrade Clang at least to the Clang 2.9
  #endif

#else
# pragma message("Unknown compiler, it is unsupported probably. Sorry, mate")
#endif

#if defined(__GNUC__)
#ifdef __x86_64__
# define _M_X64
#else
# define _M_IX86
# endif
#endif



//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* Debugging support                                                    */
/************************************************************************/
#ifndef NTL_DEBUG
  #if defined(NDEBUG)
  #elif defined(_DEBUG) || defined(DBG)
  # define NTL_DEBUG
  #endif
#endif

/************************************************************************/
/* Exceptions configuration                                             */
/************************************************************************/
#ifndef STLX_USE_EXCEPTIONS
  #if defined(_MSC_VER) || defined(__BCPLUSPLUS__)
    #ifdef _CPPUNWIND
      #define STLX_USE_EXCEPTIONS 1
    #else
      #define STLX_USE_EXCEPTIONS 0
    #endif
  #elif defined(__GNUC__) || defined(__clang__)
    #ifdef __EXCEPTIONS
      #define STLX_USE_EXCEPTIONS 1
    #else
      #define STLX_USE_EXCEPTIONS 0
    #endif
  #else
    #error define STLX_USE_EXCEPTIONS
  #endif
#endif



/************************************************************************/
/* RTTI configuration                                                   */
/************************************************************************/
#ifndef STLX_USE_RTTI
  #if defined(_MSC_VER)
    #ifdef _CPPRTTI
      #define STLX_USE_RTTI 1
    #else
      #define STLX_USE_RTTI 0
    #endif
  #elif defined(__GNUC__) || defined(__clang__)
    #ifdef __GXX_RTTI
      #define STLX_USE_RTTI 1
    #else
      #define STLX_USE_RTTI 0
    #endif
  #else
    #error define STLX_USE_RTTI
  #endif
#endif


#if defined(_MSC_VER)
# pragma warning(disable:4514)// unreferenced inline function has been removed
# pragma warning(disable:4480)// nonstandard extension used: specifying underlying type for enum 'enum'
# ifdef __ICL
#  pragma warning(disable:271)  // [remark] trailing comma is nonstandard
#  pragma warning(disable:304)  // [remark] access control not specified ("access" by default)
#  pragma warning(disable:874)  // [remark] support for placement delete is disabled
#  pragma warning(disable:981)  // [remark] operands are evaluated in unspecified order (http://software.intel.com/en-us/articles/cdiag981/)
#  pragma warning(disable:1599) // [remark] declaration hides variable "var"
#  pragma warning(disable:1879) // [warning] unimplemented pragma ignored (but supported)
#  pragma warning(disable:2268) // [remark] the declaration of the copy assignment operator for "type1s" has been suppressed because that of "type2" was suppressed
#  pragma warning(disable:2270) // [remark] the declaration of the copy assignment operator for "type1" has been suppressed because that of "type2" is inaccessible
# else
# pragma runtime_checks( "", off )
# endif
#elif defined(__GNUC__) || defined(__clang__)
# define __forceinline __attribute__((always_inline))
# define __assume(X)
#else
# define __forceinline inline
# define __assume(X)
#endif

///\name SAL macroses
#ifndef NTL_NO_SAL
#ifndef __in
# define __in
#endif
#ifndef __inout
# define __in_opt
#endif
#ifndef __inout
# define __inout
#endif
#ifndef __inout_opt
# define __inout_opt
#endif
#ifndef __out
# define __out
#endif
#ifndef __out_opt
# define __out_opt
#endif
#ifndef __optional
# define __optional
#endif
#ifndef __reserved
# define __reserved
#endif
#ifndef __deref_out
# define __deref_out
#endif
#ifndef __deref_out_opt
# define __deref_out_opt
#endif
#endif // NTL_NO_SAL
///\}

#if !(defined(_MSC_VER) && _MSC_VER >= 1400)
# define __restrict
# define __thiscall
#endif

#if defined(_MSC_VER)
# define __noalias __declspec(noalias)
#elif defined(__BCPLUSPLUS__) || defined(__GNUC__) || defined(__clang__)
# define __noalias
#endif

#define _PasteToken(x,y) x##y
#define _Join(x,y) _PasteToken(x,y)

#ifndef STATIC_ASSERT
  #if defined(__GNUC__)
    #if (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 3)
      // !fr3@K!
      // GCC specific note: __COUNTER__ is not supported prior to 4.3
      #define STATIC_ASSERT(e) typedef char _Join(_STATIC_ASSERT_, __COUNTER__) [(e)?1:-1]
    #else
      #define STATIC_ASSERT(e) typedef char _Join(_STATIC_ASSERT_, __LINE__) [(e)?1:-1]
    #endif
  #else
    #define STATIC_ASSERT(e) typedef char _Join(_STATIC_ASSERT_, __COUNTER__) [(e)?1:-1]
  #endif
#endif

#if defined(__BCPLUSPLUS__) || defined(__GNUC__)
# define __w64
#endif

#ifndef __GNUC__
  #ifdef _M_X64
  # define __SIZEOF_POINTER__ 8
  #else
  # define __SIZEOF_POINTER__ 4
  #endif // x64
#endif // __GNUC__

#if defined(__GNUC__)  || defined(__clang__)
#ifndef _M_X64
 #define __cdecl    __attribute__((cdecl))
 #define __stdcall  __attribute__((stdcall))
 #define __fastcall __attribute__((fastcall))
#else
  #define __cdecl    
  #define __stdcall  
  #define __fastcall 
#endif // x64
#endif // __GNUC__

#ifndef NTL_CRTCALL
# ifdef NTL_STLX_FORCE_CDECL
  #ifdef _MSC_VER
    #define NTL_CRTCALL __cdecl
  #elif defined(__BCPLUSPLUS__)
    #define NTL_CRTCALL __cdecl
  #endif
# else
  #define NTL_CRTCALL
# endif
#endif

#define NTL_CRTIMP __cdecl

#ifndef NTL_EXTERNAPI
  #ifdef  _MSC_VER
    #define NTL_EXTERNAPI extern "C" __declspec(dllimport) __declspec(nothrow)
    #define NTL_EXTERNVAR extern "C" __declspec(dllimport)
  #elif defined(__BCPLUSPLUS__)
    #define NTL_EXTERNAPI extern "C" __declspec(dllimport) __declspec(nothrow)
    #define NTL_EXTERNVAR extern "C" __declspec(dllimport)
  #elif defined(__GNUC__) || defined(__clang__)
    #define NTL_EXTERNAPI extern "C"
    #define NTL_EXTERNVAR extern "C"
  #endif
#endif

#ifndef NTL_EXPORTAPI
#if defined(_MSC_VER) && defined(_WINDLL) && (STLX_USE_EXCEPTIONS > 0)
# define NTL_EXPORTAPI __declspec(dllexport)
#else
# define NTL_EXPORTAPI 
#endif
#endif // NTL_EXPORTAPI


#endif // NTL__BASECONF
