

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 04:14:07 2038
 */
/* Compiler settings for ..\..\Source\Interface\sandbox.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __sandbox_h_h__
#define __sandbox_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_sandbox_0000_0000 */
/* [local] */ 

#pragma once

enum SandboxEventType
    {
        SBET_KeyEvent	= 0,
        SBET_MouseEvent	= ( SBET_KeyEvent + 1 ) 
    } ;
struct SandboxMouseEvent
    {
    int buttonIndex;
    int bKeyDown;
    int x;
    int y;
    } ;
struct SandboxKeyboardEvent
    {
    int keyIndex;
    int bKeyDown;
    } ;
struct SandboxEvent
    {
    int eventType;
    /* [switch_is] */ /* [switch_type] */ union 
        {
        /* [case()] */ struct SandboxKeyboardEvent keyboardEvent;
        /* [case()] */ struct SandboxMouseEvent mouseEvent;
        /* [default] */  /* Empty union arm */ 
        } 	;
    } ;


extern RPC_IF_HANDLE __MIDL_itf_sandbox_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sandbox_0000_0000_v0_0_s_ifspec;

#ifndef __Sandbox_INTERFACE_DEFINED__
#define __Sandbox_INTERFACE_DEFINED__

/* interface Sandbox */
/* [version][uuid] */ 

void RpcBeginImage( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ unsigned long Width,
    /* [in] */ unsigned long Height,
    /* [in] */ unsigned long Size);

void RpcSendImageChunkData( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ unsigned long BitmapSize,
    /* [length_is][size_is][in] */ const unsigned char BitmapData[  ]);

void RpcEndImage( 
    /* [in] */ handle_t IDL_handle);

boolean RpcPopEvent( 
    /* [in] */ handle_t IDL_handle,
    /* [ref][out] */ struct SandboxEvent *Event);



extern RPC_IF_HANDLE Sandbox_v1_0_c_ifspec;
extern RPC_IF_HANDLE Sandbox_v1_0_s_ifspec;
#endif /* __Sandbox_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


