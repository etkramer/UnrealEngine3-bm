/*=============================================================================
	NxForceFieldTornado.cpp
	Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineForceFieldClasses.h"
#include "ForceFunctionTornadoAngular.h"

#if WITH_NOVODEX
#include "UnNovodexSupport.h"
#endif // WITH_NOVODEX

IMPLEMENT_CLASS(ANxForceFieldTornado);

FPointer ANxForceFieldTornado::DefineForceFieldShapeDesc()
{
#if WITH_NOVODEX
	if (Shape == NULL)
	{
		return NULL;
	}

	NxForceFieldShapeDesc * desc = Shape->CreateNxDesc();

	// since NxForceFieldKernelTornado use y up logic, I must SetForceFieldPose and ReDefineFieldShapeDesc at the same time. 
	NxMat34 rot;
	rot.M.rotX(-NxPi/2);
	desc->pose.multiply(rot, desc->pose);

	return desc;
#else
	return NULL;
#endif
}

void ANxForceFieldTornado::SetForceFieldPose(FPointer ForceFieldDesc)
{
#if WITH_NOVODEX
	NxForceFieldDesc& ffDesc = *(NxForceFieldDesc*)ForceFieldDesc;

	// Rotate because SDK uses y-up and UE use z up.
	NxQuat rotActor(U2NQuaternion(Rotation.Quaternion()));
	NxMat33 rotCylinder;
	rotCylinder.rotX(NxPi/2);
	ffDesc.pose.M.multiply(rotActor, rotCylinder); 
	ffDesc.pose.t = U2NPosition(Location);
#endif
}

void ANxForceFieldTornado::DefineForceFunction(FPointer ForceFieldDesc)
{
#if WITH_NOVODEX
	NxForceFieldDesc& ffDesc = *(NxForceFieldDesc*)ForceFieldDesc;

	Kernel->setTornadoHeight(U2PScale*ForceHeight);
	Kernel->setRadius(U2PScale*ForceRadius);
	Kernel->setRadiusTop(U2PScale*ForceTopRadius);
	Kernel->setEscapeVelocitySq(U2PScale*U2PScale*EscapeVelocity*EscapeVelocity);
	Kernel->setRotationalStrength(RotationalStrength);
	Kernel->setRadialStrength(RadialStrength);
	Kernel->setBSpecialRadialForce(BSpecialRadialForceMode);
	Kernel->setLiftFallOffHeight(U2PScale*LiftFalloffHeight);
	Kernel->setLiftStrength(LiftStrength);
	Kernel->setSelfRotationStrength(SelfRotationStrength);

	ffDesc.kernel = Kernel;
	ffDesc.coordinates = NX_FFC_CYLINDRICAL;
#endif
}

void ANxForceFieldTornado::InitRBPhys()
{
#if WITH_NOVODEX
	assert(Kernel == NULL);
	Kernel = new NxForceFieldKernelTornadoAngular;

	Super::InitRBPhys();
#endif
}

void ANxForceFieldTornado::TermRBPhys(FRBPhysScene* Scene)
{
#if WITH_NOVODEX
	delete Kernel;
	Kernel = NULL;

	Super::TermRBPhys(Scene);
#endif
}

void ANxForceFieldTornado::PostLoad()
{
	Super::PostLoad();

	if (Shape)
	{
		DrawComponent = Shape->eventGetDrawComponent();
		Shape->eventFillByCylinder(ForceRadius, ForceTopRadius, ForceHeight, HeightOffset);
		// attach after the size is ready.
		if (DrawComponent)
		{
		//temptest.	AttachComponent(DrawComponent);
			Components.AddItem(DrawComponent);//temptest. I do not know a better way to initialize the DrawComponent.
		}
	}
}

void ANxForceFieldTornado::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);


	if (PropertyThatChanged)
	{
		if (appStrstr(*(PropertyThatChanged->GetName()), TEXT("Shape")) != NULL)
		{
			// original Shape object is gone
			DetachComponent(DrawComponent);
			DrawComponent = NULL;

			// if new Shape object is available
			if (Shape && Shape->eventGetDrawComponent())
			{
				DrawComponent = Shape->eventGetDrawComponent();
				Shape->eventFillByCylinder(ForceRadius, ForceTopRadius, ForceHeight, HeightOffset);
				// attach after the size is ready.
				AttachComponent(DrawComponent);
			}
		}
		else if (Shape && Shape->eventGetDrawComponent())
		{
			FComponentReattachContext ReattachContext(Shape->eventGetDrawComponent());
			if ((appStrstr(*(PropertyThatChanged->GetName()), TEXT("ForceRadius")) != NULL)
			||(appStrstr(*(PropertyThatChanged->GetName()), TEXT("ForceTopRadius")) != NULL)
			||(appStrstr(*(PropertyThatChanged->GetName()), TEXT("ForceHeight")) != NULL)
			||(appStrstr(*(PropertyThatChanged->GetName()), TEXT("HeightOffset")) != NULL))
			{
				Shape->eventFillByCylinder(ForceRadius, ForceTopRadius, ForceHeight, HeightOffset);
			}

		}
	}
}

void ANxForceFieldTornado::EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * 500.0f;

	if(!ModifiedScale.IsUniform())
	{
		const FLOAT XYMultiplier = ( ModifiedScale.X > 0.0f || ModifiedScale.Y > 0.0f ) ? 1.0f : -1.0f;
		const FLOAT ZMultiplier = ( ModifiedScale.Z > 0.0f ) ? 1.0f : -1.0f;

		ForceRadius += XYMultiplier * ModifiedScale.Size2D();
		ForceTopRadius += XYMultiplier * ModifiedScale.Size2D();
		ForceHeight += ZMultiplier * Abs(ModifiedScale.Z);
	}
	else
	{
		const FLOAT Multiplier = ( ModifiedScale.X > 0.0f || ModifiedScale.Y > 0.0f || ModifiedScale.Z > 0.0f ) ? 1.0f : -1.0f;

		ForceRadius += Multiplier * ModifiedScale.Size();
		ForceTopRadius += Multiplier * ModifiedScale.Size();
		ForceHeight += Multiplier * ModifiedScale.Size();
	}

	ForceRadius = Max( 0.f, ForceRadius );
	ForceTopRadius = Max(0.0f, ForceTopRadius);
	ForceHeight = Max( 0.0f, ForceHeight );

	if (Shape && Shape->eventGetDrawComponent())
	{
		FComponentReattachContext ReattachContext(Shape->eventGetDrawComponent());
		Shape->eventFillByCylinder(ForceRadius, ForceTopRadius, ForceHeight, HeightOffset);
	}
}
