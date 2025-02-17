/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.Xml;

namespace UnrealBuildTool
{
	/** A Visual C++ project. */
	class VCProject
	{
		/** The files contained by the project. */
		List<string> RelativeFilePaths = new List<string>();

		/** Default constructor. */
		VCProject(Stream InputStream)
		{
			// Parse the project's root node.
			XmlDocument ProjectDocument = new XmlDocument();
			ProjectDocument.Load(InputStream);
			XmlNode ProjectNode = ProjectDocument.SelectSingleNode("/VisualStudioProject");
			// Check for both . and , to account for localization. I'm choosing
			// to do it this less-elegant way because there doesn't seem to be
			// an easy way to automatically localize this value and adding a
			// single OR condition is simple.
			if ((ProjectNode.Attributes["Version"].Value == "8.00") ||
				(ProjectNode.Attributes["Version"].Value == "8,00"))
			{
				// Parse the files and filters.
				XmlNode FilesNode = ProjectNode.SelectSingleNode("Files");
				if (FilesNode != null)
				{
					ParseFileSet(FilesNode);
				}
			}
		}

		/** Parses a list of files and file sets that are contained by a XML node. */
		void ParseFileSet(XmlNode ParentNode)
		{
			// Parse the list of files directly in this node.
			foreach (XmlNode FileNode in ParentNode.SelectNodes("File"))
			{
				RelativeFilePaths.Add(FileNode.Attributes["RelativePath"].Value);
			}

			// Recursively parse filtered sub-lists of files within this file set.
			foreach (XmlNode FilterNode in ParentNode.SelectNodes("Filter"))
			{
				ParseFileSet(FilterNode);
			}
		}

		/** Reads the list of files in a project from the specified project file. */
		public static List<string> GetProjectFiles(string ProjectPath)
		{
			using (FileStream ProjectStream = new FileStream(ProjectPath, FileMode.Open, FileAccess.Read))
			{
				VCProject Project = new VCProject(ProjectStream);
				return Project.RelativeFilePaths;
			}
		}
	}
}
