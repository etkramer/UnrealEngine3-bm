//--------------------------------------------------------------------------------------
// File: Script1.cs
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
        x = (float)Math.Sin(appTime);
        z = (float)Math.Cos(appTime);
        y = (float)Math.Abs(Math.Sin(appTime)); 
    }

    /// <summary>
    /// Update dwarf's rotation
    /// </summary>
    public static void UpdateRotationY(double appTime, ref float a)
    {
        a = (float)((appTime * 5) / (Math.PI * 2));
    }
}
