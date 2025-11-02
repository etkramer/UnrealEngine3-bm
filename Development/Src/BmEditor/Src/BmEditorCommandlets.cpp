/*=============================================================================
	BmEditorCommandlets.cpp: Bm editor comandlet definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "BmEditorClasses.h"

#include "PackageHelperFunctions.h"

IMPLEMENT_CLASS(UExtractPackagesCommandlet);

INT UExtractPackagesCommandlet::Main(const FString& Params)
{
	// For now, list these out manually.
	LoadPackage(NULL, TEXT("BmGame"), LOAD_None);

	// Admin
	{
		LoadPackage(NULL, TEXT("Admin"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_A"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_A_CH1234_Grapple"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_A_CH3"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_A_CH4"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_A_CH4567_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_A_CH567_Grapple"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_A_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_A_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_B1"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B1_Animation"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B1_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B1_static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_B2"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B2_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B2_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_B3"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B3_Animation"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B3_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B3_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_B4"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B4_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_B4_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_C1"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C1_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C1_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_C3"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C3_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C3_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_C4"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C4_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C4_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_C5"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C5_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C5_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_C8"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C8_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C8_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_C9"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C9_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_C9_Static"), LOAD_None);

		LoadPackage(NULL, TEXT("Admin_S2"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_S2_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_S2_Sound"), LOAD_None);
		LoadPackage(NULL, TEXT("Admin_S2_Static"), LOAD_None);
	}

	// Cave
	/*{
		LoadPackage(NULL, TEXT("Cave"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_A"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_A_CH6"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_A_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_A_Rocks"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_A_sewer"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_A_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B1"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B1_Animation_CH1234"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B1_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B1_MixSyrum_CH567"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B1_UltraBatClaw_CH567"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B3"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B3_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B3_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B5"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B5_Art_CrocBoss"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B5_CrocBoss"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B5_venom"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B5_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B5_Static"), LOAD_None);
		// LoadPackage(NULL, TEXT("Cave_B6"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B6_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C1"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C1_CH3_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C1_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C2"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C2_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C5"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C5_ch567"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C5_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C5_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C6"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C6_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_C6_Static"), LOAD_None);
	}

	// Cells
	{
		LoadPackage(NULL, TEXT("Cell"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_A1"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_A1_c3_fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_A1_c3_static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_A1_CH4_Harley"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_A1_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_A1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B1"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B1_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B2"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B2_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B2_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B3"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B3_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B3_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_C1"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_C1_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_C1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_C2"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_C2_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_C2_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_C6"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_C6_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_C6_Static"), LOAD_None);
	}

	// Challenge
	{
		LoadPackage(NULL, TEXT("Admin_B2_CH0"), LOAD_None);
		LoadPackage(NULL, TEXT("Cave_B6_CH0"), LOAD_None);
		LoadPackage(NULL, TEXT("Cell_B1_CH0"), LOAD_None);
		LoadPackage(NULL, TEXT("CombatCave_C1_CH0"), LOAD_None);
		LoadPackage(NULL, TEXT("Combat_Cave"), LOAD_None);
		LoadPackage(NULL, TEXT("Combat_Cave_Joker"), LOAD_None);
		LoadPackage(NULL, TEXT("Combat_Cell"), LOAD_None);
		LoadPackage(NULL, TEXT("Combat_Cell_Joker"), LOAD_None);
		LoadPackage(NULL, TEXT("Combat_Garden"), LOAD_None);
		LoadPackage(NULL, TEXT("Combat_Garden_Joker"), LOAD_None);
		LoadPackage(NULL, TEXT("Combat_Max"), LOAD_None);
		LoadPackage(NULL, TEXT("Combat_Max_Joker"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B5_CH0"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_A1_CH0"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B3_CH0"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_A_CH0"), LOAD_None);
		LoadPackage(NULL, TEXT("Predator_Admin"), LOAD_None);
		LoadPackage(NULL, TEXT("Predator_Admin_Joker"), LOAD_None);
		LoadPackage(NULL, TEXT("Predator_Cave"), LOAD_None);
		LoadPackage(NULL, TEXT("Predator_Cave_Joker"), LOAD_None);
		LoadPackage(NULL, TEXT("Predator_Max"), LOAD_None);
		LoadPackage(NULL, TEXT("Predator_Max_Joker"), LOAD_None);
		LoadPackage(NULL, TEXT("Predator_Med"), LOAD_None);
		LoadPackage(NULL, TEXT("Predator_Med_Joker"), LOAD_None);
	}

	// DLC
	{

	}

	// Frontend
	{
		LoadPackage(NULL, TEXT("BatEntry"), LOAD_None);
		LoadPackage(NULL, TEXT("Benchmark"), LOAD_None);
		LoadPackage(NULL, TEXT("Boot"), LOAD_None);
		LoadPackage(NULL, TEXT("DebugFrontend"), LOAD_None);
		LoadPackage(NULL, TEXT("Frontend"), LOAD_None);
		LoadPackage(NULL, TEXT("Frontend_Batman"), LOAD_None);
		LoadPackage(NULL, TEXT("V_AaronCash"), LOAD_None);
		LoadPackage(NULL, TEXT("V_ArkhamInmate"), LOAD_None);
		LoadPackage(NULL, TEXT("V_ArmouredBatman"), LOAD_None);
		LoadPackage(NULL, TEXT("V_Bane"), LOAD_None);
		LoadPackage(NULL, TEXT("V_Batman"), LOAD_None);
		LoadPackage(NULL, TEXT("V_BatMobile"), LOAD_None);
		LoadPackage(NULL, TEXT("V_BatWing"), LOAD_None);
		LoadPackage(NULL, TEXT("V_BG"), LOAD_None);
		LoadPackage(NULL, TEXT("V_BlackgateInmate"), LOAD_None);
		LoadPackage(NULL, TEXT("V_FrankBoles"), LOAD_None);
		LoadPackage(NULL, TEXT("V_HarleyQuinn"), LOAD_None);
		LoadPackage(NULL, TEXT("V_JimGordon"), LOAD_None);
		LoadPackage(NULL, TEXT("V_Joker"), LOAD_None);
		LoadPackage(NULL, TEXT("V_KillerCroc"), LOAD_None);
		LoadPackage(NULL, TEXT("V_PoisonIvy"), LOAD_None);
		LoadPackage(NULL, TEXT("V_QuincySharp"), LOAD_None);
		LoadPackage(NULL, TEXT("V_Scarecrow"), LOAD_None);
		LoadPackage(NULL, TEXT("V_Scarface"), LOAD_None);
		LoadPackage(NULL, TEXT("V_VenomHenchman"), LOAD_None);
		LoadPackage(NULL, TEXT("V_VenomJoker"), LOAD_None);
		LoadPackage(NULL, TEXT("V_Zsasz"), LOAD_None);
	}

	// Garden
	{
		LoadPackage(NULL, TEXT("Garden"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_A"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_A_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_A_newARCH"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_A_Static"), LOAD_None);
		// LoadPackage(NULL, TEXT("Garden_B1"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B3"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B3_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B3_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B4"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B4_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B4_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B5"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B5_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B5_Static_2"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B7"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B7_CH56_StaticArch"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B7_CH5_Fountain"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B7_CH5_StaticWall"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_B7_CH6_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C1"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C1_CH4567_Ambush"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C2"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C2_CH4"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C2_CH45_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C2_CH4_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C2_CH5"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C2_CH67_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C2_CH6_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C2_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C2_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C3"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C3_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Garden_C3_Static"), LOAD_None);
	}

	// Max
	{
		LoadPackage(NULL, TEXT("Max"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_A1"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_A1_CH1"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_A1_CH2567"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_A1_CH2_Animation"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_A1_CH5_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_A1_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_A1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B1"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B1_CH1"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B1_CH234567_LiftDebris"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B1_CH267"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B1_CH5"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B1_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B3"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B3_CH234567"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B3_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B3_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B4"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B4_CH234567_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B4_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B4_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B5"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B5_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B5_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B6"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B6_CH12"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B6_CH7"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B6_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_B6_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_BatmanParty"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C0"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C1"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C1_CH1"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C1_CH23467"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C1_CH5"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C1_CH567_Venom"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C1_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C5"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C5_CH1"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C5_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C5_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C6"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C6_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C6_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C8"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C8_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C8_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_C9"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_IntroParty"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_S3"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_S3_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Max_S3_Static"), LOAD_None);
	}*/

	// Medical
	{
		LoadPackage(NULL, TEXT("Medical"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_A"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_A_CH234567_Return"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_A_CH2_Pred1"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_A_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_A_Static_Art"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B1"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B1_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B2"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B2_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B2_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B3"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B3_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B3_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B4"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B4_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B4_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B5"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B5_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B5_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B6"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B6_CH12"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B6_CH34567"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B6_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B6_Static"), LOAD_None);
		// LoadPackage(NULL, TEXT("Medical_B7"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B7_Fx"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_B7_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_C0"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_C1"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_C2"), LOAD_None);
		// LoadPackage(NULL, TEXT("Medical_C2_CH23"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_C2_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_C2_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_C5"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_C5_CH2"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_C5_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_S1"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_S1_brokenMorque2"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_S1_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Medical_S1_Static"), LOAD_None);
	}

	// Overworld
	{
		// LoadPackage(NULL, TEXT("Overworld"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A1"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A1_CH2_Combat"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A1_CH3"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A1_CH4"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A1_CH5"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A1_CH6"), LOAD_None);
		// LoadPackage(NULL, TEXT("Overworld_A1_CH7"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A1_CH7_Tunnel"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A1_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A1_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A2"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A2_CH3"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A2_CH4"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A2_CH5"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A2_CH6"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A2_CH7"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A2_Static"), LOAD_None);
		// LoadPackage(NULL, TEXT("Overworld_A3"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A3_CH2"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A3_CH3_Combat"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A3_CH4"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A3_CH5"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A3_CH6"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A3_CH7"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A3_FX"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_A3_Grave"), LOAD_None);
		// LoadPackage(NULL, TEXT("Overworld_A3_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_C0"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_C3"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_C3_CH5"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_C3_CH6"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_C3_CH7"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_C3_Static"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_C4"), LOAD_None);
		LoadPackage(NULL, TEXT("Overworld_C4_Static"), LOAD_None);
	}

	// Visitor
	/*{
		LoadPackage(NULL, TEXT("Visitor"), LOAD_None);
		LoadPackage(NULL, TEXT("Visitor_B1"), LOAD_None);
		LoadPackage(NULL, TEXT("Visitor_B1_Static"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_B2"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_B2_Fx"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_B2_Static"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_C1"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_C1_CH2_joker"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_C1_CH3_joker"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_C1_CH4_joker"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_C1_CH5_joker"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_C1_CH6_joker"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_C1_CH7_joker"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_C1_FX"), LOAD_None);
		// LoadPackage(NULL, TEXT("Visitor_c1_Static"), LOAD_None);
	}*/

	// Set GIsCooking. Without this, SavePackage() will strip the PKG_Cooked flag.
	GIsCooking = TRUE;
	GCookingTarget = UE3::EPlatformType::PLATFORM_PC;

	// Enumerate all objects, searching for force-exported packages
	for ( TObjectIterator<UPackage> It ; It ; ++It )
	{
		UPackage* Package = *It;

		// Skip "group" (not top-level) packages
		if (Package->GetOuter())
		{
			continue;
		}

		// Skip non-BM, non-cooked packages
		if (!Package->IsBmCooked())
		{
			continue;
		}

		// Skip map packages
		if (Package->ContainsMap())
		{
			continue;
		}

		warnf(TEXT("  BM1: Found package '%s'"), *Package->GetPathName());

		// Determine output path
		FString FilePath = TEXT("..\\BmGame\\Packages\\");
		FilePath += Package->GetName();
		FilePath += (Package->PackageFlags & PKG_ContainsScript) ? TEXT(".u") : TEXT(".upk");

		// Save as cooked, uncompressed
		Package->PackageFlags |= PKG_Cooked;
		Package->PackageFlags &= ~PKG_StoreCompressed;

		SavePackage(Package, NULL, RF_Standalone, *FilePath, GWarn);
	}

	return 0;
}
