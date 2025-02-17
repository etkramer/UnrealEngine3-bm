/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealBuildTool
{
	class UE3BuildNanoGame : UE3BuildGame
	{
		public string GetGameName()
		{
			return "Nano";
		}

		public FileItem GetXEXConfigFile()
		{
			return FileItem.GetExistingItemByPath("NanoGame/Live/xex.xml");
		}

		public void SetUpGameEnvironment(CPPEnvironment GameCPPEnvironment, LinkEnvironment FinalLinkEnvironment, List<UE3ProjectDesc> GameProjects)
		{
			GameProjects.Add( new UE3ProjectDesc( "NanoGame/NanoGame.vcproj" ) );
			GameCPPEnvironment.IncludePaths.Add( "NanoGame/Inc" );

			if (GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Win32)
			{
				GameProjects.Add( new UE3ProjectDesc( "NanoEditor/NanoEditor.vcproj" ) );
				GameCPPEnvironment.IncludePaths.Add("NanoEditor/Inc");
			}

			GameCPPEnvironment.Definitions.Add("GAMENAME=NANOGAME");
			GameCPPEnvironment.Definitions.Add("IS_NANOGAME=1");

			if (GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Xbox360)
			{
				// Compile and link with Vince on Xbox360.
				GameCPPEnvironment.IncludePaths.Add("../External/Vince/include");

				// Compile and link with the XNA content server on Xbox 360.
				GameCPPEnvironment.IncludePaths.Add("../External/XNAContentServer/include");
				FinalLinkEnvironment.LibraryPaths.Add("../External/XNAContentServer/lib/xbox");
			}
		}
	}
}
