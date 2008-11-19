/**\file*********************************************************************
*                                                                     \brief
*  Shared resource support
*
****************************************************************************
*/

#ifndef NTL__NT_RESOURCE
#define NTL__NT_RESOURCE

#include "basedef.hxx"
#include "handle.hxx"
#include "../stlx/chrono.hxx"

namespace ntl {
  namespace nt {

    typedef std::ratio_multiply<std::ratio<100>, std::nano>::type systime_unit;
    typedef std::chrono::duration<systime_t, systime_unit>        system_duration;

    namespace rtl
    {
      // critical section
      struct critical_section_debug
      {
        enum type { CritSect, Resource };
        uint16_t    Type;
        uint16_t    CreatorBackTraceIndex;
        struct critical_section* CriticalSection;
        list_entry  ProcessLocksList;
        uint32_t    EntryCount;
        uint32_t    ContentionCount;
        uint32_t    Spare[2];
      };

      struct critical_section
      {
        critical_section_debug* DebugInfo;
        int32_t       LockCount;
        int32_t       RecursionCount;
        legacy_handle OwningThread;
        legacy_handle LockSemaphore;
        uint32_t      reserved;
      };

      struct resource_debug
      {
        uint32_t      reserved[5];
        uint32_t      ContentionCount;
        uint32_t      Spare[2];
      };

      // shared resource
      struct resource
      {
        critical_section  CriticalSection;
        legacy_handle     SharedSemaphore;
        uint32_t          NumberOfWaitingShared;
        legacy_handle     ExclusiveSemaphore;
        uint32_t          NumberOfWaitingExclusive;

        int32_t           NumberOfActive;
        legacy_handle     ExclusiveOwnerThread;

        enum flags { None, LongTerm };
        flags             Flags;
        resource_debug*   DebugInfo;
      };

      // run once
      union run_once
      {
        void* Ptr;
      };

      typedef uint32_t __stdcall run_once_init_t(
          rtl::run_once* RunOnce,
          void* Parameter,
          void** Context
          );

    } // rtl


    typedef ntstatus __stdcall critical_section_control_t(rtl::critical_section* CriticalSection);

    NTL__EXTERNAPI
      critical_section_control_t
        RtlInitializeCriticalSection,
        RtlDeleteCriticalSection,
        RtlEnterCriticalSection,
        RtlLeaveCriticalSection;

    NTL__EXTERNAPI
      ntstatus __stdcall RtlInitializeCriticalSectionAndSpinCount(
        rtl::critical_section* CriticalSection,
        uint32_t               SpinCount
        );

    NTL__EXTERNAPI
      uint32_t __stdcall RtlTryEnterCriticalSection(rtl::critical_section* CriticalSection);

    NTL__EXTERNAPI
      uint32_t __stdcall RtlIsCriticalSectionLocked(rtl::critical_section* CriticalSection);

    NTL__EXTERNAPI
      uint32_t __stdcall RtlIsCriticalSectionLockedByThread(rtl::critical_section* CriticalSection);

    NTL__EXTERNAPI
      uint32_t __stdcall RtlGetCriticalSectionRecursionCount(rtl::critical_section* CriticalSection);

    NTL__EXTERNAPI
      uint32_t __stdcall RtlSetCriticalSectionSpinCount(rtl::critical_section* CriticalSection, uint32_t SpinCount);

    NTL__EXTERNAPI
      void __stdcall RtlEnableEarlyCriticalSectionEventCreation();

    NTL__EXTERNAPI
      void __stdcall RtlCheckForOrphanedCriticalSections(legacy_handle Thread);

    NTL__EXTERNAPI
      void __stdcall RtlpWaitForCriticalSection(rtl::critical_section* CriticalSection);

    NTL__EXTERNAPI
      void __stdcall RtlpUnWaitCriticalSection(rtl::critical_section* CriticalSection);

    NTL__EXTERNAPI
      void __stdcall RtlpNotOwnerCriticalSection(rtl::critical_section* CriticalSection);

    // run once
    NTL__EXTERNAPI
      void RtlRunOnceInitialize(rtl::run_once* RunOnce);

    NTL__EXTERNAPI
      uint32_t RtlRunOnceExecuteOnce(
        rtl::run_once*        RunOnce,
        rtl::run_once_init_t  InitFn,
        void*                 Parameter,
        void**                Context
      );

    NTL__EXTERNAPI
      uint32_t RtlRunOnceBeginInitialize(
        rtl::run_once*        RunOnce,
        uint32_t              Flags,
        void**                Context
      );

   NTL__EXTERNAPI
      uint32_t RtlRunOnceComplete(
        rtl::run_once*        RunOnce,
        uint32_t              Flags,
        void**                Context
      );



   class critical_section:
     protected rtl::critical_section
   {
   public:
     critical_section()
     {
       ntl::nt::RtlInitializeCriticalSection(this);
     }

     ~critical_section()
     {
       ntl::nt::RtlDeleteCriticalSection(this);
     }

     void acquire()
     {
       ntl::nt::RtlEnterCriticalSection(this);
     }

     void release()
     {
       ntl::nt::RtlLeaveCriticalSection(this);
     }

     bool try_acquire()
     {
       return ntl::nt::RtlTryEnterCriticalSection(this) != 0;
     }

     bool locked()
     {
       return ntl::nt::RtlIsCriticalSectionLocked(this) != 0;
     }

     bool thread_locked()
     {
       return ntl::nt::RtlIsCriticalSectionLockedByThread(this) != 0;
     }

     ntstatus wait(const systime_t& timeout, bool explicit_wait, bool alertable = true)
     {
       if(!LockSemaphore){
         if(!try_acquire() && !explicit_wait)
          return status::invalid_handle;
         // wait
         ntstatus st;

         systime_t period = timeout;
         systime_t const interval = std::chrono::duration_cast<system_duration>( std::chrono::milliseconds(50)).count();
         do{
           st = NtDelayExecution(alertable, interval);
           period -= interval;
         }while(st == status::timeout && period > 0);
         return st;
       }

       DebugInfo->EntryCount++;
       DebugInfo->ContentionCount++;
       return NtWaitForSingleObject(LockSemaphore, alertable, timeout);
     }

     template <class Clock, class Duration>
     ntstatus wait_until(const std::chrono::time_point<Clock, Duration>& abs_time, bool explicit_wait = true, bool alertable = true)
     {
       return wait(std::chrono::duration_cast<system_duration>(abs_time.time_since_epoch()).count(), explicit_wait, alertable);
     }

     template <class Rep, class Period>
     ntstatus wait_for(const std::chrono::duration<Rep, Period>& rel_time, bool explicit_wait = true, bool alertable = true)
     {
       return wait(-1i64 * std::chrono::duration_cast<system_duration>(rel_time).count(), explicit_wait, alertable);
     }
   };

  } //namespace nt
} //namespace ntl

#endif //#ifndef NTL__NT_RESOURCE

