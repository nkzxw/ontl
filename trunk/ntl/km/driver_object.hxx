/**\file*********************************************************************
 *                                                                     \brief
 *  Driver Object
 *
 ****************************************************************************
 */

#ifndef NTL__KM_DRIVER_OBJECT
#define NTL__KM_DRIVER_OBJECT

#include "basedef.hxx"
#include "string.hxx"
#include "irp.hxx"

namespace ntl {
namespace km {

struct device_object;
struct driver_object;

struct driver_extension
{
  driver_object *   DriverObject;
  long (__stdcall * AddDevice)(driver_object *, device_object *);
  uint32_t          Count;
  unicode_string    ServiceKeyName;
  struct _IO_CLIENT_EXTENSION* ClientDriverExtension;
  struct _FS_FILTER_CALLBACKS* FsFilterCallbacks;
};


struct driver_object
{
  uint16_t                    Type;
  uint16_t                    Size;
  device_object *             DeviceObject;
  uint32_t                    Flags;
  void *                      DriverStart;
  uint32_t                    DriverSize;
  nt::ldr_data_table_entry *  DriverSection;
  driver_extension *          DriverExtension;
  unicode_string              DriverName;
  unicode_string *            HardwareDatabase;
  struct _FAST_IO_DISPATCH *  FastIoDispatch;
  ntstatus(__stdcall *        DriverInit)(driver_object*, unicode_string*);
  void    (__stdcall *        DriverStartIo)(driver_object*, irp*);
  void    (__stdcall *        DriverUnload)(driver_object*);
  typedef ntstatus dispatch_t(const device_object * DeviceObject, const irp *);
  dispatch_t *                MajorFunction[28];
};

STATIC_ASSERT(sizeof(driver_object) == 0xa8);
STATIC_ASSERT(offsetof(driver_object, MajorFunction) == 0x38);


}//namspace km
}//namespace ntl


#endif//#ifndef NTL__KM_DRIVER_OBJECT
