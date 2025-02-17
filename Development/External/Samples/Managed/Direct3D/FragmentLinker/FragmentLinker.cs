//-----------------------------------------------------------------------------
// File: FragmentLinker.cs
//
// AboutMe please ...
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace FragmentLinkerSample
{
    /// <summary>FragmentLinker Sample Class</summary>
    public class FragmentLinkerClass : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public FragmentLinkerClass(Framework f) 
        { 
            // Store framework
            sampleFramework = f; 
            // Create dialogs
            hud = new Dialog(sampleFramework); 
            sampleUI = new Dialog(sampleFramework); 
        }
        #endregion

        // Variables
        private Framework sampleFramework = null; // Framework for samples
        private Font statsFont = null; // Font for drawing text
        private Sprite textSprite = null; // Sprite for batching text calls
        private ModelViewerCamera camera = new ModelViewerCamera();  // A model viewing camera
        private bool isHelpShowing = true; // If true, renders the UI help text
        private Dialog hud = null; // dialog for standard controls
        private Dialog sampleUI = null; // dialog for sample specific controls

        // Sample specific variables
        private FrameworkMesh mesh = null; // Mesh Object to be rendered
        private Vector3 objectCenter; // Center of bounding sphere of object
        private float objectRadius; // Radius of bounding sphere of object
        private Matrix worldCenter; // World matrix to center the object
        private FragmentLinker fragmentLinker = null; // Fragment linker interface
        private GraphicsStream compiledFragments  = null;   // Buffer containing compiled fragments
        private PixelShader pixelShader = null; // Pixel shader to be used
        private VertexShader vertexShader = null; // Vertex shader to be used; linked by LinkVertexShader
        private ConstantTable constTable = null; // shaders constant table

        // Global variables used by the vertex shader
        private static readonly Vector4 MaterialAmbient = new Vector4(0.3f, 0.3f, 0.3f, 1.0f);
        private static readonly Vector4 MaterialDiffuse = new Vector4(0.6f, 0.6f, 0.6f, 1.0f);
        private static readonly Vector4 LightColor = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        private static readonly Vector4 LightPosition = new Vector4(0.0f, 5.0f, -5.0f, 1.0f);

        // HUD UI Control constants
        private const int Static = -1;
        private const int ToggleFullscreen =  1;
        private const int ToggleReference =  3;
        private const int ChangeDevice =  4;
        private const int Animation =  5;
        private const int Lighting =  6;

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

            // This application is designed to work on a pure device by not using 
            // any get methods, so create a pure device if supported and using HWVP.
            if ( (caps.DeviceCaps.SupportsPureDevice) && 
                ((settings.BehaviorFlags & CreateFlags.HardwareVertexProcessing) != 0 ) )
                settings.BehaviorFlags |= CreateFlags.PureDevice;

            // For the first device created if its a REF device, optionally display a warning dialog box
            if (settings.DeviceType == DeviceType.Reference)
            {
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "Fragment Linker");
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
            statsFont = ResourceCache.GetGlobalInstance().CreateFont(e.Device, 15, 0, FontWeight.Bold, 1, false, CharacterSet.Default, Precision.Default, FontQuality.Default, PitchAndFamily.FamilyDoNotCare | PitchAndFamily.DefaultPitch, "Arial");

            // Read the D3DX effect file
            // If this fails, there should be debug output as to 
            // they the .fx file failed to compile
            string path = Utility.FindMediaFile("FragmentLinker.fx");
            using (GraphicsStream code = ShaderLoader.CompileShaderFromFile(path, "ModulateTexture", null, null, "ps_1_1", ShaderFlags.None))
            {
                // Create the pixel shader
                pixelShader = new PixelShader(e.Device, code);
                // Set it to the device
                e.Device.PixelShader = pixelShader;
            }

            // Load the mesh
            path = Utility.FindMediaFile("Dwarf\\dwarf.x");

            // Change the current directory to the mesh's directory so we can
            // find the textures.
            System.IO.FileInfo info = new System.IO.FileInfo(path);
            System.IO.Directory.SetCurrentDirectory(info.Directory.FullName);

            mesh = new FrameworkMesh(e.Device, "dwarf.x");

            // Find the mesh's center, then generate a centering matrix.
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

            // Create the fragment linker interface
            fragmentLinker = new FragmentLinker(e.Device, 0);

            // Compile the fragments to a buffer. The fragments must be linked together to form
            // a shader before they can be used for rendering.
            path = Utility.FindMediaFile("FragmentLinker.fx");
            compiledFragments = Microsoft.DirectX.Direct3D.FragmentLinker.GatherFragmentsFromFile(path, null, null, ShaderFlags.None);

            // Build the list of compiled fragments
            fragmentLinker.AddFragments(compiledFragments);

            // Store the fragment handles
            ComboBox cb1 = sampleUI.GetComboBox(Lighting);
            cb1.Clear();
            cb1.AddItem("Ambient", fragmentLinker.GetFragmentHandle("AmbientFragment"));
            cb1.AddItem("Ambient & Diffuse", fragmentLinker.GetFragmentHandle("AmbientDiffuseFragment"));

            ComboBox cb2 = sampleUI.GetComboBox(Animation);
            cb2.Clear();
            cb2.AddItem("On" , fragmentLinker.GetFragmentHandle("ProjectionFragment_Animated"));
            cb2.AddItem("Off", fragmentLinker.GetFragmentHandle("ProjectionFragment_Static"));

            // Link the desired fragments to create the vertex shader
            LinkVertexShader();

            // Setup the camera's view parameters
            camera.SetViewParameters(new Vector3(3.0f, 0.0f, -3.0f), Vector3.Empty);
        }
        
        /// <summary>
        /// Link together compiled fragments to create a vertex shader. The list of fragments
        /// used is determined by the currently selected UI options.
        /// </summary>
        private void LinkVertexShader()
        {
            const int NumberFragments = 2;
            Device device = sampleFramework.Device;
            EffectHandle[] aHandles = new EffectHandle[NumberFragments];
            
            aHandles[0] = sampleUI.GetComboBox(Animation).GetSelectedData() as EffectHandle;
            aHandles[1] = sampleUI.GetComboBox(Lighting).GetSelectedData() as EffectHandle;

            if (vertexShader != null)
            {
                vertexShader.Dispose();
            }

            string errors;
            using (GraphicsStream code = fragmentLinker.LinkShader("vs_1_1", ShaderFlags.None, aHandles, out errors))
            {
                vertexShader = new VertexShader(device, code);
                constTable = ConstantTable.FromShader(code);

            }
            // Set the global variables
            device.VertexShader = vertexShader;
            if (constTable != null)
            {
                DirectXException.IgnoreExceptions(); // We ignore exceptions because not all constants may be defined depending on which fragments are being used.
                constTable.SetValue(device, "g_vMaterialAmbient", MaterialAmbient);
                constTable.SetValue(device, "g_vMaterialDiffuse", MaterialDiffuse);
                constTable.SetValue(device, "g_vLightColor", LightColor);
                constTable.SetValue(device, "g_vLightPosition",LightPosition);
                DirectXException.EnableExceptions();
            }

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

            // Setup the camera's projection parameters
            float aspectRatio = (float)desc.Width / (float)desc.Height;
            camera.SetProjectionParameters((float)Math.PI / 4, aspectRatio, 0.1f, 1000.0f);
            camera.SetWindow(desc.Width, desc.Height);

            // Setup UI locations
            hud.SetLocation(desc.Width-170, 0);
            hud.SetSize(170,170);
            sampleUI.SetLocation(desc.Width - 170, desc.Height - 350);
            sampleUI.SetSize(170,300);

            // Make sure to link the vertex shader during reset
            LinkVertexShader();
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
            if(mesh != null)
            {
                mesh.Dispose();
                mesh = null;
            }
            if(vertexShader != null)
            {
                vertexShader.Dispose();
                vertexShader = null;
            }
            if(pixelShader != null)
            {
                pixelShader.Dispose();
                pixelShader = null;
            }
            if(fragmentLinker != null)
            {
                fragmentLinker.Dispose();
                fragmentLinker = null;
            }
            if(compiledFragments != null)
            {
                compiledFragments.Dispose();
                compiledFragments = null;
            }
        }

        /// <summary>
        /// This callback function will be called once at the beginning of every frame. This is the
        /// best location for your application to handle updates to the scene, but is not 
        /// intended to contain actual rendering calls, which should instead be placed in the 
        /// OnFrameRender callback.  
        /// </summary>
        public void OnFrameMove(Device device, double appTime, float elapsedTime)
        {
            Matrix worldMatrix;
            Matrix worldViewProjectionMatrix;

            // Update the camera's position based on user input 
            camera.FrameMove(elapsedTime);

            // Get the projection & view matrix from the camera class
            worldMatrix = worldCenter * camera.WorldMatrix;
            worldViewProjectionMatrix = worldMatrix * camera.ViewMatrix * camera.ProjectionMatrix;

            // Update the effect's variables.  Instead of using strings, it would 
            // be more efficient to cache a handle to the parameter by calling 
            // Effect.GetConstantByName

            // Ignore return codes because not all variables might be present in 
            // the constant table depending on which fragments were linked.
            DirectXException.IgnoreExceptions();
            if (constTable != null)
            {
                constTable.SetValue(device, "g_mWorldViewProjection", worldViewProjectionMatrix);
                constTable.SetValue(device, "g_mWorld", worldMatrix);
                constTable.SetValue(device, "g_fTime", (float)appTime);     
            }
            DirectXException.EnableExceptions();
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

                device.SamplerState[0].MipFilter = TextureFilter.Linear;
                device.SamplerState[0].MinFilter = TextureFilter.Linear;
                device.SamplerState[0].MagFilter = TextureFilter.Linear;

                // Render the mesh with the applied technique
                mesh.Render(device);

                // Show frame rate
                RenderText();

                // Show UI
                hud.OnRender(elapsedTime);
                sampleUI.OnRender(elapsedTime);
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
            // The helper object simply helps keep track of text position, and color
            // and then it calls textFont.DrawText(textSprite, text, rect, DrawTextFormat.NoClip, color);
            // If null is passed in as the sprite object, then it will work however the 
            // textFont.DrawText() will not be batched together.  Batching calls improves performance.
            TextHelper txtHelper = new TextHelper(statsFont, textSprite, 15);

            // Output statistics
            txtHelper.Begin();
            txtHelper.SetInsertionPoint(5,5);
            txtHelper.SetForegroundColor(System.Drawing.Color.Yellow);
            txtHelper.DrawTextLine(sampleFramework.FrameStats);
            txtHelper.DrawTextLine(sampleFramework.DeviceStats);

            txtHelper.SetForegroundColor(System.Drawing.Color.White);
            txtHelper.DrawTextLine("Selected fragments are linked on-the-fly to create the current shader" );

            // Draw help
            if (isHelpShowing)
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*6);
                txtHelper.SetForegroundColor(System.Drawing.Color.DarkOrange);
                txtHelper.DrawTextLine("Controls (F1 to hide):");

                txtHelper.SetInsertionPoint(20, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
                txtHelper.DrawTextLine( "Rotate model: Left mouse button\n" + 
                                        "Rotate camera: Right mouse button\n" + 
                                        "Zoom camera: Mouse wheel scroll\n" );

                txtHelper.SetInsertionPoint(250, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
                txtHelper.DrawTextLine( "Hide help: F1\n" + 
                                        "Quit: ESC\n" );
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
        /// messages, which are still passed to the application's OnMsgProc callback.
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

            noFurtherProcessing = sampleUI.MessageProc(hWnd, msg, wParam, lParam);
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
            changeDevice.Click += new EventHandler(OnChangeDeviceClicked);

            // Now add the sample specific UI
            y = 10;
            // Title font for comboboxes
            sampleUI.SetFont(1, "Arial", 14, FontWeight.Bold );
            Element element = sampleUI.GetDefaultElement(ControlType.StaticText, 0 );
            element.FontIndex = 1;
            element.textFormat = DrawTextFormat.Left | DrawTextFormat.Bottom;

            sampleUI.AddStatic(Static, "(V)ertex Animation", 20, 0, 105, 25 );
            ComboBox cb1 = sampleUI.AddComboBox(Animation, 20, 25, 140, 24, System.Windows.Forms.Keys.V, false);
            cb1.SetDropHeight(30);

            sampleUI.AddStatic(Static, "(L)ighting", 20, 50, 105, 25 );
            ComboBox cb2 = sampleUI.AddComboBox(Lighting, 20, 75, 140, 24, System.Windows.Forms.Keys.L, false);
            cb2.SetDropHeight(30);

            cb1.Changed += new EventHandler(OnVertexAnimationORLightingChanged);
            cb2.Changed += new EventHandler(OnVertexAnimationORLightingChanged);
        }

        /// <summary>Called when the change device button is clicked</summary>
        private void OnChangeDeviceClicked(object sender, EventArgs e)
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
                FragmentLinkerClass sample = new FragmentLinkerClass(sampleFramework);
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
                    sampleFramework.CreateWindow("FragmentLinker");
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

        /// <summary>Fired when the vertex animation or lighting has changed</summary>
        private void OnVertexAnimationORLightingChanged(object sender, EventArgs e)
        {
            LinkVertexShader();
        }
    }
}
