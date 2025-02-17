//----------------------------------------------------------------------------
// File: dxdiag.cs
//
// Desc: Sample app to read info from the DirectX Diagnostic Tool (DxDiag) by enumeration
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
using System;
using Microsoft.DirectX.Diagnostics;

namespace DxDiagOutput
{
    class DxDiagDisplay
    {
        static void Main(string[] args)
        {
            try
            {
                // Just start our recursive loop with our root container.  Don't worry
                // about checking Whql
                OutputDiagData(null, new Container(false));
            }
            catch
            {
                // Something bad happened
            }
        }




        /// <summary>
        /// Recursivly print the properties the root node and all its child to the console window
        /// </summary>
        /// <param name="parent">A string to display to show the root node of this data</param>
        /// <param name="root">The actual container for this data.</param>
        static void OutputDiagData(string parent, Container root)
        {
            try
            {
                foreach (PropertyData pd in root.Properties)
                {
                    // Just display the data
                    Console.WriteLine("{0}.{1} = {2}", parent, pd.Name, pd.Data);
                }
            }
            catch
            {
            }

            try
            {
                foreach (ContainerData cd in root.Containers)
                {
                    // Recurse all the internal nodes
                    if (parent == null)
                        OutputDiagData(cd.Name, cd.Container);
                    else
                        OutputDiagData(parent + "." + cd.Name, cd.Container);
                }
            }
            catch 
            {
            }
            
            // We are done with this container, we can dispose it.
            root.Dispose();
        }
    }
}
