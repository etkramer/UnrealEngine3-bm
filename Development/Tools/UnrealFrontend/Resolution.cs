/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealFrontend
{
	public struct Resolution
	{
		int mWidth;
		int mHeight;

		public int Width
		{
			get { return mWidth; }
			set
			{
				if(value > 0)
				{
					mWidth = value;
				}
			}
		}

		public int Height
		{
			get { return mHeight; }
			set
			{
				if(value > 0)
				{
					mHeight = value;
				}
			}
		}

		public Resolution(int Width, int Height)
		{
			this.mWidth = Width;
			this.mHeight = Height;
		}

		public static bool TryParse(string Str, out Resolution Res)
		{
			Res = new Resolution(1280, 720);
			string[] Parts = Str.Split('x');

			if(Parts.Length != 2)
			{
				return false;
			}

			int TempWidth;
			int TempHeight;

			if(!int.TryParse(Parts[0], out TempWidth))
			{
				return false;
			}

			if(!int.TryParse(Parts[1], out TempHeight))
			{
				return false;
			}

			Res.mWidth = TempWidth;
			Res.mHeight = TempHeight;

			return true;
		}

		public override string ToString()
		{
			return string.Format("{0}x{1}", mWidth.ToString(), mHeight.ToString());
		}
	}
}
