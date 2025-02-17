//----------------------------------------------------------------------------
// File: wfAdjust.cs
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using Microsoft.DirectX.DirectSound;
using Buffer = Microsoft.DirectX.DirectSound.Buffer;

namespace csAdjustSound
{
    public class wfAdjust : System.Windows.Forms.Form
    {
        Device applicationDevice = null;
        SecondaryBuffer applicationBuffer = null;
        int lastGoodFrequency = 0;
        const int maxFrequency = 200000; // The maximum frequency we'll allow this sample to support

        #region WindowsForms Variables
        private System.Windows.Forms.Button buttonSound;
        private System.Windows.Forms.TextBox textFile;
        private System.Windows.Forms.TextBox textStatus;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button buttonExit;
        private System.Windows.Forms.Button buttonPlay;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.CheckBox checkLoop;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textFreq;
        private System.Windows.Forms.TrackBar tbarFreq;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox textPan;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Timers.Timer tmrUpdate;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.RadioButton radioSticky;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.RadioButton radioGlobal;
        private System.Windows.Forms.RadioButton radioNormal;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.RadioButton radioHardware;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.RadioButton radioSoftware;
        private System.Windows.Forms.RadioButton radioDefault;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.Label lblBehavior;
        private System.Windows.Forms.TrackBar tbarPan;
        private System.Windows.Forms.TrackBar tbarVolume;
        private System.Windows.Forms.OpenFileDialog ofdFile;
        private System.Windows.Forms.TextBox textVolume;
        private System.Windows.Forms.Label lblMaxFreq;
        private System.Windows.Forms.Label lblMinFreq;
        private System.ComponentModel.Container components = null;
        #endregion

        public wfAdjust()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

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

            try
            {
                //Initialize DirectSound
                applicationDevice = new Device();
                applicationDevice.SetCooperativeLevel(this, CooperativeLevel.Priority);
            }
            catch
            {
                MessageBox.Show("Could not initialize DirectSound.  Sample will exit.", "Exiting...", MessageBoxButtons.OK, MessageBoxIcon.Error);
                this.Close();
                throw;
            }

            // Now that we have a sound device object set the frequency sliders correctly
            this.tbarFreq.Minimum = 100;
            this.tbarFreq.Maximum = maxFrequency;
            this.lblMinFreq.Text = string.Format("{0} Hz", this.tbarFreq.Minimum);
            this.lblMaxFreq.Text = string.Format("{0} KHz", this.tbarFreq.Maximum/1000);

            UpdateBehaviorText();
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (null != components) 
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
            this.radioDefault = new System.Windows.Forms.RadioButton();
            this.tmrUpdate = new System.Timers.Timer();
            this.checkLoop = new System.Windows.Forms.CheckBox();
            this.textVolume = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.radioHardware = new System.Windows.Forms.RadioButton();
            this.label12 = new System.Windows.Forms.Label();
            this.radioSoftware = new System.Windows.Forms.RadioButton();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.radioSticky = new System.Windows.Forms.RadioButton();
            this.label13 = new System.Windows.Forms.Label();
            this.radioGlobal = new System.Windows.Forms.RadioButton();
            this.radioNormal = new System.Windows.Forms.RadioButton();
            this.textFile = new System.Windows.Forms.TextBox();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.lblBehavior = new System.Windows.Forms.Label();
            this.buttonSound = new System.Windows.Forms.Button();
            this.textStatus = new System.Windows.Forms.TextBox();
            this.buttonPlay = new System.Windows.Forms.Button();
            this.tbarPan = new System.Windows.Forms.TrackBar();
            this.ofdFile = new System.Windows.Forms.OpenFileDialog();
            this.buttonExit = new System.Windows.Forms.Button();
            this.label11 = new System.Windows.Forms.Label();
            this.tbarVolume = new System.Windows.Forms.TrackBar();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.buttonStop = new System.Windows.Forms.Button();
            this.lblMaxFreq = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.lblMinFreq = new System.Windows.Forms.Label();
            this.textFreq = new System.Windows.Forms.TextBox();
            this.tbarFreq = new System.Windows.Forms.TrackBar();
            this.textPan = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.tmrUpdate)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox5.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.tbarPan)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbarVolume)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbarFreq)).BeginInit();
            this.SuspendLayout();
            // 
            // radioDefault
            // 
            this.radioDefault.Checked = true;
            this.radioDefault.Location = new System.Drawing.Point(93, 16);
            this.radioDefault.Name = "radioDefault";
            this.radioDefault.Size = new System.Drawing.Size(79, 14);
            this.radioDefault.TabIndex = 3;
            this.radioDefault.TabStop = true;
            this.radioDefault.Text = "Default";
            this.radioDefault.CheckedChanged += new System.EventHandler(this.RadioChecked);
            // 
            // tmrUpdate
            // 
            this.tmrUpdate.Enabled = true;
            this.tmrUpdate.SynchronizingObject = this;
            this.tmrUpdate.Elapsed += new System.Timers.ElapsedEventHandler(this.tmrUpdate_Elapsed);
            // 
            // checkLoop
            // 
            this.checkLoop.Location = new System.Drawing.Point(8, 399);
            this.checkLoop.Name = "checkLoop";
            this.checkLoop.Size = new System.Drawing.Size(151, 19);
            this.checkLoop.TabIndex = 3;
            this.checkLoop.Text = "Loop Sound";
            // 
            // textVolume
            // 
            this.textVolume.Location = new System.Drawing.Point(85, 148);
            this.textVolume.Name = "textVolume";
            this.textVolume.ReadOnly = true;
            this.textVolume.Size = new System.Drawing.Size(43, 20);
            this.textVolume.TabIndex = 1;
            this.textVolume.Text = "0";
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(6, 140);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(73, 38);
            this.label10.TabIndex = 2;
            this.label10.Text = "Volume";
            this.label10.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                    this.groupBox4,
                                                                                    this.groupBox2});
            this.groupBox1.Location = new System.Drawing.Point(6, 181);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(437, 91);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Buffer Settings";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                    this.radioHardware,
                                                                                    this.label12,
                                                                                    this.radioSoftware,
                                                                                    this.radioDefault});
            this.groupBox4.Location = new System.Drawing.Point(8, 48);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(423, 36);
            this.groupBox4.TabIndex = 6;
            this.groupBox4.TabStop = false;
            // 
            // radioHardware
            // 
            this.radioHardware.Location = new System.Drawing.Point(173, 16);
            this.radioHardware.Name = "radioHardware";
            this.radioHardware.Size = new System.Drawing.Size(79, 14);
            this.radioHardware.TabIndex = 3;
            this.radioHardware.Text = "Hardware";
            this.radioHardware.CheckedChanged += new System.EventHandler(this.RadioChecked);
            // 
            // label12
            // 
            this.label12.Location = new System.Drawing.Point(6, 13);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(73, 15);
            this.label12.TabIndex = 2;
            this.label12.Text = "Buffer Mixing";
            this.label12.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // radioSoftware
            // 
            this.radioSoftware.Location = new System.Drawing.Point(268, 17);
            this.radioSoftware.Name = "radioSoftware";
            this.radioSoftware.Size = new System.Drawing.Size(79, 14);
            this.radioSoftware.TabIndex = 3;
            this.radioSoftware.Text = "Software";
            this.radioSoftware.CheckedChanged += new System.EventHandler(this.RadioChecked);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                    this.radioSticky,
                                                                                    this.label13,
                                                                                    this.radioGlobal,
                                                                                    this.radioNormal});
            this.groupBox2.Location = new System.Drawing.Point(7, 11);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(423, 36);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            // 
            // radioSticky
            // 
            this.radioSticky.Location = new System.Drawing.Point(173, 16);
            this.radioSticky.Name = "radioSticky";
            this.radioSticky.Size = new System.Drawing.Size(79, 16);
            this.radioSticky.TabIndex = 3;
            this.radioSticky.Text = "Sticky";
            this.radioSticky.CheckedChanged += new System.EventHandler(this.RadioChecked);
            // 
            // label13
            // 
            this.label13.Location = new System.Drawing.Point(6, 13);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(73, 15);
            this.label13.TabIndex = 2;
            this.label13.Text = "Focus";
            this.label13.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // radioGlobal
            // 
            this.radioGlobal.Location = new System.Drawing.Point(268, 17);
            this.radioGlobal.Name = "radioGlobal";
            this.radioGlobal.Size = new System.Drawing.Size(79, 14);
            this.radioGlobal.TabIndex = 3;
            this.radioGlobal.Text = "Global";
            this.radioGlobal.CheckedChanged += new System.EventHandler(this.RadioChecked);
            // 
            // radioNormal
            // 
            this.radioNormal.Checked = true;
            this.radioNormal.Location = new System.Drawing.Point(93, 16);
            this.radioNormal.Name = "radioNormal";
            this.radioNormal.Size = new System.Drawing.Size(79, 14);
            this.radioNormal.TabIndex = 3;
            this.radioNormal.TabStop = true;
            this.radioNormal.Text = "Normal";
            this.radioNormal.CheckedChanged += new System.EventHandler(this.RadioChecked);
            // 
            // textFile
            // 
            this.textFile.Location = new System.Drawing.Point(85, 6);
            this.textFile.Name = "textFile";
            this.textFile.ReadOnly = true;
            this.textFile.Size = new System.Drawing.Size(350, 20);
            this.textFile.TabIndex = 1;
            this.textFile.Text = "";
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                    this.lblBehavior});
            this.groupBox5.Location = new System.Drawing.Point(8, 278);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(431, 120);
            this.groupBox5.TabIndex = 6;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Expected Behavior";
            // 
            // lblBehavior
            // 
            this.lblBehavior.Location = new System.Drawing.Point(6, 16);
            this.lblBehavior.Name = "lblBehavior";
            this.lblBehavior.Size = new System.Drawing.Size(422, 100);
            this.lblBehavior.TabIndex = 0;
            this.lblBehavior.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // buttonSound
            // 
            this.buttonSound.Location = new System.Drawing.Point(3, 6);
            this.buttonSound.Name = "buttonSound";
            this.buttonSound.Size = new System.Drawing.Size(74, 21);
            this.buttonSound.TabIndex = 0;
            this.buttonSound.Text = "Sound File...";
            this.buttonSound.Click += new System.EventHandler(this.buttonSound_Click);
            // 
            // textStatus
            // 
            this.textStatus.Location = new System.Drawing.Point(85, 33);
            this.textStatus.Name = "textStatus";
            this.textStatus.ReadOnly = true;
            this.textStatus.Size = new System.Drawing.Size(350, 20);
            this.textStatus.TabIndex = 1;
            this.textStatus.Text = "No File Loaded.";
            // 
            // buttonPlay
            // 
            this.buttonPlay.Enabled = false;
            this.buttonPlay.Location = new System.Drawing.Point(7, 421);
            this.buttonPlay.Name = "buttonPlay";
            this.buttonPlay.Size = new System.Drawing.Size(74, 21);
            this.buttonPlay.TabIndex = 0;
            this.buttonPlay.Text = "Play";
            this.buttonPlay.Click += new System.EventHandler(this.buttonPlay_Click);
            // 
            // tbarPan
            // 
            this.tbarPan.Location = new System.Drawing.Point(164, 97);
            this.tbarPan.Maximum = 20;
            this.tbarPan.Minimum = -20;
            this.tbarPan.Name = "tbarPan";
            this.tbarPan.Size = new System.Drawing.Size(236, 42);
            this.tbarPan.TabIndex = 4;
            this.tbarPan.TickFrequency = 5;
            this.tbarPan.Scroll += new System.EventHandler(this.tbarPan_Scroll);
            // 
            // ofdFile
            // 
            this.ofdFile.Filter = "Wave Files (*.wav)|*.wav|All Files (*.*)|*.*";
            this.ofdFile.Title = "Open Audio File";
            // 
            // buttonExit
            // 
            this.buttonExit.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonExit.Location = new System.Drawing.Point(362, 421);
            this.buttonExit.Name = "buttonExit";
            this.buttonExit.Size = new System.Drawing.Size(74, 21);
            this.buttonExit.TabIndex = 0;
            this.buttonExit.Text = "Exit";
            this.buttonExit.Click += new System.EventHandler(this.buttonExit_Click);
            // 
            // label11
            // 
            this.label11.Location = new System.Drawing.Point(6, 13);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(73, 15);
            this.label11.TabIndex = 2;
            this.label11.Text = "Focus";
            this.label11.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // tbarVolume
            // 
            this.tbarVolume.Location = new System.Drawing.Point(165, 140);
            this.tbarVolume.Maximum = 0;
            this.tbarVolume.Minimum = -50;
            this.tbarVolume.Name = "tbarVolume";
            this.tbarVolume.Size = new System.Drawing.Size(236, 42);
            this.tbarVolume.TabIndex = 4;
            this.tbarVolume.Scroll += new System.EventHandler(this.tbarVolume_Scroll);
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(131, 147);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(41, 20);
            this.label8.TabIndex = 2;
            this.label8.Text = "Low";
            this.label8.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label9
            // 
            this.label9.Location = new System.Drawing.Point(396, 149);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(47, 20);
            this.label9.TabIndex = 2;
            this.label9.Text = "High";
            this.label9.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // buttonStop
            // 
            this.buttonStop.Enabled = false;
            this.buttonStop.Location = new System.Drawing.Point(87, 421);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(74, 21);
            this.buttonStop.TabIndex = 0;
            this.buttonStop.Text = "Stop";
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // lblMaxFreq
            // 
            this.lblMaxFreq.Location = new System.Drawing.Point(396, 68);
            this.lblMaxFreq.Name = "lblMaxFreq";
            this.lblMaxFreq.Size = new System.Drawing.Size(47, 20);
            this.lblMaxFreq.TabIndex = 2;
            this.lblMaxFreq.Text = "100 KHz";
            this.lblMaxFreq.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(130, 104);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(41, 20);
            this.label5.TabIndex = 2;
            this.label5.Text = "Left";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(395, 106);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(47, 20);
            this.label6.TabIndex = 2;
            this.label6.Text = "Right";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(5, 97);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(73, 38);
            this.label7.TabIndex = 2;
            this.label7.Text = "Pan";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(4, 33);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(73, 20);
            this.label1.TabIndex = 2;
            this.label1.Text = "Status";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(6, 59);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(73, 38);
            this.label2.TabIndex = 2;
            this.label2.Text = "Frequency";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblMinFreq
            // 
            this.lblMinFreq.Location = new System.Drawing.Point(131, 66);
            this.lblMinFreq.Name = "lblMinFreq";
            this.lblMinFreq.Size = new System.Drawing.Size(41, 20);
            this.lblMinFreq.TabIndex = 2;
            this.lblMinFreq.Text = "100 Hz";
            this.lblMinFreq.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // textFreq
            // 
            this.textFreq.Location = new System.Drawing.Point(85, 67);
            this.textFreq.Name = "textFreq";
            this.textFreq.ReadOnly = true;
            this.textFreq.Size = new System.Drawing.Size(43, 20);
            this.textFreq.TabIndex = 1;
            this.textFreq.Text = "0";
            // 
            // tbarFreq
            // 
            this.tbarFreq.LargeChange = 1000;
            this.tbarFreq.Location = new System.Drawing.Point(165, 59);
            this.tbarFreq.Maximum = 100000;
            this.tbarFreq.Minimum = 100;
            this.tbarFreq.Name = "tbarFreq";
            this.tbarFreq.Size = new System.Drawing.Size(236, 42);
            this.tbarFreq.SmallChange = 100;
            this.tbarFreq.TabIndex = 4;
            this.tbarFreq.TickFrequency = 10000;
            this.tbarFreq.Value = 100;
            this.tbarFreq.Scroll += new System.EventHandler(this.tbarFreq_Scroll);
            // 
            // textPan
            // 
            this.textPan.Location = new System.Drawing.Point(85, 105);
            this.textPan.Name = "textPan";
            this.textPan.ReadOnly = true;
            this.textPan.Size = new System.Drawing.Size(43, 20);
            this.textPan.TabIndex = 1;
            this.textPan.Text = "0";
            // 
            // wfAdjust
            // 
            this.AcceptButton = this.buttonSound;
            this.CancelButton = this.buttonExit;
            this.ClientSize = new System.Drawing.Size(460, 448);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.groupBox5,
                                                                          this.groupBox1,
                                                                          this.textVolume,
                                                                          this.label8,
                                                                          this.label9,
                                                                          this.label10,
                                                                          this.tbarVolume,
                                                                          this.label5,
                                                                          this.label6,
                                                                          this.tbarPan,
                                                                          this.textPan,
                                                                          this.label7,
                                                                          this.lblMaxFreq,
                                                                          this.lblMinFreq,
                                                                          this.textFreq,
                                                                          this.tbarFreq,
                                                                          this.label2,
                                                                          this.checkLoop,
                                                                          this.buttonStop,
                                                                          this.buttonPlay,
                                                                          this.buttonExit,
                                                                          this.label1,
                                                                          this.textStatus,
                                                                          this.textFile,
                                                                          this.buttonSound});
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Name = "wfAdjust";
            this.Text = "AdjustSound";
            ((System.ComponentModel.ISupportInitialize)(this.tmrUpdate)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox5.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.tbarPan)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbarVolume)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbarFreq)).EndInit();
            this.ResumeLayout(false);

        }
        #endregion

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main() 
        {
            try
            {
                using (wfAdjust form = new wfAdjust())
                {
                    Application.Run(form);    
                }
            }
            catch{} 
        }

        private void buttonExit_Click(object sender, System.EventArgs e)
        {
            this.Dispose();
        }

        private void buttonSound_Click(object sender, System.EventArgs e)
        {
            bool FocusSticky = radioSticky.Checked;
            bool FocusGlobal = radioGlobal.Checked;
            bool MixHardware = radioHardware.Checked;
            bool MixSoftware = radioSoftware.Checked;

            // Make sure we're stopped
            this.buttonStop_Click(null, null);
            textStatus.Text = "Loading file...";
            if (null != applicationBuffer)
                applicationBuffer.Dispose();
            applicationBuffer = null;

            tbarFreq.Value = tbarFreq.Minimum;
            tbarPan.Value = 0;
            tbarVolume.Value = 0;
            // Show the open file dialog and let's load the file
            if (null == ofdFile.InitialDirectory)
            {
                // Default to the 'My documents' folder if it's available
                ofdFile.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
            }
            // Clear out any old file name that may be in there
            ofdFile.FileName = null;
            if (DialogResult.OK == ofdFile.ShowDialog())
            {
                // Save the initial dir as the last one picked.
                ofdFile.InitialDirectory = System.IO.Path.GetDirectoryName(ofdFile.FileName);
                try
                {
                    BufferDescription desc = new BufferDescription();
                    
                    desc.ControlFrequency  = true;
                    desc.ControlPan = true;
                    desc.ControlVolume = true;

                    if (FocusGlobal)
                        desc.GlobalFocus = true;

                    if (FocusSticky)
                        desc.StickyFocus = true;

                    if (MixHardware)
                        desc.LocateInHardware = true;

                    if (MixSoftware)
                        desc.LocateInSoftware = true;

                    applicationBuffer = new SecondaryBuffer(ofdFile.FileName, desc, applicationDevice);
                    tbarFreq.Value = desc.Format.SamplesPerSecond;
                    lastGoodFrequency = desc.Format.SamplesPerSecond;
                    textFile.Text = ofdFile.FileName;
                    textFreq.Text = tbarFreq.Value.ToString();
                    textStatus.Text = "File loaded.";
                    EnablePlayUI(true);
                }
                catch (Exception de)
                {
                    Console.WriteLine(de.ToString());
                    textFile.Text = null;
                    textStatus.Text = "Could not load this segment.";
                    DefaultPlayUI();
                }
            }
            else
            {
                textFile.Text = null;
                textStatus.Text = "Load aborted.";
                DefaultPlayUI();
            }
        }

        private void buttonStop_Click(object sender, System.EventArgs e)
        {
            if (null != applicationBuffer)
            {
                textStatus.Text = "Sound stopped.";
                EnablePlayUI(true);
                applicationBuffer.Stop();
                applicationBuffer.SetCurrentPosition(0);
            }
        }

        private void tbarFreq_Scroll(object sender, System.EventArgs e)
        {
            int newFrequency = 0;
            if (null != applicationBuffer)
            {
                try
                {
                    newFrequency = ((TrackBar)sender).Value;
                    // Attempt to set the frequency to the new value
                    applicationBuffer.Frequency = newFrequency;
                    textFreq.Text = newFrequency.ToString();
                    lastGoodFrequency = newFrequency;
                }
                catch 
                {
                    // Let's try to guess why it failed..
                    if ((applicationBuffer.Caps.LocateInHardware) && (newFrequency > applicationDevice.Caps.MaxSecondarySampleRate))
                    {
                        textStatus.Text = "Hardware buffers don't support greater than Caps.MaxSecondarySampleRate";
                    }
                    else if (100000 < newFrequency)
                    {
                        // Some platforms (pre-WinXP SP1) don't support 
                        // >100k Hz so they will fail when setting it higher
                        textStatus.Text = "Some OS platforms do not support >100k Hz";
                    }
                    else
                    {
                        textStatus.Text = "Setting the frequency failed";
                    }
                    // Reset to the last valid frequency
                    applicationBuffer.Frequency = lastGoodFrequency;
                    ((TrackBar)sender).Value = lastGoodFrequency;
                }
            }
        }

        private void buttonPlay_Click(object sender, System.EventArgs e)
        {
            bool FocusSticky = radioSticky.Checked;
            bool FocusGlobal = radioGlobal.Checked;
            bool MixHardware = radioHardware.Checked;
            bool MixSoftware = radioSoftware.Checked;

            if (null != applicationBuffer)
            {
                textStatus.Text = "Sound playing.";
                EnablePlayUI(false);
                Application.DoEvents(); // Process the Stop click that EnablePlayUI generates.

                // First we need to 'recreate' the buffer
                if (null != applicationBuffer)
                    applicationBuffer.Dispose();
                applicationBuffer = null;

                BufferDescription desc = new BufferDescription();
                desc.ControlFrequency  = true;
                desc.ControlPan = true;
                desc.ControlVolume = true;

                if (FocusGlobal)
                    desc.GlobalFocus = true;

                if (FocusSticky)
                    desc.StickyFocus = true;

                if (MixHardware)
                    desc.LocateInHardware = true;

                if (MixSoftware)
                    desc.LocateInSoftware = true;

                try
                {
                    applicationBuffer = new SecondaryBuffer(ofdFile.FileName, desc, applicationDevice);

                    BufferPlayFlags PlayFlags  = checkLoop.Checked ? BufferPlayFlags.Looping : 0;

                    // Before we play, make sure we're using the correct settings
                    tbarFreq_Scroll(tbarFreq, null);
                    tbarPan_Scroll(tbarPan, null);
                    tbarVolume_Scroll(tbarVolume, null);
                    applicationBuffer.Play(0, PlayFlags);
                }
                catch
                {
                    textStatus.Text = "Could not open this file with these settings.";
                    DefaultPlayUI();
                }
            }
        }

        private void tbarPan_Scroll(object sender, System.EventArgs e)
        {
            if (null != applicationBuffer)
            {
                textPan.Text = ((TrackBar)sender).Value.ToString();
                applicationBuffer.Pan = ((TrackBar)sender).Value * 500;
            }
        }

        private void tbarVolume_Scroll(object sender, System.EventArgs e)
        {
            if (null != applicationBuffer)
            {
                textVolume.Text = ((TrackBar)sender).Value.ToString();
                applicationBuffer.Volume = ((TrackBar)sender).Value * 100;
            }
        }
        private void EnablePlayUI(bool bEnable)
        {
            buttonPlay.Enabled = bEnable;
            buttonStop.Enabled = !bEnable;
            buttonSound.Enabled = bEnable;
            checkLoop.Enabled = bEnable;
            radioDefault.Enabled = bEnable;
            radioGlobal.Enabled = bEnable;
            radioHardware.Enabled = bEnable;
            radioNormal.Enabled = bEnable;
            radioSoftware.Enabled = bEnable;
            radioSticky.Enabled = bEnable;
        }
        private void DefaultPlayUI()
        {
            buttonPlay.Enabled = false;
            buttonStop.Enabled = false;
            buttonSound.Enabled = true;
            checkLoop.Enabled = false;
            radioDefault.Enabled = true;
            radioGlobal.Enabled = true;
            radioHardware.Enabled = true;
            radioNormal.Enabled = true;
            radioSoftware.Enabled = true;
            radioSticky.Enabled = true;
            textFile.Text = null;
        }
        private void tmrUpdate_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            if (null != applicationBuffer)
            {
                if (false == applicationBuffer.Status.Playing && false == applicationBuffer.Status.Looping)
                    buttonStop_Click(null, null);
            }
        }
        private void UpdateBehaviorText()
        {
            string sText = null;
            bool Looped      = checkLoop.Checked;
            bool FocusSticky = radioSticky.Checked;
            bool FocusGlobal = radioGlobal.Checked;
            bool MixHardware = radioHardware.Checked;
            bool MixSoftware = radioSoftware.Checked;

            // Figure what the user should expect based on the dialog choice
            if (FocusSticky)
            {
                sText = "Buffers with \"sticky\" focus will continue to play if the user switches to another application not using DirectSound.  However, if the user switches to another DirectSound application, all normal-focus and sticky-focus buffers in the previous application are muted.";

            }
            else if (FocusGlobal)
            {
                sText = "Buffers with global focus will continue to play if the user switches focus to another application, even if the new application uses DirectSound. The one exception is if you switch focus to a DirectSound application that uses the DSSCL_WRITEPRIMARY cooperative level. In this case, the global-focus buffers from other applications will not be audible.";
            }
            else
            {
                // Normal focus
                sText = "Buffers with normal focus will mute if the user switches focus to any other application";
            }

            if (MixHardware)
            {
                sText = sText + "\n\nWith the hardware mixing flag, the new buffer will be forced to use hardware mixing. If the device does not support hardware mixing or if the required hardware resources are not available, the call to the DirectSound.CreateSoundBuffer method will fail."; 
            }
            else if (MixSoftware)
            {
                sText = sText + "\n\nWith the software mixing flag, the new buffer will use software mixing, even if hardware resources are available.";
            }
            else 
            {
                // Default mixing
                sText = sText + "\n\nWith default mixing, the new buffer will use hardware mixing if available, otherwise software mixing will be used."; 
            }
            lblBehavior.Text = sText;
        }

        private void RadioChecked(object sender, System.EventArgs e)
        {
            UpdateBehaviorText();
        }
    }
}
