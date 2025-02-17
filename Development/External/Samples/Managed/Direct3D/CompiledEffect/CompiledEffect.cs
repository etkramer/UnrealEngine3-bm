//-----------------------------------------------------------------------------
// File: CompiledEffect.cs
//
// Starting point for new Direct3D applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace CompiledEffectSample
{
    /// <summary>CompiledEffect Sample Class</summary>
    public class CompiledEffect : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public CompiledEffect(Framework f) 
        { 
            // Store framework
            sampleFramework = f; 
            // Create dialogs
            hud = new Dialog(sampleFramework); 
        }
        #endregion

        // constants
        private static readonly ColorValue WhiteColor = new ColorValue(1.0f,1.0f,1.0f,1.0f);

        // Variables
        private Framework sampleFramework = null; // Framework for samples
        private Font statsFont = null; // Font for drawing text
        private Sprite textSprite = null; // Sprite for batching text calls
        private Effect effect = null; // D3DX Effect Interface
        private ModelViewerCamera camera = new ModelViewerCamera(); // A model viewing camera
        private bool isHelpShowing = true; // If true, renders the UI help text
        private Dialog hud = null; // dialog for standard controls
        private FrameworkMesh mesh = null; // Mesh object
        private Vector3 objectCenter; // Center of bounding sphere of object
        private float objectRadius; // Radius of bounding sphere of object
        private Matrix worldCenter; // World matrix to center the mesh

        // Effect handles
        private EffectHandle timeHandle = null; 
        private EffectHandle worldHandle = null; 
        private EffectHandle meshTextureHandle = null; 
        private EffectHandle worldViewHandle = null; 
        private EffectHandle techniqueHandle = null; 

        // HUD Ui Control constants
        private const int ToggleFullscreen = 1;
        private const int ToggleReference = 3;
        private const int ChangeDevice = 4;

        /// <summary>
        /// Called during device initialization, this code checks the device for some 
        /// minimum set of capabilities, and rejects those that don't pass by returning false.
        /// </summary>
        public bool IsDeviceAcceptable(Caps caps, Format adapterFormat, Format backBufferFormat, bool windowed)
        {
            // No fallback, need at least PS1.1
            if (caps.PixelShaderVersion < new Version(1,1))
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
                settings.BehaviorFlags |= CreateFlags.SoftwareVertexProcessing;
            }
#endif
#if(DEBUG_PS)
            settings.DeviceType = DeviceType.Reference;
#endif

            // For the first device created if its a REF device, optionally display a warning dialog box
            if (settings.DeviceType == DeviceType.Reference)
            {
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "Compiled Effect");
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
            // Initialize the stats font
            statsFont = ResourceCache.GetGlobalInstance().CreateFont(e.Device, 15, 0, FontWeight.Bold, 1, false, CharacterSet.Default,
                Precision.Default, FontQuality.Default, PitchAndFamily.FamilyDoNotCare | PitchAndFamily.DefaultPitch
                , "Arial");

            // Load the mesh
            mesh = new FrameworkMesh(e.Device, "dwarf\\dwarf.x");

            // Find the mesh's center, then generate a centering matrix
            using(VertexBuffer vb = mesh.SystemMesh.VertexBuffer)
            {
                using (GraphicsStream stm = vb.Lock(0, 0, LockFlags.None))
                {
                    try
                    {
                        objectRadius = Geometry.ComputeBoundingSphere(stm,
                            mesh.SystemMesh.NumberVertices, mesh.SystemMesh.VertexFormat, out objectCenter);

                        worldCenter = Matrix.Translation(-objectCenter);
                    }
                    finally
                    {
                        vb.Unlock();
                    }
                }
            }

            // Try to find the D3DX Effect file
            try
            {
                string path = Utility.FindMediaFile("CompiledEffect.fxo");

                // Since we are loading a binary file here and this effect has already been compiled,
                // you can not pass compiler flags here (for example to debug the shaders). 
                // To debug the shaders, you must pass these flags to the compiler that generated the
                // binary (for example fxc.exe).  In this sample, there is an extra Visual Studio configuration
                // called "Debug Shaders" that pass the debug shader flags to fxc.exe.
                effect = ResourceCache.GetGlobalInstance().CreateEffectFromFile(e.Device,
                    path, null, null, ShaderFlags.NotCloneable, null);
            }
            catch(MediaNotFoundException)
            {
                // Inform the user
                System.Windows.Forms.MessageBox.Show("Could not locate \"CompiledEffect.fxo\".\n\n" +
                    "This file is created as part of the project build process,\n" +
                    "so the associated project must be compiled inside Visual\n" +
                    "Studio before attempting to run this sample.\n\n" +
                    "If receiving this error even after compiling the project,\n" +
                    "it's likely there was a problem compiling the effect file.\n" +
                    "Check the build log to verify the custom build step was\n" +
                    "run and to look for possible fxc compile errors.",  "File Not Found",
                    System.Windows.Forms.MessageBoxButtons.OK,
                    System.Windows.Forms.MessageBoxIcon.Error);

                // Rethrow
                throw;
            }

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

            // Set effect variables as needed
            effect.SetValue("g_MaterialAmbientColor", new ColorValue(0.35f, 0.35f, 0.35f, 0));
            effect.SetValue("g_MaterialDiffuseColor", WhiteColor);

            // To read or write to D3DX effect variables we can use the string name 
            // instead of using handles, however it improves perf to use handles since then 
            // D3DX won't have to spend time doing string compares
            techniqueHandle = effect.GetTechnique("RenderScene");
            timeHandle = effect.GetParameter(null, "g_fTime");
            worldHandle = effect.GetParameter(null, "g_mWorld");
            worldViewHandle = effect.GetParameter(null, "g_mWorldViewProjection");
            meshTextureHandle = effect.GetParameter(null, "g_MeshTexture");

            // Setup the camera's projection parameters
            float aspectRatio = (float)desc.Width / (float)desc.Height;
            camera.SetProjectionParameters((float)Math.PI / 4, aspectRatio, 0.1f, 1000.0f);
            camera.SetWindow(desc.Width, desc.Height);

            // Setup UI locations
            hud.SetLocation(desc.Width-170, 0);
            hud.SetSize(170,170);
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
            if (textSprite != null)
            {
                textSprite.Dispose();
                textSprite = null;
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

            // Get the world matrix
            Matrix worldMatrix = worldCenter * camera.WorldMatrix;

            // Update the effect's variables
            effect.SetValue(worldViewHandle, worldMatrix * camera.ViewMatrix * camera.ProjectionMatrix);
            effect.SetValue(worldHandle, worldMatrix);
            effect.SetValue(timeHandle, (float)appTime);
        }

        /// <summary>
        /// This callback function will be called at the end of every frame to perform all the 
        /// rendering calls for the scene, and it will also be called if the window needs to be 
        /// repainted. After this function has returned, the sample framework will call 
        /// Device.Present to display the contents of the next buffer in the swap chain
        /// </summary>
        public void OnFrameRender(Device device, double appTime, float elapsedTime)
        {
            bool beginSceneCalled = false;

            // Clear the render target and the zbuffer 
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, 0x002D32AA, 1.0f, 0);
            try
            {
                device.BeginScene();
                beginSceneCalled = true;

                int passes = effect.Begin(0);
                for(int iPass = 0; iPass < passes; iPass++)
                {
                    effect.BeginPass(iPass);
                    mesh.Render(device);
                    effect.EndPass();
                }

                effect.End();
                // Show frame rate
                RenderText();

                // Show UI
                hud.OnRender(elapsedTime);
            }
            finally
            {
                if (beginSceneCalled)
                    device.EndScene();
            }
        }

        /// <summary>
        /// Render the help and statistics text. This function uses the Font object for 
        /// efficient text rendering.
        /// </summary>
        private void RenderText()
        {
            TextHelper txtHelper = new TextHelper(statsFont, textSprite, 15);

            // Output statistics
            txtHelper.Begin();
            txtHelper.SetInsertionPoint(5,5);
            txtHelper.SetForegroundColor(System.Drawing.Color.Yellow);
            txtHelper.DrawTextLine(sampleFramework.FrameStats);
            txtHelper.DrawTextLine(sampleFramework.DeviceStats);

            // Draw help
            if (isHelpShowing)
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*6);
                txtHelper.SetForegroundColor(System.Drawing.Color.DarkOrange);
                txtHelper.DrawTextLine("Controls (F1 to hide):");

                txtHelper.SetInsertionPoint(40, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
                txtHelper.DrawTextLine("Rotate model: Left mouse button");
                txtHelper.DrawTextLine("Rotate camera: Right mouse button");
                txtHelper.DrawTextLine("Zoom camera: Mouse wheel scroll");
                txtHelper.DrawTextLine("Quit: Esc");
                txtHelper.DrawTextLine("Hide help: F1");
            }
            else
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*2);
                txtHelper.SetForegroundColor(System.Drawing.Color.White);
                txtHelper.DrawTextLine("Press F1 for help");
            }

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
                case System.Windows.Forms.Keys.F1:
                    isHelpShowing = !isHelpShowing;
                    break;
            }
        }

        /// <summary>
        /// Before handling window messages, the sample framework passes incoming windows 
        /// messages to the application through this callback function. If the application sets 
        /// noFurtherProcessing to true, the sample framework will not process the message
        /// </summary>
        public IntPtr OnMsgProc(IntPtr hWnd, NativeMethods.WindowMessage msg, IntPtr wParam, IntPtr lParam, ref bool noFurtherProcessing)
        {
            // Give the dialog a chance to handle the message first
            noFurtherProcessing = hud.MessageProc(hWnd, msg, wParam, lParam);
            if (noFurtherProcessing)
                return IntPtr.Zero;

            // Pass all remaining windows messages to camera so it can respond to user input
            camera.HandleMessages(hWnd, msg, wParam, lParam);

            return IntPtr.Zero;
        }

        /// <summary>
        /// Initializes the application
        /// </summary>
        public void InitializeApplication()
        {
            int y = 10;
            // Initialize the HUD
            Button fullScreen = hud.AddButton(ToggleFullscreen,"Toggle full screen", 35, y, 125,22);
            Button toggleRef = hud.AddButton(ToggleReference,"Toggle reference (F3)", 35, y += 24, 125,22);
            Button changeDevice = hud.AddButton(ChangeDevice,"Change Device (F2)", 35, y += 24, 125,22);
            // Hook the button events for when these items are clicked
            fullScreen.Click += new EventHandler(OnFullscreenClicked);
            toggleRef.Click += new EventHandler(OnRefClicked);
            changeDevice.Click += new EventHandler(OnChangeDevicClicked);
        }

        /// <summary>Called when the change device button is clicked</summary>
        private void OnChangeDevicClicked(object sender, EventArgs e)
        {
            sampleFramework.ShowSettingsDialog(!sampleFramework.IsD3DSettingsDialogShowing);
        }

        /// <summary>Called when the full screen button is clicked</summary>
        private void OnFullscreenClicked(object sender, EventArgs e)
        {
            sampleFramework.ToggleFullscreen();
        }

        /// <summary>Called when the ref button is clicked</summary>
        private void OnRefClicked(object sender, EventArgs e)
        {
            sampleFramework.ToggleReference();
        }

        /// <summary>
        /// Entry point to the program. Initializes everything and goes into a message processing 
        /// loop. Idle time is used to render the scene.
        /// </summary>
        static int Main() 
        {
            System.Windows.Forms.Application.EnableVisualStyles();
            using(Framework sampleFramework = new Framework())
            {
                CompiledEffect sample = new CompiledEffect(sampleFramework);
                // Set the callback functions. These functions allow the sample framework to notify
                // the application about device changes, user input, and windows messages.  The 
                // callbacks are optional so you need only set callbacks for events you're interested 
                // in. However, if you don't handle the device reset/lost callbacks then the sample 
                // framework won't be able to reset your device since the application must first 
                // release all device resources before resetting.  Likewise, if you don't handle the 
                // device created/destroyed callbacks then the sample framework won't be able to 
                // recreate your device resources.
                sampleFramework.Disposing += new EventHandler(sample.OnDestroyDevice);
                sampleFramework.DeviceLost += new EventHandler(sample.OnLostDevice);
                sampleFramework.DeviceCreated += new DeviceEventHandler(sample.OnCreateDevice);
                sampleFramework.DeviceReset += new DeviceEventHandler(sample.OnResetDevice);
                sampleFramework.SetWndProcCallback(new WndProcCallback(sample.OnMsgProc));

                sampleFramework.SetCallbackInterface(sample);
                try
                {

                    // Show the cursor and clip it when in full screen
                    sampleFramework.SetCursorSettings(true, true);

                    // Initialize
                    sample.InitializeApplication();

                    // Initialize the sample framework and create the desired window and Direct3D 
                    // device for the application. Calling each of these functions is optional, but they
                    // allow you to set several options which control the behavior of the sampleFramework.
                    sampleFramework.Initialize( true, true, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
                    sampleFramework.CreateWindow("Compiled Effect");
                    // Hook the keyboard event
                    sampleFramework.Window.KeyDown += new System.Windows.Forms.KeyEventHandler(sample.OnKeyEvent);
                    sampleFramework.CreateDevice( 0, true, Framework.DefaultSizeWidth, Framework.DefaultSizeHeight, 
                        sample);

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
                    // Ignore any exceptions here, they would have been handled by other areas
                    return (sampleFramework.ExitCode == 0) ? 1 : sampleFramework.ExitCode; // Return an error code here
                }

                // Perform any application-level cleanup here. Direct3D device resources are released within the
                // appropriate callback functions and therefore don't require any cleanup code here.
                return sampleFramework.ExitCode;
            }
        }
    }
}
