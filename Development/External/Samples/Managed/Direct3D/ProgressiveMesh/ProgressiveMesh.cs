//-----------------------------------------------------------------------------
// File: ProgressiveMesh.cs
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

namespace ProgressiveMeshSample
{
    /// <summary>ProgressiveMesh Sample Class</summary>
    public class ProgressiveMeshes : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public ProgressiveMeshes(Framework f) 
        { 
            // Store framework
            sampleFramework = f; 
            // Create dialogs
            hud = new Dialog(sampleFramework); 
            sampleUi = new Dialog(sampleFramework); 
        }
        #endregion

        private const string OptimizedText = "Using optimized mesh {0} of {1}\nCurrent mesh vertices range: {2} / {3}\n"  +
            "Absolute vertices range: {4} / {5}\nCurrent vertices: {6}";
        private const string UnoptimizedText = "Using unoptimized mesh\nMesh vertices range: {0} / {1}\n"  +
            "Current vertices: {2}\n";

        // Variables
        private Framework sampleFramework = null; // Framework for samples
        private Font statsFont = null; // Font for drawing text
        private Sprite textSprite = null; // Sprite for batching text calls
        private Effect effect = null; // D3DX Effect Interface
        private ModelViewerCamera camera = new ModelViewerCamera(); // A model viewing camera
        private bool isHelpShowing = true; // If true, renders the UI help text
        private Dialog hud = null; // dialog for standard controls
        private Dialog sampleUi = null; // dialog for sample specific controls

        // Sample specific variables
        private ProgressiveMesh[] meshes = null;
        private ProgressiveMesh fullMesh = null;
        private int currentMeshIndex = 0;
        private Material[] meshMaterials = null;
        private Texture[] meshTextures = null;
        private Vector3 objectCenter; // Center of bounding sphere of object
        private float objectRadius; // Radius of bounding sphere of object
        private Matrix worldCenter; // World matrix to center the mesh
        private bool isShowingOptimized = true;

        // HUD Ui Control constants
        private const int ToggleFullscreen = 1;
        private const int ToggleReference = 3;
        private const int ChangeDevice = 4;
        private const int Detail = 5;
        private const int DetailLabel = 6;
        private const int UseOptimizedCheckBox = 7;

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
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "ProgressiveMesh");
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
            string path = Utility.FindMediaFile("ProgressiveMesh.fx");
            effect = ResourceCache.GetGlobalInstance().CreateEffectFromFile(e.Device,
                path, null, null, shaderFlags, null);

            // Set the technique now, it will never be updated
            effect.Technique = "RenderScene";

            // Load the mesh
            GraphicsStream adjacencyBuffer = null;
            ExtendedMaterial[] materials = null;

            // Find the mesh
            path = Utility.FindMediaFile("dwarf\\dwarf.x");
            
            // Change the current directory to the mesh's directory so we can
            // find the textures.
            string currentFolder = System.IO.Directory.GetCurrentDirectory();
            System.IO.FileInfo info = new System.IO.FileInfo(path);
            System.IO.Directory.SetCurrentDirectory(info.Directory.FullName);

            using (Mesh originalMesh = Mesh.FromFile(path, MeshFlags.Managed, e.Device,
                       out adjacencyBuffer, out materials))
            {
                int use32Bit = (int)(originalMesh.Options.Value & MeshFlags.Use32Bit);

                // Perform simple cleansing operations on mesh
                using (Mesh mesh = Mesh.Clean(CleanType.Simplification, originalMesh, adjacencyBuffer, adjacencyBuffer))
                {
                    // Perform a weld to try and remove excess vertices.
                    // Weld the mesh using all epsilons of 0.0f.  A small epsilon like 1e-6 works well too
                    WeldEpsilons epsilons = new WeldEpsilons();
                    mesh.WeldVertices(0, epsilons, adjacencyBuffer, adjacencyBuffer);

                    // Verify validity of mesh for simplification
                    mesh.Validate(adjacencyBuffer);

                    // Allocate a material/texture arrays
                    meshMaterials = new Material[materials.Length];
                    meshTextures = new Texture[materials.Length];

                    // Copy the materials and load the textures
                    for(int i = 0; i < meshMaterials.Length; i++)
                    {
                        meshMaterials[i] = materials[i].Material3D;
                        meshMaterials[i].AmbientColor = meshMaterials[i].DiffuseColor;

                        if ( (materials[i].TextureFilename != null) && (materials[i].TextureFilename.Length > 0) )
                        {
                            // Create the texture
                            meshTextures[i] = ResourceCache.GetGlobalInstance().CreateTextureFromFile(e.Device, materials[i].TextureFilename);
                        }
                    }

                    // Find the mesh's center, then generate a centering matrix
                    using(VertexBuffer vb = mesh.VertexBuffer)
                    {
                        using (GraphicsStream stm = vb.Lock(0, 0, LockFlags.NoSystemLock))
                        {
                            try
                            {
                                objectRadius = Geometry.ComputeBoundingSphere(stm,
                                    mesh.NumberVertices, mesh.VertexFormat, out objectCenter);

                                worldCenter = Matrix.Translation(-objectCenter);
                                float scaleFactor = 2.0f / objectRadius;
                                worldCenter *= Matrix.Scaling(scaleFactor, scaleFactor, scaleFactor);
                            }
                            finally
                            {
                                vb.Unlock();
                            }
                        }
                    }

                    // If the mesh is missing normals, generate them.
                    Mesh currentMesh = mesh;
                    if ((mesh.VertexFormat & VertexFormats.Normal) == 0)
                    {
                        currentMesh = mesh.Clone(MeshFlags.Managed | (MeshFlags)use32Bit,
                            mesh.VertexFormat | VertexFormats.Normal, e.Device);

                        // Compute normals now
                        currentMesh.ComputeNormals();
                    }

                    using (currentMesh)
                    {
                        // Generate progressive meshes
                        using(ProgressiveMesh pMesh = new ProgressiveMesh(currentMesh, adjacencyBuffer, null, 1, MeshFlags.SimplifyVertex))
                        {
                            int minVerts = pMesh.MinVertices;
                            int maxVerts = pMesh.MaxVertices;
                            int vertsPerMesh = (maxVerts - minVerts + 10) / 10;

                            // How many meshes should be in the array
                            int numMeshes = Math.Max(1, (int)Math.Ceiling((maxVerts - minVerts + 1) / (float)vertsPerMesh) );
                            meshes = new ProgressiveMesh[numMeshes];

                            // Clone full sized pmesh
                            fullMesh = pMesh.Clone(MeshFlags.Managed | MeshFlags.VbShare, pMesh.VertexFormat, e.Device);

                            // Clone all the separate pmeshes
                            for (int iMesh = 0; iMesh < numMeshes; iMesh++)
                            {
                                meshes[iMesh] = pMesh.Clone(MeshFlags.Managed | MeshFlags.VbShare, pMesh.VertexFormat, e.Device);

                                // Trim to appropriate space
                                meshes[iMesh].TrimByVertices(minVerts + vertsPerMesh * iMesh, minVerts + vertsPerMesh * (iMesh + 1));
                                meshes[iMesh].OptimizeBaseLevelOfDetail(MeshFlags.OptimizeVertexCache);
                            }

                            // Set the current to be max vertices
                            currentMeshIndex = numMeshes - 1;
                            meshes[currentMeshIndex].NumberVertices = maxVerts;
                            fullMesh.NumberVertices = maxVerts;

                            // Set up the slider to reflect the vertices range the mesh has
                            sampleUi.GetSlider(Detail).SetRange(meshes[0].MinVertices,
                                meshes[meshes.Length -1].MaxVertices);
                            sampleUi.GetSlider(Detail).Value = (meshes[currentMeshIndex] as BaseMesh).NumberVertices;
                        }
                    }
                }
            }

            // Restore the original folder
            System.IO.Directory.SetCurrentDirectory(currentFolder);
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

            // Setup the camera's projection parameters
            float aspectRatio = (float)desc.Width / (float)desc.Height;
            camera.SetProjectionParameters((float)Math.PI / 4, aspectRatio, 0.1f, 1000.0f);
            camera.SetWindow(desc.Width, desc.Height);

            // Setup UI locations
            hud.SetLocation(desc.Width-170, 0);
            hud.SetSize(170,170);
            sampleUi.SetLocation(0, desc.Height - 50);
            sampleUi.SetSize(desc.Width,50);

            // Update controls
            sampleUi.GetControl(DetailLabel).SetLocation((desc.Width - 200) / 2, 10);
            sampleUi.GetControl(UseOptimizedCheckBox).SetLocation(desc.Width - 130, 5);
            sampleUi.GetControl(Detail).SetSize(desc.Width - 20, 16);
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
            meshTextures = null;
            meshMaterials = null;

            if (fullMesh != null)
                fullMesh.Dispose();

            if ( (meshes != null) && (meshes.Length > 0) )
            {
                for (int i = 0; i < meshes.Length; i++)
                {
                    if (meshes[i] != null)
                        meshes[i].Dispose();
                }
            }
            meshes = null;
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
        public void OnFrameRender(Device device, double appTime, float elapsedTime)
        {
            bool beginSceneCalled = false;

            // Clear the render target and the zbuffer 
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, 0x00424B79, 1.0f, 0);
            try
            {
                device.BeginScene();
                beginSceneCalled = true;

                // Get the world matrix
                Matrix worldMatrix = worldCenter * camera.WorldMatrix;

                if ((meshes != null) && (meshes.Length > 0))
                {
                    // Update the effect's variables.  Instead of using strings, it would 
                    // be more efficient to cache a handle to the parameter by calling 
                    // Effect.GetParameter
                    effect.SetValue("g_mWorldViewProjection", worldMatrix * camera.ViewMatrix * camera.ProjectionMatrix);
                    effect.SetValue("g_mWorld", worldMatrix);

                    // Set and draw each of the materials in the mesh
                    for (int i = 0; i < meshMaterials.Length; i++)
                    {
                        effect.SetValue("g_vDiffuse", meshMaterials[i].DiffuseColor);
                        effect.SetValue("g_txScene", meshTextures[i]);
                        int passes = effect.Begin(0);
                        for(int pass = 0; pass < passes; pass++)
                        {
                            effect.BeginPass(pass);
                            if (isShowingOptimized)
                            {
                                meshes[currentMeshIndex].DrawSubset(i);
                            }
                            else
                            {
                                fullMesh.DrawSubset(i);
                            }
                            effect.EndPass();
                        }
                        effect.End();
                    }
                }
                // Show frame rate
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

            txtHelper.SetForegroundColor(System.Drawing.Color.White);
            if (isShowingOptimized)
            {
                txtHelper.DrawTextLine(OptimizedText, currentMeshIndex + 1, meshes.Length, 
                    meshes[currentMeshIndex].MinVertices, meshes[currentMeshIndex].MaxVertices, 
                    meshes[0].MinVertices, meshes[meshes.Length -1].MaxVertices, 
                    (meshes[currentMeshIndex] as BaseMesh).NumberVertices);
            }
            else
            {
                txtHelper.DrawTextLine(UnoptimizedText, fullMesh.MinFaces, fullMesh.MaxFaces, 
                    (fullMesh as BaseMesh).NumberVertices);
           }

            // Draw help
            if (isHelpShowing)
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*7);
                txtHelper.SetForegroundColor(System.Drawing.Color.DarkOrange);
                txtHelper.DrawTextLine("Controls (F1 to hide):");

                txtHelper.SetInsertionPoint(40, sampleFramework.BackBufferSurfaceDescription.Height-15*6);
                txtHelper.DrawTextLine("Rotate mesh: Left click drag");
                txtHelper.DrawTextLine("Zoom: mouse wheel");
                txtHelper.DrawTextLine("Quit: Esc");
                txtHelper.DrawTextLine("Hide help: F1");
            }
            else
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*4);
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

            noFurtherProcessing = sampleUi.MessageProc(hWnd, msg, wParam, lParam);
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

            // Now add the sample specific UI
            y = 10;
            sampleUi.IsUsingKeyboardInput = true;
            sampleUi.AddStatic(DetailLabel, "Level of Detail:", 0, y, 200, 16);
            Checkbox opt = sampleUi.AddCheckBox(UseOptimizedCheckBox, "Use optimized mesh", 50, y, 200, 20, true);
            Slider sl = sampleUi.AddSlider(Detail, 10, y += 16, 200, 16, 4, 4, 4, false);

            // Hook the sample events
            opt.Changed += new EventHandler(OnOptimizedChanged);
            sl.ValueChanged += new EventHandler(OnDetailChanged);

            // Set button masks
            camera.SetButtonMasks((int)MouseButtonMask.Left, (int)MouseButtonMask.Wheel, 0);
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

        /// <summary>Fired when the optimized checkbox is changed</summary>
        private void OnOptimizedChanged(object sender, EventArgs e)
        {
            isShowingOptimized = (sender as Checkbox).IsChecked;
        }

        /// <summary>Fired when the level of detail is changed</summary>
        private void OnDetailChanged(object sender, EventArgs e)
        {
            SetNumberVertices((sender as Slider).Value);
        }

        /// <summary>Sets the number of vertices for the progressive meshes</summary>
        private void SetNumberVertices(int numberVerts)
        {
            // Update the full mesh first
            fullMesh.NumberVertices = numberVerts;

            // If current pm valid for desired value, then set the number of vertices directly
            if ( (numberVerts >= meshes[currentMeshIndex].MinVertices) &&
                (numberVerts <= meshes[currentMeshIndex].MaxVertices) )
            {
                meshes[currentMeshIndex].NumberVertices = numberVerts;
            }
            else
            {
                // Search for the right one
                currentMeshIndex = meshes.Length - 1;
                
                // Look for the correct "bin"
                while (currentMeshIndex > 0)
                {
                    // If number of vertices is less than current max then we found one to fit
                    if (numberVerts >= meshes[currentMeshIndex].MinVertices)
                        break;

                    currentMeshIndex--;
                }

                // Set the vertices on the newly selected mesh
                meshes[currentMeshIndex].NumberVertices = numberVerts;
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
                ProgressiveMeshes sample = new ProgressiveMeshes(sampleFramework);
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
                    sampleFramework.CreateWindow("ProgressiveMesh: Using Progressive Meshes in Direct3D");
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
