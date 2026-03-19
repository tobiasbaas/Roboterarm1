/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** USBX Component                                                        */
/**                                                                       */
/**   STM32 Controller Driver                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE
#define UX_DCD_STM32_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_dcd_stm32.h"
#include "ux_utility.h"
#include "ux_device_stack.h"

/* ----- DEBUG: direct register-bang UART1 output (STM32N6) ----- */
#define _DBG_U1_ISR_D (*(volatile unsigned long *)(0x52001000UL + 0x1CUL))
#define _DBG_U1_TDR_D (*(volatile unsigned long *)(0x52001000UL + 0x28UL))
static void _dcd_dbg(const char *s){while(*s){while(!(_DBG_U1_ISR_D&(1UL<<7))){}; _DBG_U1_TDR_D=(unsigned char)*s++;}}
/* -------------------------------------------------------------- */


#if !defined(UX_DEVICE_STANDALONE)
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_stm32_transfer_request                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will initiate a transfer to a specific endpoint.      */
/*    If the endpoint is IN, the endpoint register will be set to accept  */
/*    the request.                                                        */
/*                                                                        */
/*    If the endpoint is IN, the endpoint FIFO will be filled with the    */
/*    buffer and the endpoint register set.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_stm32                             Pointer to device controller  */
/*    transfer_request                      Pointer to transfer request   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    HAL_PCD_EP_Transmit                   Transmit data                 */
/*    HAL_PCD_EP_Receive                    Receive data                  */
/*    _ux_utility_semaphore_get             Get semaphore                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    STM32 Controller Driver                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s), used ST  */
/*                                            HAL library to drive the    */
/*                                            controller,                 */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _ux_dcd_stm32_transfer_request(UX_DCD_STM32 *dcd_stm32, UX_SLAVE_TRANSFER *transfer_request)
{

UX_SLAVE_ENDPOINT       *endpoint;
UINT                    status;


    /* Get the pointer to the logical endpoint from the transfer request.  */
    endpoint =  transfer_request -> ux_slave_transfer_request_endpoint;

    /* Check for transfer direction.  Is this a IN endpoint ? */
    if (transfer_request -> ux_slave_transfer_request_phase == UX_TRANSFER_PHASE_DATA_OUT)
    {

        /* Transmit data.  */
        _dcd_dbg("[D] pre-TX\r\n");
        HAL_PCD_EP_Transmit(dcd_stm32 -> pcd_handle,
                            endpoint->ux_slave_endpoint_descriptor.bEndpointAddress,
                            transfer_request->ux_slave_transfer_request_data_pointer,
                            transfer_request->ux_slave_transfer_request_requested_length);
        _dcd_dbg("[D] post-TX\r\n");

        /* If the endpoint is a Control endpoint, all this is happening under Interrupt and there is no
           thread to suspend.  */
        if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & (UINT)~UX_ENDPOINT_DIRECTION) != 0)
        {

            /* PATCHED: Use a finite 5-second timeout instead of UX_WAIT_FOREVER.
               If the host stops scheduling IN tokens (selective suspend, driver
               stall, flow-control back-pressure), the XFRC interrupt never fires
               and the original code would block the calling thread forever.
               With a timeout we can detect and recover. */
            _dcd_dbg("[D] pre-sem\r\n");
            status =  _ux_utility_semaphore_get(&transfer_request -> ux_slave_transfer_request_semaphore,
                                                5000UL);
            _dcd_dbg("[D] post-sem\r\n");

            /* On timeout: clean up hardware state so the next write starts fresh. */
            if (status != UX_SUCCESS)
            {
                _dcd_dbg("[D] SEM TIMEOUT!\r\n");
                /* Disable TXFE interrupt for this endpoint via DIEPEMPMSK */
                uint32_t ep_num = endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & 0x0FU;
                PCD_HandleTypeDef *hpcd = dcd_stm32 -> pcd_handle;
                uint32_t USBx_BASE = (uint32_t)(hpcd -> Instance);
                USBx_DEVICE -> DIEPEMPMSK &= ~(1UL << ep_num);

                /* Flush the TX FIFO for this endpoint */
                HAL_PCD_EP_Flush(hpcd,
                                 endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress);

                transfer_request -> ux_slave_transfer_request_completion_code = UX_TRANSFER_ERROR;
                return(UX_TRANSFER_ERROR);
            }

            transfer_request -> ux_slave_transfer_request_actual_length = transfer_request->ux_slave_transfer_request_requested_length;

            /* Check the transfer request completion code. We may have had a BUS reset or
               a device disconnection.  */
            if (transfer_request -> ux_slave_transfer_request_completion_code != UX_SUCCESS)
                return(transfer_request -> ux_slave_transfer_request_completion_code);

            /* Return to caller with success.  */
            return(UX_SUCCESS);
        }
    }
    else
    {

        /* We have a request for a SETUP or OUT Endpoint.  */
        /* Receive data.  */
        HAL_PCD_EP_Receive(dcd_stm32 -> pcd_handle,
                            endpoint->ux_slave_endpoint_descriptor.bEndpointAddress,
                            transfer_request->ux_slave_transfer_request_data_pointer,
                            transfer_request->ux_slave_transfer_request_requested_length);

        /* If the endpoint is a Control endpoint, all this is happening under Interrupt and there is no
           thread to suspend.  */
        if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & (UINT)~UX_ENDPOINT_DIRECTION) != 0)
        {

            /* We should wait for the semaphore to wake us up.  */
            status =  _ux_utility_semaphore_get(&transfer_request -> ux_slave_transfer_request_semaphore,
                                                (ULONG)transfer_request -> ux_slave_transfer_request_timeout);

            /* Check the completion code. */
            if (status != UX_SUCCESS)
                return(status);

            /* Check the transfer request completion code. We may have had a BUS reset or
               a device disconnection.  */
            if (transfer_request -> ux_slave_transfer_request_completion_code != UX_SUCCESS)
                return(transfer_request -> ux_slave_transfer_request_completion_code);

            /* Return to caller with success.  */
            return(UX_SUCCESS);
        }
    }

    /* Return to caller with success.  */
    return(UX_SUCCESS);
}
#endif
