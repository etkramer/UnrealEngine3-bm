//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#ifndef _INC_ENGINE

#include "Engine.h"
#include "EngineAIClasses.h"
#include "UnPhysicalMaterial.h" //was: #include "EnginePhysicsClasses.h", but is included in PhysMat
#include "UnTerrain.h"
#include "EngineSequenceClasses.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineParticleClasses.h"
#include "EngineUIPrivateClasses.h"
#include "VoiceInterface.h"
#include "UnNet.h"
#include "DemoRecording.h"

#endif // _INC_ENGINE

#include "GameFrameworkClasses.h"

#include "UTGameClasses.h"


#ifndef _UT_INTRINSIC_CLASSES
#define _UT_INTRINSIC_CLASSES

BEGIN_COMMANDLET(UTLevelCheck,UTGame)
	void StaticInitialize()
	{
		IsEditor = FALSE;
	}
END_COMMANDLET

BEGIN_COMMANDLET(UTReplaceActor,UTGame)
	void StaticInitialize()
	{
		IsEditor = TRUE;
	}
END_COMMANDLET

FLOAT static CalcRequiredAngle(FVector Start, FVector Hit, INT Length)
{
	FLOAT Dist = (Hit - Start).Size();
	FLOAT Angle = 90 - ( appAcos( Dist / FLOAT(Length) ) * 57.2957795);

	return Clamp<FLOAT>(Angle,0,90);
}

inline UBOOL operator !=(const FTakeHitInfo& A, const FTakeHitInfo& B)
{
	return (A.Damage != B.Damage || A.HitLocation != B.HitLocation || A.Momentum != B.Momentum || A.DamageType != B.DamageType || A.HitBone != B.HitBone);
}

class UUTDemoRecDriver : public UDemoRecDriver
{
	DECLARE_CLASS(UUTDemoRecDriver,UDemoRecDriver,CLASS_Transient|CLASS_Config|CLASS_Intrinsic,UTGame)

	virtual UBOOL InitListen(FNetworkNotify* InNotify, FURL& ConnectURL, FString& Error);
};

#endif
