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

//recommended to include this cpp file ONCE
//Alternative: Add it to your project and include "../DenAPI/DenApi.h"

//the implemantations here are mandatory for the Gonuts API to work

#include "../DenAPI/DenAPI.h"
#include "../DenAPI/DenStack.h" //include, just in case
HSQAPI _DENAPI::s_SQAPI=0 ;
HDENAPI _DENAPI::s_DENAPI=0 ;
#include <string.h>
namespace _DENAPI {
	static den_interlocked_t s_ModuleInstanceCount=0 ;	

	void init(HSQAPI sq,HDENAPI den,HSQUIRRELVM v=0) {
		s_SQAPI=sq ;
		s_DENAPI=den ;
	}


	void RegisterNative( HSQUIRRELVM v,SQFUNCTION f, const SQChar *pcFunc, bool bStatic)
	{
		//HSQUIRRELVM v=GetRootVM()->GetVM() ;
		//sq_pushobject(v,GetBindingNamespace().GetObjectHandle()) ;
		SQAPI->pushstring(v,pcFunc,-1) ;
		SQAPI->newclosure(v,f,0) ;
		SQAPI->newslot(v,-3,bStatic);
	}
	void RegisterIntegerConstant(HSQUIRRELVM v, SQChar *name, int value )
	{
		SQAPI->pushconsttable(v);
		SQAPI->pushstring(v,name,-1);
		SQAPI->pushinteger(v,value);
		SQAPI->newslot(v,-3,false);
		SQAPI->poptop( v );
	}

	//module user should call this first on module_init, then initialize the target table which is on the stack when this returns SQ_OK
	//Note: When it returns SQ_ERROR, there are compatibility issues and APIs aren'T initialized properly
	SQRESULT module_init_helper(HSQUIRRELVM v, HSQAPI api,HSQMODULEINFO info,SQInteger MinimumAPIVersion,SQBool needcompiler, SQBool IAmThreadsafe) 
	{

		//check for api version and squirrel compiler setting match.
		//minimum API version is 1, plus we must have the squirrel compiler on the host
		if (!SQ_SUCCEEDED(CheckSQCompatibility(api,MinimumAPIVersion,needcompiler,IAmThreadsafe))) return SQ_ERROR ; //this module is not compatible with the host


		if (!info->modrefcountonVM) //first call for this VM's shared state?
		{
			den_interlockedInc(&s_ModuleInstanceCount) ;
			//s_ModuleInstanceCount++ ; //count the VMs (shared states, to be exact) we were facing
		} 
		SQAPI = api; //we can save the API pointer statically in all cases
		_DENAPI::s_DENAPI = 0 ;
		HostSpecific *cur=api->UserHost ;
		while (cur)
		{
			if (!strcmp(cur->name,"DEN_API")) { _DENAPI::s_DENAPI=(HDENAPI)cur->API ; cur=0 ; }
			else cur=cur->chain_next ;
		}
		if (_DENAPI::s_DENAPI) return SQ_OK;
		return SQ_ERROR ;
	}

	SQBool module_shutdown_helper(HSQUIRRELVM v,HSQMODULEINFO info)
	{

		if ((den_interlockedDec(&s_ModuleInstanceCount))<=0)
		{
			SQAPI=0 ; //assure developers awareness in case squirrel calls happen after the last VM reference got dropped. Calls will crash then!
			_DENAPI::s_DENAPI=0 ;
			return true ;
		}
		return false;
	}

	//To create a squirrel instance of a bound c++ instance and push it on the stack
	//Note it will re-use already existing squirrel instances if that very c++ instance is already linked somewhere else inside the same VM shared state.
	void PushInstance( HSQUIRRELVM v, DenBoundInterface* pxObject )
	{
		if (!pxObject) {
			// push a NULL
			SQAPI->pushnull(v);
			return;
		}
		HSQOBJECT xREF=pxObject->GetSelfReference(v) ;
		if (xREF._type==OT_INSTANCE || xREF._type==OT_WEAKREF)    
		{	//yes, there is already a reference, so deliver that one
			SQAPI->pushobject (v,xREF) ; 
			return ;
		}
		SQInteger top=SQAPI->gettop(v) ;
		SQAPI->pushroottable( v );
		SQAPI->pushobject( v, pxObject->GetClassDefinition(v) );	// use provided class definition
		SQAPI->createinstance( v, -1 );
		SQAPI->remove( v, -2 ); // remove root table from stack
		SQAPI->remove( v, -2 ); // remove class object from stack
		// set userdata
		_DENAPI::DenSQUser* pxUser = NULL;
		SQAPI->getinstanceup( v, -1, (SQUserPointer*)&pxUser, 0,false );
		pxUser->pclass = pxObject;
		pxUser->udType=1 ;
		pxObject->OnCreation(v,-1) ;
		pxObject->AddRef() ;		
		SQAPI->setreleasehook( v, -1, _DENAPI::releasehook );
		// top = instance
	}
	//Use this within constructor implementations. When a "constructor" is called, an empty class instance object already is on the top of the stack
	//This function will bind a bound c++ instance to it. However, NEVER bind twice to the same squirrel instance or weird things may happen.
	SQRESULT BindToInstanceOnStack( HSQUIRRELVM v, DenBoundInterface* pxObject, SQInteger stack_idx )
	{
		if (pxObject->GetSelfReference(v)._type!=OT_NULL) return SQ_ERROR ;
		_DENAPI::DenSQUser* pxUser = NULL;
		SQAPI->getinstanceup( v, stack_idx, (SQUserPointer*)&pxUser, pxObject->GetClassTypeTag(),false );
		if (!pxUser) return SQ_ERROR ;
		// initialize user data for instance 
		pxObject->AddRef() ;
		pxUser->pclass=pxObject ;
		pxUser->udType=1 ;
		pxObject->OnCreation(v,stack_idx) ;
		SQAPI->setreleasehook( v, stack_idx, &_DENAPI::releasehook );
		return SQ_OK ;
	}

	DenObject WrapInstance( HSQUIRRELVM vm, DenBoundInterface* pxObject)
	{
		
		_DENAPI::PushInstance(vm,pxObject) ;
		DenObject ret(vm) ; //pops instance
		return ret ;
	}



} ; //namespace _DENMAIN



//internal mandatory static implementations following, do not touch
//"empty" member iteration function. Mandatory to exist.

namespace _DENAPI {
	SQInteger SQ_nexti (HSQUIRRELVM v) 
	{ 	//All bound classes need this metamethod when the debugger encounters them
		SQAPI->pushnull(v) ;
		SQAPI->throwerror(v,_SC("non iterable instance")) ; //in case user tries to iterate a bound class. debugger will ignore this for displaying states
		return 1 ;
	}
	//default implementation for typeof operator. Mandatory to exist,
	SQInteger SQ_typeof( HSQUIRRELVM v )
	{
		_DENAPI::stack sa(v) ;
		DenBoundInterface *pxThis=sa.GetInstancePtr<DenBoundInterface>(1) ;
		if (!pxThis) return sa.Return("instance") ;
		return sa.Return(pxThis->TypeOf()) ;
	}
	//internal release hook for DenBound instances
	SQInteger releasehook( SQUserPointer p, SQInteger size ) 
	{
		::_DENAPI::DenSQUser* pxUser = (::_DENAPI::DenSQUser*)p;
		SQInteger ret=1 ;
		if (pxUser->pclass)
		{
			pxUser->pclass->ReleaseRef() ;
			pxUser->pclass=0 ;
		} 
		return 1;
	}


} ;

