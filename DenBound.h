//
// Copyright (c) 2024 Phobyx GmbH&Co.KG, Gerrit Meyer
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
#ifndef DENBOUNDCLASS_H
#define DENBOUNDCLASS_H


//types used for internal management:
#include <map>
typedef std::map<HSQUIRRELVM,HSQOBJECT> DenClassDefinitionMap ;
typedef std::map<HSQUIRRELVM,HSQOBJECT> selfrefmap_t ;

//platform dependent threading issue stuff (you can use den_mutex or interlocked macros if you want)
#if defined(WIN32) && !defined(__GNUC__)
#define _WINSOCKAPI_			// prevent inclusion of winsock.h ( instead of winsock2.h ) in windows
#include <windows.h>
#include <intrin.h>
#define den_interlockedInc(Var)						InterlockedIncrement(Var)
#define den_interlockedDec(Var)						InterlockedDecrement(Var)
#define den_interlockedCmpExchg(Dest, Exchg, Comp)	InterlockedCompareExchange((Dest), (Exchg), (Comp))
#define px_interlockedOr(Var, Bits)					_InterlockedOr((Var),(Bits))
#define px_interlockedAnd(Var, Bits)					_InterlockedAnd((Var),(Bits))
#ifdef GetClassName
#undef GetClassName
#endif
typedef volatile long den_interlocked_t ;
class den_mutex {
protected:
	CRITICAL_SECTION m_mutex ;
public:
	den_mutex() {InitializeCriticalSectionAndSpinCount(&m_mutex,4000);}
	~den_mutex() { DeleteCriticalSection(&m_mutex);}
	inline void Lock() { EnterCriticalSection(&m_mutex); }
	inline bool TryLock() { return TryEnterCriticalSection( &m_mutex ) != 0; }
	inline void Unlock() { LeaveCriticalSection(&m_mutex);}
};
#else //!WIN32
#if defined(__gnu_linux__) || defined(LINUX) || defined(__GNUC__)
#include <unistd.h>
#	define den_interlockedInc(Var)						(__sync_fetch_and_add((Var),1)+1)
#	define den_interlockedDec(Var)						(__sync_fetch_and_sub((Var),1)-1)
#	define den_interlockedCmpExchg(Dest, Exchg, Comp)	 __sync_val_compare_and_swap((Dest), (Comp), (Exchg))
#	define den_interlockedOr(Var,Bits)						__sync_fetch_and_or((Var),(Bits))
#	define den_interlockedAnd(Var,Bits)						__sync_fetch_and_and((Var),(Bits))
typedef volatile unsigned int den_interlocked_t ;  //don't use long on beagleboard or raspberry, only available on arch64!
# include <pthread.h>

class den_mutex {
protected:
	pthread_mutex_t m_mutex ;
	pthread_mutexattr_t m_attr ;
public:
	den_mutex() { pthread_mutexattr_init(&m_attr);
				  pthread_mutexattr_settype (&m_attr, PTHREAD_MUTEX_RECURSIVE_NP);
				  pthread_mutex_init(&m_mutex, &m_attr);
				}
	~den_mutex() { pthread_mutex_destroy(&m_mutex);
				   pthread_mutexattr_destroy(&m_attr); 
				 }
	inline void Lock() { pthread_mutex_lock(&m_mutex); }
	inline bool TryLock() { return pthread_mutex_trylock(&m_mutex) == 0; }
	inline void Unlock() { pthread_mutex_unlock(&m_mutex);}
};


#endif //__gnu_linux__
#endif //!WIN32

namespace _DENAPI {
	//DO NOT USE THIS STUFF manually!
	//Forward declarations
	SQInteger SQ_nexti (HSQUIRRELVM v) ;
	SQInteger SQ_typeof( HSQUIRRELVM v );
	SQInteger releasehook( SQUserPointer p, SQInteger size) ;
	struct DenSQUser {
		SQInteger udType ;
		DenBoundInterface *pclass ;
	};

};

class DenRefCounted {
public:
	DenRefCounted() : m_iRefCount(0) {};
	DenRefCounted(const DenRefCounted &) : m_iRefCount(0) {} ; //copy constructor required(!)
	virtual ~DenRefCounted() {} ;
	//General public interface:
	inline void AddRef() { den_interlockedInc(&m_iRefCount); }
	inline bool ReleaseRef() { if( den_interlockedDec(&m_iRefCount)==0 ) { delete this; return true ; } ; return false;}
	inline den_interlocked_t GetRef()	{ return m_iRefCount; }

private:
	den_interlocked_t m_iRefCount;
};


class DenBoundInterface : public DenRefCounted {
public:
	DenBoundInterface()  {} ; //:m_iRefCount(0) {} ; //refcount moved to base class
	//(However, it is initialized in DenRefCounted, so what's the damn difference?)
	//DenBoundInterface( const DenBoundInterface& ) : m_iRefCount(0) {} 
	//you MUST implement these:
	//static const SQChar *ClassName() = 0 ;
	static void BindScriptMethods(HSQUIRRELVM v) ; //top of stack will hold the class definition when called
	//You MAY override these (will be defined by the template below):
	virtual const char* TypeOf() =0 ; 
	
	//you shall not touch these:
	static SQUserPointer SGetClassTypeTag() { return 0; } //note: this function is redefined in DenBound template!
	virtual SQUserPointer GetClassTypeTag() const { return SGetClassTypeTag(); }
	HSQOBJECT GetSelfReference(HSQUIRRELVM v) {
		HSQOBJECT ret ;	
		m_refmutex.Lock() ;
		selfrefmap_t::iterator it=m_selfreferences.find(DENAPI.GetMainVM(v)) ;
		if (it==m_selfreferences.end()) SQAPI->resetobject(&ret) ;	else ret=it->second ; 
		m_refmutex.Unlock() ;
		return ret ;
	}
	/// returns handle to class definition
	static const HSQOBJECT SGetClassDefinition(HSQUIRRELVM) { HSQOBJECT ret ; SQAPI->resetobject(&ret); return ret; }; //this base class returns "no definition", important for inheritance
	virtual const HSQOBJECT GetClassDefinition(HSQUIRRELVM v) const { return SGetClassDefinition(v); }
	
	//will be called when an instance is created to set a self reference object
	bool OnCreation(HSQUIRRELVM v,SQInteger StackIdx)
	{
		v=DENAPI.GetMainVM(v) ;
		if (m_selfreferences.find(v)!=m_selfreferences.end()) return false ;
		HSQOBJECT o ; SQAPI->getstackobj (v,StackIdx,&o) ; if (o._type!=OT_INSTANCE) return false ;
		m_refmutex.Lock() ; m_selfreferences[v]=o ;	m_refmutex.Unlock() ; //no sq_addref here, otherwise the VM would never call the release hook...
		return true ;
	}
	//General public interface:
	/*moved to base class:
	inline void AddRef() { den_interlockedInc(&m_iRefCount); }
	inline bool ReleaseRef() { if( den_interlockedDec(&m_iRefCount)==0 ) { delete this; return true ; } ; return false;}
	inline den_interlocked_t GetRef()	{ return m_iRefCount; }
	*/

private:
	//den_interlocked_t m_iRefCount; moved to base class
	selfrefmap_t m_selfreferences ;
	den_mutex m_refmutex ;
};

template< class TClass, class TBase=DenBoundInterface >
class DenBound : public TBase {
public:
	//Register this class with a VM. Expects a table already on the stack (for example root table)
	//will leavte the table as is
	static void Register(HSQUIRRELVM v) {
		
		v=DENAPI.GetMainVM(v) ;
		_den_Mutex.Lock() ;
		DenClassDefinitionMap::iterator it=_den_ClassDefinitions.find(DENAPI.GetMainVM(v)) ;
		if ((it!=_den_ClassDefinitions.end()) && (it->second._type==OT_CLASS)) { _den_Mutex.Unlock() ; return ;}// avoid multiple registering on same VM	
		_den_Mutex.Unlock() ;	
		
		HSQOBJECT hClassDef; //later class definition for this class, now used as temporary parent class definition)
		hClassDef=SGetParentClassDefinition(v) ;
		//expects a table on the stack (binding namespace, for example root table)
		SQAPI->pushstring(v,TClass::ClassName(),-1) ;
		if (sq_isnull(hClassDef)) SQAPI->newclass(v,false) ; //no parent class definition
		else {
			SQAPI->pushobject(v,hClassDef) ; //push the parent class definition
			SQAPI->newclass(v,true) ; //has a parent
		}
		SQInteger top=SQAPI->gettop(v) ;//memorize current stack
		SQAPI->settypetag( v, -1, TClass::SGetClassTypeTag() );
		SQAPI->setclassudsize(v,-1,sizeof(struct _DENAPI::DenSQUser)) ;
		//SQAPI->setreleasehook(v,-1,_DENAPI::releasehook) ;
		
		// store class definition handle, and add reference to it
		
		SQAPI->getstackobj( v, -1, &hClassDef ); 
		SQAPI->addref( v, &hClassDef ); //class definitions will survive - as memory leaks!
		_den_Mutex.Lock() ;
		  _den_ClassDefinitions[v] = hClassDef; 
		_den_Mutex.Unlock() ;

		// bind the _nexti and typeof metamethod, needed for debugger on all instances
		::_DENAPI::RegisterNative( v,::_DENAPI::SQ_nexti, "_nexti",false );
		::_DENAPI::RegisterNative( v,::_DENAPI::SQ_typeof, "_typeof",false ) ;
		// let TClass register its methods (note: can override _nexti metamethod if needed)
		TClass::BindScriptMethods( v ); // NOTE: TClass has to implement "static void BindScriptMethods( HSQUIRRELVM v)"
		SQAPI->settop(v,top) ;	//recall memorized stack in case the class bind method did not handle it properly
		// finish binding
		SQAPI->newslot(v,-3,false) ; //set to current table/namespace
	}
	
	virtual const SQChar* TypeOf() { return TClass::ClassName() ; }
	static SQUserPointer SGetClassTypeTag() { return (SQUserPointer)&_den_ClassTag; }
	virtual SQUserPointer GetClassTypeTag() const { return SGetClassTypeTag(); }
	static HSQOBJECT SGetClassDefinition(HSQUIRRELVM v) {
		HSQOBJECT ret ;
		_den_Mutex.Lock() ;
		DenClassDefinitionMap::iterator it=_den_ClassDefinitions.find(DENAPI.GetMainVM(v)) ;
		if (it==_den_ClassDefinitions.end()) SQAPI->resetobject(&ret); else ret=it->second ;
		_den_Mutex.Unlock() ;
		return ret ;
	}
	virtual const HSQOBJECT GetClassDefinition(HSQUIRRELVM v) const { return SGetClassDefinition(v); }
	static const HSQOBJECT SGetParentClassDefinition(HSQUIRRELVM v) { return TBase::SGetClassDefinition(v); }  //will return a null object when no parent
	virtual const HSQOBJECT GetParentClassDefinition(HSQUIRRELVM v) const { return SGetParentClassDefinition(v); }
	
private:
	static SQInteger _den_ClassTag ; //can store flags inside if needed, all we need is its memory location address acting as a tag
	static DenClassDefinitionMap _den_ClassDefinitions ;
	static den_mutex _den_Mutex;
};


//REQUIRED IMPLEMENTATIONS - DO NOT USE OR AMEND STUFF BELOW THIS LINE!!!

//implement static members:
template< class TClass,class TBase> den_mutex DenBound<TClass,TBase>::_den_Mutex ;
template< class TClass,class TBase> SQInteger DenBound<TClass,TBase>::_den_ClassTag ;
template< class TClass,class TBase> DenClassDefinitionMap DenBound<TClass,TBase>::_den_ClassDefinitions ;




#endif
