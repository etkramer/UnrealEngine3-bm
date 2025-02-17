//----------------------------------------------------------------------------
// File: Algorithm.cs
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Drawing;
using System.Windows.Forms;
using Microsoft.DirectX.DirectSound;

namespace DSoundSample
{
    public class AlgorithmForm : Form
    {
        private Button buttonOk;
        private Button buttonCancel;
        private RadioButton radiobuttonNoVirtRadio;
        private RadioButton radiobuttonHighVirtRadio;
        private RadioButton radiobuttonLightVirtRadio;

        public AlgorithmForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
        }
        #region InitializeComponent code
        private void InitializeComponent()
        {
            this.buttonOk = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.radiobuttonNoVirtRadio = new System.Windows.Forms.RadioButton();
            this.radiobuttonHighVirtRadio = new System.Windows.Forms.RadioButton();
            this.radiobuttonLightVirtRadio = new System.Windows.Forms.RadioButton();
            this.SuspendLayout();
            // 
            // buttonOk
            // 
            this.buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOk.Location = new System.Drawing.Point(294, 72);
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.TabIndex = 0;
            this.buttonOk.Text = "OK";
            this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(375, 72);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.TabIndex = 1;
            this.buttonCancel.Text = "Cancel";
            // 
            // radiobuttonNoVirtRadio
            // 
            this.radiobuttonNoVirtRadio.Checked = true;
            this.radiobuttonNoVirtRadio.Location = new System.Drawing.Point(10, 11);
            this.radiobuttonNoVirtRadio.Name = "radiobuttonNoVirtRadio";
            this.radiobuttonNoVirtRadio.Size = new System.Drawing.Size(339, 16);
            this.radiobuttonNoVirtRadio.TabIndex = 2;
            this.radiobuttonNoVirtRadio.TabStop = true;
            this.radiobuttonNoVirtRadio.Text = "&No Virtualization (WDM or VxD. CPU efficient, but basic 3-D effect)";
            // 
            // radiobuttonHighVirtRadio
            // 
            this.radiobuttonHighVirtRadio.Location = new System.Drawing.Point(10, 28);
            this.radiobuttonHighVirtRadio.Name = "radiobuttonHighVirtRadio";
            this.radiobuttonHighVirtRadio.Size = new System.Drawing.Size(391, 16);
            this.radiobuttonHighVirtRadio.TabIndex = 3;
            this.radiobuttonHighVirtRadio.Text = "&High Quality (WDM only.  Highest quality 3D audio effect, but uses more CPU)";
            // 
            // radiobuttonLightVirtRadio
            // 
            this.radiobuttonLightVirtRadio.Location = new System.Drawing.Point(10, 46);
            this.radiobuttonLightVirtRadio.Name = "radiobuttonLightVirtRadio";
            this.radiobuttonLightVirtRadio.Size = new System.Drawing.Size(430, 16);
            this.radiobuttonLightVirtRadio.TabIndex = 4;
            this.radiobuttonLightVirtRadio.Text = "&Light Quality (WDM only.  Good 3-D audio effect, but uses less CPU than High Qua" +
                "lity)";
            // 
            // AlgorithmForm
            // 
            this.AcceptButton = this.buttonOk;
            this.ClientSize = new System.Drawing.Size(460, 111);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
																	  this.buttonOk,
																	  this.buttonCancel,
																	  this.radiobuttonNoVirtRadio,
																	  this.radiobuttonHighVirtRadio,
																	  this.radiobuttonLightVirtRadio});
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AlgorithmForm";
            this.Text = "Select 3D Algorithm ";
            this.ResumeLayout(false);

        }
        #endregion

        private void buttonOk_Click(object sender, System.EventArgs e)
        {
            if (true == radiobuttonNoVirtRadio.Checked)
                Play3DSound.guid3DAlgorithm = DSoundHelper.Guid3DAlgorithmNoVirtualization;
            else if (true == radiobuttonHighVirtRadio.Checked)
                Play3DSound.guid3DAlgorithm = DSoundHelper.Guid3DAlgorithmHrtfFull;
            else if (true == radiobuttonLightVirtRadio.Checked)
                Play3DSound.guid3DAlgorithm = DSoundHelper.Guid3DAlgorithmHrtfLight;

            this.Close();
        }
    }

}