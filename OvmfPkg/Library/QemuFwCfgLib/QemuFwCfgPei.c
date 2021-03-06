/** @file

  Stateful and implicitly initialized fw_cfg library implementation.

  Copyright (C) 2013, Red Hat, Inc.
  Copyright (c) 2011 - 2013, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2017, Advanced Micro Devices. All rights reserved.<BR>

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/QemuFwCfgLib.h>
#include <Library/MemEncryptSevLib.h>

#include "QemuFwCfgLibInternal.h"

STATIC BOOLEAN mQemuFwCfgSupported = FALSE;
STATIC BOOLEAN mQemuFwCfgDmaSupported;


/**
  Returns a boolean indicating if the firmware configuration interface
  is available or not.

  This function may change fw_cfg state.

  @retval    TRUE   The interface is available
  @retval    FALSE  The interface is not available

**/
BOOLEAN
EFIAPI
QemuFwCfgIsAvailable (
  VOID
  )
{
  return InternalQemuFwCfgIsAvailable ();
}


RETURN_STATUS
EFIAPI
QemuFwCfgInitialize (
  VOID
  )
{
  UINT32 Signature;
  UINT32 Revision;

  //
  // Enable the access routines while probing to see if it is supported.
  // For probing we always use the IO Port (IoReadFifo8()) access method.
  //
  mQemuFwCfgSupported = TRUE;
  mQemuFwCfgDmaSupported = FALSE;

  QemuFwCfgSelectItem (QemuFwCfgItemSignature);
  Signature = QemuFwCfgRead32 ();
  DEBUG ((EFI_D_INFO, "FW CFG Signature: 0x%x\n", Signature));
  QemuFwCfgSelectItem (QemuFwCfgItemInterfaceVersion);
  Revision = QemuFwCfgRead32 ();
  DEBUG ((EFI_D_INFO, "FW CFG Revision: 0x%x\n", Revision));
  if ((Signature != SIGNATURE_32 ('Q', 'E', 'M', 'U')) ||
      (Revision < 1)
     ) {
    DEBUG ((EFI_D_INFO, "QemuFwCfg interface not supported.\n"));
    mQemuFwCfgSupported = FALSE;
    return RETURN_SUCCESS;
  }

  if ((Revision & FW_CFG_F_DMA) == 0) {
    DEBUG ((DEBUG_INFO, "QemuFwCfg interface (IO Port) is supported.\n"));
  } else {
    //
    // If SEV is enabled then we do not support DMA operations in PEI phase.
    // This is mainly because DMA in SEV guest requires using bounce buffer
    // (which need to allocate dynamic memory and allocating a PAGE size'd
    // buffer can be challenge in PEI phase)
    //
    if (InternalQemuFwCfgSevIsEnabled ()) {
      DEBUG ((DEBUG_INFO, "SEV: QemuFwCfg fallback to IO Port interface.\n"));
    } else {
      mQemuFwCfgDmaSupported = TRUE;
      DEBUG ((DEBUG_INFO, "QemuFwCfg interface (DMA) is supported.\n"));
    }
  }
  return RETURN_SUCCESS;
}


/**
  Returns a boolean indicating if the firmware configuration interface is
  available for library-internal purposes.

  This function never changes fw_cfg state.

  @retval    TRUE   The interface is available internally.
  @retval    FALSE  The interface is not available internally.
**/
BOOLEAN
InternalQemuFwCfgIsAvailable (
  VOID
  )
{
  return mQemuFwCfgSupported;
}

/**
  Returns a boolean indicating whether QEMU provides the DMA-like access method
  for fw_cfg.

  @retval    TRUE   The DMA-like access method is available.
  @retval    FALSE  The DMA-like access method is unavailable.
**/
BOOLEAN
InternalQemuFwCfgDmaIsAvailable (
  VOID
  )
{
  return mQemuFwCfgDmaSupported;
}

/**

 Returns a boolean indicating whether SEV is enabled

 @retval    TRUE    SEV is enabled
 @retval    FALSE   SEV is disabled
**/
BOOLEAN
InternalQemuFwCfgSevIsEnabled (
  VOID
  )
{
  return MemEncryptSevIsEnabled ();
}

/**
 Allocate a bounce buffer for SEV DMA.

  @param[in]     NumPage  Number of pages.
  @param[out]    Buffer   Allocated DMA Buffer pointer

**/
VOID
InternalQemuFwCfgSevDmaAllocateBuffer (
  OUT    VOID     **Buffer,
  IN     UINT32   NumPages
  )
{
  //
  // We should never reach here
  //
  ASSERT (FALSE);
  CpuDeadLoop ();
}

/**
 Free the DMA buffer allocated using InternalQemuFwCfgSevDmaAllocateBuffer

  @param[in]     NumPage  Number of pages.
  @param[in]     Buffer   DMA Buffer pointer

**/
VOID
InternalQemuFwCfgSevDmaFreeBuffer (
  IN     VOID     *Buffer,
  IN     UINT32   NumPages
  )
{
  //
  // We should never reach here
  //
  ASSERT (FALSE);
  CpuDeadLoop ();
}
