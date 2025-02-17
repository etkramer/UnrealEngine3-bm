//-----------------------------------------------------------------------------
// File: CustomUI.cs
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

namespace CustomUISample
{
    /// <summary>CustomUI Sample Class</summary>
    public class CustomUI : IFrameworkCallback, IDeviceCreation
    {
        #region Creation
        /// <summary>Create a new instance of the class</summary>
        public CustomUI(Framework f) 
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
        private Effect effect = null; // D3DX Effect Interface
        private ModelViewerCamera camera = new ModelViewerCamera(); // A model viewing camera
        private Dialog hud = null; // dialog for standard controls
        private Dialog sampleUi = null; // dialog for sample specific controls

        // Sample specific controls
        private FrameworkMesh mesh = null; // Background mesh
        private Matrix viewMatrix;

        // HUD Ui Control constants
        private const int ToggleFullscreen = 1;
        private const int ToggleReference = 3;
        private const int ChangeDevice = 4;
        // Sample UI Control constants
        private const int EditBoxControl = 5;
        private const int ComboBoxControl = 6;
        private const int StaticControl = 7;
        private const int OutputStaticControl = 8;
        private const int SliderControl = 9;
        private const int CheckBoxControl = 10;
        private const int ClearEditControl = 11;
        private const int RadioButton1A = 12;
        private const int RadioButton1B = 13;
        private const int RadioButton1C = 14;
        private const int RadioButton2A = 15;
        private const int RadioButton2B = 16;
        private const int RadioButton2C = 17;
        private const int ListBoxControl = 18;
        private const int ListBoxControlMulti = 19;

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
                settings.BehaviorFlags |= CreateFlags.PureDevice;

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

            // For the first device created if its a REF device, optionally display a warning dialog box
            if (settings.DeviceType == DeviceType.Reference)
            {
                Utility.DisplaySwitchingToRefWarning(sampleFramework, "CustomUI");
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
            string path = Utility.FindMediaFile("CustomUI.fx");
            effect = ResourceCache.GetGlobalInstance().CreateEffectFromFile(e.Device,
                path, null, null, shaderFlags, null);

            // Create the mesh
            mesh = new FrameworkMesh(e.Device, "misc\\cell.x");

            // Setup the camera's view parameters
            Vector3 eye = new Vector3(0.0f, 1.5f, -7.0f);
            Vector3 at = new Vector3(0.0f, 0.2f, 0.0f);
            camera.SetViewParameters(eye, at);
            viewMatrix = Matrix.LookAtLH(eye, at, Camera.UpDirection);
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
            sampleUi.SetLocation(0, 0);
            sampleUi.SetSize(desc.Width, desc.Height);

            sampleUi.GetControl(StaticControl).SetLocation((desc.Width-200)/2, desc.Height-300);
            sampleUi.GetControl(OutputStaticControl).SetSize(desc.Width - 170, desc.Height / 4);
            sampleUi.GetControl(EditBoxControl).SetLocation(20, desc.Height - 270);
            sampleUi.GetControl(EditBoxControl).SetSize(desc.Width - 40, 32);
            sampleUi.GetControl(SliderControl).SetLocation(10, desc.Height - 140);
            sampleUi.GetControl(CheckBoxControl).SetLocation(100, desc.Height - 50);
            sampleUi.GetControl(ClearEditControl).SetLocation(100, desc.Height - 25);
            sampleUi.GetControl(ComboBoxControl).SetLocation(20, desc.Height - 180);
            sampleUi.GetControl(RadioButton1A).SetLocation(desc.Width - 140, 110);
            sampleUi.GetControl(RadioButton1B).SetLocation(desc.Width - 140, 134);
            sampleUi.GetControl(RadioButton1C).SetLocation(desc.Width - 140, 158);
            sampleUi.GetControl(RadioButton2A).SetLocation(20, desc.Height - 100);
            sampleUi.GetControl(RadioButton2B).SetLocation(20, desc.Height - 76);
            sampleUi.GetControl(RadioButton2C).SetLocation(20, desc.Height - 52);
            sampleUi.GetControl(ListBoxControl).SetLocation(desc.Width - 400, desc.Height - 180);
            sampleUi.GetControl(ListBoxControl).SetSize(190, 96);
            sampleUi.GetControl(ListBoxControlMulti).SetLocation(desc.Width - 200, desc.Height - 180);
            sampleUi.GetControl(ListBoxControlMulti).SetSize(190, 124);
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
        /// This callback function will be called once at the beginning of every frame. This is the
        /// best location for your application to handle updates to the scene, but is not 
        /// intended to contain actual rendering calls, which should instead be placed in the 
        /// OnFrameRender callback.  
        /// </summary>
        public void OnFrameMove(Device device, double appTime, float elapsedTime)
        {
            // Update the camera's position based on user input 
            camera.FrameMove(elapsedTime);

            Matrix m = Matrix.RotationY((float)Math.PI * elapsedTime / 40.0f);
            viewMatrix = m * viewMatrix;
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

                // Update the effect's variables.  Instead of using strings, it would 
                // be more efficient to cache a handle to the parameter by calling 
                // Effect.GetParameter
                effect.SetValue("worldViewProjection", camera.WorldMatrix * viewMatrix * camera.ProjectionMatrix);
                effect.SetValue("worldMatrix", camera.WorldMatrix);
                effect.Technique = "RenderScene";

                // Render the mesh
                int passes = effect.Begin(0);
                Mesh localMesh = mesh.LocalMesh;
                for (int pass = 0; pass < passes; pass++)
                {
                    effect.BeginPass(pass);
                    for(int i = 0; i < mesh.NumberMaterials; i++)
                    {
                        effect.SetValue("sceneTexture", mesh.GetTexture(i));
                        effect.CommitChanges();
                        localMesh.DrawSubset(i);
                    }

                    effect.EndPass();
                }
                effect.End();

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
            txtHelper.DrawTextLine("Press ESC to quit.");
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

            // Update some fonts
            sampleUi.SetFont(1, "Comic Sans MS", 24, FontWeight.Normal);
            sampleUi.SetFont(2, "Courier New", 16, FontWeight.Normal);

            // Now add the sample specific UI
            y = 10;
            sampleUi.AddStatic(StaticControl, "This is a static control.", 0, 0, 200, 30);
            sampleUi.AddStatic(OutputStaticControl, "This static control provides feedback for your action.  It will change as you interact with the UI controls.",
                20, 50, 620, 300);
            
            sampleUi.GetStaticText(OutputStaticControl)[0].FontColor.States[(int)ControlState.Normal] = new ColorValue(0.0f, 1.0f, 0.0f, 1.0f); // Change color to green
            sampleUi.GetStaticText(OutputStaticControl)[0].textFormat = DrawTextFormat.Left | DrawTextFormat.Top | DrawTextFormat.WordBreak;
            sampleUi.GetStaticText(OutputStaticControl)[0].FontIndex = 1;

            // Edit box
            EditBox edit = sampleUi.AddEditBox(EditBoxControl, "Edit control with default styles.  Type text here and press Enter.",
                20, 440, 600, 32);

            // Slider
            Slider sl = sampleUi.AddSlider(SliderControl, 200, 450, 200, 24, 0, 1000, 500, false);

            // Check boxes
            Checkbox hotKey = sampleUi.AddCheckBox(CheckBoxControl, "This is a checkbox with hotkey.  Press 'C' to toggle the check state.",
                150, 450, 350, 24, false, System.Windows.Forms.Keys.C, false);
            Checkbox clearEdit = sampleUi.AddCheckBox(ClearEditControl, "This checkbox controls whether the edit control text is cleared when Enter is pressed.",
                150, 460, 430, 24, false);

            // Combo box
            ComboBox combo = sampleUi.AddComboBox(ComboBoxControl, 0, 0, 200, 24);
            if (combo != null)
            {
                // Add some items
                combo.SetDropHeight(100);
                combo.AddItem("Combo box item", 0x11111111);
                combo.AddItem("Placeholder", 0x12121212);
                combo.AddItem("One more", 0x13131313);
                combo.AddItem("I can't get enough", 0x14141414);
                combo.AddItem("Ok, last one, I promise", 0x15151515);
            }

            // Radio buttons
            sampleUi.AddRadioButton(RadioButton1A, 1, "Radio group 1 Amy", 0, 50, 200, 24, false);
            sampleUi.AddRadioButton(RadioButton1B, 1, "Radio group 1 Brian", 0, 50, 200, 24, false);
            sampleUi.AddRadioButton(RadioButton1C, 1, "Radio group 1 Clark", 0, 50, 200, 24, false);

            sampleUi.AddRadioButton(RadioButton2A, 2, "Single", 0, 50, 70, 24, false);
            sampleUi.AddRadioButton(RadioButton2B, 2, "Double", 0, 50, 70, 24, false);
            sampleUi.AddRadioButton(RadioButton2C, 2, "Trouble", 0, 50, 70, 24, false);

            // List boxes
            ListBox singleBox = sampleUi.AddListBox(ListBoxControl, 30, 400, 200, 150, ListBoxStyle.SingleSelection);
            for (int i = 0; i < 15; i++)
                singleBox.AddItem("Single-selection listbox item " + i.ToString(), i);

            ListBox multiBox = sampleUi.AddListBox(ListBoxControlMulti, 30, 400, 200, 150, ListBoxStyle.Multiselection);
            for (int i = 0; i < 30; i++)
                multiBox.AddItem("Multi-selection listbox item " + i.ToString(), i);

            // Hook some events
            sl.ValueChanged += new EventHandler(OnValueChanged);
            hotKey.Changed += new EventHandler(OnHotkeyBoxChanged);
            clearEdit.Changed += new EventHandler(OnClearEditChanged);
            combo.Changed += new EventHandler(OnComboChanged);
            edit.Changed += new EventHandler(OnEditChanged);
            edit.Enter += new EventHandler(OnEnterHit);
            // All the radio buttons
            sampleUi.GetRadioButton(RadioButton1A).Changed += new EventHandler(OnRadioButtonChanged);
            sampleUi.GetRadioButton(RadioButton1B).Changed += new EventHandler(OnRadioButtonChanged);
            sampleUi.GetRadioButton(RadioButton1C).Changed += new EventHandler(OnRadioButtonChanged);
            sampleUi.GetRadioButton(RadioButton2A).Changed += new EventHandler(OnRadioButtonChanged);
            sampleUi.GetRadioButton(RadioButton2B).Changed += new EventHandler(OnRadioButtonChanged);
            sampleUi.GetRadioButton(RadioButton2C).Changed += new EventHandler(OnRadioButtonChanged);
            // Finally the listboxes
            singleBox.DoubleClick += new EventHandler(OnDoubleClick);
            singleBox.Selection += new EventHandler(OnSingleSelection);
            multiBox.DoubleClick += new EventHandler(OnDoubleClick);
            multiBox.Selection += new EventHandler(OnMultiSelection);
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

        /// <summary>Called when the slider value has changed</summary>
        private void OnValueChanged(object sender, EventArgs e)
        {
            sampleUi.GetStaticText(OutputStaticControl).SetText
                ("You adjusted the slider control.\nThe new value reported is " + 
                (sender as Slider).Value.ToString());
        }

        /// <summary>Called when the edit box has changed</summary>
        private void OnEditChanged(object sender, EventArgs e)
        {
            EditBox eb = sender as EditBox;
            sampleUi.GetStaticText(OutputStaticControl).SetText
                ("You changed the edit box control.\nThe new value is: \n" + 
                eb.Text.ToString());
        }

        /// <summary>Called when the enter key has been hit on the edit box</summary>
        private void OnEnterHit(object sender, EventArgs e)
        {
            EditBox eb = sender as EditBox;
            sampleUi.GetStaticText(OutputStaticControl).SetText
                ("You have pressed Enter in the edit control.\nThe content of the edit control is:\n" + 
                eb.Text.ToString());

            // Clear the text if needed
            if (sampleUi.GetCheckbox(ClearEditControl).IsChecked)
                eb.Text = string.Empty;
        }

        /// <summary>Called when the slider value has changed</summary>
        private void OnHotkeyBoxChanged(object sender, EventArgs e)
        {
            sampleUi.GetStaticText(OutputStaticControl).SetText(string.Format
                ("You {0} the upper check box.", (sender as Checkbox).IsChecked ? "checked" : "cleared"));
        }

        /// <summary>Called when the slider value has changed</summary>
        private void OnClearEditChanged(object sender, EventArgs e)
        {
            sampleUi.GetStaticText(OutputStaticControl).SetText(string.Format
                ("You {0} the upper check box.\nNow edit control will {1}", 
                (sender as Checkbox).IsChecked ? "checked" : "cleared",
                (sender as Checkbox).IsChecked ? "be cleared when you press Enter to send the text" 
                : "retain the text context when you press Enter to send the text"));
        }

        /// <summary>Called when the combo box has changed</summary>
        private void OnComboChanged(object sender, EventArgs e)
        {
            sampleUi.GetStaticText(OutputStaticControl).SetText(string.Format
                ("You selected a new item in the combox box.\nThe new item is {0} and the associated data is {1}", 
                (sender as ComboBox).GetSelectedItem().ItemText,
                ((int)(sender as ComboBox).GetSelectedItem().ItemData).ToString("x")));
        }

        /// <summary>Called when the combo box has changed</summary>
        private void OnRadioButtonChanged(object sender, EventArgs e)
        {
            RadioButton rb = sender as RadioButton;

            sampleUi.GetStaticText(OutputStaticControl).SetText(string.Format
                ("You selected a new radio button in the {0} radio group.\nThe new button is {1}", 
                (rb.ButtonGroup != 0) ? "UPPER" : "LOWER",
                rb.GetTextCopy()));
        }

        /// <summary>Called when a listbox is double clicked</summary>
        private void OnDoubleClick(object sender, EventArgs e)
        {
            ListBox lb = sender as ListBox;
            ListBoxItem lbi = lb[lb.GetSelectedIndex(-1)];

            sampleUi.GetStaticText(OutputStaticControl).SetText(string.Format
                ("You double clicked an item in the {0} list box.\nThe item is {1}", 
                (lb.Style == ListBoxStyle.SingleSelection) ? "left" : "right",
                lbi.ItemText));
        }

        /// <summary>Called when the a selection is made on the single listbox</summary>
        private void OnSingleSelection(object sender, EventArgs e)
        {
            ListBox lb = sender as ListBox;
            ListBoxItem lbi = lb[lb.GetSelectedIndex(-1)];

            sampleUi.GetStaticText(OutputStaticControl).SetText(string.Format
                ("You changed the selection in the left list box.\nThe item is {0}", 
                lbi.ItemText));
        }

        /// <summary>Called when the a selection is made on the multi listbox</summary>
        private void OnMultiSelection(object sender, EventArgs e)
        {
            ListBox lb = sender as ListBox;
            int selectedIndex = -1;
            string updatedText = "You changed the selection in the right list box.  The selected item(s) are\n";

            while ((selectedIndex = lb.GetSelectedIndex(selectedIndex)) != -1)
            {
                updatedText += (selectedIndex.ToString() + ",");
            }

            // Get rid of the last comma
            updatedText = updatedText.Substring(0, updatedText.Length -1);
            sampleUi.GetStaticText(OutputStaticControl).SetText(updatedText);
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
                CustomUI sample = new CustomUI(sampleFramework);
                // Set the callback functions. These functions allow the sample framework to notify
                // the application about device changes, user input, and windows messages.  The 
                // callbacks are optional so you need only set callbacks for events you're interested 
                // in. However, if you don't handle the device reset/lost callbacks then the sample 
                // framework won't be able to reset your device since the application must first 
                // release all device resources before resetting.  Likewise, if you don't handle the 
                // device created/destroyed callbacks then the sample framework won't be able to 
                // recreate your device resources.
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
                    sampleFramework.CreateWindow("CustomUI");
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
