//----------------------------------------------------------------------------
// File: wfEnum.cs
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using Microsoft.DirectX.DirectSound;

namespace csEnumDevices
{
	public class wfEnum : System.Windows.Forms.Form
	{
		private struct DeviceDescription
		{
			public DeviceInformation info;
			public override string ToString()
			{
				return info.Description;
			}
			public DeviceDescription(DeviceInformation d)
			{
				info = d;
			}
		}
		// The devices
		private Device applicationDevice = null;
		private Capture applicationCapture = null;
		private DevicesCollection devicesCollection = null;
		private CaptureDevicesCollection captureCollection = null;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.ComboBox comboboxSound;
		private System.Windows.Forms.ComboBox comboboxCapture;
		private System.Windows.Forms.Button buttonCreate;
		private System.Windows.Forms.Button buttonExit;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		public wfEnum()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			// Retrieve the DirectSound Devices first.
			devicesCollection = new DevicesCollection();
			
			foreach (DeviceInformation dev in devicesCollection)
			{
				DeviceDescription dd = new DeviceDescription(dev);
				comboboxSound.Items.Add(dd);
			}

			// Select the first item in the combobox
			if (0 < comboboxSound.Items.Count)
				comboboxSound.SelectedIndex = 0;
			
			// Now the capture devices
			captureCollection = new CaptureDevicesCollection();
			
			foreach (DeviceInformation dev in captureCollection)
			{
				DeviceDescription dd = new DeviceDescription(dev);
				comboboxCapture.Items.Add(dd);
			}

			// Select the first item in the combobox
			if (comboboxCapture.Items.Count > 0)
				comboboxCapture.SelectedIndex = 0;
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
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.comboboxCapture = new System.Windows.Forms.ComboBox();
			this.buttonCreate = new System.Windows.Forms.Button();
			this.buttonExit = new System.Windows.Forms.Button();
			this.comboboxSound = new System.Windows.Forms.ComboBox();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(6, 4);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(288, 15);
			this.label1.TabIndex = 0;
			this.label1.Text = "This sample simply shows how to enumerate devices.";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(6, 29);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(79, 17);
			this.label2.TabIndex = 1;
			this.label2.Text = "Sound Device:";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(6, 53);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(91, 17);
			this.label3.TabIndex = 1;
			this.label3.Text = "Capture Device:";
			// 
			// comboboxCapture
			// 
			this.comboboxCapture.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboboxCapture.DropDownWidth = 202;
			this.comboboxCapture.Location = new System.Drawing.Point(92, 51);
			this.comboboxCapture.Name = "comboboxCapture";
			this.comboboxCapture.Size = new System.Drawing.Size(202, 21);
			this.comboboxCapture.TabIndex = 3;
			// 
			// buttonCreate
			// 
			this.buttonCreate.Location = new System.Drawing.Point(9, 81);
			this.buttonCreate.Name = "buttonCreate";
			this.buttonCreate.Size = new System.Drawing.Size(86, 21);
			this.buttonCreate.TabIndex = 4;
			this.buttonCreate.Text = "Create";
			this.buttonCreate.Click += new System.EventHandler(this.buttonCreate_Click);
			// 
			// buttonExit
			// 
			this.buttonExit.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.buttonExit.Location = new System.Drawing.Point(205, 81);
			this.buttonExit.Name = "buttonExit";
			this.buttonExit.Size = new System.Drawing.Size(86, 21);
			this.buttonExit.TabIndex = 4;
			this.buttonExit.Text = "Exit";
			this.buttonExit.Click += new System.EventHandler(this.buttonExit_Click);
			// 
			// comboboxSound
			// 
			this.comboboxSound.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboboxSound.DropDownWidth = 202;
			this.comboboxSound.Location = new System.Drawing.Point(92, 27);
			this.comboboxSound.Name = "comboboxSound";
			this.comboboxSound.Size = new System.Drawing.Size(202, 21);
			this.comboboxSound.TabIndex = 2;
			// 
			// wfEnum
			// 
			this.AcceptButton = this.buttonCreate;
			this.CancelButton = this.buttonExit;
			this.ClientSize = new System.Drawing.Size(300, 121);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.buttonExit,
																		  this.buttonCreate,
																		  this.comboboxCapture,
																		  this.comboboxSound,
																		  this.label3,
																		  this.label2,
																		  this.label1});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.Name = "wfEnum";
			this.Text = "DirectSound Enumerate Devices";
			this.ResumeLayout(false);

		}
		#endregion
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		static void Main() 
		{
			Application.Run(new wfEnum());
		}

		private void buttonExit_Click(object sender, System.EventArgs e)
		{
			this.Dispose();
		}

		private void buttonCreate_Click(object sender, System.EventArgs e)
		{
			DeviceDescription itemSelect;

			// Check to see if there are any devices available.
			if ((0 == comboboxSound.Items.Count) && (0 == comboboxCapture.Items.Count))
			{
				MessageBox.Show("No devices available.");
				return;
			}

			// First try to create the Sound Device
			try
			{
				itemSelect = (DeviceDescription)comboboxSound.Items[comboboxSound.SelectedIndex];
				if (Guid.Empty == itemSelect.info.DriverGuid)
					applicationDevice = new Device();
				else
					applicationDevice = new Device(itemSelect.info.DriverGuid);
			}
			catch
			{
				MessageBox.Show("Could not create DirectSound device.", "Failure!", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			
			// Next try to create a capture device
			try
			{
				itemSelect = (DeviceDescription)comboboxCapture.Items[comboboxCapture.SelectedIndex];
				if (Guid.Empty == itemSelect.info.DriverGuid)
					applicationCapture = new Capture();
				else
					applicationCapture = new Capture(itemSelect.info.DriverGuid);
			}
			catch
			{
				MessageBox.Show("Could not create DirectSound Capture device.", "Failure!", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			MessageBox.Show("Devices created successfully.", "Success!", MessageBoxButtons.OK, MessageBoxIcon.Information);
		}
	}
}
