//--------------------------------------------------------------------------------------
// File: PrtOptionsForm.cs
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

using System;
using System.Collections;
using System.Xml;
using System.Configuration;
using System.Drawing;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace PrtPerVertexSample
{
    class PrtOptions
    {
        #region Public Static Member Variables

        #region Constants
        public const int MaxPcaVectors = 24;
        
        /// <summary>
        /// Subsurface scattering parameters from:
        /// "A Practical Model for Subsurface Light Transport", 
        /// Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan.
        /// SIGGRAPH 2001
        /// </summary>
        public static PredefinedMaterial[] PredefinedMaterials = new PredefinedMaterial[]
        {
            //      name                         scattering (R/G/B/A)                          absorption (R/G/B/A)                                  reflectance (R/G/B/A)                         index of refraction
            new PredefinedMaterial("Default",    new ColorValue(2.00f, 2.00f, 2.00f, 1.0f),    new ColorValue(0.0030f, 0.0030f, 0.0460f, 1.0f),      new ColorValue(1.00f, 1.00f, 1.00f, 1.0f),    1.3f),
            new PredefinedMaterial("Apple",      new ColorValue(2.29f, 2.39f, 1.97f, 1.0f),    new ColorValue(0.0030f, 0.0030f, 0.0460f, 1.0f),      new ColorValue(0.85f, 0.84f, 0.53f, 1.0f),    1.3f),
            new PredefinedMaterial("Chicken1",   new ColorValue(0.15f, 0.21f, 0.38f, 1.0f),    new ColorValue(0.0150f, 0.0770f, 0.1900f, 1.0f),      new ColorValue(0.31f, 0.15f, 0.10f, 1.0f),    1.3f),
            new PredefinedMaterial("Chicken2",   new ColorValue(0.19f, 0.25f, 0.32f, 1.0f),    new ColorValue(0.0180f, 0.0880f, 0.2000f, 1.0f),      new ColorValue(0.32f, 0.16f, 0.10f, 1.0f),    1.3f),
            new PredefinedMaterial("Cream",      new ColorValue(7.38f, 5.47f, 3.15f, 1.0f),    new ColorValue(0.0002f, 0.0028f, 0.0163f, 1.0f),      new ColorValue(0.98f, 0.90f, 0.73f, 1.0f),    1.3f),
            new PredefinedMaterial("Ketchup",    new ColorValue(0.18f, 0.07f, 0.03f, 1.0f),    new ColorValue(0.0610f, 0.9700f, 1.4500f, 1.0f),      new ColorValue(0.16f, 0.01f, 0.00f, 1.0f),    1.3f),
            new PredefinedMaterial("Marble",     new ColorValue(2.19f, 2.62f, 3.00f, 1.0f),    new ColorValue(0.0021f, 0.0041f, 0.0071f, 1.0f),      new ColorValue(0.83f, 0.79f, 0.75f, 1.0f),    1.5f),
            new PredefinedMaterial("Potato",     new ColorValue(0.68f, 0.70f, 0.55f, 1.0f),    new ColorValue(0.0024f, 0.0090f, 0.1200f, 1.0f),      new ColorValue(0.77f, 0.62f, 0.21f, 1.0f),    1.3f),
            new PredefinedMaterial("Skimmilk",   new ColorValue(0.70f, 1.22f, 1.90f, 1.0f),    new ColorValue(0.0014f, 0.0025f, 0.0142f, 1.0f),      new ColorValue(0.81f, 0.81f, 0.69f, 1.0f),    1.3f),
            new PredefinedMaterial("Skin1",      new ColorValue(0.74f, 0.88f, 1.01f, 1.0f),    new ColorValue(0.0320f, 0.1700f, 0.4800f, 1.0f),      new ColorValue(0.44f, 0.22f, 0.13f, 1.0f),    1.3f),
            new PredefinedMaterial("Skin2",      new ColorValue(1.09f, 1.59f, 1.79f, 1.0f),    new ColorValue(0.0130f, 0.0700f, 0.1450f, 1.0f),      new ColorValue(0.63f, 0.44f, 0.34f, 1.0f),    1.3f),
            new PredefinedMaterial("Spectralon", new ColorValue(11.60f,20.40f,14.90f, 1.0f),   new ColorValue(0.0000f, 0.0000f, 0.0000f, 1.0f),      new ColorValue(1.00f, 1.00f, 1.00f, 1.0f),    1.3f),
            new PredefinedMaterial("Wholemilk",  new ColorValue(2.55f, 3.21f, 3.77f, 1.0f),    new ColorValue(0.0011f, 0.0024f, 0.0140f, 1.0f),      new ColorValue(0.91f, 0.88f, 0.76f, 1.0f),    1.3f),
            new PredefinedMaterial("Custom",     new ColorValue(0.00f, 0.00f, 0.00f, 1.0f),    new ColorValue(0.0000f, 0.0000f, 0.0000f, 1.0f),      new ColorValue(0.00f, 0.00f, 0.00f, 1.0f),    0.0f),
        };

        public static Hashtable PredefinedMaterialsHT = initializeHashtable();

        #endregion

        #endregion

        #region Private Static Member Variables
        private static OptionsFile optionsFile = new OptionsFile();
        #endregion

        #region Public Static Member Functions
        public static SimulatorOptions GlobalOptions { get { return optionsFile.Options; } }
        public static OptionsFile GlobalOptionsFile { get { return optionsFile; } }
        #endregion

        #region Private Static Member Functions
        private static Hashtable initializeHashtable()
        {
            Hashtable ht = new Hashtable();
            for(int i = 0; i < PredefinedMaterials.Length; i++)
            {
                ht.Add(PredefinedMaterials[i].Name, PredefinedMaterials[i]);
            }

            return ht;
        }
        #endregion
    }

    /// <summary>
    /// Struct to store material parameters
    /// </summary>
    public struct PredefinedMaterial
    {
        public string Name;
        public ColorValue ReducedScattering;
        public ColorValue Absorption;
        public ColorValue Diffuse;
        public float RelativeIndexOfRefraction;

        public PredefinedMaterial(string name, ColorValue reducedScattering, ColorValue absoption, ColorValue diffuse, float RelativeIndexOfRefraction)
        {
            this.Name = name;
            this.ReducedScattering = reducedScattering;
            this.Absorption = absoption;
            this.Diffuse = diffuse;
            this.RelativeIndexOfRefraction = RelativeIndexOfRefraction;
        }
    };

    class OptionsFile
    {
        #region Creation
        public OptionsFile()
        {
            ResetSettings();

            // Find out the executing assembly information
            System.Reflection.Assembly executingAssembly = System.Reflection.Assembly.GetExecutingAssembly();

            // And the executable's folder
            string exeFolder = System.IO.Path.GetDirectoryName(executingAssembly.Location);

            fileName = exeFolder + "\\options.xml";
            try
            {
                LoadOptions(fileName);
            }
            catch(System.IO.FileNotFoundException)
            {
                // Never mind. We will create the file again
            }
        }
        #endregion

        #region Instance Data
        private string fileName;
        public SimulatorOptions options;
        #endregion

        #region Properties
        public SimulatorOptions Options { get{ return options;} }
        #endregion

        public void LoadOptions()
        {
            LoadOptions(string.Empty);
        }

        public void LoadOptions(string fileName)
        {
            if( fileName == string.Empty)
                fileName = this.fileName;

            XmlDocument doc = new XmlDocument();
            XmlNode node;

            doc.Load(fileName);

            node = doc.FirstChild.FirstChild;

            options = new SimulatorOptions();
            int boolHelper = 0;
            XmlHelper.GetValue( ref node, "InitialDir", out options.InitialDir );
            XmlHelper.GetValue( ref node, "InputMesh", out options.InputMesh );
            XmlHelper.GetValue( ref node, "ResultsFile", out options.ResultsFileName );
            XmlHelper.GetValue( ref node, "Order", out options.Order );
            XmlHelper.GetValue( ref node, "NumberRays", out options.NumberRays );
            XmlHelper.GetValue( ref node, "NumberBounces", out options.NumberBounces );
            XmlHelper.GetValue( ref node, "ShowTooltips", out boolHelper );
            options.ShowTooltips = (boolHelper != 0);

            XmlHelper.GetValue( ref node, "NumberClusters", out options.NumberClusters );
            int quality;
            XmlHelper.GetValue( ref node, "Quality", out quality );
            options.Quality = (CompressionQuality) quality;
            XmlHelper.GetValue( ref node, "NumberPcaVectors", out options.NumberPcaVectors );

            XmlHelper.GetValue( ref node, "IsSubsurfaceScattering", out boolHelper );
            options.IsSubsurfaceScattering = (boolHelper != 0);
            XmlHelper.GetValue( ref node, "LengthScale", out options.LengthScale );
            XmlHelper.GetValue( ref node, "NumberChannels", out options.NumberChannels );
            XmlHelper.GetValue( ref node, "PredefinedMaterialIndex", out options.PredefinedMaterialIndex );

            XmlHelper.GetValue( ref node, "Diffuse", out options.Diffuse );
            XmlHelper.GetValue( ref node, "Absorption", out options.Absorption );
            XmlHelper.GetValue( ref node, "ReducedScattering", out options.ReducedScattering );

            XmlHelper.GetValue( ref node, "RelativeIndexOfRefraction", out options.RelativeIndexOfRefraction );

            XmlHelper.GetValue( ref node, "IsAdaptive", out boolHelper );
            options.IsAdaptive = (boolHelper != 0);
            XmlHelper.GetValue( ref node, "IsRobustMeshRefine", out boolHelper );
            options.IsRobustMeshRefine = (boolHelper != 0);
            XmlHelper.GetValue( ref node, "robustMeshRefineMinEdgeLength", out options.RobustMeshRefineMinEdgeLength );
            XmlHelper.GetValue( ref node, "robustMeshRefineMaxSubdiv", out options.RobustMeshRefineMaxSubdiv );
            XmlHelper.GetValue( ref node, "IsAdaptiveDL", out boolHelper );
            options.IsAdaptiveDL = (boolHelper != 0);
            XmlHelper.GetValue( ref node, "AdaptiveDLMinEdgeLength", out options.AdaptiveDLMinEdgeLength );
            XmlHelper.GetValue( ref node, "AdaptiveDLThreshold", out options.AdaptiveDLThreshold );
            XmlHelper.GetValue( ref node, "AdaptiveDLMaxSubdiv", out options.AdaptiveDLMaxSubdiv );
            XmlHelper.GetValue( ref node, "IsAdaptiveBounce", out boolHelper );
            options.IsAdaptiveBounce = (boolHelper != 0);
            XmlHelper.GetValue( ref node, "AdaptiveBounceMinEdgeLength", out options.AdaptiveBounceMinEdgeLength );
            XmlHelper.GetValue( ref node, "AdaptiveBounceThreshold", out options.AdaptiveBounceThreshold );
            XmlHelper.GetValue( ref node, "AdaptiveBounceMaxSubdiv", out options.AdaptiveBounceMaxSubdiv );
            XmlHelper.GetValue( ref node, "OutputMesh", out options.OutputMesh );
            XmlHelper.GetValue( ref node, "IsSaveCompressedResults", out boolHelper );
            options.IsSaveCompressedResults = (boolHelper != 0);
            XmlHelper.GetValue( ref node, "IsBinaryOutputXFile", out boolHelper );
            options.IsBinaryOutputXFile = (boolHelper != 0);
        }

        public void SaveOptions(string fileName)
        {
            if( fileName == String.Empty)
                fileName = this.fileName;

            XmlDocument doc = new XmlDocument();
            XmlNode docNode = (XmlNode) doc;
            XmlNode topNode;

            XmlHelper.CreateChildNode(ref doc, ref docNode, "PrtOptions", XmlNodeType.Element, out topNode);
            XmlHelper.CreateNewValue( ref doc, ref topNode, "InitialDir", options.InitialDir );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "InputMesh", options.InputMesh );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "ResultsFile", options.ResultsFileName );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "Order", options.Order );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "NumberRays", options.NumberRays );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "NumberBounces", options.NumberBounces );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "ShowTooltips", options.ShowTooltips? 1 : 0 );

            XmlHelper.CreateNewValue( ref doc, ref topNode, "NumberClusters", options.NumberClusters );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "Quality", (int) options.Quality );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "NumberPcaVectors", options.NumberPcaVectors );

            XmlHelper.CreateNewValue( ref doc, ref topNode, "IsSubsurfaceScattering", options.IsSubsurfaceScattering? 1 : 0 );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "LengthScale", options.LengthScale );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "NumberChannels", (int)options.NumberChannels );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "PredefinedMatIndex", options.PredefinedMaterialIndex );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "Diffuse", options.Diffuse );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "Absorption", options.Absorption );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "ReducedScattering", options.ReducedScattering );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "RelativeIndexOfRefraction", options.RelativeIndexOfRefraction );

            XmlHelper.CreateNewValue( ref doc, ref topNode, "IsAdaptive", options.IsAdaptive? 1 : 0 );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "IsRobustMeshRefine", options.IsRobustMeshRefine? 1 : 0 );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "RobustMeshRefineMinEdgeLength", options.RobustMeshRefineMinEdgeLength );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "RobustMeshRefineMaxSubdiv", options.RobustMeshRefineMaxSubdiv );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "IsAdaptiveDL", options.IsAdaptiveDL? 1 : 0 );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "AdaptiveDLMinEdgeLength", options.AdaptiveDLMinEdgeLength );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "AdaptiveDLThreshold", options.AdaptiveDLThreshold );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "AdaptiveDLMaxSubdiv", options.AdaptiveDLMaxSubdiv );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "IsAdaptiveBounce", options.IsAdaptiveBounce? 1 : 0 );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "AdaptiveBounceMinEdgeLength", options.AdaptiveBounceMinEdgeLength );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "AdaptiveBounceThreshold", options.AdaptiveBounceThreshold );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "AdaptiveBounceMaxSubdiv", options.AdaptiveBounceMaxSubdiv );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "OutputMesh", options.OutputMesh );

            XmlHelper.CreateNewValue( ref doc, ref topNode, "IsSaveCompressedResults", options.IsSaveCompressedResults? 1 : 0 );
            XmlHelper.CreateNewValue( ref doc, ref topNode, "IsBinaryOutputXFile", options.IsBinaryOutputXFile? 1 : 0 );

            // First delete the existing file
            try
            {
                System.IO.File.Delete(fileName);
            }
            catch
            {
                // Ignore any file deletion erros
            }
            doc.Save(fileName);
        }

        public void ResetSettings()
        {
            options = new SimulatorOptions();

            options.InputMesh = "misc\\shapes1.x";
            options.InitialDir = System.IO.Path.GetDirectoryName(Utility.FindMediaFile(options.InputMesh));
            options.ResultsFileName = "shapes1_prtresults.pca";
            options.Order = 6;
            options.NumberRays = 1024;
            options.NumberBounces = 1;
            options.IsSubsurfaceScattering = false;
            options.LengthScale = 25.0f;
            options.NumberChannels = 3;

            options.PredefinedMaterialIndex = 0;
            options.RelativeIndexOfRefraction = PrtOptions.PredefinedMaterials[0].RelativeIndexOfRefraction;
            options.Diffuse = PrtOptions.PredefinedMaterials[0].Diffuse;
            options.Absorption = PrtOptions.PredefinedMaterials[0].Absorption;
            options.ReducedScattering = PrtOptions.PredefinedMaterials[0].ReducedScattering;

            options.IsAdaptive = false;
            options.IsRobustMeshRefine = true;
            options.RobustMeshRefineMinEdgeLength = 0.0f;
            options.RobustMeshRefineMaxSubdiv = 2;
            options.IsAdaptiveDL = true;
            options.AdaptiveDLMinEdgeLength = 0.03f;
            options.AdaptiveDLThreshold = 8e-5f;
            options.AdaptiveDLMaxSubdiv = 3;
            options.IsAdaptiveBounce = false;
            options.AdaptiveBounceMinEdgeLength = 0.03f;
            options.AdaptiveBounceThreshold = 8e-5f;
            options.AdaptiveBounceMaxSubdiv = 3;
            options.OutputMesh = "shapes1_adaptive.x";
            options.IsBinaryOutputXFile = true;

            options.IsSaveCompressedResults = true;
            //options.Quality = CompressionQuality.FastLowQuality;
            options.Quality = CompressionQuality.SlowHighQuality;
            options.NumberPcaVectors = 24;
            options.NumberClusters = 1;
        }
    }

    class XmlHelper
    {
        public static void CreateChildNode( ref XmlDocument doc, ref XmlNode parentNode, string name, XmlNodeType type, out XmlNode newNode )
        {
            newNode = doc.CreateNode(type, name, null);
            parentNode.AppendChild(newNode);
        }

        public static void CreateNewValue( ref XmlDocument doc, ref XmlNode node, string name, string newValue )
        {
            XmlNode newNode;
            XmlNode newTextNode;
            CreateChildNode( ref doc, ref node, name, XmlNodeType.Element, out newNode );
            CreateChildNode( ref doc, ref newNode, name, XmlNodeType.Text, out newTextNode );
            newTextNode.Value = newValue;
        }

        public static void CreateNewValue( ref XmlDocument doc, ref XmlNode node, string name, int newValue )
        {
            string stringValue = string.Format("{0}", newValue );
            CreateNewValue( ref doc, ref node, name, stringValue);
        }

        public static void CreateNewValue( ref XmlDocument doc, ref XmlNode node, string name, float newValue )
        {
            string stringValue = string.Format("{0}", newValue );
            CreateNewValue( ref doc, ref node, name, stringValue);
        }

        public static void CreateNewValue( ref XmlDocument doc, ref XmlNode node, string name, ColorValue newValue )
        {
            string stringValue = string.Format("{0}", newValue.ToArgb() );
            CreateNewValue( ref doc, ref node, name, stringValue);
        }

        public static void GetValue( ref XmlNode node, string name, out string value )
        {
            value = String.Empty;

            if( node == null )
                return;

            if(node.Name == name)
            {
                if(node.FirstChild != null)
                {
                    if(node.FirstChild.NodeType == XmlNodeType.Text)
                        value = node.FirstChild.Value;
                }
            }

            if(node.NextSibling != null)
                node = node.NextSibling;
        }

        public static void GetValue( ref XmlNode node, string name, out int value )
        {
            string stringValue;
            GetValue( ref node, name, out stringValue); if(stringValue == string.Empty) stringValue = "0";
            value = int.Parse(stringValue);
        }

        public static void GetValue( ref XmlNode node, string name, out bool value )
        {
            string stringValue;
            GetValue( ref node, name, out stringValue);
            if(stringValue == string.Empty)
                stringValue = "False";
            else
                stringValue = "True";
            value = bool.Parse(stringValue);
        }

        public static void GetValue( ref XmlNode node, string name, out float value )
        {
            string stringValue;
            GetValue( ref node, name, out stringValue);  if(stringValue == string.Empty) stringValue = "0";
            value = float.Parse(stringValue);
        }

        public static void GetValue( ref XmlNode node, string name, out ColorValue value )
        {
            string stringValue;
            GetValue( ref node, name, out stringValue);  if(stringValue == string.Empty) stringValue = "0";
            value = ColorValue.FromArgb(int.Parse(stringValue));
        }
    }
}
