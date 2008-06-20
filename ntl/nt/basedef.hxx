/**\file*********************************************************************
 *                                                                     \brief
 *  NT Types support library
 *
 ****************************************************************************
 */

#ifndef NTL__NT_BASEDEF
#define NTL__NT_BASEDEF

#include "status.hxx"
#include "../basedef.hxx"
#include "../stdlib.hxx"
#include "string.hxx"
#include "../pe/image.hxx"

namespace ntl {
namespace nt {

/**\addtogroup  native_types_support *** NT Types support library ***********
 *@{*/


///\todo implement as the double_linked typedef
struct list_entry
{
  union { list_entry * Flink; list_entry * next; };
  union { list_entry * Blink; list_entry * prev; };

  void link(list_entry * prev, list_entry * next)
  {
    this->prev = prev; this->next = next;
    prev->next = this; next->prev = this;
  }

  void unlink()
  {
    next->prev = prev;
    prev->next = next;
  }
};


struct list_head : public list_entry
{
  list_head()
  {
    next = this;
    prev = this;
  }

  bool empty() const { return next == this; }

  void insert(list_entry * position, list_entry * entry)
  {
    entry->link(position, position->next);
  }

  list_entry * erase(list_entry * position)
  { 
    list_entry * const next = position->next;
    position->unlink();
    return next;
  }

  void insert_tail(list_entry * entry)
  {
    entry->link(prev, this);
  }

  list_entry * begin() { return next; }
  list_entry * end()   { return this; }
  
};


struct single_list_entry
{
  single_list_entry * Next;

  single_list_entry * next() const { return Next; }
  void next(single_list_entry * p)  { Next = p; }
};

typedef single_list_entry slist_entry;

alignas(8)
struct slist_header : public slist_entry
{
  uint16_t    Depth;
  uint16_t    Sequence;
};


/// Masks for the predefined standard access types
enum access_mask
{
  no_access = 0,
  delete_access             = 0x00010000L,
  read_control              = 0x00020000L,
  write_dac                 = 0x00040000L,
  write_owner               = 0x00080000L,
  synchronize               = 0x00100000L,
  standard_rights_required  = delete_access | read_control | write_dac | write_owner,
  standard_rights_read      = read_control,
  standard_rights_write     = read_control,
  standard_rights_execute   = read_control,
  standard_rights_all       = standard_rights_required | synchronize,
  specific_rights_all       = 0x0000FFFFL,
  access_system_security    = 0x01000000L,
  maximum_allowed           = 0x02000000L,
  generic_read              = 0x80000000L,
  generic_write             = 0x40000000L,
  generic_execute           = 0x20000000L,
  generic_all               = 0x10000000L
};

static inline 
access_mask  operator | (access_mask m, access_mask m2) 
{ 
  return bitwise_or(m, m2);
}

static inline 
access_mask  operator & (access_mask m, access_mask m2) 
{ 
  return bitwise_and(m, m2);
}


struct generic_mapping
{
  access_mask generic_read;
  access_mask generic_write;
  access_mask generic_execute;
  access_mask generic_all;
};


struct io_status_block 
{
  union { ntstatus  Status; void * Pointer; };
  uintptr_t Information;
};


typedef
void __stdcall
  io_apc_routine(
    const void *            ApcContext,
    const io_status_block * IoStatusBlock,
    uint32_t                Reserved
    );


/**@} native_types_support */

enum system_power_state 
{
  PowerSystemUnspecified,
  PowerSystemWorking,
  PowerSystemSleeping1,
  PowerSystemSleeping2,
  PowerSystemSleeping3,
  PowerSystemHibernate,
  PowerSystemShutdown,
  PowerSystemMaximum
};


enum power_action 
{
  PowerActionNone,
  PowerActionReserved,
  PowerActionSleep,
  PowerActionHibernate,
  PowerActionShutdown,
  PowerActionShutdownReset,
  PowerActionShutdownOff,
  PowerActionWarmEject
};

enum device_power_state
{
  PowerDeviceUnspecified,
  PowerDeviceD0,
  PowerDeviceD1,
  PowerDeviceD2,
  PowerDeviceD3,
  PowerDeviceMaximum
};


struct ldr_data_table_entry
{
  /* 0x00 */ list_entry           InLoadOrderLinks;
  /* 0x08 */ list_entry           InMemoryOrderLinks;
  /* 0x10 */ list_entry           InInitializationOrderLinks;
  /* 0x18 */ pe::image *          DllBase;
  /* 0x1c */ void *               EntryPoint;
  /* 0x20 */ uint32_t             SizeOfImage;
  /* 0x24 */ const_unicode_string FullDllName;
  /* 0x2c */ const_unicode_string BaseDllName;
  /* 0x34 */ uint32_t             Flags;
  /* 0x38 */ uint16_t             LoadCount;
  /* 0x3a */ uint16_t             TlsIndex;
//  /* 0x3c */ list_entry           HashLinks;
  /* 0x3c */ void *               SectionPointer;
  /* 0x40 */ uint32_t             CheckSum;
  union {
  /* 0x44 */ uint32_t             TimeDateStamp;
  /* 0x44 */ void *               LoadedImports;
  };
  /* 0x48 */ void *               EntryPointActivationContext;
  /* 0x4c */ void *               PatchInformation;

  struct find_dll
  {
    /**\todo  Redesign:
     *  Thre is no good reason to pass an arbitrary list_entry to ctor since 
     *  offset to BaseDllName is *harcoded* by reinterpret_cast.
     *  So head ptr are to stick to a ldr_data_table_entry itself; and perhaps 
     *  a ptr to member should passed (if one wants search InMemoryOrderLinks BTW).
     */
    find_dll(list_head * head) : head(head) {}

    const pe::image * operator()(const char name[]) const
    {
      if ( head )
        for ( list_entry * it = head->begin(); it != head->end(); it = it->next )
        {
          const ldr_data_table_entry * const entry = 
                                    reinterpret_cast<ldr_data_table_entry *>(it);
          // work arround ghost modules
          if(!entry->BaseDllName.size())
            continue;
          for ( unsigned short i = 0; i != entry->BaseDllName.size(); ++i )
          {
            if ( (entry->BaseDllName[i] ^ name[i]) & 0x5F )
              goto other_name;
          }
          return entry->DllBase;
          other_name:;
        }
      return 0;
    }

    private:
      list_head * head;

  };

};
STATIC_ASSERT(sizeof(ldr_data_table_entry) == 0x50 || sizeof(ldr_data_table_entry) == 0x98);


NTL__EXTERNAPI
ntstatus __stdcall
  NtDelayExecution(
    bool            Alertable,
    const int64_t * DelayInterval
    );


template<times TimeResolution>
static inline
ntstatus sleep(
  uint32_t        time_resolution,
  bool            alertable = false)
{
  const int64_t interval = int64_t(-1) * TimeResolution * time_resolution;
  return NtDelayExecution(alertable, &interval);
}

/// default milliseconds
static inline
ntstatus sleep(
  uint32_t        ms,
  bool            alertable = false)
{
  const int64_t interval = int64_t(-1) * milliseconds * ms;
  return NtDelayExecution(alertable, &interval);
}



}//namespace nt
}//namespace ntl

#endif//#ifndef NTL__NT_BASEDEF
