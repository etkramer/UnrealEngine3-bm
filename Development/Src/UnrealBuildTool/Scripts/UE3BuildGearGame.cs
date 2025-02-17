/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealBuildTool
{
	class UE3BuildGearGame : UE3BuildGame
	{
		public string GetGameName()
		{
			return "Gear";
		}

		public FileItem GetXEXConfigFile()
		{
			return FileItem.GetExistingItemByPath("GearGame/Live/xex.xml");
		}

		public void SetUpGameEnvironment(CPPEnvironment GameCPPEnvironment, LinkEnvironment FinalLinkEnvironment, List<UE3ProjectDesc> GameProjects)
		{
			GameProjects.Add( new UE3ProjectDesc( "GearGame/GearGame.vcproj" ) );
			GameCPPEnvironment.IncludePaths.Add("GearGame/Inc");

			if (GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Win32)
			{
				GameProjects.Add( new UE3ProjectDesc( "GearEditor/GearEditor.vcproj" ) );
				GameCPPEnvironment.IncludePaths.Add("GearEditor/Inc");
			}

            // Mars-Modif [Begin]
            //GameProjects.Add(new UE3ProjectDesc("MarsGame/MarsGame.vcproj"));
            //GameCPPEnvironment.IncludePaths.Add("MarsGame/Inc");
            // Mars-Modif [End

			GameCPPEnvironment.Definitions.Add("GAMENAME=GEARGAME");
			GameCPPEnvironment.Definitions.Add("IS_GEARGAME=1");

			if (GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Xbox360)
			{
				// Compile and link with the XNA content server on Xbox 360.
				GameCPPEnvironment.IncludePaths.Add("../External/XNAContentServer/include");
				FinalLinkEnvironment.LibraryPaths.Add("../External/XNAContentServer/lib/xbox");
			}
		}
	}
}
