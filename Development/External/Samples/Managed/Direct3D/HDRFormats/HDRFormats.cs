//-----------------------------------------------------------------------------
// File: HDRFormats.cs
//
// Desc: This sample shows how to do a float pass motion blur effect using 
//       floating point textures and multiple render targets.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace HdrFormatsSample
{
    /// <summary>HdrFormats Sample Class</summary>
    public class HdrFormats : IFrameworkCallback, IDeviceCreation, IDisposable
    {
        #region Support Types
        /// <summary>
        /// Skybox that surrounds the scene.
        /// </summary>
        private class Skybox
        {
            #region Private Fields
            private float skyboxScale = 1f;   // the scale of the skybox
            private Device device = null;   // current DirectX device
            private VertexBuffer vb = null;   // vertex buffer used to render the skybox
            private Effect effect = null;   // effect applied to the skybox
            private EffectHandle invWorldViewProjHandle = null;   // handle used to set the inverse world-view projection matrix for the effect
            private EffectHandle techniqueHandle = null;   // handle used to set the current technique for the effect
            private EffectHandle environmentMapHandle = null;   // handle used to set the current environment map for the effect
            #endregion
            #region Public Fields
            public CubeTexture environmentMap = null;   // the environment map that will be applied to the skybox
            #endregion
            #region Methods
            #region Creation
            /// <summary>
            /// Public constructor.
            /// </summary>
            /// <param name="scale">The scale that the skybox will be rendered at.</param>
            public Skybox(float scale)
            {
                skyboxScale = scale;
            }
            #endregion
            #region Device Interface Events
            /// <summary>
            /// This event will be fired immediately after the Direct3D device has been 
            /// created, which will happen during application initialization and windowed/full screen 
            /// toggles. This is the best location to create Pool.Managed resources since these 
            /// resources need to be reloaded whenever the device is destroyed. Resources created  
            /// here should be released in the Disposing event. 
            /// </summary>
            public void OnCreateDevice(object sender, DeviceEventArgs e)
            {
                device = e.Device;
                // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
                // shader debugger. Debugging vertex shaders requires either REF or software vertex 
                // processing, and debugging pixel shaders requires REF.  The 
                // ShaderFlags.Force*SoftwareNoOptimizations flag improves the debug experience in the 
                // shader debugger.  It enables source level debugging, prevents instruction 
                // reordering, prevents dead code elimination, and forces the compiler to compile 
                // against the next higher available software target, which ensures that the 
                // unoptimized shaders do not exceed the shader model limitations.  Setting these 
                // flags will cause slower rendering since the shaders will be unoptimized and 
                // forced into software.  See the DirectX documentation for more information about 
                // using the shader debugger.
                ShaderFlags shaderFlags = ShaderFlags.NotCloneable;
#if(DEBUG_VS)
                shaderFlags |= ShaderFlags.ForceVertexShaderSoftwareNoOptimizations;
#endif
#if(DEBUG_PS)
                shaderFlags |= ShaderFlags.ForcePixelShaderSoftwareNoOptimizations;
#endif
                // Read the D3DX effect file
                string path = Utility.FindMediaFile("skybox.fx");
                effect = ResourceCache.GetGlobalInstance().CreateEffectFromFile(e.Device, path, null, null, shaderFlags, null);
                // grab the effect handles that we will use frequently
                invWorldViewProjHandle = effect.GetParameter(null, "g_mInvWorldViewProjection");
                techniqueHandle = effect.GetTechnique("Skybox");
                environmentMapHandle = effect.GetParameter(null, "g_EnvironmentTexture");
                // create the vertex buffer
                vb = new VertexBuffer(typeof(CustomVertex.PositionOnly), 4, device, Usage.None, CustomVertex.PositionOnly.Format, Pool.Managed);
                // create the vertices in the vertex buffer
                float highWidth = -1f - (1f / (float) e.BackBufferDescription.Width);
                float highHeight = -1f - (1f / (float) e.BackBufferDescription.Height);
                float lowWidth = 1f + (1f / (float) e.BackBufferDescription.Width);
                float lowHeight = 1f + (1f / (float) e.BackBufferDescription.Height);
                CustomVertex.PositionOnly[] vertices = vb.Lock(0, typeof(CustomVertex.PositionOnly), LockFlags.None, 4) as CustomVertex.PositionOnly[];
                vertices[0].X = lowWidth;     vertices[0].Y = lowHeight;    vertices[0].Z = 1f;
                vertices[1].X = lowWidth;     vertices[1].Y = highHeight;   vertices[1].Z = 1f;
                vertices[2].X = highWidth;    vertices[2].Y = lowHeight;    vertices[2].Z = 1f;
                vertices[3].X = highWidth;    vertices[3].Y = highHeight;   vertices[3].Z = 1f;
                vb.Unlock();
            }
            /// <summary>
            /// This event will be fired immediately after the Direct3D device has been 
            /// reset, which will happen after a lost device scenario. This is the best location to 
            /// create Pool.Default resources since these resources need to be reloaded whenever 
            /// the device is lost. Resources created here should be released in the OnLostDevice 
            /// event. 
            /// </summary>
            public void OnResetDevice(object sender, DeviceEventArgs e)
            {
            }
            /// <summary>
            /// This event function will be called fired after the Direct3D device has 
            /// entered a lost state and before Device.Reset() is called. Resources created
            /// in the OnResetDevice callback should be released here, which generally includes all 
            /// Pool.Default resources. See the "Lost Devices" section of the documentation for 
            /// information about lost devices.
            /// </summary>
            public void OnLostDevice(object sender, EventArgs e)
            {
            }
            /// <summary>
            /// This callback function will be called immediately after the Direct3D device has 
            /// been destroyed, which generally happens as a result of application termination or 
            /// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
            /// should be released here, which generally includes all Pool.Managed resources. 
            /// </summary>
            public void OnDestroyDevice(object sender, EventArgs e)
            {
                // release the vertex buffer
                if (vb != null)
                {
                    vb.Dispose();
                    vb = null;
                }
                // release the environment map, regardless of who created it
                if (environmentMap != null)
                {
                    environmentMap.Dispose();
                    environmentMap = null;
                }
            }
            #endregion
            #region Rendering
            /// <summary>
            /// Render the skybox into the device.
            /// </summary>
            /// <param name="worldViewProj">The world-view projection matrix to apply to the skybox.</param>
            public void Render(Matrix worldViewProj)
            {
                // create the inverse world-view-projection matrix
                Matrix invWorldViewProj = Matrix.Invert(worldViewProj);
                // set up the effect
                effect.SetValue(invWorldViewProjHandle, invWorldViewProj);
                effect.SetValue(environmentMapHandle, environmentMap);
                effect.Technique = techniqueHandle;
                // set up the device
                device.VertexFormat = CustomVertex.PositionOnly.Format;
                device.SetStreamSource(0, vb, 0);
                // render the skybox
                int passes = effect.Begin(0);
                for (int i = 0; i < passes; i++)
                {
                    effect.BeginPass(i);
                    device.DrawPrimitives(PrimitiveType.TriangleStrip, 0, 2);
                    effect.EndPass();
                }
                effect.End();
            }
            #endregion
            #endregion
        }
        #endregion

        #region Enumerations
        private enum HdrRenderingMode 
        { 
            Decoded, 
            RGBEncoded, 
            AlphaEncoded,
            Count
        };   // the rendering modes that the app could support
        /// <summary>
        /// Converts a rendering mode to a string.
        /// </summary>
        /// <param name="mode">The rendering mode to convert.</param>
        /// <param name="full">If true, then the full description of the mode is used, otherwise a short description.</param>
        /// <returns>A string corresponding to the mode parameter.</returns>
        private static string HdrRenderingModeToString(HdrRenderingMode mode, bool full)
        {
            switch (mode)
            {
                case HdrRenderingMode.Decoded:
                    return full ? "Decoded scene" : "Decoded scene";
                case HdrRenderingMode.RGBEncoded:
                    return full ? "RGB channels of encoded textures" : "RGB channels";
                case HdrRenderingMode.AlphaEncoded:
                    return full ? "Alpha channel of encoded textures" : "Alpha channel";
            }
            return "BadMode";
        }
        private enum HdrEncodingMode 
        { 
            FP16, 
            FP32, 
            RGB16, 
            RGBE8,
            Count
        };   // the texture encoding modes that the app could support
        /// <summary>
        /// Specifies if the texture encoding mode uses floating-point colors.
        /// </summary>
        /// <param name="mode">The texture encoding mode to check.</param>
        /// <returns>True if the encoding mode uses floating-point colors; false otherwise.</returns>
        private static bool HdrEncodingModeIsFloatingPoint(HdrEncodingMode mode)
        {
            switch (mode)
            {
                case HdrEncodingMode.FP16:
                case HdrEncodingMode.FP32:
                    return true;
            }
            return false; 
        }
        /// <summary>
        /// Converts a texture encoding float to a DirectX texture format.
        /// </summary>
        /// <param name="mode">The texture encoding mode to convert.</param>
        /// <returns>A DirectX texture format corresponding to the mode parameter.</returns>
        private static Format HdrEncodingModeToFormat(HdrEncodingMode mode)
        {
            switch (mode)
            {
                case HdrEncodingMode.FP16:
                    return Format.A16B16G16R16F;
                case HdrEncodingMode.FP32:
                    return Format.A32B32G32R32F;
                case HdrEncodingMode.RGB16:
                    return Format.A16B16G16R16;
                case HdrEncodingMode.RGBE8:
                    return Format.A8R8G8B8;
            }
            return Format.Unknown;
        }
        /// <summary>
        /// Converts a texture encoding float to a string.
        /// </summary>
        /// <remarks>The short description (full=false) is used to find techniques in the effect file.</remarks>
        /// <param name="mode">The texture encoding mode to convert.</param>
        /// <param name="full">If true, then the full description of the mode is used, otherwise a short description.</param>
        /// <returns>A string corresponding to the mode parameter.</returns>
        private static string HdrEncodingModeToString(HdrEncodingMode mode, bool full)
        {
            switch (mode)
            {
                case HdrEncodingMode.FP16:
                    return full ? "16-Bit floating-point (FP16)" : "FP16";
                case HdrEncodingMode.FP32:
                    return full ? "32-Bit floating-point (FP32)" : "FP32";
                case HdrEncodingMode.RGB16:
                    return full ? "16-Bit integer (RGB16)" : "RGB16";
                case HdrEncodingMode.RGBE8:
                    return full ? "8-bit integer w/ shared exponent (RGBE8)" : "RGBE8";
            }
            return "BadMode";
        }
        private enum HdrTechnique 
        { 
            Scene, 
            DownScale2x2Lum, 
            DownScale3x3, 
            DownScale3x3BrightPass, 
            FinalPass,
            Count
        };   // the effect-file techniques that the app could support
        /// <summary>
        /// Converts a effect-file technique mode to a string.
        /// </summary>
        /// <param name="mode">The effect-file technique to convert.</param>
        /// <returns>A string corresponding to the technique parameter.</returns>
        private static string HdrTechniqueToString(HdrTechnique technique)
        {
            switch (technique)
            {
                case HdrTechnique.Scene:
                    return "Scene";
                case HdrTechnique.DownScale2x2Lum:
                    return "DownScale2x2_Lum";
                case HdrTechnique.DownScale3x3:
                    return "DownScale3x3";
                case HdrTechnique.DownScale3x3BrightPass:
                    return "DownScale3x3_BrightPass";
                case HdrTechnique.FinalPass:
                    return "FinalPass";
            }
            return "BadTechnique";
        }
        #endregion
        #region Fields
        #region Effect Control constants
        private const int NumTonemapTextures = 5;   // number of stages in the 3x3 down-scaling of average luminance textures
        private const int NumBloomTextures = 2;   // number of bloom textures
        private const float RGB16Maximum = 100f;   // maximum value to scale to for the RGB16 texture encoding mode
        #endregion
        #region HUD UI Control constants
        private const int ToggleFullscreen = 1;   // value for the toggle-fullscreen button in the sample framework UI
        private const int ToggleReference = 3;   // value for the toggle-reference-mode button in the sample framework UI
        private const int ChangeDevice = 4;   // value for the change-device button in the sample framework UI
        private const int EncodingModeCombo = 5;   // value for the texture-encoding-mode combo-box in the sample framework UI
        private const int RenderingModeCombo = 6;   // value for the rendering-mode combo-box in the sample framework UI
        private const int BloomCheck = 7;   // value for the bloom checkbox in the sample framework UI
        #endregion
        #region Variables
        private Framework sampleFramework = null;   // sample framework used by this object
        private Device device = null;   // D3D device
        private Effect effect = null;   // D3DX effect interface
        private EffectHandle[,] techHandles = new EffectHandle[(int) HdrEncodingMode.Count, (int) HdrTechnique.Count];   // Technique handles from HdrFormats.fx
        private EffectHandle techFinalPassRGBHandle = null;   // handle used to set the final-pass RGB technique for the effect
        private EffectHandle techFinalPassAlphaHandle = null;   // handle used to set the final-pass alpha technique for the effect
        private EffectHandle eyePointHandle = null;   // handle used to set the eye point data for the effect
        private EffectHandle worldHandle = null;   // handle used to set the world matrix for the effect
        private EffectHandle worldViewProjHandle = null;   // handle used to set the world-view projection matrix for the effect
        private EffectHandle cubemapTexHandle = null;   // handle used to set the cube-map texture for the effect
        private EffectHandle sampleOffsetsHandle = null;   // handle used to set the sample-offsets array data for the effect
        private EffectHandle sampleWeightsHandle = null;   // handle used to set the sample-weights array data for the effect
        private Texture renderTex = null;   // texture that acts as a render target
        private Texture brightpassTex = null;   // texture used as a bright-pass filter
        private Texture[] arrTonemapTex = new Texture[NumTonemapTextures];   // tone-mapping calculation textures
        private Texture[] arrBloomTex = new Texture[NumBloomTextures];   // blooming effect intermediate textures
        private Skybox[] arrSkybox = new Skybox[(int) HdrEncodingMode.Count];   // skyboxes, one for each texture encoding mode
        private FrameworkMesh mainMesh = null;   // primary mesh for the scene
        private VertexBuffer quadvb = null;   // vertex buffer used to render the fullscreen quads
        private ModelViewerCamera camera = new ModelViewerCamera();   // the camera used to view the model
        private HdrRenderingMode currentRenderingMode = HdrRenderingMode.Decoded;   // the current rendering mode
        private HdrEncodingMode currentEncodingMode = HdrEncodingMode.RGBE8;   // the current texture encoding mode
        private bool doBloom = true;   // bloom effect flag
        private bool supportsR16F;   // RGB16 texture mode support flag
        private bool supportsR32F;   // RGB32 texture mode support flag
        private bool supports128FCube;   // 128F texture mode support flag 
        private bool supports128Cube;   // 128 texture mode support flag
        private static double[] powersOfTwo = new double[257];   // static array containing the powers of two, from 2 ^ 0 to 2 ^ 256
        private static readonly VertexElement[] meshDecl = new VertexElement[] 
            {
                new VertexElement(0, 0,  DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Position, 0), 
                new VertexElement(0, 12, DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Normal, 0), 
                new VertexElement(0, 24, DeclarationType.Float2, DeclarationMethod.Default, DeclarationUsage.TextureCoordinate, 0), 
                VertexElement.VertexDeclarationEnd 
            };
        #endregion
        #region UI variables
        private Font statsFont = null;   // Font for drawing text
        private Sprite textSprite = null;   // Sprite for batching text calls
        private Dialog hud = null;   // dialog for standard controls
        private Dialog sampleUi = null;   // dialog for sample specific controls
        private ComboBox encodingCombo = null;   // combo box for the encoding mode, for easy access
        private Checkbox bloomCheck = null;   // check box for the bloom, for easy access
        private bool showHelpText = false;   // If true, renders the UI help text
        private bool showFramerate = true;   // if true, renders the framerate text
        #endregion
        #endregion
        #region Methods
        #region Creation
        /// <summary>
        /// Create a new instance of the class
        /// </summary>
        /// <param name="f">The sample framework that the HdrFormats object will use.</param>
        public HdrFormats(Framework f) 
        { 
            // Store framework
            sampleFramework = f; 
            // Create dialogs
            hud = new Dialog(sampleFramework); 
            sampleUi = new Dialog(sampleFramework); 
            // fill power-of-two array
            for (int i = 0; i <= 256; i++)
            {
                powersOfTwo[i] = Math.Pow(2.0, (double) (i - 128));
            }
            // Set the callback functions. These functions allow the sample framework to notify
            // the application about device changes, user input, and windows messages.  The 
            // callbacks are optional so you need only set callbacks for events you're interested 
            // in. However, if you don't handle the device reset/lost callbacks then the sample 
            // framework won't be able to reset your device since the application must first 
            // release all device resources before resetting.  Likewise, if you don't handle the 
            // device created/destroyed callbacks then the sample framework won't be able to 
            // recreate your device resources.
            sampleFramework.Disposing += new EventHandler(OnDestroyDevice);
            sampleFramework.DeviceLost += new EventHandler(OnLostDevice);
            sampleFramework.DeviceCreated += new DeviceEventHandler(OnCreateDevice);
            sampleFramework.DeviceReset += new DeviceEventHandler(OnResetDevice);
            sampleFramework.SetWndProcCallback(new WndProcCallback(OnMsgProc));
            sampleFramework.SetCallbackInterface(this);
        }
        #endregion
        #region Device Management
        /// <summary>
        /// Called during device initialization, this code checks the device for some 
        /// minimum set of capabilities, and rejects those that don't pass by returning false.
        /// </summary>
        /// <param name="caps">Device capability structure.</param>
        /// <param name="adapterFormat">Surface format used by the device.</param>
        /// <param name="backBufferFormat">Surface format used by the back-buffer</param>
        /// <param name="windowed">If true, the device is running in windowed mode.</param>
        /// <returns>True if the device as configured is acceptable; false otherwise.</returns>
        public bool IsDeviceAcceptable(Caps caps, Format adapterFormat, Format backBufferFormat, bool windowed)
        {
            // must support 2.0 pixel shaders or better
            if (caps.PixelShaderVersion.Major < 2)
                return false;
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
        /// <param name="settings">The settings used to create a D3D device.</param>
        /// <param name="caps">Device capability structure.</param>
        public void ModifyDeviceSettings(DeviceSettings settings, Caps caps)
        {
            // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW, then switch to SWVP.
            if ((!caps.DeviceCaps.SupportsHardwareTransformAndLight) ||
                (caps.VertexShaderVersion < new Version(1,1)))
            {
                settings.BehaviorFlags = CreateFlags.SoftwareVertexProcessing;
            }
            else
            {
                settings.BehaviorFlags = CreateFlags.HardwareVertexProcessing;
            }
            // This application is designed to work on a pure device by not using 
            // any get methods, so create a pure device if supported and using HWVP.
            if ((caps.DeviceCaps.SupportsPureDevice) && 
                ((settings.BehaviorFlags & CreateFlags.HardwareVertexProcessing) != 0))
            {
                settings.BehaviorFlags |= CreateFlags.PureDevice;
            }
            // Debugging vertex shaders requires either REF or software vertex processing 
            // and debugging pixel shaders requires REF.  
#if(DEBUG_VS)
            if (settings.DeviceType != DeviceType.Reference)
            {
                settings.BehaviorFlags &= ~CreateFlags.HardwareVertexProcessing;
                settings.BehaviorFlags |= CreateFlags.SoftwareVertexProcessing;
            }
#endif
#if(DEBUG_PS)
            settings.DeviceType = DeviceType.Reference;
#endif
            // fill device state flags
            supportsR16F = Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, settings.AdapterFormat, Usage.RenderTarget,
                ResourceType.Textures, Format.R16F);
            supportsR32F = Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, settings.AdapterFormat, Usage.RenderTarget,
                ResourceType.Textures, Format.R32F);
            supports128FCube = Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, settings.AdapterFormat, Usage.RenderTarget,
                ResourceType.CubeTexture, Format.A16B16G16R16F);
            supports128Cube = Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, settings.AdapterFormat, Usage.None,
                ResourceType.CubeTexture, Format.A16B16G16R16);

            // For the first device created if its a REF device, optionally display a warning dialog box
            if (settings.DeviceType == DeviceType.Reference)
            {
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "HDRFormats");
            }
        }
        #endregion
        #region Device Interface Events
        /// <summary>
        /// This event will be fired immediately after the Direct3D device has been 
        /// created, which will happen during application initialization and windowed/full screen 
        /// toggles. This is the best location to create Pool.Managed resources since these 
        /// resources need to be reloaded whenever the device is destroyed. Resources created  
        /// here should be released in the Disposing event. 
        /// </summary>
        private void OnCreateDevice(object sender, DeviceEventArgs e)
        {
            device = e.Device;
            // Initialize the font
            statsFont = ResourceCache.GetGlobalInstance().CreateFont(e.Device, 15, 0, FontWeight.Bold, 1, false, CharacterSet.Default,
                Precision.Default, FontQuality.Default, PitchAndFamily.FamilyDoNotCare | PitchAndFamily.DefaultPitch
                , "Arial");
            // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
            // shader debugger. Debugging vertex shaders requires either REF or software vertex 
            // processing, and debugging pixel shaders requires REF.  The 
            // ShaderFlags.Force*SoftwareNoOptimizations flag improves the debug experience in the 
            // shader debugger.  It enables source level debugging, prevents instruction 
            // reordering, prevents dead code elimination, and forces the compiler to compile 
            // against the next higher available software target, which ensures that the 
            // unoptimized shaders do not exceed the shader model limitations.  Setting these 
            // flags will cause slower rendering since the shaders will be unoptimized and 
            // forced into software.  See the DirectX documentation for more information about 
            // using the shader debugger.
            ShaderFlags shaderFlags = ShaderFlags.NotCloneable;
#if(DEBUG_VS)
            shaderFlags |= ShaderFlags.ForceVertexShaderSoftwareNoOptimizations;
#endif
#if(DEBUG_PS)
            shaderFlags |= ShaderFlags.ForcePixelShaderSoftwareNoOptimizations;
#endif
            // Read the D3DX effect file
            string path = Utility.FindMediaFile("HDRFormats.fx");
            effect = ResourceCache.GetGlobalInstance().CreateEffectFromFile(e.Device,
                path, null, null, shaderFlags, null);
            // retrieve techniques (some may not be present)
            for (int i = 0; i < (int) HdrEncodingMode.Count; i++)
            {
                for (int j = 0; j < (int) HdrTechnique.Count; j++)
                {
                    // there are no entries for FP32 in the FX file
                    if ( i == (int) HdrEncodingMode.FP32 )
                        techHandles[i, j] = effect.GetTechnique(HdrTechniqueToString((HdrTechnique) j) + "_FP16");
                    else
                        techHandles[i, j] = effect.GetTechnique(HdrTechniqueToString((HdrTechnique) j) + "_" + HdrEncodingModeToString((HdrEncodingMode) i, false));
                }
            }
            // retrieve the effect handles that will be used frequently
            techFinalPassRGBHandle = effect.GetTechnique("FinalPassEncoded_RGB");
            techFinalPassAlphaHandle = effect.GetTechnique("FinalPassEncoded_A");
            eyePointHandle = effect.GetParameter(null, "g_vEyePt");
            worldHandle = effect.GetParameter(null, "g_mWorld");
            worldViewProjHandle = effect.GetParameter(null, "g_mWorldViewProj");
            cubemapTexHandle = effect.GetParameter(null, "g_tCube");
            sampleOffsetsHandle = effect.GetParameter(null, "g_avSampleOffsets");
            sampleWeightsHandle = effect.GetParameter(null, "g_avSampleWeights");
            // clear the encoding mode combo box - only currently supported modes will be entered
            encodingCombo.Clear();
            // load skybox and environment textures
            CubeTexture cubeTexture32 = null, encodedTexture = null;
            bool bScratchTexture = false;
            path = Utility.FindMediaFile("Light Probes\\uffizi_cross.dds");
            // FP16
            if (supportsR16F && supports128FCube)
            {
                CubeTexture cubeTexture16 = ResourceCache.GetGlobalInstance().CreateCubeTextureFromFileEx(device, path, 0, 1, Usage.None, HdrEncodingModeToFormat(HdrEncodingMode.FP16), 
                    Pool.Managed, Filter.None, Filter.None, 0);
                arrSkybox[(int) HdrEncodingMode.FP16] = new Skybox(50f);
                arrSkybox[(int) HdrEncodingMode.FP16].OnCreateDevice(sender, e);
                arrSkybox[(int) HdrEncodingMode.FP16].environmentMap = cubeTexture16;
                encodingCombo.AddItem(HdrEncodingModeToString(HdrEncodingMode.FP16, false), HdrEncodingMode.FP16);
            }
            else
                arrSkybox[(int) HdrEncodingMode.FP16] = null;
            // FP32
            if (supportsR32F && supports128FCube)
            {
                cubeTexture32 = ResourceCache.GetGlobalInstance().CreateCubeTextureFromFileEx(device, path, 0, 1, Usage.None, HdrEncodingModeToFormat(HdrEncodingMode.FP32), 
                    Pool.Managed, Filter.None, Filter.None, 0);
                arrSkybox[(int) HdrEncodingMode.FP32] = new Skybox(50f);
                arrSkybox[(int) HdrEncodingMode.FP32].OnCreateDevice(sender, e);
                arrSkybox[(int) HdrEncodingMode.FP32].environmentMap = cubeTexture32;
                encodingCombo.AddItem(HdrEncodingModeToString(HdrEncodingMode.FP32, false), HdrEncodingMode.FP32);
            }
            else
                arrSkybox[(int) HdrEncodingMode.FP32] = null;
            // create a scratch texture to use if the device doesn't support half-size floating point
            if (cubeTexture32 == null)
            {
                cubeTexture32 = ResourceCache.GetGlobalInstance().CreateCubeTextureFromFileEx(device, path, 0, 1, Usage.None, HdrEncodingModeToFormat(HdrEncodingMode.FP32), 
                    Pool.Scratch, Filter.None, Filter.None, 0);
                bScratchTexture = true;
            }
            // RGB16
            if (supports128Cube)
            {
                encodedTexture = CreateEncodedTexture(cubeTexture32, HdrEncodingMode.RGB16);
                arrSkybox[(int) HdrEncodingMode.RGB16] = new Skybox(50f);
                arrSkybox[(int) HdrEncodingMode.RGB16].OnCreateDevice(sender, e);
                arrSkybox[(int) HdrEncodingMode.RGB16].environmentMap = encodedTexture;
                encodingCombo.AddItem(HdrEncodingModeToString(HdrEncodingMode.RGB16, false), HdrEncodingMode.RGB16);
            }
            else
                arrSkybox[(int) HdrEncodingMode.RGB16] = null;
            // RGBE8 - always support
            encodedTexture = CreateEncodedTexture(cubeTexture32, HdrEncodingMode.RGBE8);
            arrSkybox[(int) HdrEncodingMode.RGBE8] = new Skybox(50f);
            arrSkybox[(int) HdrEncodingMode.RGBE8].OnCreateDevice(sender, e);
            arrSkybox[(int) HdrEncodingMode.RGBE8].environmentMap = encodedTexture;
            encodingCombo.AddItem(HdrEncodingModeToString(HdrEncodingMode.RGBE8, false), HdrEncodingMode.RGBE8);
            encodingCombo.SetSelectedByData(HdrEncodingMode.RGBE8);
            // get rid of the scratch texture, if we created one
            if (bScratchTexture)
                cubeTexture32.Dispose();
            // load the main mesh
            path = Utility.FindMediaFile("Misc\\teapot.x");
            mainMesh = new FrameworkMesh(device, path);
            mainMesh.IsUsingMaterials = false;
            mainMesh.SetVertexDeclaration(device, meshDecl);
            // create the quad vertex buffer
            quadvb = new VertexBuffer(typeof(CustomVertex.TransformedTextured), 4, device, Usage.None, CustomVertex.TransformedTextured.Format, Pool.Managed);
            // Setup the camera's view parameters
            camera.SetViewParameters(new Vector3(0.0f, 0.0f, -5.0f), Vector3.Empty);
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
            SurfaceDescription desc = e.BackBufferDescription;
            // Create a sprite to help batch calls when drawing many lines of text
            textSprite = new Sprite(e.Device);
            // call OnResetDevice on every skybox
            for (int i = 0; i < (int) HdrEncodingMode.Count; i++)
            {
                if (arrSkybox[i] != null)
                {
                    arrSkybox[i].OnResetDevice(sender, e);
                }
            }
			// create render and bright pass textures
			Format fmt = Format.Unknown;
			switch(currentEncodingMode)
			{
				case HdrEncodingMode.FP16:
				case HdrEncodingMode.FP32:
					fmt = Format.A16B16G16R16F;
					break;
				case HdrEncodingMode.RGBE8:
					fmt = Format.A8R8G8B8;
					break;
				case HdrEncodingMode.RGB16:
					fmt = Format.A16B16G16R16;
					break;
			}
			renderTex = new Texture(device, desc.Width, desc.Height, 1, Usage.RenderTarget, fmt, 
                Pool.Default);
			brightpassTex = new Texture(device, desc.Width / 8, desc.Height / 8, 1, Usage.RenderTarget, 
                Format.A8R8G8B8, Pool.Default);
			// create the tone maps
			int texdim = 1;
			fmt = Format.Unknown;
			switch(currentEncodingMode)
			{
				case HdrEncodingMode.FP16:
					fmt = Format.R16F;
					break;
				case HdrEncodingMode.FP32:
					fmt = Format.R32F;
					break;
				case HdrEncodingMode.RGBE8:
					fmt = Format.A8R8G8B8;
					break;
				case HdrEncodingMode.RGB16:
					fmt = Format.A16B16G16R16;
					break;
			}
			// for each scale stage, create a texture to hold the intermediate results
			// of the luminance calculation
			for (int i = 0; i < NumTonemapTextures; i++)
			{
				arrTonemapTex[i] = new Texture(device, texdim, texdim, 1, Usage.RenderTarget, 
					fmt, Pool.Default);
				texdim *= 3;
			}
			// create the temporary blooming effect textures
			for (int i = 0; i < NumBloomTextures; i++)
			{
				arrBloomTex[i] = new Texture(device, desc.Width / 8, desc.Height / 8, 1, Usage.RenderTarget,
                    Format.A8R8G8B8, Pool.Default);
			}
			// set up UI locations
			hud.SetLocation(desc.Width - 170, 0);
			hud.SetSize(170, 170);
			sampleUi.SetLocation(desc.Width - 150, desc.Height - 150);
			sampleUi.SetSize(150, 110);
            // set up the camera's projection parameters
            float aspectRatio = (float) desc.Width / (float) desc.Height;
            camera.SetProjectionParameters((float) Math.PI / 4, aspectRatio, 0.1f, 5000.0f);
            camera.SetWindow(desc.Width, desc.Height);
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
            // release the text sprite
            if (textSprite != null)
            {
                textSprite.Dispose();
                textSprite = null;
            }
            // call OnLostDevice on every skybox
            for (int i = 0; i < (int) HdrEncodingMode.Count; i++)
            {
                if (arrSkybox[i] != null)
                {
                    arrSkybox[i].OnLostDevice(sender, e);
                }
            }
			// release the render textures
            if (renderTex != null)
            {
                renderTex.Dispose();
                renderTex = null;
            }
            if (brightpassTex != null)
            {
                brightpassTex.Dispose();
                brightpassTex = null;
            }
            // release the tonemap textures
            for (int i = 0; i < NumTonemapTextures; i++)
            {
                if (arrTonemapTex[i] != null)
                {
                    arrTonemapTex[i].Dispose();
                    arrTonemapTex[i] = null;
                }
            }
            // release the temporary blooming effect textures
            for (int i = 0; i < NumBloomTextures; i++)
            {
                if (arrBloomTex[i] != null)
                {
                    arrBloomTex[i].Dispose();
                    arrBloomTex[i] = null;
                }
            }
        }
        /// <summary>
        /// This callback function will be called immediately after the Direct3D device has 
        /// been destroyed, which generally happens as a result of application termination or 
        /// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
        /// should be released here, which generally includes all Pool.Managed resources. 
        /// </summary>
        private void OnDestroyDevice(object sender, EventArgs e)
        {
            // call OnDestroyDevice on every skybox and get rid of them
            for (int i = 0; i < (int) HdrEncodingMode.Count; i++)
            {
                if (arrSkybox[i] != null)
                {
                    arrSkybox[i].OnDestroyDevice(sender, e);
                    arrSkybox[i] = null;
                }
            }
            // clean up the mesh
            if (mainMesh != null)
            {
                mainMesh.Dispose();  
                mainMesh = null;     
            }
            // clean up the screen-quad vertex buffer
            if (quadvb != null)
            {
                quadvb.Dispose();
                quadvb = null;
            }
            // clean up effect handles
            for (int i = 0; i < (int) HdrEncodingMode.Count; i++)
            {
                for (int j = 0; j < (int) HdrTechnique.Count; j++)
                {
                    techHandles[i, j] = null;
                }
            }
        }
        #endregion
        #region Frame Events
        /// <summary>
        /// This callback function will be called once at the beginning of every frame. This is the
        /// best location for your application to handle updates to the scene, but is not 
        /// intended to contain actual rendering calls, which should instead be placed in the 
        /// OnFrameRender callback.  
        /// </summary>
        /// <param name="device">The current D3D Device</param>
        /// <param name="appTime">The application time.</param>
        /// <param name="elapsedTime">The time elapsed inside the application.</param>
        public void OnFrameMove(Device device, double appTime, float elapsedTime)
        {
            // Update the camera's position based on user input 
            camera.FrameMove(elapsedTime);
            unsafe
            {
                Vector3 eyeloc = camera.EyeLocation;
                effect.SetValue(eyePointHandle, (void *) &eyeloc, sizeof(Vector3));
            }
        }
        /// <summary>
        /// This callback function will be called at the end of every frame to perform all the 
        /// rendering calls for the scene, and it will also be called if the window needs to be 
        /// repainted. After this function has returned, the sample framework will call 
        /// Device.Present to display the contents of the next buffer in the swap chain
        /// </summary>
        /// <param name="device">The current D3D Device</param>
        /// <param name="appTime">The application time.</param>
        /// <param name="elapsedTime">The time elapsed inside the application.</param>
        public void OnFrameRender(Device device, double appTime, float elapsedTime)
        {
            bool beginSceneCalled = false;
            // save the backbuffer surface
            using(Surface surfBackBuffer = device.GetRenderTarget(0))
            {
                // set the render target to the texture from this object
                using(Surface surfRenderTarget = renderTex.GetSurfaceLevel(0))
                {
                    device.SetRenderTarget(0, surfRenderTarget);
                    // Clear the render target and the zbuffer 
                    device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, 0x000000FF, 1f, 0);
                    // draw the scene
                    try
                    {
                        device.BeginScene();
                        beginSceneCalled = true;
                        // calculate the world-view projection matrix
                        Matrix worldViewProjMatrix = Matrix.Multiply(camera.ViewMatrix, camera.ProjectionMatrix);
                        // render the skybox
                        arrSkybox[(int) currentEncodingMode].Render(worldViewProjMatrix);
                        // call the rendering functions
                        RenderModel();
                        MeasureLuminance();
                        BrightPassFilter();
                        if (doBloom)
                            RenderBloom();
                        // set the device states for the final pass
                        device.SetRenderTarget(0, surfBackBuffer);
                        device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, 0x000000FF, 1f, 0);
                        device.SetTexture(0, renderTex);
                        device.SetTexture(1, arrTonemapTex[0]);
                        device.SetTexture(2, doBloom ? arrBloomTex[0] : null);
                        device.SamplerState[2].MinFilter = TextureFilter.Linear;
                        device.SamplerState[2].MagFilter = TextureFilter.Linear;
                        // set the effect technique for the final pass
                        switch (currentRenderingMode)
                        {
                            case HdrRenderingMode.Decoded:
                                effect.Technique = techHandles[(int) currentEncodingMode, (int) HdrTechnique.FinalPass];
                                break;
                            case HdrRenderingMode.RGBEncoded:
                                effect.Technique = techFinalPassRGBHandle;
                                break;
                            case HdrRenderingMode.AlphaEncoded:
                                effect.Technique = techFinalPassAlphaHandle;
                                break;
                        }
                        // render the final pass
                        int inumPasses = effect.Begin(FX.None);
                        for (int ipass = 0; ipass < inumPasses; ipass++)
                        {
                            effect.BeginPass(ipass);
                            DrawFullScreenQuad(0f, 0f, 1f, 1f);
                            effect.EndPass();
                        }
                        effect.End();
                        // clear the device states
                        device.SetTexture(0, null);
                        device.SetTexture(1, null);
                        device.SetTexture(2, null);
                        // Show frame rate
                        if (showFramerate == true)
                            RenderText();
                        // Show UI
                        hud.OnRender(elapsedTime);
                        sampleUi.OnRender(elapsedTime);
                    }
                    finally
                    {
                        if (beginSceneCalled)
                            device.EndScene();
                    }
                }        
            }
        }
        #endregion
        #region UI Events
        /// <summary>
        /// As a convenience, the sample framework inspects the incoming windows messages for
        /// keystroke messages and decodes the message parameters to pass relevant keyboard
        /// messages to the application.  The framework does not remove the underlying keystroke 
        /// messages, which are still passed to the application's MsgProc callback.
        /// </summary>
        /// <param name="key">The key that was pressed.</param>
        /// <param name="isKeyDown">If true, the key is currently down.</param>
        /// <param name="isKeyUp">If true, the key is currently up.</param>
        private void OnKeyEvent(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            // only handle key-down events
            switch(e.KeyCode)
            {
                case System.Windows.Forms.Keys.F1:  // toggle the help text
                    showHelpText = !showHelpText;
                    break;
                case System.Windows.Forms.Keys.T:   // toggle the framerate text
                    showFramerate = !showFramerate;
                    break;
            }
        }
        /// <summary>
        /// Before handling window messages, the sample framework passes incoming windows 
        /// messages to the application through this callback function. If the application sets 
        /// noFurtherProcessing to true, the sample framework will not process the message
        /// </summary>
        /// <param name="noFurtherProcessing">Receives the no-further-processing flag.</param>
        public IntPtr OnMsgProc(IntPtr hWnd, NativeMethods.WindowMessage msg, IntPtr wParam, IntPtr lParam, ref bool noFurtherProcessing)
        {
            // give the HUD a chance to handle the message first
            noFurtherProcessing = hud.MessageProc(hWnd, msg, wParam, lParam);
            if (noFurtherProcessing)
                return IntPtr.Zero;
            // give the sample UI a chance to handle the message
            noFurtherProcessing = sampleUi.MessageProc(hWnd, msg, wParam, lParam);
            if (noFurtherProcessing)
                return IntPtr.Zero;
            // Pass all remaining windows messages to camera so it can respond to user input
            camera.HandleMessages(hWnd, msg, wParam, lParam);
            return IntPtr.Zero;
        }
        /// <summary>
        /// Called when the change-device button is clicked.
        /// </summary>
        private void OnChangeDeviceClicked(object sender, EventArgs e)
        {
            sampleFramework.ShowSettingsDialog(!sampleFramework.IsD3DSettingsDialogShowing);
        }
        /// <summary>
        /// Called when the full-screen button is clicked.
        /// </summary>
        private void OnFullscreenClicked(object sender, EventArgs e)
        {
            sampleFramework.ToggleFullscreen();
        }
        /// <summary>
        /// Called when the ref button is clicked.
        /// </summary>
        private void OnRefClicked(object sender, EventArgs e)
        {
            sampleFramework.ToggleReference();
        }
        /// <summary>
        /// Called when the rendering mode combo box is changed.
        /// </summary>
        private void OnRenderingModeChanged(object sender, EventArgs e)
        {
            ComboBox cbx = (ComboBox) sender;
            currentRenderingMode = (HdrRenderingMode) cbx.GetSelectedData();
            // only enable the bloom checkbox if the current rendering mode is Decoded
            bloomCheck.IsEnabled = (currentRenderingMode == HdrRenderingMode.Decoded);
            bloomCheck.IsChecked = (bloomCheck.IsEnabled && doBloom);
        }
        /// <summary>
        /// Called when the encoding mode combo box is changed.
        /// </summary>
        private void OnEncodingModeChanged(object sender, EventArgs e)
        {
            ComboBox cbx = (ComboBox) sender;
            currentEncodingMode = (HdrEncodingMode) cbx.GetSelectedData();
            // reset the device
            OnLostDevice(sender, e);
            OnResetDevice(sender, new DeviceEventArgs(device, sampleFramework.BackBufferSurfaceDescription));
        }
        /// <summary>
        /// Called when the bloom-enable check box is changed.
        /// </summary>
        private void OnBloomCheckChanged(object sender, EventArgs e)
        {
            // don't change the bloom flag if the checkbox isn't enabled
            if (bloomCheck.IsEnabled == false)
                return;
            // set the bloom flag to the state of the checkbox
            doBloom = bloomCheck.IsChecked;
        }
        #endregion
        #region Encoded Texture Methods
        /// <summary>
        /// Creates a copy of the provided cube texture in the encoding mode requested.
        /// </summary>
        /// <remarks>Because this function operates on GraphicsStream.InternalDataPointer, it has been marked unsafe.</remarks>
        /// <param name="originalCubeTexture">The original cube texture.</param>
        /// <param name="mode">The texture encoding mode to use for the new cube texture.</param>
        /// <returns>A cube texture with the data from originalCubeTexture in the specified texture encoding mode.</returns>
        private unsafe CubeTexture CreateEncodedTexture(CubeTexture originalCubeTexture, HdrEncodingMode mode)
        {
            // don't bother if the parameters are invalid
            if (device == null)
                throw(new Direct3DXException("The device was not valid, so CreateEncodedTexture() could not run."));
            if (originalCubeTexture == null)
                throw(new ArgumentNullException("originalCubeTexture", "The original cube texture passed to CreateEncodedTexture() was not valid."));
            if (HdrEncodingModeIsFloatingPoint(mode))
                throw(new ArgumentNullException("mode", "The texture encoding mode passed to CreateEncodedTexture() was not valid."));
            // determine the surface format to use
            Format fmt = HdrEncodingModeToFormat(mode);
            // create a texture with the same dimensions as the original to store the encoded texture
            SurfaceDescription desc = originalCubeTexture.GetLevelDescription(0);
            CubeTexture cubeTexture = new CubeTexture(device, desc.Width, 1, Usage.None, fmt, Pool.Managed);
            // copy each face
            for (int face = 0; face < 6; face++)
            {
                // get the source pixel data for this face in the original texture
                ColorValue[,] sourceStream = originalCubeTexture.LockRectangle(typeof(ColorValue), 
                    (CubeMapFace) face, 0, LockFlags.ReadOnly, desc.Width, desc.Height) as ColorValue[,];
                // get the destination pixel data for this face in the new texture
                GraphicsStream destStream = cubeTexture.LockRectangle((CubeMapFace) face, 0, LockFlags.None);
                void *dest = destStream.InternalDataPointer;
                // encode each pixel based on the mode
                for (int y = 0; y < desc.Width; y++)
                {
                    for (int x = 0; x < desc.Height; x++)
                    {
                        EncodePixel(mode, sourceStream[y,x], ref dest);
                    }
                }
                // unlock the pixel data
                cubeTexture.UnlockRectangle((CubeMapFace) face, 0);
                originalCubeTexture.UnlockRectangle((CubeMapFace) face, 0);
            }
            return cubeTexture;
        }
        /// <summary>Encode the given FloatingPointColor pixel into a pixel of the requested mode.</summary>
        /// <remarks>The dataOutput parameter pointer is incremented, by this function, according to the size of the pixel that was stored in it.</remarks>
        /// <param name="mode">The mode we want the output pixel to be.</param>
        /// <param name="dataInput">The ColorValue of the original pixel.</param>
        /// <param name="dataOutput">Receives the output pixel.</param>
        private unsafe static void EncodePixel(HdrEncodingMode mode, ColorValue dataInput, ref void *dataOutput)
        {
            switch (mode)
            {
                case HdrEncodingMode.RGBE8:
                {
                    int *pixel = (int*) dataOutput;
                    *pixel = EncodeRGBE8Pixel(dataInput);
                    pixel++;    // increment the pointer by the size of an RGBE8 pixel (32 bits, sizeof(int))
                    dataOutput = (void*) pixel; // assign the pointer back to dataOutput
                    return;
                }
                case HdrEncodingMode.RGB16:
                {
                    ulong *pixel = (ulong*) dataOutput;
                    *pixel = EncodeRGB16Pixel(dataInput);
                    pixel++;    // increment the pointer by the size of an RGB16 pixel (64 bits, sizeof(ulong))
                    dataOutput = (void*) pixel; // assign the pointer back to dataOutput
                    return;
                }
            }
            // encoding mode wasn't handled - bad mode, throw argument exception
            throw(new ArgumentException("Bad mode in HdrFormats.EncodePixel()", "mode"));
        }
        /// <summary>
        /// Encode the given ColorValue pixel into a RGBE8 pixel.
        /// </summary>
        /// <param name="dataInput">The ColorValue pixel we wish to convert.</param>
        /// <returns>A DirectX ColorValue with the RGBE8 pixel, implicitly casted to an int value.</returns>
        private static int EncodeRGBE8Pixel(ColorValue dataInput)
        {
            // calculate the maximum component
            double maxComponent = (double) Math.Max(dataInput.Red, Math.Max(dataInput.Green, dataInput.Blue));
            // calculate the 2-base exponent required to encode the pixel value
            int nExp = log2_ceiling(maxComponent);
            // find the power of two using that exponent (pre-calculated)
            float fDivisor = (float) powersOfTwo[nExp + 128];
            // create a new ColorValue with scaled color values, encoding the exponent in the alpha value
            return new ColorValue(LimitToZeroAndOne(dataInput.Red / fDivisor), LimitToZeroAndOne(dataInput.Green / fDivisor),
                LimitToZeroAndOne(dataInput.Blue / fDivisor), (float) (nExp + 128) / 255f).ToArgb();
        }
        /// <summary>
        /// Encode the given ColorValue pixel into a RGB16 pixel.
        /// </summary>
        /// <param name="dataInput">The ColorValue pixel we wish to convert.</param>
        /// <returns>A RGB16 pixel, stored in a ulong value.</returns>
        private static ulong EncodeRGB16Pixel(ColorValue dataInput)
        {
            ulong data = 0;
            // set the RGB16 color values into the bytes of the UInt64 value
            unsafe
            {
                ushort* preturnBytes = (ushort*) &data;
                *preturnBytes++ = (ushort) (LimitToZeroAndOne(dataInput.Red / RGB16Maximum) * ushort.MaxValue);
                *preturnBytes++ = (ushort) (LimitToZeroAndOne(dataInput.Green / RGB16Maximum) * ushort.MaxValue);
                *preturnBytes++ = (ushort) (LimitToZeroAndOne(dataInput.Blue / RGB16Maximum) * ushort.MaxValue);
            }
            return data;
        }
        #endregion
        #region Rendering Methods
        /// <summary>
        /// Determine the offsets at which to sample a texture of the given description.
        /// </summary>
        /// <remarks>The center of the texture is determined at 0, and is stored in the first value of the output arrays.  The other values are to the left and right of that point.</remarks>
        /// <param name="D3DTexSize">The size of the texture to sample.</param>
        /// <param name="afTexCoordOffset">Receives the texture coordinates to sample at (center at 0) - must have a length of at least 16.</param>
        /// <param name="avColorWeight">Receives the color weights fot the samples (center at 0) - must have a length of at least 16.</param>
        /// <param name="fDeviation">The rho to use in the Gaussian distribution</param>
        /// <param name="fMultiplier">The scale for the values not at the center</param>
        private static void GetSampleOffsetsBloom(int D3DTexSize, ref float[] afTexCoordOffset, ref Vector4[] avColorWeight, float fDeviation, float fMultiplier)
        {
            // check output array lengths
            if (afTexCoordOffset.Length < 16)
                throw(new ArgumentException("GetSampleOffsets_Bloom output array \"afTexCoordOffset\" is not the proper length (16).", "afTexCoordOffset"));
            if (avColorWeight.Length < 16)
                throw(new ArgumentException("GetSampleOffsets_Bloom output array \"avColorWeight\" is not the proper length (16).", "avColorWeight"));
            // find the width of each pixel, in texture coordinates
            float tu = 1f / (float) D3DTexSize;
            // determine the texture coordinate/weight for the center pixel
            float weight = 1f * GaussianDistribution(0f, 0f, fDeviation);
            avColorWeight[0] = new Vector4(weight, weight, weight, 1f);
            afTexCoordOffset[0] = 0f;
            // determine the texture coordinates/weights for the pixel samples to the right of the center
            for (int i = 1; i < 8; i++)
            {
                weight = fMultiplier * GaussianDistribution((float) i, 0f, fDeviation);
                afTexCoordOffset[i] = i * tu;
                avColorWeight[i] = new Vector4(weight, weight, weight, 1f);
            }
            // mirror the values on the right to the left (same weights, inverse texture coordinate
            for (int i = 8; i < 15; i++)
            {
                avColorWeight[i] = avColorWeight[i - 7];
                afTexCoordOffset[i] = -1f * afTexCoordOffset[i - 7];
            }
        }
        /// <summary>
        /// Get the texture coordinate offsets to be used inside the DownScale2x2_Lum pixel shader.
        /// </summary>
        /// <param name="widthSource">The width of the source texture.</param>
        /// <param name="heightSource">The height of the source texture.</param>
        /// <param name="widthDest">The width of the destination texture</param>
        /// <param name="heightDest">The height of the destination texture.</param>
        /// <param name="avSampleOffsets">Receives the texture coordinates to use in the DownScale2x2_Lum pixel shader - must have a length of at least 4.</param>
        private static void GetSampleOffsetsDownScale2x2Lum(int widthSource, int heightSource, int widthDest, int heightDest, ref Vector2[] avSampleOffsets)
        {
            // check output array length
            if (avSampleOffsets.Length < 4)
                throw(new ArgumentException("GetSampleOffsets_DownScale2x2_Lum output array avSampleOffsets is not the proper length (4).", "avSampleOffsets"));
            // determine the size of the pixels in the source texture, in texture coordinates
            float tU = 1f / (float) widthSource;
            float tV = 1f / (float) heightSource;
            // determine the amount to add to the texture coordinates to find the source coordinate in the destination
            float deltaU = (float) widthSource / (float) widthDest / 2f;
            float deltaV = (float) heightSource / (float) heightDest / 2f;
            // calculate the offsets
            int index = 0;
            for (int y = 0; y < 2; y++)
            {
                for (int x = 0; x < 2; x++)
                {
                    avSampleOffsets[index].X = (x - 0.5f) * deltaU * tU;
                    avSampleOffsets[index].Y = (y - 0.5f) * deltaV * tV;
                    index++;
                }
            }
        }
        /// <summary>
        /// Get the texture coordinate offsets to be used inside the DownScale3x3 pixel shader.
        /// </summary>
        /// <param name="widthSource">The width of the texture.</param>
        /// <param name="heightSource">The height of the texture.</param>
        /// <param name="avSampleOffsets">Receives the texture coordinates to use in the DownScale3x3_Lum pixel shader - must have a length of at least 9.</param>
        private static void GetSampleOffsetsDownScale3x3(int width, int height, ref Vector2[] avSampleOffsets)
        {
            // check output array length
            if (avSampleOffsets.Length < 9)
                throw(new ArgumentException("GetSampleOffsets_DownScale3x3 output array avSampleOffsets is not the proper length (16).", "avSampleOffsets"));
            // determine the width of a pixel in the texture, in texture coordinates
            float tU = 1f / (float) width;
            float tV = 1f / (float) height;
            // calculate the offsets (3x3 grid)
            int index = 0;
            for (int y = -1; y <= 1; y++)
            {
                for (int x = -1; x <= 1; x++)
                {
                    avSampleOffsets[index].X = x * tU;
                    avSampleOffsets[index].Y = y * tV;
                    index++;
                }
            }
        }
        /// <summary>
        /// Render the main model.
        /// </summary>
        private void RenderModel()
        {
            // set the transformations
            Matrix mWorldViewProj = Matrix.Multiply(Matrix.Multiply(camera.WorldMatrix, camera.ViewMatrix), camera.ProjectionMatrix);
            effect.SetValue(worldHandle, camera.WorldMatrix);
            effect.SetValue(worldViewProjHandle, mWorldViewProj);
            effect.SetValue(cubemapTexHandle, arrSkybox[(int) currentEncodingMode].environmentMap);
            // set the technique
            effect.Technique = techHandles[(int) currentEncodingMode, (int) HdrTechnique.Scene];
            // draw the mesh
            int inumPasses = effect.Begin(FX.None);
            for (int ipass = 0; ipass < inumPasses; ipass++)
            {
                effect.BeginPass(ipass);
                mainMesh.Render(device);
                effect.EndPass();
            }
            effect.End();
        }
        /// <summary>
        /// Measure the average log luminance in the scene.
        /// </summary>
        /// <remarks>The results are stored in the arrTonemapTex texture array (textures must already exist).</remarks>
        private void MeasureLuminance()
        {
            Vector2[] avSampleOffsets = new Vector2[16];
            //-------------------------------------------------------------------------
            // Initial sampling pass to convert the image to the log of the grayscale 
            // (renderTex into the last texture in the array using Downscale2x2_Lum)
            //-------------------------------------------------------------------------
            // get the texture information
            Texture texSrc = renderTex;
            SurfaceDescription descSrc = texSrc.GetLevelDescription(0);
            Texture texDest = arrTonemapTex[NumTonemapTextures - 1];
            using(Surface surfDest = texDest.GetSurfaceLevel(0))
            {
                SurfaceDescription descDest = surfDest.Description;
                // set the technique in the effect
                effect.Technique = techHandles[(int) currentEncodingMode, (int) HdrTechnique.DownScale2x2Lum];
                // get the sample offsets for the luminance technique and copy them into the effect
                GetSampleOffsetsDownScale2x2Lum(descSrc.Width, descSrc.Height, descDest.Width, descDest.Height, ref avSampleOffsets);
                unsafe
                {
                    fixed(void *p = &avSampleOffsets[0])
                    {
                        effect.SetValue(sampleOffsetsHandle, p, sizeof(Vector2) * 16);
                    }
                }
                // set the device states
                device.SetRenderTarget(0, surfDest);
                device.SetTexture(0, texSrc);
                device.SamplerState[0].MinFilter = TextureFilter.Point;
                device.SamplerState[0].MagFilter = TextureFilter.Point;
                // render the scene into the texture
                int inumPasses = effect.Begin(FX.None);
                for (int ipass = 0; ipass < inumPasses; ipass++)
                {
                    effect.BeginPass(ipass);
                    DrawFullScreenQuad(0f, 0f, 1f, 1f);
                    effect.EndPass();
                }
                effect.End();
                // clear the data
                device.SetTexture(0, null);
            }
            //-------------------------------------------------------------------------
            // iterate through the remaining tone map textures 
            // ([i] texture rendered into [i-1] texture using Downscale3x3)
            //------------------------------------------------------------------------- 
            for (int i = NumTonemapTextures - 1; i > 0; i--)
            {
                // get the texture information
                texSrc = arrTonemapTex[i];
                descSrc = texSrc.GetLevelDescription(0);
                texDest = arrTonemapTex[i - 1];
                using(Surface surfDest = texDest.GetSurfaceLevel(0))
                {
                    // set the technique in the effect
                    effect.Technique = techHandles[(int) currentEncodingMode, (int) HdrTechnique.DownScale3x3];
                    // get the sample offsets and set them in the effect
                    GetSampleOffsetsDownScale3x3(descSrc.Width, descSrc.Height, ref avSampleOffsets);
                    unsafe
                    {
                        fixed(void *p = &avSampleOffsets[0])
                        {
                            effect.SetValue(sampleOffsetsHandle, p, sizeof(Vector2) * 16);
                        }
                    }
                    // set the device states
                    device.SetRenderTarget(0, surfDest);
                    device.SetTexture(0, texSrc);
                    device.SamplerState[0].MinFilter = TextureFilter.Point;
                    device.SamplerState[0].MagFilter = TextureFilter.Point;
                    // render into the texture
                    int inumPasses = effect.Begin(FX.None);
                    for (int ipass = 0; ipass < inumPasses; ipass++)
                    {
                        effect.BeginPass(ipass);
                        DrawFullScreenQuad(0f, 0f, 1f, 1f);
                        effect.EndPass();
                    }
                    effect.End();
                    // clear the data
                    device.SetTexture(0, null);
                }
            }
        }
        /// <summary>
        /// Prepare for the bloom pass by removing dark information from the scene.
        /// </summary>
        private void BrightPassFilter()
        {
            using(Surface surfBrightPass = brightpassTex.GetSurfaceLevel(0))
            {
                SurfaceDescription backbufDesc = sampleFramework.BackBufferSurfaceDescription;
                // set the technique in the effect
                effect.Technique = techHandles[(int) currentEncodingMode, (int) HdrTechnique.DownScale3x3BrightPass];
                // calculate the sample offsets for the backbuffer and copy the data into the effect
                Vector2[] avSampleOffsets = new Vector2[16];
                GetSampleOffsetsDownScale3x3(backbufDesc.Width / 2, backbufDesc.Height / 2, ref avSampleOffsets);
                unsafe
                {
                    fixed(void *p = &avSampleOffsets[0])
                    {
                        effect.SetValue(sampleOffsetsHandle, p, sizeof(Vector2) * 16);
                    }
                }
                // set the device states
                device.SetRenderTarget(0, surfBrightPass);
                device.SetTexture(0, renderTex);
                device.SetTexture(1, arrTonemapTex[0]);
                device.SamplerState[0].MinFilter = TextureFilter.Point;
                // render into the texture
                int inumPasses = effect.Begin(FX.None);
                for (int ipass = 0; ipass < inumPasses; ipass++)
                {
                    effect.BeginPass(ipass);
                    DrawFullScreenQuad(0f, 0f, 1f, 1f);
                    effect.EndPass();
                }
                effect.End();
            }
        }
        /// <summary>
        /// Render the blooming effect.
        /// </summary>
        private void RenderBloom()
        {
            float[] afSampleOffsets = new float[16];
            Vector4[] avSampleWeights = new Vector4[16];
            Vector2[] avSampleOffsets = new Vector2[16];
            SurfaceDescription desc = brightpassTex.GetLevelDescription(0);
            //-------------------------------------------------------------------------
            // Render from the renderTex into the second bloom texture
            //-------------------------------------------------------------------------
            using(Surface surfDest = arrBloomTex[1].GetSurfaceLevel(0))
            {
                // determine the sample offsets and weights and copy them into the effect
                GetSampleOffsetsBloom(desc.Width, ref afSampleOffsets, ref avSampleWeights, 3f, 1.25f);
                for (int i = 0; i < 16; i++)
                {
                    avSampleOffsets[i] = new Vector2(afSampleOffsets[i], 0f);
                }
                unsafe
                {
                    fixed(void *p = &avSampleOffsets[0])
                    {
                        effect.SetValue(sampleOffsetsHandle, p, sizeof(Vector2) * 16);
                    }
                }
                effect.SetValue(sampleWeightsHandle, avSampleWeights);
                // set the technique in the effect
                effect.Technique = effect.GetTechnique("Bloom");
                // set the device states
                device.SetRenderTarget(0, surfDest);
                device.SetTexture(0, brightpassTex);
                device.SamplerState[0].MinFilter = TextureFilter.Point;
                device.SamplerState[0].MagFilter = TextureFilter.Point;
                // render into the texture
                int inumPasses = effect.Begin(FX.None);
                for (int iPass = 0; iPass < inumPasses; iPass++)
                {
                    effect.BeginPass(iPass);
                    // draw a fullscreen quad to sample the RT
                    DrawFullScreenQuad(0f, 0f, 1f, 1f);
                    effect.EndPass();
                }
                effect.End();
                // clear the data
                device.SetTexture(0, null);
            }
            //-------------------------------------------------------------------------
            // Render from the second bloom texture into the first
            //-------------------------------------------------------------------------
            using(Surface surfDest = arrBloomTex[0].GetSurfaceLevel(0))
            {
                // determine the sample offsets and weights and copy them into the effect
                GetSampleOffsetsBloom(desc.Height, ref afSampleOffsets, ref avSampleWeights, 3f, 1.25f);
                for (int i = 0; i < 16; i++)
                {
                    avSampleOffsets[i] = new Vector2(0f, afSampleOffsets[i]);
                }
                unsafe
                {
                    fixed(void *p = &avSampleOffsets[0])
                    {
                        effect.SetValue(sampleOffsetsHandle, p, sizeof(Vector2) * 16);
                    }
                }
                effect.SetValue(sampleWeightsHandle, avSampleWeights);
                // set the technique in the effect
                effect.Technique = effect.GetTechnique("Bloom");
                // set the device states
                device.SetRenderTarget(0, surfDest);
                device.SetTexture(0, arrBloomTex[1]);
                device.SamplerState[0].MinFilter = TextureFilter.Point;
                device.SamplerState[0].MagFilter = TextureFilter.Point;
                // render into the texture
                int inumPasses = effect.Begin(FX.None);
                for (int iPass = 0; iPass < inumPasses; iPass++)
                {
                    effect.BeginPass(iPass);
                    // draw a fullscreen quad to sample the RT
                    DrawFullScreenQuad(0f, 0f, 1f, 1f);
                    effect.EndPass();
                }
                effect.End();
                // clear the data
                device.SetTexture(0, null);
            }
        }
        /// <summary>
        /// Draw a properly aligned quad covering the entire render target.
        /// </summary>
        /// <param name="fLeftU">The left (minimum) u texture coordinate for the render target.</param>
        /// <param name="fTopV">The top (minimum) u texture coordinate for the render target.</param>
        /// <param name="fRightU">The right (maximum) u texture coordinate for the render target.</param>
        /// <param name="fBottomV">The bottom (maximum) u texture coordinate for the render target.</param>
        private void DrawFullScreenQuad(float fLeftU, float fTopV, float fRightU, float fBottomV)
        {
            // acquire render target width and height
            Surface pSurfRT = device.GetRenderTarget(0);
            SurfaceDescription dtdsdRT = pSurfRT.Description;
            pSurfRT.Dispose();
            // ensure that we're directly mapping texels to pixels by offset by 0.5
            // for more info see the doc page titled "Directly Mapping Texels to Pixels"
            float fWidth5 = (float) dtdsdRT.Width - 0.5f;
            float fHeight5 = (float) dtdsdRT.Height - 0.5f;
            // set up the quad
            CustomVertex.TransformedTextured[] verts = quadvb.Lock(0,typeof(CustomVertex.TransformedTextured),LockFlags.None, 4) as CustomVertex.TransformedTextured[];
            verts[0].X = -0.5f;       verts[0].Y = -0.5f;       verts[0].Z = 0.5f;    verts[0].Rhw = 1;
            verts[0].Tu = fLeftU;     verts[0].Tv = fTopV;
            verts[1].X = fWidth5;     verts[1].Y = -0.5f;       verts[1].Z = 0.5f;    verts[1].Rhw = 1;
            verts[1].Tu = fRightU;    verts[1].Tv = fTopV;
            verts[2].X = -0.5f;       verts[2].Y = fHeight5;    verts[2].Z = 0.5f;    verts[2].Rhw = 1;
            verts[2].Tu = fLeftU;     verts[2].Tv = fBottomV;
            verts[3].X = fWidth5;     verts[3].Y = fHeight5;    verts[3].Z = 0.5f;    verts[3].Rhw = 1;
            verts[3].Tu = fRightU;    verts[3].Tv = fBottomV;
            quadvb.Unlock();
            // Draw the quad
            device.RenderState.ZBufferEnable = false;
            device.VertexFormat = CustomVertex.TransformedTextured.Format;
            device.SetStreamSource(0, quadvb, 0);
            device.DrawPrimitives(PrimitiveType.TriangleStrip, 0, 2);
            device.RenderState.ZBufferEnable = true;
        }
        /// <summary>
        /// Render the help and statistics text. This function uses the Font object for efficient text rendering.
        /// </summary>
        private void RenderText()
        {
            TextHelper txtHelper = new TextHelper(statsFont, textSprite, 15);
            // begin the font write
            txtHelper.Begin();
            txtHelper.SetInsertionPoint(2, 0);
            // write the sample framework statistics
            txtHelper.SetForegroundColor(System.Drawing.Color.Yellow);
            txtHelper.DrawTextLine(sampleFramework.FrameStats);
            txtHelper.DrawTextLine(sampleFramework.DeviceStats);
            // write the current texture encoding and rendering modes
            txtHelper.SetForegroundColor(System.Drawing.Color.White);
            txtHelper.DrawTextLine(HdrEncodingModeToString(currentEncodingMode, true));
            txtHelper.DrawTextLine(HdrRenderingModeToString(currentRenderingMode, true));
            // write the help text
            if (showHelpText)
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*6);
                txtHelper.SetForegroundColor(System.Drawing.Color.DarkOrange);
                txtHelper.DrawTextLine("Controls (F1 to hide):");
                txtHelper.SetInsertionPoint(20, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
                txtHelper.DrawTextLine("Rotate model: Left mouse button");
                txtHelper.DrawTextLine("Rotate camera: Right mouse button");
                txtHelper.DrawTextLine("Zoom camera: Mouse wheel scroll");
                txtHelper.DrawTextLine("Quit: ESC");
                txtHelper.SetInsertionPoint(250, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
                txtHelper.DrawTextLine("Cycle encoding: E");
                txtHelper.DrawTextLine("Cycle render mode: R");
                txtHelper.DrawTextLine("Toggle bloom: B");
                txtHelper.DrawTextLine("Hide text: T");
                txtHelper.SetInsertionPoint(410, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
                txtHelper.DrawTextLine("Hide help: F1");
                txtHelper.DrawTextLine("Change Device: F2");
                txtHelper.DrawTextLine("Toggle HAL/REF: F3");
                txtHelper.DrawTextLine("View readme: F9");
            }
            else
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*2);
                txtHelper.SetForegroundColor(System.Drawing.Color.White);
                txtHelper.DrawTextLine("Press F1 for help");
            }
            txtHelper.End();
        }
        #endregion
        #region Mathematical Helper Functions
        /// <summary>
        /// Limits the provided value to being between 0 and 1.
        /// </summary>
        /// <param name="val">The value to be limited.</param>
        /// <returns>The provided value, limited to being between zero and one.</returns>
        private static float LimitToZeroAndOne(float val)
        {
            return Math.Max(0f, Math.Min(1f, val));
        }
        /// <summary>
        /// Perform a Gaussian distribution at rho between x and y.
        /// </summary>
        /// <param name="x">One of the anchor values for the distribution.</param>
        /// <param name="y">One of the anchor values for the distribution.</param>
        /// <param name="rho">The location between the two anchor values to evaluate.</param>
        /// <returns>Gaussian distribution evaluation at rho between x and y.</returns>
        private static float GaussianDistribution(float x, float y, float rho)
        {
            float g = 1f / (float) Math.Sqrt((double) (2f * Math.PI * rho * rho));
            g *= (float) Math.Exp((double) (-1f * (x * x + y * y) / (2 * rho * rho)));
            return g;
        }
        /// <summary>
        /// Calculates the log-base-2 of the provided value.
        /// </summary>
        /// <remarks>This function uses the precalculated values in powersOfTwo[].</remarks>
        /// <param name="val">The value we wish to convert.</param>
        /// <returns>The log-base-2 (rounded up) of the provided value.</returns>
        private static int log2_ceiling(double val)
        {
            int iMax = 256;
            int iMin = 0;
            while ((iMax - iMin) > 1)
            {
                int iMiddle = (iMax + iMin) / 2;
                if (val > powersOfTwo[iMiddle])
                    iMin = iMiddle;
                else
                    iMax = iMiddle;
            }
            return iMax - 128;
        }
        #endregion
        #region Miscellaneous Methods
        /// <summary>
        /// Initializes the application.
        /// </summary>
        public void InitializeApplication()
        {
            // initialize the HUD
            int y = 10;
            Button fullScreen = hud.AddButton(ToggleFullscreen,"Toggle full screen", 35, y, 125, 22);
            fullScreen.Click += new EventHandler(OnFullscreenClicked);
            Button toggleRef = hud.AddButton(ToggleReference,"Toggle reference (F3)", 35, y += 24, 125, 22);
            toggleRef.Click += new EventHandler(OnRefClicked);
            Button changeDevice = hud.AddButton(ChangeDevice,"Change Device (F2)", 35, y += 24, 125, 22);
            changeDevice.Click += new EventHandler(OnChangeDeviceClicked);
            // add the HdrFormats sample specific UI
            encodingCombo = sampleUi.AddComboBox(EncodingModeCombo, 0, 25, 140, 24, System.Windows.Forms.Keys.E, false);
            encodingCombo.SetDropHeight(50);
            encodingCombo.Changed += new EventHandler(OnEncodingModeChanged);
            ComboBox renderingCombo = sampleUi.AddComboBox(RenderingModeCombo, 0, 70, 140, 24, System.Windows.Forms.Keys.R, false);
            renderingCombo.SetDropHeight(30);
            for (int i = 0; i < (int) HdrRenderingMode.Count; i++)
            {
                renderingCombo.AddItem(HdrRenderingModeToString((HdrRenderingMode) i, false), (HdrRenderingMode) i);
            }
            renderingCombo.Changed += new EventHandler(OnRenderingModeChanged);
            bloomCheck = sampleUi.AddCheckBox(BloomCheck, "Show (B)loom", 0, 110, 140, 18, doBloom, System.Windows.Forms.Keys.B, false);
            bloomCheck.Changed += new EventHandler(OnBloomCheckChanged);
            bloomCheck.IsEnabled = (currentRenderingMode == HdrRenderingMode.Decoded);
            bloomCheck.IsChecked = (bloomCheck.IsEnabled && doBloom);
        }
        /// <summary>
        /// Disposes of the application properly.
        /// </summary>
        public void Dispose()
        {
            // release the text sprite
            if (textSprite != null)
            {
                textSprite.Dispose();
                textSprite = null;
            }
            // release the render textures
            if (renderTex != null)
            {
                renderTex.Dispose();
                renderTex = null;
            }
            if (brightpassTex != null)
            {
                brightpassTex.Dispose();
                brightpassTex = null;
            }
            // release the tonemap textures
            for (int i = 0; i < NumTonemapTextures; i++)
            {
                if (arrTonemapTex[i] != null)
                {
                    arrTonemapTex[i].Dispose();
                    arrTonemapTex[i] = null;
                }
            }
            // release the temporary blooming effect textures
            for (int i = 0; i < NumBloomTextures; i++)
            {
                if (arrBloomTex[i] != null)
                {
                    arrBloomTex[i].Dispose();
                    arrBloomTex[i] = null;
                }
            }
            // release the mesh
            if (mainMesh != null)
            {
                mainMesh.Dispose();  
                mainMesh = null;     
            }
            // release the screen-quad vertex buffer
            if (quadvb != null)
            {
                quadvb.Dispose();
                quadvb = null;
            }
            // release the device
            if (device != null)
            {
                device.Dispose();
                device = null;
            }
            // do not release the effect or font - the resource cache will take care of it
        }
        #endregion
        #endregion

        #region Main
        /// <summary>
        /// Entry point to the program. Initializes everything and goes into a message processing 
        /// loop. Idle time is used to render the scene.
        /// </summary>
        static int Main() 
        {
            // create the framework
            System.Windows.Forms.Application.EnableVisualStyles();
            using(Framework sampleFramework = new Framework())
            {
                // create the HdrFormats object
                HdrFormats sample = new HdrFormats(sampleFramework);
                try
                {
                    // show the cursor and clip it when in full screen
                    sampleFramework.SetCursorSettings(true, true);
                    // initialize the HdrFormats sample
                    sample.InitializeApplication();
                    // Initialize the sample framework and create the desired window and Direct3D 
                    // device for the application. Calling each of these functions is optional, but they
                    // allow you to set several options which control the behavior of the sampleFramework.
                    sampleFramework.Initialize(true, true, true); // Parse the command line, handle the default hotkeys, and show msgboxes
                    sampleFramework.CreateWindow("HdrFormats");
                    sampleFramework.CreateWindow("EmptyProject");
                    // Hook the keyboard event
                    sampleFramework.Window.KeyDown += new System.Windows.Forms.KeyEventHandler(sample.OnKeyEvent);
                    sampleFramework.CreateDevice(0, true, Framework.DefaultSizeWidth, Framework.DefaultSizeHeight, sample);
                    // Pass control to the sample framework for handling the message pump and 
                    // dispatching render calls. The sample framework will call your FrameMove 
                    // and FrameRender callback when there is idle time between handling window messages.
                    sampleFramework.MainLoop();
                }
#if(DEBUG)
                catch (Exception e)
                {
                    // In debug mode show this error (maybe - depending on settings)
                    sampleFramework.DisplayErrorMessage(e);
#else
               catch
               {
                   // In release mode fail silently
#endif
                    sample.Dispose();
                    // Ignore any exceptions here, they would have been handled by other areas
                    return (sampleFramework.ExitCode == 0) ? 1 : sampleFramework.ExitCode; // Return an error code here
                }
                // Perform any application-level cleanup here. Direct3D device resources are released within the
                // appropriate callback functions and therefore don't require any cleanup code here.
                sample.Dispose();
                return sampleFramework.ExitCode;
            }
        }
        #endregion
    }
}
