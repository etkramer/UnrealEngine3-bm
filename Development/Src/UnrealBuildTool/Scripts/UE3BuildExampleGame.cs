/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
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

		public void SetUpGameEnvironment(CPPEnvironment GameCPPEnvironment, LinkEnvironment FinalLinkEnvironment, List<UE3ProjectDesc> GameProjects)
		{
			GameCPPEnvironment.IncludePaths.Add("ExampleGame/Inc");
			GameProjects.Add( new UE3ProjectDesc( "ExampleGame/ExampleGame.vcproj") );

			if (GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Win32)
			{
				GameProjects.Add( new UE3ProjectDesc( "ExampleEditor/ExampleEditor.vcproj") );
				GameCPPEnvironment.IncludePaths.Add("ExampleEditor/Inc");
			}

			GameCPPEnvironment.Definitions.Add("GAMENAME=EXAMPLEGAME");
			GameCPPEnvironment.Definitions.Add("IS_EXAMPLEGAME=1");

		}
	}
}
