//
// SqModule: API used for creating modules for GoNuts
//

//
// Copyright (c) 2016-2022 Phobyx GmbH&Co.KG, Gerrit Meyer
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
//
#ifndef WIN32
#ifndef __unix
#define __unix
#endif
#endif
#if !defined(_SQ_MODULE_H_)
#define _SQ_MODULE_H_

#include "squirrel.h" //needed for macros, structs and pointer types. Do NOT use its api functions though! 
//You MUST NOT use another Squirrel to operate on gonuts VMs - crashes would be imminent due to special features, e.g. memory management, dtors and more


#define HSQAPI_VERSION 320

#ifdef __cplusplus
extern "C" {
#endif
	//the module certificate structure for module authentication/trust
	struct SQModCert
	{
	  SQInteger ModMagic ; //must be 0x13333331 (even on 64 bit systems) to make the cert struct valid
	  SQModCert *previous ; //previous intermediate certificate in chain (if any) or NULL (for root cert)
	  SQInteger pubkeylen ; //length of the public key in bytes. 
	  SQUserPointer pubkey ; //Pointer to the public key. (For Modules it usually is arbitrary data, not able to sign or validate anything)
	  SQInteger signaturelen ; //length of the signature in bytes
	  SQUserPointer signature ; //Pointer to the signature for this certificate
	  const unsigned char *moduleid ; //null terminated module identification  string, maximum length 255 characters(including termination). Recommeded: a GUUID string
	  const unsigned char *issuer ; //optional pointer to null terminated name/id of the issuer (user defined), maximum length 255 characters (including termination)
	  
	  //moduleid and issuer are fully defined by issuer. It is recommended to fill in meaningful, human readable ASCII data identifying the module and/or owner
	  //either ONE of moduleid or issuer can be NULL. For intermediate certs, moduleid shall be null and issuer is mandatory
	  //for the final module cert moduleid is mandatory and issuer is optional (but recommended)
	}  ;
	
	//Info structure the module should provide back to gonuts. gonuts will pass a pointer to a pointer you shall set to this (static) structure you provide.
	typedef struct  
	{
		SQInteger version ; //version of the module
		SQInteger subversion ; //sub-version of the module (recommended: a revision number from your versioning software, be it svn, git or whatever you use)
		const SQChar *modulename ; //pointer to a short static null terminated string indicating the module name, e.g. "densqlite\0" 
		const unsigned char *moduleid ; //null terminated module identification string. recommended: point to the same static string as in the SQModCert struct
		SQBool extend ; //set to 0, mandatory
	} SQModuleIdentity;
	typedef SQModuleIdentity* HSQMODULEID ;

	//Dynamic info structure passed to the module. Fully volatile, if your module wants to store anything from this, you need to copy!
	typedef struct 
	{
	  SQInteger modrefcountonVM ; //the _current_ reference count of the module on the SHARED STATE of the current VM.
	  const SQChar *filename ;//the name of your module's shared library (not necessarily the name used by ::import!), null terminated
	  const SQChar *libname ;//the name of your module as used with ::import, null terminated
	  SQBool extend ; //ignore
	} SQmoduleInfo;
	typedef SQmoduleInfo* HSQMODULEINFO;

	//Compile Time Information structure. Keep size multiple of 8, NEVER change order of members!
	//This struct must be kept platform independent/compatible (and hence packed)
#if defined(__GNUC__)
#define pstruct	struct __attribute ((packed))
#elif defined(WIN32)
#pragma pack(push,1)
#define pstruct	struct 
#else
#error unknown environment
#endif
	pstruct SQmoduleAPIhead
	{	
		char PtrSize ;		//sizeof ( void * ) 
		char SQIntegerSize ;  //sizeof ( SQInteger )
		char SQFloatSize ;	//sizeof ( SQFloat )
		char GCAvailable ;	//0=Production mode, no Garbage Collector, 1=test mode, GC available
		char Unicode ;		//0=ASCII, 1=Unicode
		char CompilerAvailable ;// 0=no compiler, 1=compiler available
		char MultiThreadedEnvironment ; //0=single threaded, 1=VMs in different threads
		char reserved ;  //possibly becomes the value of SQ_ALIGNMENT some day
	} ;
#if defined(WIN32) && !defined(__GNUC__)
#pragma pack(pop)
#endif
#undef pstruct

	//Host may provide specific API using this struct. The API is User/Host defined.
	//Used for modules using host specific stuff, like own libraries
	//It is also used for Den, using the name "DEN_API"
	struct HostSpecific { 
		HostSpecific *chain_next ; //pointer to another HostSpecific, if any
		char name[80];			   //this array holds a null terminated ASCII String identifying the host specific API
		unsigned int version ;
		void *API ; //pointer to a function list or ANY data the host defines. Needs a cast depending on name/version
	} ;

	

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @cond DEV
    /// Allows modules to interface with Squirrel's C api without linking to the squirrel library
    /// If new functions are added to the Squirrel API, they should be added here too. 
	/// Never change any prototypes or order of the first members, though! (compatibility/platform issues)
	/// Avoid changing any existing members or member order.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef struct {
		void * Reserved; //reserved for later further extensions
		//TODO: sqstdlib pointers
	} API2StdLibExtension ;
	typedef API2StdLibExtension* HSQSTDAPI ;
	
	typedef struct {
	
		//WARNING: No changes below this line!
		SQmoduleAPIhead		sqconfig ;
		SQInteger		APIVersion ;
		HostSpecific	*UserHost;	  //Host specific API, if any, or NULL
		void*		Reserved; 
		
		
		//API Version 1 starts here (Squirrel 3.1)
				
        /*vm*/
		SQInteger		(*getversion)() ;
        HSQUIRRELVM     (*open)(SQInteger initialstacksize); //do not use gonuts features on such a VM. 
        HSQUIRRELVM     (*newthread)(HSQUIRRELVM friendvm, SQInteger initialstacksize); //better use DENAPI.RequestVM instead!
        void            (*seterrorhandler)(HSQUIRRELVM v);
        void            (*close)(HSQUIRRELVM v); //do not use on gonuts VMs!
        void            (*setforeignptr)(HSQUIRRELVM v,SQUserPointer p);  //do not use on gonuts VMs!
        SQUserPointer   (*getforeignptr)(HSQUIRRELVM v); //do not use on gonuts VMs
        void            (*setprintfunc)(HSQUIRRELVM v, SQPRINTFUNCTION printfunc, SQPRINTFUNCTION); //Squirrel 2: will ignore second parameter
        SQPRINTFUNCTION (*getprintfunc)(HSQUIRRELVM v);
        SQRESULT        (*suspendvm)(HSQUIRRELVM v);
        SQRESULT        (*wakeupvm)(HSQUIRRELVM v,SQBool resumedret,SQBool retval,SQBool raiseerror,SQBool throwerror);
        SQInteger       (*getvmstate)(HSQUIRRELVM v);

        /*compiler*/
        SQRESULT        (*compile)(HSQUIRRELVM v,SQLEXREADFUNC read,SQUserPointer p,const SQChar *sourcename,SQBool raiseerror);
        SQRESULT        (*compilebuffer)(HSQUIRRELVM v,const SQChar *s,SQInteger size,const SQChar *sourcename,SQBool raiseerror);
        void            (*enabledebuginfo)(HSQUIRRELVM v, SQBool enable);
        void            (*notifyallexceptions)(HSQUIRRELVM v, SQBool enable);  
        void            (*setcompilererrorhandler)(HSQUIRRELVM v,SQCOMPILERERROR f);

        /*stack operations*/
        void            (*push)(HSQUIRRELVM v,SQInteger idx);
        void            (*pop)(HSQUIRRELVM v,SQInteger nelemstopop);
        void            (*poptop)(HSQUIRRELVM v);
        void            (*remove)(HSQUIRRELVM v,SQInteger idx);
        SQInteger       (*gettop)(HSQUIRRELVM v);
        void            (*settop)(HSQUIRRELVM v,SQInteger newtop);

        SQRESULT        (*reservestack)(HSQUIRRELVM v,SQInteger nsize); //squirrel 2: will always return SQ_OK 
        SQInteger       (*cmp)(HSQUIRRELVM v);
        void            (*move)(HSQUIRRELVM dest,HSQUIRRELVM src,SQInteger idx);

        /*object creation handling*/
        SQUserPointer   (*newuserdata)(HSQUIRRELVM v,SQUnsignedInteger size);
        void            (*newtable)(HSQUIRRELVM v);
        void            (*newarray)(HSQUIRRELVM v,SQInteger size);
        void            (*newclosure)(HSQUIRRELVM v,SQFUNCTION func,SQUnsignedInteger nfreevars);
        SQRESULT        (*setparamscheck)(HSQUIRRELVM v,SQInteger nparamscheck,const SQChar *typemask);
        SQRESULT        (*bindenv)(HSQUIRRELVM v,SQInteger idx);
        void            (*pushstring)(HSQUIRRELVM v,const SQChar *s,SQInteger len);
        void            (*pushfloat)(HSQUIRRELVM v,SQFloat f);
        void            (*pushinteger)(HSQUIRRELVM v,SQInteger n);
        void            (*pushbool)(HSQUIRRELVM v,SQBool b);
        void            (*pushuserpointer)(HSQUIRRELVM v,SQUserPointer p);
        void            (*pushnull)(HSQUIRRELVM v);
        SQObjectType    (*gettype)(HSQUIRRELVM v,SQInteger idx);
        SQInteger       (*getsize)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*getbase)(HSQUIRRELVM v,SQInteger idx);
        SQBool          (*instanceof)(HSQUIRRELVM v);
        SQRESULT        (*tostring)(HSQUIRRELVM v,SQInteger idx); //squirrel 2: will ALWAYS return SQ_OK

        void            (*tobool)(HSQUIRRELVM v, SQInteger idx, SQBool *b);
        SQRESULT        (*getstring)(HSQUIRRELVM v,SQInteger idx,const SQChar **c);
        SQRESULT        (*getinteger)(HSQUIRRELVM v,SQInteger idx,SQInteger *i);
        SQRESULT        (*getfloat)(HSQUIRRELVM v,SQInteger idx,SQFloat *f);
        SQRESULT        (*getbool)(HSQUIRRELVM v,SQInteger idx,SQBool *b);
        SQRESULT        (*getthread)(HSQUIRRELVM v,SQInteger idx,HSQUIRRELVM *thread);
        SQRESULT        (*getuserpointer)(HSQUIRRELVM v,SQInteger idx,SQUserPointer *p);
        SQRESULT        (*getuserdata)(HSQUIRRELVM v,SQInteger idx,SQUserPointer *p,SQUserPointer *typetag);
        SQRESULT        (*settypetag)(HSQUIRRELVM v,SQInteger idx,SQUserPointer typetag);
        SQRESULT        (*gettypetag)(HSQUIRRELVM v,SQInteger idx,SQUserPointer *typetag);
        void            (*setreleasehook)(HSQUIRRELVM v,SQInteger idx,SQRELEASEHOOK hook);
        SQChar*         (*getscratchpad)(HSQUIRRELVM v,SQInteger minsize);
        SQRESULT        (*getclosureinfo)(HSQUIRRELVM v,SQInteger idx,SQInteger *nparams,SQInteger *nfreevars);
        SQRESULT        (*setnativeclosurename)(HSQUIRRELVM v,SQInteger idx,const SQChar *name);
        SQRESULT        (*setinstanceup)(HSQUIRRELVM v, SQInteger idx, SQUserPointer p);
        SQRESULT        (*getinstanceup310)(HSQUIRRELVM v, SQInteger idx, SQUserPointer *p,SQUserPointer typetag); //Old prototype Squirrel 3.10, now wrapped for compatibility
        SQRESULT        (*setclassudsize)(HSQUIRRELVM v, SQInteger idx, SQInteger udsize);
        SQRESULT        (*newclass)(HSQUIRRELVM v,SQBool hasbase);
        SQRESULT        (*createinstance)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*setattributes)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*getattributes)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*getclass)(HSQUIRRELVM v,SQInteger idx);
        void            (*weakref)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*getdefaultdelegate)(HSQUIRRELVM v,SQObjectType t);

        /*object manipulation*/
        void            (*pushroottable)(HSQUIRRELVM v);
        void            (*pushregistrytable)(HSQUIRRELVM v);
        void            (*pushconsttable)(HSQUIRRELVM v);
        SQRESULT        (*setroottable)(HSQUIRRELVM v);
        SQRESULT        (*setconsttable)(HSQUIRRELVM v);
        SQRESULT        (*newslot)(HSQUIRRELVM v, SQInteger idx, SQBool bstatic);
        SQRESULT        (*deleteslot)(HSQUIRRELVM v,SQInteger idx,SQBool pushval);
        SQRESULT        (*set)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*get)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*rawget)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*rawset)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*rawdeleteslot)(HSQUIRRELVM v,SQInteger idx,SQBool pushval);
        SQRESULT        (*arrayappend)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*arraypop)(HSQUIRRELVM v,SQInteger idx,SQBool pushval);
        SQRESULT        (*arrayresize)(HSQUIRRELVM v,SQInteger idx,SQInteger newsize);
        SQRESULT        (*arrayreverse)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*arrayremove)(HSQUIRRELVM v,SQInteger idx,SQInteger itemidx);
        SQRESULT        (*arrayinsert)(HSQUIRRELVM v,SQInteger idx,SQInteger destpos);
        SQRESULT        (*setdelegate)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*getdelegate)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*clone)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*setfreevariable)(HSQUIRRELVM v,SQInteger idx,SQUnsignedInteger nval);
        SQRESULT        (*next)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*getweakrefval)(HSQUIRRELVM v,SQInteger idx);
        SQRESULT        (*clear)(HSQUIRRELVM v,SQInteger idx);

        /*calls*/
        SQRESULT        (*call)(HSQUIRRELVM v,SQInteger params,SQBool retval,SQBool raiseerror);
        SQRESULT        (*resume)(HSQUIRRELVM v,SQBool retval,SQBool raiseerror);
        const SQChar*   (*getlocal)(HSQUIRRELVM v,SQUnsignedInteger level,SQUnsignedInteger idx);
        const SQChar*   (*getfreevariable)(HSQUIRRELVM v,SQInteger idx,SQUnsignedInteger nval);
        SQRESULT        (*throwerror)(HSQUIRRELVM v,const SQChar *err);
        void            (*reseterror)(HSQUIRRELVM v);
        void            (*getlasterror)(HSQUIRRELVM v);

        /*raw object handling*/
        SQRESULT        (*getstackobj)(HSQUIRRELVM v,SQInteger idx,HSQOBJECT *po);
        void            (*pushobject)(HSQUIRRELVM v,HSQOBJECT obj);
        void            (*addref)(HSQUIRRELVM v,HSQOBJECT *po);
        SQBool          (*release)(HSQUIRRELVM v,HSQOBJECT *po);
        void            (*resetobject)(HSQOBJECT *po);
        const SQChar*   (*objtostring)(const HSQOBJECT *o);
        SQBool          (*objtobool)(const HSQOBJECT *o);
        SQInteger       (*objtointeger)(const HSQOBJECT *o);
        SQFloat         (*objtofloat)(const HSQOBJECT *o);
        SQRESULT        (*getobjtypetag)(const HSQOBJECT *o,SQUserPointer * typetag);

        /*GC*/
        SQInteger       (*collectgarbage)(HSQUIRRELVM v);

        /*serialization*/
        SQRESULT        (*writeclosure)(HSQUIRRELVM vm,SQWRITEFUNC writef,SQUserPointer up);
        SQRESULT        (*readclosure)(HSQUIRRELVM vm,SQREADFUNC readf,SQUserPointer up);

        /*mem allocation. With Den you are highly recommended to use them, but take note: They're utilizing thread local(!!!) pools optimized for Squirrel!*/
        void*           (*malloc)(SQUnsignedInteger size);
        void*           (*realloc)(void* p,SQUnsignedInteger oldsize,SQUnsignedInteger newsize);
        void            (*free)(void *p,SQUnsignedInteger size);

        /*debug*/
        SQRESULT        (*stackinfos)(HSQUIRRELVM v,SQInteger level,SQStackInfos *si);
        void            (*setdebughook)(HSQUIRRELVM v);
		//API VERSION 1 ENDS HERE
		//API VERSION 2 EXTENDS 1 BY (Squirrel 3.2)
		SQRESULT        (*getinstanceup)(HSQUIRRELVM v, SQInteger idx, SQUserPointer *p,SQUserPointer typetag,SQBool throwerror); //Squirrel 3.2 prototype
		SQRESULT		(*tailcall)(HSQUIRRELVM v, SQInteger nparams);
		void			(*pushthread)(HSQUIRRELVM v, HSQUIRRELVM thread);
		//API VERSION 2 ENDS HERE
		//API VERSION 3 EXTENDS 2 BY:
		// Maintainers will add new functions here

		//sqstdlib:
		//

		
		//Introduced with API Vesion 2:
		void		   *lastjumptableentry ; //last entry always has a value of 0xDEADBEEF !  (to allow for NULL pointers some day, whatever use they could have)
		void		   *jumptableendmarkproof ; //where always followed by a NULL pointer.
	
    } sq_api;
    typedef sq_api* HSQAPI;
	
    /// @endcond

////////////////////////////////////////////////////////////
/// Module dll/so export prototypes. 
////////////////////////////////////////////////////////////
#ifdef WIN32
#define SQMOD_EXPORT __declspec(dllexport)
#define SQMOD_IMPORT __declspec(dllimport)
#else
#define SQMOD_EXPORT __attribute__((visibility("default")))
#define SQMOD_IMPORT

#endif

#ifndef _MODULE_NO_PROTOTYPES
//Modules need to implement these
#ifdef __cplusplus
#define CDECL extern "C"
#else
#define CDECL 
#endif
//Module initialization. 
//Note: this may get called several times (per ::import call)
//It shall check compatibility, for example using the provided inline function below (CheckSQCompatibility)
//Return SQ_OK after initialisation, return SQ_ERROR when incompatible or an error occured.
//You may return -5 to indicate an exception and that used sq_throwerror in your function. (Returning SQ_ERROR is silent)
#ifdef WIN32
	CDECL __declspec(dllexport) SQRESULT gonutsmodule_init(HSQUIRRELVM v,HSQAPI api,HSQMODULEINFO info,HSQMODULEID *identity) ; 
#else
	CDECL __attribute__((visibility("default")))  SQRESULT gonutsmodule_init(HSQUIRRELVM v,HSQAPI api,HSQMODULEINFO info,HSQMODULEID *identity) ; 
#endif
 

//Module deinitialization. 
//Usually gets called ONCE per VM at shutdown, when the host is about to sq_close(v)
//Should free memory and sq_release all objects the module referenced for the given VM
//Attention: A module must take care properly of the situation where it got initialized
//for more than one VM. See docs
//return value should be always SQ_OK (reserved for future changes)

#ifdef WIN32
CDECL __declspec(dllexport)  SQRESULT gonutsmodule_shutdown(HSQUIRRELVM v,HSQMODULEINFO info) ; //deinitialize module. 
#else
CDECL __attribute__((visibility("default")))  SQRESULT gonutsmodule_shutdown(HSQUIRRELVM v,HSQMODULEINFO info) ; //deinitialize module.  
#endif

//Module authentication 
//called every now and then, mainly right before initialization via gonutsmodule_init
//You need to provide a certificate (chain) by returning a pointer to a (static!) ModCert structure here.
//if you don't have one, you can return NULL to indicate your module is neither signed nor licensed
//The host may deny loading of the module depending on the certificate data, in which case sqmodule_init usually won't be even called
//This function hence MUST NOT initialize ANY dynamic objects.
//The passed in SQUserPointer is reserved for future use, expect it to be NULL
//(Note: certificates can be issued using the gonuts toolchain, depending on your license and any CA certs you have)
//


#ifdef WIN32
CDECL __declspec(dllexport)  const SQModCert * gonutsmodule_auth(SQUserPointer unused) ; //authentication call
#else
CDECL __attribute__((visibility("default")))  const SQModCert * gonutsmodule_auth(SQUserPointer unused) ; //authentication call
#endif


#endif //_MODULE_NO_PROTOTYPES


	//recommended: use this inline function in your sqmodule_init function to check if your module is compatible:
	//pass the API, your desired minimum API version (or 0 for automatic check for major squirrel versions) and
    //wether your module needs the Squirrel Compiler and if it is threadsafe
	//However, if you use the DenAPI, it will make use of this inline function anyways.
	//Important note: It does NOT check for presence of the garbage collector, because a mismatch is no compatibility issue as long as you strictly use the API.
	inline SQRESULT CheckSQCompatibility(HSQAPI hapi,SQInteger MinimumAPIVersion,SQBool bneedcompiler,SQBool threadsafemodule) {
		
		if (!hapi) return SQ_ERROR ;
		SQmoduleAPIhead *pHead=&(hapi->sqconfig) ;
		if	(	(pHead->PtrSize!=sizeof(void*))
			 || (pHead->SQIntegerSize!=sizeof(SQInteger))
			 || (pHead->SQFloatSize!=sizeof(SQFloat))
#ifdef SQUNICODE
			 || (pHead->Unicode==0)
#else
			 || (pHead->Unicode==1)
#endif
#ifdef NO_COMPILER
			 || ( !pHead->CompilerAvailable && bneedcompiler)
#endif
			 || (hapi->APIVersion<MinimumAPIVersion) 
			 || ((!MinimumAPIVersion) && (hapi->APIVersion<HSQAPI_VERSION))
			 || ((!threadsafemodule) && (pHead->MultiThreadedEnvironment))
			)
		{
				return SQ_ERROR; 
		}
		const SQInteger vers=SQUIRREL_VERSION_NUMBER;
		if ((hapi->getversion()/100)!=vers/100) return SQ_ERROR;  //just checks for major squirrel versions. Do your own check if necessary!
		return SQ_OK ;
	}
#ifdef __cplusplus
} /*extern "C"*/
#endif


#endif /*_SQ_MODULE_H_*/
