/**\file*********************************************************************
 *                                                                     \brief
 *  NT Thread Environment
 *
 ****************************************************************************
 */

#ifndef NTL__NT_TEB
#define NTL__NT_TEB

#include "exception.hxx"
#include "handle.hxx"


namespace ntl {

///\tode move to another header
namespace intrinsic {
  
#if defined(_M_IX86)

  extern "C" uint8_t  __cdecl __readfsbyte (uint32_t Offset);
  extern "C" uint16_t __cdecl __readfsword (uint32_t Offset);
  extern "C" uint32_t __cdecl __readfsdword(uint32_t Offset);
  extern "C" uint64_t __cdecl __readfsqword(uint32_t Offset);

  extern "C" void __cdecl __writefsbyte (uint32_t Offset, uint8_t Data);
  extern "C" void __cdecl __writefsword (uint32_t Offset, uint16_t Data);
  extern "C" void __cdecl __writefsdword(uint32_t Offset, uint32_t Data);
  extern "C" void __cdecl __writefsqword(uint32_t Offset, uint64_t Data);

#pragma intrinsic(__readfsbyte, __readfsword, __readfsdword/*, __readfsqword*/)
#pragma intrinsic(__writefsbyte, __writefsword, __writefsdword/*, __writefsqword*/)

#elif defined(_M_X64)

  extern "C" uint8_t  __cdecl __readgsbyte (uint32_t Offset);
  extern "C" uint16_t __cdecl __readgsword (uint32_t Offset);
  extern "C" uint32_t __cdecl __readgsdword(uint32_t Offset);
  extern "C" uint64_t __cdecl __readgsqword(uint32_t Offset);

  extern "C" void __cdecl __writegsbyte (uint32_t Offset, uint8_t Data);
  extern "C" void __cdecl __writegsword (uint32_t Offset, uint16_t Data);
  extern "C" void __cdecl __writegsdword(uint32_t Offset, uint32_t Data);
  extern "C" void __cdecl __writegsqword(uint32_t Offset, uint64_t Data);

#pragma intrinsic(__readgsbyte, __readgsword, __readgsdword, __readgsqword)
#pragma intrinsic(__writegsbyte, __writegsword, __writegsdword, __writegsqword)

#endif

}//namespace intrinsic

#if defined(_M_IX86)

  __forceinline uint8_t  readsysptr(uint32_t Offset, uint8_t)
  { 
    return intrinsic::__readfsbyte(Offset);
  }
  __forceinline uint16_t readsysptr(uint32_t Offset, uint16_t)
  { 
    return intrinsic::__readfsword(Offset);
  }
  __forceinline uint32_t readsysptr(uint32_t Offset, uint32_t)
  { 
    return intrinsic::__readfsdword(Offset);
  }
  __forceinline uint64_t readsysptr(uint32_t Offset, uint64_t)
  { 
    return intrinsic::__readfsdword(Offset) | (uint64_t)intrinsic::__readfsdword(Offset+4) << 32;
  }
  __forceinline uintptr_t readsysptr(uint32_t Offset, const void* = 0)
  { 
    return intrinsic::__readfsdword(Offset);
  }

  __forceinline void writesysptr(uint32_t Offset, uint8_t Data)
  { 
    intrinsic::__writefsbyte(Offset, Data);
  }
  __forceinline void writesysptr(uint32_t Offset, uint16_t Data)
  { 
    intrinsic::__writefsword(Offset, Data);
  }
  __forceinline void writesysptr(uint32_t Offset, uint32_t Data)
  { 
    intrinsic::__writefsdword(Offset, Data);
  }
  __forceinline void writesysptr(uint32_t Offset, uint64_t Data)
  { 
    intrinsic::__writefsdword(Offset, static_cast<uint32_t>(Data));
    intrinsic::__writefsdword(Offset+4, static_cast<uint32_t>(Data >> 32));
  }
  __forceinline void writesysptr(uint32_t Offset, const void* Data)
  { 
    intrinsic::__writefsdword(Offset, reinterpret_cast<uintptr_t>(Data));
  }

#elif defined(_M_X64)

  __forceinline uint8_t  readsysptr(uint32_t Offset, uint8_t)
  { 
    return intrinsic::__readgsbyte(Offset);
  }
  __forceinline uint16_t readsysptr(uint32_t Offset, uint16_t)
  { 
    return intrinsic::__readgsword(Offset);
  }
  __forceinline uint32_t readsysptr(uint32_t Offset, uint32_t)
  { 
    return intrinsic::__readgsdword(Offset);
  }
  __forceinline uint64_t readsysptr(uint32_t Offset, uint64_t)
  { 
    return intrinsic::__readgsqword(Offset);
  }
  __forceinline uintptr_t readsysptr(uint32_t Offset, const void*)
  { 
    return intrinsic::__readgsqword(Offset);
  }

  __forceinline void writesysptr(uint32_t Offset, uint8_t Data)
  { 
    intrinsic::__writegsbyte(Offset, Data);
  }
  __forceinline void writesysptr(uint32_t Offset, uint16_t Data)
  { 
    intrinsic::__writegsword(Offset, Data);
  }
  __forceinline void writesysptr(uint32_t Offset, uint32_t Data)
  { 
    intrinsic::__writegsdword(Offset, Data);
  }
  __forceinline void writesysptr(uint32_t Offset, uint64_t Data)
  { 
    intrinsic::__writegsqword(Offset, Data);
  }
  __forceinline void writesysptr(uint32_t Offset, const void* Data = 0)
  { 
    intrinsic::__writegsqword(Offset, reinterpret_cast<uintptr_t>(Data));
  }

#endif


namespace nt {

struct peb;

/// Thread information block
/// @note mapped at fs:0x00
struct tib
{
  /* 0x00 */  exception::registration * ExceptionList;
  /* 0x04 */  void                    * StackBase;    ///< upper stack address
  /* 0x08 */  void                    * StackLimit;   ///< lower stack address
  /* 0x0C */  void                    * SubSystemTib;
              union
              {
  /* 0x10 */    void                  * FiberData;
  /* 0x10 */    uint32_t                Version;
              };
  /* 0x14 */  void                    * ArbitraryUserPointer;
  /* 0x18 */  tib                     * Self; ///< flat address of this structure
};

STATIC_ASSERT(sizeof(tib) == 0x1C);


struct client_id
{
  legacy_handle UniqueProcess;
  legacy_handle UniqueThread;
};


struct teb : public tib
{
  template<typename type>
  static inline
  type get(type teb::* member) 
  {
    const teb * const p = 0;
    const uint32_t offset = reinterpret_cast<uint32_t>(&(p->*member));
    return (type)(readsysptr(offset, type()));
  }

  template<typename type, typename type2>
  static inline
  void set(type teb::* member, type2 value) 
  {
    const teb * const p = 0;
    const uint32_t offset = reinterpret_cast<uint32_t>(&(p->*member));
    writesysptr(offset, value);
  }

  static __forceinline
  teb & instance() { return *static_cast<teb*>(get(&tib::Self)); }

  // common part
  /* 0x1c */  void *    EnvironmentPointer;
  /* 0x20 */  client_id ClientId;
  /* 0x28 */  void *    ActiveRpcHandle;
  /* 0x2c */  void *    ThreadLocalStoragePointer;
  /* 0x30 */  peb *     ProcessEnvironmentBlock;
  /* 0x34 */  uint32_t  LastErrorValue;
#if 0
  /*<thisrel this+0x38>*/ /*|0x4|*/ unsigned long CountOfOwnedCriticalSections;
  /*<thisrel this+0x3c>*/ /*|0x4|*/ void* CsrClientThread;
  /*<thisrel this+0x40>*/ /*|0x4|*/ void* Win32ThreadInfo;
  /*<thisrel this+0x44>*/ /*|0x68|*/ unsigned long User32Reserved[26];
  /*<thisrel this+0xac>*/ /*|0x14|*/ unsigned long UserReserved[5];
  /*<thisrel this+0xc0>*/ /*|0x4|*/ void* WOW32Reserved;                      // wntdll!X86SwitchTo64BitMode()
  /*<thisrel this+0xc4>*/ /*|0x4|*/ unsigned long CurrentLocale;
  /*<thisrel this+0xc8>*/ /*|0x4|*/ unsigned long FpSoftwareStatusRegister;
  /*<thisrel this+0xcc>*/ /*|0xd8|*/ void* SystemReserved1[54];
  /*<thisrel this+0x1a4>*/ /*|0x4|*/ long ExceptionCode;
  /*<thisrel this+0x1a8>*/ /*|0x14|*/ struct _ACTIVATION_CONTEXT_STACK ActivationContextStack;
  /*<thisrel this+0x1bc>*/ /*|0x18|*/ unsigned char SpareBytes1[24];
  /*<thisrel this+0x1d4>*/ /*|0x4e0|*/ struct _GDI_TEB_BATCH GdiTebBatch;
  /*<thisrel this+0x6b4>*/ /*|0x8|*/ struct _CLIENT_ID RealClientId;
  /*<thisrel this+0x6bc>*/ /*|0x4|*/ void* GdiCachedProcessHandle;
  /*<thisrel this+0x6c0>*/ /*|0x4|*/ unsigned long GdiClientPID;
  /*<thisrel this+0x6c4>*/ /*|0x4|*/ unsigned long GdiClientTID;
  /*<thisrel this+0x6c8>*/ /*|0x4|*/ void* GdiThreadLocalInfo;
  /*<thisrel this+0x6cc>*/ /*|0xf8|*/ unsigned long Win32ClientInfo[62];
  /*<thisrel this+0x7c4>*/ /*|0x3a4|*/ void* glDispatchTable[233];
  /*<thisrel this+0xb68>*/ /*|0x74|*/ unsigned long glReserved1[29];
  /*<thisrel this+0xbdc>*/ /*|0x4|*/ void* glReserved2;
  /*<thisrel this+0xbe0>*/ /*|0x4|*/ void* glSectionInfo;
  /*<thisrel this+0xbe4>*/ /*|0x4|*/ void* glSection;
  /*<thisrel this+0xbe8>*/ /*|0x4|*/ void* glTable;
  /*<thisrel this+0xbec>*/ /*|0x4|*/ void* glCurrentRC;
  /*<thisrel this+0xbf0>*/ /*|0x4|*/ void* glContext;
  /*<thisrel this+0xbf4>*/ /*|0x4|*/ unsigned long LastStatusValue;
  /*<thisrel this+0xbf8>*/ /*|0x8|*/ struct _UNICODE_STRING StaticUnicodeString;
  /*<thisrel this+0xc00>*/ /*|0x20a|*/ unsigned short StaticUnicodeBuffer[261];
  /*<thisrel this+0xe0c>*/ /*|0x4|*/ void* DeallocationStack;
  /*<thisrel this+0xe10>*/ /*|0x100|*/ void* TlsSlots[64];
  /*<thisrel this+0xf10>*/ /*|0x8|*/ struct _LIST_ENTRY TlsLinks;
  /*<thisrel this+0xf18>*/ /*|0x4|*/ void* Vdm;
  /*<thisrel this+0xf1c>*/ /*|0x4|*/ void* ReservedForNtRpc;
  /*<thisrel this+0xf20>*/ /*|0x8|*/ void* DbgSsReserved[2];
  /*<thisrel this+0xf28>*/ /*|0x4|*/ unsigned long HardErrorsAreDisabled;
  /*<thisrel this+0xf2c>*/ /*|0x40|*/ void* Instrumentation[16];
  /*<thisrel this+0xf6c>*/ /*|0x4|*/ void* WinSockData;
  /*<thisrel this+0xf70>*/ /*|0x4|*/ uintptr_t GdiBatchCount                  // teb64* on win32 process (on wow64)
  /*<thisrel this+0xf74>*/ /*|0x1|*/ unsigned char InDbgPrint;
  /*<thisrel this+0xf75>*/ /*|0x1|*/ unsigned char FreeStackOnTermination;
  /*<thisrel this+0xf76>*/ /*|0x1|*/ unsigned char HasFiberData;
  /*<thisrel this+0xf77>*/ /*|0x1|*/ unsigned char IdealProcessor;
  /*<thisrel this+0xf78>*/ /*|0x4|*/ unsigned long Spare3;
  /*<thisrel this+0xf7c>*/ /*|0x4|*/ void* ReservedForPerf;
  /*<thisrel this+0xf80>*/ /*|0x4|*/ void* ReservedForOle;
  /*<thisrel this+0xf84>*/ /*|0x4|*/ unsigned long WaitingOnLoaderLock;
  /*<thisrel this+0xf88>*/ /*|0xc|*/ struct _Wx86ThreadState Wx86Thread;
  /*<thisrel this+0xf94>*/ /*|0x4|*/ void** TlsExpansionSlots;
  /*<thisrel this+0xf98>*/ /*|0x4|*/ unsigned long ImpersonationLocale;
  /*<thisrel this+0xf9c>*/ /*|0x4|*/ unsigned long IsImpersonating;
  /*<thisrel this+0xfa0>*/ /*|0x4|*/ void* NlsCache;
  /*<thisrel this+0xfa4>*/ /*|0x4|*/ void* pShimData;
  /*<thisrel this+0xfa8>*/ /*|0x4|*/ unsigned long HeapVirtualAffinity;
  /*<thisrel this+0xfac>*/ /*|0x4|*/ void* CurrentTransactionHandle;
  /*<thisrel this+0xfb0>*/ /*|0x4|*/ struct _TEB_ACTIVE_FRAME* ActiveFrame;
  /*<thisrel this+0xfb4>*/ /*|0x1|*/ unsigned char SafeThunkCall;
  /*<thisrel this+0xfb5>*/ /*|0x3|*/ unsigned char BooleanSpare[3];
#endif
};
// <size 0xfb8> XPSP2


}//namespace nt
}//namespace ntl

#endif//#ifndef NTL__NT_TEB