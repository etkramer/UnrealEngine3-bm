using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace UnrealFrontend
{
	/// <summary>
	/// A single chapter entry within the gow2 chapters config file.
	/// </summary>
	[Serializable]
	public class GoW2ChapterEntry
	{
		/// <summary>
		/// The name of the chapter.
		/// </summary>
		[XmlAttribute("name")]
		public string Name = "";
		
		/// <summary>
		/// The index of the chapter.
		/// </summary>
		[XmlAttribute("index")]
		public int Index;

		public override string ToString()
		{
			return this.Name;
		}
	}

	/// <summary>
	/// A gears of war 2 config file containing chapter information.
	/// </summary>
	[XmlRoot("Chapters")]
	[Serializable]
	public class GoW2ChaptersConfig
	{
		GoW2ChapterEntry[] mChapters = new GoW2ChapterEntry[0];

		/// <summary>
		/// Gets/Sets an array of chapters associated with the config file.
		/// </summary>
		[XmlElement("ChapterEntry")]
		//[XmlArrayItem("ChapterEntry")]
		public GoW2ChapterEntry[] Chapters
		{
			get { return mChapters; }
			set
			{
				if(value != null)
				{
					mChapters = value;
				}
			}
		}
	}
}
