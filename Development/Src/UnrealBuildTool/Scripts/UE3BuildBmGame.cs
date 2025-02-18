/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealBuildTool
{
	class UE3BuildBmGame : UE3BuildGame
	{
		public string GetGameName()
		{
			return "BmGame";
		}
		
		public FileItem GetXEXConfigFile()
		{
			return FileItem.GetExistingItemByPath("BmGame/Live/xex.xml");
		}

		public void SetUpGameEnvironment(CPPEnvironment GameCPPEnvironment, LinkEnvironment FinalLinkEnvironment, List<string> GameProjects)
		{
			GameCPPEnvironment.IncludePaths.Add("BmGame/Inc");
			GameProjects.Add("BmGame/BmGame.vcproj");

			if (GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Win32)
			{
				GameProjects.Add("BmEditor/BmEditor.vcproj");
				GameCPPEnvironment.IncludePaths.Add("BmEditor/Inc");
			}

			GameCPPEnvironment.Definitions.Add("GAMENAME=BMGAME");
			GameCPPEnvironment.Definitions.Add("IS_BMGAME=1");

		}
	}
}
