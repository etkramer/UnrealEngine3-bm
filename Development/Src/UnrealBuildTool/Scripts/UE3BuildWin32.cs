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
		void SetUpWin32Environment()
        {            
            GlobalCPPEnvironment.Definitions.Add("_WINDOWS=1");
			GlobalCPPEnvironment.Definitions.Add("WIN32=1");
			GlobalCPPEnvironment.Definitions.Add("_WIN32_WINNT=0x0500");
			GlobalCPPEnvironment.Definitions.Add("WINVER=0x0500");

			// Add the Windows public module include paths.
			GlobalCPPEnvironment.IncludePaths.Add("OnlineSubsystemLive/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("OnlineSubsystemPC/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("WinDrv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("D3D9Drv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("D3D10Drv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("ALAudio/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("Launch/Inc");

			// Compile and link with libPNG
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/libPNG");
			GlobalCPPEnvironment.Definitions.Add("PNG_NO_FLOATING_POINT_SUPPORTED=1");

			// Compile and link with lzo.
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/lzopro/include");
			FinalLinkEnvironment.AdditionalLibraries.Add("../External/lzopro/lib/lzopro.lib");

			// Compile and link with zlib.
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/zlib/inc");
			FinalLinkEnvironment.AdditionalLibraries.Add("../External/zlib/Lib/zlib.lib");

			// Compile and link with wxWidgets.
			GlobalCPPEnvironment.Definitions.Add("WXUSINGDLL");
			GlobalCPPEnvironment.Definitions.Add("wxUSE_NO_MANIFEST");
			GlobalCPPEnvironment.IncludePaths.Add("../External/wxWidgets/include");
			GlobalCPPEnvironment.IncludePaths.Add("../External/wxWidgets/lib/vc_dll");
			GlobalCPPEnvironment.IncludePaths.Add("../External/wxExtended/wxDockit/include");
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				// Use the debug wxWidgets libraries.
				FinalLinkEnvironment.LibraryPaths.Add("../External/wxWidgets/lib/vc_dll");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_core.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_aui.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_xrc.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_richtext.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_qa.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_media.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_html.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_adv.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_net.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28ud_xml.lib");
			}
			else
			{
				// Use the release wxWidgets libraries.
				FinalLinkEnvironment.LibraryPaths.Add("../External/wxWidgets/lib/vc_dll");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_core.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_aui.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_xrc.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_richtext.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_qa.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_media.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_html.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_adv.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_net.lib");
				FinalLinkEnvironment.AdditionalLibraries.Add("wxmsw28u_xml.lib");
			}

			// Explicitly exclude the MS C++ runtime libraries we're not using, to ensure other libraries we link with use the same
			// runtime library as the engine.
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				FinalLinkEnvironment.ExcludedLibraries.Add("MSVCRT");
				FinalLinkEnvironment.ExcludedLibraries.Add("MSVCPRT");
			}
			else
			{
				FinalLinkEnvironment.ExcludedLibraries.Add("MSVCRTD");
				FinalLinkEnvironment.ExcludedLibraries.Add("MSVCPRTD");
			}
			FinalLinkEnvironment.ExcludedLibraries.Add("LIBC");
			FinalLinkEnvironment.ExcludedLibraries.Add("LIBCMT");
			FinalLinkEnvironment.ExcludedLibraries.Add("LIBCPMT");
			FinalLinkEnvironment.ExcludedLibraries.Add("LIBCP");
			FinalLinkEnvironment.ExcludedLibraries.Add("LIBCD");
			FinalLinkEnvironment.ExcludedLibraries.Add("LIBCMTD");
			FinalLinkEnvironment.ExcludedLibraries.Add("LIBCPMTD");
			FinalLinkEnvironment.ExcludedLibraries.Add("LIBCPD");

			// Add the library used for the delayed loading of DLLs.
			FinalLinkEnvironment.AdditionalLibraries.Add("delayimp.lib");

			// Compile and link with FCollada.
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/FCollada-3.05B/FCollada/LibXML/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/FCollada-3.05B/FCollada");
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				FinalLinkEnvironment.LibraryPaths.Add("../External/FCollada-3.05B/FCollada/Output/Debug Unicode Win32");
			}
			else
			{
				FinalLinkEnvironment.LibraryPaths.Add("../External/FCollada-3.05B/FCollada/Output/Release Unicode Win32");
			}
			FinalLinkEnvironment.AdditionalLibraries.Add("FColladaU.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("FArchiveXML.lib");

			// Compile and link with DirectX.
			GlobalCPPEnvironment.SystemIncludePaths.Add("$(DXSDK_DIR)/include");
			FinalLinkEnvironment.LibraryPaths.Add("$(DXSDK_DIR)/Lib/x86");
			FinalLinkEnvironment.AdditionalLibraries.Add("d3d10.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("d3dx10.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("dxgi.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("d3d9.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("dinput8.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("dxguid.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("XInput.lib");
			FinalLinkEnvironment.DelayLoadDLLs.Add("dxgi.dll");
			FinalLinkEnvironment.DelayLoadDLLs.Add("d3d10.dll");

			// Compile and link with NVIDIA Texture Tools.
			FinalLinkEnvironment.DelayLoadDLLs.Add("nvtt.dll");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/nvTextureTools2/src/project/vc8");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/nvTextureTools2/src/src");
			FinalLinkEnvironment.AdditionalLibraries.Add("../External/nvTextureTools2/lib/nvtt.lib");

			// Compile and link with NVIDIA triangle strip generator.
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				FinalLinkEnvironment.AdditionalLibraries.Add("../External/nvTriStrip/Lib/nvTriStripD.lib");
			}
			else
			{
				FinalLinkEnvironment.AdditionalLibraries.Add("../External/nvTriStrip/Lib/nvTriStrip.lib");
			}

			// Compile and link with Ogg/Vorbis.
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/libogg-1.1.3/include");
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/libvorbis-1.1.2/include");
			FinalLinkEnvironment.LibraryPaths.Add("../External/libvorbis-1.1.2/win32/Release");
			FinalLinkEnvironment.LibraryPaths.Add("../External/libogg-1.1.3/win32/Release");
			FinalLinkEnvironment.AdditionalLibraries.Add("vorbis.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("vorbisenc.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("vorbisfile.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("ogg.lib");

			// Compile and link with OpenAL.
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/OpenAL-1.1/include");

			// Compile and link with Win32 API libraries.
			FinalLinkEnvironment.AdditionalLibraries.Add("rpcrt4.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("wsock32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("dbghelp.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("Comdlg32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("comctl32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("Winmm.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("kernel32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("user32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("gdi32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("winspool.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("comdlg32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("advapi32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("shell32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("ole32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("oleaut32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("uuid.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("odbc32.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("odbccp32.lib");

			// Set a definition if the Xbox SDK is installed.
			if (Environment.GetEnvironmentVariable("XEDK") != null)
			{
				// Compile and link with the Xbox 360 SDK Win32 API.
				GlobalCPPEnvironment.Definitions.Add("XDKINSTALLED=1");
				GlobalCPPEnvironment.SystemIncludePaths.Add("$(XEDK)/include/win32/vs2005");
				FinalLinkEnvironment.LibraryPaths.Add("$(XEDK)/lib/win32/vs2005");
			}
			else
			{
				GlobalCPPEnvironment.Definitions.Add("XDKINSTALLED=0");
			}

			// Set up the part of the environment that is used on both PC and Xbox360.
			SetUpWin32AndXbox360Environment();

			// Add the Win32-specific projects.
			NonGameProjects.Add("D3D10Drv/D3D10Drv.vcproj");
			NonGameProjects.Add("D3D9Drv/D3D9Drv.vcproj");
			NonGameProjects.Add("WinDrv/WinDrv.vcproj");
			NonGameProjects.Add("ALAudio/ALAudio.vcproj");
			NonGameProjects.Add("Editor/Editor.vcproj");
			NonGameProjects.Add("UnrealEd/UnrealEd.vcproj");
		}

		void SetUpWin32AndXbox360Environment()
		{
			// Link with either the debug or release D3DX9 library.
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				FinalLinkEnvironment.AdditionalLibraries.Add("d3dx9d.lib");
			}
			else
			{
				FinalLinkEnvironment.AdditionalLibraries.Add("d3dx9.lib");
			}
		}

		IEnumerable<FileItem> GetWin32OutputItems()
		{
			// Verify that the user has specified the expected output extension.
			if (Path.GetExtension(OutputPath).ToUpperInvariant() != ".EXE")
			{
				throw new BuildException("Unexpected output extension: {0} instead of .EXE", Path.GetExtension(OutputPath));
			}

			// Put the non-executable output files (PDB, import library, etc) in the Binaries/lib/<configuration> directory.
			string RelativeOutputDirectory;
			switch(Configuration)
			{
				case UnrealTargetConfiguration.Debug:				RelativeOutputDirectory = "lib/Debug"; break;
				case UnrealTargetConfiguration.Release:				RelativeOutputDirectory = "lib/Release"; break;
				case UnrealTargetConfiguration.Shipping:			RelativeOutputDirectory = "lib/ReleaseShippingPC"; break;
				case UnrealTargetConfiguration.ShippingDebugConsole:RelativeOutputDirectory = "lib/ReleaseShippingPC-DebugConsole"; break;
				default:
					throw new BuildException("Invalid configuration specified for the Win32 platform: {0}",Configuration);
			}
			FinalLinkEnvironment.OutputDirectory = Path.Combine(Path.GetDirectoryName(OutputPath),RelativeOutputDirectory);

			// Link the EXE file.
			FinalLinkEnvironment.OutputFilePath = OutputPath;
			FileItem EXEFile = FinalLinkEnvironment.LinkExecutable();

			// Return a list of the output files.
			FileItem[] OutputFiles =
			{
				EXEFile
			};
			return OutputFiles;
		}
	}
}