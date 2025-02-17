//--------------------------------------------------------------------------------------
// File: PrtSimulator.cs
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

using System;
using System.IO;
using System.Drawing;
using System.Threading;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace PrtPerVertexSample
{
    class SimulatorOptions
    {
        // General settings
        public string InitialDir;
        public string InputMesh;
        public string ResultsFileName;
        public int NumberRays;
        public int Order;
        public int NumberChannels;
        public int NumberBounces;
        public bool IsSubsurfaceScattering;
        public float LengthScale;
        public bool ShowTooltips;

        // Material options
        public int PredefinedMaterialIndex;
        public ColorValue Diffuse;
        public ColorValue Absorption;
        public ColorValue ReducedScattering;
        public float RelativeIndexOfRefraction;

        // IsAdaptive options
        public bool IsAdaptive;
        public bool IsRobustMeshRefine;
        public float RobustMeshRefineMinEdgeLength;
        public int RobustMeshRefineMaxSubdiv;
        public bool IsAdaptiveDL;
        public float AdaptiveDLMinEdgeLength;
        public float AdaptiveDLThreshold;
        public int AdaptiveDLMaxSubdiv;
        public bool IsAdaptiveBounce;
        public float AdaptiveBounceMinEdgeLength;
        public float AdaptiveBounceThreshold;
        public int AdaptiveBounceMaxSubdiv;
        public string OutputMesh;
        public bool IsBinaryOutputXFile;
    
        // Compression options
        public bool IsSaveCompressedResults;
        public CompressionQuality Quality;
        public int NumberClusters;
        public int NumberPcaVectors;

        public SimulatorOptions Clone()
        {
            SimulatorOptions copy = new SimulatorOptions();

            // General settings
            copy.InitialDir = InitialDir;
            copy.InputMesh = InputMesh;
            copy.ResultsFileName = ResultsFileName;
            copy.NumberRays = NumberRays;
            copy.Order = Order;
            copy.NumberChannels = NumberChannels;
            copy.NumberBounces = NumberBounces;
            copy.IsSubsurfaceScattering = IsSubsurfaceScattering;
            copy.LengthScale = LengthScale;
            copy.ShowTooltips = ShowTooltips;

            // Material options
            copy.PredefinedMaterialIndex = PredefinedMaterialIndex;
            copy.Diffuse = Diffuse;
            copy.Absorption = Absorption;
            copy.ReducedScattering = ReducedScattering;
            copy.RelativeIndexOfRefraction = RelativeIndexOfRefraction;

            // IsAdaptive options
            copy.IsAdaptive = IsAdaptive;
            copy.IsRobustMeshRefine = IsRobustMeshRefine;
            copy.RobustMeshRefineMinEdgeLength = RobustMeshRefineMinEdgeLength;
            copy.RobustMeshRefineMaxSubdiv = RobustMeshRefineMaxSubdiv;
            copy.IsAdaptiveDL = IsAdaptiveDL;
            copy.AdaptiveDLMinEdgeLength = AdaptiveDLMinEdgeLength;
            copy.AdaptiveDLThreshold = AdaptiveDLThreshold;
            copy.AdaptiveDLMaxSubdiv = AdaptiveDLMaxSubdiv;
            copy.IsAdaptiveBounce = IsAdaptiveBounce;
            copy.AdaptiveBounceMinEdgeLength = AdaptiveBounceMinEdgeLength;
            copy.AdaptiveBounceThreshold = AdaptiveBounceThreshold;
            copy.AdaptiveBounceMaxSubdiv = AdaptiveBounceMaxSubdiv;
            copy.OutputMesh = OutputMesh;
            copy.IsBinaryOutputXFile = IsBinaryOutputXFile;
        
            // Compression options
            copy.IsSaveCompressedResults = IsSaveCompressedResults;
            copy.Quality = Quality;
            copy.NumberClusters = NumberClusters;
            copy.NumberPcaVectors = NumberPcaVectors;

            return copy;
        }
    };

    class PrtSimulatorException : DirectXSampleException
    {
        public PrtSimulatorException() : base("Exception in the PrtSimulator class.") {}
        public PrtSimulatorException(string errorDescription) : base(errorDescription) {}
        public PrtSimulatorException(Exception inner) : base("Exception in the PrtSimulator class.", inner) {}
    }


    class PrtSimulator
    {
        #region Statics
        private static object SyncObject = new object();
        private PercentageCompleteCallback EngineCallback = null;
        #endregion

        #region Instance Data
        private float percentDone = 0.0f;
        private int currentPass = 1;
        private string currentPassName = String.Empty;
        private int numberPasses = 1;

        private bool isRunning = false;
        private bool isStopSimulator = false;

        private Device device;
        private SimulatorOptions options;

        private PrtEngine prtEngine = null;
        private PrtMesh prtMesh = null;

        public Thread thread = null;
        private string threadName = String.Empty;

        #endregion

        #region Simple Properties/Methods

        public bool IsRunning { get { return isRunning = (thread != null) && thread.IsAlive; } }
        public float PercentComplete { get { return percentDone * 100.0f; } }
        public int CurrentPass { get { return currentPass; } }
        public string CurrentPassName { get { return currentPassName; } }
        public int NumPasses { get { return numberPasses; } }

        #endregion

        #region Creation

        public PrtSimulator()
        {
        }

        #endregion

        public void Dispose()
        {
            if(prtEngine != null)
            {
                prtEngine.Dispose();
                prtEngine = null;
            }
            if(prtMesh != null)
            {
                prtMesh.Dispose();
                prtMesh = null;
            }
        }

        public void Run(Device deviceLocal, SimulatorOptions optionsLocal, PrtMesh prtMeshLocal)
        {
            if( IsRunning ) 
                throw new PrtSimulatorException("Simulator is running when Run is called again");

            device = deviceLocal;
            options = optionsLocal.Clone();
            prtMesh = prtMeshLocal;

            isRunning = true;
            isStopSimulator = false;
            percentDone = 0.0f;

            // Launch the Prt simulator on another thread cause it'll 
            // likely take a while and the UI would be unresponsive otherwise
            thread = new Thread(new ThreadStart(PrtSimulationThreadProc));
            thread.Start();
        }

        public void Stop()
        {
            if( IsRunning )
            {
                lock(SyncObject)
                {
                    isStopSimulator = true;
                }

                // Wait for it to close
                if(thread.Join(10000) == false)
                    throw new PrtSimulatorException("PrtSimulation Thread hasn't exited");

                isStopSimulator = false;
            }
        }

        public void PrtSimulationThreadProc()
        {
            // Reset precent complete
            percentDone = 0.0f;

            if( !prtMesh.IsMeshLoaded )
                throw new PrtSimulatorException("Mesh not loaded");

            PrtBuffer dataTotal = null;
            PrtBuffer bufferA = null;
            PrtBuffer bufferB = null;
            SphericalHarmonicMaterial[] shMaterials = null;

            numberPasses = options.NumberBounces;
            if( options.IsSubsurfaceScattering )
                numberPasses *= 2;
            if( options.IsAdaptive && options.IsRobustMeshRefine )
                numberPasses++;

            numberPasses += 2;

            currentPass = 1;
            percentDone = -1.0f;
            currentPassName = "Initializing PRT engine";

            Mesh mesh = prtMesh.Mesh;

            bool extractUVs = false;
            if( options.IsAdaptive && prtMesh.AlbedoTexture != null )
                extractUVs = true;

            prtEngine = new PrtEngine(mesh, null, extractUVs, null);

            EngineCallback = new PercentageCompleteCallback(PrtSimulatorCB);
            prtEngine.SetCallback(EngineCallback, 0.001f);
            prtEngine.SetSamplingInfo( options.NumberRays, false, true, false, 0.0f );

            if( options.IsAdaptive && prtMesh.AlbedoTexture != null )
            {
                prtEngine.SetPerTexelAlbedo( prtMesh.AlbedoTexture, options.NumberChannels, null);
            }

            int numberMeshes = mesh.GetAttributeTable().Length;

            // This sample treats all subsets as having the same 
            // material properties but they don't have too
            ExtendedMaterial[] material = prtMesh.Materials;
            shMaterials = new SphericalHarmonicMaterial[numberMeshes];
            for( int i = 0; i < numberMeshes; ++i )
            {
                shMaterials[i].Diffuse = Color.FromArgb(options.Diffuse.ToArgb());
                shMaterials[i].IsMirror = false;
                shMaterials[i].IsSubsurfaceScattering = options.IsSubsurfaceScattering;
                shMaterials[i].RelativeIndexOfRefraction  = options.RelativeIndexOfRefraction;
                shMaterials[i].Absorption = Color.FromArgb(options.Absorption.ToArgb());
                shMaterials[i].ReducedScattering = Color.FromArgb(options.ReducedScattering.ToArgb());

                shMaterials[i].Diffuse = prtMesh.Materials[i].Material3D.Diffuse;
            }

            bool setAlbedoFromMaterial = true;
            if( options.IsAdaptive && prtMesh.AlbedoTexture != null )
                setAlbedoFromMaterial = false;

            prtEngine.SetMeshMaterials(shMaterials, options.NumberChannels, setAlbedoFromMaterial, options.LengthScale);

            if( !options.IsSubsurfaceScattering )
            {
                // Not doing subsurface scattering

                if( options.IsAdaptive && options.IsRobustMeshRefine ) 
                {
                    currentPass++;
                    percentDone = -1.0f;
                    currentPassName = "Robust Mesh Refine";
                    prtEngine.RobustMeshRefine( options.RobustMeshRefineMinEdgeLength, options.RobustMeshRefineMaxSubdiv );
                }

                int numberSamples = prtEngine.NumberVertices;
                dataTotal = new PrtBuffer(numberSamples, options.Order * options.Order, options.NumberChannels);

                currentPass++;
                currentPassName = "Computing Direct Lighting";
                percentDone = 0.0f;
                if( options.IsAdaptive && options.IsAdaptiveDL )
                {
                    try 
                    {
                        prtEngine.ComputeDirectLightingSphericalHarmonicsAdaptive(options.Order, options.AdaptiveDLThreshold, options.AdaptiveDLMinEdgeLength, options.AdaptiveDLMaxSubdiv, dataTotal);
                    }
                    catch
                    {
                        goto LEarlyExit;
                    }
                }
                else
                {
                    try 
                    {
                        prtEngine.ComputeDirectLightingSphericalHarmonics(options.Order, dataTotal);
                    }
                    catch
                    {
                        goto LEarlyExit;
                    }
                }

                if( options.NumberBounces > 1 )
                {
                    numberSamples = prtEngine.NumberVertices;
                    bufferA = new PrtBuffer(numberSamples, options.Order * options.Order, options.NumberChannels);
                    bufferB = new PrtBuffer(numberSamples, options.Order * options.Order, options.NumberChannels);
                    bufferA.AddBuffer(dataTotal);
                }

                for( uint bounce = 1; bounce < options.NumberBounces; ++bounce )
                {
                    currentPass++;
                    currentPassName = string.Format("Computing Bounce {0} Lighting", bounce + 1 );
                    percentDone = 0.0f;
                    try
                    {
                        if( options.IsAdaptive && options.IsAdaptiveBounce )
                            prtEngine.ComputeBounceAdaptive(bufferA, options.AdaptiveBounceThreshold, options.AdaptiveBounceMinEdgeLength, options.AdaptiveBounceMaxSubdiv, bufferB, dataTotal);
                        else
                            prtEngine.ComputeBounce( bufferA, bufferB, dataTotal );
                    }
                    catch
                    {
                        goto LEarlyExit; // handle user aborting simulator via callback 
                    }

                    // Swap pBufferA and pBufferB
                    PrtBuffer bufferTemp;
                    bufferTemp = bufferA;
                    bufferA = bufferB;
                    bufferB = bufferTemp;
                }

                if( options.IsAdaptive )
                {
                    mesh = prtEngine.GetAdaptedMesh( device);
                    prtMesh.SetMesh(device, mesh);

                    XFileFormat xFileFormat;
                    if( options.IsBinaryOutputXFile )
                        xFileFormat = XFileFormat.Binary;
                    else 
                        xFileFormat = XFileFormat.Text;

                    // Save the mesh
                    prtMesh.Mesh.Save(options.OutputMesh, (int []) null, prtMesh.Materials, xFileFormat);
                }

                if(bufferA != null)
                    bufferA.Dispose();

                if(bufferB != null)
                    bufferB.Dispose();
            }
            else
            {
                // Doing subsurface scattering

                if( options.IsAdaptive && options.IsRobustMeshRefine ) 
                    prtEngine.RobustMeshRefine( options.RobustMeshRefineMinEdgeLength, options.RobustMeshRefineMaxSubdiv );

                int numberSamples = prtEngine.NumberVertices;
                bufferA = new PrtBuffer(numberSamples, options.Order * options.Order, options.NumberChannels);
                bufferB = new PrtBuffer(numberSamples, options.Order * options.Order, options.NumberChannels);
                dataTotal = new PrtBuffer(numberSamples, options.Order * options.Order, options.NumberChannels);

                currentPass = 1;
                currentPassName = "Computing Direct Lighting";
                percentDone = 0.0f;
                if( options.IsAdaptive && options.IsAdaptiveDL )
                {
                    try
                    {
                        prtEngine.ComputeDirectLightingSphericalHarmonicsAdaptive(options.Order, options.AdaptiveDLThreshold, options.AdaptiveDLMinEdgeLength, options.AdaptiveDLMaxSubdiv, bufferA);
                    }
                    catch
                    {
                        goto LEarlyExit; // handle user aborting simulator via callback 
                    }
                }
                else
                {
                    try
                    {
                        prtEngine.ComputeDirectLightingSphericalHarmonics( options.Order, bufferA );
                    }
                    catch
                    {
                        goto LEarlyExit; // handle user aborting simulator via callback 
                    }
                }

                currentPass++;
                currentPassName = "Computing Subsurface Direct Lighting";
                try
                {
                    prtEngine.ComputeSubsurfaceScattering( bufferA, bufferB, dataTotal );
                }
                catch
                {
                    goto LEarlyExit; // handle user aborting simulator via callback 
                }

                for( uint bounce = 1; bounce < options.NumberBounces; ++bounce )
                {
                    currentPass++;
                    currentPassName = string.Format("Computing Bounce {0} Lighting", bounce + 1);
                    percentDone = 0.0f;
                    try
                    {
                        if( options.IsAdaptive && options.IsAdaptiveBounce )
                            prtEngine.ComputeBounceAdaptive( bufferB, options.AdaptiveBounceThreshold, options.AdaptiveBounceMinEdgeLength, options.AdaptiveBounceMaxSubdiv, bufferA, null);
                        prtEngine.ComputeBounce( bufferA, bufferB, dataTotal );
                    }
                    catch
                    {
                        goto LEarlyExit; // handle user aborting simulator via callback 
                    }

                    currentPass++;
                    currentPassName = string.Format("Computing Subsurface Bounce {0} Lighting", bounce + 1 );
                    try
                    {
                        prtEngine.ComputeSubsurfaceScattering( bufferB, bufferA, dataTotal );
                    }
                    catch
                    {
                        goto LEarlyExit; // handle user aborting simulator via callback 
                    }
                }

                if( options.IsAdaptive )
                {
                    mesh = prtEngine.GetAdaptedMesh( device);
                    prtMesh.SetMesh(device, mesh);

                    XFileFormat xFileFormat;
                    if( options.IsBinaryOutputXFile )
                        xFileFormat = XFileFormat.Binary;
                    else 
                        xFileFormat = XFileFormat.Text;

                    // Save the mesh
                    prtMesh.Mesh.Save(options.OutputMesh, (int []) null, prtMesh.Materials, xFileFormat);
                }

                if(bufferA != null)
                    bufferA.Dispose();

                if(bufferB != null)
                    bufferB.Dispose();
            }

            currentPass++;
            currentPassName = "Compressing Buffer";
            percentDone = -1.0f;

            Directory.SetCurrentDirectory( options.InitialDir );
            prtMesh.SetPrtBuffer( dataTotal, options.ResultsFileName );
            prtMesh.CompressBuffer(CompressionQuality.FastLowQuality, 1, 24);

            if( options.IsSaveCompressedResults )
            {
                PrtCompressedBuffer compBuffer = prtMesh.CompressedBuffer;
                compBuffer.Save(options.ResultsFileName);
            }
            else
            {
                dataTotal.Save(options.ResultsFileName);
            }
            prtMesh.ExtractCompressedDataForPrtShader();

            isRunning = false;
            percentDone = 1.0f;

            if(prtEngine != null)
            {
                prtEngine.Dispose();
                prtEngine = null;
            }

            return;

            LEarlyExit:

            // Usually fails becaused user stoped the simulator
            isRunning = false;

            if(prtEngine != null)
                prtEngine.Dispose();

            if(bufferA != null)
                bufferA.Dispose();

            if(bufferB != null)
                bufferB.Dispose();

            if(dataTotal != null)
                dataTotal.Dispose();
        }


        /// <summary>
        /// records the percent done and stops the simulator if requested
        /// </summary>
        private bool PrtSimulatorCB( float simulatorPercentage )
        {
            lock(SyncObject)
            {
                // Update percentage
                percentDone = simulatorPercentage;

                // In this callback, returning false will stop the simulator
                return (!isStopSimulator);
            }
        }
    }
}
