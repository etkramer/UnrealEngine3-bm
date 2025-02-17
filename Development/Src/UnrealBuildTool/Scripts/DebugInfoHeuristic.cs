/**
 * This is a a semi "control panel" which has simple and explicit settings for turning on/off the creation of debug info
 * based on platform and configuration.
 * 
 * We separate this out as more than likely each developer will want to have different setting and will not have to constantly be resolving.
 * 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.IO;

namespace UnrealBuildTool
{
    class DebugInfoHeuristic
    {
        /** This function allows one to have an arbitrary configuration of per platform per config for whether or not to create debug info */
        public static bool ShouldCreateDebugInfo( UnrealTargetPlatform Platform, UnrealTargetConfiguration Configuration )
        {
            switch( Platform )
            {
                case UnrealTargetPlatform.Win32:
                    switch( Configuration )
                    {
                        case UnrealTargetConfiguration.Debug: return true;
                        case UnrealTargetConfiguration.Release: return true;
                        case UnrealTargetConfiguration.Shipping: return true;
                        case UnrealTargetConfiguration.ShippingDebugConsole: return true;
                        default: return true;
                    };

                case UnrealTargetPlatform.Xbox360:
                    switch( Configuration )
                    {
                        case UnrealTargetConfiguration.Debug: return true;
                        case UnrealTargetConfiguration.Release: return true;
                        case UnrealTargetConfiguration.Shipping: return true;
                        case UnrealTargetConfiguration.ShippingDebugConsole: return true;
                        default: return true;
                    };

                default: return true;
            };

        }

    }
}
