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
#ifndef DENTACK_H
#define DENSTACK_H
#include "squirrel.h"
#include "../DenAPI/DenBound.h"
//a simple stack helper class, fully inline
namespace _DENAPI {


class stack {
private:
	SQInteger _top,_offs ;
	HSQUIRRELVM v ;
public:
	stack( HSQUIRRELVM v, SQInteger offset=0, SQInteger count=-1 )
		: v(v)
	{
		_offs = offset;
		_top = SQAPI->gettop(v) - offset;
		if( count >= 0 && count <= _top ) _top = count;
	}
	inline HSQUIRRELVM VM() { return v ;}
	inline SQInteger Offset() { return _offs;}

	inline HSQOBJECT GetObject(SQInteger idx) {	HSQOBJECT x; _DENAPI::s_SQAPI->resetobject(&x); _DENAPI::s_SQAPI->getstackobj(v,idx+_offs,&x); return x; }
	inline SQObjectType GetType(SQInteger idx) { return _DENAPI::s_SQAPI->gettype(v,idx+_offs) ; }
	inline DenObject GetDenObject(SQInteger idx) { DenObject x(v,idx); return x ;}
	
	inline SQFloat GetFloat(SQInteger idx) { SQFloat x = 0.0f;	_DENAPI::s_SQAPI->getfloat(v,idx+_offs,&x); return x; }
	inline SQInteger GetInt(SQInteger idx) { SQInteger x = 0; _DENAPI::s_SQAPI->getinteger(v,idx+_offs,&x); return x; }
	inline const SQChar *GetString(SQInteger idx) { const SQChar *x = 0; 	_DENAPI::s_SQAPI->getstring(v,idx+_offs,&x);	return x; }
	inline SQBool GetBool(SQInteger idx) { SQBool ret=SQFalse; _DENAPI::s_SQAPI->getbool(v,idx+_offs,&ret); return ret; }
	inline SQUserPointer GetUserPointer( SQInteger idx ) {	SQUserPointer x = 0;	_DENAPI::s_SQAPI->getuserpointer(v,idx+_offs,&x);	return x; }
	inline SQInteger GetParamCount() {	return _top; }
	inline SQInteger Return(const SQChar *s) {	_DENAPI::s_SQAPI->pushstring(v,s,-1);  return 1; }
	inline SQInteger Return(SQFloat f) { _DENAPI::s_SQAPI->pushfloat(v,f); return 1; }
	inline SQInteger Return(SQInteger i) {	_DENAPI::s_SQAPI->pushinteger(v,i); return 1 ;} ;
	inline SQInteger Return(bool b) { _DENAPI::s_SQAPI->pushbool(v,b);	return 1;}
	inline SQInteger Return(SQUserPointer p) {	if (p) _DENAPI::s_SQAPI->pushuserpointer(v,p); else _DENAPI::s_SQAPI->pushnull(v) ; return 1; }
	inline SQInteger Return( HSQOBJECT o ) { _DENAPI::s_SQAPI->pushobject(v,o); return 1; }
	inline SQInteger ReturnNull(void) {	_DENAPI::s_SQAPI->pushnull(v);	return 1; }
	inline SQInteger Return(void) { return 0; }
	inline SQInteger Return(DenBoundInterface *BoundClass) { _DENAPI::PushInstance(v,BoundClass) ; return 1 ;}
	inline SQInteger Return(DenObject &obj) { obj.Push(v) ; return 1 ; }
	//tries to get a certain DenBoundInterface class, returns 0 when no such class present
	template<class T> T* GetInstancePtr( SQInteger idx ) const
	{
		// optimized to return false early (without casts..)
		SQUserPointer up = GetInstanceUp( idx, T::SGetClassTypeTag() );
		if( up == 0 ) 
		{	_DENAPI::s_SQAPI->reseterror(v) ;
			return 0;
		}
		_DENAPI::DenSQUser* pxUser = static_cast<_DENAPI::DenSQUser*>(up);
		if( pxUser->pclass == 0 ) return 0;
		return static_cast<T*>(pxUser->pclass);
	}

	// return whether the object at idx is of given class type (
	template<class T> bool IsInstancePtr( SQInteger idx )
	{
		// optimized to return false early (without casts..)
		SQUserPointer up = GetInstanceUp( idx, T::SGetClassTypeTag() );
		if( up == NULL ) { _DENAPI::s_SQAPI->reseterror(v) ; return false; }
		::_DENAPI::DenSQUser* pxUser = static_cast<_DENAPI::DenSQUser*>(up);
		return (pxUser->pclass != NULL) ;
	}
private: 
	//No external access to user pointers, (used for DenBound classes)
	SQUserPointer GetInstanceUp(SQInteger idx,SQUserPointer tag=0) const
	{	
		SQUserPointer self;
		if( SQ_FAILED(_DENAPI::s_SQAPI->getinstanceup(v,idx+_offs,(SQUserPointer*)&self,tag)) ) return 0;
		return self;
	}
	SQUserPointer GetUserData(SQInteger idx,SQUserPointer tag=0)
	{
		SQUserPointer otag;
		SQUserPointer up;
		if( SQ_SUCCEEDED(_DENAPI::s_SQAPI->getuserdata(v,idx+_offs,&up,&otag)) && tag==otag ) return up;
		return 0;
	}
};


}
#endif