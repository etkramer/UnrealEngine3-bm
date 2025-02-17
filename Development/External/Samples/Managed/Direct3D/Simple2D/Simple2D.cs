//-----------------------------------------------------------------------------
// File: Simple2D.cs
//
// A very straightforward example of using the Microsoft.DirectX.Direct3D.Sprite
// interface to draw 2D sprites in screen coordinates.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;
using System.Collections;

namespace Simple2DSample
{
    /// <summary>SpriteSample Sample Class</summary>
    public class Simple2D : IFrameworkCallback, IDeviceCreation
    {
        //Private struct used for a virtual "game board"
        private struct Location
        {
            public TileIndex Ground;
            public TileIndex Middle;
            public TileIndex Foreground;
        }

        //Hard-coded tile names
        private enum TileIndex
        {
            None = -1,
            Grass = 0,
            Rocks = 1,
            Flowers = 2,
            Cloud = 3
        }


        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public Simple2D(Framework f)
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
        private Font statsFont = null; // Font for drawing text
        private Sprite textSprite = null; // Sprite for batching text calls
        private ModelViewerCamera camera = new ModelViewerCamera(); // A model viewing camera
        private bool isHelpShowing = true; // If true, renders the UI help text
        private Dialog hud = null; // dialog for standard controls
        private Dialog sampleUi = null; // dialog for sample specific controls

        // HUD Ui Control constants
        private const int ToggleFullscreen = 1;
        private const int ToggleReference = 3;
        private const int ChangeDevice = 4;
        private const int ButtonToggleCanvas = ChangeDevice + 1;
        private const int CheckUnsafeParticle = ChangeDevice + 2;
        private const int ButtonToggleTiles = ChangeDevice + 3;
        private const int ButtonToggleAnimatedSprite = ChangeDevice + 4;
        private const int ButtonToggleMovingSprite = ChangeDevice + 5;

        //Sample specifics
        private Sprite batchSprite;
        private UniformTileSet exampleTileSet;
        private AnimatedSprite animatedSprite;
        private MovingSprite movingSprite;
        private Canvas canvas;
        private Texture donutTexture, tileTexture, mosquitoTexture;
        private Location[,] gameBoard = new Location[1000, 1000];
        private TileInstance[] visibleTiles;
        private Random rand = new Random();
        private const float fps = 60;
        private float timeToAdvance = 0;
        private SurfaceDescription bbufferDesc;
        private ParticleEffect effect;
        private Vector2 upperLeft;
        private float zoomFactor;
        private const int tileWidth = 64;
        private const int tileHeight = 64;
        private bool particleIsUsingUnsafeMethods;
        private bool canvasEnabled, tilesEnabled, animatedSpriteEnabled, movingSpriteEnabled;


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
            if ((!caps.DeviceCaps.SupportsHardwareTransformAndLight) ||
                (caps.VertexShaderVersion < new Version(1, 1)))
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
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "SpriteSample");
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

            // Setup the camera's view parameters
            camera.SetViewParameters(new Vector3(0.0f, 0.0f, -5.0f), Vector3.Empty);


            //initialized the tile-view
            upperLeft = new Vector2(0, 0);
            zoomFactor = 1;

            //Generate a randmoized "game board" using the tiles in the tile set.
            for (int i = 0; i < gameBoard.GetLength(0); i++)
            {
                for (int j = 0; j < gameBoard.GetLength(1); j++)
                {
                    //add a tile every time a location is set to an index other than "None"
                    gameBoard[i, j].Ground = TileIndex.Grass;
                    if (rand.Next(3) == 1)
                    {
                        if (rand.Next(5) == 1) gameBoard[i, j].Middle = TileIndex.Flowers;
                        else gameBoard[i, j].Middle = TileIndex.Rocks;

                    }
                    else gameBoard[i, j].Middle = TileIndex.None;
                    if (rand.Next(5) == 1) gameBoard[i, j].Foreground = TileIndex.Cloud;
                    else gameBoard[i, j].Foreground = TileIndex.None;
                }
            }

            //Since we're casting the value 0xFF000000 as an Int32, we must specify 
            //unchecked to avoid compiler errors.
            unchecked
            {

                //The texture used is in the "Managed" pool, meaning that it will automatically
                //restore itself on a Device.Reset().  This kind of texture requires less manual management than
                //a default pool texture.
                tileTexture = TextureLoader.FromFile(e.Device, Utility.FindMediaFile("tiles.tga"), 320, 64, 1,
                    Usage.None, Format.A8R8G8B8, Pool.Managed, Filter.None, Filter.None, (int)0xFF000000);


                //create the example tile set
                exampleTileSet = new UniformTileSet(tileTexture, 256, 64);
                exampleTileSet.CreateTiles(tileWidth, tileHeight, 0, 0, 4, 1);
                UpdateVisibleTiles();


                //Create the texture for the moving sprite
                mosquitoTexture = TextureLoader.FromFile(e.Device, Utility.FindMediaFile("mosquito.bmp"), 160, 100, 0,
                    Usage.None, Format.A8R8G8B8, Pool.Managed, Filter.None, Filter.None, (int)(0xFF000000));

                movingSprite = new MovingSprite(mosquitoTexture, new System.Drawing.Rectangle(0, 0, 160, 100), rand, 200f);

                //create the texture for our animated sprite
                donutTexture = TextureLoader.FromFile(e.Device, Utility.FindMediaFile("donut.bmp"), 640, 384, 0,
                    Usage.None, Format.A8R8G8B8, Pool.Managed, Filter.None, Filter.None, (int)(0xFF000000));

                animatedSprite = new AnimatedSprite(donutTexture, 64, 64, 6, 10, 60, 0);


            }

            //Create a Canvas and a particle effect object for the 
            //canvas-based per-pixel effect
            canvas = new Canvas(e.Device, 640, 480);
            effect = new ParticleEffect(6, 6, canvas, rand, 4f, 20);
        }

        /// <summary>
        /// This function is an example of visibility determination for uniformly sized tiles.
        /// This ensures that Sprite.Draw() is only called on sprites that are actually visible,
        /// rather than wasting a bunch of calls to elements that are offscreen.  This function
        /// determines what part of the gameboard is visible, and populates that data (with
        /// positional information) to a "visible sprites" array wich is passed to the 
        /// tile drawing method in OnFrameRender().
        /// </summary>
        private void UpdateVisibleTiles()
        {
            //first determine the number of tiles in view
            float displayWidth = (float)sampleFramework.BackBufferSurfaceDescription.Width;
            float displayHeight = (float)sampleFramework.BackBufferSurfaceDescription.Height;
            float adjustedTileWidth = (float)tileWidth * zoomFactor;
            float adjustedTileHeight = (float)tileHeight * zoomFactor;


            //determine where we are in the location map
            int leftTile = (int)(upperLeft.X / adjustedTileWidth) - 1;
            int topTile = (int)(upperLeft.Y / adjustedTileHeight) - 1;
            int rightTile = (int)((upperLeft.X / adjustedTileWidth) + displayWidth / adjustedTileWidth) + 1;
            int bottomTile = (int)((upperLeft.Y / adjustedTileHeight) + displayHeight / adjustedTileHeight) + 1;

            //clamp the values to the gameboard size
            if (leftTile < 0) leftTile = 0;
            if (rightTile < 0) rightTile = 0;
            if (topTile < 0) topTile = 0;
            if (bottomTile < 0) bottomTile = 0;
            if (leftTile > (gameBoard.GetLength(0) - 1)) leftTile = gameBoard.GetLength(0) - 1;
            if (rightTile > (gameBoard.GetLength(0) - 1)) rightTile = gameBoard.GetLength(0) - 1;
            if (topTile > (gameBoard.GetLength(1) - 1)) topTile = gameBoard.GetLength(1) - 1;
            if (bottomTile > (gameBoard.GetLength(1) - 1)) bottomTile = gameBoard.GetLength(1) - 1;

            //get the total number of visible locations (for debugging purposes only)
            int numLocations = (rightTile - leftTile + 1) * (bottomTile - topTile + 1);

            //find out how many tiles need to be drawn
            int numTiles = 0;
            for (int i = leftTile; i <= rightTile; i++)
            {
                for (int j = topTile; j <= bottomTile; j++)
                {
                    //add a tile every time a location is set to an index other than "None"
                    if (gameBoard[i, j].Ground != TileIndex.None) numTiles++;
                    if (gameBoard[i, j].Middle != TileIndex.None) numTiles++;
                    if (gameBoard[i, j].Foreground != TileIndex.None) numTiles++;
                }

            }

            //create a new array if necessary
            //we want to avoid initializing a new array if possible to save on
            //garbage collections
            if ((visibleTiles == null) || (numTiles != visibleTiles.Length))
            {
                //An alternative is to figure out how many tiles could be possible ever during gamplay and
                //set that as the visible array at startup.  That kind of functionality should be more efficient
                //but may waste some memory.  Profiling can indicate if there is a valid savings from such
                //an operation.
                visibleTiles = new TileInstance[numTiles];
            }

            //This loop populates the visible tiles with information
            //from the game board.
            int ti = 0;
            for (int i = leftTile; i <= rightTile; i++)
            {
                for (int j = topTile; j <= bottomTile; j++)
                {
                    for (int l = 0; l < 3; l++)
                    {

                        int tileIndex = -1;
                        switch (l)
                        {
                            case 0:
                                tileIndex = (int)gameBoard[i, j].Ground;
                                break;
                            case 1:
                                tileIndex = (int)gameBoard[i, j].Middle;
                                break;
                            case 2:
                                tileIndex = (int)gameBoard[i, j].Foreground;
                                break;

                        }
                        if (tileIndex != -1)
                        {
                            visibleTiles[ti].TileIndex = tileIndex;
                            //these two lines set the position of the tile in screen coordinates
                            //(relative to the upper-left-hand corner
                            visibleTiles[ti].X = (i * adjustedTileWidth) - upperLeft.X;
                            visibleTiles[ti].Y = (j * adjustedTileHeight) - upperLeft.Y;
                            visibleTiles[ti].Z = 0;
                            visibleTiles[ti].Color = System.Drawing.Color.White;
                            ti++;
                        }
                    }
                }
            }
            //The last step is to tell the tile set what scale to draw the tiles at.
            exampleTileSet.SetTileDestinationSize(adjustedTileWidth, adjustedTileHeight);
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
            bbufferDesc = e.BackBufferDescription;
            // Create a sprite to help batch calls when drawing many lines of text
            textSprite = new Sprite(e.Device);

            // Setup the camera's projection parameters
            float aspectRatio = (float)bbufferDesc.Width / (float)bbufferDesc.Height;
            camera.SetProjectionParameters((float)Math.PI / 4, aspectRatio, 0.1f, 1000.0f);
            camera.SetWindow(bbufferDesc.Width, bbufferDesc.Height);

            // Setup UI locations
            hud.SetLocation(bbufferDesc.Width - 170, 0);
            hud.SetSize(170, 170);
            sampleUi.SetLocation(bbufferDesc.Width - 170, bbufferDesc.Height - 350);
            sampleUi.SetSize(170, 300);

            batchSprite = new Sprite(e.Device);

            //set up the static gameboard
            UpdateVisibleTiles();

            movingSprite.SetScreenSize(bbufferDesc.Width, bbufferDesc.Height);
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
            if (batchSprite != null)
            {
                batchSprite.Dispose();
                batchSprite = null;
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
            if (mosquitoTexture != null)
            {
                mosquitoTexture.Dispose();
                mosquitoTexture = null;
            }
            if (donutTexture != null)
            {
                donutTexture.Dispose();
                donutTexture = null;
            }
            if (tileTexture != null)
            {
                tileTexture.Dispose();
                tileTexture = null;
            }
            if (canvas != null)
            {
                canvas.Dispose();
                canvas = null;
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
            // Update the camera's position based on user input 
            camera.FrameMove(elapsedTime);



            if (movingSpriteEnabled) movingSprite.Move(elapsedTime);

            //This technique is called "throttling" and limits events  to a
            //specific framerate.  This technique works well for "whole number" advancement
            //such as advancing a frame of animation.
            timeToAdvance -= elapsedTime;
            if (timeToAdvance < 0)
            {
                //The Particle effect was far more efficient using an integer
                //implementation, so we are using throttled rendering
                if (canvasEnabled)
                {
                    if (particleIsUsingUnsafeMethods)
                        effect.UpdateUnsafe();
                    else
                        effect.Update();
                }


                if (animatedSpriteEnabled) animatedSprite.AdvanceFrame();

                //throttling is simple, but can cause jittery animation and
                //perceived slowdown under certain conditions (such as low framerate)
                timeToAdvance += 1 / fps;
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
            device.Clear(ClearFlags.ZBuffer | ClearFlags.Target, 0x00, 1.0f, 0);
            try
            {
                device.BeginScene();
                beginSceneCalled = true;

                //All draw calls are done between Begin/End pairs on the sprite interface
                //For this example, the only sprite flag is AlphaBlend.  The sample
                //does not need the sprite engine to sort by z-values or textures (this is
                //handled automatically in the various drawing algorithms).
                batchSprite.Begin(SpriteFlags.AlphaBlend);

                if (tilesEnabled)
                    exampleTileSet.DrawTiles(visibleTiles, batchSprite);

                if (animatedSpriteEnabled)
                    animatedSprite.Draw(batchSprite, new Vector3(bbufferDesc.Width / 2, bbufferDesc.Height / 2, 0f), System.Drawing.Color.White);

                if (canvasEnabled)
                    canvas.Draw(batchSprite, System.Drawing.Rectangle.Empty,
                        new System.Drawing.Rectangle(0, 0, bbufferDesc.Width, bbufferDesc.Height),
                        System.Drawing.Color.White);

                if (movingSpriteEnabled)
                    movingSprite.Draw(batchSprite);


                //End calls flush() internally, which sorts the sprites and draws them
                batchSprite.End();

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
            txtHelper.SetInsertionPoint(5, 5);
            txtHelper.SetForegroundColor(System.Drawing.Color.Yellow);
            txtHelper.DrawTextLine(sampleFramework.FrameStats);
            txtHelper.DrawTextLine(sampleFramework.DeviceStats);

            txtHelper.SetForegroundColor(System.Drawing.Color.White);
            if (tilesEnabled && (visibleTiles != null))
                txtHelper.DrawTextLine("Tiles Being Drawn: " + visibleTiles.Length);

            // Draw help
            if (isHelpShowing)
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height - 15 * 6);
                txtHelper.SetForegroundColor(System.Drawing.Color.DarkOrange);
                txtHelper.DrawTextLine("Controls (F1 to hide):");

                txtHelper.SetInsertionPoint(40, sampleFramework.BackBufferSurfaceDescription.Height - 15 * 5);
                txtHelper.DrawTextLine("Move Tile View: Up Arrow, Down Arrow, Left Arrow, Right Arrow");
                txtHelper.DrawTextLine("Zoom Tiles: Z, X");
                txtHelper.DrawTextLine("Quit: Esc");
                txtHelper.DrawTextLine("Hide help: F1");
            }
            else
            {
                txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height - 15 * 2);
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
            switch (e.KeyCode)
            {
                case System.Windows.Forms.Keys.F1:
                    isHelpShowing = !isHelpShowing;
                    break;
                case System.Windows.Forms.Keys.Up:
                    if (tilesEnabled)
                    {
                        upperLeft.Y -= 20f;
                        UpdateVisibleTiles();
                    }
                    break;
                case System.Windows.Forms.Keys.Down:
                    if (tilesEnabled)
                    {
                        upperLeft.Y += 20f;
                        UpdateVisibleTiles();
                    }
                    break;
                case System.Windows.Forms.Keys.Left:
                    if (tilesEnabled)
                    {
                        upperLeft.X -= 20f;
                        UpdateVisibleTiles();
                    }
                    break;
                case System.Windows.Forms.Keys.Right:
                    if (tilesEnabled)
                    {
                        upperLeft.X += 20f;
                        UpdateVisibleTiles();
                    }
                    break;

                case System.Windows.Forms.Keys.Z:
                    if (tilesEnabled)
                    {
                        //This section corrects the virtual "eye" position when zooming in
                        Vector2 screenCenter = new Vector2(bbufferDesc.Width / 2f, bbufferDesc.Height / 2f);
                        Vector2 gameCenter = (screenCenter + upperLeft) * (1f / zoomFactor);
                        gameCenter.X = (float)Math.Round(gameCenter.X);
                        gameCenter.Y = (float)Math.Round(gameCenter.Y);

                        zoomFactor += .1f;
                        if (zoomFactor > 4f) zoomFactor = 4f;

                        upperLeft = (gameCenter * zoomFactor) - screenCenter;

                        UpdateVisibleTiles();
                    }
                    break;
                case System.Windows.Forms.Keys.X:
                    if (tilesEnabled)
                    {
                        //This section corrects the virtual "eye" position when zooming out
                        Vector2 screenCenter = new Vector2(bbufferDesc.Width / 2f, bbufferDesc.Height / 2f);
                        Vector2 gameCenter = (screenCenter + upperLeft) * (1f / zoomFactor);
                        gameCenter.X = (float)Math.Round(gameCenter.X);
                        gameCenter.Y = (float)Math.Round(gameCenter.Y);

                        zoomFactor -= .1f;
                        if (zoomFactor < .3f) zoomFactor = .3f;

                        upperLeft = (gameCenter * zoomFactor) - screenCenter;

                        UpdateVisibleTiles();
                    }
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
            Button fullScreen = hud.AddButton(ToggleFullscreen, "Toggle full screen", 35, y, 125, 22);
            Button toggleRef = hud.AddButton(ToggleReference, "Toggle reference (F3)", 35, y += 24, 125, 22);
            Button changeDevice = hud.AddButton(ChangeDevice, "Change Device (F2)", 35, y += 24, 125, 22);
            // Hook the button events for when these items are clicked
            fullScreen.Click += new EventHandler(OnFullscreenClicked);
            toggleRef.Click += new EventHandler(OnRefClicked);
            changeDevice.Click += new EventHandler(OnChangeDevicClicked);


            y = 10;

            Button toggleFaiy = sampleUi.AddButton(ButtonToggleCanvas, "Toggle Canvas", 10, y += 24, 125, 22);
            Checkbox unsafeParticle = sampleUi.AddCheckBox(CheckUnsafeParticle, "Use Unsafe Methods", 30, y += 24, 125, 22, true);
            Button toggleTiles = sampleUi.AddButton(ButtonToggleTiles, "Toggle Tiles", 10, y += 24, 125, 22);
            Button toggleAnimatedSprite = sampleUi.AddButton(ButtonToggleAnimatedSprite, "Toggle Animated Sprite", 10, y += 24, 125, 22);
            Button toggleMovingSprite = sampleUi.AddButton(ButtonToggleMovingSprite, "Toggle Moving Sprite", 10, y += 24, 125, 22);

            toggleFaiy.Click += new EventHandler(OnToggleParticle);
            unsafeParticle.Changed += new EventHandler(OnChangeUnsafe);
            toggleTiles.Click += new EventHandler(OnToggleTiles);
            toggleAnimatedSprite.Click += new EventHandler(OnToggleAnimatedSprite);
            toggleMovingSprite.Click += new EventHandler(OnToggleMovingSprite);

            particleIsUsingUnsafeMethods = true;
            tilesEnabled = true;

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

        private void OnToggleParticle(object sender, EventArgs e)
        {
            canvasEnabled = !canvasEnabled;
        }

        private void OnToggleTiles(object sender, EventArgs e)
        {
            tilesEnabled = !tilesEnabled;
        }

        private void OnChangeUnsafe(object sender, EventArgs e)
        {
            particleIsUsingUnsafeMethods = sampleUi.GetCheckbox(CheckUnsafeParticle).IsChecked;
        }

        private void OnToggleAnimatedSprite(object sender, EventArgs e)
        {
            animatedSpriteEnabled = !animatedSpriteEnabled;
        }

        private void OnToggleMovingSprite(object sender, EventArgs e)
        {
            movingSpriteEnabled = !movingSpriteEnabled;
        }


        /// <summary>
        /// Entry point to the program. Initializes everything and goes into a message processing 
        /// loop. Idle time is used to render the scene.
        /// </summary>
        static int Main()
        {
            System.Windows.Forms.Application.EnableVisualStyles();
            using (Framework sampleFramework = new Framework())
            {
                Simple2D sample = new Simple2D(sampleFramework);
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
                    sampleFramework.Initialize(true, true, true); // Parse the command line, handle the default hotkeys, and show msgboxes
                    sampleFramework.CreateWindow("SpriteSample");
                    // Hook the keyboard event
                    sampleFramework.Window.KeyDown += new System.Windows.Forms.KeyEventHandler(sample.OnKeyEvent);
                    sampleFramework.CreateDevice(0, true, Framework.DefaultSizeWidth, Framework.DefaultSizeHeight,
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
