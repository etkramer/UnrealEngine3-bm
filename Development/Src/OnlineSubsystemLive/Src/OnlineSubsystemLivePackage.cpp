/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#if XBOX || WITH_PANORAMA

#include "OnlineSubsystemLive.h"

#if WITH_UE3_NETWORKING

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName ONLINESUBSYSTEMLIVE_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "OnlineSubsystemLiveClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "OnlineSubsystemLiveClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NATIVES_ONLY
#undef NAMES_ONLY

#endif	//#if WITH_UE3_NETWORKING

#else	//#if XBOX || WITH_PANORAMA
typedef int INT;
#endif	//#if XBOX || WITH_PANORAMA

#if WITH_UE3_NETWORKING

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsOnlineSubsystemLive( INT& Lookup )
{
#if XBOX || WITH_PANORAMA
	AUTO_INITIALIZE_REGISTRANTS_ONLINESUBSYSTEMLIVE;
	extern void RegisterLiveNetClasses(void);
	RegisterLiveNetClasses();
#else
	if (Lookup)
	{
	}
#endif
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesOnlineSubsystemLive()
{
#if XBOX || WITH_PANORAMA
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
    #define AUTOGENERATE_NAME(name) ONLINESUBSYSTEMLIVE_##name = FName(TEXT(#name));
	#include "OnlineSubsystemLiveClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
#endif
}

#endif	//#if WITH_UE3_NETWORKING
