//-----------------------------------------------------------------------------
// File: frmUI.cs
//
// Desc: The Mouse sample obtains and displays Mouse data.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
using System;
using System.Windows.Forms;
using Microsoft.DirectX.DirectInput;
using Microsoft.DirectX;
using System.Threading;

namespace Mouse
{	
	public class frmUI : System.Windows.Forms.Form
	{
		Device applicationDevice = null;
        Thread threadData = null;
        AutoResetEvent eventFire = null;
        delegate void UIDelegate();

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label lblButton0;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label lblButton1;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label lblButton2;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.ListBox lbStatus;	
		private System.ComponentModel.Container components = null;

        public void MouseEvent()
        {
            // This is the mouse event thread.
            // This thread will wait until a
            // mouse event occurrs, and invoke
            // the delegate on the main thread to
            // grab the current mouse state and update
            // the UI.
            while(Created)                
            {
                eventFire.WaitOne(-1, false);
                
                try
                {
                    applicationDevice.Poll();
                }
                catch(InputException)
                {
                    continue;
                }
                this.BeginInvoke(new UIDelegate(UpdateUI));
            }
        }

        
        
        
        [MTAThread]
		public static void Main()
		{
            Application.Run(new frmUI());
		}




		public frmUI()
		{
			// Required for Windows Form Designer support.
			InitializeComponent();
		}

        
        
        
        /// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose(bool disposing)
		{
			if (disposing)
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose(disposing);
            
            eventFire.Set();
		}
		#region Windows Form Designer generated code

        
        
        
        /// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.lbStatus = new System.Windows.Forms.ListBox();
            this.lblButton2 = new System.Windows.Forms.Label();
            this.lblButton0 = new System.Windows.Forms.Label();
            this.lblButton1 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // lbStatus
            // 
            this.lbStatus.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lbStatus.Items.AddRange(new object[] {
                                                          "0,0,0"});
            this.lbStatus.Location = new System.Drawing.Point(32, 200);
            this.lbStatus.Name = "lbStatus";
            this.lbStatus.Size = new System.Drawing.Size(96, 106);
            this.lbStatus.TabIndex = 4;
            // 
            // lblButton2
            // 
            this.lblButton2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lblButton2.Location = new System.Drawing.Point(32, 144);
            this.lblButton2.Name = "lblButton2";
            this.lblButton2.Size = new System.Drawing.Size(96, 16);
            this.lblButton2.TabIndex = 0;
            this.lblButton2.Text = "Up";
            this.lblButton2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblButton0
            // 
            this.lblButton0.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lblButton0.Location = new System.Drawing.Point(32, 32);
            this.lblButton0.Name = "lblButton0";
            this.lblButton0.Size = new System.Drawing.Size(96, 16);
            this.lblButton0.TabIndex = 0;
            this.lblButton0.Text = "Up";
            this.lblButton0.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblButton1
            // 
            this.lblButton1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lblButton1.Location = new System.Drawing.Point(32, 88);
            this.lblButton1.Name = "lblButton1";
            this.lblButton1.Size = new System.Drawing.Size(96, 16);
            this.lblButton1.TabIndex = 0;
            this.lblButton1.Text = "Up";
            this.lblButton1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(32, 184);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(99, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Coordinates (x,y,z)";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(56, 16);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(43, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Button0";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(56, 72);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(43, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Button1";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(56, 128);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(43, 13);
            this.label3.TabIndex = 1;
            this.label3.Text = "Button2";
            // 
            // frmUI
            // 
            this.ClientSize = new System.Drawing.Size(162, 325);
            this.ControlBox = false;
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.lbStatus,
                                                                          this.label4,
                                                                          this.label3,
                                                                          this.label2,
                                                                          this.label1,
                                                                          this.lblButton2,
                                                                          this.lblButton1,
                                                                          this.lblButton0});
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.KeyPreview = true;
            this.MaximizeBox = false;
            this.Name = "frmUI";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Mouse (Escape exits)";
            this.Load += new System.EventHandler(this.frmUI_Load);
            this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.frmUI_KeyUp);
            this.Activated += new System.EventHandler(this.frmUI_Activated);
            this.ResumeLayout(false);

        }
		#endregion

        
        
        
        private void frmUI_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
		{
            // If escape is pressed, user wants to exit.
			if (e.KeyCode == Keys.Escape)
				Dispose();
		}
		
        
        
        
        public void UpdateUI()
		{
		    MouseState mouseStateData = new MouseState();
            
            // Get the current state of the mouse device.
            mouseStateData = applicationDevice.CurrentMouseState;
			byte[] buttons = mouseStateData.GetMouseButtons();
			
			// Display some info about the device.
			if (0 != buttons[0]) 
				lblButton0.Text="Down";
			else
				lblButton0.Text="Up";
			if (0 != buttons[1])
				lblButton1.Text="Down";
			else
				lblButton1.Text="Up";
			if (0 != buttons[2])
				lblButton2.Text="Down";	
			else
				lblButton2.Text="Up";
			if (0 != (mouseStateData.X | mouseStateData.Y | mouseStateData.Z))
			{
				lbStatus.Items.Add(mouseStateData.X + ", " + mouseStateData.Y + ", " + mouseStateData.Z);
				if (lbStatus.Items.Count > 10) 
					lbStatus.Items.RemoveAt(1);
				lbStatus.SelectedIndex = lbStatus.Items.Count-1;
			}
		}

        
        
        
        private void frmUI_Load(object sender, System.EventArgs e)
        {
            threadData = new Thread(new ThreadStart(this.MouseEvent));
            threadData.Start();

            eventFire = new AutoResetEvent(false);
            
			// Create the device.
            try
            {
                applicationDevice = new Device(SystemGuid.Mouse);
            }
            catch(InputException)
            {
                MessageBox.Show("Unable to create device. Sample will now exit.");
                Close();
            }
			// Set the cooperative level for the device.
			applicationDevice.SetCooperativeLevel(this, CooperativeLevelFlags.Exclusive | CooperativeLevelFlags.Foreground);
            // Set a notification event.    
            applicationDevice.SetEventNotification(eventFire);
            // Acquire the device.
            try{ applicationDevice.Acquire(); }
            catch{}                
        }

        
        
        
        private void frmUI_Activated(object sender, System.EventArgs e)
        {
            // Acquire the device whenever the application window is activated.
            if (null != applicationDevice)
            {
                try{applicationDevice.Acquire();}
                catch{}
            }
        }
	}
}
