/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Xml;
using System.Xml.Serialization;

namespace UnrealFrontend
{
	/// <summary>
	/// Summary description for CookerSettings.
	/// </summary>
	public class CookerSettings
	{
		/// <summary>
		/// This is the set of supported game names
		/// </summary>
		[XmlArray("Platforms")]
		public string[] Platforms = new string[0];
		/// <summary>
		/// This is the set of supported game names
		/// </summary>
		[XmlArray("Games")]
		public string[] Games = new string[0];
		/// <summary>
		/// This is the set of supported PC build configurations
		/// </summary>
		[XmlArray("PCConfigs")]
		public string[] PCConfigs = new string[0];
		/// <summary>
		/// This is the set of supported Console build configurations
		/// </summary>
		[XmlArray("ConsoleConfigs")]
		public string[] ConsoleConfigs = new string[0];
		/// <summary>
		/// Directory inside the Local Application directory to store user settings
		/// </summary>
		[XmlAttribute]
		public string UserSettingsDirectory = "";

		/// <summary>
		/// Needed for XML serialization. Does nothing
		/// </summary>
		public CookerSettings()
		{
		}
	}
}
