/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealBuildTool
{
	class UE3BuildUTGame : UE3BuildGame
	{
		public string GetGameName()
		{
			return "UT";
		}

		public FileItem GetXEXConfigFile()
		{
			return FileItem.GetExistingItemByPath("UTGame/Live/xex.xml");
		}

		public void SetUpGameEnvironment(CPPEnvironment GameCPPEnvironment, LinkEnvironment FinalLinkEnvironment, List<string> GameProjects)
		{
			GameProjects.Add("UTGame/UTGame.vcproj");
			GameCPPEnvironment.IncludePaths.Add("UTGame/Inc");

			if (GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Win32)
			{
				GameProjects.Add("UTEditor/UTEditor.vcproj");
				GameCPPEnvironment.IncludePaths.Add("UTEditor/Inc");
			}

			GameCPPEnvironment.Definitions.Add("GAMENAME=UTGAME");
			GameCPPEnvironment.Definitions.Add("IS_UTGAME=1");
		}
	}
}
