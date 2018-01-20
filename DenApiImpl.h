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
#ifndef DENHELPER_H
#define DENHELPER_H

//convenience macros
#define SQAPI ::_DENAPI::s_SQAPI 
#define DENAPI ::_DENAPI::s_DENAPI->API
#define DENTAGS ::_DENAPI::s_DENAPI->Tags

#define DENBIND_FUNC(v,name) \
	_DENAPI::RegisterNative(v,SQ_ ## name,#name,false);

#define den_assert(test,vm,text) ((test)?true:!(SQAPI->throwerror(vm,text)))


class DenObject ;
class DenBoundInterface ;
namespace _DENAPI {
	//API entry points for Squirrel and Den. You can use the defines above for convenience access
	//======================================================================================
	extern HSQAPI s_SQAPI ;
	extern HDENAPI s_DENAPI ;
	
	//function to create bound class instances in Squirrel and push them to the stack:
	void PushInstance( HSQUIRRELVM v, DenBoundInterface* pxObject ) ;
	//function to use in a constructor, to bind a bound class to an instance object already present:
	SQRESULT BindToInstanceOnStack( HSQUIRRELVM vm, DenBoundInterface* pxObject, SQInteger stack_idx=1 );
	//shortcut to get a DenObject from a bound class 
	DenObject WrapInstance(HSQUIRRELVM v, DenBoundInterface* pxObject) ;

	//convenience functions to use in "BindScriptMethods" of bound classes:
	//======================================================================================
	//Register a member function (function, function name and whether it is static or not)
	//You may also use this to register functions to tables/namespaces, however when doing so bStatic must be false!
	void RegisterNative( HSQUIRRELVM v,SQFUNCTION f, const SQChar *pcFunc, bool bStatic) ; 
	//Register an integer constant
	void RegisterIntegerConstant(HSQUIRRELVM v, SQChar *name, int value );


	//call this first in your module_init function. When it returns SQ_OK, continue by populating the table on the stack with your classes and functions
	//In case it returns SQ_ERROR, your module is disfunctional and can't get initialized!
	//remember note module_init  can be called several times for the same VM/Thread.
	SQRESULT module_init_helper(HSQUIRRELVM v, HSQAPI api,HSQMODULEINFO info,SQInteger MinimumAPIVersion=1,SQBool needcompiler=true, SQBool IAmThreadsafe=true) ;
	
	//call this in your module_shutdown function. When it returns true, you need to deinitialize global stuff, otherwise your module is still used in another thread
	//(only works in conjunction with module_init_helper!)
	//remember: unlike module_init, shutdown will be called only ONCE per thread
	SQBool module_shutdown_helper(HSQUIRRELVM v,HSQMODULEINFO info) ;


	SQInteger SQ_nexti (HSQUIRRELVM v);
	SQInteger SQ_typeof( HSQUIRRELVM v );
	SQInteger releasehook( SQUserPointer p, SQInteger size );
} ; //namespace _DENMAIN


//tool class to ease object handling. References an HSQOBJECT over its lifetime
class DenObject {
public:
	inline SQObjectType GetType() const { return _o._type ; } //put before constructors for some compilers, bcs. constr are using this inline
	inline void Reset() { if (GetType()!=OT_WEAKREF) { SQAPI->release(_v,&_o)	; } SQAPI->resetobject(&_o) ; } //same for destructor

	DenObject() : _v(0) { SQAPI->resetobject(&_o) ; }
	DenObject(HSQUIRRELVM v) :_v(v) { SQAPI->resetobject(&_o) ; SQAPI->getstackobj(_v,-1,&_o); if (GetType()!=OT_WEAKREF) SQAPI->addref(_v,&_o); SQAPI->pop(_v,1) ; } //pop object from stack
	DenObject(HSQUIRRELVM v,SQInteger stackindex) :_v(v) { SQAPI->resetobject(&_o);  SQAPI->getstackobj(_v,stackindex,&_o) ; if (GetType()!=OT_WEAKREF) SQAPI->addref(_v,&_o); } //get from stack, indexed (no pop)
	DenObject(const DenObject &other) : _o(other._o),_v(other._v) { if (GetType()!=OT_WEAKREF) SQAPI->addref(_v,&_o); } //copy constructor
	DenObject(HSQUIRRELVM v,HSQOBJECT &o) : _o(o),_v(v) { if (GetType()!=OT_WEAKREF) SQAPI->addref(v,&_o) ; } //initialize from HSQOBJECT
	virtual ~DenObject() {  Reset() ; } 

	inline HSQUIRRELVM VM() { return _v ;}
	inline HSQOBJECT GetHSQOBJECT() { return _o ; }
	inline void Push() { SQAPI->pushobject(_v,_o) ; }
	inline void Push(HSQUIRRELVM v) { SQAPI->pushobject(v,_o) ; } //for friend VM use. Use with care
private:
	HSQOBJECT _o ;
	HSQUIRRELVM _v ;

};

#endif
