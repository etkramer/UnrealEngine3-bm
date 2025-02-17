/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealBuildTool
{
	class UE3BuildExampleGame : UE3BuildGame
	{
		public string GetGameName()
		{
			return "Example";
		}
		
		public FileItem GetXEXConfigFile()
		{
			return FileItem.GetExistingItemByPath("ExampleGame/Live/xex.xml");
		}

		public void SetUpGameEnvironment(CPPEnvironment GameCPPEnvironment, LinkEnvironment FinalLinkEnvironment, List<string> GameProjects)
		{
			GameCPPEnvironment.IncludePaths.Add("ExampleGame/Inc");
			GameProjects.Add("ExampleGame/ExampleGame.vcproj");

			if (GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Win32)
			{
				GameProjects.Add("ExampleEditor/ExampleEditor.vcproj");
				GameCPPEnvironment.IncludePaths.Add("ExampleEditor/Inc");
			}

			GameCPPEnvironment.Definitions.Add("GAMENAME=EXAMPLEGAME");
			GameCPPEnvironment.Definitions.Add("IS_EXAMPLEGAME=1");

		}
	}
}
