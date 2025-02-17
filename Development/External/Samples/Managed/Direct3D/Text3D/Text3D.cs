//-----------------------------------------------------------------------------
// File: Text3D.cs
//
// Desc: Example code showing how to do text in a Direct3D scene.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

// #define DEBUG_VS   // Uncomment this line to debug vertex shaders 
// #define DEBUG_PS   // Uncomment this line to debug pixel shaders 

using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace Text3DSample
{
    /// <summary>
    /// Class for rendering text in a 3d scene
    /// </summary>
    public class Text3D : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public Text3D(Framework f) 
        { 
            // Store framework
            sampleFramework = f; 
            // Create dialogs
            hud = new Dialog(sampleFramework); 
            sampleUi = new Dialog(sampleFramework); 
        }
        #endregion

        // Variables
        private Framework sampleFramework = null; // Framework for samples
        private Sprite textSprite = null; // Sprite for batching text calls
        private FirstPersonCamera camera = new FirstPersonCamera(); // A first person viewing camera
        private Dialog hud = null; // dialog for standard controls
        private Dialog sampleUi = null; // dialog for sample specific controls

        // Interop to call get device caps
        private const int LogPixelsY = 90;
        [System.Runtime.InteropServices.DllImport("gdi32.dll")]
        private static extern int GetDeviceCaps(IntPtr hdc, int cap);

        // Font scene variables
        private Mesh mesh3DText = null;  // Mesh to draw 3d text
        private Font firstFont = null;
        private Font secondFont = null;
        private Font statsFont = null;
        private Sprite batchSprite = null;
        private Material textMaterial;

        private Matrix objectOne = Matrix.Identity;
        private Matrix objectTwo = Matrix.Identity;

        private string fontName = "Arial";
        private const int FontSize = 15;
        private const int FontSize2 = 12;

        private System.Drawing.Font currentFont;

        private const string BigText = "This is a single call to Font.DrawText() with a large text buffer.  It shows that Font supports word wrapping. " +
            "If you resize the window, the words will automatically wrap to the next line.  It also supports\r\nhard line breaks.  Font also supports " +
            "Unicode text with correct word wrapping for right-to-left languages.";

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
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "Text3D");
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
            int height;
            int logPixels;
            // Initialize all of the fonts
            using(System.Drawing.Graphics g = sampleFramework.Window.CreateGraphics())
            {
                System.IntPtr dc = g.GetHdc();
                logPixels = GetDeviceCaps(dc, LogPixelsY);
                g.ReleaseHdc(dc);
            }

            height = -FontSize * logPixels / 72;

            firstFont = new Font(e.Device, height, 0, FontWeight.Bold,
                1, false, CharacterSet.Default, Precision.Default, FontQuality.Default, 
                PitchAndFamily.DefaultPitch | PitchAndFamily.FamilyDoNotCare, fontName);

            height = -FontSize2 * logPixels / 72;
            secondFont = new Font(e.Device, height, 0, FontWeight.Bold,
                0, false, CharacterSet.Default, Precision.Default, FontQuality.Default, 
                PitchAndFamily.DefaultPitch | PitchAndFamily.FamilyDoNotCare, fontName);

            // Initialize the stats font
            statsFont = ResourceCache.GetGlobalInstance().CreateFont(e.Device, 15, 0, FontWeight.Bold, 1, false, CharacterSet.Default,
                Precision.Default, FontQuality.Default, PitchAndFamily.FamilyDoNotCare | PitchAndFamily.DefaultPitch
                , "Arial");

            batchSprite = new Sprite(e.Device);

            // Store font
            currentFont = new System.Drawing.Font(fontName, FontSize);
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

            // Setup UI locations
            hud.SetLocation(desc.Width-170, 0);
            hud.SetSize(170,170);
            sampleUi.SetLocation(desc.Width - 170, desc.Height - 350);
            sampleUi.SetSize(170,300);

            // Restore the fonts / sprite
            firstFont.OnResetDevice();
            secondFont.OnResetDevice();
            batchSprite.OnResetDevice();

            // Restore the states
            e.Device.TextureState[0].ColorArgument1 = TextureArgument.TextureColor;
            e.Device.TextureState[0].ColorArgument2 = TextureArgument.Diffuse;
            e.Device.TextureState[0].ColorOperation = TextureOperation.Modulate;
            e.Device.SamplerState[0].MinFilter = TextureFilter.Linear;
            e.Device.SamplerState[0].MagFilter = TextureFilter.Linear;

            e.Device.RenderState.ZBufferEnable = true;
            e.Device.RenderState.DitherEnable = true;
            e.Device.RenderState.SpecularEnable = true;
            e.Device.RenderState.Lighting = true;
            e.Device.RenderState.AmbientColor = unchecked((int)0x80808080);

            // Create a light
            Vector3 lightDirection = new Vector3(10,-10,10);
            e.Device.Lights[0].Type = LightType.Directional;
            e.Device.Lights[0].DiffuseColor = new ColorValue(1.0f,1.0f,1.0f,1.0f);
            e.Device.Lights[0].Direction = Vector3.Normalize(lightDirection);
            e.Device.Lights[0].Enabled = true;

            // Set the transform matrices
            Vector3 eyeLocation = new Vector3(0.0f,-5.0f,-10.0f);
            Vector3 lookAtPoint = new Vector3(0.2f, 0.0f,  0.0f);
            e.Device.Transform.World = Matrix.Identity;

            camera.SetViewParameters(eyeLocation, lookAtPoint);
            float aspectRatio = (float)desc.Width / (float)desc.Height;
            camera.SetProjectionParameters((float)Math.PI / 4, aspectRatio, 1.0f, 1000.0f);

            if (mesh3DText != null)
                mesh3DText.Dispose();

            // Create our 3d text mesh
            mesh3DText = Mesh.TextFromFont(e.Device, currentFont, "This is calling Mesh.TextFromFont", 0.001f, 0.4f);
            // Create the material that will be used for the mesh
            textMaterial = new Material();
            textMaterial.AmbientColor = textMaterial.DiffuseColor = new ColorValue(0,16,180,255);
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
            if (firstFont != null)
                firstFont.OnLostDevice();
            if (secondFont != null)
                secondFont.OnLostDevice();
            if (batchSprite != null)
                batchSprite.OnLostDevice();

            if (mesh3DText != null)
            {
                mesh3DText.Dispose();
                mesh3DText = null;
            }
            if (textSprite != null)
            {
                textSprite.Dispose();
                textSprite = null;
            }
        }

        /// <summary>
        /// This event will be fired immediately after the Direct3D device has 
        /// been destroyed, which generally happens as a result of application termination or 
        /// windowed/full screen toggles. Resources created in the OnCreateDevice event 
        /// should be released here, which generally includes all Pool.Managed resources. 
        /// </summary>
        private void OnDestroyDevice(object sender, EventArgs e)
        {
            if (firstFont != null)
            {
                firstFont.Dispose();
                firstFont = null;
            }
            if (secondFont != null)
            {
                secondFont.Dispose();
                secondFont = null;
            }
            if (batchSprite != null)
            {
                batchSprite.Dispose();
                batchSprite = null;
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
            // Setup five rotation matrices (for rotating text strings)
            Vector3 vAxis1 = new Vector3(1.0f,2.0f,0.0f);
            Vector3 vAxis2 = new Vector3(1.0f,0.0f,0.0f);
            objectOne = Matrix.RotationAxis(vAxis1, (float)appTime / 2.0f);
            objectTwo = Matrix.RotationAxis(vAxis2, (float)appTime);

            objectTwo *= Matrix.Scaling(0.5f, 0.5f, 0.5f);

            // Add some translational values to the matrices
            objectOne.M41 = 1.0f;   objectOne.M42 = 6.0f;   objectOne.M43 = 20.0f; 
            objectTwo.M41 = -4.0f;  objectTwo.M42 = -1.0f;  objectTwo.M43 = 0.0f; 
        }

        /// <summary>
        /// This callback function will be called at the end of every frame to perform all the 
        /// rendering calls for the scene, and it will also be called if the window needs to be 
        /// repainted. After this function has returned, the sample framework will call 
        /// Device.Present to display the contents of the next buffer in the swap chain
        /// </summary>
        public void OnFrameRender(Device device, double appTime, float elapsedTime)
        {
            // Get the view & projection matrix from camera.
            // User can't control camera for this simple sample
            device.Transform.View = camera.ViewMatrix;
            device.Transform.Projection = camera.ProjectionMatrix;
            bool beginSceneCalled = false;

            // Clear the render target and the zbuffer 
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, 0, 1.0f, 0);
            try
            {
                device.BeginScene();
                beginSceneCalled = true;

                int height = sampleFramework.PresentParameters.BackBufferHeight;
                System.Drawing.Rectangle rect;
                // Demonstration 1
                // Draw a simple line using DrawText
#if (true)
                // Pass in DrawTextFormat.NoClip so we don't have to calc the bottom/right of the rect
                firstFont.DrawText(null, "This is a trivial call to Font.DrawText", new System.Drawing.Rectangle(150,200, 0, 0), 
                    DrawTextFormat.NoClip, System.Drawing.Color.Red);
#else
                // If you wanted to calc the bottom/rect of the rect make these 2 calls
                rect = new System.Drawing.Rectangle(150,200, 0, 0);
                firstFont.DrawText(null, "This is a trivial call to DrawText", ref rect, 
                    DrawTextFormat.CalculateRect, System.Drawing.Color.Red);
                firstFont.DrawText(null, "This is a trivial call to Font.DrawText", rect, 
                    DrawTextFormat.None, System.Drawing.Color.Red);
#endif
    
                // Demonstration 2
                // Allow multiple draw calls to sort by texture changes by Sprite
                // When drawing 2D text use flags: SpriteFlags.AlphaBlend | SpriteFlags.SortTexture
                // When drawing 3D text use flags: SpriteFlags.AlphaBlend | SpriteFlags.SortDepthBackToFront
                
                batchSprite.Begin(SpriteFlags.AlphaBlend | SpriteFlags.SortTexture);
                rect = new System.Drawing.Rectangle(10, height - 15 * 6, 0, 0);
                secondFont.DrawText(batchSprite, "These multiple calls to Font.DrawText() using the same Sprite.", 
                    rect, DrawTextFormat.NoClip, unchecked((int)0xffffffff));
                rect = new System.Drawing.Rectangle(10, height - 15 * 5, 0, 0);
                secondFont.DrawText(batchSprite, "Font now caches letters on one or more textures.", 
                    rect, DrawTextFormat.NoClip, unchecked((int)0xffffffff));
                rect = new System.Drawing.Rectangle(10, height - 15 * 4, 0, 0);
                secondFont.DrawText(batchSprite, "In order to sort by texture state changes on multiple", 
                    rect, DrawTextFormat.NoClip, unchecked((int)0xffffffff));
                rect = new System.Drawing.Rectangle(10, height - 15 * 3, 0, 0);
                secondFont.DrawText(batchSprite, "draw calls to DrawText() pass a Sprite and use", 
                    rect, DrawTextFormat.NoClip, unchecked((int)0xffffffff));
                rect = new System.Drawing.Rectangle(10, height - 15 * 2, 0, 0);
                secondFont.DrawText(batchSprite, "flags SpriteFlags.AlphaBlend | SpriteFlags.SortTexture", 
                    rect, DrawTextFormat.NoClip, unchecked((int)0xffffffff));
                batchSprite.End();
    
                // Demonstration 3:
                // Word wrapping and unicode text.  
                // Note that not all fonts support dynamic font linking. 
                System.Drawing.Rectangle rc = new System.Drawing.Rectangle(10, 60, 
                    sampleFramework.PresentParameters.BackBufferWidth - 150, 
                    sampleFramework.PresentParameters.BackBufferHeight - 10);

                secondFont.DrawText(null, BigText, rc, 
                    DrawTextFormat.Left | DrawTextFormat.WordBreak | DrawTextFormat.ExpandTabs,
                    System.Drawing.Color.CornflowerBlue);


                // Draw D3DXFont mesh in 3D (blue)
                if (mesh3DText != null)
                {
                    device.Material = textMaterial;
                    device.Transform.World = objectTwo;
                    mesh3DText.DrawSubset(0);
                }

                // Show frame rate
                statsFont.DrawText(null, sampleFramework.FrameStats, new System.Drawing.Rectangle(2,0,0,0),
                    DrawTextFormat.NoClip, unchecked((int)0xffffff00));
                statsFont.DrawText(null, sampleFramework.DeviceStats, new System.Drawing.Rectangle(2,15,0,0),
                    DrawTextFormat.NoClip, unchecked((int)0xffffff00));

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
        /// Before handling window messages, the sample framework passes incoming windows 
        /// messages to the application through this callback function. If the application sets 
        /// noFurtherProcessing to true, the sample framework will not process the message
        /// </summary>
        public IntPtr OnMsgProc(IntPtr hWnd, NativeMethods.WindowMessage msg, IntPtr wParam, IntPtr lParam, ref bool noFurtherProcessing)
        {
            // Give the dialog a chance to handle the message first
            if (!hud.MessageProc(hWnd, msg, wParam, lParam))
            {
                if (!sampleUi.MessageProc(hWnd, msg, wParam, lParam))
                {
                    // Pass all remaining windows messages to camera so it can respond to user input
                    camera.HandleMessages(hWnd, msg, wParam, lParam);
                }
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
            changeDevice.Click += new EventHandler(OnChangeDeviceClicked);

            // Now add the sample specific UI
            y = 10;
            Button changeFont = sampleUi.AddButton(5, "Change Font", 35, y += 24, 125, 22);
            changeFont.Click += new EventHandler(OnChangeFont);
            
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

        /// <summary>Called when the change font button is clicked</summary>
        private void OnChangeFont(object sender, EventArgs e)
        {
            System.Windows.Forms.FontDialog dlg = new System.Windows.Forms.FontDialog();

            // Show the dialog
            dlg.FontMustExist = true;
            dlg.Font = currentFont;

            // Make the framework go back to windowed if it isn't
            if (!sampleFramework.IsWindowed)
                sampleFramework.ToggleFullscreen();

            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK) // We selected something
            {
                currentFont = dlg.Font;
                if (firstFont != null)
                    firstFont.Dispose();

                // Set the new font
                firstFont = new Font(sampleFramework.Device, currentFont);

                if (mesh3DText != null)
                    mesh3DText.Dispose();

                // Create our 3d text mesh
                mesh3DText = Mesh.TextFromFont(sampleFramework.Device, currentFont
                    , "This is calling Mesh.TextFromFont", 0.001f, 0.4f);
            }
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
                Text3D sample = new Text3D(sampleFramework);
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
                    sampleFramework.CreateWindow("Text3D");
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
