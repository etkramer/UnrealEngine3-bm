//-----------------------------------------------------------------------------
// File: PrtPerVertex.cs
//
// This sample demonstrates how use D3DXSHPrtSimulation(), a per vertex  
// precomputed radiance transfer (Prt) simulator that uses low-order 
// spherical harmonics (SH) and records the results to a file. The sample 
// then demonstrates how compress the results with clustered principal 
// component analysis (CPca) and view the compressed results with arbitrary 
// lighting in real time with a vs_1_1 vertex shader
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

using System;
using System.Text;
using System.Text.RegularExpressions;
using System.Drawing;
using System.IO;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace PrtPerVertexSample
{
    /// <summary>PrtPerVertex Sample Class</summary>
    class PrtPerVertex : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public PrtPerVertex(Framework sampleFramework) 
        { 
            for( int i = 0; i < NumberSkyBoxes; i++)
                skybox[i] = new Skybox();

            for( int i = 0; i < NumberSkyBoxes; i++)
                for( int j = 0; j < 3; j++)
                    skyboxLightSH[i, j] = new float[SphericalHarmonics.MaximumOrder * SphericalHarmonics.MaximumOrder];

            // Store framework
            SampleFramework = sampleFramework;
            // Create dialogs
            hud = new Dialog(SampleFramework); 
            startUpUI   = new Dialog(SampleFramework); 
            startUpUI2  = new Dialog(SampleFramework);
            simulatorRunningUI  = new Dialog(SampleFramework);
            renderingUI = new Dialog(SampleFramework);
            renderingUI2    = new Dialog(SampleFramework);
            renderingUI3    = new Dialog(SampleFramework);
            compressionUI   = new Dialog(SampleFramework);
        }
        #endregion

        #region Instance Data
        // Variables
        public static Framework SampleFramework = null; // Framework for samples
        private Microsoft.DirectX.Direct3D.Font statsFont = null; // Font for drawing text
        private Sprite textSprite = null; // Sprite for batching text calls
        private ModelViewerCamera camera = new ModelViewerCamera(); // A model viewing camera
        private bool isHelpShowing = true; // If true, renders the UI help text

        private Dialog hud = null; // dialog for standard controls
        private Dialog startUpUI = null; // dialog for startup
        private Dialog startUpUI2 = null; // dialog for startup
        private Dialog simulatorRunningUI = null; // dialog for while simulator running
        private Dialog renderingUI = null; // dialog for while Prt rendering 
        private Dialog renderingUI2 = null; // dialog for while Prt rendering 
        private Dialog renderingUI3 = null; // dialog for while Prt rendering 
        private Dialog compressionUI = null; // dialog for Prt compression settings

        private PrtMesh prtMesh = new PrtMesh();

        private bool IsRenderEnvMap = true;
        private bool IsRenderUI = true;
        private bool IsRenderArrows = true;
        private bool IsRenderMesh = true;
        private bool IsRenderText = true;
        private bool IsRenderWithAlbedoTexture = true;
        private bool IsRenderCompressionUI = false;
        private bool IsWireframe = false;
        private bool IsRenderSHProjection = false;

        private float currentObjectRadius = -1.0f;
        private int technique = 0;
        private float lightScaleForPrt     = 0.0f;
        private float lightScaleForSHIrrad = 0.0f;
        private float lightScaleForNDotL   = 1.0f;

        private const int NumberSkyBoxes = 5;
        private Skybox[] skybox = new Skybox[NumberSkyBoxes];
        private float[,][] skyboxLightSH = new float[NumberSkyBoxes, 3][];
        private int skyboxA = 0;
        private int skyboxB = 1;

        public const int MaximumLights = 10;
        private DirectionWidget[] lightControl = new DirectionWidget[MaximumLights];
        private int numberActiveLights;
        private int activeLight;

        private SHCubeProj projData = new SHCubeProj();

        private enum ApplicationState
        {
            Startup,
            SimulatorOptions,
            SimulatorRunning,
            LoadPrtBuffer,
            RenderScene
        };

        private ApplicationState applicationState = ApplicationState.Startup; // State of the application
        #endregion

        #region Static Data
        public static PrtSimulator Simulator = new PrtSimulator();
        #endregion

        #region Global Constants
        public static readonly ColorValue White = ColorValue.FromColor(Color.White);
        public static readonly ColorValue Yellow = ColorValue.FromColor(Color.Yellow);
        public static readonly Color ClearColor1 = Color.FromArgb(0, 45, 50, 170);
        #endregion

        #region UI Control Constants
        // HUD Ui Control constants
        private const int Static = 0;
        private const int ToggleFullscreen = 1;
        private const int ToggleReference = 3;
        private const int ChangeDevice = 4;

        private const int NumberLights = 6;
        private const int NumberLightsStatic = 7;
        private const int ActiveLight = 8;
        private const int LightScale = 9;
        private const int LightScaleStatic = 10;

        private const int LoadPrtBuffer = 12;
        private const int SimulatorInt32 = 13;
        private const int StopSimulator = 14;

        private const int EnvironmentOne = 15;
        private const int EnvironmentTwo = 16;
        private const int Directional = 17;
        private const int EnvironmentBlend = 18;
        private const int EnvironmentA = 19;
        private const int EnvironmentB = 20;

        private const int RenderUI = 21;
        private const int RenderMap = 22;
        private const int RenderArrows = 23;
        private const int RenderMesh = 24;
        private const int SimulatorStatus = 25;
        private const int SimulatorStatus2 = 26;

        private const int NumberPcaVectors = 27;
        private const int NumberClusters = 29;
        private const int MaximumConstants = 31;
        private const int CurrentConstants = 32;
        private const int CurrentConstantsStatic = 34;

        private const int Apply = 33;
        private const int LightAngleStatic = 35;
        private const int LightAngle = 36;
        private const int RenderTexture = 37;
        private const int WireFrame = 38;
        private const int Restart = 40;
        private const int Compression = 41;
        private const int SHProjection = 42;

        private const int TechniquePrt = 43;
        private const int TechniqueSHIrrad = 44;
        private const int TechniqueNDotL = 45;

        private const int Scene1 = 46;
        private const int Scene2 = 47;
        private const int Scene3 = 48;
        private const int Scene4 = 49;
        private const int Scene5 = 50;

        #endregion

        private struct SHCubeProj 
        {
            public float[] Red, Green, Blue;
            public int OrderUse; // order to use
            public float[] ConvolutionCoefficients; // convolution coefficients

            public void InitDiffCubeMap(float[] R, float[] G, float[] B)
            {
                Red = R;
                Green = G;
                Blue = B;

                OrderUse = 3; // go to 5 is a bit more accurate...

                ConvolutionCoefficients = new float[6];
                ConvolutionCoefficients[0] = 1.0f;
                ConvolutionCoefficients[1] = 2.0f/3.0f;
                ConvolutionCoefficients[2] = 1.0f/4.0f;
                ConvolutionCoefficients[3] = ConvolutionCoefficients[5] = 0.0f;
                ConvolutionCoefficients[4] = -6.0f/144.0f;
            }

            public void Init(float[] R, float[] G, float[] B)
            {
                Red = R;
                Green = G;
                Blue = B;

                OrderUse = 6;

                ConvolutionCoefficients = new float[6];
                for(int i = 0; i < 6; i++)
                    ConvolutionCoefficients[i] = 1.0f;
            }
        };

        private Vector4 SHCubeFill(Vector3 texCoord, Vector3 texelSize)
        {
            texCoord.Normalize();
            float[] vals = new float[36];
            SphericalHarmonics.EvaluateDirection(vals, projData.OrderUse, texCoord);

            Vector4 outVector = Vector4.Empty; // just clear it out...

            int l, m, index = 0;
            for(l = 0; l < projData.OrderUse; l++) 
            {
                float ConvUse = projData.ConvolutionCoefficients[l];
                for(m = 0; m < 2 * l + 1; m++) 
                {
                    outVector.X += ConvUse * vals[index] * projData.Red[index];
                    outVector.Y += ConvUse * vals[index] * projData.Green[index];
                    outVector.Z += ConvUse * vals[index] * projData.Blue[index];
                    outVector.W = 1;

                    index++;
                }
            }

            return outVector;
        }


        /// <summary>
        /// Called during device initialization, this code checks the device for some 
        /// minimum set of capabilities, and rejects those that don't pass by returning false.
        /// </summary>
        public bool IsDeviceAcceptable(Caps caps, Format adapterFormat, Format backBufferFormat, bool windowed)
        {
            // Skip back buffer formats that don't support alpha blending
            if (!Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, adapterFormat, 
                Usage.QueryPostPixelShaderBlending, ResourceType.Textures, backBufferFormat))
                return false;

            return true;
        }

        /// <summary>
        /// This callback function is called immediately before a device is created to allow the 
        /// application to modify the device settings. The supplied settings parameter 
        /// contains the settings that the framework has selected for the new device, and the 
        /// application can make any desired changes directly to this structure.  Note however that 
        /// the sample framework will not correct invalid device settings so care must be taken 
        /// to return valid device settings, otherwise creating the Device will fail.  
        /// </summary>
        public void ModifyDeviceSettings(DeviceSettings settings, Caps caps)
        {
            // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
            // then switch to SWVP.
            if ( (!caps.DeviceCaps.SupportsHardwareTransformAndLight) ||
                (caps.VertexShaderVersion < new Version(1,1)) )
            {
                settings.BehaviorFlags = CreateFlags.SoftwareVertexProcessing;
            }
            else
            {
                settings.BehaviorFlags = CreateFlags.HardwareVertexProcessing;
            }

            // The PRT simulator runs on a seperate thread.  If you are just 
            // loading simulator results, then this isn't needed
            settings.BehaviorFlags |= CreateFlags.MultiThreaded;

            // This application is designed to work on a pure device by not using 
            // any get methods, so create a pure device if supported and using HWVP.
            if ( (caps.DeviceCaps.SupportsPureDevice) && 
                ((settings.BehaviorFlags & CreateFlags.HardwareVertexProcessing) != 0 ) )
                settings.BehaviorFlags |= CreateFlags.PureDevice;

            // Debugging vertex shaders requires either REF or software vertex processing 
            // and debugging pixel shaders requires REF.  
#if(DEBUG_VS)
            if (settings.DeviceType != DeviceType.Reference )
            {
                settings.BehaviorFlags &= ~CreateFlags.HardwareVertexProcessing;
                settings.BehaviorFlags &= ~CreateFlags.PureDevice;
                settings.BehaviorFlags |= CreateFlags.SoftwareVertexProcessing;
            }
#endif
#if(DEBUG_PS)
            settings.DeviceType = DeviceType.Reference;
#endif
            // For the first device created if its a REF device, optionally display a warning dialog box
            if (settings.DeviceType == DeviceType.Reference)
            {
                Utility.DisplaySwitchingToRefWarning(SampleFramework, "PRTPerVertex");
            }
        }

        /// <summary>
        /// This event will be fired immediately after the Direct3D device has been 
        /// created, which will happen during application initialization and windowed/full screen 
        /// toggles. This is the best location to create Pool.Managed resources since these 
        /// resources need to be reloaded whenever the device is destroyed. Resources created  
        /// here should be released in the Disposing event. 
        /// </summary>
        private void OnCreateDevice(object sender, DeviceEventArgs e)
        {
            string cubeMapFileName;

            Device device = e.Device;

            for(int i = 0; i < NumberSkyBoxes; ++i )
            {
                switch( i )
                {
                    case 0: cubeMapFileName = "Light Probes\\rnl_cross.dds"; break;
                    case 1: cubeMapFileName = "Light Probes\\uffizi_cross.dds"; break;
                    case 2: cubeMapFileName = "Light Probes\\galileo_cross.dds"; break;
                    case 3: cubeMapFileName = "Light Probes\\grace_cross.dds"; break;
                    case 4: 
                    default: cubeMapFileName = "Light Probes\\stpeters_cross.dds"; break;
                }

                skybox[i].OnCreateDevice( device, 50, cubeMapFileName, "SkyBox.fx" );

                SphericalHarmonics.ProjectCubeMap(6, skybox[i].EnvironmentMap, skyboxLightSH[i, 0], skyboxLightSH[i, 1], skyboxLightSH[i, 2] );

                // now compute the SH projection of the skybox...
                CubeTextureRequirements ctr = new CubeTextureRequirements();
                ctr.Format = Format.A16B16G16R16F;
                TextureLoader.CheckCubeTextureRequirements(device, Usage.None, Pool.Managed, out ctr);
                CubeTexture shCubeTexture = new CubeTexture(device, 256, 1, Usage.None, ctr.Format, Pool.Managed);

                projData.Init(skyboxLightSH[i,0], skyboxLightSH[i,1], skyboxLightSH[i,2]);
                TextureLoader.FillTexture(shCubeTexture, new Fill3DTextureCallback(SHCubeFill));
                skybox[i].InitSH(shCubeTexture);
            }

            prtMesh.OnCreateDevice(device);

            // Initialize the font
            statsFont = ResourceCache.GetGlobalInstance().CreateFont(e.Device, 15, 0, FontWeight.Bold, 0, false, CharacterSet.Default, Precision.Default, FontQuality.Default, PitchAndFamily.FamilyDoNotCare | PitchAndFamily.DefaultPitch, "Arial");
            DirectionWidget.OnCreateDevice(device);

            // Setup the camera's view parameters
            camera.SetViewParameters(new Vector3(0.0f, 0.0f, -5.0f), Vector3.Empty);

            int max = (SampleFramework.DeviceCaps.MaxVertexShaderConst - 4) / 2;
            compressionUI.GetSlider( NumberClusters ).SetRange( 1, max );

            string str = string.Format("Max VS Constants: {0}", SampleFramework.DeviceCaps.MaxVertexShaderConst);
            compressionUI.GetStaticText( MaximumConstants ).SetText( str );
            UpdateConstText();
        }
        
        /// <summary>
        /// This event will be fired immediately after the Direct3D device has been 
        /// reset, which will happen after a lost device scenario. This is the best location to 
        /// create Pool.Default resources since these resources need to be reloaded whenever 
        /// the device is lost. Resources created here should be released in the OnLostDevice 
        /// event. 
        /// </summary>
        private void OnResetDevice(object sender, DeviceEventArgs e)
        {
            SurfaceDescription backBufferSurfaceDesc = e.BackBufferDescription;

            for(int i = 0; i < NumberSkyBoxes; ++i )
                skybox[i].OnResetDevice( backBufferSurfaceDesc ); 

            prtMesh.OnResetDevice();

            if(statsFont != null)
                statsFont.OnResetDevice();

            Device device = e.Device;

            // Create a sprite to help batch calls when drawing many lines of text
            if(textSprite == null)
                textSprite = new Sprite(device);
            else
                textSprite.OnResetDevice();

            for( int i = 0; i < MaximumLights; i++ )
                lightControl[i].OnResetDevice( backBufferSurfaceDesc );

            // Setup the camera's projection parameters
            float aspectRatio = (float)backBufferSurfaceDesc.Width / (float)backBufferSurfaceDesc.Height;
            camera.SetProjectionParameters((float)Math.PI / 4, aspectRatio, 1.0f, 10000.0f);
            camera.SetWindow(backBufferSurfaceDesc.Width, backBufferSurfaceDesc.Height);
            camera.SetButtonMasks((int) MouseButtonMask.Left, (int) MouseButtonMask.Wheel, (int) MouseButtonMask.Right);
            camera.IsAttachedToModel = true;

            if( prtMesh.Mesh != null )
            {
                SetRadii();
            }

            // Setup UI locations
            hud.SetLocation(backBufferSurfaceDesc.Width - 170, 0);
            hud.SetSize(170, 100);

            startUpUI.SetLocation( (backBufferSurfaceDesc.Width - 300) / 2, (backBufferSurfaceDesc.Height - 200) / 2 );
            startUpUI.SetSize( 300, 200 );

            startUpUI2.SetLocation( 50, (backBufferSurfaceDesc.Height - 200) );
            startUpUI2.SetSize( 300, 200 );

            compressionUI.SetLocation( 0, 150 );
            compressionUI.SetSize( 200, 200 );

            simulatorRunningUI.SetLocation( (backBufferSurfaceDesc.Width - 600) / 2, (backBufferSurfaceDesc.Height - 100) / 2 );
            simulatorRunningUI.SetSize( 600, 100 );

            renderingUI.SetLocation( 0, backBufferSurfaceDesc.Height - 125 );
            renderingUI.SetSize( backBufferSurfaceDesc.Width, 125 );

            renderingUI2.SetLocation( backBufferSurfaceDesc.Width - 170, 100 );
            renderingUI2.SetSize( 170, 400 );

            renderingUI3.SetLocation( 10, 30 );
            renderingUI3.SetSize( 200, 100 );
        }

        private void SetRadii()
        {
            // Update camera's viewing radius based on the object radius
            float objectRadius = prtMesh.Radius;
            if( currentObjectRadius != objectRadius )
            {
                currentObjectRadius = objectRadius;
                camera.SetRadius( objectRadius * 3.0f, objectRadius * 0.1f, objectRadius * 20.0f );
            }
            camera.SetModelCenter( prtMesh.Center );
            for( int i = 0; i < MaximumLights; i++ )
                lightControl[i].Radius = objectRadius;
        }

        /// <summary>
        /// This event function will be called fired after the Direct3D device has 
        /// entered a lost state and before Device.Reset() is called. Resources created
        /// in the OnResetDevice callback should be released here, which generally includes all 
        /// Pool.Default resources. See the "Lost Devices" section of the documentation for 
        /// information about lost devices.
        /// </summary>
        private void OnLostDevice(object sender, EventArgs e)
        {
            if(skybox != null)
                for(int i = 0; i < NumberSkyBoxes; ++i )
                {
                    if(skybox[i] != null)
                        skybox[i].OnLostDevice();
                }

            DirectionWidget.OnLostDevice();

            if (textSprite != null)
            {
                textSprite.Dispose();
                textSprite = null;
            }

            if( prtMesh != null)
                prtMesh.OnLostDevice();
        }

        /// <summary>
        /// This callback function will be called immediately after the Direct3D device has 
        /// been destroyed, which generally happens as a result of application termination or 
        /// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
        /// should be released here, which generally includes all Pool.Managed resources. 
        /// </summary>
        private void OnDestroyDevice(object sender, EventArgs e)
        {
            if(skybox != null)
                for(int i = 0; i < NumberSkyBoxes; ++i )
                {
                    if(skybox[i] != null)
                        skybox[i].OnDestroyDevice();
                }

            if(prtMesh != null)
                prtMesh.OnDestroyDevice();

            DirectionWidget.OnDestroyDevice();

            if(Simulator != null)
                Simulator.Stop();
        }

        /// <summary>
        /// This callback function will be called once at the beginning of every frame. This is the
        /// best location for your application to handle updates to the scene, but is not 
        /// intended to contain actual rendering calls, which should instead be placed in the 
        /// OnFrameRender callback.  
        /// </summary>
        public void OnFrameMove(Device device, double appTime, float elapsedTime)
        {
            // Update the camera's position based on user input 
            camera.FrameMove(elapsedTime);
        }

        private void RenderStartup(Device device, double appTime, float elapsedTime)
        {
            bool beginSceneCalled = false;

            // Clear the render target and the zbuffer 
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, ClearColor1 , 1.0f, 0);

            // Render the scene
            try
            {
                device.BeginScene();
                beginSceneCalled = true;

                RenderText();
                if( IsRenderUI )
                {
                    hud.OnRender( elapsedTime );
                    startUpUI.OnRender( elapsedTime );
                    startUpUI2.OnRender( elapsedTime );
                }
            }
            finally
            {
                if (beginSceneCalled)
                    device.EndScene();
            }
        }

        /// <summary>
        /// Load a mesh and optionally generate the PRT results file if they aren't already cached
        /// </summary>
        void LoadSceneAndOptGenResults( Device device, string meshFileName, string resultsFileName,
            int numberRays, int numberBounces, bool isSubSurface )
        {
            string meshFileLongName = Utility.FindMediaFile(meshFileName);

            try
            {
                string resultsFileLongName = Utility.FindMediaFile(resultsFileName);
            }
            catch(MediaNotFoundException)
            {
                SimulatorOptions options = new SimulatorOptions();

                options.InputMesh = meshFileName;
                
                options.InitialDir = System.IO.Path.GetDirectoryName(Utility.FindMediaFile(options.InputMesh));
                options.ResultsFileName = System.IO.Path.GetFileName(resultsFileName);
                options.Order = 6;
                options.NumberRays = numberRays;
                options.NumberBounces = numberBounces;
                options.IsSubsurfaceScattering = isSubSurface;
                options.LengthScale = 25.0f;
                options.NumberChannels = 3;

                options.PredefinedMaterialIndex = 0;
                options.RelativeIndexOfRefraction = 1.3f;
                options.Diffuse = White;
                options.Absorption = new ColorValue( 0.0030f, 0.0030f, 0.0460f, 1.0f );
                options.ReducedScattering = new ColorValue( 2.00f, 2.00f, 2.00f, 1.0f );

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
                options.Quality = CompressionQuality.SlowHighQuality;
                options.NumberPcaVectors = 24;
                options.NumberClusters = 1;

                prtMesh.LoadMesh( device, meshFileName );

                SetRadii();

                Simulator.Run( device, options, prtMesh );
                applicationState = ApplicationState.SimulatorRunning;

                return;
            }
            LoadScene( device, meshFileName, resultsFileName);
        }

        private void LoadScene(Device device, string meshFileName, string resultsFileName )
        {
            string meshFileLongName = Utility.FindMediaFile(meshFileName);
            string resultsFileLongName = Utility.FindMediaFile(resultsFileName);

            prtMesh.LoadMesh(device, meshFileName);

            // Setup the camera's view parameters
            Vector3 eyePt = new Vector3(0.0f, 0.0f, -5.0f);
            Vector3 lookAtPt = new Vector3(0.0f, 0.0f, -0.0f);
            camera.Reset();
            camera.SetViewQuat( new Quaternion(0.0f, 0.0f, 0.0f, 1.0f) );
            camera.SetWorldQuat( new Quaternion(0.0f, 0.0f, 0.0f, 1.0f) );
            camera.SetViewParameters( eyePt, lookAtPt );
            // Update camera's viewing radius
            currentObjectRadius = prtMesh.Radius;
            camera.SetRadius( currentObjectRadius * 3.0f, currentObjectRadius * 0.1f, currentObjectRadius * 20.0f );
            camera.SetModelCenter( prtMesh.Center );

            for( int i = 0; i < MaximumLights; i++ )
                lightControl[i].Radius = currentObjectRadius;

            bool loadCompressed = false;
            if(resultsFileLongName.EndsWith(".pca"))
                loadCompressed = true;

            if( loadCompressed )
            {
                prtMesh.LoadCompPrtBufferFromFile( resultsFileLongName );
            }
            else
            {
                prtMesh.LoadPrtBufferFromFile( resultsFileLongName );
                prtMesh.CompressBuffer(CompressionQuality.FastLowQuality, 1, 24);
            }

            prtMesh.ExtractCompressedDataForPrtShader();
            prtMesh.LoadEffects( device, SampleFramework.DeviceCaps);

            applicationState = ApplicationState.RenderScene;
            Dialog.SetRefreshTime((float)FrameworkTimer.GetTime()); 
            IsRenderCompressionUI = false;
        }

        void ResetUI()
        {
            renderingUI.GetComboBox(EnvironmentA).SetSelectedByData( 0 );
            renderingUI.GetComboBox(EnvironmentB).SetSelectedByData( 2 );
            renderingUI.GetSlider(EnvironmentOne).Value = 50; 
            renderingUI.GetSlider(EnvironmentTwo).Value = 50; 
            renderingUI.GetSlider(EnvironmentBlend).Value = 0;

            renderingUI2.GetCheckbox( RenderUI).IsChecked = true;
            renderingUI2.GetCheckbox( RenderMap).IsChecked = true;
            IsRenderEnvMap = true;
            renderingUI2.GetCheckbox( RenderArrows).IsChecked = true;
            renderingUI2.GetCheckbox( RenderMesh).IsChecked = true;
            renderingUI2.GetCheckbox( RenderTexture).IsChecked = true;
            renderingUI2.GetCheckbox( WireFrame ).IsChecked = false;
            renderingUI2.GetCheckbox( SHProjection).IsChecked = false;

            renderingUI2.GetSlider( LightScale ).SetRange( 0, 200 );
            renderingUI2.GetSlider( LightScale ).Value = 0;
            renderingUI2.GetSlider( NumberLights).Value = 1;

            string str = string.Format("# Lights: {0}", renderingUI2.GetSlider( NumberLights).Value );
            renderingUI2.GetStaticText( NumberLightsStatic).SetText( str );
            numberActiveLights = renderingUI2.GetSlider( NumberLights).Value;
            activeLight %= numberActiveLights;

            renderingUI2.GetSlider( LightAngle).Value = 45;
            int lightAngle = renderingUI2.GetSlider( LightAngle).Value;
            str = string.Format("Cone Angle: {0}", lightAngle );
            renderingUI2.GetStaticText( LightAngleStatic).SetText( str );

            renderingUI3.GetRadioButton( TechniqueNDotL).SetChecked( false, true );
            renderingUI3.GetRadioButton( TechniqueSHIrrad).SetChecked( false, true );
            renderingUI3.GetRadioButton( TechniquePrt).SetChecked( true, true );
        }

        private void UpdateLightingEnvironment()
        {
            float env1 = renderingUI.GetSlider( EnvironmentOne ).Value / 100.0f;
            float env2 = renderingUI.GetSlider( EnvironmentTwo ).Value / 100.0f;
            float envBlend = renderingUI.GetSlider( EnvironmentBlend ).Value / 100.0f;
            float lightScale = renderingUI2.GetSlider( LightScale ).Value * 0.01f;
            float coneRadius = (float) Math.PI * renderingUI2.GetSlider( LightAngle ).Value / 180.0f;

            float[,][] light = new float[MaximumLights, 3][];
            for(int i = 0; i < MaximumLights; i++)
                for(int j = 0; j < 3; j++)
                    light[i,j] = new float[SphericalHarmonics.MaximumOrder * SphericalHarmonics.MaximumOrder];

            // Calculate light scale color
            ColorValue lightColor = ColorOperator.Scale(PrtPerVertex.White, lightScale);

            int order = prtMesh.Order;

            // Pass in the light direction, the intensity of each channel, and it returns
            // the source radiance as an array of order^2 SH coefficients for each color channel.  
            Vector3 lightDirObjectSpace;
            Matrix worldInv;
            worldInv = Matrix.Invert(camera.WorldMatrix);

            for( int i = 0; i < numberActiveLights; i++ )
            {
                // Transform the world space light dir into object space
                // Note that if there's multiple objects using Prt in the scene, then
                // for each object you need to either evaulate the lights in object space
                // evaulate the lights in world and rotate the light coefficients 
                // into object space.
                lightDirObjectSpace = Vector3.TransformNormal(lightControl[i].LightDirection, worldInv);

                // This sample uses SphericalHarmonics.EvaluateDirectionalLight(), but there's other 
                // types of lights provided by SphericalHarmonics.Evaluate*.  Pass in the 
                // order of SH, color of the light, and the direction of the light 
                // in object space.
                // The output is the source radiance coefficients for the SH basis functions.  
                // There are 3 outputs, one for each channel (R,G,B). 
                // Each output is an array of order^2 floats.
                SphericalHarmonics.EvaluateConeLight(order, lightDirObjectSpace, coneRadius, 
                    lightColor.Red, lightColor.Green, lightColor.Blue, light[i,0], light[i,1], light[i,2]);
            }

            float[][] sum = new float[3][];
            for(int i = 0; i < 3; i++)
                sum[i] = new float[SphericalHarmonics.MaximumOrder * SphericalHarmonics.MaximumOrder];

            // For multiple lights, just them sum up using SphericalHarmonics.Add()
            for( int i = 0; i < numberActiveLights; i++ )
            {
                // SphericalHarmonics.Add() will add order^2 floats.  There are 3 color channels, 
                // so call it 3 times.
                SphericalHarmonics.Add( sum[0], order, sum[0], light[i, 0] );
                SphericalHarmonics.Add( sum[1], order, sum[1], light[i, 1] );
                SphericalHarmonics.Add( sum[2], order, sum[2], light[i, 2] );
            }

            float[][] skybox1 = new float[3][];
            for( int i = 0; i < 3; i++)
                skybox1[i] = new float[SphericalHarmonics.MaximumOrder * SphericalHarmonics.MaximumOrder];

            float[][] skybox1Rot = new float[3][];
            for( int i = 0; i < 3; i++)
                skybox1Rot[i] = new float[SphericalHarmonics.MaximumOrder * SphericalHarmonics.MaximumOrder];

            SphericalHarmonics.Scale( skybox1[0], order, skyboxLightSH[skyboxA, 0], env1 * (1.0f - envBlend) );
            SphericalHarmonics.Scale( skybox1[1], order, skyboxLightSH[skyboxA, 1], env1 * (1.0f - envBlend) );
            SphericalHarmonics.Scale( skybox1[2], order, skyboxLightSH[skyboxA, 2], env1 * (1.0f - envBlend) );
            SphericalHarmonics.Rotate( skybox1Rot[0], order, worldInv, skybox1[0] );
            SphericalHarmonics.Rotate( skybox1Rot[1], order, worldInv, skybox1[1] );
            SphericalHarmonics.Rotate( skybox1Rot[2], order, worldInv, skybox1[2] );
            SphericalHarmonics.Add( sum[0], order, sum[0], skybox1Rot[0] );
            SphericalHarmonics.Add( sum[1], order, sum[1], skybox1Rot[1] );
            SphericalHarmonics.Add( sum[2], order, sum[2], skybox1Rot[2] );

            float[][] skybox2 = new float[3][];
            for( int i = 0; i < 3; i++)
                skybox2[i] = new float[SphericalHarmonics.MaximumOrder * SphericalHarmonics.MaximumOrder];

            float[][] skybox2Rot = new float[3][];
            for( int i = 0; i < 3; i++)
                skybox2Rot[i] = new float[SphericalHarmonics.MaximumOrder * SphericalHarmonics.MaximumOrder];

            SphericalHarmonics.Scale( skybox2[0], order, skyboxLightSH[skyboxB, 0], env2 * envBlend );
            SphericalHarmonics.Scale( skybox2[1], order, skyboxLightSH[skyboxB, 1], env2 * envBlend );
            SphericalHarmonics.Scale( skybox2[2], order, skyboxLightSH[skyboxB, 2], env2 * envBlend );
            SphericalHarmonics.Rotate( skybox2Rot[0], order, worldInv, skybox2[0] );
            SphericalHarmonics.Rotate( skybox2Rot[1], order, worldInv, skybox2[1] );
            SphericalHarmonics.Rotate( skybox2Rot[2], order, worldInv, skybox2[2] );
            SphericalHarmonics.Add( sum[0], order, sum[0], skybox2Rot[0] );
            SphericalHarmonics.Add( sum[1], order, sum[1], skybox2Rot[1] );
            SphericalHarmonics.Add( sum[2], order, sum[2], skybox2Rot[2] );

            prtMesh.ComputeShaderConstants( sum[0], sum[1], sum[2], order * order );
            prtMesh.ComputeSHIrradEnvMapConstants( sum[0], sum[1], sum[2] );
        }

        private void RenderSimulatorRunning(Device device, double appTime, float elapsedTime)
        {
            bool beginSceneCalled = false;

            // Clear the render target and the zbuffer 
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, ClearColor1 , 1.0f, 0);

            // Render the scene
            try
            {
                device.BeginScene();
                beginSceneCalled = true;

                RenderText();
                simulatorRunningUI.OnRender( elapsedTime );
            }
            finally
            {
                if (beginSceneCalled)
                    device.EndScene();
            }
        }

        //--------------------------------------------------------------------------------------
        private void RenderPrt(Device device, double appTime, float elapsedTime)
        {
            bool beginSceneCalled = false;

            device.SetRenderState(RenderStates.FillMode, (int)(IsWireframe? FillMode.WireFrame : FillMode.Solid));

            // Clear the render target and the zbuffer 
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, Color.Black , 1.0f, 0);

            // Render the scene
            try
            {
                device.BeginScene();
                beginSceneCalled = true;
                
                Matrix worldMatrix = camera.WorldMatrix;
                Matrix viewMatrix = camera.ViewMatrix;
                Matrix projectionMatrix = camera.ProjectionMatrix;
                Matrix worldViewProjection = worldMatrix * viewMatrix * projectionMatrix;

                if( IsRenderEnvMap && !IsWireframe )
                {
                    float env1 = renderingUI.GetSlider( EnvironmentOne ).Value / 100.0f;
                    float env2 = renderingUI.GetSlider( EnvironmentTwo ).Value / 100.0f;
                    float envBlend = renderingUI.GetSlider( EnvironmentBlend ).Value / 100.0f;

                    device.SetRenderState( RenderStates.AlphaBlendEnable, true );
                    device.SetRenderState( RenderStates.SourceBlend, (int) Blend.SourceAlpha );
                    device.SetRenderState( RenderStates.DestinationBlend, (int) Blend.One);

                    skybox[skyboxA].DrawSH = IsRenderSHProjection;
                    skybox[skyboxB].DrawSH = IsRenderSHProjection;

                    Matrix viewProjection = viewMatrix * projectionMatrix;
                    skybox[skyboxA].Render( viewProjection, (1.0f - envBlend), env1 );
                    skybox[skyboxB].Render( viewProjection, (envBlend), env2 );

                    device.SetRenderState( RenderStates.AlphaBlendEnable, false );
                }

                float lightScale = renderingUI2.GetSlider( LightScale ).Value * 0.01f;
                if( IsRenderArrows && lightScale > 0.0f )
                {
                    // Render the light spheres so the user can visually see the light direction
                    for( int i = 0; i < numberActiveLights; i++ )
                    {
                        ColorValue arrowColor = ( i == activeLight ) ? Yellow : White;
                        lightControl[i].OnRender( arrowColor, viewMatrix, projectionMatrix, camera.EyeLocation);
                    }
                }

                if( IsRenderMesh )
                {
                    switch( technique )
                    {
                        case 0: // Prt
                            prtMesh.RenderWithPrt( device, worldViewProjection, IsRenderWithAlbedoTexture );
                            break;

                        case 1: // SHIrradEnvMap
                            prtMesh.RenderWithSHIrradEnvMap( device, worldViewProjection, IsRenderWithAlbedoTexture );
                            break;

                        case 2: // N dot L
                        {
                            Matrix worldInv = Matrix.Invert(camera.WorldMatrix);
                            prtMesh.RenderWithNDotL( device, worldViewProjection, worldInv, IsRenderWithAlbedoTexture, lightControl, numberActiveLights, lightScale );
                            break;
                        }
                    }
                }
                RenderText();
                if( IsRenderUI )
                {
                    hud.OnRender( elapsedTime );
                    renderingUI.OnRender(elapsedTime);
                    renderingUI2.OnRender(elapsedTime);
                    renderingUI3.OnRender(elapsedTime);
                    if( IsRenderCompressionUI )
                        compressionUI.OnRender(elapsedTime);
                }
            }
            finally
            {
                if (beginSceneCalled)
                    device.EndScene();
            }
        }

        /// <summary>
        /// This callback function will be called at the end of every frame to perform all the 
        /// rendering calls for the scene, and it will also be called if the window needs to be 
        /// repainted. After this function has returned, the sample framework will call 
        /// Device.Present to display the contents of the next buffer in the swap chain
        /// </summary>
        public void OnFrameRender(Device device, double appTime, float elapsedTime)
        {
            switch( applicationState )
            {
                case ApplicationState.Startup: 
                    RenderStartup( device, appTime, elapsedTime ); 
                    break;

                case ApplicationState.SimulatorOptions:
                {
                    PrtOptionsForm dlg = new PrtOptionsForm();
                    RetryDialog:
                    // Reset state so multiple load dialogs don't show
                    applicationState = ApplicationState.Startup;
                    System.Windows.Forms.DialogResult result = dlg.ShowDialog();
                    if( result == System.Windows.Forms.DialogResult.OK )
                    {                
                        SimulatorOptions options = dlg.GetOptions();
                        prtMesh.LoadMesh( device, options.InputMesh );

                        SetRadii();

                        Simulator.Run( device, options, prtMesh );
                        applicationState = ApplicationState.SimulatorRunning;
                    }
                    else if (result == System.Windows.Forms.DialogResult.Retry)
                    {
                        goto RetryDialog;
                    }
                    else
                    {
                        applicationState = ApplicationState.Startup; 
                    }
                    break;
                }

                case ApplicationState.SimulatorRunning:
                {
                    string str;
                    if( Simulator.PercentComplete >= 0.0f )
                        str = String.Format("Step {0} of {1}: {2:f1}% done", Simulator.CurrentPass, Simulator.NumPasses, Simulator.PercentComplete );
                    else
                        str = String.Format("Step {0} of {1} (progress n/a)", Simulator.CurrentPass, Simulator.NumPasses );
                    simulatorRunningUI.GetStaticText( SimulatorStatus ).SetText( str );
                    simulatorRunningUI.GetStaticText( SimulatorStatus2 ).SetText( Simulator.CurrentPassName );

                    RenderSimulatorRunning( device, appTime, elapsedTime );
                    System.Threading.Thread.Sleep(50); // Yield time to simulator thread

                    if( !Simulator.IsRunning )
                    {
                        prtMesh.LoadEffects( device, SampleFramework.DeviceCaps);
                        applicationState = ApplicationState.RenderScene;
                        Dialog.SetRefreshTime((float)FrameworkTimer.GetTime()); 
                        IsRenderCompressionUI = false;
                    }
                    RenderSimulatorRunning( device, appTime, elapsedTime );
                    break;
                }
                   
                case ApplicationState.LoadPrtBuffer:
                {
                    PrtLoadForm dlg = new PrtLoadForm();
                    // Reset state so multiple load dialogs don't show
                    applicationState = ApplicationState.Startup;
                    if( dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK )
                    {
                        SimulatorOptions options = dlg.GetOptions();
                        if( !options.IsAdaptive)
                            LoadScene( device, options.InputMesh, options.ResultsFileName );
                        else
                            LoadScene( device, options.OutputMesh, options.ResultsFileName );
                    }
                    else
                    {
                        applicationState = ApplicationState.Startup;
                    }
                    break;
                }

                case ApplicationState.RenderScene: 
                    UpdateLightingEnvironment();
                    RenderPrt( device, appTime, elapsedTime ); 
                    break;
            
                default:
                    System.Diagnostics.Debug.Assert(false);
                    break;
            }
//          abra = false;
        }

        /// <summary>
        /// Render the help and statistics text. This function uses the Font object for 
        /// efficient text rendering.
        /// </summary>
        private void RenderText()
        {
            if(!IsRenderText) return;

            // The helper object simply helps keep track of text position, and color
            // and then it calls textFont.DrawText(textSprite, text, rect, DrawTextFormat.NoClip, color);
            // If null is passed in as the sprite object, then it will work however the 
            // textFont.DrawText() will not be batched together.  Batching calls improves performance.
            TextHelper txtHelper = new TextHelper(statsFont, textSprite, 15);

            // Output statistics
            txtHelper.Begin();
            txtHelper.SetInsertionPoint(5,5);
            txtHelper.SetForegroundColor(Color.Yellow);
            txtHelper.DrawTextLine(SampleFramework.FrameStats);
            txtHelper.DrawTextLine(SampleFramework.DeviceStats);

            if(!IsRenderUI)
                txtHelper.DrawTextLine("Press '4' to show UI");

            txtHelper.End();
        }

        /// <summary>
        /// As a convenience, the sample framework inspects the incoming windows messages for
        /// keystroke messages and decodes the message parameters to pass relevant keyboard
        /// messages to the application.  The framework does not remove the underlying keystroke 
        /// messages, which are still passed to the application's MsgProc callback.
        /// </summary>
        private void OnKeyEvent(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            switch(e.KeyCode)
            {
                // Demo hotkeys
                case System.Windows.Forms.Keys.Z: 
                {
                    LoadSceneAndOptGenResults( SampleFramework.Device, "PRT Demo\\cube_on_plane.x", 
                            "PRT Demo\\cube_on_plane_1k_6b_prtresults.pca", 1024, 6, false ); 

                    renderingUI.IsUsingNonUserEvents = true;
                    renderingUI2.IsUsingNonUserEvents = true;
                    renderingUI3.IsUsingNonUserEvents = true;
                    ResetUI();
                    renderingUI2.GetCheckbox( RenderTexture ).IsChecked = false;
                    renderingUI.IsUsingNonUserEvents = false;
                    renderingUI2.IsUsingNonUserEvents = false;
                    renderingUI3.IsUsingNonUserEvents = false;
                    break;
                }

                case System.Windows.Forms.Keys.X: 
                {
                    LoadSceneAndOptGenResults( SampleFramework.Device, "PRT Demo\\LandShark.x", 
                            "PRT Demo\\02_LandShark_1k_prtresults.pca", 1024, 1, false ); 

                    renderingUI.IsUsingNonUserEvents = false;
                    renderingUI2.IsUsingNonUserEvents = false;
                    renderingUI3.IsUsingNonUserEvents = false;
                    ResetUI();
                    renderingUI.IsUsingNonUserEvents = false;
                    renderingUI2.IsUsingNonUserEvents = false;
                    renderingUI3.IsUsingNonUserEvents = false;
                    break;
                }

                case System.Windows.Forms.Keys.C: 
                {
                    LoadSceneAndOptGenResults( SampleFramework.Device, "PRT Demo\\wall_with_pillars.x", 
                        "PRT Demo\\wall_with_pillars_1k_prtresults.pca", 1024, 1, false ); 

                    renderingUI.IsUsingNonUserEvents = true;
                    renderingUI2.IsUsingNonUserEvents = true;
                    renderingUI3.IsUsingNonUserEvents = true;
                    ResetUI();
                    renderingUI.IsUsingNonUserEvents = false;
                    renderingUI2.IsUsingNonUserEvents = false;
                    renderingUI3.IsUsingNonUserEvents = false;
                    break;
                }

                case System.Windows.Forms.Keys.V: 
                {
                    LoadSceneAndOptGenResults( SampleFramework.Device, "PRT Demo\\Head_Sad.x", 
                        "PRT Demo\\Head_Sad_1k_prtresults.pca", 1024, 1, false ); 

                    renderingUI.IsUsingNonUserEvents = true;
                    renderingUI2.IsUsingNonUserEvents = true;
                    renderingUI3.IsUsingNonUserEvents = true;
                    ResetUI();
                    renderingUI.IsUsingNonUserEvents = false;
                    renderingUI2.IsUsingNonUserEvents = false;
                    renderingUI3.IsUsingNonUserEvents = false;
                    break;
                }

                case System.Windows.Forms.Keys.B: 
                {
                    LoadSceneAndOptGenResults( SampleFramework.Device, "PRT Demo\\Head_Big_Ears.x", 
                        "PRT Demo\\Head_Big_Ears_1k_subsurf_prtresults.pca", 1024, 1, true ); 

                    renderingUI.IsUsingNonUserEvents = true;
                    renderingUI2.IsUsingNonUserEvents = true;
                    renderingUI3.IsUsingNonUserEvents = true;
                    ResetUI();
                    renderingUI.GetSlider(EnvironmentOne).Value = 0; 
                    renderingUI.GetSlider(EnvironmentTwo).Value = 0;
                    renderingUI2.GetSlider( LightScale).SetRange( 0, 1000 );
                    renderingUI2.GetSlider( LightScale).Value = 200;
                    renderingUI.IsUsingNonUserEvents = false;
                    renderingUI2.IsUsingNonUserEvents = false;
                    renderingUI3.IsUsingNonUserEvents = false;
                    break;
                }

                case System.Windows.Forms.Keys.F1: isHelpShowing = !isHelpShowing; break;
                case System.Windows.Forms.Keys.F8:
                case System.Windows.Forms.Keys.W: IsWireframe = !IsWireframe; renderingUI2.GetCheckbox( WireFrame).IsChecked = IsWireframe; break;
            }
        }

        /// <summary>
        /// Before handling window messages, the sample framework passes incoming windows 
        /// messages to the application through this callback function. If the application sets 
        /// noFurtherProcessing to true, the sample framework will not process the message
        /// </summary>
        public IntPtr OnMsgProc(IntPtr hWnd, NativeMethods.WindowMessage msg, IntPtr wParam, IntPtr lParam, ref bool noFurtherProcessing)
        {
            if( msg == NativeMethods.WindowMessage.KeyDown && wParam.ToInt32() == (Int32) System.Windows.Forms.Keys.F8 )
                noFurtherProcessing = true;

            // Give the dialog a chance to handle the message first
            noFurtherProcessing = hud.MessageProc(hWnd, msg, wParam, lParam);
            if (noFurtherProcessing)
                return IntPtr.Zero;

            switch( applicationState)
            {
                case ApplicationState.Startup:
                    // Give the dialog a chance to handle the message first
                    noFurtherProcessing = startUpUI.MessageProc(hWnd, msg, wParam, lParam);
                    if (noFurtherProcessing)
                        return IntPtr.Zero;

                    // Give the dialog a chance to handle the message first
                    noFurtherProcessing = startUpUI2.MessageProc(hWnd, msg, wParam, lParam);
                    if (noFurtherProcessing)
                        return IntPtr.Zero;

                    break;

                case ApplicationState.SimulatorRunning:
                    // Give the dialog a chance to handle the message first
                    noFurtherProcessing = simulatorRunningUI.MessageProc(hWnd, msg, wParam, lParam);
                    if (noFurtherProcessing)
                        return IntPtr.Zero;

                    break;

                case ApplicationState.RenderScene:
                    if(IsRenderUI || msg == NativeMethods.WindowMessage.KeyDown || msg == NativeMethods.WindowMessage.KeyUp)
                    {
                        // Give the dialog a chance to handle the message first
                        noFurtherProcessing = renderingUI.MessageProc(hWnd, msg, wParam, lParam);
                        if (noFurtherProcessing)
                            return IntPtr.Zero;

                        // Give the dialog a chance to handle the message first
                        noFurtherProcessing = renderingUI2.MessageProc(hWnd, msg, wParam, lParam);
                        if (noFurtherProcessing)
                            return IntPtr.Zero;

                        // Give the dialog a chance to handle the message first
                        noFurtherProcessing = renderingUI3.MessageProc(hWnd, msg, wParam, lParam);
                        if (noFurtherProcessing)
                            return IntPtr.Zero;

                        if(IsRenderCompressionUI)
                        {
                            // Give the dialog a chance to handle the message first
                            noFurtherProcessing = compressionUI.MessageProc(hWnd, msg, wParam, lParam);
                            if (noFurtherProcessing)
                                return IntPtr.Zero;
                        }
                    }

                    lightControl[activeLight].HandleMessages( hWnd, msg, wParam, lParam );

                    // Pass all remaining windows messages to camera so it can respond to user input
                    camera.HandleMessages(hWnd, msg, wParam, lParam);

                    break;

                default:
                    break;
            }

            return IntPtr.Zero;
        }

        /// <summary>
        /// Initializes the application
        /// </summary>
        public void InitializeApplication()
        {
            for(int i = 0; i < MaximumLights; i++)
            {
                lightControl[i] = new DirectionWidget();
                lightControl[i].LightDirection = new Vector3( (float) Math.Sin(Math.PI * 2 * i / MaximumLights - Math.PI / 6),
                    0, -(float)Math.Cos(Math.PI * 2 * i / MaximumLights - Math.PI / 6)) ;
                lightControl[i].RotateButtonMask = MouseButtonMask.Middle;
            }

            activeLight = 0;
            numberActiveLights = 1;

            int y = 10;
            // Initialize the HUD
            Button fullScreenButton = hud.AddButton(ToggleFullscreen,"Toggle full screen", 35, y, 125,22);
            Button toggleRefButton = hud.AddButton(ToggleReference,"Toggle reference (F3)", 35, y += 24, 125,22);
            Button changeDeviceButton = hud.AddButton(ChangeDevice,"Change Device (F2)", 35, y += 24, 125,22);
            // Hook the button events for when these items are clicked
            fullScreenButton.Click += new EventHandler(OnFullscreenClicked);
            toggleRefButton.Click += new EventHandler(OnRefClicked);
            changeDeviceButton.Click += new EventHandler(OnChangeDeviceClicked);

            // Now add the sample specific UI

            // Title font for comboboxes
            startUpUI.SetFont( 1, "Arial", 24, FontWeight.Bold );
            Element element = startUpUI.GetDefaultElement( ControlType.StaticText, 0 );
            element.FontIndex = 1;
            element.textFormat = DrawTextFormat.Center | DrawTextFormat.Bottom;

            y = 10; 
            // Initialize startUpUI
            startUpUI.AddStatic( -1, "What would you like to do?", 0, 10, 300, 22 );
            Button simulatorButton      = startUpUI.AddButton(SimulatorInt32, "Run Prt simulator", 90, 42, 125, 40 );
            Button loadPrtBufferButton  = startUpUI.AddButton(LoadPrtBuffer, "View saved results", 90, 84, 125, 40 );
            // Hook the button events for when these items are clicked
            simulatorButton.Click       += new EventHandler(OnSimulatorClicked);
            loadPrtBufferButton.Click   += new EventHandler(OnLoadPrtClicked);

            y = 10; 
            // Initialize startUpUI2
            Button scene1Button = startUpUI2.AddButton(Scene1, "Demo Scene 1 (Z)", 0, y += 24, 125, 22);
            Button scene2Button = startUpUI2.AddButton(Scene2, "Demo Scene 2 (X)", 0, y += 24, 125, 22);
            Button scene3Button = startUpUI2.AddButton(Scene3, "Demo Scene 3 (C)", 0, y += 24, 125, 22);
            Button scene4Button = startUpUI2.AddButton(Scene4, "Demo Scene 4 (V)", 0, y += 24, 125, 22);
            Button scene5Button = startUpUI2.AddButton(Scene5, "Demo Scene 5 (B)", 0, y += 24, 125, 22);
            // Hook the button events for when these items are clicked
            scene1Button.Click += new EventHandler(OnScene1Clicked);
            scene2Button.Click += new EventHandler(OnScene2Clicked);
            scene3Button.Click += new EventHandler(OnScene3Clicked);
            scene4Button.Click += new EventHandler(OnScene4Clicked);
            scene5Button.Click += new EventHandler(OnScene5Clicked);

            // Title font for simulatorRunningUI
            simulatorRunningUI.SetFont( 1, "Arial", 24, FontWeight.Bold );
            element = simulatorRunningUI.GetDefaultElement( ControlType.StaticText, 0 );
            element.FontIndex = 1;
            element.textFormat = DrawTextFormat.Center | DrawTextFormat.Bottom;

            y = 10; 
            // Initialize simulatorRunningUI
            simulatorRunningUI.AddStatic( SimulatorStatus, String.Empty, 0, 0, 600, 22 );
            simulatorRunningUI.AddStatic( SimulatorStatus2, String.Empty, 0, 30, 600, 22 );
            Button stopSimulatorButton = simulatorRunningUI.AddButton( StopSimulator, "Stop Prt simulator", 240, 60, 125, 40 );
            // Hook the button events for when these items are clicked
            stopSimulatorButton.Click += new EventHandler(OnStopSimulatorClicked);

            // Title font for renderingUI2
            renderingUI2.SetFont( 1, "Arial", 14, FontWeight.Bold );
            element = renderingUI2.GetDefaultElement( ControlType.StaticText, 0 );
            element.FontIndex = 1;
            element.textFormat = DrawTextFormat.Left | DrawTextFormat.Bottom;

            y = 10; 
            // Initialize renderingUI2
            Checkbox renderUICB = renderingUI2.AddCheckBox( RenderUI, "Show UI (4)", 35, y += 24, 125, 22, IsRenderUI, System.Windows.Forms.Keys.D4, true);
            Checkbox renderMapCB = renderingUI2.AddCheckBox( RenderMap, "Background (5)", 35, y += 24, 125, 22, IsRenderEnvMap, System.Windows.Forms.Keys.D5, true);
            Checkbox renderArrowsCB = renderingUI2.AddCheckBox( RenderArrows, "Arrows (6)", 35, y += 24, 125, 22, IsRenderArrows, System.Windows.Forms.Keys.D6, true);  
            Checkbox renderMeshCB = renderingUI2.AddCheckBox( RenderMesh, "Mesh (7)", 35, y += 24, 125, 22, IsRenderMesh, System.Windows.Forms.Keys.D7, true);   
            Checkbox renderTextureCB = renderingUI2.AddCheckBox( RenderTexture, "Texture (8)", 35, y += 24, 125, 22, IsRenderWithAlbedoTexture, System.Windows.Forms.Keys.D8, true);   
            Checkbox renderWireFrameCB = renderingUI2.AddCheckBox( WireFrame, "Wireframe (W)", 35, y += 24, 125, 22, IsWireframe ); 
            Checkbox renderSHProjectionCB = renderingUI2.AddCheckBox( SHProjection, "SH Projection (H)", 35, y += 24, 125, 22, IsRenderSHProjection, System.Windows.Forms.Keys.H, true);   
            // Add the event handlers
            renderUICB.Changed += new EventHandler(OnRenderUICBChanged);
            renderMapCB.Changed += new EventHandler(OnRenderMapCBChanged);
            renderArrowsCB.Changed += new EventHandler(OnRenderArrowsCBChanged);
            renderMeshCB.Changed += new EventHandler(OnRenderMeshCBChanged);
            renderTextureCB.Changed += new EventHandler(OnRenderTextureCBChanged);
            renderWireFrameCB.Changed += new EventHandler(OnRenderWireFrameCBChanged);
            renderSHProjectionCB.Changed += new EventHandler(OnRenderSHProjectionCBChanged);
            
            string str;

            y += 10;
            
            // Initialize renderingUI2 more
            str = String.Format("Light scale: {0:f2}", 0.0f );
            renderingUI2.AddStatic( LightScaleStatic, str, 35, y += 24, 125, 22 );
            Slider lightScaleSlider = renderingUI2.AddSlider( LightScale, 50, y += 24, 100, 22, 0, 200, (int)(lightScaleForPrt * 100.0f), true);
            // Add the event handler
            lightScaleSlider.ValueChanged += new EventHandler(OnLightScaleSliderChanged);

            str = String.Format("# Lights: {0}", numberActiveLights);
            renderingUI2.AddStatic( NumberLightsStatic, str, 35, y += 24, 125, 22 );
            Slider numberLightsSlider = renderingUI2.AddSlider( NumberLights, 50, y += 24, 100, 22, 1, MaximumLights, numberActiveLights, true );
            // Add the event handler
            numberLightsSlider.ValueChanged += new EventHandler(OnNumLightsSliderChanged);

            str = String.Format("Cone Angle: 45");
            renderingUI2.AddStatic( LightAngleStatic, str, 35, y += 24, 125, 22 );
            Slider lightAngleSlider = renderingUI2.AddSlider( LightAngle, 50, y += 24, 100, 22, 1, 180, 45, true);
            // Add the event handler
            lightAngleSlider.ValueChanged += new EventHandler(OnLightAngleSliderChanged);

            y += 5;
            Button activeLightButton = renderingUI2.AddButton( ActiveLight, "Change active light (K)", 35, y += 24, 125, 22, System.Windows.Forms.Keys.K, true );

            y += 5;
            Button compressionButton = renderingUI2.AddButton( Compression, "Compression Settings", 35, y += 24, 125, 22 );

            y += 5;
            Button restartButton = renderingUI2.AddButton( Restart, "Restart", 35, y += 24, 125, 22 );

            // Add the event handlers
            activeLightButton.Click += new EventHandler(OnActiveLightButtonClicked);
            compressionButton.Click += new EventHandler(OnCompressionButtonClicked);
            restartButton.Click += new EventHandler(OnRestartButtonClicked);

            bool isEnable = lightScaleSlider.Value != 0;
            lightAngleSlider.IsEnabled = isEnable;
            numberLightsSlider.IsEnabled = isEnable;
            renderingUI2.GetStaticText(LightAngleStatic).IsEnabled = isEnable;
            renderingUI2.GetStaticText(NumberLightsStatic).IsEnabled = isEnable;
            activeLightButton.IsEnabled = isEnable;

            // Title font for renderingUI3
            renderingUI3.SetFont( 1, "Arial", 14, FontWeight.Bold);
            element = renderingUI3.GetDefaultElement( ControlType.StaticText, 0 );
            element.FontIndex = 1;
            element.textFormat = DrawTextFormat.Left | DrawTextFormat.Bottom;

            y = 0; 
            // Initialize renderingUI3
            renderingUI3.AddStatic( Static, "(T)echnique", 0, y, 150, 25 );
            RadioButton techniquePrt        = renderingUI3.AddRadioButton( TechniquePrt     , 1, "(1) PRT"          , 0, y += 24, 150, 22, true, System.Windows.Forms.Keys.D1, true );
            RadioButton techniqueSHIrradRB  = renderingUI3.AddRadioButton( TechniqueSHIrrad , 1, "(2) SHIrradEnvMap", 0, y += 24, 150, 22, false, System.Windows.Forms.Keys.D2, true );
            RadioButton techniqueNDotLRB    = renderingUI3.AddRadioButton( TechniqueNDotL   , 1, "(3) N dot L"      , 0, y += 24, 150, 22, false, System.Windows.Forms.Keys.D3, true );
            // Add the event handlers
            techniquePrt.Changed += new EventHandler(OnTechniquePrtChanged);
            techniqueSHIrradRB.Changed += new EventHandler(OnTechniqueSHIrradRBChanged); 
            techniqueNDotLRB.Changed += new EventHandler(OnTechniqueNDotLRBChanged);

            // Title font for renderingUI
            renderingUI.SetFont( 1, "Arial", 14, FontWeight.Bold);
            element = renderingUI.GetDefaultElement( ControlType.StaticText, 0 );
            element.FontIndex = 1;
            element.textFormat = DrawTextFormat.Left | DrawTextFormat.Bottom;

            int startY = 0;
            int x = 10;
            y = startY; 
            renderingUI.AddStatic( Static, "(F)irst Light Probe", x, y += 24, 150, 25 );
            ComboBox environmentACB = renderingUI.AddComboBox( EnvironmentA, x, y += 24, 150, 22, System.Windows.Forms.Keys.F, true );   
            environmentACB.SetDropHeight( 30 );

            environmentACB.AddItem( "Eucalyptus Grove", 0 );
            if(NumberSkyBoxes > 1)
                environmentACB.AddItem( "The Uffizi Gallery", 1 );
            if(NumberSkyBoxes > 2)
                environmentACB.AddItem( "Galileo's Tomb", 2 );
            if(NumberSkyBoxes > 3)
                environmentACB.AddItem( "Grace Cathedral", 3 );
            if(NumberSkyBoxes > 4)
                environmentACB.AddItem( "St. Peter's Basilica", 4 );

            environmentACB.SetSelectedByData( skyboxA );
            // Add the event handler
            environmentACB.Changed += new EventHandler(OnEnvironmentACBChanged);

            renderingUI.AddStatic( Static, "First Light Probe Scaler", x, y += 24, 150, 25 );
            Slider environmentOneSlider = renderingUI.AddSlider( EnvironmentOne, x, y += 24, 150, 22, 0, 100, 50, true );

            x += 175;
            y = startY + 30; 
            renderingUI.AddStatic( Static, "Light Probe Blend", x, y += 24, 100, 25 );
            Slider environmentBlendSlider = renderingUI.AddSlider( EnvironmentBlend, x, y += 24, 100, 22, 0, 100, 0, true );   

            x += 125;
            y = startY; 
            renderingUI.AddStatic( Static, "(S)econd Light Probe", x, y += 24, 150, 25 );
            ComboBox environmentBCB = renderingUI.AddComboBox( EnvironmentB, x, y += 24, 150, 22, System.Windows.Forms.Keys.S, true );
            environmentBCB.SetDropHeight( 30 );

            environmentBCB.AddItem( "Eucalyptus Grove", 0 );
            if(NumberSkyBoxes > 1)
                environmentBCB.AddItem( "The Uffizi Gallery", 1 );
            if(NumberSkyBoxes > 2)
                environmentBCB.AddItem( "Galileo's Tomb", 2 );
            if(NumberSkyBoxes > 3)
                environmentBCB.AddItem( "Grace Cathedral", 3 );
            if(NumberSkyBoxes > 4)
                environmentBCB.AddItem( "St. Peter's Basilica", 4 );

            environmentBCB.SetSelectedByData( skyboxB );
            // Add the event handler
            environmentBCB.Changed += new EventHandler(OnEnvironmentBCBChanged);

            renderingUI.AddStatic( Static, "Second Light Probe Scaler", x, y += 24, 150, 25 );
            Slider environmentTwoSlider = renderingUI.AddSlider( EnvironmentTwo, x, y += 24, 150, 22, 0, 100, 50, true );

            y = 10;

            SimulatorOptions options = PrtOptions.GlobalOptions;

            // CompressionUI
            compressionUI.AddStatic( NumberPcaVectors, "Number of Pca Vectors: 24", 30, y += 24, 120, 22 );
            Slider numberPcaSlider = compressionUI.AddSlider( NumberPcaVectors, 30, y += 24, 120, 22, 1, 6, options.NumberPcaVectors / 4, true );
            numberPcaSlider.ValueChanged += new EventHandler(OnNumberPcaSliderValueChanged);

            compressionUI.AddStatic( NumberClusters, "Number of Clusters: 1", 30, y += 24, 120, 22 );
            Slider numberClustersSlider = compressionUI.AddSlider( NumberClusters, 30, y += 24, 120, 22, 1, 40, options.NumberClusters, true );
            numberClustersSlider.ValueChanged += new EventHandler(OnNumberClustersSliderValueChanged);

            compressionUI.AddStatic( MaximumConstants, "Maximum VS Constants: 0", 30, y += 24, 120, 22 );
            compressionUI.AddStatic( CurrentConstantsStatic, "Current VS Constants:", 30, y += 24, 120, 22 );
            compressionUI.AddStatic( CurrentConstants, "0", -10, y += 12, 250, 22 );
            Button applyButton = compressionUI.AddButton( Apply, "Apply", 30, y += 24, 120, 24 );
            applyButton.Click += new EventHandler(OnApplyButtonClicked);

            str = string.Format("Number of Pca: {0}", options.NumberPcaVectors );
            compressionUI.GetStaticText( NumberPcaVectors ).SetText( str );
            str = string.Format("Number of Clusters: {0}", options.NumberClusters );
            compressionUI.GetStaticText( NumberClusters ).SetText( str );           
            UpdateConstText();
        }

        /// <summary>
        /// update the dlg's text & controls
        /// </summary>
        private void UpdateConstText()
        {
            int numberClusters = compressionUI.GetSlider( NumberClusters ).Value;
            int numberPcaVectors        = compressionUI.GetSlider( NumberPcaVectors ).Value * 4;
            int maximumVertexShaderConst = SampleFramework.DeviceCaps.MaxVertexShaderConst;
            int numberVConsts   = numberClusters * (1 + 3 * numberPcaVectors / 4) + 4;
            string str = string.Format("{0} * (1 + (3 * {1} / 4)) + 4 = {2}", numberClusters, numberPcaVectors, numberVConsts );
            compressionUI.GetStaticText( CurrentConstantsStatic ).SetText(str);

            bool isEnable = numberVConsts < maximumVertexShaderConst;
            compressionUI.GetButton( Apply ).IsEnabled = isEnable;
        }


        #region Event Handler Routines

        /* hud */
        /// <summary>Called when the change device button is clicked</summary>
        private void OnChangeDeviceClicked(object sender, EventArgs e)
        {
            SampleFramework.ShowSettingsDialog(!SampleFramework.IsD3DSettingsDialogShowing);
        }

        /// <summary>Called when the full screen button is clicked</summary>
        private void OnFullscreenClicked(object sender, EventArgs e)
        {
            SampleFramework.ToggleFullscreen();
        }

        /// <summary>Called when the ref button is clicked</summary>
        private void OnRefClicked(object sender, EventArgs e)
        {
            SampleFramework.ToggleReference();
        }

        /* startUpUI */
        /// <summary>Called when the Run Prt simulator button is clicked</summary>
        private void OnSimulatorClicked(object sender, EventArgs e)
        {
            applicationState = ApplicationState.SimulatorOptions;
        }

        /// <summary>Called when the View saved results button is clicked</summary>
        private void OnLoadPrtClicked(object sender, EventArgs e)
        {
            applicationState = ApplicationState.LoadPrtBuffer;
        }

        /* startUpUI2 */
        /// <summary>Called when the Demo Scene 1 button is clicked</summary>
        private void OnScene1Clicked(object sender, EventArgs e)
        {
            OnKeyEvent(this, new System.Windows.Forms.KeyEventArgs(System.Windows.Forms.Keys.Z));
        }

        /// <summary>Called when the Demo Scene 2 button is clicked</summary>
        private void OnScene2Clicked(object sender, EventArgs e)
        {
            OnKeyEvent(this, new System.Windows.Forms.KeyEventArgs(System.Windows.Forms.Keys.X));
        }

        /// <summary>Called when the Demo Scene 3 button is clicked</summary>
        private void OnScene3Clicked(object sender, EventArgs e)
        {
            OnKeyEvent(this, new System.Windows.Forms.KeyEventArgs(System.Windows.Forms.Keys.C));
        }

        /// <summary>Called when the Demo Scene 4 button is clicked</summary>
        private void OnScene4Clicked(object sender, EventArgs e)
        {
            OnKeyEvent(this, new System.Windows.Forms.KeyEventArgs(System.Windows.Forms.Keys.V));
        }

        /// <summary>Called when the Demo Scene 5 button is clicked</summary>
        private void OnScene5Clicked(object sender, EventArgs e)
        {
            OnKeyEvent(this, new System.Windows.Forms.KeyEventArgs(System.Windows.Forms.Keys.B));
        }

        /* simulatorRunningUI */
        /// <summary>Called when the Stop Prt simulator button is clicked</summary>
        private void OnStopSimulatorClicked(object sender, EventArgs e)
        {
            Simulator.Stop();
            applicationState = ApplicationState.Startup;
        }

        /* renderingUI2 */
        /// <summary>Called when the Show UI button is clicked</summary>
        private void OnRenderUICBChanged(object sender, EventArgs e)
        {
            IsRenderUI = renderingUI2.GetCheckbox( RenderUI ).IsChecked;
        }

        /// <summary>Called when the Background button is clicked</summary>
        private void OnRenderMapCBChanged(object sender, EventArgs e)
        {
            IsRenderEnvMap = renderingUI2.GetCheckbox( RenderMap ).IsChecked;
        }

        /// <summary>Called when the Arrows button is clicked</summary>
        private void OnRenderArrowsCBChanged(object sender, EventArgs e)
        {
            IsRenderArrows = renderingUI2.GetCheckbox( RenderArrows ).IsChecked;
        }

        /// <summary>Called when the Mesh button is clicked</summary>
        private void OnRenderMeshCBChanged(object sender, EventArgs e)
        {
            IsRenderMesh = renderingUI2.GetCheckbox( RenderMesh ).IsChecked;
        }

        /// <summary>Called when the Texture button is clicked</summary>
        private void OnRenderTextureCBChanged(object sender, EventArgs e)
        {
            IsRenderWithAlbedoTexture = renderingUI2.GetCheckbox( RenderTexture ).IsChecked;
        }

        /// <summary>Called when the Wireframe button is clicked</summary>
        private void OnRenderWireFrameCBChanged(object sender, EventArgs e)
        {
            IsWireframe = renderingUI2.GetCheckbox( WireFrame ).IsChecked;
        }

        /// <summary>Called when the SH Projection button is clicked</summary>
        private void OnRenderSHProjectionCBChanged(object sender, EventArgs e)
        {
            IsRenderSHProjection = renderingUI2.GetCheckbox( SHProjection ).IsChecked;
        }

        /// <summary>Called when the Light scale Slider is changed</summary>
        private void OnLightScaleSliderChanged(object sender, EventArgs e)
        {
            float lightScale = (float) (renderingUI2.GetSlider( LightScale ).Value * 0.01f);
            string str = string.Format("Light scale: {0:f2}", lightScale );
            renderingUI2.GetStaticText( LightScaleStatic ).SetText( str );

            bool isEnable = (renderingUI2.GetSlider( LightScale ).Value != 0 );
            renderingUI2.GetSlider( LightAngle ).IsEnabled = 
            renderingUI2.GetSlider( NumberLights ).IsEnabled = 
            renderingUI2.GetStaticText( LightAngleStatic ).IsEnabled = 
            renderingUI2.GetStaticText( NumberLightsStatic ).IsEnabled = 
            renderingUI2.GetButton( ActiveLight ).IsEnabled = isEnable;
        }

        /// <summary>Called when the # Lights Slider is changed</summary>
        private void OnNumLightsSliderChanged(object sender, EventArgs e)
        {
            if( !lightControl[ActiveLight].IsBeingDragged )
            {
                string str = string.Format("# Lights: {0}", renderingUI2.GetSlider( NumberLights ).Value );
                renderingUI2.GetStaticText( NumberLightsStatic ).SetText( str );

                numberActiveLights = renderingUI2.GetSlider( NumberLights ).Value;
                activeLight %= numberActiveLights;
            }
        }

        /// <summary>Called when the Cone Angle Slider is changed</summary>
        private void OnLightAngleSliderChanged(object sender, EventArgs e)
        {
            int lightAngle = renderingUI2.GetSlider( LightAngle ).Value;
            string str = string.Format("Cone Angle: {0}", lightAngle );
            renderingUI2.GetStaticText( LightAngleStatic ).SetText( str );
        }
        
        /// <summary>Called when the "Change active light (K)" button is clicked</summary>
        private void OnActiveLightButtonClicked(object sender, EventArgs e)
        {
            if( !lightControl[ActiveLight].IsBeingDragged )
            {
                activeLight++;
                activeLight %= numberActiveLights;
            }
        }
        
        /// <summary>Called when the "Compression Settings" button is clicked</summary>
        private void OnCompressionButtonClicked(object sender, EventArgs e)
        {
            if( prtMesh.IsUncompressedBufferLoaded )
            {
                IsRenderCompressionUI = !IsRenderCompressionUI;
            }
            else
            {
                System.Windows.Forms.MessageBox.Show(
                    "To change compression settings during rendering, please load an uncompressed buffer.  To make one use the simulator settings dialog to save an uncompressed buffer.",
                    "PRTPerVertex", 
                    System.Windows.Forms.MessageBoxButtons.OK);
            }
        }

        /// <summary>Called when the "Restart" button is clicked</summary>
        private void OnRestartButtonClicked(object sender, EventArgs e)
        {
            applicationState = ApplicationState.Startup;
        }

        /// <summary>Called when the "Number of Pca Vectors" Slider is changed</summary>
        private void OnNumberPcaSliderValueChanged(object sender, EventArgs e)
        {
            int numberPcaVectors = compressionUI.GetSlider( NumberPcaVectors ).Value * 4;
            string str = string.Format("Number of PCA: {0}", numberPcaVectors );
            compressionUI.GetStaticText( NumberPcaVectors ).SetText( str );           

            UpdateConstText();
        }
        
        /// <summary>Called when the "Number of Clusters" Slider is changed</summary>
        private void OnNumberClustersSliderValueChanged(object sender, EventArgs e)
        {
            int numberClusters = compressionUI.GetSlider( NumberClusters ).Value;
            string str = string.Format("Number of Clusters: {0}", numberClusters );
            compressionUI.GetStaticText( NumberClusters ).SetText( str );           

            UpdateConstText();
        }
        
        /// <summary>Called when the "(1) PRT" Radio Button is changed</summary>
        private void OnTechniquePrtChanged(object sender, EventArgs e)
        {
            float lightScale = (float) (renderingUI2.GetSlider( LightScale ).Value * 0.01f);
            switch( technique )
            { 
                case 0: lightScaleForPrt = lightScale; break; 
                case 1: lightScaleForSHIrrad = lightScale; break; 
                case 2: lightScaleForNDotL = lightScale; break; 
            }
            technique = 0;
            renderingUI2.GetCheckbox( RenderMap ).IsChecked = true;
            IsRenderEnvMap = true;
            renderingUI2.IsUsingNonUserEvents = true;
            renderingUI2.GetSlider( LightScale ).Value = (int)(lightScaleForPrt * 100.0f);
            renderingUI2.IsUsingNonUserEvents = false;
        }

        /// <summary>Called when the "(2) SHIrradEnvMap" Radio Button is changed</summary>
        private void OnTechniqueSHIrradRBChanged(object sender, EventArgs e)
        {
            float lightScale = (float) (renderingUI2.GetSlider( LightScale ).Value * 0.01f);
            switch( technique )
            { 
                case 0: lightScaleForPrt = lightScale; break; 
                case 1: lightScaleForSHIrrad = lightScale; break; 
                case 2: lightScaleForNDotL = lightScale; break; 
            }
            technique = 1;
            renderingUI2.GetCheckbox( RenderMap ).IsChecked = true;
            IsRenderEnvMap = true;
            renderingUI2.IsUsingNonUserEvents = true;
            renderingUI2.GetSlider( LightScale ).Value = (int)(lightScaleForSHIrrad * 100.0f);
            renderingUI2.IsUsingNonUserEvents = false;
        }

        /// <summary>Called when the "(3) N dot L" Radio Button is changed</summary>
        private void OnTechniqueNDotLRBChanged(object sender, EventArgs e)
        {
            float lightScale = (float) (renderingUI2.GetSlider( LightScale ).Value * 0.01f);
            switch( technique )
            { 
                case 0: lightScaleForPrt = lightScale; break; 
                case 1: lightScaleForSHIrrad = lightScale; break; 
                case 2: lightScaleForNDotL = lightScale; break; 
            }
            technique = 2;
            renderingUI2.GetCheckbox( RenderMap ).IsChecked = false;
            IsRenderEnvMap = false;
            renderingUI2.IsUsingNonUserEvents = true;
            renderingUI2.GetSlider( LightScale ).Value = (int)(lightScaleForNDotL * 100.0f);
            renderingUI2.IsUsingNonUserEvents = false;
        }

        /// <summary>Called when the "(F)irst Light Probe" ComboBox is changed</summary>
        private void OnEnvironmentACBChanged(object sender, EventArgs e)
        {
            skyboxA = (int) renderingUI.GetComboBox( EnvironmentA ).GetSelectedData();
        }

        /// <summary>Called when the "(S)econd Light Probe" ComboBox is changed</summary>
        private void OnEnvironmentBCBChanged(object sender, EventArgs e)
        {
            skyboxB = (int) renderingUI.GetComboBox( EnvironmentB ).GetSelectedData();
        }

        /// <summary>Called when the "Apply" button is clicked</summary>
        private void OnApplyButtonClicked(object sender, EventArgs e)
        {
            int numberPcaVectors = compressionUI.GetSlider( NumberPcaVectors ).Value * 4;
            int numberClusters = compressionUI.GetSlider( NumberClusters ).Value;

            SimulatorOptions options = PrtOptions.GlobalOptions;
            options.NumberClusters = numberClusters;
            options.NumberPcaVectors = numberPcaVectors;
            PrtOptions.GlobalOptionsFile.SaveOptions(string.Empty);

            prtMesh.CompressBuffer( options.Quality, options.NumberClusters, options.NumberPcaVectors);
            prtMesh.ExtractCompressedDataForPrtShader();
            prtMesh.LoadEffects( SampleFramework.Device, SampleFramework.DeviceCaps);
        }

        #endregion

        /// <summary>
        /// Entry point to the program. Initializes everything and goes into a message processing 
        /// loop. Idle time is used to render the scene.
        /// </summary>
        static int Main()
        {
            System.Windows.Forms.Application.EnableVisualStyles();
            using(Framework SampleFramework = new Framework())
            {
                PrtPerVertex sample = new PrtPerVertex(SampleFramework);
                // Set the callback functions. These functions allow the sample framework to notify
                // the application about device changes, user input, and windows messages.  The 
                // callbacks are optional so you need only set callbacks for events you're interested 
                // in. However, if you don't handle the device reset/lost callbacks then the sample 
                // framework won't be able to reset your device since the application must first 
                // release all device resources before resetting.  Likewise, if you don't handle the 
                // device created/destroyed callbacks then the sample framework won't be able to 
                // recreate your device resources.
                SampleFramework.Disposing += new EventHandler(sample.OnDestroyDevice);
                SampleFramework.DeviceLost += new EventHandler(sample.OnLostDevice);
                SampleFramework.DeviceCreated += new DeviceEventHandler(sample.OnCreateDevice);
                SampleFramework.DeviceReset += new DeviceEventHandler(sample.OnResetDevice);

                SampleFramework.SetWndProcCallback(new WndProcCallback(sample.OnMsgProc));

                SampleFramework.SetCallbackInterface(sample);
                try
                {
                    // Show the cursor and clip it when in full screen
                    SampleFramework.SetCursorSettings(true, true);

                    // Initialize
                    sample.InitializeApplication();

                    // Initialize the sample framework and create the desired window and Direct3D 
                    // device for the application. Calling each of these functions is optional, but they
                    // allow you to set several options which control the behavior of the SampleFramework.
                    SampleFramework.Initialize( true, true, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
                    //There is no way to do the override unless you parse command line parameters.
                    SampleFramework.CreateWindow("PrtPerVertex");
                    // Hook the keyboard event
                    SampleFramework.Window.KeyDown += new System.Windows.Forms.KeyEventHandler(sample.OnKeyEvent);
                    SampleFramework.CreateDevice( 0, true, Framework.DefaultSizeWidth, Framework.DefaultSizeHeight, sample);
                    SampleFramework.Window.ClientSize = new Size(800,600);

                    // Pass control to the sample framework for handling the message pump and 
                    // dispatching render calls. The sample framework will call your FrameMove 
                    // and FrameRender callback when there is idle time between handling window messages.
                    SampleFramework.MainLoop();

                }
#if(DEBUG)
                catch (Exception e)
                {
                    // In debug mode show this error (maybe - depending on settings)
                    SampleFramework.DisplayErrorMessage(e);
#else
                catch
                {
                // In release mode fail silently
#endif
                    // Ignore any exceptions here, they would have been handled by other areas
                    return (SampleFramework.ExitCode == 0) ? 1 : SampleFramework.ExitCode; // Return an error code here
                }

                // Perform any application-level cleanup here. Direct3D device resources are released within the
                // appropriate callback functions and therefore don't require any cleanup code here.
                return SampleFramework.ExitCode;
            }
        }
    }
}
