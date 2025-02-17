using System;
using System.Runtime.InteropServices;
using System.Security;
using System.Security.Permissions;
using System.IO;
using Microsoft.Win32;


namespace Epic.UnrealDebugger2005
{
    /// <summary>
    /// 
    /// </summary>       

    public class WTGlobals
    {
        public static string RegStr = "Software\\Epic\\Unreal";

        public static string WT_DLLPATH; // = "...\\Binaries\\WTDebugger\\Debug\\";
        public static string WT_SDK_DLL; // = WT_DLLPATH + "UCDebuggerSDK.dll";
        public static string WT_INTERFACEDLL; // = WT_DLLPATH + "UCDebuggerSocket.dll";
        public static string WT_WATCHFILE; // = WT_DLLPATH + "WatchFile.txt";
        public static string WT_ATTACHFILE; // = WT_DLLPATH + "WatchFile.txt";
		public static string WT_COMMANDLINEARGS;	// = -autodebug -vadebug;
        public static string WT_GAMEPATH; // = "C:\\Unreal\\UnrealEngine3\\Binaries\\DEBUG-ExampleGame.exe";
        public static UCDebuggerPkg gUCDebuggerPkg = null;
        public static bool gAttached = false;

        public WTGlobals()
        {
        }

        public static void SetGameExe(string gamePath)
        {
            WT_GAMEPATH = gamePath;
            FileInfo game = new FileInfo(WT_GAMEPATH);
            WT_DLLPATH = game.Directory.FullName + "\\WTDebugger\\";
            String tmpPath = System.IO.Path.GetTempPath() + "\\UCDebugger";
            System.IO.Directory.CreateDirectory(tmpPath);
            WT_ATTACHFILE = tmpPath + "\\attach.txt";
            WT_INTERFACEDLL = tmpPath + "UCDebuggerSocket.dll";
            WT_WATCHFILE = tmpPath + "\\WatchFile.txt";
            WT_SDK_DLL = WT_DLLPATH + "UCDebuggerSDK.dll";
        }
    }
}
