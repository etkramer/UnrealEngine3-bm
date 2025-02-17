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
		void SetUpWin32Environment()
        {            
            GlobalCPPEnvironment.Definitions.Add("_WINDOWS=1");
			GlobalCPPEnvironment.Definitions.Add("WIN32=1");
			GlobalCPPEnvironment.Definitions.Add("_WIN32_WINNT=0x0502");
			GlobalCPPEnvironment.Definitions.Add("WINVER=0x0502");

			// Add the Windows public module include paths.
			GlobalCPPEnvironment.IncludePaths.Add("OnlineSubsystemLive/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("OnlineSubsystemGameSpy/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("OnlineSubsystemPC/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("WinDrv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("D3D9Drv/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("D3D10Drv/Inc");
           // GlobalCPPEnvironment.IncludePaths.Add( "XAudio2/Inc" );
			GlobalCPPEnvironment.IncludePaths.Add("Launch/Inc");
			GlobalCPPEnvironment.IncludePaths.Add("$(CommonProgramFiles)");

// Atlas-Modif [Begin]
			// Compile and link with Atlas
            //FinalLinkEnvironment.LibraryPaths.Add("../../../Atlas/lib");
            //FinalLinkEnvironment.LibraryPaths.Add("../../../Atlas/external/lib");
            //if (Configuration == UnrealTargetConfiguration.Debug)
            //{
            //    FinalLinkEnvironment.AdditionalLibraries.Add("AtlasLibBase_D.lib");
            //    FinalLinkEnvironment.AdditionalLibraries.Add("AtlasLibCommon_D.lib");
            //    FinalLinkEnvironment.AdditionalLibraries.Add("AtlasLibClient_D.lib");
            //    FinalLinkEnvironment.AdditionalLibraries.Add("libboost_system-vc80-mt-gd-1_35.lib");
            //    FinalLinkEnvironment.AdditionalLibraries.Add("log4boost_Debug.lib");
            //}
            //else
            //{
            //    //FinalLinkEnvironment.AdditionalLibraries.Add("AtlasLibBase.lib");
            //    FinalLinkEnvironment.AdditionalLibraries.Add("AtlasLibCommon.lib");
            //    FinalLinkEnvironment.AdditionalLibraries.Add("AtlasLibClient.lib");
            //    FinalLinkEnvironment.AdditionalLibraries.Add("libboost_system-vc80-mt-1_35.lib");
            //    FinalLinkEnvironment.AdditionalLibraries.Add("log4boost_Release.lib");
            //}
// Atlas-Modif [End]

			// Compile and link with GameSpy.
			SetUpGameSpyEnvironment();

            // Compile and link with Games for Windows Live if desired
            SetUpWindowsLiveEnvironment();

            // Compile and link with libPNG
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/libPNG");
			GlobalCPPEnvironment.Definitions.Add("PNG_NO_FLOATING_POINT_SUPPORTED=1");

			// Compile and link with lzo.
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/lzopro/include");
			FinalLinkEnvironment.LibraryPaths.Add("../External/lzopro/lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("lzopro.lib");

			// Compile and link with zlib.
			GlobalCPPEnvironment.SystemIncludePaths.Add("../External/zlib/inc");
			FinalLinkEnvironment.LibraryPaths.Add("../External/zlib/Lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("zlib.lib");

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

            // Required for 3D spatialisation in XAudio2
            FinalLinkEnvironment.AdditionalLibraries.Add( "X3DAudio.lib" );
           // FinalLinkEnvironment.AdditionalLibraries.Add( "xapobase.lib" );
           // FinalLinkEnvironment.AdditionalLibraries.Add( "XAPOFX.lib" );

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
				FinalLinkEnvironment.LibraryPaths.Add( "../External/FCollada-3.05B/FCollada/Output/Release Unicode Win32" );
			}
			FinalLinkEnvironment.AdditionalLibraries.Add("FColladaU.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("FArchiveXML.lib");

            FinalLinkEnvironment.LibraryPaths.Add("../External/nvTextureTools2/lib");

			// Compile and link with DirectX.
// Mars-Merge [Begin]
            //GlobalCPPEnvironment.SystemIncludePaths.Add("$(DXSDK_DIR)/include");
            //FinalLinkEnvironment.LibraryPaths.Add("$(DXSDK_DIR)/Lib/x86");
            GlobalCPPEnvironment.SystemIncludePaths.Add("$(DXSDK_DIR_August2008)/include");
            FinalLinkEnvironment.LibraryPaths.Add("$(DXSDK_DIR_August2008)/Lib/x86");
// Mars-Merge [End]
			FinalLinkEnvironment.AdditionalLibraries.Add("d3d10.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("d3dx10.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("dxgi.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("d3d9.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("dinput8.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("dxguid.lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("XInput.lib");
			FinalLinkEnvironment.DelayLoadDLLs.Add("dxgi.dll");
			FinalLinkEnvironment.DelayLoadDLLs.Add("d3d10.dll");

            FinalLinkEnvironment.DelayLoadDLLs.Add("../External/wxWidgets/lib/vc_dll");
            //FinalLinkEnvironment.DelayLoadDLLs.Add("../External/FaceFX/FxSDK/lib/win32/vs8");

			// Compile and link with NVIDIA Texture Tools.
            GlobalCPPEnvironment.SystemIncludePaths.Add("../External/nvTextureTools-2.0.4/src/src");
			FinalLinkEnvironment.LibraryPaths.Add("../External/nvTextureTools-2.0.4/lib");
			FinalLinkEnvironment.AdditionalLibraries.Add("nvtt.lib");

			// Compile and link with NVIDIA triangle strip generator.
			FinalLinkEnvironment.LibraryPaths.Add("../External/nvTriStrip/Lib");
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				FinalLinkEnvironment.AdditionalLibraries.Add("nvTriStripD.lib");
			}
			else
			{
				FinalLinkEnvironment.AdditionalLibraries.Add("nvTriStrip.lib");
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
            FinalLinkEnvironment.AdditionalLibraries.Add("../External/FaceFX/FxSDK/lib/win32/vs8/FxSDK_Unreal_Release.lib");

			// Compile and with OpenAL.
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
			NonGameProjects.Add( new UE3ProjectDesc( "D3D10Drv/D3D10Drv.vcproj" ) );
			NonGameProjects.Add( new UE3ProjectDesc( "D3D9Drv/D3D9Drv.vcproj" ) );
			NonGameProjects.Add( new UE3ProjectDesc( "WinDrv/WinDrv.vcproj" ) );
           // NonGameProjects.Add( new UE3ProjectDesc( "XAudio2/XAudio2.vcproj" ) );
            NonGameProjects.Add( new UE3ProjectDesc( "Editor/Editor.vcproj" ) );
			NonGameProjects.Add( new UE3ProjectDesc( "UnrealEd/UnrealEd.vcproj" ) );

			// Add library paths for libraries included via pragma comment(lib)
			FinalLinkEnvironment.LibraryPaths.Add("../External/AgPerfMon/lib/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/Bink/lib/Win32/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/GamersSDK/4.2.1/lib/Win32/");
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				FinalLinkEnvironment.LibraryPaths.Add("../External/SpeedTreeRT/lib/PC/Debug/");
			}
			else
			{
				FinalLinkEnvironment.LibraryPaths.Add("../External/SpeedTreeRT/lib/PC/Release/");
			}
			FinalLinkEnvironment.LibraryPaths.Add("../External/DECtalk464/lib/Win32");
			FinalLinkEnvironment.LibraryPaths.Add("../External/FaceFX/FxSDK/lib/win32/vs8/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/PhysX/SDKs/lib/win32/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/PhysX/Nxd/lib/win32/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/libPNG/lib/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/nvDXT/Lib/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/ConvexDecomposition/Lib/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/FaceFX/FxCG/lib/win32/vs8/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/FaceFX/FxAnalysis/lib/win32/vs8/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/FaceFX/Studio/External/libresample-0.1.3/");
			FinalLinkEnvironment.LibraryPaths.Add("../External/PhysX/SDKs/TetraMaker/lib/win32/");
			if (Configuration == UnrealTargetConfiguration.Debug)
			{
				FinalLinkEnvironment.LibraryPaths.Add("../External/TestTrack/Lib/Debug/");
			}
			else
			{
				FinalLinkEnvironment.LibraryPaths.Add("../External/TestTrack/Lib/Release/");
			}


			// Setup CLR environment
			if( BuildConfiguration.bAllowManagedCode )
			{
				// Set a global C++ definition so that any CLR-based features can turn themselves on
				GlobalCPPEnvironment.Definitions.Add( "WITH_MANAGED_CODE=1" );


				// Add C++/CLI projects
				NonGameProjects.Add( new UE3ProjectDesc( "UnrealEdCLR/UnrealEdCLR.vcproj", CPPCLRMode.CLREnabled ) );


				// Add required .NET Framework assemblies
				{
					String DotNet30AssemblyPath =
						Environment.GetFolderPath( Environment.SpecialFolder.ProgramFiles ) +
						"/Reference Assemblies/Microsoft/Framework/v3.0";
					String DotNet35AssemblyPath =
						Environment.GetFolderPath( Environment.SpecialFolder.ProgramFiles ) +
						"/Reference Assemblies/Microsoft/Framework/v3.5";

					GlobalCPPEnvironment.FrameworkAssemblyDependencies.Add( "System.dll" );
					GlobalCPPEnvironment.FrameworkAssemblyDependencies.Add( "System.Data.dll" );
					GlobalCPPEnvironment.FrameworkAssemblyDependencies.Add( "System.Xml.dll" );

					GlobalCPPEnvironment.FrameworkAssemblyDependencies.Add( DotNet30AssemblyPath + "/PresentationCore.dll" );
					GlobalCPPEnvironment.FrameworkAssemblyDependencies.Add( DotNet30AssemblyPath + "/PresentationFramework.dll" );
					GlobalCPPEnvironment.FrameworkAssemblyDependencies.Add( DotNet30AssemblyPath + "/WindowsBase.dll" );

//					GlobalCPPEnvironment.FrameworkAssemblyDependencies.Add( DotNet35AssemblyPath + "/System.Core.dll" );
				}


				// Add private assembly dependencies.  If any of these are changed, then CLR projects will
				// be forced to rebuild.
				{
					// The editor needs to be able to link against it's own .NET assembly dlls/exes.  For example,
					// this is needed so that we can reference the "UnrealEdCSharp" assembly.  At runtime, the
					// .NET loader will locate these using the settings specified in the target's app.config file
					String EditorDLLPath = Path.GetDirectoryName( OutputPath ) + "/Editor/";
					if( Configuration == UnrealTargetConfiguration.Debug )
					{
						EditorDLLPath += "Debug/";
					}
					else
					{
						EditorDLLPath += "Release/";
					}

					// Add "UnrealEdCSharp" private assembly as a global dependency for CLR-based files
					// @WPF: Really we should auto-detect these dependencies similar to how CPPHeaders.cs checks for #includes
					GlobalCPPEnvironment.PrivateAssemblyDependencies.Add( EditorDLLPath + "UnrealEdCSharp.dll" );
				}


				// Use of WPF in managed projects requires single-threaded apartment model for CLR threads
				FinalLinkEnvironment.AdditionalArguments += " /CLRTHREADATTRIBUTE:STA";
			}
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
			// Add a path suffix for G4WLive (all changes here should have matching changes in Core\Src\UnVcWin32.cpp)
			if (GlobalCPPEnvironment.Definitions.Contains( "WITH_PANORAMA=1" ) &&
				(Configuration == UnrealTargetConfiguration.Debug ||
				 Configuration == UnrealTargetConfiguration.Release ||
				 Configuration == UnrealTargetConfiguration.Shipping))
			{
				RelativeOutputDirectory += "-G4WLive";
			}
			FinalLinkEnvironment.OutputDirectory = Path.Combine(Path.GetDirectoryName(OutputPath),RelativeOutputDirectory);

			// Link the EXE file.
			FinalLinkEnvironment.OutputFilePath = OutputPath;
			FileItem EXEFile = FinalLinkEnvironment.LinkExecutable();

            // Do post build step for Windows Live if requested
            if (GlobalCPPEnvironment.Definitions.Contains("WITH_PANORAMA=1"))
            {
                File.SetAttributes(string.Format("{0}.cfg", EXEFile.AbsolutePath), FileAttributes.Normal);
                // Creates a cfg file so Live can load
                File.Copy(string.Format(".\\{0}Game\\Live\\LiveConfig.xml", Game.GetGameName()),
                    string.Format("{0}.cfg", EXEFile.AbsolutePath),true);
            }

            // Return a list of the output files.
			FileItem[] OutputFiles =
			{
				EXEFile
			};
			return OutputFiles;
		}
	}
}
