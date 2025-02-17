//--------------------------------------------------------------------------------------
// File: HLSLwithoutEffects.cs
//
// Desc: Sample showing a simple vertex shader in D3DX High Level Shader
//       Language (HLSL) without using an Effect object.  Not using the 
//       Effect object is a more difficult method of using HLSL.  See
//       the BasicHLSL sample for a simpler method of using HLSL.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// #define DEBUG_VS   // Uncomment this line to debug vertex shaders 
// #define DEBUG_PS   // Uncomment this line to debug pixel shaders 

using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace HLSLWithoutEffectsSample
{
    /// <summary>
    /// Class for using HLSL without the Effects framework
    /// </summary>
    public class HlslWithoutEffects : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public HlslWithoutEffects(Framework f) 
        { 
            // Store framework
            sampleFramework = f; 
            // Create dialogs
            hud = new Dialog(sampleFramework); 
        }
        #endregion

        // Constants
        private const int VerticesPerEdge = 64;
        private const int NumberVertices = VerticesPerEdge  * VerticesPerEdge;
        private const int NumberIndices = 6 * (VerticesPerEdge - 1) * (VerticesPerEdge - 1);
        private static readonly int ClearColor = System.Drawing.Color.FromArgb(45,50,170).ToArgb();

        // Variables
        private Framework sampleFramework = null; // Framework for samples
        private Font drawingFont = null; // Font for drawing text
        private Sprite textSprite = null; // Sprite for batching text calls
        private bool isHelpShowing = true; // If true, show the help ui
        private ModelViewerCamera camera = new ModelViewerCamera(); // A model viewing camera
        private Dialog hud = null; // dialog for standard controls
        private EffectHandle worldViewHandle = null; // Effect Handle for the 'world view' matrix
        private EffectHandle timeHandle = null; // Effect Handle for the time variable

        // Scene variables
        private VertexBuffer vb = null;
        private IndexBuffer ib = null;
        private VertexShader shader = null;
        private ConstantTable constantTable = null;
        private VertexDeclaration vertexDecl = null;

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
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "HLSLWithoutEffects");
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
            // Initialize the font
            drawingFont = ResourceCache.GetGlobalInstance().CreateFont(e.Device, 15, 0, FontWeight.Bold, 1, false, CharacterSet.Default,
                Precision.Default, FontQuality.Default, PitchAndFamily.FamilyDoNotCare | PitchAndFamily.DefaultPitch
                , "Arial");

            // Create the vertex shader and declaration
            VertexElement[] elements = new VertexElement[]
                {
                    new VertexElement(0, 0, DeclarationType.Float2, DeclarationMethod.Default, DeclarationUsage.Position, 0),
                    VertexElement.VertexDeclarationEnd
                };

            vertexDecl = new VertexDeclaration(e.Device, elements);

            // Find the shader file
            string path = Utility.FindMediaFile("HLSLwithoutEffects.vsh");

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
            ShaderFlags shaderFlags = 0;
#if(DEBUG_VS)
            shaderFlags |= ShaderFlags.ForceVertexShaderSoftwareNoOptimizations;
#endif
#if(DEBUG_PS)
            shaderFlags |= ShaderFlags.ForcePixelShaderSoftwareNoOptimizations;
#endif

            string errors;

            using(GraphicsStream code = ShaderLoader.CompileShaderFromFile(path, "Ripple", null, null,
                      "vs_1_1", shaderFlags, out errors, out constantTable))
            {

                // We will store these constants in an effect handle here for performance reasons.
                // You could simply use the string value (i.e., "worldViewProj") in the SetValue call
                // and it would work just as well, but that actually requires an allocation to be made
                // and can actually slow your performance down.  It's much more efficient to simply
                // cache these handles for use later
                worldViewHandle = constantTable.GetConstant(null, "worldViewProj");
                timeHandle = constantTable.GetConstant(null, "appTime");
                
                // Create the shader
                shader = new VertexShader(e.Device, code);
            }

            // Setup the camera's view parameters
            camera.SetViewQuat(new Quaternion(-0.275f, 0.3f, 0.0f, 0.7f));
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

            // Setup render states
            e.Device.RenderState.Lighting = false;
            e.Device.RenderState.CullMode = Cull.None;

            // Create and initialize index buffer
            ib = new IndexBuffer(typeof(short), NumberIndices, e.Device, Usage.None, Pool.Default);
            GraphicsStream data = ib.Lock(0, 0, LockFlags.None);
            for(int y = 1; y < VerticesPerEdge; y++)
            {
                for (int x = 1; x < VerticesPerEdge; x++)
                {
                    data.Write((short)( (y-1) * VerticesPerEdge + (x-1) ));
                    data.Write((short)( (y-0) * VerticesPerEdge + (x-1) ));
                    data.Write((short)( (y-1) * VerticesPerEdge + (x-0) ));

                    data.Write((short)( (y-1) * VerticesPerEdge + (x-0) ));
                    data.Write((short)( (y-0) * VerticesPerEdge + (x-1) ));
                    data.Write((short)( (y-0) * VerticesPerEdge + (x-0) ));
                }
            }
            ib.Unlock();

            // Create and initialize vertex buffer
            vb = new VertexBuffer(typeof(Vector2), NumberVertices, e.Device, Usage.None, VertexFormats.None, Pool.Default);
            data = vb.Lock(0, 0, LockFlags.None);
            for(int y = 0; y < VerticesPerEdge; y++)
            {
                for (int x = 0; x < VerticesPerEdge; x++)
                {
                    data.Write(new Vector2( ((float)x / (float)(VerticesPerEdge-1) - 0.5f) * (float)Math.PI,
                        ((float)y / (float)(VerticesPerEdge-1) - 0.5f) * (float)Math.PI ));
                }
            }
            vb.Unlock();

            // Setup the camera's projection parameters
            float aspectRatio = (float)desc.Width / (float)desc.Height;
            camera.SetProjectionParameters((float)Math.PI/4.0f, aspectRatio, 0.1f, 1000.0f);
            camera.SetWindow(desc.Width, desc.Height);
            camera.MaximumRadius = 30.0f;

            // Resize the hud
            hud.SetLocation(desc.Width - 170, 0);
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
            if (vb != null)
                vb.Dispose();
            if (ib != null)
                ib.Dispose();
            if (textSprite != null)
                textSprite.Dispose();
        }

        /// <summary>
        /// This event will be fired immediately after the Direct3D device has 
        /// been destroyed, which generally happens as a result of application termination or 
        /// windowed/full screen toggles. Resources created in the OnCreateDevice event 
        /// should be released here, which generally includes all Pool.Managed resources. 
        /// </summary>
        private void OnDestroyDevice(object sender, EventArgs e)
        {
            if (shader != null)
                shader.Dispose();
            if (constantTable != null)
                constantTable.Dispose();
            if (vertexDecl != null)
                vertexDecl.Dispose();
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

            // Setup vertex shader constants
            Matrix worldViewProj, world, view, proj;
            world = camera.WorldMatrix;
            view = camera.ViewMatrix;
            proj = camera.ProjectionMatrix;
            worldViewProj = world * view * proj;

            constantTable.SetValue(device, worldViewHandle, worldViewProj);
            constantTable.SetValue(device, timeHandle, (float)appTime);
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
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, ClearColor, 1.0f, 0);
            try
            {
                device.BeginScene();
                beginSceneCalled = true;
            
                device.VertexDeclaration = vertexDecl;
                device.VertexShader = shader;
                device.SetStreamSource(0, vb, 0);
                device.Indices = ib;

                device.DrawIndexedPrimitives(PrimitiveType.TriangleList, 0, 0, NumberVertices, 0, NumberIndices / 3);

                RenderText();
                // Render the HUD
                hud.OnRender(elapsedTime);
            }
            finally
            {
                if (beginSceneCalled)
                    device.EndScene();
            }
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
        /// Render the help and statistics text. This function uses the Font object for 
        /// efficient text rendering.
        /// </summary>
        private void RenderText()
        {
            TextHelper txtHelper = new TextHelper(drawingFont, textSprite, 15);

            // Output statistics
            txtHelper.Begin();
            txtHelper.SetInsertionPoint(5,5);
            txtHelper.SetForegroundColor(new ColorValue(1.0f, 1.0f, 0.0f, 1.0f).ToArgb());
            txtHelper.DrawTextLine(sampleFramework.FrameStats);
            txtHelper.DrawTextLine(sampleFramework.DeviceStats);

            // Draw help
            if (isHelpShowing)
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*6);
                txtHelper.SetForegroundColor(new ColorValue(1.0f, 0.75f, 0.0f, 1.0f).ToArgb());
                txtHelper.DrawTextLine("Controls (F1 to hide):");

                txtHelper.SetInsertionPoint(40, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
                txtHelper.DrawTextLine("Rotate model: Left mouse button");
                txtHelper.DrawTextLine("Rotate camera: Right mouse button");
                txtHelper.DrawTextLine("Zoom camera: Mouse wheel scroll");
                txtHelper.DrawTextLine("Hide help: F1");
            }
            else
            {
                txtHelper.SetForegroundColor(new ColorValue(1.0f, 1.0f, 1.0f, 1.0f).ToArgb());
                txtHelper.DrawTextLine("Press F1 for help");
            }

            txtHelper.End();
        }

        /// <summary>
        /// Before handling window messages, the sample framework passes incoming windows 
        /// messages to the application through this callback function. If the application sets 
        /// noFurtherProcessing to true, the sample framework will not process the message
        /// </summary>
        public IntPtr OnMsgProc(IntPtr hWnd, NativeMethods.WindowMessage msg, IntPtr wParam, IntPtr lParam, ref bool noFurtherProcessing)
        {
            // Give the dialog a chance to handle the message first
            if (!hud.MessageProc(hWnd, msg, wParam, lParam))
            {
                // Pass all remaining windows messages to camera so it can respond to user input
                camera.HandleMessages(hWnd, msg, wParam, lParam);
            }
            return IntPtr.Zero;
        }

        /// <summary>
        /// Adds the guid stuff to the application
        /// </summary>
        public void InitializeApplication()
        {
            int y = 10;
            // Create the buttons
            Button fullScreen = hud.AddButton(2,"Toggle full screen", 35, y, 125,22);
            Button toggleRef = hud.AddButton(3,"Toggle reference (F3)", 35, y += 24, 125,22);
            Button changeDevice = hud.AddButton(4,"Change Device (F2)", 35, y += 24, 125,22);
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
                HlslWithoutEffects sample = new HlslWithoutEffects(sampleFramework);
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
                    sampleFramework.CreateWindow("HlslWithoutEffects");
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
