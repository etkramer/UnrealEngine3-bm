/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;

namespace UnrealBuildTool
{
	partial class UE3BuildTarget
	{
		void SetUpGameSpyEnvironment()
		{
			// If GameSpy isn't available locally, disable it.
			if (!Directory.Exists("../External/GameSpy"))
			{
				GlobalCPPEnvironment.Definitions.Add("WITH_GAMESPY=0");
			}
			else
			{
				// Compile and link with GameSpy.
				GlobalCPPEnvironment.SystemIncludePaths.Add("../External/GameSpy");
				GlobalCPPEnvironment.SystemIncludePaths.Add("../External/GameSpy/voice2/speex-1.0.5/include");
				GlobalCPPEnvironment.Definitions.Add("GSI_UNICODE=1");
				GlobalCPPEnvironment.Definitions.Add("SB_ICMP_SUPPORT=1");
				GlobalCPPEnvironment.Definitions.Add("_CRT_SECURE_NO_WARNINGS=1");
				GlobalCPPEnvironment.Definitions.Add("UNIQUEID=1");
			}
		}

		void SetUpPhysXEnvironment()
		{
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/PhysX/SDKs/Foundation/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/PhysX/SDKs/Physics/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/PhysX/SDKs/Cooking/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/PhysX/SDKs/PhysXLoader/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/PhysX/SDKs/PhysXExtensions/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/PhysX/SDKs/TetraMaker/NxTetra");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/PhysX/Nxd/include");
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				FinalLinkEnvironment.DelayLoadDLLs.Add("PhysXLoaderDEBUG.dll");
			}
			else
			{
				FinalLinkEnvironment.DelayLoadDLLs.Add("PhysXLoader.dll");
			}
		}

		void SetUpFaceFXEnvironment()
		{
			// If FaceFX isn't available locally, disable it.
			if(!Directory.Exists("../External/FaceFX"))
			{
				GlobalCPPEnvironment.Definitions.Add("WITH_FACEFX=0");
			}
			else
			{
				// Compile and link with the FaceFX SDK on all platforms.
				GlobalCPPEnvironment.SystemIncludePaths.Add("../External/FaceFX/FxSDK/Inc");
				GlobalCPPEnvironment.SystemIncludePaths.Add("../External/FaceFX/FxCG/Inc");
				GlobalCPPEnvironment.SystemIncludePaths.Add("../External/FaceFX/FxAnalysis/Inc");

				if (Platform == UnrealTargetPlatform.Win32)
				{
					// Compile and link with FaceFX studio on Win32.
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/Main/Inc");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/Widgets/Inc");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/Framework/Audio/Inc");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/Framework/Commands/Inc");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/Framework/Console/Inc");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/Framework/GUI/Inc");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/Framework/Misc/Inc");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/Framework/Proxies/Inc");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/Framework/Gestures/Inc");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/External/OpenAL/include");
					GlobalCPPEnvironment.IncludePaths.Add("../External/FaceFX/Studio/External/libresample-0.1.3");
					NonGameProjects.Add("../External/FaceFX/Studio/Studio_vs8.vcproj");
				}
			}
		}

		void SetUpBinkEnvironment()
		{
			// If Bink isn't available locally, disable it.
			if (!Directory.Exists("../External/Bink"))
			{
				GlobalCPPEnvironment.Definitions.Add("WITH_BINK=0");
			}
			else
			{
				GlobalCPPEnvironment.SystemIncludePaths.Add("../External/Bink");
			}
		}
	}
}
