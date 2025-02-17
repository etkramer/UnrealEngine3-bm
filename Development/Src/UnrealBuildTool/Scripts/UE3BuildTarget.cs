/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
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

		void SetUpGameEnvironment(CPPEnvironment GameCPPEnvironment, LinkEnvironment FinalLinkEnvironment, List<UE3ProjectDesc> GameProjects);
	}

	enum UnrealTargetPlatform
	{
		Unknown,
		Win32,
		PS3,
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


	class UE3ProjectDesc
	{
		/** The project path and file name */
		public string ProjectPath;

		/** Sets whether the CLR (Common Language Runtime) should be enabled for this project */
		public CPPCLRMode CLRMode = CPPCLRMode.CLRDisabled;


		public UE3ProjectDesc( string InProjectPath )
		{
			ProjectPath = InProjectPath;
			CLRMode = CPPCLRMode.CLRDisabled;
		}

		public UE3ProjectDesc( string InProjectPath, CPPCLRMode InCLRMode )
		{
			ProjectPath = InProjectPath;
			CLRMode = InCLRMode;
		}
	}


	partial class UE3BuildTarget : Target
	{
		/** Game we're building. */
		UE3BuildGame Game = null;
		
		/** Platform as defined by the VCProject and passed via the command line. Not the same as internal config names. */
		UnrealTargetPlatform Platform = UnrealTargetPlatform.Unknown;
		
		/** Target as defined by the VCProject and passed via the command line. Not necessarily the same as internal name. */
		UnrealTargetConfiguration Configuration = UnrealTargetConfiguration.Unknown;
		
		/** Output path of final executable. */
		string OutputPath = null;
		
		/** Additional definitions passed via the command line. */
		List<string> AdditionalDefinitions = new List<string>();

		/** The C++ environment that all the environments used to compile UE3-based projects are derived from. */
		CPPEnvironment GlobalCPPEnvironment = new CPPEnvironment();

		/** The link environment that produces the output executable. */
		LinkEnvironment FinalLinkEnvironment = new LinkEnvironment();

		/** A list of projects that are compiled with the game-independent compilation environment. */
		List<UE3ProjectDesc> NonGameProjects = new List<UE3ProjectDesc>();

		/** A list of projects that are compiled with the game-dependent compilation environment. */
		List<UE3ProjectDesc> GameProjects = new List<UE3ProjectDesc>();

        /**
		* @return List of all projects being built for this Target configuration
		*/
        public List<string> GetProjectPaths() 
        {
            List<string> AllProjects = new List<string>(); 
            foreach (UE3ProjectDesc ProjDesc in NonGameProjects)
            {
                AllProjects.Add(ProjDesc.ProjectPath);
            }
            foreach (UE3ProjectDesc ProjDesc in GameProjects)
            {
                AllProjects.Add(ProjDesc.ProjectPath);
            }
            return AllProjects;
        }

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

			// Validates current settings and updates if required.
			BuildConfiguration.ValidateConfiguration( GlobalCPPEnvironment.TargetConfiguration, GlobalCPPEnvironment.TargetPlatform );

			// Initialize/ serialize the dependency cache from disc.
			InitializeDependencyCaches();

			// Compile the game and platform independent projects.
			CompileNonGameProjects();

			// Compile the game-specific projects.
			CompileGameProjects();

			// Save modified dependency cache and discard it.
			SaveDependencyCaches();

			// Put the non-executable output files (PDB, import library, etc) in the intermediate directory.
			// Note that this is overridden in GetWin32OutputItems().
			FinalLinkEnvironment.OutputDirectory = GlobalCPPEnvironment.OutputDirectory;

			// Link the object files into an executable.
			if (Platform == UnrealTargetPlatform.Win32)
			{
				return GetWin32OutputItems();
			}
			else if (Platform == UnrealTargetPlatform.Xbox360)
			{
				return GetXbox360OutputItems();
			}
			else
			{
				return GetPS3OutputItems();
			}
		}

		/**
		 * @return Name of target, e.g. name of the game being built.
		 */
		public string GetTargetName()
		{
			return Game.GetGameName();
		}
		/**
		 * @return Name of configuration, e.g. "Release"
		 */
		public string GetConfigurationName()
		{
			return Platform.ToString();
		}
		/**
		 * @return Name of platform, e.g. "Win32"
		 */
		public string GetPlatformName()
		{
			return Configuration.ToString();
		}
		/**
		 * @return TRUE if debug information is created, FALSE otherwise.
		 */
		public bool IsCreatingDebugInfo()
		{
			return GlobalCPPEnvironment.bCreateDebugInfo;
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
                    case "NANOGAME":
                        Game = new UE3BuildNanoGame();
                        break;
					case "UTGAME":
						Game = new UE3BuildUTGame();
						break;

					// Platform names:
					case "WIN32":
						Platform = UnrealTargetPlatform.Win32;
						break;
					case "XBOX360":
						Platform = UnrealTargetPlatform.Xbox360;
						break;
					case "PS3":
						Platform = UnrealTargetPlatform.PS3;
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
			// Incorporate toolchain in platform name.
			string PlatformString = Platform.ToString();
			if( BuildConfiguration.bUseIntelCompiler )
			{
				PlatformString += "-ICL";
			}

			// Determine the directory to store intermediate files in for this target.
			GlobalCPPEnvironment.OutputDirectory = Path.Combine(
				Path.Combine(
					BuildConfiguration.BaseIntermediatePath,
					PlatformString
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
            GlobalCPPEnvironment.SystemIncludePaths.Add("../External/nvTextureTools2/src/src");
            GlobalCPPEnvironment.SystemIncludePaths.Add("../External/nvTextureTools2/src/project/vc8");            
			GlobalCPPEnvironment.IncludePaths.Add("Engine/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Editor/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("GameFramework/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("IpDrv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("UnrealScriptTest/inc");
			GlobalCPPEnvironment.IncludePaths.Add("UnrealEd/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("UnrealEd/FaceFX");
            GlobalCPPEnvironment.IncludePaths.Add("ALAudio/Inc");
//// Atlas-Modif [Begin]
//            GlobalCPPEnvironment.IncludePaths.Add("Atlas/Inc");
//            GlobalCPPEnvironment.IncludePaths.Add("../../../Atlas/include");
//            GlobalCPPEnvironment.IncludePaths.Add("../../../Atlas/external");
//            GlobalCPPEnvironment.IncludePaths.Add("../../../Atlas/external/boost-1.35.0");
//            GlobalCPPEnvironment.IncludePaths.Add("../../../Atlas/external/log4boost-1.0/include/");
//// Atlas-Modif [End]

			if( BuildConfiguration.bAllowManagedCode )
			{
				GlobalCPPEnvironment.IncludePaths.Add("UnrealEdCLR/Inc");
			}

			// Compile and link with PhysX.
            if (Platform == UnrealTargetPlatform.PS3)
            {
                SetUpPS3PhysXEnvironment();
            }
            else
            {
                SetUpPhysXEnvironment();
            }

			// Compile and link with FaceFX.
			SetUpFaceFXEnvironment();

			// Compile and link with Bink.
			SetUpBinkEnvironment();

			// Add the definitions specified on the command-line.
			GlobalCPPEnvironment.Definitions.AddRange(AdditionalDefinitions);

			// Add the projects compiled for all games, all platforms.
			NonGameProjects.Add( new UE3ProjectDesc( "Core/Core.vcproj" ) );
			NonGameProjects.Add( new UE3ProjectDesc( "Engine/Engine.vcproj" ) );
			NonGameProjects.Add( new UE3ProjectDesc( "GameFramework/GameFramework.vcproj" ) );
			NonGameProjects.Add( new UE3ProjectDesc( "IpDrv/IpDrv.vcproj" ) );
			NonGameProjects.Add( new UE3ProjectDesc( "UnrealScriptTest/UnrealScriptTest.vcproj" ) );
            NonGameProjects.Add(new UE3ProjectDesc("ALAudio/ALAudio.vcproj"));
//// Atlas-Modif [Begin]
//            NonGameProjects.Add( new UE3ProjectDesc( "Atlas/Atlas.vcproj" ) );
//// Atlas-Modif [End]

			// Launch is compiled for all games/platforms, but uses the game-dependent defines, and so needs to be in the game project list.
			GameProjects.Add( new UE3ProjectDesc( "Launch/Launch.vcproj" ) );

            // Always disable debug info when stripping headers for faster iterations
            if (BuildConfiguration.bRemoveUnusedHeaders)
            {
                UE3BuildConfiguration.bDisableDebugInfo = true;
            }

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
				case UnrealTargetPlatform.PS3: MainCompilePlatform = CPPTargetPlatform.PS3_PPU; break;
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
				case UnrealTargetPlatform.PS3:
					SetUpPS3Environment();
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
					FinalLinkEnvironment.bIsShippingBinary = true;
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
					FinalLinkEnvironment.bIsShippingBinary = true;
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

		/** Initializes/ creates the dependency caches. */
		void InitializeDependencyCaches()
		{			
			CPPEnvironment.IncludeCache = DependencyCache.Create( Path.Combine(GlobalCPPEnvironment.OutputDirectory,"DependencyCache.bin") );
			CPPEnvironment.DirectIncludeCache = DependencyCache.Create( Path.Combine(GlobalCPPEnvironment.OutputDirectory,"DirectDependencyCache.bin") );
		}

		/** Saves dependency caches and discard them. */
		void SaveDependencyCaches()
		{
			if (CPPEnvironment.IncludeCache != null)
			{
				CPPEnvironment.IncludeCache.Save();
				CPPEnvironment.IncludeCache = null;
			}
			if (CPPEnvironment.DirectIncludeCache != null)
			{
				CPPEnvironment.DirectIncludeCache.Save();
				CPPEnvironment.DirectIncludeCache = null;
			}
		}        
	}
}
