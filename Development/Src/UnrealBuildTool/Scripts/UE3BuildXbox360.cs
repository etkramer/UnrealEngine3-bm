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
	partial class UE3BuildTarget
	{
		void SetUpXbox360Environment()
		{
			GlobalCPPEnvironment.Definitions.Add("_XBOX=1");
			GlobalCPPEnvironment.Definitions.Add("XBOX=1");
			GlobalCPPEnvironment.Definitions.Add("__STATIC_LINK=1");
			GlobalCPPEnvironment.Definitions.Add("_MBCS");

			// Add the Xbox 360 public project include paths.
			GlobalCPPEnvironment.IncludePaths.Add("Xenon/XeAudio/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Xenon/XeCore/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Xenon/XeD3DDrv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Xenon/XeD3DDrv/Src");
			GlobalCPPEnvironment.IncludePaths.Add("Xenon/XeDrv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Xenon/XeEngine/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Xenon/XeIpDrv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Xenon/XeLaunch/Src");
            //GlobalCPPEnvironment.IncludePaths.Add( "XAudio2/Inc" );
            GlobalCPPEnvironment.IncludePaths.Add( "OnlineSubsystemLive/Inc" );
			GlobalCPPEnvironment.IncludePaths.Add("OnlineSubsystemPC/Inc");

			// Include lzo and zlib headers.
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/lzopro/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/zlib/inc");

			// Compile and link with the Xbox 360 runtime libraries.
			GlobalCPPEnvironment.SystemIncludePaths.Add("$(XEDK)/include/xbox");
			FinalLinkEnvironment.LibraryPaths.Add("$(XEDK)/lib/xbox");
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
                FinalLinkEnvironment.AdditionalLibraries.Add( "xmcored.lib" );
                FinalLinkEnvironment.AdditionalLibraries.Add( "xgraphicsd.lib" );
				FinalLinkEnvironment.AdditionalLibraries.Add("xboxkrnl.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("xnetd.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("x3daudiod.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("xonlined.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("xmpd.lib");

				// Debug builds use LIBCMTD, so suppress any references to the other versions of the library.
				FinalLinkEnvironment.ExcludedLibraries.Add("LIBCMT");

				// Debug builds use XAPILIBD, so suppress any references to the other versions of the library.
				FinalLinkEnvironment.ExcludedLibraries.Add("xapilib");
				FinalLinkEnvironment.ExcludedLibraries.Add("xapilibi");

                // Compile and link with XAudio2
                FinalLinkEnvironment.AdditionalLibraries.Add( "xaudiod2.lib" );
            }
			else
			{
				if (Configuration == UnrealTargetConfiguration.Release)
				{
					FinalLinkEnvironment.AdditionalLibraries.Add("x3daudio.lib");
					// Release builds use xapilibi, so suppress any references to the other versions of the library.
					FinalLinkEnvironment.ExcludedLibraries.Add("xapilib");
					FinalLinkEnvironment.ExcludedLibraries.Add("xapilibd");
				}
				else
				{
					FinalLinkEnvironment.AdditionalLibraries.Add("x3daudioltcg.lib");
					// Shipping builds use xapilib, so suppress any references to the other versions of the library.
					FinalLinkEnvironment.ExcludedLibraries.Add("xapilibi");
					FinalLinkEnvironment.ExcludedLibraries.Add("xapilibd");
				}

                FinalLinkEnvironment.AdditionalLibraries.Add( "xmcore.lib" );
                FinalLinkEnvironment.AdditionalLibraries.Add( "xgraphics.lib" );
				FinalLinkEnvironment.AdditionalLibraries.Add("xboxkrnl.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("xnet.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("xonline.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("xmp.lib");

                // Compile and link with XAudio2
             //   FinalLinkEnvironment.AdditionalLibraries.Add( "xaudio2.lib" );

				// Release and Shipping builds use LIBCMT, so suppress any references to the other versions of the library.
				FinalLinkEnvironment.ExcludedLibraries.Add("LIBCMTD");
			}

            FinalLinkEnvironment.AdditionalLibraries.Add( "XAPOFX.lib" );

			// Set up the part of the environment that is used on both PC and Xbox360.
			SetUpWin32AndXbox360Environment();

			// Add the Xbox360-specific projects.
			NonGameProjects.Add( new UE3ProjectDesc( "Xenon/Xenon.vcproj" ) );
            NonGameProjects.Add( new UE3ProjectDesc( "Xenon/XeCore/XenonCore.vcproj" ) );
            NonGameProjects.Add( new UE3ProjectDesc( "Xenon/XeD3DDrv/XenonD3DDrv.vcproj" ) );
            NonGameProjects.Add( new UE3ProjectDesc( "Xenon/XeDrv/XenonDrv.vcproj" ) );
            NonGameProjects.Add( new UE3ProjectDesc( "Xenon/XeEngine/XenonEngine.vcproj" ) );
            NonGameProjects.Add( new UE3ProjectDesc( "Xenon/XeLaunch/XenonLaunch.vcproj" ) );
			NonGameProjects.Add( new UE3ProjectDesc( "OnlineSubsystemLive/OnlineSubsystemLive.vcproj" ) );
         //   NonGameProjects.Add( new UE3ProjectDesc( "XAudio2/XAudio2.vcproj" ) );

			// Add library search paths for libraries included via pragma comment (lib)
			FinalLinkEnvironment.LibraryPaths.Add("../External/Bink/lib/Xenon/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/GamersSDK/4.2.1/lib/Xenon/");
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				FinalLinkEnvironment.LibraryPaths.Add("../External/SpeedTreeRT/lib/Xenon/Debug/");
			}
			else
			{
				FinalLinkEnvironment.LibraryPaths.Add("../External/SpeedTreeRT/lib/Xenon/Release/");
			}
			FinalLinkEnvironment.LibraryPaths.Add("../External/DECtalk464/lib/Xenon/");
			FinalLinkEnvironment.LibraryPaths.Add("Xenon/External/FaceFX/FxSDK/lib/xbox360/vs8/");
			FinalLinkEnvironment.LibraryPaths.Add("Xenon/External/PhysX/SDKs/lib/xbox360/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/XNAContentServer/lib/xbox/");
		}

		IEnumerable<FileItem> GetXbox360OutputItems()
		{
			// Verify that the user has specified the expected output extension.
			if (Path.GetExtension(OutputPath).ToUpperInvariant() != ".XEX")
			{
				throw new BuildException("Unexpected output extension: {0} instead of .XEX", Path.GetExtension(OutputPath));
			}

			// Put the non-executable output files (PDB, import library, etc) in the Binaries/Xenon/lib/<configuration> directory.
			string RelativeOutputDirectory;
			switch (Configuration)
			{
				case UnrealTargetConfiguration.Debug:
					RelativeOutputDirectory = "Binaries/Xenon/lib/XeDebug"; break;
				case UnrealTargetConfiguration.Release:
					RelativeOutputDirectory = "Binaries/Xenon/lib/XeRelease"; break;
				case UnrealTargetConfiguration.Shipping:
					RelativeOutputDirectory = "Binaries/Xenon/lib/XeReleaseLTCG"; break;
				case UnrealTargetConfiguration.ShippingDebugConsole:
					RelativeOutputDirectory = "Binaries/Xenon/lib/XeReleaseLTCG-DebugConsole"; break;
				default:
					throw new BuildException("Invalid configuration specified for the Win32 platform: {0}", Configuration);
			}
			FinalLinkEnvironment.OutputDirectory = Path.Combine(Path.GetDirectoryName(OutputPath), RelativeOutputDirectory);

			// Link the EXE file.
			FinalLinkEnvironment.OutputFilePath = Path.ChangeExtension(OutputPath, ".exe");
			FileItem EXEFile = FinalLinkEnvironment.LinkExecutable();

			// Convert the EXE to an Xbox XEX file.
			FileItem OutputFile = Xbox360ToolChain.CreateXEXFromEXE(
				EXEFile,
				OutputPath,
				Game.GetXEXConfigFile()
				);

			if (UE3BuildConfiguration.bDeployAfterCompile)
			{
				// Run CookerSync to copy the XEX to the console.
				Action CookerSyncAction = new Action();
				CookerSyncAction.CommandPath = "../../Binaries/CookerSync.exe";
				CookerSyncAction.CommandArguments = string.Format("{0} -p Xbox360", Game.GetGameName());
				CookerSyncAction.bCanExecuteRemotely = false;
				CookerSyncAction.StatusDescription = "Deploying files to devkit...";
				CookerSyncAction.WorkingDirectory = "../../Binaries";
				CookerSyncAction.PrerequisiteItems.Add(OutputFile);

				// Use the -AllowNoTarget to disable the error CookerSync would otherwise throw when no default target to copy to is configured.
				CookerSyncAction.CommandArguments += " -AllowNoTarget";

				// Use non-interactive mode so CookerSync just fails if the target devkit isn't turned on.
				CookerSyncAction.CommandArguments += " -NonInter";

				// Don't regenerate the TOC to prevent conflicts when compiling at the same time as cooking.
				CookerSyncAction.CommandArguments += " -notoc";

				// Use the UnrealBuildTool tagset to only copy the executables.
				CookerSyncAction.CommandArguments += " -x UnrealBuildTool";

				// use the default target as we aren't supplying a target
				CookerSyncAction.CommandArguments += " -deftarget";

				// reboot targets
				CookerSyncAction.CommandArguments += " -o";

				// Give the CookerSync action a produced file that it doesn't write to, ensuring that it will always run.
				OutputFile = FileItem.GetItemByPath("Dummy.txt");
				CookerSyncAction.ProducedItems.Add(OutputFile);
			}
			
			// Return a list of the output files.
			return new FileItem[]
			{
				OutputFile
			};
		}
	}
}