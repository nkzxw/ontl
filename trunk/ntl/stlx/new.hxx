/**\file*********************************************************************
 *                                                                     \brief
 *  Dynamic memory management [18.4 lib.support.dynamic]
 *
 ****************************************************************************
 */

#ifndef NTL__STLX_NEW
#define NTL__STLX_NEW

#include "cstddef.hxx"
#include "exception.hxx"

namespace std {

/**\addtogroup  lib_language_support *** Language support library [18] ******
 *@{*/

/**\defgroup  lib_support_dynamic ****** Dynamic memory management [18.4] ***
 *@{*/

/**\defgroup  lib_alloc_errors ********* Storage allocation errors [18.4.2] *
 *@{*/

/// Class bad_alloc [18.4.2.1 lib.bad.alloc]
class bad_alloc : public exception
{
  public:
    bad_alloc() __ntl_nothrow {}
    bad_alloc(const bad_alloc&) __ntl_nothrow {}
    bad_alloc& operator=(const bad_alloc&) __ntl_nothrow {}
    virtual ~bad_alloc() __ntl_nothrow {}
    virtual const char* what() const __ntl_nothrow { return "bad_alloc"; }
};

struct  nothrow_t {};
__declspec(selectany)
extern
const nothrow_t nothrow;

/// Type new_handler [18.4.2.2 lib.new.handler]
typedef void (*new_handler)();

/// set_new_handler [18.4.2.3 lib.set.new.handler]
new_handler set_new_handler(new_handler new_p) __ntl_nothrow;

/**@} lib_alloc_errors */
/**@} lib_support_dynamic */
/**@} lib_language_support */

}//namespace std

#ifdef _MSC_VER
#define operator  __cdecl operator
#else
//#define operator  operator
#endif

/**\addtogroup  lib_language_support
 *@{*/

/**\addtogroup  lib_support_dynamic
 *@{*/

// Storage allocation and deallocation [18.4.1 lib.new.delete]

///\name  Single-object forms [18.4.1.1 lib.new.delete.single]

void* operator new      (std::size_t size) __ntl_throws(std::bad_alloc);
void  operator delete   (void* ptr) __ntl_nothrow;

void* operator new      (std::size_t size, const std::nothrow_t&) __ntl_nothrow;
void  operator delete   (void* ptr, const std::nothrow_t&) __ntl_nothrow;

///\name  Array forms [18.4.1.2 lib.new.delete.array]

void* operator new[]    (std::size_t size) __ntl_throws(std::bad_alloc);
void  operator delete[] (void* ptr) __ntl_nothrow;

void* operator new[]    (std::size_t size, const std::nothrow_t&) __ntl_nothrow;
void  operator delete[] (void* ptr, const std::nothrow_t&) __ntl_nothrow;

///\name  Placement forms [18.4.1.3 lib.new.delete.placement]
///\note  Standard says nothing about inline nor static, but we're avoiding LNK2005

__forceinline
void* operator new      (std::size_t, void* ptr) __ntl_nothrow  { return ptr; }

__forceinline
void  operator delete   (void*, void*) __ntl_nothrow            {}

__forceinline
void* operator new[]    (std::size_t, void* ptr) __ntl_nothrow  { return ptr; }

__forceinline
void  operator delete[] (void*, void*) __ntl_nothrow            {}

#undef operator

///@}
/**@} lib_support_dynamic */
/**@} lib_language_support */

#endif//#ifndef NTL__STLX_NEW