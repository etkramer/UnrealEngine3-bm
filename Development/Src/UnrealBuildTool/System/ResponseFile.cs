/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace UnrealBuildTool
{
	class ResponseFile
	{
		/** Creates a file from a list of strings; each string is placed on a line in the file. */
		public static string Create(List<string> Lines)
		{
			string TempFileName = Path.GetTempFileName();
			using (FileStream ResponseFileStream = new FileStream(TempFileName,FileMode.Create,FileAccess.Write))
			{
				using (StreamWriter StreamWriter = new StreamWriter(ResponseFileStream))
				{
					foreach (string Line in Lines)
					{
						StreamWriter.WriteLine(Line);
					}
				}
			}
			return TempFileName;
		}
	}
}
