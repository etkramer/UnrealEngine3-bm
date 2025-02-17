using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace UnrealConsole.Main
{
	[StructLayout(LayoutKind.Sequential)]
	public struct POINT
	{
		public int X;
		public int Y;

		public POINT(int x, int y)
		{
			this.X = x;
			this.Y = y;
		}

		public static implicit operator System.Drawing.Point(POINT p)
		{
			return new System.Drawing.Point(p.X, p.Y);
		}

		public static implicit operator POINT(System.Drawing.Point p)
		{
			return new POINT(p.X, p.Y);
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct MSG
	{
		public IntPtr hwnd;
		public uint message;
		public IntPtr wParam;
		public IntPtr lParam;
		public uint time;
		public POINT pt;
	}

	public static class Win32Interop
	{
		// Constants from the Platform SDK.
		public const int EM_SETEVENTMASK = 1073;
		public const int EM_GETSCROLLPOS = 0x0400 + 221;
		public const int EM_SETSCROLLPOS = 0x0400 + 222;
		public const int WM_SETREDRAW = 11;
		public const int WM_KEYDOWN = 0x100;
		public const int WM_KEYUP = 0x101;
		public const int WM_ACTIVATEAPP = 0x1c;


		// import the SendMessage function so we can send windows messages to the control
		[DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = false)]
		public static extern IntPtr SendMessage(HandleRef hWnd, uint msg, IntPtr wParam, ref POINT lParam);

		[DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = false)]
		public static extern IntPtr SendMessage(HandleRef hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

		[DllImport("user32.dll")]
		//[return: MarshalAs(UnmanagedType.Bool)]
		public static extern int GetMessage(out MSG lpMsg, IntPtr hWnd, uint wMsgFilterMin, uint wMsgFilterMax);

		[DllImport("user32.dll")]
		public static extern bool TranslateMessage([In] ref MSG lpMsg);

		[DllImport("user32.dll")]
		public static extern IntPtr DispatchMessage([In] ref MSG lpmsg);

		[DllImport("user32.dll")]
		public static extern void PostQuitMessage(int nExitCode);

	}
}
