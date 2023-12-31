//convenience macros, include them separately when needed
//IMPORTANT: Include AFTER including squirrel headers or DenAPI headers or things screw up
//hiding the indirection makes things easier not only when moving existing code to a module 

#define sq_setdebughook SQAPI->setdebughook
#define sq_stackinfos SQAPI->stackinfos
#define sq_stackinfos SQAPI->stackinfos
#define sq_malloc SQAPI->malloc
#define sq_realloc SQAPI->realloc
#define sq_free SQAPI->free
#define sq_readclosure SQAPI->readclosure
#define sq_writeclosure SQAPI->writeclosure
#define sq_collectgarbage SQAPI->collectgarbage
#define sq_getobjtypetag SQAPI->getobjtypetag
#define sq_objtofloat SQAPI->objtofloat
#define sq_objtointeger SQAPI->objtointeger
#define sq_objtobool SQAPI->objtobool
#define sq_objtostring SQAPI->objtostring
#define sq_resetobject SQAPI->resetobject
#define sq_release SQAPI->release
#define sq_addref SQAPI->addref
#define sq_pushobject SQAPI->pushobject
#define sq_getstackobj SQAPI->getstackobj
#define sq_getlasterror SQAPI->getlasterror
#define sq_reseterror SQAPI->reseterror
#define sq_throwerror SQAPI->throwerror
#define sq_getfreevariable SQAPI->getfreevariable
#define sq_getlocal SQAPI->getlocal
#define sq_resume SQAPI->resume
#define sq_call SQAPI->call
#define sq_clear SQAPI->clear
#define sq_getweakrefval SQAPI->getweakrefval
#define sq_next SQAPI->next
#define sq_setfreevariable SQAPI->setfreevariable
#define sq_clone SQAPI->clone
#define sq_getdelegate SQAPI->getdelegate
#define sq_setdelegate SQAPI->setdelegate
#define sq_arrayinsert SQAPI->arrayinsert
#define sq_arrayremove SQAPI->arrayremove
#define sq_arrayreverse SQAPI->arrayreverse
#define sq_arrayresize SQAPI->arrayresize
#define sq_arraypop SQAPI->arraypop
#define sq_arrayappend SQAPI->arrayappend
#define sq_rawdeleteslot SQAPI->rawdeleteslot
#define sq_rawset SQAPI->rawset
#define sq_rawget SQAPI->rawget
#define sq_get SQAPI->get
#define sq_set SQAPI->set
#define sq_deleteslot SQAPI->deleteslot
#define sq_newslot SQAPI->newslot
#define sq_setconsttable SQAPI->setconsttable
#define sq_setroottable SQAPI->setroottable
#define sq_pushconsttable SQAPI->pushconsttable
#define sq_pushregistrytable SQAPI->pushregistrytable
#define sq_pushroottable SQAPI->pushroottable
#define sq_getdefaultdelegate SQAPI->getdefaultdelegate
#define sq_weakref SQAPI->weakref
#define sq_getclass SQAPI->getclass
#define sq_getattributes SQAPI->getattributes
#define sq_setattributes SQAPI->setattributes
#define sq_createinstance SQAPI->createinstance
#define sq_newclass SQAPI->newclass
#define sq_setclassudsize SQAPI->setclassudsize
#define sq_getinstanceup SQAPI->getinstanceup
#define sq_getinstanceup310 SQAPI->getinstanceup310
#define sq_setinstanceup SQAPI->setinstanceup
#define sq_setnativeclosurename SQAPI->setnativeclosurename
#define sq_getclosureinfo SQAPI->getclosureinfo
#define sq_getscratchpad SQAPI->getscratchpad
#define sq_setreleasehook SQAPI->setreleasehook
#define sq_gettypetag SQAPI->gettypetag
#define sq_settypetag SQAPI->settypetag
#define sq_getuserdata SQAPI->getuserdata
#define sq_getuserpointer SQAPI->getuserpointer
#define sq_getthread SQAPI->getthread
#define sq_getbool SQAPI->getbool
#define sq_getfloat SQAPI->getfloat
#define sq_getinteger SQAPI->getinteger
#define sq_getstring SQAPI->getstring
#define sq_tobool SQAPI->tobool
#define sq_tostring SQAPI->tostring
#define sq_instanceof SQAPI->instanceof
#define sq_getbase SQAPI->getbase
#define sq_getsize SQAPI->getsize
#define sq_gettype SQAPI->gettype
#define sq_pushnull SQAPI->pushnull
#define sq_pushuserpointer SQAPI->pushuserpointer
#define sq_pushbool SQAPI->pushbool
#define sq_pushinteger SQAPI->pushinteger
#define sq_pushfloat SQAPI->pushfloat
#define sq_pushstring SQAPI->pushstring
#define sq_bindenv SQAPI->bindenv
#define sq_setparamscheck SQAPI->setparamscheck
#define sq_newclosure SQAPI->newclosure
#define sq_newarray SQAPI->newarray
#define sq_newtable SQAPI->newtable
#define sq_newuserdata SQAPI->newuserdata
#define sq_move SQAPI->move
#define sq_cmp SQAPI->cmp
#define sq_reservestack SQAPI->reservestack
#define sq_settop SQAPI->settop
#define sq_gettop SQAPI->gettop
#define sq_remove SQAPI->remove
#define sq_poptop SQAPI->poptop
#define sq_pop SQAPI->pop
#define sq_push SQAPI->push
#define sq_setcompilererrorhandler SQAPI->setcompilererrorhandler
#define sq_notifyallexceptions SQAPI->notifyallexceptions
#define sq_enabledebuginfo SQAPI->enabledebuginfo
#define sq_compilebuffer SQAPI->compilebuffer
#define sq_compile SQAPI->compile
#define sq_getvmstate SQAPI->getvmstate
#define sq_wakeupvm SQAPI->wakeupvm
#define sq_suspendvm SQAPI->suspendvm
#define sq_getprintfunc SQAPI->getprintfunc
#define sq_setprintfunc SQAPI->setprintfunc
#define sq_getforeignptr you_should_know_what_you_are_doing() /*do not use in combination with gonuts*/
#define sq_setforeignptr you_should_know_what_you_are_doing() /*crashes imminent with gonuts features*/
#define sq_close you_should_know_what_you_are_doing() /*do not use in combination with gonuts*/
#define sq_seterrorhandler SQAPI->seterrorhandler
#define sq_newthread SQAPI->newthread
#define sq_open you_should_know_what_you_are_doing() /*do not use in combination with gonuts*/
#define sq_getversion SQAPI->getversion


//#error bla



//...to be continued



