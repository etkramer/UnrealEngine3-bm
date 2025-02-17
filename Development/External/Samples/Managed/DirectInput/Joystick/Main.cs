//-----------------------------------------------------------------------------
// File: Main.cs
//
// Desc: The Joystick sample obtains and displays joystick data.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
using System;
using System.Windows.Forms;
using Microsoft.DirectX.DirectInput;
using Microsoft.DirectX;

namespace Joystick
{
    public class MainClass : System.Windows.Forms.Form
    {
        // These three fields hold common data that
        // different threads will have to access
        public static JoystickState state = new JoystickState();
        private Device applicationDevice = null;
        public static int numPOVs = 0;
		private int SliderCount = 0; // Number of returned slider controls

		#region Window control declarations
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label labelXAxis;
		private System.Windows.Forms.Label labelYAxis;
		private System.Windows.Forms.Label labelZAxis;
		private System.Windows.Forms.Label labelZRotation;
		private System.Windows.Forms.Label labelYRotation;
		private System.Windows.Forms.Label labelXRotation;
		private System.Windows.Forms.Label labelPOV1;
		private System.Windows.Forms.Label labelPOV0;
		private System.Windows.Forms.Label labelSlider1;
		private System.Windows.Forms.Label labelSlider0;
		private System.Windows.Forms.Label labelPOV2;
		private System.Windows.Forms.Label labelPOV3;
		private System.Windows.Forms.Button buttonExit;
		private System.Windows.Forms.Label labelXAxisText;
		private System.Windows.Forms.Label labelYAxisText;
		private System.Windows.Forms.Label labelZAxisText;
		private System.Windows.Forms.Label labelXRotationText;
		private System.Windows.Forms.Label labelYRotationText;
		private System.Windows.Forms.Label labelZRotationText;
		private System.Windows.Forms.Label labelSlider0Text;
		private System.Windows.Forms.Label labelSlider1Text;
		private System.Windows.Forms.Label labelPOV2Text;
		private System.Windows.Forms.Label labelPOV1Text;
		private System.Windows.Forms.Label labelPOV0Text;
		private System.Windows.Forms.Label labelPOV3Text;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label labelButtons;
		private System.Windows.Forms.Label label2;
		#endregion
        private System.Windows.Forms.Timer timer1;
        private System.ComponentModel.IContainer components;
        

        
        
        #region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.labelButtons = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.labelPOV3 = new System.Windows.Forms.Label();
            this.labelSlider1 = new System.Windows.Forms.Label();
            this.labelSlider0 = new System.Windows.Forms.Label();
            this.labelPOV2 = new System.Windows.Forms.Label();
            this.labelPOV1 = new System.Windows.Forms.Label();
            this.labelPOV0 = new System.Windows.Forms.Label();
            this.labelZRotation = new System.Windows.Forms.Label();
            this.labelYRotation = new System.Windows.Forms.Label();
            this.labelXRotation = new System.Windows.Forms.Label();
            this.labelZAxis = new System.Windows.Forms.Label();
            this.labelYAxis = new System.Windows.Forms.Label();
            this.labelXAxis = new System.Windows.Forms.Label();
            this.labelPOV3Text = new System.Windows.Forms.Label();
            this.labelPOV2Text = new System.Windows.Forms.Label();
            this.labelPOV1Text = new System.Windows.Forms.Label();
            this.labelPOV0Text = new System.Windows.Forms.Label();
            this.labelSlider1Text = new System.Windows.Forms.Label();
            this.labelSlider0Text = new System.Windows.Forms.Label();
            this.labelZRotationText = new System.Windows.Forms.Label();
            this.labelYRotationText = new System.Windows.Forms.Label();
            this.labelXRotationText = new System.Windows.Forms.Label();
            this.labelZAxisText = new System.Windows.Forms.Label();
            this.labelYAxisText = new System.Windows.Forms.Label();
            this.labelXAxisText = new System.Windows.Forms.Label();
            this.buttonExit = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                    this.labelButtons,
                                                                                    this.label1,
                                                                                    this.labelPOV3,
                                                                                    this.labelSlider1,
                                                                                    this.labelSlider0,
                                                                                    this.labelPOV2,
                                                                                    this.labelPOV1,
                                                                                    this.labelPOV0,
                                                                                    this.labelZRotation,
                                                                                    this.labelYRotation,
                                                                                    this.labelXRotation,
                                                                                    this.labelZAxis,
                                                                                    this.labelYAxis,
                                                                                    this.labelXAxis,
                                                                                    this.labelPOV3Text,
                                                                                    this.labelPOV2Text,
                                                                                    this.labelPOV1Text,
                                                                                    this.labelPOV0Text,
                                                                                    this.labelSlider1Text,
                                                                                    this.labelSlider0Text,
                                                                                    this.labelZRotationText,
                                                                                    this.labelYRotationText,
                                                                                    this.labelXRotationText,
                                                                                    this.labelZAxisText,
                                                                                    this.labelYAxisText,
                                                                                    this.labelXAxisText});
            this.groupBox1.Location = new System.Drawing.Point(8, 64);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(264, 184);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Joystick State";
            // 
            // labelButtons
            // 
            this.labelButtons.Location = new System.Drawing.Point(72, 157);
            this.labelButtons.Name = "labelButtons";
            this.labelButtons.Size = new System.Drawing.Size(184, 13);
            this.labelButtons.TabIndex = 25;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(16, 157);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(46, 13);
            this.label1.TabIndex = 24;
            this.label1.Text = "Buttons:";
            // 
            // labelPOV3
            // 
            this.labelPOV3.AutoSize = true;
            this.labelPOV3.Enabled = false;
            this.labelPOV3.Location = new System.Drawing.Point(192, 128);
            this.labelPOV3.Name = "labelPOV3";
            this.labelPOV3.Size = new System.Drawing.Size(10, 13);
            this.labelPOV3.TabIndex = 23;
            this.labelPOV3.Text = "0";
            // 
            // labelSlider1
            // 
            this.labelSlider1.AutoSize = true;
            this.labelSlider1.Enabled = false;
            this.labelSlider1.Location = new System.Drawing.Point(192, 40);
            this.labelSlider1.Name = "labelSlider1";
            this.labelSlider1.Size = new System.Drawing.Size(10, 13);
            this.labelSlider1.TabIndex = 22;
            this.labelSlider1.Text = "0";
            // 
            // labelSlider0
            // 
            this.labelSlider0.AutoSize = true;
            this.labelSlider0.Enabled = false;
            this.labelSlider0.Location = new System.Drawing.Point(192, 24);
            this.labelSlider0.Name = "labelSlider0";
            this.labelSlider0.Size = new System.Drawing.Size(10, 13);
            this.labelSlider0.TabIndex = 21;
            this.labelSlider0.Text = "0";
            // 
            // labelPOV2
            // 
            this.labelPOV2.AutoSize = true;
            this.labelPOV2.Enabled = false;
            this.labelPOV2.Location = new System.Drawing.Point(192, 112);
            this.labelPOV2.Name = "labelPOV2";
            this.labelPOV2.Size = new System.Drawing.Size(10, 13);
            this.labelPOV2.TabIndex = 20;
            this.labelPOV2.Text = "0";
            // 
            // labelPOV1
            // 
            this.labelPOV1.AutoSize = true;
            this.labelPOV1.Enabled = false;
            this.labelPOV1.Location = new System.Drawing.Point(192, 96);
            this.labelPOV1.Name = "labelPOV1";
            this.labelPOV1.Size = new System.Drawing.Size(10, 13);
            this.labelPOV1.TabIndex = 19;
            this.labelPOV1.Text = "0";
            // 
            // labelPOV0
            // 
            this.labelPOV0.AutoSize = true;
            this.labelPOV0.Enabled = false;
            this.labelPOV0.Location = new System.Drawing.Point(192, 80);
            this.labelPOV0.Name = "labelPOV0";
            this.labelPOV0.Size = new System.Drawing.Size(10, 13);
            this.labelPOV0.TabIndex = 18;
            this.labelPOV0.Text = "0";
            // 
            // labelZRotation
            // 
            this.labelZRotation.AutoSize = true;
            this.labelZRotation.Enabled = false;
            this.labelZRotation.Location = new System.Drawing.Point(88, 128);
            this.labelZRotation.Name = "labelZRotation";
            this.labelZRotation.Size = new System.Drawing.Size(10, 13);
            this.labelZRotation.TabIndex = 17;
            this.labelZRotation.Text = "0";
            // 
            // labelYRotation
            // 
            this.labelYRotation.AutoSize = true;
            this.labelYRotation.Enabled = false;
            this.labelYRotation.Location = new System.Drawing.Point(88, 112);
            this.labelYRotation.Name = "labelYRotation";
            this.labelYRotation.Size = new System.Drawing.Size(10, 13);
            this.labelYRotation.TabIndex = 16;
            this.labelYRotation.Text = "0";
            // 
            // labelXRotation
            // 
            this.labelXRotation.AutoSize = true;
            this.labelXRotation.Enabled = false;
            this.labelXRotation.Location = new System.Drawing.Point(88, 96);
            this.labelXRotation.Name = "labelXRotation";
            this.labelXRotation.Size = new System.Drawing.Size(10, 13);
            this.labelXRotation.TabIndex = 15;
            this.labelXRotation.Text = "0";
            // 
            // labelZAxis
            // 
            this.labelZAxis.AutoSize = true;
            this.labelZAxis.Enabled = false;
            this.labelZAxis.Location = new System.Drawing.Point(88, 56);
            this.labelZAxis.Name = "labelZAxis";
            this.labelZAxis.Size = new System.Drawing.Size(10, 13);
            this.labelZAxis.TabIndex = 14;
            this.labelZAxis.Text = "0";
            // 
            // labelYAxis
            // 
            this.labelYAxis.AutoSize = true;
            this.labelYAxis.Enabled = false;
            this.labelYAxis.Location = new System.Drawing.Point(88, 40);
            this.labelYAxis.Name = "labelYAxis";
            this.labelYAxis.Size = new System.Drawing.Size(10, 13);
            this.labelYAxis.TabIndex = 13;
            this.labelYAxis.Text = "0";
            // 
            // labelXAxis
            // 
            this.labelXAxis.AutoSize = true;
            this.labelXAxis.Enabled = false;
            this.labelXAxis.Location = new System.Drawing.Point(88, 24);
            this.labelXAxis.Name = "labelXAxis";
            this.labelXAxis.Size = new System.Drawing.Size(10, 13);
            this.labelXAxis.TabIndex = 12;
            this.labelXAxis.Text = "0";
            // 
            // labelPOV3Text
            // 
            this.labelPOV3Text.AutoSize = true;
            this.labelPOV3Text.Enabled = false;
            this.labelPOV3Text.Location = new System.Drawing.Point(136, 128);
            this.labelPOV3Text.Name = "labelPOV3Text";
            this.labelPOV3Text.Size = new System.Drawing.Size(41, 13);
            this.labelPOV3Text.TabIndex = 11;
            this.labelPOV3Text.Text = "POV 3:";
            // 
            // labelPOV2Text
            // 
            this.labelPOV2Text.AutoSize = true;
            this.labelPOV2Text.Enabled = false;
            this.labelPOV2Text.Location = new System.Drawing.Point(136, 112);
            this.labelPOV2Text.Name = "labelPOV2Text";
            this.labelPOV2Text.Size = new System.Drawing.Size(41, 13);
            this.labelPOV2Text.TabIndex = 10;
            this.labelPOV2Text.Text = "POV 2:";
            // 
            // labelPOV1Text
            // 
            this.labelPOV1Text.AutoSize = true;
            this.labelPOV1Text.Enabled = false;
            this.labelPOV1Text.Location = new System.Drawing.Point(136, 96);
            this.labelPOV1Text.Name = "labelPOV1Text";
            this.labelPOV1Text.Size = new System.Drawing.Size(41, 13);
            this.labelPOV1Text.TabIndex = 9;
            this.labelPOV1Text.Text = "POV 1:";
            // 
            // labelPOV0Text
            // 
            this.labelPOV0Text.AutoSize = true;
            this.labelPOV0Text.Enabled = false;
            this.labelPOV0Text.Location = new System.Drawing.Point(136, 80);
            this.labelPOV0Text.Name = "labelPOV0Text";
            this.labelPOV0Text.Size = new System.Drawing.Size(41, 13);
            this.labelPOV0Text.TabIndex = 8;
            this.labelPOV0Text.Text = "POV 0:";
            // 
            // labelSlider1Text
            // 
            this.labelSlider1Text.AutoSize = true;
            this.labelSlider1Text.Enabled = false;
            this.labelSlider1Text.Location = new System.Drawing.Point(136, 40);
            this.labelSlider1Text.Name = "labelSlider1Text";
            this.labelSlider1Text.Size = new System.Drawing.Size(46, 13);
            this.labelSlider1Text.TabIndex = 7;
            this.labelSlider1Text.Text = "Slider 1:";
            // 
            // labelSlider0Text
            // 
            this.labelSlider0Text.AutoSize = true;
            this.labelSlider0Text.Enabled = false;
            this.labelSlider0Text.Location = new System.Drawing.Point(136, 24);
            this.labelSlider0Text.Name = "labelSlider0Text";
            this.labelSlider0Text.Size = new System.Drawing.Size(46, 13);
            this.labelSlider0Text.TabIndex = 6;
            this.labelSlider0Text.Text = "Slider 0:";
            // 
            // labelZRotationText
            // 
            this.labelZRotationText.AutoSize = true;
            this.labelZRotationText.Enabled = false;
            this.labelZRotationText.Location = new System.Drawing.Point(16, 128);
            this.labelZRotationText.Name = "labelZRotationText";
            this.labelZRotationText.Size = new System.Drawing.Size(60, 13);
            this.labelZRotationText.TabIndex = 5;
            this.labelZRotationText.Text = "Z Rotation:";
            // 
            // labelYRotationText
            // 
            this.labelYRotationText.AutoSize = true;
            this.labelYRotationText.Enabled = false;
            this.labelYRotationText.Location = new System.Drawing.Point(16, 112);
            this.labelYRotationText.Name = "labelYRotationText";
            this.labelYRotationText.Size = new System.Drawing.Size(60, 13);
            this.labelYRotationText.TabIndex = 4;
            this.labelYRotationText.Text = "Y Rotation:";
            // 
            // labelXRotationText
            // 
            this.labelXRotationText.AutoSize = true;
            this.labelXRotationText.Enabled = false;
            this.labelXRotationText.Location = new System.Drawing.Point(16, 96);
            this.labelXRotationText.Name = "labelXRotationText";
            this.labelXRotationText.Size = new System.Drawing.Size(60, 13);
            this.labelXRotationText.TabIndex = 3;
            this.labelXRotationText.Text = "X Rotation:";
            // 
            // labelZAxisText
            // 
            this.labelZAxisText.AutoSize = true;
            this.labelZAxisText.Enabled = false;
            this.labelZAxisText.Location = new System.Drawing.Point(16, 56);
            this.labelZAxisText.Name = "labelZAxisText";
            this.labelZAxisText.Size = new System.Drawing.Size(39, 13);
            this.labelZAxisText.TabIndex = 2;
            this.labelZAxisText.Text = "Z Axis:";
            // 
            // labelYAxisText
            // 
            this.labelYAxisText.AutoSize = true;
            this.labelYAxisText.Enabled = false;
            this.labelYAxisText.Location = new System.Drawing.Point(16, 40);
            this.labelYAxisText.Name = "labelYAxisText";
            this.labelYAxisText.Size = new System.Drawing.Size(39, 13);
            this.labelYAxisText.TabIndex = 1;
            this.labelYAxisText.Text = "Y Axis:";
            // 
            // labelXAxisText
            // 
            this.labelXAxisText.AutoSize = true;
            this.labelXAxisText.Enabled = false;
            this.labelXAxisText.Location = new System.Drawing.Point(16, 24);
            this.labelXAxisText.Name = "labelXAxisText";
            this.labelXAxisText.Size = new System.Drawing.Size(39, 13);
            this.labelXAxisText.TabIndex = 0;
            this.labelXAxisText.Text = "X Axis:";
            // 
            // buttonExit
            // 
            this.buttonExit.Location = new System.Drawing.Point(232, 264);
            this.buttonExit.Name = "buttonExit";
            this.buttonExit.Size = new System.Drawing.Size(40, 24);
            this.buttonExit.TabIndex = 0;
            this.buttonExit.Text = "Exit";
            this.buttonExit.Click += new System.EventHandler(this.buttonExit_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(8, 24);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(255, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "This sample continously polls the joystick for data.";
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // MainClass
            // 
            this.ClientSize = new System.Drawing.Size(280, 293);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.label2,
                                                                          this.buttonExit,
                                                                          this.groupBox1});
            this.KeyPreview = true;
            this.MaximizeBox = false;
            this.Name = "MainClass";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Joystick";
            this.Load += new System.EventHandler(this.MainClass_Load);
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);

        }
		#endregion




		public MainClass()
		{
            try
            {
                // Load the icon from our resources
                System.Resources.ResourceManager resources = new System.Resources.ResourceManager(this.GetType());
                this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            }
            catch
            {
                // It's no big deal if we can't load our icons, but try to load the embedded one
                try { this.Icon = new System.Drawing.Icon(this.GetType(), "directx.ico"); } 
                catch {}
            }
            //
			// Required for Windows Form Designer support.
			//
			InitializeComponent();
        }




        private void buttonExit_Click(object sender, System.EventArgs e)
		{
			Close();
		}




        public bool InitDirectInput()
        {
            // Enumerate joysticks in the system.
            foreach (DeviceInstance instance in Manager.GetDevices(DeviceClass.GameControl, EnumDevicesFlags.AttachedOnly))
            {
                // Create the device.  Just pick the first one
                applicationDevice = new Device(instance.InstanceGuid);
                break;
            }

            if (null == applicationDevice)
            {
                MessageBox.Show("Unable to create a joystick device. Sample will exit.", "No joystick found");
                return false;
            }

            // Set the data format to the c_dfDIJoystick pre-defined format.
            applicationDevice.SetDataFormat(DeviceDataFormat.Joystick);
            // Set the cooperative level for the device.
            applicationDevice.SetCooperativeLevel(this, CooperativeLevelFlags.Exclusive | CooperativeLevelFlags.Foreground);
            // Enumerate all the objects on the device.
            foreach (DeviceObjectInstance d in applicationDevice.Objects)
            {
                // For axes that are returned, set the DIPROP_RANGE property for the
                // enumerated axis in order to scale min/max values.

                if ((0 != (d.ObjectId & (int)DeviceObjectTypeFlags.Axis)))
                {
                    // Set the range for the axis.
                    applicationDevice.Properties.SetRange(ParameterHow.ById, d.ObjectId, new InputRange(-1000, +1000));
                }
                // Update the controls to reflect what
                // objects the device supports.
                UpdateControls(d);
            }
            return true;
        }

        
        
        
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        static void Main()
        {
            // Create a new instance of 
            // the MainClass class.
            Application.Run(new MainClass());
        }

        
        
        
        protected override void Dispose(bool disposing)
        {
            timer1.Stop();

            // Unacquire all DirectInput objects.
            if (null != applicationDevice)
                applicationDevice.Unacquire();

            if (disposing)
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose(disposing);
        }

        
        
        
        public void GetData()
        {
            // Make sure there is a valid device.
            if (null == applicationDevice)
                return;

            try
            {
                // Poll the device for info.
                applicationDevice.Poll();
            }
            catch(InputException inputex)
            {
                if ((inputex is NotAcquiredException) || (inputex is InputLostException))
                {
                    // Check to see if either the app
                    // needs to acquire the device, or
                    // if the app lost the device to another
                    // process.
                    try
                    {
                        // Acquire the device.
                        applicationDevice.Acquire();
                    }
                    catch(InputException)
                    {
                        // Failed to acquire the device.
                        // This could be because the app
                        // doesn't have focus.
                        return;
                    }
                }
                
            } //catch(InputException inputex)

            // Get the state of the device.
            try {state = applicationDevice.CurrentJoystickState;}
                // Catch any exceptions. None will be handled here, 
                // any device re-aquisition will be handled above.  
            catch(InputException)
            {
                return;
            }
            
            UpdateUI();
        }
        
        
        
        
        private void UpdateUI()
		{
			// This function updated the UI with
			// joystick state information.

			string strText = null;

			labelXAxis.Text = state.X.ToString();
			labelYAxis.Text = state.Y.ToString();
			labelZAxis.Text = state.Z.ToString();

			labelXRotation.Text = state.Rx.ToString();
			labelYRotation.Text = state.Ry.ToString();
			labelZRotation.Text = state.Rz.ToString();
		
			
			int[] slider = state.GetSlider();
			
			labelSlider0.Text = slider[0].ToString();
			labelSlider1.Text = slider[1].ToString();

			int[] pov = state.GetPointOfView();

			labelPOV0.Text = pov[0].ToString();
			labelPOV1.Text = pov[1].ToString();
			labelPOV2.Text = pov[2].ToString();
			labelPOV3.Text = pov[3].ToString();

			// Fill up text with which buttons are pressed
			byte[] buttons = state.GetButtons();

			int button = 0;
			foreach (byte b in buttons)
			{
				if (0!= (b & 0x80))
					strText += button.ToString("00 ");
				button++;
			}
			labelButtons.Text = strText;
		}




		public void UpdateControls(DeviceObjectInstance d)
		{
			// Set the UI to reflect what objects the joystick supports.
			if (ObjectTypeGuid.XAxis == d.ObjectType)
			{
				labelXAxis.Enabled = true;
				labelXAxisText.Enabled = true;
			}
			if (ObjectTypeGuid.YAxis == d.ObjectType)
			{
				labelYAxis.Enabled = true;
				labelYAxisText.Enabled = true;
			}
			if (ObjectTypeGuid.ZAxis == d.ObjectType)
			{
				labelZAxis.Enabled = true;
				labelZAxisText.Enabled = true;
			}
			if (ObjectTypeGuid.RxAxis == d.ObjectType)
			{
				labelXRotation.Enabled = true;
				labelXRotationText.Enabled = true;
			}
			if (ObjectTypeGuid.RyAxis == d.ObjectType)
			{
				labelYRotation.Enabled = true;
				labelYRotationText.Enabled = true;
			}
			if (ObjectTypeGuid.RzAxis == d.ObjectType)
			{
				labelZRotation.Enabled = true;
				labelZRotationText.Enabled = true;
			}
			if (ObjectTypeGuid.Slider == d.ObjectType)
			{
				switch (SliderCount++)
				{
					case 0 :
						labelSlider0.Enabled = true;
						labelSlider0Text.Enabled = true;
						break;

					case 1 :
						labelSlider1.Enabled = true;
						labelSlider1Text.Enabled = true;
						break;
				}
			}
			if (ObjectTypeGuid.PointOfView == d.ObjectType)
			{
				switch (numPOVs++)
				{
					case 0 :
						labelPOV0.Enabled = true;
						labelPOV0Text.Enabled = true;
						break;

					case 1 :
						labelPOV1.Enabled = true;
						labelPOV1Text.Enabled = true;
						break;

					case 2 :
						labelPOV2.Enabled = true;
						labelPOV2Text.Enabled = true;
						break;

					case 3 :
						labelPOV3.Enabled = true;
						labelPOV3Text.Enabled = true;
						break;
				}
			}
		}




        private void MainClass_Load(object sender, System.EventArgs e)
        {
            if (!InitDirectInput())
                Close();

            timer1.Start();
        }




        private void timer1_Tick(object sender, System.EventArgs e)
        {
            GetData();
        }
    }
}
