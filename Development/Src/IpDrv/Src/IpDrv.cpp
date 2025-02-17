/*=============================================================================
	IpDrv.cpp: Unreal TCP/IP driver definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnIpDrv.h"
#include "HTTPDownload.h"

#if WITH_UE3_NETWORKING

/*-----------------------------------------------------------------------------
	Declarations.
-----------------------------------------------------------------------------*/

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName IPDRV_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "IpDrvClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Import natives
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "IpDrvClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NATIVES_ONLY
#undef NAMES_ONLY

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsIpDrv( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_IPDRV
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesIpDrv()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
	#define AUTOGENERATE_NAME(name) IPDRV_##name=FName(TEXT(#name));
	#include "IpDrvClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}

DECLARE_STAT_NOTIFY_PROVIDER_FACTORY(FStatNotifyProvider_UDPFactory,
	FStatNotifyProvider_UDP,UdpProvider);



/*-----------------------------------------------------------------------------
	Global variables (maybe move this to DebugCommunication.cpp)
-----------------------------------------------------------------------------*/

#if !SHIPPING_PC_GAME
/** Network communication between UE3 executable and the "Unreal Console" tool.								*/
FDebugServer*	GDebugChannel						= NULL;
#endif

#endif	//#if WITH_UE3_NETWORKING
