//-----------------------------------------------------------------------------
// File: SimpleAnimation.cs
//
// Simple skeletal animation using Managed DirectX
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace SimpleAnimationSample
{
    /// <summary>SimpleAnimation Sample Class</summary>
    public class SimpleAnimation : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public SimpleAnimation(Framework f) 
        { 
            // Store framework
            sampleFramework = f; 
            // Create dialogs
            hud = new Dialog(sampleFramework); 
        }
        #endregion

        // Variables
        private Framework sampleFramework = null; // Framework for samples
        private Font statsFont = null; // Font for drawing text
        private Sprite textSprite = null; // Sprite for batching text calls
        private ModelViewerCamera camera = new ModelViewerCamera(); // A model viewing camera
        private bool isHelpShowing = false; // If true, renders the UI help text
        private Dialog hud = null; // dialog for standard controls

        // Sample specific items

        // Animation root frame
        private AnimationRootFrame rootFrame; 
        private float objectRadius = 0.0f; // Radius of the object
        private Vector3 objectCenter; // Center of the object
        private bool isDestroyed = false;
        private static readonly ColorValue WhiteColor = new ColorValue(1.0f, 1.0f, 1.0f, 1.0f);
        private static readonly ColorValue Color2 = new ColorValue(0.25f, 0.25f, 0.25f, 1.0f);

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
                 (caps.VertexShaderVersion < new Version(1,1)) || 
                 (caps.MaxVertexBlendMatrixIndex < 12) )
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
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "Simple Animation");
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

            // Create our allocate hierarchy derived class
            AnimationAllocation alloc = new AnimationAllocation(this);

            // Load our file
            string meshFile = Utility.FindMediaFile("tiny\\tiny.x");
            // Store the current folder, then switch to the folder where the media was found
            string currentFolder = System.IO.Directory.GetCurrentDirectory();
            System.IO.Directory.SetCurrentDirectory(new System.IO.FileInfo(meshFile).Directory.FullName);
            // Now load the mesh hierarchy
            rootFrame = Mesh.LoadHierarchyFromFile(meshFile, MeshFlags.Managed, e.Device, alloc, null);
            // Restore the folder
            System.IO.Directory.SetCurrentDirectory(currentFolder);

            // Calculate the center and radius of a bounding sphere
            objectRadius = Frame.CalculateBoundingSphere(rootFrame.FrameHierarchy, 
                out objectCenter);

            // Setup the matrices for animation
            SetupBoneMatrices(rootFrame.FrameHierarchy as AnimationFrame);
            isDestroyed = false;
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
            camera.SetProjectionParameters((float)Math.PI / 4, aspectRatio, objectRadius / 64.0f, objectRadius * 200.0f);
            camera.SetViewParameters(new Vector3(0, 0, -2.2f * objectRadius), Vector3.Empty);
            camera.SetWindow(desc.Width, desc.Height);

            // Set device transforms
            e.Device.Transform.Projection = camera.ProjectionMatrix;
            e.Device.Transform.View = camera.ViewMatrix;

            // Setup a light
            e.Device.Lights[0].Type = LightType.Directional;
            e.Device.Lights[0].Direction = new Vector3(0.0f, -1.0f, 1.0f);
            e.Device.Lights[0].DiffuseColor = WhiteColor;
            e.Device.Lights[0].Enabled = true;

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
            if (isDestroyed)
                return; // Only clean up once

            Frame.Destroy(rootFrame.FrameHierarchy, new AnimationAllocation(this));
            if (rootFrame.AnimationController != null)
            {
                rootFrame.AnimationController.Dispose();
            }

            isDestroyed = true;
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

            // Build the world matrix
            Matrix worldMatrix = Matrix.Translation(-objectCenter);
            worldMatrix *= camera.WorldMatrix;

            // Set world matrix
            device.Transform.World = worldMatrix;

            // Has any time elapsed?
            if (elapsedTime > 0.0f)
            {
                if (rootFrame.AnimationController != null)
                    rootFrame.AnimationController.AdvanceTime(elapsedTime);

                UpdateFrameMatrices(rootFrame.FrameHierarchy as AnimationFrame,
                    worldMatrix);
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
            bool beginSceneCalled = false;

            // Clear the render target and the zbuffer 
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, 0x002D32AA, 1.0f, 0);
            try
            {
                device.BeginScene();
                beginSceneCalled = true;

                // Render the animation
                DrawFrame(rootFrame.FrameHierarchy as AnimationFrame);

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
        /// <summary>Update the frames matrices and combine it with it's parents</summary>
        private void UpdateFrameMatrices(AnimationFrame frame, Matrix parentMatrix)
        {
            frame.CombinedTransformationMatrix = frame.TransformationMatrix * 
                parentMatrix;

            if (frame.FrameSibling != null)
            {
                UpdateFrameMatrices(frame.FrameSibling as AnimationFrame, parentMatrix);
            }

            if (frame.FrameFirstChild != null)
            {
                UpdateFrameMatrices(frame.FrameFirstChild as AnimationFrame, 
                    frame.CombinedTransformationMatrix);
            }
        }

        /// <summary>Draw a frame and all child and sibling frames</summary>
        private void DrawFrame(AnimationFrame frame)
        {
            AnimationMeshContainer mesh = frame.MeshContainer as AnimationMeshContainer;
            while(mesh != null)
            {
                DrawMeshContainer(mesh, frame);

                mesh = mesh.NextContainer as AnimationMeshContainer;
            }

            if (frame.FrameSibling != null)
            {
                DrawFrame(frame.FrameSibling as AnimationFrame);
            }

            if (frame.FrameFirstChild != null)
            {
                DrawFrame(frame.FrameFirstChild as AnimationFrame);
            }
        }

        /// <summary>Render a mesh container</summary>
        private void DrawMeshContainer(AnimationMeshContainer mesh, AnimationFrame parent)
        {
            Device device = sampleFramework.Device;
            // first check for skinning
            if (mesh.SkinInformation != null)
            {
                if (mesh.NumberInfluences == 1)
                    device.RenderState.VertexBlend = VertexBlend.ZeroWeights;
                else
                    device.RenderState.VertexBlend = (VertexBlend)(mesh.NumberInfluences - 1);

                if (mesh.NumberInfluences > 0)
                    device.RenderState.IndexedVertexBlendEnable = true;

                BoneCombination[] bones = mesh.GetBones();

                for(int iAttrib = 0; iAttrib < mesh.NumberAttributes; iAttrib++)
                {
                    // first, get world matrices
                    for (int iPaletteEntry = 0; iPaletteEntry < mesh.NumberPaletteEntries; 
                        ++iPaletteEntry)
                    {
                        int iMatrixIndex = bones[iAttrib].BoneId[iPaletteEntry];
                        if (iMatrixIndex != -1)
                        {
                            device.Transform.SetWorldMatrixByIndex(iPaletteEntry, 
                                mesh.GetOffsetMatrices()[iMatrixIndex] * 
                                mesh.GetFrames()[iMatrixIndex].
                                CombinedTransformationMatrix);

                        }
                    }

                    // Setup the material
                    device.Material = mesh.GetMaterials()[bones[iAttrib].AttributeId].Material3D;
                    device.SetTexture(0, mesh.GetTextures()[bones[iAttrib].AttributeId]);

                    // Finally draw the subset
                    mesh.MeshData.Mesh.DrawSubset(iAttrib);
                }
            }
            else
            {
                // Standard mesh, just draw it using FF
                device.RenderState.VertexBlend = VertexBlend.Disable;

                // Set up transforms
                device.Transform.World = parent.CombinedTransformationMatrix;

                ExtendedMaterial[] materials = mesh.GetMaterials();
                for (int i = 0; i < materials.Length; ++i)
                {
                    device.Material = materials[i].Material3D;
                    device.SetTexture(0, mesh.GetTextures()[i]);
                    mesh.MeshData.Mesh.DrawSubset(i);
                }
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
            txtHelper.DrawTextLine("Rendering simple animation");

            // Draw help
            if (isHelpShowing)
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*6);
                txtHelper.SetForegroundColor(System.Drawing.Color.DarkOrange);
                txtHelper.DrawTextLine("Controls (F1 to hide):");

                txtHelper.SetInsertionPoint(40, sampleFramework.BackBufferSurfaceDescription.Height-15*5);
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
        /// Generate the skinned mesh information
        /// </summary>
        public void GenerateSkinnedMesh(AnimationMeshContainer mesh)
        {
            if (mesh.SkinInformation == null)
                throw new ArgumentException();  // There is nothing to generate

            MeshFlags flags = MeshFlags.OptimizeVertexCache;

            Caps caps = sampleFramework.DeviceCaps;
            if (caps.VertexShaderVersion >= new Version(1,1))
            {
                flags |= MeshFlags.Managed;
            }
            else
            {
                flags |= MeshFlags.SystemMemory;
            }

            int numMaxFaceInfl;
            using(IndexBuffer ib = mesh.MeshData.Mesh.IndexBuffer)
            {
                numMaxFaceInfl = mesh.SkinInformation.GetMaxFaceInfluences(ib, 
                    mesh.MeshData.Mesh.NumberFaces);
            }
            // 12 entry palette guarantees that any triangle (4 independent 
            // influences per vertex of a tri) can be handled
            numMaxFaceInfl = (int)Math.Min(numMaxFaceInfl, 12);

            if (caps.MaxVertexBlendMatrixIndex + 1 >= numMaxFaceInfl)
            {
                mesh.NumberPaletteEntries = (int)Math.Min((caps.
                    MaxVertexBlendMatrixIndex+ 1) / 2, 
                    mesh.SkinInformation.NumberBones);

                flags |= MeshFlags.Managed;
            }

            int influences = 0;
            BoneCombination[] bones = null;

            // Use ConvertToBlendedMesh to generate a drawable mesh
            MeshData data = mesh.MeshData;
            data.Mesh = mesh.SkinInformation.ConvertToIndexedBlendedMesh(data.Mesh, flags, 
                mesh.GetAdjacencyStream(), mesh.NumberPaletteEntries, out influences, 
                out bones);

            // Store this info
            mesh.NumberInfluences = influences;
            mesh.SetBones(bones);

            // Get the number of attributes
            mesh.NumberAttributes = bones.Length;

            mesh.MeshData = data;
        }

        /// <summary>This method will set the bone matrices for a frame</summary>
        private void SetupBoneMatrices(AnimationFrame frame)
        {
            // First do the mesh container this frame contains (if it does)
            if (frame.MeshContainer != null)
            {
                SetupBoneMatrices(frame.MeshContainer as AnimationMeshContainer);
            }
            // Next do any siblings this frame may contain
            if (frame.FrameSibling != null)
            {
                SetupBoneMatrices(frame.FrameSibling as AnimationFrame);
            }
            // Finally do the children of this frame
            if (frame.FrameFirstChild != null)
            {
                SetupBoneMatrices(frame.FrameFirstChild as AnimationFrame);
            }
        }

        /// <summary>Sets the bone matrices for a mesh container</summary>
        private void SetupBoneMatrices(AnimationMeshContainer mesh)
        {
            // Is there skin information?  If so, setup the matrices
            if (mesh.SkinInformation != null)
            {
                int numberBones = mesh.SkinInformation.NumberBones;

                AnimationFrame[] frameMatrices = new AnimationFrame[numberBones];
                for(int i = 0; i< numberBones; i++)
                {
                    AnimationFrame frame = Frame.Find(rootFrame.FrameHierarchy, 
                        mesh.SkinInformation.GetBoneName(i)) as AnimationFrame;

                    if (frame == null)
                        throw new InvalidOperationException("Could not find valid bone.");

                    frameMatrices[i] = frame;
                }
                mesh.SetFrames(frameMatrices);
            }
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
                SimpleAnimation sample = new SimpleAnimation(sampleFramework);
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
                    sampleFramework.CreateWindow("SimpleAnimation");
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

    #region Derived Frame Class
    /// <summary>
    /// The frame that will hold mesh animation 
    /// </summary>
    public class AnimationFrame : Frame
    {
        // Store the combined transformation matrix
        private Matrix combined = Matrix.Identity;
        /// <summary>The combined transformation matrix</summary>
        public Matrix CombinedTransformationMatrix
        {
            get { return combined; } set { combined = value; }
        }
    }
    #endregion
    #region Derived Mesh Container
    /// <summary>
    /// The mesh container class that will hold the animation data
    /// </summary>
    public class AnimationMeshContainer : MeshContainer
    {
        // Array data
        private Texture[] meshTextures = null;
        private BoneCombination[] bones;
        private Matrix[] offsetMatrices;
        private AnimationFrame[] frameMatrices;

        // Instance data
        private int numAttributes = 0;
        private int numInfluences = 0;
        private int numPalette = 0;

        // Public properties

        /// <summary>Retrieve the textures used for this container</summary>
        public Texture[] GetTextures() { return meshTextures; }
        /// <summary>Set the textures used for this container</summary>
        public void SetTextures(Texture[] textures) { meshTextures = textures; }
        
        /// <summary>Retrieve the bone combinations used for this container</summary>
        public BoneCombination[] GetBones() { return bones; }
        /// <summary>Set the bone combinations used for this container</summary>
        public void SetBones(BoneCombination[] b) { bones = b; }
        
        /// <summary>Retrieve the animation frames used for this container</summary>
        public AnimationFrame[] GetFrames() { return frameMatrices; }
        /// <summary>Set the animation frames used for this container</summary>
        public void SetFrames(AnimationFrame[] frames) { frameMatrices = frames; }
        
        /// <summary>Retrieve the offset matrices used for this container</summary>
        public Matrix[] GetOffsetMatrices() { return offsetMatrices; }
        /// <summary>Set the offset matrices used for this container</summary>
        public void SetOffsetMatrices(Matrix[] matrices) { offsetMatrices = matrices; }
        
        /// <summary>Total number of attributes this mesh container contains</summary>
        public int NumberAttributes { get { return numAttributes; } set { numAttributes = value; } }
        /// <summary>Total number of influences this mesh container contains</summary>
        public int NumberInfluences { get { return numInfluences; } set { numInfluences = value; } }
        /// <summary>Total number of palette entries this mesh container contains</summary>
        public int NumberPaletteEntries { get { return numPalette; } set { numPalette = value; } }
    }
    #endregion

    #region Animation Allocation Hierarchy
    /// <summary>
    /// AllocateHierarchy derived class
    /// </summary>
    public class AnimationAllocation : AllocateHierarchy
    {
        SimpleAnimation parent = null;
        /// <summary>Create new instance of this class</summary>
        public AnimationAllocation(SimpleAnimation p) { parent = p; }

        /// <summary>Create a new frame</summary>
        public override Frame CreateFrame(string name)
        {
            AnimationFrame frame = new AnimationFrame();
            frame.Name = name;
            frame.TransformationMatrix = Matrix.Identity;
            frame.CombinedTransformationMatrix = Matrix.Identity;

            return frame;
        }

        /// <summary>Create a new mesh container</summary>
        public override MeshContainer CreateMeshContainer(string name, 
            MeshData meshData, ExtendedMaterial[] materials, 
            EffectInstance[] effectInstances, GraphicsStream adjacency, 
            SkinInformation skinInfo)
        {
            // We only handle meshes here
            if (meshData.Mesh == null)
                throw new ArgumentException();

            // We must have a vertex format mesh
            if (meshData.Mesh.VertexFormat == VertexFormats.None)
                throw new ArgumentException();

            AnimationMeshContainer mesh = new AnimationMeshContainer();

            mesh.Name = name;
            int numFaces = meshData.Mesh.NumberFaces;
            Device dev = meshData.Mesh.Device;
            
            // Make sure there are normals
            if ((meshData.Mesh.VertexFormat & VertexFormats.Normal) == 0)
            {
                // Clone the mesh
                Mesh tempMesh = meshData.Mesh.Clone(meshData.Mesh.Options.Value, 
                    meshData.Mesh.VertexFormat | VertexFormats.Normal, dev);

                // Destroy current mesh, use the new one
                meshData.Mesh.Dispose();
                meshData.Mesh = tempMesh;
                meshData.Mesh.ComputeNormals();
            }

            // Store the materials
            mesh.SetMaterials(materials);
            mesh.SetAdjacency(adjacency);
            Texture[] meshTextures = new Texture[materials.Length];
            
            // Create any textures
            for (int i = 0; i < materials.Length; i++)
            {
                if (materials[i].TextureFilename != null)
                {
                    meshTextures[i] = ResourceCache.GetGlobalInstance().CreateTextureFromFile(
                        dev, materials[i].TextureFilename);
                }
            }
            mesh.SetTextures(meshTextures);
            mesh.MeshData = meshData;

            // If there is skinning info, save any required data
            if (skinInfo != null)
            {
                mesh.SkinInformation = skinInfo;
                int numBones = skinInfo.NumberBones;
                Matrix[] offsetMatrices = new Matrix[numBones];

                for (int i = 0; i < numBones; i++)
                    offsetMatrices[i] = skinInfo.GetBoneOffsetMatrix(i);

                mesh.SetOffsetMatrices(offsetMatrices);

                parent.GenerateSkinnedMesh(mesh);
            }

            return mesh;
        }

    }
    #endregion
}
