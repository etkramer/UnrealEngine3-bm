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
		void SetUpPS3Environment()
		{            
			GlobalCPPEnvironment.Definitions.Add("PS3_NO_TLS=0");
			GlobalCPPEnvironment.Definitions.Add("PS3=1");
			GlobalCPPEnvironment.Definitions.Add("_PS3=1");
			GlobalCPPEnvironment.Definitions.Add("REQUIRES_ALIGNED_ACCESS=1");
			GlobalCPPEnvironment.Definitions.Add("EXCEPTIONS_DISABLED=1");

			// Compile and link with GameSpy.
			SetUpGameSpyEnvironment();

			GlobalCPPEnvironment.IncludePaths.Add("PS3");
			GlobalCPPEnvironment.IncludePaths.Add("PS3/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("PS3/SPU/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("PS3/RHI/Inc");

			GlobalCPPEnvironment.IncludePaths.Add("OnlineSubsystemGameSpy/Inc");

			GlobalCPPEnvironment.SystemIncludePaths.Add("PS3/External/PhysX/SDKs/PhysXLoader/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("PS3/External/PhysX/SDKs/Foundation/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("PS3/External/PhysX/SDKs/Physics/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("PS3/External/PhysX/SDKs/Cooking/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("PS3/External/PhysX/SDKs/PhysXExtensions/include");

			GlobalCPPEnvironment.SystemIncludePaths.Add("$(CELL_SDK_WIN)/target");
			GlobalCPPEnvironment.SystemIncludePaths.Add("$(CELL_SDK_WIN)/target/common/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("$(CELL_SDK_WIN)/target/ppu/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("$(SN_PS3_PATH)/include/sn");

			FinalLinkEnvironment.LibraryPaths.Add("../External/GamersSDK/4.2.1/lib/PS3");
			FinalLinkEnvironment.AdditionalLibraries.Add("VoiceCmds");

			// Link with the system libraries.
			FinalLinkEnvironment.LibraryPaths.Add("$(CELL_SDK_WIN)/target/ppu/lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("m");					// Math
			FinalLinkEnvironment.AdditionalLibraries.Add("mic_stub");			// Microphone
			FinalLinkEnvironment.AdditionalLibraries.Add("net_stub");			// Networking
			FinalLinkEnvironment.AdditionalLibraries.Add("netctl_stub");
			FinalLinkEnvironment.AdditionalLibraries.Add("padfilter");			// Controller
			FinalLinkEnvironment.AdditionalLibraries.Add("io_stub");
			FinalLinkEnvironment.AdditionalLibraries.Add("mstreamSPURSMP3");	// MP3 SPU support
			FinalLinkEnvironment.AdditionalLibraries.Add("fs_stub");			// Filesystem
			FinalLinkEnvironment.AdditionalLibraries.Add("sysutil_stub");		// System utils
			FinalLinkEnvironment.AdditionalLibraries.Add("edgezlib");			// SPU zlib support
			FinalLinkEnvironment.AdditionalLibraries.Add("sync_stub");
			FinalLinkEnvironment.AdditionalLibraries.Add("spurs_stub");			// SPURS
			FinalLinkEnvironment.AdditionalLibraries.Add("sysmodule_stub");
			FinalLinkEnvironment.AdditionalLibraries.Add("ovis_stub");
			FinalLinkEnvironment.AdditionalLibraries.Add("rtc_stub");			// Real-time clock
			FinalLinkEnvironment.AdditionalLibraries.Add("audio_stub");			// Audio
			FinalLinkEnvironment.AdditionalLibraries.Add("sysutil_np_stub");	// Network start dialog utility
			FinalLinkEnvironment.AdditionalLibraries.Add("l10n_stub");			// Internationalization
			FinalLinkEnvironment.AdditionalLibraries.Add("pthread");			// POSIX threads

			if (Configuration != UnrealTargetConfiguration.Shipping)
			{
				FinalLinkEnvironment.AdditionalLibraries.Add("lv2dbg_stub");	// Non-shipping exception-handling
			}

			FinalLinkEnvironment.LibraryPaths.Add("../External/Bink/lib/PS3");
			FinalLinkEnvironment.AdditionalLibraries.Add("BinkPS3");

			FinalLinkEnvironment.LibraryPaths.Add("PS3/External/zlib/lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("z");

			FinalLinkEnvironment.LibraryPaths.Add("PS3/External/PhysX/SDKs/lib/PS3");
			FinalLinkEnvironment.AdditionalLibraries.Add("PhysX");
			FinalLinkEnvironment.AdditionalLibraries.Add("PhysXExtensions");

			// Compile and link with GCM.
			switch(UE3BuildConfiguration.PS3GCMType)
			{
				case PS3GCMType.Release:
					// Use standard release GCM libraries.
					FinalLinkEnvironment.AdditionalLibraries.Add("gcm_cmd");
					break;
				case PS3GCMType.Debug:
					// Use debuggable GCM libraries.
					GlobalCPPEnvironment.Definitions.Add("CELL_GCM_DEBUG=1");
					FinalLinkEnvironment.AdditionalLibraries.Add("gcm_cmddbg");
					break;
			};
			FinalLinkEnvironment.AdditionalLibraries.Add("gcm_sys_stub");

            if ((Configuration == UnrealTargetConfiguration.Release) || (Configuration == UnrealTargetConfiguration.Shipping))
            {
                FinalLinkEnvironment.LibraryPaths.Add("../External/SpeedTreeRT/lib/PS3/Release");
                FinalLinkEnvironment.AdditionalLibraries.Add("SpeedTreeRT");
            }
            else
            {
                FinalLinkEnvironment.LibraryPaths.Add("../External/SpeedTreeRT/lib/PS3/Debug");
                FinalLinkEnvironment.AdditionalLibraries.Add("SpeedTreeRT_d");
            }

			FinalLinkEnvironment.LibraryPaths.Add("PS3/External/FaceFx/lib");

			if (Configuration == UnrealTargetConfiguration.Shipping)
			{
				FinalLinkEnvironment.AdditionalLibraries.Add("FaceFX_FinalRelease");
			}
			else
			{
				FinalLinkEnvironment.AdditionalLibraries.Add("FaceFX");
			}

			// Add libNull as the last library path, so if any of the optional libraries from above don't exist, they will be found in libNull.
			FinalLinkEnvironment.LibraryPaths.Add("PS3/libNull");

			// Add the PS3-specific projects.
			NonGameProjects.Add("PS3/PS3.vcproj");
			NonGameProjects.Add("OnlineSubsystemGamespy/OnlineSubsystemGamespy.vcproj");
		}

		void PS3AddDataFileToExecutable(string InputDataFilePath/*,string SectionName*/)
		{
			// Add the object file as an input to the link environment.
			FinalLinkEnvironment.InputFiles.Add(FileItem.GetItemByPath(InputDataFilePath));
		}

		IEnumerable<FileItem> GetPS3OutputItems()
		{
			// Verify that the user has specified the expected output extension.
			if (Path.GetExtension(OutputPath).ToUpperInvariant() != ".ELF")
			{
				throw new BuildException("Unexpected output extension: {0} instead of .ELF", Path.GetExtension(OutputPath));
			}

			// Link with the precompiled mstream object files.
			string SCE_PS3_ROOT = Environment.GetEnvironmentVariable("SCE_PS3_ROOT");
			PS3AddDataFileToExecutable(Path.Combine(SCE_PS3_ROOT, "target/spu/lib/pic/multistream/mstream_dsp_i3dl2.ppu.o"));
			PS3AddDataFileToExecutable(Path.Combine(SCE_PS3_ROOT, "target/spu/lib/pic/multistream/mstream_dsp_filter.ppu.o"));
			PS3AddDataFileToExecutable(Path.Combine(SCE_PS3_ROOT, "target/spu/lib/pic/multistream/mstream_dsp_para_eq.ppu.o"));

			// Link all the modules into a .xelf file in the final output directory.
			// The .xelf is not the final .elf output, but is kept around for accessing symbols in UnrealConsole regardless of if the
			// final .elf has been stripped of its symbols.
			string XELFPath = Path.ChangeExtension(OutputPath, ".xelf");
			FinalLinkEnvironment.OutputFilePath = XELFPath;
			FileItem PreEncryptedELFFile = FinalLinkEnvironment.LinkExecutable();

			if (Configuration == UnrealTargetConfiguration.Shipping)
			{
				// Strip the debug info from the .xelf file and write the stripped executable to a .strelf file in the intermediate directory.
				string STRELFPath = Path.Combine(GlobalCPPEnvironment.OutputDirectory, Path.GetFileNameWithoutExtension(OutputPath) + ".strelf");
				PreEncryptedELFFile = PS3ToolChain.StripDebugInfoFromExecutable(PreEncryptedELFFile, STRELFPath);
			}

			// Encrypt the executable, and write the result to a .elf file in the final output directory.
			FileItem ELFFile = PS3ToolChain.EncryptExecutable(PreEncryptedELFFile,OutputPath);

			// Return a list of the output files.
			FileItem[] OutputFiles =
			{
				ELFFile
			};
			return OutputFiles;
		}
	}
}
