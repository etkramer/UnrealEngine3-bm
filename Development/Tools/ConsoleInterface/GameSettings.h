#pragma once

using namespace System;
using namespace System::Xml::Serialization;

namespace ConsoleInterface
{
	//forward declarations
	ref class FileGroup;

	/// <summary>
	/// Summary description for per-game CookerSettings.
	/// </summary>
	[Serializable]
	public ref class GameSettings
	{
	public:
		/// <summary>
		/// Set of file groups for syncing
		/// </summary>
		[XmlArray]
		array<FileGroup^> ^FileGroups;

		/// <summary>
		/// This is the set of supported game names
		/// </summary>
		[XmlAttribute]
		String ^XGDFileRelativePath;

		[XmlAttribute]
		String ^PS3TitleID;

	public:
		GameSettings();
	};
}