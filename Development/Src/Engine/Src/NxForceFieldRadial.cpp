/*=============================================================================
	NxForceFieldRadial.cpp
	Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 


#include "EnginePrivate.h"
#include "EngineForceFieldClasses.h"
#include "ForceFunctionRadial.h"

#if WITH_NOVODEX
#include "UnNovodexSupport.h"
#endif // WITH_NOVODEX

IMPLEMENT_CLASS(ANxForceFieldRadial);

FPointer ANxForceFieldRadial::DefineForceFieldShapeDesc()
{
#if WITH_NOVODEX
	return Shape ? Shape->CreateNxDesc() : NULL;
#else
	return NULL;
#endif
}

void ANxForceFieldRadial::DefineForceFunction(FPointer ForceFieldDesc)
{
#if WITH_NOVODEX
	NxForceFieldDesc& ffDesc = *(NxForceFieldDesc*)ForceFieldDesc;

	Kernel->setRadialStrength(ForceStrength);
	Kernel->setRadius(U2PScale*ForceRadius);
	Kernel->setRadiusRecip(1.0f/(U2PScale*ForceRadius));
	Kernel->setSelfRotationStrength(SelfRotationStrength);
	Kernel->setBLinearFalloff(ForceFalloff==RIF_Linear);

	ffDesc.kernel = Kernel;
	ffDesc.coordinates = NX_FFC_SPHERICAL;
#endif
}

void ANxForceFieldRadial::InitRBPhys()
{
#if WITH_NOVODEX
	assert(Kernel == NULL);
	Kernel = new NxForceFieldKernelRadial;

	Super::InitRBPhys();
#endif
}

void ANxForceFieldRadial::TermRBPhys(FRBPhysScene* Scene)
{
#if WITH_NOVODEX
	delete Kernel;
	Kernel = NULL;

	Super::TermRBPhys(Scene);
#endif
}

void ANxForceFieldRadial::PostLoad()
{
	Super::PostLoad();

	if (Shape)
	{
		DrawComponent = Shape->eventGetDrawComponent();
		Shape->eventFillBySphere(ForceRadius);
		// attach after the size is ready.
		//temptest.	AttachComponent(DrawComponent);
		if (DrawComponent)
		{
			Components.AddItem(DrawComponent);//temptest. I do not know a better way to initialize the DrawComponent.
		}
	}
}

void ANxForceFieldRadial::PostEditChange(UProperty* PropertyThatChanged)
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
				Shape->eventFillBySphere(ForceRadius);
				// attach after the size is ready.
				AttachComponent(DrawComponent);
			}
		}
		else if (Shape && Shape->eventGetDrawComponent())
		{
			FComponentReattachContext ReattachContext(Shape->eventGetDrawComponent());
			if (appStrstr(*(PropertyThatChanged->GetName()), TEXT("ForceRadius")) != NULL)
			{
				Shape->eventFillBySphere(ForceRadius);
			}
		}
	}
}

void ANxForceFieldRadial::EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * 500.0f;

	const FLOAT Multiplier = ( ModifiedScale.X > 0.0f || ModifiedScale.Y > 0.0f || ModifiedScale.Z > 0.0f ) ? 1.0f : -1.0f;
	ForceRadius += Multiplier * ModifiedScale.Size();
	ForceRadius = Max( 0.f, ForceRadius );

	if (Shape && Shape->eventGetDrawComponent())
	{
		FComponentReattachContext ReattachContext(Shape->eventGetDrawComponent());
		Shape->eventFillBySphere(ForceRadius);
	}
}
