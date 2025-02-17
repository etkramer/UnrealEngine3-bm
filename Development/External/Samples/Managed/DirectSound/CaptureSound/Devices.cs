//----------------------------------------------------------------------------
// File: Devices.cs
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Drawing;
using System.Windows.Forms;
using Microsoft.DirectX.DirectSound;

public class DevicesForm : Form
{
	private Button buttonOk;
	private Button buttonCancel;
	private Label labelStatic;
	private ComboBox comboboxCaptureDeviceCombo;
	private MainForm mf = null;

	CaptureDevicesCollection devices = new CaptureDevicesCollection();

	public DevicesForm(MainForm mf)
	{
		//
		// Required for Windows Form Designer support
		//
		InitializeComponent();
		this.mf = mf;

		foreach (DeviceInformation info in devices)
			comboboxCaptureDeviceCombo.Items.Add(info.Description);

		comboboxCaptureDeviceCombo.SelectedIndex = 0;
	}
    #region InitializeComponent code
	private void InitializeComponent()
	{
        this.buttonOk = new System.Windows.Forms.Button();
        this.buttonCancel = new System.Windows.Forms.Button();
        this.labelStatic = new System.Windows.Forms.Label();
        this.comboboxCaptureDeviceCombo = new System.Windows.Forms.ComboBox();
        this.SuspendLayout();
        // 
        // buttonOk
        // 
        this.buttonOk.Location = new System.Drawing.Point(10, 41);
        this.buttonOk.Name = "buttonOk";
        this.buttonOk.TabIndex = 0;
        this.buttonOk.Text = "OK";
        this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
        // 
        // buttonCancel
        // 
        this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
        this.buttonCancel.Location = new System.Drawing.Point(231, 41);
        this.buttonCancel.Name = "buttonCancel";
        this.buttonCancel.TabIndex = 1;
        this.buttonCancel.Text = "Cancel";
        // 
        // labelStatic
        // 
        this.labelStatic.Location = new System.Drawing.Point(0, 14);
        this.labelStatic.Name = "labelStatic";
        this.labelStatic.Size = new System.Drawing.Size(88, 13);
        this.labelStatic.TabIndex = 2;
        this.labelStatic.Text = "Capture Device:";
        // 
        // comboboxCaptureDeviceCombo
        // 
        this.comboboxCaptureDeviceCombo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
        this.comboboxCaptureDeviceCombo.Location = new System.Drawing.Point(93, 11);
        this.comboboxCaptureDeviceCombo.Name = "comboboxCaptureDeviceCombo";
        this.comboboxCaptureDeviceCombo.Size = new System.Drawing.Size(213, 21);
        this.comboboxCaptureDeviceCombo.Sorted = true;
        this.comboboxCaptureDeviceCombo.TabIndex = 3;
        // 
        // DevicesForm
        // 
        this.AcceptButton = this.buttonOk;
        this.CancelButton = this.buttonCancel;
        this.ClientSize = new System.Drawing.Size(316, 79);
        this.Controls.Add(this.buttonOk);
        this.Controls.Add(this.buttonCancel);
        this.Controls.Add(this.labelStatic);
        this.Controls.Add(this.comboboxCaptureDeviceCombo);
        this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
        this.Name = "DevicesForm";
        this.Text = "Select Capture Device";
        this.ResumeLayout(false);

    }
    #endregion
	private void buttonOk_Click(object sender, System.EventArgs e)
	{
		if (0 < comboboxCaptureDeviceCombo.Items.Count)
			mf.CaptureDeviceGuid = devices[0].DriverGuid;
		
		Close();
	}
}
