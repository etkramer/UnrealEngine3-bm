//-----------------------------------------------------------------------------
// File: BasicHLSL.cs
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

namespace BasicHLSLSample
{
    /// <summary>Class for rendering a 3d scene using basic HLSL</summary>
    public class BasicHLSL : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public BasicHLSL(Framework f) 
        { 
            // Store framework
            sampleFramework = f; 
            // Create dialogs
            hud = new Dialog(sampleFramework); 
            sampleUi = new Dialog(sampleFramework); 
        }
        #endregion

        // constants
        private static readonly ColorValue WhiteColor = new ColorValue(1.0f,1.0f,1.0f,1.0f);
        private static readonly ColorValue YellowColor = new ColorValue(1.0f,1.0f,0.0f,1.0f);

        // Variables
        private Framework sampleFramework = null; // Framework for samples
        private Font statsFont = null; // Font for drawing text
        private Sprite textSprite = null; // Sprite for batching text calls
        private Effect effect = null; // D3DX Effect Interface
        private ModelViewerCamera camera = new ModelViewerCamera(); // A model viewing camera
        private Mesh mesh = null; // The mesh object
        private Texture meshTexture = null; // The meshes texture
        private bool isHelpShowing = true; // If true, renders the UI help text
        private bool isUsingPreshader; // If false, the NoPreshader flag is used when compiling the shader
        private Dialog hud; // dialog for standard controls
        private Dialog sampleUi; // dialog for sample specific controls
        private Matrix worldFix;
        
        // Light stuff
        DirectionWidget[] lightControl = new DirectionWidget[MaxNumberLights];
        private const int MaxNumberLights = 3;
        private float lightScale;
        private int numberActiveLights;
        private int activeLight;

        // HUD Ui Control constants
        private const int ToggleFullscreen = 1;
        private const int ToggleReference = 3;
        private const int ChangeDevice = 4;
        private const int EnablePreshader = 5;
        private const int NumberLights = 6;
        private const int NumberLightsStatic = 7;
        private const int ActiveLightControl = 8;
        private const int LightScaleControl = 9;
        private const int LightScaleStatic = 10;

        /// <summary>
        /// Called during device initialization, this code checks the device for some 
        /// minimum set of capabilities, and rejects those that don't pass by returning false.
        /// </summary>
        public bool IsDeviceAcceptable(Caps caps, Format adapterFormat, Format backBufferFormat, bool windowed)
        {
            // No fallback defined by this app, so reject any device that 
            // doesn't support at least ps1.1
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
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "BasicHLSL");
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
            mesh = LoadMesh(e.Device, "tiny\\tiny.x");

            // Calculate a bounding sphere
            float radius = 0.0f;
            using (GraphicsStream data = mesh.LockVertexBuffer(LockFlags.None))
            {
                Vector3 center;
                radius = Geometry.ComputeBoundingSphere(data, mesh.NumberVertices, mesh.VertexFormat, out center);

                worldFix = Matrix.Translation(-center);
                worldFix *= Matrix.RotationY((float)Math.PI);
                worldFix *= Matrix.RotationX((float)Math.PI / 2.0f);

                // Setup direction widget
                DirectionWidget.OnCreateDevice(e.Device);
                for(int i = 0; i < MaxNumberLights; i++)
                    lightControl[i].Radius = radius;
                
                // Finally unlock the vertex buffer
                mesh.UnlockVertexBuffer();
            }

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
            // Preshaders are parts of the shader that the effect system pulls out of the 
            // shader and runs on the host CPU. They should be used if you are GPU limited. 
            // The ShaderFlags.NoPreShader flag disables preshaders.
            if (!isUsingPreshader)
                shaderFlags |= ShaderFlags.NoPreShader;

            // Read the D3DX effect file
            string path = Utility.FindMediaFile("BasicHLSL.fx");
            effect = ResourceCache.GetGlobalInstance().CreateEffectFromFile(e.Device,
                path, null, null, shaderFlags, null);

            // Create the mesh texture from a file
            path = Utility.FindMediaFile("tiny\\tiny_skin.dds");
            meshTexture = ResourceCache.GetGlobalInstance().CreateTextureFromFile(e.Device, path);

            // Set effect variables as needed
            effect.SetValue("g_MaterialAmbientColor", new ColorValue(0.35f, 0.35f, 0.35f, 0));
            effect.SetValue("g_MaterialDiffuseColor", WhiteColor);
            effect.SetValue("g_MeshTexture", meshTexture);

            // Setup the camera's view parameters
            camera.SetViewParameters(new Vector3(0.0f, 0.0f, -15.0f), Vector3.Empty);
            camera.SetRadius(radius * 3.0f, radius * 0.5f, radius * 10.0f);
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

            // Reset items
            for (int i = 0; i < MaxNumberLights; i++)
                lightControl[i].OnResetDevice(desc);

            // Setup the camera's projection parameters
            float aspectRatio = (float)desc.Width / (float)desc.Height;
            camera.SetProjectionParameters((float)Math.PI / 4, aspectRatio, 0.1f, 5000.0f);
            camera.SetWindow(desc.Width, desc.Height);
            camera.SetButtonMasks((int)MouseButtonMask.Left, (int)MouseButtonMask.Wheel, (int)MouseButtonMask.Middle);

            // Setup UI locations
            hud.SetLocation(desc.Width-170, 0);
            hud.SetSize(170,170);
            sampleUi.SetLocation(desc.Width - 170, desc.Height - 300);
            sampleUi.SetSize(170,300);
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

            // Update the direction widget
            DirectionWidget.OnLostDevice();
        }

        /// <summary>
        /// This event will be fired immediately after the Direct3D device has 
        /// been destroyed, which generally happens as a result of application termination or 
        /// windowed/full screen toggles. Resources created in the OnCreateDevice event 
        /// should be released here, which generally includes all Pool.Managed resources. 
        /// </summary>
        private void OnDestroyDevice(object sender, EventArgs e)
        {
            // Update the direction widget
            DirectionWidget.OnDestroyDevice();
            if (mesh != null)
                mesh.Dispose();
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

        /// <summary>
        /// This callback function will be called at the end of every frame to perform all the 
        /// rendering calls for the scene, and it will also be called if the window needs to be 
        /// repainted. After this function has returned, the sample framework will call 
        /// Device.Present to display the contents of the next buffer in the swap chain
        /// </summary>
        public unsafe void OnFrameRender(Device device, double appTime, float elapsedTime)
        {
            bool beginSceneCalled = false;

            // Clear the render target and the zbuffer 
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, unchecked((int)0x8C003F3F), 1.0f, 0);
            try
            {
                device.BeginScene();
                beginSceneCalled = true;

                Vector3* pLightDir = stackalloc Vector3[MaxNumberLights];
                ColorValue* pLightDiffuse = stackalloc ColorValue[MaxNumberLights];

                // Render the arrows so the user can visually see the light direction
                for (int i = 0; i < numberActiveLights; i++)
                {
                    ColorValue arrowColor = (i == activeLight) ? YellowColor : WhiteColor;
                    lightControl[i].OnRender(arrowColor, camera.ViewMatrix, camera.ProjectionMatrix, camera.EyeLocation);
                    // Get the light direction and color
                    pLightDir[i] = lightControl[i].LightDirection;
                    pLightDiffuse[i] = ColorOperator.Scale(WhiteColor, lightScale);
                }

                Matrix worldMatrix = worldFix * camera.WorldMatrix;
                // Update the effects now
                effect.SetValue("g_LightDir", pLightDir, sizeof(Vector3) * MaxNumberLights);
                effect.SetValue("g_LightDiffuse", pLightDiffuse, sizeof(ColorValue) * MaxNumberLights);

                // Update the effect's variables.  Instead of using strings, it would 
                // be more efficient to cache a handle to the parameter by calling 
                // Effect.GetParameter
                effect.SetValue("worldViewProjection", worldMatrix * camera.ViewMatrix * camera.ProjectionMatrix);
                effect.SetValue("worldMatrix", worldMatrix);
                effect.SetValue("appTime", (float)appTime);

                effect.SetValue("g_MaterialDiffuseColor", WhiteColor);
                effect.SetValue("g_nNumLights", numberActiveLights);

                // Render the scene with this technique as defined in the .fx file
                switch(numberActiveLights)
                {
                    case 1: effect.Technique = "RenderSceneWithTexture1Light"; break;
                    case 2: effect.Technique = "RenderSceneWithTexture2Light"; break;
                    case 3: effect.Technique = "RenderSceneWithTexture3Light"; break;
                }

                // Apply the technique contained in the effect 
                int passes = effect.Begin(0);
                for (int pass = 0; pass < passes; pass++)
                {
                    effect.BeginPass(pass);

                    // The effect interface queues up the changes and performs them 
                    // with the CommitChanges call. You do not need to call CommitChanges if 
                    // you are not setting any parameters between the BeginPass and EndPass.
                    // effect.CommitChanges() );

                    // Render the mesh with the applied technique
                    mesh.DrawSubset(0);
                    effect.EndPass();
                }
                effect.End();

                // Show frame rate and help, etc
                RenderText(appTime);

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

        /// <summary>
        /// Render the help and statistics text. This function uses the Font object for 
        /// efficient text rendering.
        /// </summary>
        private void RenderText(double appTime)
        {
            TextHelper txtHelper = new TextHelper(statsFont, textSprite, 15);

            // Output statistics
            txtHelper.Begin();
            txtHelper.SetInsertionPoint(2,0);
            txtHelper.SetForegroundColor(unchecked((int)0xffffff00));
            txtHelper.DrawTextLine(sampleFramework.FrameStats);
            txtHelper.DrawTextLine(sampleFramework.DeviceStats);

            txtHelper.SetForegroundColor(unchecked((int)0xffffffff));
            txtHelper.DrawTextLine("appTime: {0} sin(appTime): {1}", appTime.ToString("f2",
                System.Globalization.CultureInfo.CurrentUICulture), Math.Sin(appTime).ToString("f4",
                System.Globalization.CultureInfo.CurrentUICulture));

            // Draw help
            if (isHelpShowing)
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*6);
                txtHelper.SetForegroundColor(System.Drawing.Color.DarkOrange);
                txtHelper.DrawTextLine("Controls (F1 to hide):");

                txtHelper.SetInsertionPoint(20, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
                txtHelper.DrawTextLine("Rotate model: Left mouse button");
                txtHelper.DrawTextLine("Rotate light: Right mouse button");
                txtHelper.DrawTextLine("Rotate camera: Middle mouse button");
                txtHelper.DrawTextLine("Zoom camera: Mouse wheel scroll");

                txtHelper.SetInsertionPoint(250, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
                txtHelper.DrawTextLine("Hide help: F1");
                txtHelper.DrawTextLine("Quit: Esc");
            }
            else
            {
                txtHelper.SetForegroundColor(unchecked((int)0xffffffff));
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

            noFurtherProcessing = sampleUi.MessageProc(hWnd, msg, wParam, lParam);
            if (noFurtherProcessing)
                return IntPtr.Zero;

            // Give the light control a chance now
            lightControl[activeLight].HandleMessages(hWnd, msg, wParam, lParam);

            // Pass all remaining windows messages to camera so it can respond to user input
            camera.HandleMessages(hWnd, msg, wParam, lParam);

            return IntPtr.Zero;
        }

        /// <summary>
        /// This function loads the mesh and ensures the mesh has normals; it also optimizes the 
        /// mesh for the graphics card's vertex cache, which improves performance by organizing 
        /// the internal triangle list for less cache misses.
        /// </summary>
        private Mesh LoadMesh(Device device, string filename)
        {
            // Load the mesh with D3DX and get back a Mesh.  For this
            // sample we'll ignore the X file's embedded materials since we know 
            // exactly the model we're loading.  See the mesh samples such as
            // "OptimizedMesh" for a more generic mesh loading example.
            string path = Utility.FindMediaFile(filename);
            Mesh m = Mesh.FromFile(path, MeshFlags.Managed, device);

            // Make sure there are normals which are required for lighting
            if ((m.VertexFormat & VertexFormats.Normal) == 0)
            {
                // Clone the mesh
                Mesh tempMesh = null;
                using(m)
                {
                    tempMesh = m.Clone(m.Options.Value, m.VertexFormat | VertexFormats.Normal,
                        device);
                    // Compute the normals
                    tempMesh.ComputeNormals();
                }
                m = tempMesh;
            }

            // Optimize the mesh for this graphics card's vertex cache 
            // so when rendering the mesh's triangle list the vertices will 
            // cache hit more often so it won't have to re-execute the vertex shader 
            // on those vertices so it will improve perf.     
            int[] adj = new int[m.NumberFaces * 3];
            m.GenerateAdjacency(1e-6f, adj);
            m.OptimizeInPlace(MeshFlags.OptimizeVertexCache, adj);

            // return the mesh
            return m;
        }

        /// <summary>Initializes the application</summary>
        public void InitializeApplication()
        {
            isUsingPreshader = true;

            for (int i = 0; i < MaxNumberLights; i++)
            {
                lightControl[i] = new DirectionWidget();
                lightControl[i].LightDirection = new Vector3((float)Math.Sin((float)Math.PI
                    * 2 * i / MaxNumberLights-(float)Math.PI/6), 0, -(float)Math.Cos((float)Math.PI
                    * 2 * i / MaxNumberLights-(float)Math.PI/6));
            }

            activeLight = 0;
            numberActiveLights = 1;
            lightScale = 1.0f;

            int y = 10;
            // Initialize the dialogs
            Button fullScreen = hud.AddButton(ToggleFullscreen,"Toggle full screen", 35, y, 125,22);
            Button toggleRef = hud.AddButton(ToggleReference,"Toggle reference (F3)", 35, y += 24, 125,22);
            Button changeDevice = hud.AddButton(ChangeDevice,"Change Device (F2)", 35, y += 24, 125,22);
            // Hook the button events for when these items are clicked
            fullScreen.Click += new EventHandler(OnFullscreenClicked);
            toggleRef.Click += new EventHandler(OnRefClicked);
            changeDevice.Click += new EventHandler(OnChangeDevicClicked);

            // Now add the sample specific UI
            y = 10;
            sampleUi.AddStatic(NumberLightsStatic, string.Format("# Lights: {0}", numberActiveLights), 35, y += 24, 125, 22);
            Slider lightSlider = sampleUi.AddSlider(NumberLights,50, y += 24, 100, 22, 1, MaxNumberLights, numberActiveLights, false);
            
            y += 24;
            sampleUi.AddStatic(LightScaleStatic, string.Format("Light scale: {0}", lightScale.ToString("f2", 
                System.Globalization.CultureInfo.CurrentUICulture)), 35, y += 24, 125, 22);
            Slider scaleSlider = sampleUi.AddSlider(LightScaleControl,50, y += 24, 100, 22, 0, 20, (int)(lightScale * 10.0f), false);
            
            y += 24;
            Button activeLightControl = sampleUi.AddButton(ActiveLightControl, "Change active light (K)", 35, y += 24, 125, 22,
                System.Windows.Forms.Keys.K, false);
            Checkbox preShader = sampleUi.AddCheckBox(EnablePreshader, "Enable preshaders", 35, y += 24, 125, 22, isUsingPreshader);

            // Hook the events
            preShader.Changed += new EventHandler(OnPreshaderClick);
            lightSlider.ValueChanged += new EventHandler(OnNumberLightsChanged);
            activeLightControl.Click += new EventHandler(OnActiveLightClick);
            scaleSlider.ValueChanged += new EventHandler(OnLightScaleChanged);
        }

        /// <summary>Called when the light scale has changed</summary>
        private void OnLightScaleChanged(object sender, EventArgs e)
        {
            Slider sl = sender as Slider;
            lightScale = (float)(sl.Value * 0.10f);

            StaticText text = sampleUi.GetStaticText(LightScaleStatic);
            text.SetText(string.Format("Light scale: {0}", lightScale.ToString("f2",
                System.Globalization.CultureInfo.CurrentUICulture)));
        }

        /// <summary>Called when active light button has been clicked</summary>
        private void OnActiveLightClick(object sender, EventArgs e)
        {
            if (!lightControl[activeLight].IsBeingDragged)
            {
                activeLight++;
                activeLight %= numberActiveLights;
            }
        }

        /// <summary>Called when the number of lights has changed</summary>
        private void OnNumberLightsChanged(object sender, EventArgs e)
        {
            if (!lightControl[activeLight].IsBeingDragged)
            {
                StaticText text = sampleUi.GetStaticText(NumberLightsStatic);
                Slider lights = sampleUi.GetSlider(NumberLights);

                numberActiveLights = lights.Value;

                text.SetText(string.Format("# Lights: {0}", numberActiveLights));

                activeLight %= numberActiveLights;
            }
        }

        /// <summary>Called when the pre-shader button is changed</summary>
        private void OnPreshaderClick(object sender, EventArgs e)
        {
            Checkbox cb = sender as Checkbox;
            isUsingPreshader = cb.IsChecked;

            if (sampleFramework.Device != null)
            {
                // Destroy and recreate the objects to update the preshader stuff
                OnLostDevice(null, EventArgs.Empty);
                OnDestroyDevice(null, EventArgs.Empty);
                OnCreateDevice(null, new DeviceEventArgs(sampleFramework.Device, sampleFramework.BackBufferSurfaceDescription));
                OnResetDevice(null, new DeviceEventArgs(sampleFramework.Device, sampleFramework.BackBufferSurfaceDescription));
            }
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
                BasicHLSL sample = new BasicHLSL(sampleFramework);
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
                    sampleFramework.CreateWindow("BasicHLSL");
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
