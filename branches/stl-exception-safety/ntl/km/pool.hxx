/**\file*********************************************************************
 *                                                                     \brief
 *  Kernel Mode pools
 *
 ****************************************************************************
 */

#ifndef NTL__KM_POOL
#define NTL__KM_POOL

#include "../cstddef"

#ifndef NTL__POOL_TAG
#define NTL__POOL_TAG 'LTN_'  // _NTL
#endif

#pragma comment(linker, "/subsystem:native,5")
#pragma comment(lib,    "ntoskrnl.lib")

namespace ntl {
namespace km {


enum pool_type
{
  NonPagedPool,
  PagedPool,
  NonPagedPoolMustSucceed,
  DontUseThisType,
  NonPagedPoolCacheAligned,
  PagedPoolCacheAligned,
  NonPagedPoolCacheAlignedMustS,
  MaxPoolType,
  NonPagedPoolSession = 32,
  PagedPoolSession,
  NonPagedPoolMustSucceedSession,
  DontUseThisTypeSession,
  NonPagedPoolCacheAlignedSession,
  PagedPoolCacheAlignedSession,
  NonPagedPoolCacheAlignedMustSSession,
};


NTL__EXTERNAPI
__declspec(restrict) __declspec(noalias)
void * __stdcall
  ExAllocatePoolWithTag(pool_type, size_t, unsigned long tag);

NTL__EXTERNAPI
__declspec(noalias)
void __stdcall
  ExFreePoolWithTag(void * __restrict p, unsigned long tag); 


template<pool_type PoolType = PagedPool>
class pool
{
  ///////////////////////////////////////////////////////////////////////////
  public:

    __declspec(restrict) __declspec(noalias)
    static __forceinline
    void * alloc(size_t size, unsigned long tag = NTL__POOL_TAG)
    { 
      return ExAllocatePoolWithTag(PoolType, size, tag);
    }

    __declspec(noalias)
    static __forceinline
    void free(void * const p, unsigned long tag = NTL__POOL_TAG)
    { 
      ExFreePoolWithTag(p, tag);
    }

};//class pool


}//namspace km
}//namespace ntl


#endif//#ifndef NTL__KM_POOL