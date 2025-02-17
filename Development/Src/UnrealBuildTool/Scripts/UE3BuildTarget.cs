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
	/** An interface implemented by UE3 games to define their build behavior. */
	interface UE3BuildGame
	{
		string GetGameName();

		FileItem GetXEXConfigFile();

		void SetUpGameEnvironment(CPPEnvironment GameCPPEnvironment, LinkEnvironment FinalLinkEnvironment, List<string> GameProjects);
	}

	enum UnrealTargetPlatform
	{
		Unknown,
		Win32,
		Xbox360
	}

	enum UnrealTargetConfiguration
	{
		Unknown,
		Debug,
		Release,
		Shipping,
		ShippingDebugConsole
	}

	partial class UE3BuildTarget : Target
	{
		UE3BuildGame Game = null;
		UnrealTargetPlatform Platform = UnrealTargetPlatform.Unknown;
		UnrealTargetConfiguration Configuration = UnrealTargetConfiguration.Unknown;
		string OutputPath = null;
		List<string> AdditionalDefinitions = new List<string>();

		/** The C++ environment that all the environments used to compile UE3-based projects are derived from. */
		CPPEnvironment GlobalCPPEnvironment = new CPPEnvironment();

		/** The link environment that produces the output executable. */
		LinkEnvironment FinalLinkEnvironment = new LinkEnvironment();

		/** A list of projects that are compiled with the game-independent compilation environment. */
		List<string> NonGameProjects = new List<string>();

		/** A list of projects that are compiled with the game-dependent compilation environment. */
		List<string> GameProjects = new List<string>();

		public IEnumerable<FileItem> Build(string[] Arguments)
		{
			// Parse the command-line arguments.
			ParseArguments(Arguments);

			// Describe what's being built.
			if (BuildConfiguration.bPrintDebugInfo)
			{
				Console.WriteLine("Building {0} - {1} - {2}", Game.GetGameName(), Platform, Configuration);
			}

			// Set up the global compile and link environment in GlobalCPPEnvironment and FinalLinkEnvironment.
			SetUpGlobalEnvironment();
			SetUpPlatformEnvironment();
			SetUpConfigurationEnvironment();

			// Compile the game and platform independent projects.
			CompileNonGameProjects();

			// Compile the game-specific projects.
			CompileGameProjects();

			// Put the non-executable output files (PDB, import library, etc) in the intermediate directory.
			// Note that this is overridden in GetWin32OutputItems().
			FinalLinkEnvironment.OutputDirectory = GlobalCPPEnvironment.OutputDirectory;

			// Link the object files into an executable.
			if (Platform == UnrealTargetPlatform.Win32)
			{
				return GetWin32OutputItems();
			}
			else/* if (Platform == UnrealTargetPlatform.Xbox360)*/
			{
				return GetXbox360OutputItems();
			}
		}

		void ParseArguments(string[] Arguments)
		{
			for (int ArgumentIndex = 0; ArgumentIndex < Arguments.Length; ArgumentIndex++)
			{
				switch (Arguments[ArgumentIndex].ToUpperInvariant())
				{
					// Game names:
					case "EXAMPLEGAME":
						Game = new UE3BuildExampleGame();
						break;
					case "GEARGAME":
						Game = new UE3BuildGearGame();
						break;
					case "BMGAME":
						Game = new UE3BuildBmGame();
						break;

					// Platform names:
					case "WIN32":
						Platform = UnrealTargetPlatform.Win32;
						break;
					case "XBOX360":
						Platform = UnrealTargetPlatform.Xbox360;
						break;

					// Configuration names:
					case "DEBUG":
						Configuration = UnrealTargetConfiguration.Debug;
						break;
					case "RELEASE":
						Configuration = UnrealTargetConfiguration.Release;
						break;
					case "SHIPPING":
						Configuration = UnrealTargetConfiguration.Shipping;
						break;
					case "SHIPPINGDEBUGCONSOLE":
						Configuration = UnrealTargetConfiguration.ShippingDebugConsole;
						break;

					// -Output specifies the output filename.
					case "-OUTPUT":
						if (ArgumentIndex + 1 >= Arguments.Length)
						{
							throw new BuildException("Expected path after -output argument, but found nothing.");
						}
						ArgumentIndex++;
						OutputPath = Arguments[ArgumentIndex];
						break;

					// -Define <definition> adds a definition to the global C++ compilation environment.
					case "-DEFINE":
						if (ArgumentIndex + 1 >= Arguments.Length)
						{
							throw new BuildException("Expected path after -output argument, but found nothing.");
						}
						ArgumentIndex++;
						AdditionalDefinitions.Add(Arguments[ArgumentIndex]);
						break;

					// Enable deployment.
					case "-DEPLOY":
						UE3BuildConfiguration.bDeployAfterCompile = true;
						break;

					// Disable debug info.
					case "-NODEBUGINFO":
						UE3BuildConfiguration.bDisableDebugInfo = true;
						break;
				}
			}

			// Verify that the required parameters have been found.
			if (Game == null)
			{
				throw new BuildException("Couldn't find game name on command-line.");
			}
			if (Platform == UnrealTargetPlatform.Unknown)
			{
				throw new BuildException("Couldn't find platform name on command-line.");
			}
			if (Configuration == UnrealTargetConfiguration.Unknown)
			{
				throw new BuildException("Couldn't find configuration name on command-line.");
			}
			if (OutputPath == null)
			{
				throw new BuildException("Couldn't find output path on command-line.");
			}
		}

		void SetUpGlobalEnvironment()
		{
			// Determine the directory to store intermediate files in for this target.
			GlobalCPPEnvironment.OutputDirectory = Path.Combine(
				Path.Combine(
					BuildConfiguration.BaseIntermediatePath,
					Platform.ToString()
					),
				Configuration.ToString()
				);

			GlobalCPPEnvironment.Definitions.Add("UNICODE");
			GlobalCPPEnvironment.Definitions.Add("_UNICODE");
			GlobalCPPEnvironment.Definitions.Add("__UNREAL__");

			// Add the platform-independent public project include paths.
			GlobalCPPEnvironment.IncludePaths.Add("Core/Inc/Epic");
			GlobalCPPEnvironment.IncludePaths.Add("Core/Inc/Licensee");
			GlobalCPPEnvironment.IncludePaths.Add("Core/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Engine/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Editor/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("GameFramework/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("IpDrv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("UnrealScriptTest/inc");
			GlobalCPPEnvironment.IncludePaths.Add("UnrealEd/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("UnrealEd/FaceFX");

			// Compile and link with PhysX.
			SetUpPhysXEnvironment();

			// Compile and link with FaceFX.
			SetUpFaceFXEnvironment();

			// Compile and link with Bink.
			SetUpBinkEnvironment();

			// Add the definitions specified on the command-line.
			GlobalCPPEnvironment.Definitions.AddRange(AdditionalDefinitions);

			// Add the projects compiled for all games, all platforms.
			NonGameProjects.Add("Core/Core.vcproj");
			NonGameProjects.Add("Engine/Engine.vcproj");
			NonGameProjects.Add("GameFramework/GameFramework.vcproj");
			NonGameProjects.Add("IpDrv/IpDrv.vcproj");
			NonGameProjects.Add("UnrealScriptTest/UnrealScriptTest.vcproj");

			// Launch is compiled for all games/platforms, but uses the game-dependent defines, and so needs to be in the game project list.
			GameProjects.Add("Launch/Launch.vcproj");

			// Create debug info based on the heuristics specified by the user.
			GlobalCPPEnvironment.bCreateDebugInfo =
				!UE3BuildConfiguration.bDisableDebugInfo &&
				DebugInfoHeuristic.ShouldCreateDebugInfo(Platform, Configuration);
		}

		void SetUpPlatformEnvironment()
		{
			// Determine the primary C++ platform to compile the engine for.
			CPPTargetPlatform MainCompilePlatform;
			switch (Platform)
			{
				case UnrealTargetPlatform.Win32: MainCompilePlatform = CPPTargetPlatform.Win32; break;
				case UnrealTargetPlatform.Xbox360: MainCompilePlatform = CPPTargetPlatform.Xbox360; break;
				default: throw new BuildException("Unrecognized target platform");
			}

			FinalLinkEnvironment.TargetPlatform = MainCompilePlatform;
			GlobalCPPEnvironment.TargetPlatform = MainCompilePlatform;

			// Set up the platform-specific environment.
			switch(Platform)
			{
				case UnrealTargetPlatform.Win32:
					SetUpWin32Environment();
					break;
				case UnrealTargetPlatform.Xbox360:
					SetUpXbox360Environment();
					break;
			}
		}

		void SetUpConfigurationEnvironment()
		{
			// Determine the C++ compile/link configuration based on the Unreal configuration.
			CPPTargetConfiguration CompileConfiguration;
			switch (Configuration)
			{
				default:
				case UnrealTargetConfiguration.Debug:
					CompileConfiguration = CPPTargetConfiguration.Debug;
					GlobalCPPEnvironment.Definitions.Add("_DEBUG=1");
					GlobalCPPEnvironment.Definitions.Add("FINAL_RELEASE=0");
					break;
				case UnrealTargetConfiguration.Release:
					CompileConfiguration = CPPTargetConfiguration.Release;
					GlobalCPPEnvironment.Definitions.Add("NDEBUG=1");
					GlobalCPPEnvironment.Definitions.Add("FINAL_RELEASE=0");
					break;
				case UnrealTargetConfiguration.Shipping:
					if(Platform == UnrealTargetPlatform.Win32)
					{
						// On Windows, Shipping is the same as release, but with SHIPPING_PC_GAME=1 defined.
						CompileConfiguration = CPPTargetConfiguration.Release;
						GlobalCPPEnvironment.Definitions.Add("NDEBUG=1");
						GlobalCPPEnvironment.Definitions.Add("FINAL_RELEASE=0");
						GlobalCPPEnvironment.Definitions.Add("SHIPPING_PC_GAME=1");
					}
					else
					{
						CompileConfiguration = CPPTargetConfiguration.ReleaseLTCG;
						GlobalCPPEnvironment.Definitions.Add("NDEBUG=1");
						GlobalCPPEnvironment.Definitions.Add("FINAL_RELEASE=1");
						GlobalCPPEnvironment.Definitions.Add("NO_LOGGING=1");
					}
					break;
				case UnrealTargetConfiguration.ShippingDebugConsole:
					if(Platform == UnrealTargetPlatform.Win32)
					{
						// On Windows, ShippingDebugConsole is the same as release, but with SHIPPING_PC_GAME=1 defined.
						CompileConfiguration = CPPTargetConfiguration.Release;
						GlobalCPPEnvironment.Definitions.Add("NDEBUG=1");
						GlobalCPPEnvironment.Definitions.Add("FINAL_RELEASE=0");
						GlobalCPPEnvironment.Definitions.Add("SHIPPING_PC_GAME=1");
					}
					else
					{
						CompileConfiguration = CPPTargetConfiguration.ReleaseLTCG;
						GlobalCPPEnvironment.Definitions.Add("NDEBUG=1");
						GlobalCPPEnvironment.Definitions.Add("FINAL_RELEASE=1");
						GlobalCPPEnvironment.Definitions.Add("FINAL_RELEASE_DEBUGCONSOLE=1");
						GlobalCPPEnvironment.Definitions.Add("NO_LOGGING=1");
					}
					break;
			}

			// Set up the global C++ compilation and link environment.
			GlobalCPPEnvironment.TargetConfiguration = CompileConfiguration;
			FinalLinkEnvironment.TargetConfiguration = CompileConfiguration;

		}

		void CompileNonGameProjects()
		{
			// Output .obj files for the game-independent projects to a directory that is the same regardless of the target game.
			CPPEnvironment GameIndependentCPPEnvironment = new CPPEnvironment(GlobalCPPEnvironment);
			GameIndependentCPPEnvironment.OutputDirectory = Path.Combine(GlobalCPPEnvironment.OutputDirectory, "Shared");

			// Compile the cross-platform engine projects.
			Utils.CompileProjects(GameIndependentCPPEnvironment, FinalLinkEnvironment, NonGameProjects);
		}

		void CompileGameProjects()
		{
			// Set up the game-specific environment.
			CPPEnvironment GameCPPEnvironment = new CPPEnvironment(GlobalCPPEnvironment);
			Game.SetUpGameEnvironment(GameCPPEnvironment, FinalLinkEnvironment, GameProjects);

			// Output .obj files for the game-dependent projects to a directory that is dependent on the target game.
			GameCPPEnvironment.OutputDirectory = Path.Combine(GameCPPEnvironment.OutputDirectory, Game.GetGameName());

			// Compile the game-dependent projects.
			Utils.CompileProjects(GameCPPEnvironment, FinalLinkEnvironment, GameProjects);
		}
	}
}