//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================
#include "UTGame.h"
#include "UTGameUIClasses.h"
#include "UTGameVehicleClasses.h"
#include "EngineMaterialClasses.h"

IMPLEMENT_CLASS(UUTUIScene_Hud);
IMPLEMENT_CLASS(UUTUIScene_Scoreboard);
IMPLEMENT_CLASS(UUTUIScene_MOTD);
IMPLEMENT_CLASS(UUTUIScene_MapVote);
IMPLEMENT_CLASS(UUTUI_HudWidget);

/*
IMPLEMENT_CLASS(UUIState_UTObjStates);
	IMPLEMENT_CLASS(UUIState_UTObjActive);
	IMPLEMENT_CLASS(UUIState_UTObjBuilding);
	IMPLEMENT_CLASS(UUIState_UTObjCritical);
	IMPLEMENT_CLASS(UUIState_UTObjInactive);
	IMPLEMENT_CLASS(UUIState_UTObjNeutral);
	IMPLEMENT_CLASS(UUIState_UTObjProtected);
	IMPLEMENT_CLASS(UUIState_UTObjUnderAttack);
*/

/*=========================================================================================
	THis is the acutal hud actor
========================================================================================= */


/**
 * Look at all Transient properties of the hud that are children of HudWidget and
 * auto-link them to objects in the scene
 */
void AUTHUD::LinkToHudScene()
{
}

void AUTHUD::DisplayWeaponBar()
{
	INT i, SelectedWeapon, LastWeapIndex, PrevWeapIndex, NextWeapIndex, FirstWeaponIndex;
	FLOAT TotalOffsetX = 0.f, OffsetX, OffsetY, BoxOffsetSize, OffsetSizeX, OffsetSizeY, DesiredWeaponScale[10], Delta, MaxWidth;
	AUTWeapon *W = NULL;
	AUTVehicle *V = NULL;
	FLinearColor FadedAmmoBarColor;
	FLOAT SelectedAmmoBarX = 0.f, SelectedAmmoBarY = 0.f, AlphaScale, AmmoCountScale;
	FRotator r;
	r.Roll = 0;
	r.Yaw = 0;
	r.Pitch = 0;

	// never show weapon bar in split screen
	if ( (PawnOwner == NULL) || bIsSplitScreen )
	{
		return;
	}

	FirstWeaponIndex = WorldInfo->bUseConsoleInput ? 1 : 0;
	if ( !bShowQuickPick && (PawnOwner->InvManager != NULL) && (Cast<AUTWeapon>(PawnOwner->InvManager->PendingWeapon) != NULL)
		&& (Cast<AUTWeapon>(PawnOwner->InvManager->PendingWeapon)->InventoryGroup >FirstWeaponIndex) )
	{
		LastWeaponBarDrawnTime = WorldInfo->TimeSeconds;
	}

	if ( (PawnOwner->Weapon == NULL) || (PawnOwner->InvManager == NULL) || (Cast<AUTVehicle>(PawnOwner) != NULL) )
	{
		V = Cast<AUTVehicle>(PawnOwner);
		if ( V != NULL )
		{
			if ( V->bHasWeaponBar )
			{
				V->eventDisplayWeaponBar(Canvas, this);
			}
			else if ( PawnOwner->Weapon != LastSelectedWeapon )
			{
				LastSelectedWeapon = PawnOwner->Weapon;
				PlayerOwner->eventReceiveLocalizedMessage( WeaponSwitchMessage,0,NULL,NULL, LastSelectedWeapon );
			}
		}
		return;
	}
	if ( bOnlyShowWeaponBarIfChanging )
	{
		if ( WorldInfo->TimeSeconds - LastWeaponBarDrawnTime > 1.0 )
		{
			return;
		}
		AlphaScale = Clamp<FLOAT>(1.0 - 3.0 * (WorldInfo->TimeSeconds - LastWeaponBarDrawnTime - 0.333), 0.0, 1.0);
	}
	else
	{
		AlphaScale = 1.0;
	}

	// FIXME manage weapon list based on weapon add/discard from inventorymanager
	for ( i=0; i<10; i++ )
	{
		WeaponList[i] = NULL;
	}
	i = 0;
	AInventory *Inv = PawnOwner->InvManager->InventoryChain;
	while (Inv != NULL)
	{
		AUTWeapon *W = Cast<AUTWeapon>(Inv);
		if (W != NULL && W->InventoryGroup < 11 && W->InventoryGroup > 0)
		{
			WeaponList[W->InventoryGroup-1] = W;
		}
		Inv = Inv->Inventory;
	}

	SelectedWeapon = (PawnOwner->InvManager->PendingWeapon != NULL) ? Cast<AUTWeapon>(PawnOwner->InvManager->PendingWeapon)->InventoryGroup-1 : Cast<AUTWeapon>(PawnOwner->Weapon)->InventoryGroup-1;
	Delta = WeaponScaleSpeed * (WorldInfo->TimeSeconds - LastHUDUpdateTime);
	BoxOffsetSize = HUDScaleX * WeaponBarScale * WeaponBoxWidth;
	PrevWeapIndex = -1;
	NextWeapIndex = -1;
	LastWeapIndex = -1;

	if ( (PawnOwner->InvManager->PendingWeapon != NULL) && (PawnOwner->InvManager->PendingWeapon != LastSelectedWeapon) )
	{
		LastSelectedWeapon = PawnOwner->InvManager->PendingWeapon;

		// clear any pickup messages for this weapon
		for ( i=0; i<8; i++ )
		{
			if( LocalMessages[i].Message == NULL )
			{
				break;
			}
			if( LocalMessages[i].OptionalObject == LastSelectedWeapon->GetClass() )
			{
				LocalMessages[i].EndOfLife = WorldInfo->TimeSeconds - 1.0;
				break;
			}
		}

		PlayerOwner->eventReceiveLocalizedMessage( WeaponSwitchMessage,0,NULL,NULL, LastSelectedWeapon );
	}

	// calculate offsets
	for ( i=FirstWeaponIndex; i<10; i++ )
	{
		if ( WeaponList[i] != NULL )
		{
			// optimization if needed - cache desiredweaponscale[] when pending weapon changes
			if ( SelectedWeapon == i )
			{
				PrevWeapIndex = LastWeapIndex;
				if ( BouncedWeapon == i )
				{
					DesiredWeaponScale[i] = SelectedWeaponScale;
				}
				else
				{
					DesiredWeaponScale[i] = BounceWeaponScale;
					if ( CurrentWeaponScale[i] >= DesiredWeaponScale[i] )
					{
						BouncedWeapon = i;
					}
				}
			}
			else
			{
				if ( LastWeapIndex == SelectedWeapon )
				{
					NextWeapIndex = i;
				}
				DesiredWeaponScale[i] = 1.0;
			}
			if ( CurrentWeaponScale[i] != DesiredWeaponScale[i] )
			{
				if ( DesiredWeaponScale[i] > CurrentWeaponScale[i] )
				{
					CurrentWeaponScale[i] = Min<FLOAT>(CurrentWeaponScale[i]+Delta,DesiredWeaponScale[i]);
				}
				else
				{
					CurrentWeaponScale[i] = Max<FLOAT>(CurrentWeaponScale[i]-Delta,DesiredWeaponScale[i]);
				}
			}
			TotalOffsetX += CurrentWeaponScale[i] * BoxOffsetSize;
			LastWeapIndex = i;
		}
		else if ( bShowOnlyAvailableWeapons )
		{
			CurrentWeaponScale[i] = 0;
		}
		else
		{
			// draw empty background
			CurrentWeaponScale[i] = 1.0;
			TotalOffsetX += BoxOffsetSize;
		}
	}
	if ( !bShowOnlyAvailableWeapons )
	{
		PrevWeapIndex = SelectedWeapon - 1;
		NextWeapIndex = SelectedWeapon + 1;
	}

	OffsetX = HUDScaleX * WeaponBarXOffset + 0.5 * (Canvas->ClipX - TotalOffsetX);
	OffsetY = Canvas->ClipY - HUDScaleY * WeaponBarY;

	// @TODO - manually reorganize canvas calls, or can this be automated?
	// draw weapon boxes
	Canvas->SetDrawColor(255,255,255,255);
	OffsetSizeX = HUDScaleX * WeaponBarScale * 96 * SelectedBoxScale;
	OffsetSizeY = HUDScaleY * WeaponBarScale * 64 * SelectedBoxScale;
	FadedAmmoBarColor = AmmoBarColor;
	FadedAmmoBarColor.A *= AlphaScale;
	for ( i=FirstWeaponIndex; i<10; i++ )
	{
		if ( WeaponList[i] != NULL )
		{
			Canvas->SetPos(OffsetX, OffsetY - OffsetSizeY*CurrentWeaponScale[i]);
			if ( SelectedWeapon == i )
			{
				//Current slot overlay
				TeamHUDColor.A = SelectedWeaponAlpha * AlphaScale;
				Canvas->DrawTile(AltHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY, 530, 248, 69, 49, TeamHUDColor);

				Canvas->SetPos(OffsetX, OffsetY - OffsetSizeY*CurrentWeaponScale[i]);
				Canvas->DrawTile(AltHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY, 459, 148, 69, 49, TeamHUDColor);

				Canvas->SetPos(OffsetX, OffsetY - OffsetSizeY*CurrentWeaponScale[i]);
				Canvas->DrawTile(AltHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY, 459, 248, 69, 49, TeamHUDColor);

				// draw ammo bar ticks for selected weapon
				SelectedAmmoBarX = HUDScaleX * (SelectedWeaponAmmoOffsetX - WeaponBarXOffset) + OffsetX;
				SelectedAmmoBarY = Canvas->ClipY - HUDScaleY * (WeaponBarY + CurrentWeaponScale[i]*WeaponAmmoOffsetY);
				Canvas->SetPos(SelectedAmmoBarX, SelectedAmmoBarY);
				MaxWidth = CurrentWeaponScale[i]*HUDScaleY * WeaponBarScale * WeaponAmmoLength;

				Canvas->DrawTileStretched(AltHudTexture, Canvas->CurX, Canvas->CurY, MaxWidth, CurrentWeaponScale[i]*HUDScaleY*WeaponBarScale*WeaponAmmoThickness, 407, 479, Min<FLOAT>(118, MaxWidth), 16, FadedAmmoBarColor);
			}
			else
			{
				TeamHUDColor.A = OffWeaponAlpha * AlphaScale;
				Canvas->DrawTile(AltHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY, 459, 148, 69, 49, TeamHUDColor);

				// draw slot overlay?
				if ( i == PrevWeapIndex )
				{
					Canvas->SetPos(OffsetX, OffsetY - OffsetSizeY*CurrentWeaponScale[i]);
					Canvas->DrawTile(AltHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY, 530, 97, 69, 49, TeamHUDColor);
				}
				else if ( i == NextWeapIndex )
				{
					Canvas->SetPos(OffsetX, OffsetY - OffsetSizeY*CurrentWeaponScale[i]);
					Canvas->DrawTile(AltHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY, 530, 148, 69, 49, TeamHUDColor);
				}
			}
			OffsetX += CurrentWeaponScale[i] * BoxOffsetSize;
		}
		else if ( !bShowOnlyAvailableWeapons )
		{
			TeamHUDColor.A = EmptyWeaponAlpha * AlphaScale;
			Canvas->SetPos(OffsetX, OffsetY - OffsetSizeY*CurrentWeaponScale[i]);
			Canvas->DrawTile(AltHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY, 459, 97, 69, 49, TeamHUDColor);

			// draw slot overlay?
			if ( i == PrevWeapIndex )
			{
				Canvas->SetPos(OffsetX, OffsetY - OffsetSizeY*CurrentWeaponScale[i]);
				Canvas->DrawTile(AltHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY, 530, 198, 69, 49, TeamHUDColor);
			}
			else if ( i == NextWeapIndex )
			{
				Canvas->SetPos(OffsetX, OffsetY - OffsetSizeY*CurrentWeaponScale[i]);
				Canvas->DrawTile(AltHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY, 459, 198, 69, 49, TeamHUDColor);
			}
			OffsetX += BoxOffsetSize;
		}
	}

	// draw weapon ammo bars
	// Ammo Bar:  273,494 12,13 (The ammo bar is meant to be stretched)
	Canvas->SetDrawColor(255,255,255,255);
	OffsetX = HUDScaleX * WeaponAmmoOffsetX + 0.5 * (Canvas->ClipX - TotalOffsetX);
	OffsetSizeY = HUDScaleY * WeaponBarScale * WeaponAmmoThickness;
	FadedAmmoBarColor = AmmoBarColor;
	FadedAmmoBarColor.A *= AlphaScale;
	for ( i=FirstWeaponIndex; i<10; i++ )
	{
		if ( (WeaponList[i] != NULL) && (WeaponList[i]->AmmoCount > 0) )
		{
			if ( SelectedWeapon == i )
			{
				Canvas->SetPos(SelectedAmmoBarX - 0.2*HUDScaleY * WeaponBarScale * WeaponAmmoLength*CurrentWeaponScale[i], SelectedAmmoBarY);
			}
			else
			{
				Canvas->SetPos(OffsetX, Canvas->ClipY - HUDScaleY * (WeaponBarY + CurrentWeaponScale[i]*WeaponAmmoOffsetY));
			}
			AmmoCountScale = 0.3 + Min<FLOAT>(1.0,((FLOAT)WeaponList[i]->AmmoCount)/((FLOAT)WeaponList[i]->MaxAmmoCount));
			Canvas->DrawTileStretched(AltHudTexture, Canvas->CurX, Canvas->CurY, HUDScaleY * WeaponBarScale * WeaponAmmoLength*CurrentWeaponScale[i]*AmmoCountScale, CurrentWeaponScale[i]*OffsetSizeY, 273, 494,12,13, FadedAmmoBarColor);
		}
		OffsetX += CurrentWeaponScale[i] * BoxOffsetSize;
	}

	// draw weapon numbers
	if ( !bNoWeaponNumbers )
	{
		OffsetX = HUDScaleX * (WeaponAmmoOffsetX + WeaponXOffset) * 0.5 + 0.5 * (Canvas->ClipX - TotalOffsetX);
		OffsetY = Canvas->ClipY - HUDScaleY * (WeaponBarY + WeaponYOffset);
		Canvas->SetDrawColor(255,255,255,255);
		Canvas->DrawColor.A = static_cast<BYTE>(255.0 * AlphaScale);
		Canvas->Font = HudFonts(0);
		for ( i=0; i<10; i++ )
		{
			if ( WeaponList[i] != NULL )
			{
				Canvas->SetPos(OffsetX, OffsetY - OffsetSizeY*CurrentWeaponScale[i]);
				FString IndexText = FString::Printf(TEXT("%d"),((i+1)%10));
				Canvas->DrawText(IndexText);
				OffsetX += CurrentWeaponScale[i] * BoxOffsetSize;
			}
			else if ( !bShowOnlyAvailableWeapons )
			{
				OffsetX += BoxOffsetSize;
			}
		}
	}

	// draw weapon icons
	OffsetX = HUDScaleX * WeaponXOffset + 0.5 * (Canvas->ClipX - TotalOffsetX);
	OffsetY = Canvas->ClipY - HUDScaleY * (WeaponBarY + WeaponYOffset);
	OffsetSizeX = HUDScaleX * WeaponBarScale * 100;
	OffsetSizeY = HUDScaleY * WeaponBarScale * WeaponYScale;
	Canvas->SetDrawColor(255,255,255,255);
	Canvas->DrawColor.A = static_cast<BYTE>(255.0 * AlphaScale);

	r.Yaw=2048;

	for ( i=FirstWeaponIndex; i<10; i++ )
	{
		if ( WeaponList[i] != NULL )
		{
			OffsetSizeX = HUDScaleX * WeaponBarScale * 100;
			OffsetSizeY = OffsetSizeX * (WeaponList[i]->IconCoordinates.VL / WeaponList[i]->IconCoordinates.UL);

			Canvas->SetPos(OffsetX, OffsetY - 1.1f * OffsetSizeY * CurrentWeaponScale[i]);
			Canvas->DrawRotatedTile(IconHudTexture, r,
					CurrentWeaponScale[i]*OffsetSizeX, CurrentWeaponScale[i]*OffsetSizeY,
					WeaponList[i]->IconCoordinates.U, WeaponList[i]->IconCoordinates.V, WeaponList[i]->IconCoordinates.UL, WeaponList[i]->IconCoordinates.VL,1.0,1.0);
			OffsetX += CurrentWeaponScale[i] * BoxOffsetSize;
		}
		else if ( !bShowOnlyAvailableWeapons )
		{
			OffsetX += BoxOffsetSize;
		}
	}
}

void AUTHUD::DrawGlowText(const FString &Text, FLOAT X, FLOAT Y, FLOAT MaxHeightInPixels, FLOAT PulseTime, UBOOL bRightJustified)
{
	if ( Canvas )
	{
		INT XL,YL;
		FLOAT Scale = 1.f;

		Canvas->CurX = 0;
		Canvas->CurY = 0;

		// Size the string
		Canvas->WrappedStrLenf(GlowFonts[0], Scale, Scale, XL, YL, *Text);
XL += 5;
		FLOAT Width = FLOAT(XL);
		FLOAT Height = FLOAT(YL);

		// Calculate the scaling
		Scale = MaxHeightInPixels / YL;
		Width *= Scale;
		Height *= Scale;

		// Render the string
		X = bRightJustified ? X - Width : X;

		if ( WorldInfo->TimeSeconds - PulseTime < PulseDuration )
		{
			FLOAT PulsePct = (WorldInfo->TimeSeconds - PulseTime)/PulseDuration;
			FLOAT PulseChangeAt = 1.f - PulseSplit;
			FLOAT PulseScale = 1.f;

			if (PulsePct >= PulseChangeAt) // Growing
			{
				PulseScale = 1.f + PulseMultiplier * (1.f - (PulsePct - PulseChangeAt)/PulseSplit);
			}
			else
			{
				PulseScale = 1.f + PulseMultiplier * PulsePct/PulseChangeAt;
			}

			// Glow it up
			Canvas->CurX = X - 0.5f*Width*(PulseScale - 1.f);
			Canvas->CurY = Y - 0.5f*Height*(PulseScale - 1.f);
			Scale *= PulseScale;
			Canvas->WrappedPrint(true, XL, YL, GlowFonts[1], Scale, Scale, false, *Text);
		}
		else
		{
			// non-glowy
			Canvas->CurX = X;
			Canvas->CurY = Y;
			Canvas->WrappedPrint(true, XL, YL, GlowFonts[0], Scale, Scale, false, *Text);
		}
	}
}

void AUTHUD::TranslateBindToFont(const FString& InBindStr, UFont*& DrawFont, FString& OutBindStr)
{
	//Initialize the output
	DrawFont = NULL;
	OutBindStr = TEXT("");

	//Get the console iconfont/char sequence from the binding
	if (InBindStr != "")
	{
		//Search from the end for the start of the font </tag>
		INT RightIndex = InBindStr.InStr("<Fonts:", TRUE, TRUE);
		if (RightIndex >= 0)
		{
			//Search from the front for the end of the font <tag>
			INT FontIndex = InBindStr.InStr("<Fonts:", FALSE, TRUE);
			if (FontIndex >= 0)
			{
				INT LeftIndex = InBindStr.InStr(">", FALSE, TRUE, FontIndex + 7);
				if (LeftIndex >= 0)
				{
					OutBindStr = InBindStr.Mid(LeftIndex+1, RightIndex-LeftIndex-1);
					DrawFont = ConsoleIconFont;
				}
			}
		}
		else
		{
			DrawFont = HudFonts(2);
			OutBindStr = InBindStr;
		}
	}
}


/*=========================================================================================
  UTUIScene_Hud - Hud Scenes
 have a speical 1 on 1 releationship with a viewport
  ========================================================================================= */
/**
 * @Returns the UTHUD associated with this scene
 */

AUTHUD* UUTUIScene_Hud::GetPlayerHud()
{
	AUTPlayerController* PC = GetUTPlayerOwner();
	if ( PC )
	{
		return Cast<AUTHUD>( PC->myHUD );
	}
	return NULL;
}

/*=========================================================================================
  UTUI_HudWidget - UTUI_HudWidgets are collections of other widgets that share game logic.
  ========================================================================================= */

/** 
 * Cache a reference to the scene owner
 * @see UUIObject.Intiailize
 */

void UUTUI_HudWidget::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);
	UTHudSceneOwner = Cast<UUTUIScene_Hud>(inOwnerScene);
	OpacityTarget = Opacity;
}


void UUTUI_HudWidget::Tick_Widget(FLOAT DeltaTime)
{
	AGameReplicationInfo* GRI = GWorld->GetWorldInfo()->GRI;

	// Auto-Manage visibility

	if ( !bManualVisibility )
	{
		if ( GRI && GIsGame )
		{
			UBOOL bNewHidden;
			if ( GRI->bMatchHasBegun )
			{
				if ( GRI->bMatchIsOver )
				{
					bNewHidden = !bVisibleAfterMatch;
				}
				else
				{
					bNewHidden = !bVisibleDuringMatch;
				}
			}
			else
			{
				bNewHidden = !bVisibleBeforeMatch;
			}

			if ( eventPlayerOwnerIsSpectating() && !bVisibleWhileSpectating )
			{
				bNewHidden = TRUE;
			}

			const UBOOL bCurrentlyHidden = IsHidden();
			if ( bNewHidden != bCurrentlyHidden )
			{
				eventSetVisibility(!bNewHidden);
			}
		}
		else
		{
			eventSetVisibility(TRUE);
		}
	}

	Super::Tick_Widget(DeltaTime);
	eventWidgetTick(DeltaTime);

	// Update Any Animations

	UpdateAnimations(DeltaTime);
}

void UUTUI_HudWidget::UpdateAnimations(FLOAT DeltaTime)
{
	for (INT i=0; i<Animations.Num(); i++)
	{
		if ( Animations(i).bIsPlaying )
		{

			INT SeqIndex = Animations(i).SeqIndex;
			FWidgetAnimSequence Seq = Animations(i).Sequences(SeqIndex);
			
			FLOAT A = ( Seq.EndValue - Animations(i).Value ) ;
			FLOAT B = ( DeltaTime / Animations(i).Time );

			Animations(i).Value += A * B;
			Animations(i).Time -= DeltaTime;
			
			if ( Animations(i).Time < 0.0f )
			{
				Animations(i).bIsPlaying = false;
				Animations(i).Value = Seq.EndValue;

				if ( Animations(i).bNotifyWhenFinished )
				{
					UTSceneOwner->eventOnAnimationFinished(this, Animations(i).Tag, Seq.Tag);
				}
			}
			SetPosition( Animations(i).Value, (Animations(i).Property == EAPT_PositionLeft) ? UIFACE_Left : UIFACE_Top, EVALPOS_PercentageOwner);
		}
	}
}

UBOOL UUTUI_HudWidget::PlayAnimation(FName AnimTag, FName SeqTag, UBOOL bForceBeginning)
{
	UBOOL bResults = false;
	for (INT i=0; i<Animations.Num(); i++)
	{
		if ( Animations(i).Tag == AnimTag && Animations(i).Property != EAPT_None )
		{
			for ( INT SeqIndex = 0; SeqIndex < Animations(i).Sequences.Num(); SeqIndex++ )
			{
				FWidgetAnimSequence Seq = Animations(i).Sequences(SeqIndex);
				if (Seq.Tag == SeqTag)
				{
					Animations(i).SeqIndex = SeqIndex;
					if (!Animations(i).bIsPlaying || bForceBeginning)
					{
						Animations(i).Value = Seq.StartValue;
						Animations(i).Time = Seq.Rate;
					}
					else
					{
						FLOAT Pos = Abs(Seq.EndValue - Animations(i).Value);
						FLOAT Max = Abs(Seq.EndValue - Seq.StartValue);
						FLOAT Perc =  Pos / Max;
						Animations(i).Time = Seq.Rate * Perc;
					}
					
					Animations(i).bIsPlaying = true;
					bResults = true;
					break;
				}
			}
			break;
		}
	}
	return bResults;
}

void UUTUI_HudWidget::StopAnimation(FName AnimTag, UBOOL bForceEnd)
{
	for (INT i=0; i<Animations.Num(); i++)
	{
		if ( Animations(i).Tag == AnimTag )
		{
			Animations(i).bIsPlaying = false;
			if ( bForceEnd )
			{
				FWidgetAnimSequence Seq = Animations(i).Sequences(Animations(i).SeqIndex);
				Animations(i).Value = Seq.EndValue;
				SetPosition( Animations(i).Value, (Animations(i).Property == EAPT_PositionLeft) ? UIFACE_Left : UIFACE_Top, EVALPOS_PercentageOwner);
			}
			break;
		}
	}
}


/*=========================================================================================
  The base Scoreboard Class
  ========================================================================================= */

void UUTUIScene_Scoreboard::Tick(FLOAT DeltaTime)
{
	if ( GIsGame || bEditorRealTimePreview )
	{
		eventTickScene(DeltaTime);
	}

	Super::Tick(DeltaTime);
}


/*=========================================================================================
  UTUIScene_MOTD
  ========================================================================================= */

void UUTUIScene_MOTD::Tick(FLOAT DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GIsGame && !bFadingOut)
	{
		ULocalPlayer* LP = GetPlayerOwner(0);
		if (LP && LP->Actor && LP->Actor->PlayerReplicationInfo)
		{
			if ( !LP->Actor->PlayerReplicationInfo->bIsSpectator )
			{
				eventFinish();
			}
		}
	}
}

void UUTUIScene_MapVote::Tick(FLOAT DeltaTime)
{
	FLOAT FullSeconds = 0.0;
	INT Secs, Mins, Hours;
	if (TimeRemaining)
	{
		AUTGameReplicationInfo* GRI = Cast<AUTGameReplicationInfo>(GWorld->GetWorldInfo()->GRI);
		if ( GRI )
		{
			FullSeconds = GRI->MapVoteTimeRemaining;
		}

		Hours = appTrunc(FullSeconds / 3600.0);
		FullSeconds -= (Hours * 3600);
		Mins = appTrunc(FullSeconds / 60.0);
		FullSeconds -= (Mins * 60);
		Secs = appTrunc(FullSeconds);

		Hours = Clamp<INT>(Hours,0,99);
		Mins = Clamp<INT>(Mins,0,59);
		Secs = Clamp<INT>(Secs,0,59);

		FString TimeStr = FString::Printf(TEXT("%02i:%02i:%02i"),Hours,Mins,Secs);
		TimeRemaining->SetValue(TimeStr);
	}

	Super::Tick(DeltaTime);
}

/* DrawActorOverlays()
draw overlays for actors that were rendered this tick
*/
void AUTHUD::DrawActorOverlays(FVector ViewPoint, FRotator ViewRotation)
{
	// determine rendered camera position
	FVector ViewDir = ViewRotation.Vector();

	for ( INT i=0; i<PostRenderedActors.Num(); i++ )
	{
		AActor *A = PostRenderedActors(i);
		if ( A )
		{
			A->NativePostRenderFor(PlayerOwner,Canvas,ViewPoint,ViewDir);
		}
	}
}

void AUTGameObjective::SetHUDLocation(FVector NewHUDLocation)
{
	HUDLocation = NewHUDLocation;
}

void AUTPawn::SetHUDLocation(FVector NewHUDLocation)
{
	HUDLocation = NewHUDLocation;
}

void AUTCarriedObject::SetHUDLocation(FVector NewHUDLocation)
{
	HUDLocation = NewHUDLocation;
}

void AUTVehicle::SetHUDLocation(FVector NewHUDLocation)
{
	HUDLocation = NewHUDLocation;
}

void AUTVehicleFactory::SetHUDLocation(FVector NewHUDLocation)
{
	HUDLocation = NewHUDLocation;
}

/**
  * Update Node positions and sensor array
  */
void UUTMapInfo::UpdateNodes(AUTPlayerController *PlayerOwner)
{
	Sensors.Empty();
	for ( INT i=0; i < Objectives.Num(); i++ )
	{
		if ( Objectives(i) )
		{
			Objectives(i)->bAlreadyRendered = FALSE;
			Objectives(i)->SetHUDLocation(UpdateHUDLocation(Objectives(i)->Location)); 
			if (  Objectives(i)->bHasSensor && Objectives(i)->WorldInfo->GRI && Objectives(i)->WorldInfo->GRI->OnSameTeam(Objectives(i), PlayerOwner) )
			{
				Sensors.AddItem(Objectives(i));
			}
		}
	}
}

/**
  * Update Node positions and sensor array
  */
void UUTMapInfo::RenderLinks(UCanvas *Canvas, AUTPlayerController *PlayerOwner)
{
	for ( INT i=0; i<Objectives.Num(); i++ )
	{
		if ( Objectives(i) && Objectives(i)->IconHudTexture && !Objectives(i)->bIsDisabled )
		{
			// draw attack icons
			if ( Objectives(i)->bUnderAttack )
			{
				FLOAT AttackScale = 0.03f * Canvas->ClipX * (1.5f + 0.5f*appSin(6.f*PlayerOwner->WorldInfo->TimeSeconds));
				Canvas->CurX = Objectives(i)->HUDLocation.X - 0.5f*AttackScale;
				Canvas->CurY = Objectives(i)->HUDLocation.Y - 0.5f*AttackScale;
				Objectives(i)->AttackLinearColor.B = ColorPercent;
				Canvas->DrawTile(Objectives(i)->IconHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, AttackScale, AttackScale * Objectives(i)->IconCoords.VL/Objectives(i)->IconCoords.UL, Objectives(i)->AttackCoords.U, Objectives(i)->AttackCoords.V, Objectives(i)->AttackCoords.UL, Objectives(i)->AttackCoords.VL, Objectives(i)->AttackLinearColor);
			}

			// draw node icons
			FLinearColor NodeColor = Objectives(i)->ControlColor[::Min<INT>(Objectives(i)->DefenderTeamIndex, 2)];
			if ( Objectives(i)->bIsConstructing )
			{
				NodeColor *= ColorPercent;
				NodeColor.R += Objectives(i)->ControlColor[2].R * (1.f - ColorPercent);
				NodeColor.G += Objectives(i)->ControlColor[2].G * (1.f - ColorPercent);
				NodeColor.B += Objectives(i)->ControlColor[2].B * (1.f - ColorPercent);
			}
			if ( Objectives(i)->HighlightScale > 1.f )
			{
				FLOAT CurrentScale = (PlayerOwner->WorldInfo->TimeSeconds - Objectives(i)->LastHighlightUpdate)/Objectives(i)->HighlightSpeed;
				Objectives(i)->HighlightScale = ::Max(1.f, Objectives(i)->HighlightScale - CurrentScale * Objectives(i)->MaxHighlightScale);
				Objectives(i)->DrawIcon(Canvas, Objectives(i)->HUDLocation, Objectives(i)->MinimapIconScale * Objectives(i)->HighlightScale * MapScale, 1.f, PlayerOwner, NodeColor);
			}
			else
			{
				Objectives(i)->DrawIcon(Canvas, Objectives(i)->HUDLocation, Objectives(i)->MinimapIconScale * MapScale, 1.f, PlayerOwner, NodeColor);
			}
		}
	}

	// draw links
	for ( INT i=0; i<Objectives.Num(); i++ )
	{
		if ( Objectives(i) && !Objectives(i)->bIsDisabled )
		{
			Objectives(i)->RenderMyLinks(this, Canvas, PlayerOwner, ColorPercent);
		}
	}
}

/**
  * Update Node positions and sensor array
  */
void UUTMapInfo::RenderAdditionalInformation(UCanvas *Canvas, AUTPlayerController *PlayerOwner)
{
	// draw extra info
	for ( INT i=0; i<Objectives.Num(); i++ )
	{
		if ( Objectives(i) && Objectives(i)->bScriptRenderAdditionalMinimap && !Objectives(i)->bIsDisabled )
		{
			Objectives(i)->eventRenderMinimap(this, Canvas, PlayerOwner, ColorPercent);
		}
	}
}

void AUTGameObjective::RenderMyLinks( UUTMapInfo *MP, UCanvas *Canvas, AUTPlayerController *PlayerOwner, FLOAT ColorPercent )
{
}

void AUTGameObjective::DrawIcon(UCanvas *Canvas, FVector IconLocation, FLOAT IconWidth, FLOAT IconAlpha, AUTPlayerController* PlayerOwner, FLinearColor DrawColor)
{
	FLOAT YoverX = IconCoords.VL/IconCoords.UL;
	Canvas->CurX = IconLocation.X - 0.5f*IconWidth;
	Canvas->CurY = IconLocation.Y - 0.5f*IconWidth * YoverX;
	if ( IconHudTexture )
	{
		Canvas->DrawTile(IconHudTexture, Canvas->OrgX+Canvas->CurX, Canvas->OrgY+Canvas->CurY, IconWidth, IconWidth * YoverX, IconCoords.U, IconCoords.V, IconCoords.UL, IconCoords.VL,DrawColor);
	}
}



