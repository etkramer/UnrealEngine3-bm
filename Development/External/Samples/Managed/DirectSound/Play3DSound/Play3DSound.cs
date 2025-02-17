//----------------------------------------------------------------------------
// File: Play3DSound.cs
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Drawing;
using Color = System.Drawing.Color;
using System.Windows.Forms;
using Microsoft.DirectX;
using Microsoft.DirectX.DirectSound;
using Microsoft.Samples.DirectX.UtilityToolkit;
using Buffer = Microsoft.DirectX.DirectSound.Buffer;

namespace DSoundSample
{
    public class Play3DSound : Form
    {
        #region Class Variables
        private System.Windows.Forms.Button buttonSoundfile;
        private Label labelFilename;
        private Label labelStatic;
        private Label labelStatus;
        private GroupBox groupboxStatic;
        private Label labelStatic1;
        private Label labelDopplerfactor;
        private TrackBar trackbarDopplerSlider;
        private Label labelStatic2;
        private Label labelRollofffactor;
        private TrackBar trackbarRolloffSlider;
        private Label labelStatic3;
        private Label labelMindistance;
        private TrackBar trackbarMindistanceSlider;
        private Label labelStatic4;
        private Label labelMaxdistance;
        private TrackBar trackbarMaxdistanceSlider;
        private GroupBox groupboxStatic1;
        private PictureBox pictureboxRenderWindow;
        private TrackBar trackbarVerticalSlider;
        private TrackBar trackbarHorizontalSlider;
        private System.Windows.Forms.Button buttonPlay;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.Button buttonCancel;
        private CheckBox checkboxDefer;
        private System.Windows.Forms.Button buttonApply;
        private System.Timers.Timer timerMovement;
        #endregion

        public delegate void UpdateGridDelegate(float x, float y);
        private Listener3DSettings listenerParameters = new Listener3DSettings();
        private Buffer3DSettings application3DSettings = new Buffer3DSettings();
        private SecondaryBuffer applicationBuffer = null;
        public static Guid guid3DAlgorithm = Guid.Empty;
        private Device applicationDevice = null;
        private Listener3D applicationListener = null;
        private Buffer3D applicationBuffer3D = null;
        private Graphics applicationGraphics = null;
        private const float maxOrbitRadius = 1.0f;
        private string FileName = string.Empty;
        private string Path = string.Empty;
        private Bitmap bitmapGrid = null;
        private int GridHeight = 0;
        private int GridWidth = 0;
        private int X = 0;
        private int Y = 0;

        [STAThread]
        public static void Main()
        {
            try
            {
                using (Play3DSound form = new Play3DSound())
                {
                    Application.Run(form);
                }
            }
            catch { }
        }

        public Play3DSound()
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
                catch { }
            }
            //
            // Required for Windows Form Designer support
            //

            InitializeComponent();

            BufferDescription description = new BufferDescription();
            WaveFormat fmt = new WaveFormat();
            description.PrimaryBuffer = true;
            description.Control3D = true;
            Buffer buff = null;

            fmt.FormatTag = WaveFormatTag.Pcm;
            fmt.Channels = 2;
            fmt.SamplesPerSecond = 22050;
            fmt.BitsPerSample = 16;
            fmt.BlockAlign = (short)(fmt.BitsPerSample / 8 * fmt.Channels);
            fmt.AverageBytesPerSecond = fmt.SamplesPerSecond * fmt.BlockAlign;

            try
            {
                applicationDevice = new Device();
                applicationDevice.SetCooperativeLevel(this, CooperativeLevel.Priority);
            }
            catch
            {
                MessageBox.Show("Unable to create sound device. Sample will now exit.");
                this.Close();
                throw;
            }

            // Get the primary buffer and set the format.
            buff = new Buffer(description, applicationDevice);
            buff.Format = fmt;

            applicationListener = new Listener3D(buff);
            listenerParameters = applicationListener.AllParameters;

            labelFilename.Text = String.Empty;
            labelStatus.Text = "No file loaded.";

            try
            {
                pictureboxRenderWindow.BackgroundImage = new Bitmap(this.GetType(), "grid.jpg");
            }
            catch
            {
                // Load the grid from a file if resource loading failed
                string path = Utility.FindMediaFile("grid.jpg");
                pictureboxRenderWindow.BackgroundImage = Image.FromFile(path);
            }
            GridWidth = pictureboxRenderWindow.Width;
            GridHeight = pictureboxRenderWindow.Height;

            trackbarDopplerSlider.Maximum = ConvertLogScaleToLinearSliderPos(DSoundHelper.MaxDopplerFactor);
            trackbarDopplerSlider.Minimum = ConvertLogScaleToLinearSliderPos(DSoundHelper.MinDopplerFactor);

            trackbarRolloffSlider.Maximum = ConvertLogScaleToLinearSliderPos(DSoundHelper.MaxRolloffFactor);
            trackbarRolloffSlider.Minimum = ConvertLogScaleToLinearSliderPos(DSoundHelper.MinRolloffFactor);

            trackbarMindistanceSlider.Maximum = 40;
            trackbarMindistanceSlider.Minimum = 1;

            trackbarMaxdistanceSlider.Maximum = 40;
            trackbarMaxdistanceSlider.Minimum = 1;

            trackbarVerticalSlider.Maximum = 100;
            trackbarVerticalSlider.Minimum = -100;
            trackbarVerticalSlider.Value = 100;

            trackbarHorizontalSlider.Maximum = 100;
            trackbarHorizontalSlider.Minimum = -100;
            trackbarHorizontalSlider.Value = 100;

            SetSlidersPos(0, 0, maxOrbitRadius, maxOrbitRadius * 20.0f);
            SliderChanged();
        }
        #region InitializeComponent code
        private void InitializeComponent()
        {
            this.buttonSoundfile = new System.Windows.Forms.Button();
            this.labelFilename = new System.Windows.Forms.Label();
            this.labelStatic = new System.Windows.Forms.Label();
            this.labelStatus = new System.Windows.Forms.Label();
            this.groupboxStatic = new System.Windows.Forms.GroupBox();
            this.buttonApply = new System.Windows.Forms.Button();
            this.trackbarMaxdistanceSlider = new System.Windows.Forms.TrackBar();
            this.trackbarDopplerSlider = new System.Windows.Forms.TrackBar();
            this.trackbarMindistanceSlider = new System.Windows.Forms.TrackBar();
            this.trackbarRolloffSlider = new System.Windows.Forms.TrackBar();
            this.checkboxDefer = new System.Windows.Forms.CheckBox();
            this.labelMaxdistance = new System.Windows.Forms.Label();
            this.labelStatic4 = new System.Windows.Forms.Label();
            this.labelRollofffactor = new System.Windows.Forms.Label();
            this.labelStatic2 = new System.Windows.Forms.Label();
            this.labelStatic3 = new System.Windows.Forms.Label();
            this.labelMindistance = new System.Windows.Forms.Label();
            this.labelStatic1 = new System.Windows.Forms.Label();
            this.labelDopplerfactor = new System.Windows.Forms.Label();
            this.groupboxStatic1 = new System.Windows.Forms.GroupBox();
            this.pictureboxRenderWindow = new System.Windows.Forms.PictureBox();
            this.trackbarVerticalSlider = new System.Windows.Forms.TrackBar();
            this.trackbarHorizontalSlider = new System.Windows.Forms.TrackBar();
            this.buttonPlay = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.timerMovement = new System.Timers.Timer();
            this.groupboxStatic.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarMaxdistanceSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarDopplerSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarMindistanceSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarRolloffSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarVerticalSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarHorizontalSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.timerMovement)).BeginInit();
            this.SuspendLayout();
            // 
            // buttonSoundfile
            // 
            this.buttonSoundfile.Location = new System.Drawing.Point(10, 11);
            this.buttonSoundfile.Name = "buttonSoundfile";
            this.buttonSoundfile.Size = new System.Drawing.Size(78, 21);
            this.buttonSoundfile.TabIndex = 0;
            this.buttonSoundfile.Text = "Sound &file...";
            this.buttonSoundfile.Click += new System.EventHandler(this.buttonSoundfile_Click);
            // 
            // labelFilename
            // 
            this.labelFilename.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.labelFilename.Location = new System.Drawing.Point(102, 11);
            this.labelFilename.Name = "labelFilename";
            this.labelFilename.Size = new System.Drawing.Size(334, 21);
            this.labelFilename.TabIndex = 1;
            this.labelFilename.Text = "Static";
            this.labelFilename.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // labelStatic
            // 
            this.labelStatic.Location = new System.Drawing.Point(10, 41);
            this.labelStatic.Name = "labelStatic";
            this.labelStatic.Size = new System.Drawing.Size(63, 21);
            this.labelStatic.TabIndex = 2;
            this.labelStatic.Text = "Status";
            this.labelStatic.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // labelStatus
            // 
            this.labelStatus.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.labelStatus.Location = new System.Drawing.Point(102, 41);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(334, 21);
            this.labelStatus.TabIndex = 3;
            this.labelStatus.Text = "Static";
            this.labelStatus.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // groupboxStatic
            // 
            this.groupboxStatic.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                               this.buttonApply,
                                                                               this.trackbarMaxdistanceSlider,
                                                                               this.trackbarDopplerSlider,
                                                                               this.trackbarMindistanceSlider,
                                                                               this.trackbarRolloffSlider,
                                                                               this.checkboxDefer,
                                                                               this.labelMaxdistance,
                                                                               this.labelStatic4,
                                                                               this.labelRollofffactor,
                                                                               this.labelStatic2,
                                                                               this.labelStatic3,
                                                                               this.labelMindistance});
            this.groupboxStatic.Location = new System.Drawing.Point(10, 72);
            this.groupboxStatic.Name = "groupboxStatic";
            this.groupboxStatic.Size = new System.Drawing.Size(300, 240);
            this.groupboxStatic.TabIndex = 4;
            this.groupboxStatic.TabStop = false;
            this.groupboxStatic.Text = "Sound properties";
            // 
            // buttonApply
            // 
            this.buttonApply.Location = new System.Drawing.Point(184, 208);
            this.buttonApply.Name = "buttonApply";
            this.buttonApply.Size = new System.Drawing.Size(88, 18);
            this.buttonApply.TabIndex = 25;
            this.buttonApply.Text = "Apply Settings";
            this.buttonApply.Click += new System.EventHandler(this.buttonApply_Click);
            // 
            // trackbarMaxdistanceSlider
            // 
            this.trackbarMaxdistanceSlider.AutoSize = false;
            this.trackbarMaxdistanceSlider.Location = new System.Drawing.Point(160, 168);
            this.trackbarMaxdistanceSlider.Name = "trackbarMaxdistanceSlider";
            this.trackbarMaxdistanceSlider.Size = new System.Drawing.Size(135, 45);
            this.trackbarMaxdistanceSlider.TabIndex = 16;
            this.trackbarMaxdistanceSlider.Text = "Slider3";
            this.trackbarMaxdistanceSlider.TickStyle = System.Windows.Forms.TickStyle.None;
            this.trackbarMaxdistanceSlider.Scroll += new System.EventHandler(this.trackbarSlider_Scroll);
            // 
            // trackbarDopplerSlider
            // 
            this.trackbarDopplerSlider.AutoSize = false;
            this.trackbarDopplerSlider.Location = new System.Drawing.Point(160, 24);
            this.trackbarDopplerSlider.Name = "trackbarDopplerSlider";
            this.trackbarDopplerSlider.Size = new System.Drawing.Size(135, 45);
            this.trackbarDopplerSlider.TabIndex = 7;
            this.trackbarDopplerSlider.Text = "Slider1";
            this.trackbarDopplerSlider.TickStyle = System.Windows.Forms.TickStyle.None;
            this.trackbarDopplerSlider.Scroll += new System.EventHandler(this.trackbarSlider_Scroll);
            // 
            // trackbarMindistanceSlider
            // 
            this.trackbarMindistanceSlider.AutoSize = false;
            this.trackbarMindistanceSlider.Location = new System.Drawing.Point(160, 120);
            this.trackbarMindistanceSlider.Name = "trackbarMindistanceSlider";
            this.trackbarMindistanceSlider.Size = new System.Drawing.Size(135, 45);
            this.trackbarMindistanceSlider.TabIndex = 13;
            this.trackbarMindistanceSlider.Text = "Slider3";
            this.trackbarMindistanceSlider.TickStyle = System.Windows.Forms.TickStyle.None;
            this.trackbarMindistanceSlider.Scroll += new System.EventHandler(this.trackbarSlider_Scroll);
            // 
            // trackbarRolloffSlider
            // 
            this.trackbarRolloffSlider.AutoSize = false;
            this.trackbarRolloffSlider.Location = new System.Drawing.Point(160, 72);
            this.trackbarRolloffSlider.Name = "trackbarRolloffSlider";
            this.trackbarRolloffSlider.Size = new System.Drawing.Size(135, 45);
            this.trackbarRolloffSlider.TabIndex = 10;
            this.trackbarRolloffSlider.Text = "Slider2";
            this.trackbarRolloffSlider.TickStyle = System.Windows.Forms.TickStyle.None;
            this.trackbarRolloffSlider.Scroll += new System.EventHandler(this.trackbarSlider_Scroll);
            // 
            // checkboxDefer
            // 
            this.checkboxDefer.Appearance = System.Windows.Forms.Appearance.Button;
            this.checkboxDefer.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.checkboxDefer.Location = new System.Drawing.Point(64, 208);
            this.checkboxDefer.Name = "checkboxDefer";
            this.checkboxDefer.Size = new System.Drawing.Size(87, 18);
            this.checkboxDefer.TabIndex = 24;
            this.checkboxDefer.Text = "Defer Settings";
            this.checkboxDefer.CheckedChanged += new System.EventHandler(this.checkboxDefer_CheckedChanged);
            // 
            // labelMaxdistance
            // 
            this.labelMaxdistance.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.labelMaxdistance.Location = new System.Drawing.Point(96, 168);
            this.labelMaxdistance.Name = "labelMaxdistance";
            this.labelMaxdistance.Size = new System.Drawing.Size(52, 21);
            this.labelMaxdistance.TabIndex = 15;
            this.labelMaxdistance.Text = "0";
            this.labelMaxdistance.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // labelStatic4
            // 
            this.labelStatic4.Location = new System.Drawing.Point(16, 168);
            this.labelStatic4.Name = "labelStatic4";
            this.labelStatic4.Size = new System.Drawing.Size(71, 21);
            this.labelStatic4.TabIndex = 14;
            this.labelStatic4.Text = "Max distance";
            this.labelStatic4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // labelRollofffactor
            // 
            this.labelRollofffactor.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.labelRollofffactor.Location = new System.Drawing.Point(96, 72);
            this.labelRollofffactor.Name = "labelRollofffactor";
            this.labelRollofffactor.Size = new System.Drawing.Size(52, 21);
            this.labelRollofffactor.TabIndex = 9;
            this.labelRollofffactor.Text = "0";
            this.labelRollofffactor.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // labelStatic2
            // 
            this.labelStatic2.Location = new System.Drawing.Point(16, 72);
            this.labelStatic2.Name = "labelStatic2";
            this.labelStatic2.Size = new System.Drawing.Size(71, 21);
            this.labelStatic2.TabIndex = 8;
            this.labelStatic2.Text = "Rolloff factor";
            this.labelStatic2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // labelStatic3
            // 
            this.labelStatic3.Location = new System.Drawing.Point(16, 120);
            this.labelStatic3.Name = "labelStatic3";
            this.labelStatic3.Size = new System.Drawing.Size(71, 21);
            this.labelStatic3.TabIndex = 11;
            this.labelStatic3.Text = "Min distance";
            this.labelStatic3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // labelMindistance
            // 
            this.labelMindistance.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.labelMindistance.Location = new System.Drawing.Point(96, 120);
            this.labelMindistance.Name = "labelMindistance";
            this.labelMindistance.Size = new System.Drawing.Size(52, 21);
            this.labelMindistance.TabIndex = 12;
            this.labelMindistance.Text = "0";
            this.labelMindistance.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // labelStatic1
            // 
            this.labelStatic1.Location = new System.Drawing.Point(25, 96);
            this.labelStatic1.Name = "labelStatic1";
            this.labelStatic1.Size = new System.Drawing.Size(75, 21);
            this.labelStatic1.TabIndex = 5;
            this.labelStatic1.Text = "Doppler factor";
            this.labelStatic1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // labelDopplerfactor
            // 
            this.labelDopplerfactor.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.labelDopplerfactor.Location = new System.Drawing.Point(108, 96);
            this.labelDopplerfactor.Name = "labelDopplerfactor";
            this.labelDopplerfactor.Size = new System.Drawing.Size(52, 21);
            this.labelDopplerfactor.TabIndex = 6;
            this.labelDopplerfactor.Text = "0";
            this.labelDopplerfactor.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // groupboxStatic1
            // 
            this.groupboxStatic1.Location = new System.Drawing.Point(318, 72);
            this.groupboxStatic1.Name = "groupboxStatic1";
            this.groupboxStatic1.Size = new System.Drawing.Size(162, 240);
            this.groupboxStatic1.TabIndex = 17;
            this.groupboxStatic1.TabStop = false;
            this.groupboxStatic1.Text = "Sound movement";
            // 
            // pictureboxRenderWindow
            // 
            this.pictureboxRenderWindow.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.pictureboxRenderWindow.Location = new System.Drawing.Point(352, 136);
            this.pictureboxRenderWindow.Name = "pictureboxRenderWindow";
            this.pictureboxRenderWindow.Size = new System.Drawing.Size(69, 70);
            this.pictureboxRenderWindow.TabIndex = 18;
            this.pictureboxRenderWindow.TabStop = false;
            this.pictureboxRenderWindow.Text = "130";
            // 
            // trackbarVerticalSlider
            // 
            this.trackbarVerticalSlider.AutoSize = false;
            this.trackbarVerticalSlider.Location = new System.Drawing.Point(432, 128);
            this.trackbarVerticalSlider.Name = "trackbarVerticalSlider";
            this.trackbarVerticalSlider.Orientation = System.Windows.Forms.Orientation.Vertical;
            this.trackbarVerticalSlider.Size = new System.Drawing.Size(45, 84);
            this.trackbarVerticalSlider.TabIndex = 19;
            this.trackbarVerticalSlider.Text = "Slider1";
            this.trackbarVerticalSlider.TickStyle = System.Windows.Forms.TickStyle.None;
            this.trackbarVerticalSlider.Scroll += new System.EventHandler(this.trackbarSlider_Scroll);
            // 
            // trackbarHorizontalSlider
            // 
            this.trackbarHorizontalSlider.AutoSize = false;
            this.trackbarHorizontalSlider.Location = new System.Drawing.Point(344, 208);
            this.trackbarHorizontalSlider.Name = "trackbarHorizontalSlider";
            this.trackbarHorizontalSlider.Size = new System.Drawing.Size(82, 45);
            this.trackbarHorizontalSlider.TabIndex = 20;
            this.trackbarHorizontalSlider.Text = "Slider1";
            this.trackbarHorizontalSlider.TickStyle = System.Windows.Forms.TickStyle.None;
            this.trackbarHorizontalSlider.Scroll += new System.EventHandler(this.trackbarSlider_Scroll);
            // 
            // buttonPlay
            // 
            this.buttonPlay.Enabled = false;
            this.buttonPlay.Location = new System.Drawing.Point(8, 328);
            this.buttonPlay.Name = "buttonPlay";
            this.buttonPlay.TabIndex = 21;
            this.buttonPlay.Text = "&Play";
            this.buttonPlay.Click += new System.EventHandler(this.buttonPlay_Click);
            // 
            // buttonStop
            // 
            this.buttonStop.Enabled = false;
            this.buttonStop.Location = new System.Drawing.Point(88, 328);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.TabIndex = 22;
            this.buttonStop.Text = "&Stop";
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(400, 328);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.TabIndex = 23;
            this.buttonCancel.Text = "E&xit";
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // timerMovement
            // 
            this.timerMovement.Enabled = true;
            this.timerMovement.SynchronizingObject = this;
            this.timerMovement.Elapsed += new System.Timers.ElapsedEventHandler(this.MovementTimer_Elapsed);
            // 
            // Play3DSound
            // 
            this.AcceptButton = this.buttonSoundfile;
            this.ClientSize = new System.Drawing.Size(496, 358);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                      this.buttonSoundfile,
                                                                      this.labelFilename,
                                                                      this.labelStatic,
                                                                      this.labelStatus,
                                                                      this.labelStatic1,
                                                                      this.labelDopplerfactor,
                                                                      this.pictureboxRenderWindow,
                                                                      this.trackbarVerticalSlider,
                                                                      this.trackbarHorizontalSlider,
                                                                      this.buttonPlay,
                                                                      this.buttonStop,
                                                                      this.buttonCancel,
                                                                      this.groupboxStatic,
                                                                      this.groupboxStatic1});
            this.Name = "Play3DSound";
            this.Text = "Play3DSound";
            this.groupboxStatic.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.trackbarMaxdistanceSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarDopplerSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarMindistanceSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarRolloffSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarVerticalSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackbarHorizontalSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.timerMovement)).EndInit();
            this.ResumeLayout(false);

        }
        #endregion
        int ConvertLogScaleToLinearSliderPos(float fValue)
        {
            //-----------------------------------------------------------------------------
            // Name: ConvertLinearSliderPosToLogScale()
            // Desc: Converts a quasi logrithmic scale to a slider position
            //-----------------------------------------------------------------------------
            if (fValue > 0.0f && fValue <= 0.1f)
            {
                return (int)(fValue / 0.01f);
            }
            else if (fValue > 0.1f && fValue <= 1.0f)
            {
                return (int)(fValue / 0.1f) + 10;
            }
            else if (fValue > 1.0f && fValue <= 10.0f)
            {
                return (int)(fValue / 1.0f) + 20;
            }
            else if (fValue > 10.0f && fValue <= 100.0f)
            {
                return (int)(fValue / 10.0f) + 30;
            }
            return 0;
        }
        float ConvertLinearSliderPosToLogScale(int iSliderPos)
        {
            if (iSliderPos > 0 && iSliderPos <= 10)
            {
                return iSliderPos * 0.01f;
            }
            else if (iSliderPos > 10 && iSliderPos <= 20)
            {
                return (iSliderPos - 10) * 0.1f;
            }
            else if (iSliderPos > 20 && iSliderPos <= 30)
            {
                return (iSliderPos - 20) * 1.0f;
            }
            else if (iSliderPos > 30 && iSliderPos <= 40)
            {
                return (iSliderPos - 30) * 10.0f;
            }
            return 0.0f;
        }
        void SetSlidersPos(float fDopplerValue, float fRolloffValue, float fMinDistValue, float fMaxDistValue)
        {
            //-----------------------------------------------------------------------------
            // Name: SetSlidersPos()
            // Desc: Sets the slider positions
            //-----------------------------------------------------------------------------
            int lDopplerSlider = ConvertLogScaleToLinearSliderPos(fDopplerValue);
            int lRolloffSlider = ConvertLogScaleToLinearSliderPos(fRolloffValue);
            int lMinDistSlider = ConvertLogScaleToLinearSliderPos(fMinDistValue);
            int lMaxDistSlider = ConvertLogScaleToLinearSliderPos(fMaxDistValue);

            trackbarDopplerSlider.Value = lDopplerSlider;
            trackbarRolloffSlider.Value = lRolloffSlider;
            trackbarMindistanceSlider.Value = lMinDistSlider;
            trackbarMaxdistanceSlider.Value = lMaxDistSlider;
        }

        private void buttonSoundfile_Click(object sender, System.EventArgs e)
        {
            // Stop the timer while dialogs are displayed
            timerMovement.Enabled = false;
            if (OpenSoundFile())
                timerMovement.Enabled = true;
        }
        bool OpenSoundFile()
        {
            //-----------------------------------------------------------------------------
            // Name: OnOpenSoundFile()
            // Desc: Called when the user requests to open a sound file
            //-----------------------------------------------------------------------------

            OpenFileDialog ofd = new OpenFileDialog();
            Guid guid3DAlgorithm = Guid.Empty;

            // Get the default media path (something like C:\WINDOWS\MEDIA)
            if (string.Empty == Path)
                Path = Environment.SystemDirectory.Substring(0, Environment.SystemDirectory.LastIndexOf("\\")) + "\\media";

            ofd.DefaultExt = ".wav";
            ofd.Filter = "Wave Files|*.wav|All Files|*.*";
            ofd.FileName = FileName;
            ofd.InitialDirectory = Path;

            if (null != applicationBuffer)
            {
                applicationBuffer.Stop();
                applicationBuffer.SetCurrentPosition(0);
            }

            // Update the UI controls to show the sound as loading a file
            buttonPlay.Enabled = buttonStop.Enabled = false;
            labelStatus.Text = "Loading file...";

            // Display the OpenFileName dialog. Then, try to load the specified file
            if (DialogResult.Cancel == ofd.ShowDialog(this))
            {
                labelStatus.Text = "Load aborted.";
                timerMovement.Enabled = true;
                return false;
            }
            FileName = ofd.FileName;
            Path = ofd.FileName.Substring(0, ofd.FileName.LastIndexOf("\\"));

            // Free any previous sound, and make a new one
            if (null != applicationBuffer)
            {
                applicationBuffer.Dispose();
                applicationBuffer = null;
            }

            // Get the software DirectSound3D emulation algorithm to use
            // Ask the user for this sample, so display the algorithm dialog box.       
            AlgorithmForm frm = new AlgorithmForm();
            if (DialogResult.Cancel == frm.ShowDialog(this))
            {
                // User canceled dialog box
                labelStatus.Text = "Load aborted.";
                labelFilename.Text = string.Empty;
                return false;
            }

            LoadSoundFile(ofd.FileName);
            if (null == applicationBuffer)
                return false;

            // Get the 3D buffer from the secondary buffer
            try
            {
                applicationBuffer3D = new Buffer3D(applicationBuffer);
            }
            catch (DirectXException)
            {
                labelStatus.Text = "Could not get 3D buffer.";
                labelFilename.Text = string.Empty;
                return false;
            }

            // Get the 3D buffer parameters
            application3DSettings = applicationBuffer3D.AllParameters;

            // Set new 3D buffer parameters
            application3DSettings.Mode = Mode3D.HeadRelative;
            applicationBuffer3D.AllParameters = application3DSettings;

            if (true == applicationBuffer.Caps.LocateInHardware)
                labelStatus.Text = "File loaded using hardware mixing.";
            else
                labelStatus.Text = "File loaded using software mixing.";

            // Update the UI controls to show the sound as the file is loaded
            labelFilename.Text = FileName;
            EnablePlayUI(true);

            // Remember the file for next time
            if (null != applicationBuffer3D)
            {
                FileName = ofd.FileName;
            }
            Path = FileName.Substring(0, FileName.LastIndexOf("\\"));

            // Set the slider positions
            SetSlidersPos(0.0f, 0.0f, maxOrbitRadius, maxOrbitRadius * 2.0f);
            SliderChanged();
            return true;
        }
        private void LoadSoundFile(string newFileName)
        {
            BufferDescription description = new BufferDescription();
            WaveFormat wf = new WaveFormat();

            buttonPlay.Enabled = false;
            buttonStop.Enabled = false;
            labelStatus.Text = "Loading file...";

            description.Guid3DAlgorithm = guid3DAlgorithm;
            description.Control3D = true;

            if (null != applicationBuffer)
            {
                applicationBuffer.Stop();
                applicationBuffer.SetCurrentPosition(0);
            }

            // Load the wave file into a DirectSound buffer
            try
            {
                applicationBuffer = new SecondaryBuffer(newFileName, description, applicationDevice);
                if (applicationBuffer.NotVirtualized)
                    MessageBox.Show(this, "The 3D virtualization algorithm requested is not supported under this " +
                        "operating system.  It is available only on Windows 2000, Windows ME, and Windows 98 with WDM " +
                        "drivers and beyond. This buffer was created without virtualization.", "DirectSound Sample", MessageBoxButtons.OK);
            }
            catch (ArgumentException)
            {
                // Check to see if it was a stereo buffer that threw the exception.
                labelStatus.Text = "Wave file must be mono for 3D control.";
                labelFilename.Text = string.Empty;
                return;
            }
            catch
            {
                // Unknown error, but not a critical failure, so just update the status
                labelStatus.Text = "Could not create sound buffer.";
                return;
            }

            if (WaveFormatTag.Pcm != (WaveFormatTag.Pcm & description.Format.FormatTag))
            {
                labelStatus.Text = "Wave file must be PCM for 3D control.";
                if (null != applicationBuffer)
                    applicationBuffer.Dispose();
                applicationBuffer = null;
            }

            // Remember the file for next time
            if (null != applicationBuffer)
                FileName = newFileName;

            labelStatus.Text = "Ready.";
            labelFilename.Text = FileName;
        }

        private bool RestoreBuffer()
        {
            if (false == applicationBuffer.Status.BufferLost)
                return false;

            while (true == applicationBuffer.Status.BufferLost)
            {
                applicationBuffer.Restore();
                Application.DoEvents();
            }
            return true;
        }

        private void PlaySound()
        {
            BufferPlayFlags flags = BufferPlayFlags.Looping;

            if (RestoreBuffer())
            {
                LoadSoundFile(FileName);
                applicationBuffer.SetCurrentPosition(0);
            }
            applicationBuffer.Play(0, flags);
        }

        private void buttonCancel_Click(object sender, System.EventArgs e)
        {
            this.Close();
        }

        private void buttonPlay_Click(object sender, System.EventArgs e)
        {
            EnablePlayUI(false);
            PlaySound();
        }
        private void EnablePlayUI(bool bEnable)
        {
            if (true == bEnable)
            {
                buttonPlay.Enabled = true;
                buttonStop.Enabled = false;
                buttonPlay.Focus();
            }
            else
            {
                buttonPlay.Enabled = false;
                buttonStop.Enabled = true;
                buttonStop.Focus();
            }
        }
        void SliderChanged()
        {
            //-----------------------------------------------------------------------------
            // Name: SliderChanged()  
            // Desc: Called when the dialog's slider bars are changed by the user, or need
            //       updating
            //-----------------------------------------------------------------------------

            // Get the position of the sliders
            float DopplerFactor = ConvertLinearSliderPosToLogScale(trackbarDopplerSlider.Value);
            float RolloffFactor = ConvertLinearSliderPosToLogScale(trackbarRolloffSlider.Value);
            float MinDistance = ConvertLinearSliderPosToLogScale(trackbarMindistanceSlider.Value);
            float MaxDistance = ConvertLinearSliderPosToLogScale(trackbarMaxdistanceSlider.Value);

            // Set the static text boxes
            labelDopplerfactor.Text = DopplerFactor.ToString("0.00");
            labelRollofffactor.Text = RolloffFactor.ToString("0.00");
            labelMindistance.Text = MinDistance.ToString("0.00");
            labelMaxdistance.Text = MaxDistance.ToString("0.00");

            // Set the options in the DirectSound buffer
            Set3DParameters(DopplerFactor, RolloffFactor, MinDistance, MaxDistance);

            buttonApply.Enabled = (true == checkboxDefer.Checked) ? true : false;
        }
        void Set3DParameters(float DopplerFactor, float RolloffFactor, float MinDistance, float MaxDistance)
        {
            //-----------------------------------------------------------------------------
            // Name: Set3DParameters()
            // Desc: Set the 3D buffer parameters
            //-----------------------------------------------------------------------------

            // Every change to 3-D sound buffer and listener settings causes 
            // DirectSound to remix, at the expense of CPU cycles. 
            // To minimize the performance impact of changing 3-D settings, 
            // use the Deferred property on Listener3D or Buffer3D objects 
            // Then call the Listener::CommitDeferredSettings method to execute 
            // all of the deferred commands at once.

            listenerParameters.DopplerFactor = DopplerFactor;
            listenerParameters.RolloffFactor = RolloffFactor;

            if (null != applicationListener)
            {
                applicationListener.Deferred = checkboxDefer.Checked;
                applicationListener.AllParameters = listenerParameters;
            }

            application3DSettings.MinDistance = MinDistance;
            application3DSettings.MaxDistance = MaxDistance;

            if (null != applicationBuffer3D)
            {
                applicationBuffer3D.Deferred = checkboxDefer.Checked;
                applicationBuffer3D.AllParameters = application3DSettings;
            }
        }
        void Stop()
        {
            if (null != applicationBuffer3D)
            {
                applicationBuffer.Stop();
                applicationBuffer.SetCurrentPosition(0);
            }

            // Update the UI controls to show the sound as stopped
            EnablePlayUI(true);
            labelStatus.Text = "Sound stopped.";
        }
        private void buttonStop_Click(object sender, System.EventArgs e)
        {
            EnablePlayUI(true);
            Stop();
        }
        private void checkboxDefer_CheckedChanged(object sender, System.EventArgs e)
        {
            SliderChanged();
        }
        private void Apply()
        {
            // Call the DirectSound.3DListener.CommitDeferredSettings 
            // method to execute all of the deferred commands at once.
            // This is many times more efficent than recomputing everything
            // for every call.
            if (null != applicationListener)
                applicationListener.CommitDeferredSettings();
        }
        private void buttonApply_Click(object sender, System.EventArgs e)
        {
            Apply();
        }
        private void MovementTimer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            //-----------------------------------------------------------------------------
            // Name: MovementTimer_Elapsed()
            // Desc: Periodically updates the position of the object 
            //-----------------------------------------------------------------------------

            if (!this.IsHandleCreated)
                return; 
            float XScale;
            float YScale;

            XScale = trackbarHorizontalSlider.Value / 110.0f;
            YScale = trackbarVerticalSlider.Value / 110.0f;
            float t = Environment.TickCount / 1000.0f;

            // Move the sound object around the listener. The maximum radius of the
            // orbit is 27.5 units.
            Vector3 vPosition;
            vPosition.X = maxOrbitRadius * XScale * (float)Math.Sin(t);
            vPosition.Y = 0.0f;
            vPosition.Z = maxOrbitRadius * YScale * (float)Math.Cos(t);

            Vector3 vVelocity;
            vVelocity.X = maxOrbitRadius * XScale * (float)Math.Sin(t + 0.05f);
            vVelocity.Y = 0.0f;
            vVelocity.Z = maxOrbitRadius * YScale * (float)Math.Cos(t + 0.05f);

            // Show the object's position on the dialog's grid control
            this.BeginInvoke(new UpdateGridDelegate(this.UpdateGrid), new object[] { vPosition.X, vPosition.Z });

            // Set the sound buffer velocity and position
            SetObjectProperties(vPosition, vVelocity);
        }

        void UpdateGrid(float x, float y)
        {
            //-----------------------------------------------------------------------------
            // Name: UpdateGrid()
            // Desc: Draws a red dot in the dialog's grid bitmap at the x,y coordinate.
            //-----------------------------------------------------------------------------         
            if (this.IsDisposed)
                return; 

            if (null == bitmapGrid)
            {
                bitmapGrid = new Bitmap(pictureboxRenderWindow.BackgroundImage);
                applicationGraphics = pictureboxRenderWindow.CreateGraphics();
            }

            // Convert the world space x,y coordinates to pixel coordinates
            X = (int)((x / maxOrbitRadius + 1) * (pictureboxRenderWindow.ClientRectangle.Left + pictureboxRenderWindow.ClientRectangle.Right) / 2);
            Y = (int)((-y / maxOrbitRadius + 1) * (pictureboxRenderWindow.ClientRectangle.Top + pictureboxRenderWindow.ClientRectangle.Bottom) / 2);

            // Draw a crosshair object in red pixels
            Rectangle r = new Rectangle(X, Y, 3, 3);

            applicationGraphics.DrawImage(bitmapGrid, 1, 1);
            applicationGraphics.DrawEllipse(new Pen(Color.Red), r);
        }
        void SetObjectProperties(Vector3 position, Vector3 velocity)
        {
            //-----------------------------------------------------------------------------
            // Name: SetObjectProperties()
            // Desc: Sets the position and velocity on the 3D buffer
            //-----------------------------------------------------------------------------

            // Every change to 3-D sound buffer and listener settings causes 
            // DirectSound to remix, at the expense of CPU cycles. 
            // To minimize the performance impact of changing 3-D settings, 
            // use the Deferred flag of the DirectSound.Listener3D or DirectSound.Buffer3D methods that 
            // change 3-D settings. Then call the DirectSound.Listener3D.CommitSettings 
            // method to execute all of the deferred commands at once.
            application3DSettings.Position = position;
            application3DSettings.Velocity = velocity;

            if (null != applicationBuffer3D)
            {
                applicationBuffer3D.AllParameters = application3DSettings;
            }
        }
        private void trackbarSlider_Scroll(object sender, System.EventArgs e)
        {
            SliderChanged();
        }
    }
}