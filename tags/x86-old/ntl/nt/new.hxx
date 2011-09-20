/**\file*********************************************************************
 *                                                                     \brief
 *  new and delete operators
 *
 ****************************************************************************
 */

#ifndef NTL__NT_NEW
#define NTL__NT_NEW

#define NTL_NT

#ifndef NTL_NO_NEW 

#ifdef NTL__KM_NEW
#error km/new.hxx already included
#endif

#include "heap.hxx"
#include <new>

///\name  Single-object forms

__forceinline
void * __cdecl
  operator new(std::size_t size) throw(std::bad_alloc)
{
  return ntl::nt::heap::alloc(ntl::nt::process_heap(), size);
}

__forceinline
void __cdecl
  operator delete(void* ptr) __ntl_nothrow
{
  ntl::nt::heap::free(ntl::nt::process_heap(), ptr);
}

__forceinline
void * __cdecl
  operator new(std::size_t size, const std::nothrow_t&) __ntl_nothrow
{
  return ntl::nt::heap::alloc(ntl::nt::process_heap(), size);
}

__forceinline
void __cdecl
  operator delete(void* ptr, const std::nothrow_t&) __ntl_nothrow
{
  ntl::nt::heap::free(ntl::nt::process_heap(), ptr);
}


///\name  Array forms

__forceinline
void * __cdecl
  operator new[](std::size_t size) throw(std::bad_alloc)
{
  return ntl::nt::heap::alloc(ntl::nt::process_heap(), size);
}

__forceinline
void __cdecl
  operator delete[](void* ptr) __ntl_nothrow
{
  ntl::nt::heap::free(ntl::nt::process_heap(), ptr);
}

__forceinline
void * __cdecl
  operator new[](std::size_t size, const std::nothrow_t&) __ntl_nothrow
{
  return ntl::nt::heap::alloc(ntl::nt::process_heap(), size);
}

__forceinline
void __cdecl
  operator delete[](void* ptr, const std::nothrow_t&) __ntl_nothrow
{
  ntl::nt::heap::free(ntl::nt::process_heap(), ptr);
}

#endif//#ifndef NTL_NO_NEW 

#endif//#ifndef NTL__NT_NEW