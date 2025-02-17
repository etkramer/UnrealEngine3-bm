//-----------------------------------------------------------------------------
// File: HDRCubeMap.cs
//
// The sample demonstrates high dynamic range lighting with cube maps
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace HDRCubeMapSample
{
    /// <summary>Structure used to hold the orbit data</summary>
    struct OrbitData
    {
        public string MeshFile; // XFile name
        public Vector3 Axis; // Axis of rotation
        public float Radius; // Orbit radius
        public float Speed; // Orbit speed in radians per second
        
        /// <summary>Create a new instance of this object</summary>
        public OrbitData(string mesh, Vector3 rotation, float r, float s)
        {
            MeshFile = mesh; Axis = rotation; Radius = r; Speed = s;
        }
    }

    /// <summary>Encapsulate an object in the scene with its world transformation matrix</summary>
    class SceneObject
    {
        public Matrix WorldMatrix = Matrix.Identity; // World transformation
        public FrameworkMesh Mesh; // Mesh object

        /// <summary>Create a new instance</summary>
        public SceneObject(FrameworkMesh m) { Mesh = m; }

        /// <summary>
        /// Compute the world transformation matrix to center the mesh at origin in world space
        /// and scale its size to the specified radius.
        /// </summary>
        public void WorldCenterAndScaleToRadius(float radius)
        {
            // Compute bounding sphere
            Vector3 center;
            float radiusBound = Mesh.ComputeBoundingSphere(out center);

            // Translate and scale
            float scaleFator = radius / radiusBound;
            WorldMatrix = Matrix.Translation(-center) * Matrix.Scaling(scaleFator, scaleFator, scaleFator);
        }
    }

    /// <summary>Encapsulate an orbiter object in the scene with related data</summary>
    class Orbiter : SceneObject
    {
        private Vector3 axis = new Vector3(0.0f, 1.0f, 0.0f); // orbit axis
        private float radius = 1.0f; // Orbit radius
        private float speed = (float)Math.PI; // Orbit speed in radians per second

        /// <summary>Create a new instance</summary>
        public Orbiter(FrameworkMesh m) : base(m) {  }

        /// <summary>Call this after the world matrix is initialized</summary>
        public void SetOrbit(Vector3 rotation, float r, float s)
        {
            // Normalize the rotation
            rotation.Normalize();
            // Store the variables
            axis = rotation; radius = r; speed = s;

            // Translate by radius in the -Y direction
            Matrix m = Matrix.Translation(0.0f, -radius, 0.0f);
            WorldMatrix *= m;

            // Apply rotation from X axis to orbit axis
            Vector3 xAxis = new Vector3(1.0f, 0.0f, 0.0f);
            Vector3 rotationVector = Vector3.Cross(axis, xAxis); // Axis of rotation
            
            // If the cross product is 0, axis is on the X axis
            // So we either rotate 0 or PI.
            if (rotationVector.LengthSq() == 0)
            {
                if (axis.X < 0.0f)
                    m = Matrix.RotationY((float)Math.PI);
                else
                    m = Matrix.Identity;
            }
            else
            {
                float angle = (float)Math.Acos(Vector3.Dot(axis, xAxis) ); // Angle to rotate
                // Find out direction to rotate
                Vector3 xCrossRot = Vector3.Cross(xAxis, rotationVector);
                if (Vector3.Dot(xCrossRot, axis) >= 0)
                    angle = -angle;
                
                // Calculate the rotation
                m = Matrix.RotationAxis(rotationVector, angle);
            }
            
            // Update world matrix
            WorldMatrix *= m;
        }

        /// <summary>Compute the orbit transformation and apply to WorldMatrix</summary>
        public void Orbit(float elapsedTime)
        {
            WorldMatrix *= Matrix.RotationAxis(axis, speed * elapsedTime);
        }
    }

    /// <summary>Encapsulate a light object in the scene with related data</summary>
    struct LightObject
    {
        public Vector4 Position; // Position in world space
        public Vector4 MoveDirection; // Direction in which it moves
        public float MoveDistance; // Maximum distance it can move
        public Matrix WorldMatrix; // World transform matrix for the light before animation
        public Matrix WorkingMatrix; // Working matrix (world transform after animation)
    }
    /// <summary>Stores cached handles for an effect in a technique group</summary>
    struct TechniqueGroup
    {
        public EffectHandle RenderScene;
        public EffectHandle RenderLight;
        public EffectHandle RenderEnvMap;
    }

    /// <summary>Class for rendering high dynamic range cube maps</summary>
    public class HDRCubeMap : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public HDRCubeMap(Framework f) 
        { 
            // Store framework
            sampleFramework = f; 
            // Create dialogs
            hud = new Dialog(sampleFramework); 
        }
        #endregion

        // Constants
        private const int EnvironmentMapSize = 256;
        // Currently, 4 is the only number of lights supported.
        private const int NumberLights = 4;
        private const float LightMeshRadius = 0.2f;
        private static readonly int HelpTextColor = new ColorValue(0.0f, 1.0f, 0.3f, 1.0f).ToArgb();
        private static readonly VertexElement[] decl = new VertexElement[] {
            new VertexElement(0, 0, DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Position, 0), 
            new VertexElement(0, 12, DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Normal, 0), 
            new VertexElement(0, 24, DeclarationType.Float2, DeclarationMethod.Default, DeclarationUsage.TextureCoordinate, 0), 
            VertexElement.VertexDeclarationEnd };

        // The meshes that will be the reflecting meshes, not required to be these meshes
        private static readonly string[] MeshFileNames = new string[] {
                "misc\\teapot.x",
                "misc\\skullocc.x",
                "misc\\car.x"};

        // Mesh file to use for orbiter objects
        // These don't have to use the same mesh file.
        private static readonly OrbitData[] OrbitersInfo = new OrbitData[] {
            new OrbitData("airplane\\airplane 2.x", new Vector3(-1.0f, 0.0f, 0.0f), 2.0f, (float)Math.PI), 
            new OrbitData("airplane\\airplane 2.x", new Vector3(0.3f, 1.0f, 0.3f), 2.5f, (float)Math.PI / 2.0f) };
                                                                               
        // Variables
        private Framework sampleFramework = null; // Framework for samples
        private Font statsFont = null; // Font for drawing text
        private Sprite textSprite = null; // Sprite for batching text calls
        private Effect effect = null; // D3DX Effect Interface
        private ModelViewerCamera camera = new ModelViewerCamera(); // A model viewing camera
        private bool isHelpShowing = true; // If true, renders the UI help text
        private Dialog hud = null; // dialog for standard controls
        private LightObject[] lights = new LightObject[NumberLights]; // Parameters of light objects
        private Vector4[] lightPositions = new Vector4[NumberLights]; // Positions of the lights in space
        private Vector4 lightIntensity; // Light intensity
        private SceneObject[] envMeshes = new SceneObject[MeshFileNames.Length]; // Mesh to receive environment mapping
        private int currentMesh = 0; // Index of mesh that is being displayed
        private FrameworkMesh roomMesh; // Mesh representing room (wall, floor, ceiling)
        private FrameworkMesh lightMesh; // Mesh for the light object
        private Orbiter[] orbiters = new Orbiter[OrbitersInfo.Length]; // Orbiter meshes

        private VertexDeclaration vertexDecl = null; // Vertex decl for the sample
        private CubeTexture[] cubeMapFloating = new CubeTexture[2]; // Floating point format cubemap
        private CubeTexture cubeMapInt32 = null; // 32-bit cube map (for fallback)
        private Surface depthCube = null; // Depth-stencil buffer for rendering to cube texture
        private int numberFloatingCube = 0; // Number of cube maps required for using floating point
        private TechniqueGroup[] groupFloating = new TechniqueGroup[2]; // Group of techniques to use with floating pointcubemaps (2 cubes max)
        private TechniqueGroup groupInt32; // Group of techniques to use with integer cubemaps

        private TechniqueGroup[] currentTechGroup; // Technique group currently used
        private CubeTexture[] currentCubeTexture; // Cube Texture currently used

        // Handles for effect
        private EffectHandle worldViewHandle = null; // Handle for world+view matrix in effect
        private EffectHandle projHandle = null; // Handle for projection matrix in effect
        private EffectHandle[] cubeTextureHandle = new EffectHandle[2]; // Handle for the cube texture in effect
        private EffectHandle sceneTextureHandle = null; // Handle for the scene texture in effect
        private EffectHandle lightIntensityHandle = null; // Handle for the light intensity in effect
        private EffectHandle lightPositionHandle = null; // Handle for view space light positions in effect
        private EffectHandle reflectivityHandle = null; // Handle for reflectivity in effect

        private int numberCubes; // Number of cube maps used based on current cubemap format

        private bool usingFloatCubeMap; // Whether we use floating point format cube map
        private float reflectivity; // Reflectivity value. Ranges from 0 to 1.

        // HUD Ui Control constants
        private const int ToggleFullscreen = 1;
        private const int ToggleReference = 3;
        private const int ChangeDevice = 4;
        private const int ChangeMesh = 5;
        private const int ResetParams = 6;
        private const int SliderLightText = 7;
        private const int SliderLight = 8;
        private const int SliderReflectText = 9;
        private const int SliderReflect = 10;
        private const int CheckHDR = 11;

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

            // Must support cube textures
            if (!caps.TextureCaps.SupportsCubeMap)
                return false;

            // Must support pixel shader 2.0
            if (caps.PixelShaderVersion < new Version(2,0))
                return false;

            // Need to support Format.A16B16R16F render target
            if (!Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, adapterFormat, Usage.RenderTarget,
                ResourceType.CubeTexture, Format.A16B16G16R16F))
            {
                // If not, need to support Format.G16R16F render target as fallback
                if (!Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, adapterFormat, Usage.RenderTarget,
                    ResourceType.CubeTexture, Format.G16R16F))
                {
                    return false;
                }
            }
            

            // Need to support Format.A8R8G8B8 render target
            if (!Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, adapterFormat, Usage.RenderTarget,
                ResourceType.CubeTexture, Format.A8R8G8B8))
            {
                return false;
            }

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
            // Initialize the number of cube maps required when using floating point format
            if (!Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, settings.AdapterFormat, Usage.RenderTarget,
                ResourceType.CubeTexture, Format.A16B16G16R16F))
            {
                numberCubes = numberFloatingCube = 2;
            }
            else
            {
                numberCubes = numberFloatingCube = 1;
            }

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
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "HDR CubeMap");
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
            string path = Utility.FindMediaFile("HDRCubeMap.fx");
            effect = ResourceCache.GetGlobalInstance().CreateEffectFromFile(e.Device,
                path, null, null, shaderFlags, null);

            // Store the handles
            worldViewHandle = effect.GetParameter(null, "worldViewMatrix");
            projHandle = effect.GetParameter(null, "projMatrix");
            cubeTextureHandle[0] = effect.GetParameter(null, "cubeMapTexture");
            cubeTextureHandle[1] = effect.GetParameter(null, "cubeMapTexture2");
            sceneTextureHandle = effect.GetParameter(null, "sceneTexture");
            lightIntensityHandle = effect.GetParameter(null, "lightIntensityVector");
            reflectivityHandle = effect.GetParameter(null, "reflectivity");
            lightPositionHandle = effect.GetParameter(null, "lightPositionView");

            // Determine the technique to render with

            // Integer cube map
            groupInt32.RenderScene = effect.GetTechnique("RenderScene");
            groupInt32.RenderLight = effect.GetTechnique("RenderLight");
            groupInt32.RenderEnvMap = effect.GetTechnique("RenderHDREnvMap");

            // Floating point cubemaps
            if (numberFloatingCube == 2)
            {
                // Two floating point G16R16F cube maps
                groupFloating[0].RenderScene = effect.GetTechnique("RenderSceneFirstHalf");
                groupFloating[0].RenderLight = effect.GetTechnique("RenderLightFirstHalf");
                groupFloating[0].RenderEnvMap = effect.GetTechnique("RenderHDREnvMap2Tex");
                groupFloating[1].RenderScene = effect.GetTechnique("RenderSceneSecondHalf");
                groupFloating[1].RenderLight = effect.GetTechnique("RenderLightSecondHalf");
                groupFloating[1].RenderEnvMap = effect.GetTechnique("RenderHDREnvMap2Tex");
            }
            else
            {
                // Single floating point cube map
                groupFloating[0].RenderScene = effect.GetTechnique("RenderScene");
                groupFloating[0].RenderLight = effect.GetTechnique("RenderLight");
                groupFloating[0].RenderEnvMap = effect.GetTechnique("RenderHDREnvMap");
            }

            // Initialize reflectivity
            effect.SetValue(reflectivityHandle, reflectivity);

            // Initialize light intensity
            effect.SetValue(lightIntensityHandle, lightIntensity);

            // Create vertex declaration
            vertexDecl = new VertexDeclaration(e.Device, decl);

            // Load the meshes
            for(int i = 0; i < MeshFileNames.Length; i++)
            {
                // Load the meshes
                envMeshes[i] = new SceneObject(LoadMesh(e.Device, MeshFileNames[i]));
                envMeshes[i].WorldCenterAndScaleToRadius(1.0f); // Scale to a radius of one
            }

            // Load the room object
            roomMesh = LoadMesh(e.Device, "room.x");

            // Load the light object
            lightMesh = LoadMesh(e.Device, "misc\\sphere0.x");

            // Initialize the world matrices for the lights
            Vector3 center;
            float radius = lightMesh.ComputeBoundingSphere(out center);
            float scaleFactor = LightMeshRadius / radius;
            Matrix lightWorld = Matrix.Translation(-center) * Matrix.Scaling(scaleFactor, scaleFactor, scaleFactor);

            for (int i = 0; i < NumberLights; i++)
            {
                lights[i].WorldMatrix = lightWorld * Matrix.Translation(lights[i].Position.X, lights[i].Position.Y, lights[i].Position.Z);
            }

            // Load the orbiters
            for (int i = 0; i < OrbitersInfo.Length; i++)
            {
                orbiters[i] = new Orbiter(LoadMesh(e.Device, OrbitersInfo[i].MeshFile));
                orbiters[i].WorldCenterAndScaleToRadius(0.7f);
                orbiters[i].SetOrbit(OrbitersInfo[i].Axis, OrbitersInfo[i].Radius, OrbitersInfo[i].Speed);
            }

            // World transform to identity
            e.Device.Transform.World = Matrix.Identity;

            // Setup camera's view params
            camera.SetViewParameters(new Vector3(0.0f, 0.0f, -2.5f), Vector3.Empty);
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
            camera.SetRadius(3.0f, 1.0f, 6.0f);

            // Setup UI locations
            hud.SetLocation(desc.Width-170, 0);
            hud.SetSize(170,170);
            // Move non-HUD items to lower part of screen
            int y = desc.Height - 170;
            Control c;
            if ((c = hud.GetControl(CheckHDR)) != null)
                c.SetLocation(35, y);
            if ((c = hud.GetControl(ChangeMesh)) != null)
                c.SetLocation(35, y += 24);
            if ((c = hud.GetControl(ResetParams)) != null)
                c.SetLocation(35, y += 24);
            if ((c = hud.GetControl(SliderLightText)) != null)
                c.SetLocation(35, y += 35);
            if ((c = hud.GetControl(SliderLight)) != null)
                c.SetLocation(35, y += 17);
            if ((c = hud.GetControl(SliderReflectText)) != null)
                c.SetLocation(35, y += 24);
            if ((c = hud.GetControl(SliderReflect)) != null)
                c.SetLocation(35, y += 17);

            try
            {
                // Create the cube textures
                cubeMapFloating[0] = new CubeTexture(e.Device, EnvironmentMapSize, 1, Usage.RenderTarget,
                    Format.A16B16G16R16F, Pool.Default);
            }
            catch
            {
                // Create 2 G16R16 textures as fallback
                cubeMapFloating[0] = new CubeTexture(e.Device, EnvironmentMapSize, 1, Usage.RenderTarget,
                    Format.G16R16F, Pool.Default);
                cubeMapFloating[1] = new CubeTexture(e.Device, EnvironmentMapSize, 1, Usage.RenderTarget,
                    Format.G16R16F, Pool.Default);
            }

            // Create the integer cube map
            cubeMapInt32 = new CubeTexture(e.Device, EnvironmentMapSize, 1, Usage.RenderTarget,
                Format.A8R8G8B8, Pool.Default);

            // Create the stencil buffer to be used with the cube textures
            DeviceSettings settings = sampleFramework.DeviceSettings;
            depthCube = e.Device.CreateDepthStencilSurface(EnvironmentMapSize, EnvironmentMapSize, 
                settings.presentParams.AutoDepthStencilFormat, MultiSampleType.None, 0, true);

			// Setup the current group information
			currentTechGroup = new TechniqueGroup[2]; for (int i = 0; i < 2; i++) currentTechGroup[i] = groupFloating[i];
			currentCubeTexture = new CubeTexture[2];  for (int i = 0; i < 2; i++) currentCubeTexture[i] = cubeMapFloating[i];
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

            // Now the textures/surfaces
            if (cubeMapFloating[0] != null)
                cubeMapFloating[0].Dispose();
            if (cubeMapFloating[1] != null)
                cubeMapFloating[1].Dispose();
            if (cubeMapInt32 != null)
                cubeMapInt32.Dispose();
            if (depthCube != null)
                depthCube.Dispose();
            
            // Set null
            cubeMapFloating[0] = cubeMapFloating[1] = cubeMapInt32 = null;
            depthCube = null;
        }

        /// <summary>
        /// This event will be fired immediately after the Direct3D device has 
        /// been destroyed, which generally happens as a result of application termination or 
        /// windowed/full screen toggles. Resources created in the OnCreateDevice event 
        /// should be released here, which generally includes all Pool.Managed resources. 
        /// </summary>
        private void OnDestroyDevice(object sender, EventArgs e)
        {
            if (vertexDecl != null)
                vertexDecl.Dispose();
            
            if (roomMesh != null)
                roomMesh.Dispose();
            
            if (lightMesh != null)
                lightMesh.Dispose();

            for (int i = 0; i < MeshFileNames.Length; i++)
                if (envMeshes[i] != null)
                    envMeshes[i].Mesh.Dispose();
            for (int i = 0; i < orbiters.Length; i++)
                if (orbiters[i] != null)
                    orbiters[i].Mesh.Dispose();
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

            // Update the orbiters
            for (int i = 0; i < orbiters.Length; i++)
                orbiters[i].Orbit(elapsedTime);
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

            RenderSceneIntoCubeMap(device, appTime);

            // Clear the render target and the zbuffer 
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, 0x000000ff, 1.0f, 0);
            try
            {
                // Begin the scene
                device.BeginScene();
                beginSceneCalled = true;

                RenderScene(device, camera.ViewMatrix, camera.ProjectionMatrix, currentTechGroup[0], true, appTime);

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

        /// <summary>Set up the cube map by rendering the scene into it</summary>
        private void RenderSceneIntoCubeMap(Device device, double appTime)
        {
            // The projection matrix has a FOV of 90 degrees and asp ratio of 1
            Matrix proj = Matrix.PerspectiveFovLH((float)Math.PI * 0.5f, 1.0f, 0.01f, 100.0f);
            Matrix viewDir = camera.ViewMatrix;
            viewDir.M41 = viewDir.M42 = viewDir.M43 = 0.0f;

            using (Surface oldRenderTarget = device.GetRenderTarget(0))
            {
                Surface oldDepthStencil = null;

                // Ignore any exceptions for this call
                DirectXException.IgnoreExceptions();
                oldDepthStencil = device.DepthStencilSurface;
                DirectXException.EnableExceptions();

                // If the device has a depth-stencil buffer, use
                // the depth stencil buffer created for the cube textures.
                if (oldDepthStencil != null)
                    device.DepthStencilSurface = depthCube;

                for (int c = 0; c < numberCubes; ++c)
                {
                    for (int f = 0; f < 6; ++f)
                    {
                        using(Surface s = currentCubeTexture[c].GetCubeMapSurface((CubeMapFace)f, 0))
                        {
                            device.SetRenderTarget(0, s);
                            Matrix viewMatrix = viewDir * Utility.GetCubeMapViewMatrix(f);

                            // Clear the scene
                            device.Clear(ClearFlags.ZBuffer, 0x000000ff, 1.0f, 0);
                            bool beginSceneCalled = false;

                            try
                            {
                                // Begin the scene
                                device.BeginScene();
                                beginSceneCalled = true;

                                RenderScene(device, viewMatrix, proj, currentTechGroup[c], false, appTime);
                            }
                            finally
                            {
                                if (beginSceneCalled)
                                    device.EndScene();
                            }

                        }
                    }
                }

                // Restore the depth-stencil buffer and render target
                if (oldDepthStencil != null)
                {
                    device.DepthStencilSurface = oldDepthStencil;
                    oldDepthStencil.Dispose();
                }
                device.SetRenderTarget(0, oldRenderTarget);
            }
        }
        /// <summary>Set up the cube map by rendering the scene into it</summary>
        private void RenderScene(Device device, Matrix view, Matrix proj, TechniqueGroup techGroup,
            bool renderEnvMappedMesh, double appTime)
        {
            int passes;
            Matrix worldView;

            // Set the projection matrix
            effect.SetValue(projHandle, proj);

            // Write camera-space light positions to effect
            for (int i = 0; i < lightPositions.Length; i++)
            {
                // Animate the lights
                float disp = ( 1.0f + (float)Math.Cos( (float)Math.IEEERemainder( (float)appTime, (float)Math.PI ) ) ) * 0.5f * lights[i].MoveDistance; // Distance to move
                Vector4 move = lights[i].MoveDirection * disp; // In vector form
                lights[i].WorkingMatrix = Matrix.Translation(move.X, move.Y, move.Z); // Matrix form
                lights[i].WorkingMatrix = lights[i].WorldMatrix * lights[i].WorkingMatrix;
                move += lights[i].Position; // Animated world coords
                lightPositions[i] = Vector4.Transform(move, view);
            }
            effect.SetValue(lightPositionHandle, lightPositions);

            //
            // Render the environment-mapped mesh if specified
            //

            if (renderEnvMappedMesh)
            {
                effect.Technique = techGroup.RenderEnvMap;
                // Combine the offset and scaling transformation with
                // rotation from the camera to form the final
                // world transformation matrix.
                worldView = envMeshes[currentMesh].WorldMatrix * camera.WorldMatrix;
                worldView *= view;

                effect.SetValue(worldViewHandle, worldView);
                passes = effect.Begin(0);

                // Set the cube textures
                for (int i = 0; i < numberCubes; i++)
                    effect.SetValue(cubeTextureHandle[i], currentCubeTexture[i]);

                for (int iPass = 0; iPass < passes; iPass++)
                {
                    effect.BeginPass(iPass);
                    Mesh m = envMeshes[currentMesh].Mesh.LocalMesh;
                    for (int i = 0; i < envMeshes[currentMesh].Mesh.NumberMaterials; ++i)
                        m.DrawSubset(i);
                    effect.EndPass();
                }
                effect.End();
            }

            //
            // Render light spheres
            //
            effect.Technique = techGroup.RenderLight;
            passes = effect.Begin(0);
            for (int p = 0; p < passes; ++p)
            {
                effect.BeginPass(p);
                for (int i = 0; i < NumberLights; ++i)
                {
                    worldView = lights[i].WorkingMatrix * view;
                    effect.SetValue(worldViewHandle, worldView);
                    effect.CommitChanges();
                    lightMesh.Render(device);
                }
                effect.EndPass();
            }
            effect.End();

            //
            // Render the rest of the scene
            //
            effect.Technique = techGroup.RenderScene;
            passes = effect.Begin(0);
            for (int p = 0; p < passes; ++p)
            {
                effect.BeginPass(p);
                //
                // Orbiters
                //
                for (int i = 0; i < orbiters.Length; i++)
                {
                    worldView = orbiters[i].WorldMatrix * view;
                    effect.SetValue(worldViewHandle, worldView);

                    // Iterate through each subset and render with its texture
                    for (int m = 0; m < orbiters[i].Mesh.NumberMaterials; ++m)
                    {
                        effect.SetValue(sceneTextureHandle, orbiters[i].Mesh.GetTexture(m));
                        effect.CommitChanges();
                        orbiters[i].Mesh.LocalMesh.DrawSubset(m);
                    }
                }

                //
                // The room object (walls, floor, ceiling)
                //
                effect.SetValue(worldViewHandle, view);
                // Iterate through each subset and render with its texture
                for (int m = 0; m < roomMesh.NumberMaterials; ++m)
                {
                    effect.SetValue(sceneTextureHandle, roomMesh.GetTexture(m));
                    effect.CommitChanges();
                    roomMesh.LocalMesh.DrawSubset(m);
                }
                effect.EndPass();
            }
            effect.End();
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


            txtHelper.SetForegroundColor(System.Drawing.Color.Orange);
            // Draw help
            if (isHelpShowing)
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height-15*12);
                txtHelper.DrawTextLine("Controls:");

                txtHelper.SetInsertionPoint(40, sampleFramework.BackBufferSurfaceDescription.Height-15*11);
                txtHelper.DrawTextLine("Rotate object: Left drag mouse");
                txtHelper.DrawTextLine("Adjust camera: Right drag mouse\nZoom In/Out: Mouse wheel");
                txtHelper.DrawTextLine("\nAdjust reflectivity: E,D\nAdjust light intensity: W,S");
                txtHelper.DrawTextLine("\n\nHide help: F1");
                txtHelper.DrawTextLine("\n\nQuit: ESC");
            }
            else
            {
                txtHelper.DrawTextLine("Press F1 for help");
            }
            
            txtHelper.SetForegroundColor(System.Drawing.Color.LightGreen);
            txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height - 48);
            txtHelper.DrawTextLine("Cube map format");
            txtHelper.DrawTextLine("Material reflectivity ( e/E, d/D )");
            txtHelper.DrawTextLine("Light intensity ( w/W, s/S )");
            txtHelper.SetInsertionPoint(190, sampleFramework.BackBufferSurfaceDescription.Height - 48);
            txtHelper.DrawTextLine("{0}\n{1}\n{2}", usingFloatCubeMap ? "Floating-point (D3D9 / HDR)" : 
                "Integer (D3D8)", reflectivity, lightIntensity.X);

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

            //
            // Use NativeMethods.WindowMessage.Character to handle parameter adjustment so
            // that we can control the granularity based on
            // the letter cases.
            if (msg == NativeMethods.WindowMessage.Character)
            {
                char c = (char)wParam.ToInt32();
                switch(c)
                {
                    case 'W':
                    case 'w':
                        if (c == 'w')
                            lightIntensity += new Vector4(0.1f, 0.1f, 0.1f, 0.0f);
                        else
                            lightIntensity += new Vector4(10.0f, 10.0f, 10.0f, 0.0f);

                        if (lightIntensity.X > 150.0f)
                        {
                            lightIntensity.X =
                            lightIntensity.Y =
                            lightIntensity.Z = 150.0f;
                        }

                        effect.SetValue(lightIntensityHandle, lightIntensity);
                        UpdateUiWithChanges();
                        break;
                    case 'S':
                    case 's':
                        if (c == 's')
                            lightIntensity -= new Vector4(0.1f, 0.1f, 0.1f, 0.0f);
                        else
                            lightIntensity -= new Vector4(10.0f, 10.0f, 10.0f, 0.0f);

                        if (lightIntensity.X < 0.0f)
                        {
                            lightIntensity.X =
                            lightIntensity.Y =
                            lightIntensity.Z = 0.0f;
                        }

                        effect.SetValue(lightIntensityHandle, lightIntensity);
                        UpdateUiWithChanges();
                        break;
                    case 'D':
                    case 'd':
                        if (c == 'd')
                            reflectivity -= 0.01f;
                        else
                            reflectivity -= 0.1f;

                        if (reflectivity < 0.0f)
                            reflectivity = 0.0f;

                        effect.SetValue(reflectivityHandle, reflectivity);
                        UpdateUiWithChanges();
                        break;
                    case 'E':
                    case 'e':
                        if (c == 'e')
                            reflectivity += 0.01f;
                        else
                            reflectivity += 0.1f;

                        if (reflectivity > 1.0f)
                            reflectivity = 1.0f;

                        effect.SetValue(reflectivityHandle, reflectivity);
                        UpdateUiWithChanges();
                        break;
                }
            }
            return IntPtr.Zero;
        }

        /// <summary>Load mesh from file and convert vertices to our format</summary>
        private FrameworkMesh LoadMesh(Device device, string filename)
        {
            FrameworkMesh mesh = new FrameworkMesh(device, filename);
            mesh.SetVertexDeclaration(device, decl);

            return mesh;
        }

        /// <summary>Initializes the application</summary>
        public void InitializeApplication()
        {
            // Change CheckBox default visual style
            Element e = hud.GetDefaultElement(ControlType.CheckBox, 0);
            e.FontColor.States[(int)ControlState.Normal] = new ColorValue(0, 1.0f, 0, 1.0f);

            // Change Static default visual style
            e = hud.GetDefaultElement(ControlType.StaticText, 0);
            e.textFormat = DrawTextFormat.Left | DrawTextFormat.Center;
            e.FontColor.States[(int)ControlState.Normal] = new ColorValue(0, 1.0f, 0, 1.0f);

            int y = 10;
            // Initialize the dialogs
            Button fullScreen = hud.AddButton(ToggleFullscreen,"Toggle full screen", 35, y, 125,22);
            Button toggleRef = hud.AddButton(ToggleReference,"Toggle reference (F3)", 35, y += 24, 125,22);
            Button changeDevice = hud.AddButton(ChangeDevice,"Change Device (F2)", 35, y += 24, 125,22);
            Button changeMesh = hud.AddButton(ChangeMesh, "Change Mesh (N)", 35, y+=24, 125, 22, System.Windows.Forms.Keys.N, false);
            Button resetParams = hud.AddButton(ResetParams, "Reset Paramters (R)", 35, y+=24, 125, 22, System.Windows.Forms.Keys.R, false);
            Checkbox checkHdr = hud.AddCheckBox(CheckHDR, "Use HDR Texture (F)", 35, y+=24, 130, 22, true, System.Windows.Forms.Keys.F, false);
            hud.AddStatic(SliderLightText, "Light Intensity", 35, y+=24, 125, 16);
            Slider lightSlider = hud.AddSlider(SliderLight, 35, y += 17, 125, 22, 0, 1500, 0, false);
            hud.AddStatic(SliderReflectText, "Reflectivity", 35, y+=24, 125, 16);
            Slider reflectSlider = hud.AddSlider(SliderReflect, 35, y += 17, 125, 22);

            // Hook the button events for when these items are clicked
            fullScreen.Click += new EventHandler(OnFullscreenClicked);
            toggleRef.Click += new EventHandler(OnRefClicked);
            changeDevice.Click += new EventHandler(OnChangeDevicClicked);
            // Hook the other events
            checkHdr.Changed += new EventHandler(OnChangeHdr);
            changeMesh.Click += new EventHandler(OnChangeMesh);
            resetParams.Click += new EventHandler(OnResetParams);
            lightSlider.ValueChanged += new EventHandler(OnLightIntensityChanged);
            reflectSlider.ValueChanged += new EventHandler(OnReflectChanged);


            // Reset the parameters
            ResetParameters();

            // Initialize the camera parameters
            camera.SetModelCenter(Vector3.Empty);
			camera.IsPositionMovementEnabled = false;

            // Set the light positions
            lights[0].Position = new Vector4(-3.5f, 2.3f, -4.0f, 1.0f);
            lights[0].MoveDirection = new Vector4(0.0f, 0.0f, 1.0f, 0.0f);
            lights[0].MoveDistance = 8.0f;
            if (NumberLights > 1)
            {
                lights[1].Position = new Vector4(3.5f, 2.3f,  4.0f, 1.0f);
                lights[1].MoveDirection = new Vector4(0.0f, 0.0f, -1.0f, 0.0f);
                lights[1].MoveDistance = 8.0f;
            }
            if (NumberLights > 2)
            {
                lights[2].Position = new Vector4(-3.5f, 2.3f,  4.0f, 1.0f);
                lights[2].MoveDirection = new Vector4(1.0f, 0.0f, 0.0f, 0.0f);
                lights[2].MoveDistance = 7.0f;
            }
            if (NumberLights > 3)
            {
                lights[3].Position = new Vector4(3.5f, 2.3f, -4.0f, 1.0f);
                lights[3].MoveDirection = new Vector4(-1.0f, 0.0f, 0.0f, 0.0f);
                lights[3].MoveDistance = 7.0f;
            }

        }

        /// <summary>Reset light and material parameters to default values</summary>
        private void ResetParameters()
        {
            usingFloatCubeMap = true;
            reflectivity = 0.4f;
            lightIntensity = new Vector4(24.0f, 24.0f, 24.0f, 24.0f);

            // Update the effect if it exists
            if (effect != null)
            {
                effect.SetValue(reflectivityHandle, reflectivity);
                effect.SetValue(lightIntensityHandle, lightIntensity);
            }

            UpdateUiWithChanges();
        }

        /// <summary>Updates the user interface with the current state of the scene</summary>
        private void UpdateUiWithChanges()
        {
            StaticText s = hud.GetStaticText(SliderLightText);
            s.SetText("Light Intensity: " + lightIntensity.X.ToString());
            s = hud.GetStaticText(SliderReflectText);
            s.SetText("Reflectivity: " + reflectivity.ToString());

            Slider sl = hud.GetSlider(SliderLight);
            sl.Value = (int)(lightIntensity.X * 10.0f);
            sl = hud.GetSlider(SliderReflect);
            sl.Value = (int)(reflectivity * 100.0f);
            Checkbox box = hud.GetCheckbox(CheckHDR);
            box.IsChecked = usingFloatCubeMap;
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
                HDRCubeMap sample = new HDRCubeMap(sampleFramework);
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
                    sampleFramework.CreateWindow("HDRCubeMap");
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

        #region Sample UI Event Handlers
        private void OnChangeHdr(object sender, EventArgs e)
        {
            Checkbox cb = sender as Checkbox;
            usingFloatCubeMap = cb.IsChecked;
            if (usingFloatCubeMap)
            {
                numberCubes = numberFloatingCube;
				// Setup the current group information
				for (int i = 0; i < 2; i++) currentTechGroup[i] = groupFloating[i];
				for (int i = 0; i < 2; i++) currentCubeTexture[i] = cubeMapFloating[i];
            }
            else
            {
                numberCubes = 1;
                currentCubeTexture[0] = cubeMapInt32;
                currentTechGroup[0] = groupInt32;
            }
        }

        private void OnChangeMesh(object sender, EventArgs e)
        {
            if(++currentMesh == MeshFileNames.Length)
                currentMesh = 0;
        }

        private void OnResetParams(object sender, EventArgs e)
        {
            ResetParameters();
        }

        private void OnLightIntensityChanged(object sender, EventArgs e)
        {
            Slider sl = sender as Slider;
            lightIntensity.X = lightIntensity.Y = lightIntensity.Z = sl.Value * 0.1f;
            if (effect != null)
                effect.SetValue(lightIntensityHandle, lightIntensity);
            UpdateUiWithChanges();
        }

        private void OnReflectChanged(object sender, EventArgs e)
        {
            Slider sl = sender as Slider;
            reflectivity = sl.Value * 0.01f;
            if (effect != null)
                effect.SetValue(reflectivityHandle, reflectivity);
            UpdateUiWithChanges();
        }
        #endregion
    }
}
