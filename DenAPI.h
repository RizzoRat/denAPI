//
// SqModule: API used for modules to communicate with SquirrelDen
//

//
// Copyright (c) 2017 Phobyx GmbH&Co.KG, Gerrit Meyer
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//  claim that you wrote the original software. If you use this software
//  in a product, an acknowledgment in the product documentation would be
//  appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be
//  misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//  distribution.
//
//  4. Any compiled module must not break compatibility to gonuts or be used 
//	for reverse engineering or to circumvent protection and license mechanisms.

#pragma once
#ifndef DENAPI_H
#define DENAPI_H
#include "../DenAPI/sqmodule.h"

#if defined(_LP64)
typedef unsigned long long den64 ;		
typedef signed long long dens64 ;		

// 64 Bit Windows
#elif defined(_WIN64)
typedef unsigned long long den64 ;		
typedef signed long long dens64 ;		

// 32 Bit any
#else
typedef unsigned long long den64 ;		
typedef long long dens64 ;		
#endif



#ifdef __cplusplus
extern "C" {
#endif

//CHANGES BELOW THIS LINE is maintainers responsibility ONLY.
typedef void (*DenBufferDeallocFunc)(char *where) ;
typedef struct {
	struct HostSpecific host ;
	struct GonutsClasstags {
		SQUserPointer buffertag ;
		SQUserPointer sockbuftag ;
		SQUserPointer eventtag ;
	} Tags ;
	struct Denfuncs {	
		//Some important Squirrel Standard lib functions provided by DEN:
		//note not all sqstd functions make sense to call directly, in case you need them you can always
		//use them the squirrel VM way (push the root table (all stdlib functions reside there), 
		//perform a get and do a sqcall manually, clean up stack). Same applies for some gonuts 
		SQUserPointer	(*std_createblob)(HSQUIRRELVM v, SQInteger size);
		SQRESULT		(*std_getblob)(HSQUIRRELVM v,SQInteger idx,SQUserPointer *ptr);
		SQInteger		(*std_getblobsize)(HSQUIRRELVM v,SQInteger idx);
		
		SQRESULT		(*std_format)(HSQUIRRELVM v, SQInteger nformatstringidx, SQInteger * outlen, SQChar ** output);
		//remember dofile and loadfile do not support pnuts, encrypted and/or compressed files! Use ScriptRequire/Release instead!
		SQRESULT		(*std_dofile)(HSQUIRRELVM v, const SQChar * filename, SQBool retval, SQBool printerror);
		SQRESULT		(*std_loadfile)(HSQUIRRELVM v, const SQChar * filename, SQBool printerror);
		SQRESULT		(*std_writeclosuretofile)(HSQUIRRELVM v, const SQChar * filename);

		void			(*std_seterrorhandlers)(HSQUIRRELVM v);
		void			(*std_printcallstack)(HSQUIRRELVM v);


		//Basic Den VM handling
		HSQUIRRELVM (*GetMainVM)			(HSQUIRRELVM v) ; //get the root VM from any given VM
		HSQUIRRELVM (*RequestVM)			(HSQUIRRELVM parent,SQInteger stacksize) ; //request a new friend VM from pool. stacksize -1 is default stacksize (=same as root VM)
		SQRESULT	(*ReleaseVM)			(HSQUIRRELVM child) ; //release a pooled friend VM
		
		void		(*LockSharedState)		(HSQUIRRELVM v) ; //lock a VM after unlocking and before you return control to the gonuts VM. Carefully read documentation
		bool		(*TryLockSharedState)	(HSQUIRRELVM v) ;
		void		(*UnlockSharedState)	(HSQUIRRELVM v) ; //unlock a VM when you are going to pass control to the OS, like doing a sleep, select, wait or anything multithreading related

		//Extended VM handling (creating new VMs)
		HSQUIRRELVM (*NewMasterVM)				(SQInteger stacksize,SQInteger argc,SQChar **argv) ; //create a complete fresh, independent VM context
		SQRESULT	(*CloseMasterVM)			(HSQUIRRELVM v) ; //close a master VM. Note all pooled and friend VMs and objects you may have stored become invalid (bad pointers)!!!

		//gonuts Script handling
		SQRESULT	(*ScriptRequire)		(HSQUIRRELVM v,SQChar *scriptid, SQChar *name_space) ; //namespace =0 will derive namespace from scriptid
		SQRESULT	(*ScriptRelease)		(HSQUIRRELVM v,SQChar *scriptid) ;
		SQRESULT	(*ScriptShrink)			(HSQUIRRELVM v,SQChar *scriptid) ; //free overhead memory (disables Script.Run()!)
		//handling for supported gonuts classes (buffer, sockbuf and the like)
		bool		(*isGonutsType)			(HSQUIRRELVM v,SQInteger idx) ;
		bool		(*IsOfGonutsType)			(HSQUIRRELVM v,SQInteger idx,SQUserPointer which) ; //use tags provided in this API (see GonutsClassTags Tags)
		//SQBuffer creation (use GetAnyBuffer/CopyAnyBuffer to retrieve its data)
		SQRESULT	(*CreateBuffer)			(HSQUIRRELVM v,SQUserPointer buffer,SQInteger size,DenBufferDeallocFunc dealloc) ; //dealloc=NULL will not deallocate on destruction
		SQRESULT	(*CreateEmptyBuffer)	(HSQUIRRELVM v,SQInteger size) ; //create an uninitalized buffer of certain size (that will deallocate itself)
		
		//Event handling 
		//you cannot create an event in C++, if you need to do so you should compile/call a small squirrel function
		SQRESULT	(*Event_fire)			(HSQUIRRELVM v,SQInteger stackidx,SQInteger flags) ; //manually fire an event object at given stack position
		SQRESULT	(*Event_activate)		(HSQUIRRELVM v,SQInteger stackidx,SQInteger milliseconds) ; //milliseconds is interval, where -1 is to be used for non-timers
		SQRESULT	(*Event_deactivate)		(HSQUIRRELVM v,SQInteger stackidx) ;

		//sockbuf Handling
		SQRESULT	(*CreateSockbuf)		(HSQUIRRELVM v) ; //Create a new sockbuf
		SQRESULT	(*SockbufAdd)		(HSQUIRRELVM v,SQInteger idx,SQUserPointer data,SQInteger datasize) ; //Add Data to a buffer
		SQRESULT	(*CopyoutSockbuf)	(HSQUIRRELVM v,SQInteger idx,SQUserPointer BufPtr,SQInteger *BufSiz) ;//Read Data from a sockbuf without consuming
		SQRESULT	(*ReadSockbuf)		(HSQUIRRELVM v,SQInteger idx,SQUserPointer BufPtr,SQInteger *BufSiz) ;//Read Data from an sockbuf, consuming
		SQInteger	(*SockbufSize)		(HSQUIRRELVM v,SQInteger idx) ;//Get current size of EvBuffer
		//SQRESULT	(*CreateEvent)		(HSQUIRRELVM v,densocket_t x) ;

		//Binary stuff support (will accept strings, blobs and any of the two buffer classes, note that GetAnyBuffer will not work on EvBuffers though)
		SQChar *	(*GetAnyBuffer)		(HSQUIRRELVM v,SQInteger idx,SQInteger *BufSiz) ; //get a pointer to any buffer object without copying. Returns null on failure or when a copy is needed. Note: BufSiz NULL will not return any buffer size
		SQChar *	(*CopyAnybuffer)	(HSQUIRRELVM v,SQInteger idx,SQInteger *BufSiz) ;  //create a copy of any posssible buffer object. Returned value must be freed (via SQAPI->free), if there's no buffer this returns NULL. 
		//note that yet there is no direct support for things like Hash, base64encode/decode, tohex and the like; however you may manually use them easily via the stack utilizing SQAPI->sqcall
		
		//64bit class support. Note that the int64 instances are immutable, you can not change the "value" of an instance. Hence, all you can do is get the value and push an instance with the value
		bool		(*isint64) (HSQUIRRELVM v,SQInteger idx) ; //check if the object on the stack at the index is an int64 instance
		SQRESULT	(*getint64) (HSQUIRRELVM v,SQInteger idx,dens64 *target64) ; //get the value of the int64 at the given stack position, returns SQ_OK when done, otherwise SQ_ERROR
		void		(*pushint64) (HSQUIRRELVM v,dens64 value) ; //push the given 64 bit value as int64 instance on the stack
	} API;

} denAPI;
typedef denAPI* HDENAPI;


//TODO:
/*
typedef struct {
	struct sqstdlib {
		SQUserPointer	(*std_createblob)(HSQUIRRELVM v, SQInteger size);
		SQRESULT		(*std_getblob)(HSQUIRRELVM v,SQInteger idx,SQUserPointer *ptr);
		SQInteger		(*std_getblobsize)(HSQUIRRELVM v,SQInteger idx);
	} stdlib;
	struct HostSpecific host ;
} stdlibAPI;
*/

#ifdef __cplusplus
} //extern "C"
#endif

#include "../DenAPI/DenApiImpl.h"
#include "../DenAPI/DenStack.h"



#endif