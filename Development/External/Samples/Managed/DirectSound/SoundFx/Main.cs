//----------------------------------------------------------------------------
// File: Main.cs
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using Microsoft.DirectX;
using Microsoft.DirectX.DirectSound;
using Buffer = Microsoft.DirectX.DirectSound.Buffer;

public class MainForm : Form
{
    #region Control declarations
    private System.Windows.Forms.ListBox listboxEffects;
    private System.Windows.Forms.RadioButton radiobuttonRadioSine;
    private Button buttonOk;
    private GroupBox groupboxFrame;
    private Label labelParamName1;
    private Label labelParamValue1;
    private TrackBar trackbarSlider1;
    private Label labelParamMin1;
    private Label labelParamMax1;
    private Label labelParamName2;
    private Label labelParamValue2;
    private TrackBar trackbarSlider2;
    private Label labelParamMin2;
    private Label labelParamMax2;
    private Label labelParamName3;
    private Label labelParamValue3;
    private TrackBar trackbarSlider3;
    private Label labelParamMin3;
    private Label labelParamMax3;
    private Label labelParamName4;
    private Label labelParamValue4;
    private TrackBar trackbarSlider4;
    private Label labelParamMin4;
    private Label labelParamMax4;
    private Label labelParamName5;
    private Label labelParamValue5;
    private TrackBar trackbarSlider5;
    private Label labelParamMin5;
    private Label labelParamMax5;
    private Label labelParamName6;
    private Label labelParamValue6;
    private TrackBar trackbarSlider6;
    private Label labelParamMin6;
    private Label labelParamMax6;
    private RadioButton radiobuttonTriangle;
    private RadioButton radiobuttonSquare;
    private GroupBox groupboxFrameWaveform;
    private Button buttonOpen;
    private Label labelTextFilename;
    private Label labelStatic2;
    private Label labelTextStatus;
    private CheckBox checkboxLoop;
    private Button buttonPlay;
    private Button buttonStop;
    private Label labelStatic3;
    private GroupBox groupboxFramePhase;
    private Label labelStatic4;
    private RadioButton radiobuttonRadioNeg180;
    private RadioButton radiobuttonRadioNeg90;
    private RadioButton radiobuttonRadioZero;
    private RadioButton radiobuttonRadio90;
    private RadioButton radiobuttonRadio180;
    private System.Windows.Forms.GroupBox groupboxEffects;
    #endregion

    private struct EffectInfo
    {
        public EffectDescription description;
        public object EffectSettings;
        public object Effect;
    };
    ArrayList effectDescription = new ArrayList();
    SecondaryBuffer applicationBuffer = null;
    Device applicationDevice = null;
    string fileName = string.Empty;
    string path = string.Empty;
    bool shouldLoop = false;
    private System.Windows.Forms.Button buttonDelete;
    private System.Windows.Forms.Timer timer1;
    private System.ComponentModel.IContainer components;
    private System.Windows.Forms.ComboBox comboEffects;
    private int currentIndex = 0;
    private bool isIgnoringSettings = false;

    [STAThread]
    public static void Main()
    {
        try
        {
            using (MainForm f = new MainForm())
            {
                Application.Run(f);
            }
        }
        catch{}
    }    
    public MainForm()
    {
        InitializeComponent();
        InitDirectSound();
        ClearUI(true);
    }
    #region InitializeComponent code
    private void InitializeComponent()
    {
        this.components = new System.ComponentModel.Container();
        this.buttonOk = new System.Windows.Forms.Button();
        this.groupboxFrame = new System.Windows.Forms.GroupBox();
        this.labelParamName1 = new System.Windows.Forms.Label();
        this.labelParamValue1 = new System.Windows.Forms.Label();
        this.trackbarSlider1 = new System.Windows.Forms.TrackBar();
        this.labelParamMin1 = new System.Windows.Forms.Label();
        this.labelParamMax1 = new System.Windows.Forms.Label();
        this.labelParamName2 = new System.Windows.Forms.Label();
        this.labelParamValue2 = new System.Windows.Forms.Label();
        this.trackbarSlider2 = new System.Windows.Forms.TrackBar();
        this.labelParamMin2 = new System.Windows.Forms.Label();
        this.labelParamMax2 = new System.Windows.Forms.Label();
        this.labelParamName3 = new System.Windows.Forms.Label();
        this.labelParamValue3 = new System.Windows.Forms.Label();
        this.trackbarSlider3 = new System.Windows.Forms.TrackBar();
        this.labelParamMin3 = new System.Windows.Forms.Label();
        this.labelParamMax3 = new System.Windows.Forms.Label();
        this.labelParamName4 = new System.Windows.Forms.Label();
        this.labelParamValue4 = new System.Windows.Forms.Label();
        this.trackbarSlider4 = new System.Windows.Forms.TrackBar();
        this.labelParamMin4 = new System.Windows.Forms.Label();
        this.labelParamMax4 = new System.Windows.Forms.Label();
        this.labelParamName5 = new System.Windows.Forms.Label();
        this.labelParamValue5 = new System.Windows.Forms.Label();
        this.trackbarSlider5 = new System.Windows.Forms.TrackBar();
        this.labelParamMin5 = new System.Windows.Forms.Label();
        this.labelParamMax5 = new System.Windows.Forms.Label();
        this.labelParamName6 = new System.Windows.Forms.Label();
        this.labelParamValue6 = new System.Windows.Forms.Label();
        this.trackbarSlider6 = new System.Windows.Forms.TrackBar();
        this.labelParamMin6 = new System.Windows.Forms.Label();
        this.labelParamMax6 = new System.Windows.Forms.Label();
        this.radiobuttonTriangle = new System.Windows.Forms.RadioButton();
        this.radiobuttonSquare = new System.Windows.Forms.RadioButton();
        this.radiobuttonRadioSine = new System.Windows.Forms.RadioButton();
        this.groupboxFrameWaveform = new System.Windows.Forms.GroupBox();
        this.buttonOpen = new System.Windows.Forms.Button();
        this.labelTextFilename = new System.Windows.Forms.Label();
        this.labelStatic2 = new System.Windows.Forms.Label();
        this.labelTextStatus = new System.Windows.Forms.Label();
        this.checkboxLoop = new System.Windows.Forms.CheckBox();
        this.buttonPlay = new System.Windows.Forms.Button();
        this.buttonStop = new System.Windows.Forms.Button();
        this.labelStatic3 = new System.Windows.Forms.Label();
        this.labelStatic4 = new System.Windows.Forms.Label();
        this.radiobuttonRadioNeg180 = new System.Windows.Forms.RadioButton();
        this.radiobuttonRadioNeg90 = new System.Windows.Forms.RadioButton();
        this.radiobuttonRadioZero = new System.Windows.Forms.RadioButton();
        this.radiobuttonRadio90 = new System.Windows.Forms.RadioButton();
        this.radiobuttonRadio180 = new System.Windows.Forms.RadioButton();
        this.groupboxFramePhase = new System.Windows.Forms.GroupBox();
        this.groupboxEffects = new System.Windows.Forms.GroupBox();
        this.buttonDelete = new System.Windows.Forms.Button();
        this.listboxEffects = new System.Windows.Forms.ListBox();
        this.comboEffects = new System.Windows.Forms.ComboBox();
        this.timer1 = new System.Windows.Forms.Timer(this.components);
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider1)).BeginInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider2)).BeginInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider3)).BeginInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider4)).BeginInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider5)).BeginInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider6)).BeginInit();
        this.groupboxFrameWaveform.SuspendLayout();
        this.groupboxFramePhase.SuspendLayout();
        this.groupboxEffects.SuspendLayout();
        this.SuspendLayout();
        // 
        // buttonOk
        // 
        this.buttonOk.Location = new System.Drawing.Point(87, 432);
        this.buttonOk.Name = "buttonOk";
        this.buttonOk.Size = new System.Drawing.Size(67, 23);
        this.buttonOk.TabIndex = 0;
        this.buttonOk.Text = "E&xit";
        this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
        // 
        // groupboxFrame
        // 
        this.groupboxFrame.Location = new System.Drawing.Point(165, 76);
        this.groupboxFrame.Name = "groupboxFrame";
        this.groupboxFrame.Size = new System.Drawing.Size(525, 380);
        this.groupboxFrame.TabIndex = 1;
        this.groupboxFrame.TabStop = false;
        this.groupboxFrame.Text = "Parameters";
        // 
        // labelParamName1
        // 
        this.labelParamName1.Location = new System.Drawing.Point(169, 112);
        this.labelParamName1.Name = "labelParamName1";
        this.labelParamName1.Size = new System.Drawing.Size(117, 13);
        this.labelParamName1.TabIndex = 2;
        this.labelParamName1.TextAlign = System.Drawing.ContentAlignment.TopRight;
        // 
        // labelParamValue1
        // 
        this.labelParamValue1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelParamValue1.Location = new System.Drawing.Point(294, 112);
        this.labelParamValue1.Name = "labelParamValue1";
        this.labelParamValue1.Size = new System.Drawing.Size(67, 16);
        this.labelParamValue1.TabIndex = 3;
        this.labelParamValue1.Text = "Value";
        this.labelParamValue1.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // trackbarSlider1
        // 
        this.trackbarSlider1.Location = new System.Drawing.Point(426, 112);
        this.trackbarSlider1.Name = "trackbarSlider1";
        this.trackbarSlider1.Size = new System.Drawing.Size(195, 45);
        this.trackbarSlider1.TabIndex = 4;
        this.trackbarSlider1.Text = "Slider1";
        this.trackbarSlider1.TickStyle = System.Windows.Forms.TickStyle.None;
        this.trackbarSlider1.Scroll += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // labelParamMin1
        // 
        this.labelParamMin1.Location = new System.Drawing.Point(366, 112);
        this.labelParamMin1.Name = "labelParamMin1";
        this.labelParamMin1.Size = new System.Drawing.Size(60, 16);
        this.labelParamMin1.TabIndex = 5;
        this.labelParamMin1.Text = "min";
        this.labelParamMin1.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamMax1
        // 
        this.labelParamMax1.Location = new System.Drawing.Point(627, 120);
        this.labelParamMax1.Name = "labelParamMax1";
        this.labelParamMax1.Size = new System.Drawing.Size(52, 13);
        this.labelParamMax1.TabIndex = 6;
        this.labelParamMax1.Text = "max";
        this.labelParamMax1.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamName2
        // 
        this.labelParamName2.Location = new System.Drawing.Point(169, 160);
        this.labelParamName2.Name = "labelParamName2";
        this.labelParamName2.Size = new System.Drawing.Size(117, 13);
        this.labelParamName2.TabIndex = 7;
        this.labelParamName2.TextAlign = System.Drawing.ContentAlignment.TopRight;
        // 
        // labelParamValue2
        // 
        this.labelParamValue2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelParamValue2.Location = new System.Drawing.Point(294, 160);
        this.labelParamValue2.Name = "labelParamValue2";
        this.labelParamValue2.Size = new System.Drawing.Size(67, 16);
        this.labelParamValue2.TabIndex = 8;
        this.labelParamValue2.Text = "Value";
        this.labelParamValue2.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // trackbarSlider2
        // 
        this.trackbarSlider2.Location = new System.Drawing.Point(426, 163);
        this.trackbarSlider2.Name = "trackbarSlider2";
        this.trackbarSlider2.Size = new System.Drawing.Size(195, 45);
        this.trackbarSlider2.TabIndex = 9;
        this.trackbarSlider2.Text = "Slider1";
        this.trackbarSlider2.TickStyle = System.Windows.Forms.TickStyle.None;
        this.trackbarSlider2.Scroll += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // labelParamMin2
        // 
        this.labelParamMin2.Location = new System.Drawing.Point(366, 160);
        this.labelParamMin2.Name = "labelParamMin2";
        this.labelParamMin2.Size = new System.Drawing.Size(60, 16);
        this.labelParamMin2.TabIndex = 10;
        this.labelParamMin2.Text = "min";
        this.labelParamMin2.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamMax2
        // 
        this.labelParamMax2.Location = new System.Drawing.Point(627, 168);
        this.labelParamMax2.Name = "labelParamMax2";
        this.labelParamMax2.Size = new System.Drawing.Size(52, 13);
        this.labelParamMax2.TabIndex = 11;
        this.labelParamMax2.Text = "max";
        this.labelParamMax2.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamName3
        // 
        this.labelParamName3.Location = new System.Drawing.Point(169, 208);
        this.labelParamName3.Name = "labelParamName3";
        this.labelParamName3.Size = new System.Drawing.Size(117, 13);
        this.labelParamName3.TabIndex = 12;
        this.labelParamName3.TextAlign = System.Drawing.ContentAlignment.TopRight;
        // 
        // labelParamValue3
        // 
        this.labelParamValue3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelParamValue3.Location = new System.Drawing.Point(294, 208);
        this.labelParamValue3.Name = "labelParamValue3";
        this.labelParamValue3.Size = new System.Drawing.Size(67, 16);
        this.labelParamValue3.TabIndex = 13;
        this.labelParamValue3.Text = "Value";
        this.labelParamValue3.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // trackbarSlider3
        // 
        this.trackbarSlider3.Location = new System.Drawing.Point(426, 208);
        this.trackbarSlider3.Name = "trackbarSlider3";
        this.trackbarSlider3.Size = new System.Drawing.Size(195, 45);
        this.trackbarSlider3.TabIndex = 14;
        this.trackbarSlider3.Text = "Slider1";
        this.trackbarSlider3.TickStyle = System.Windows.Forms.TickStyle.None;
        this.trackbarSlider3.Scroll += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // labelParamMin3
        // 
        this.labelParamMin3.Location = new System.Drawing.Point(366, 208);
        this.labelParamMin3.Name = "labelParamMin3";
        this.labelParamMin3.Size = new System.Drawing.Size(60, 16);
        this.labelParamMin3.TabIndex = 15;
        this.labelParamMin3.Text = "min";
        this.labelParamMin3.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamMax3
        // 
        this.labelParamMax3.Location = new System.Drawing.Point(627, 216);
        this.labelParamMax3.Name = "labelParamMax3";
        this.labelParamMax3.Size = new System.Drawing.Size(52, 13);
        this.labelParamMax3.TabIndex = 16;
        this.labelParamMax3.Text = "max";
        this.labelParamMax3.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamName4
        // 
        this.labelParamName4.Location = new System.Drawing.Point(169, 256);
        this.labelParamName4.Name = "labelParamName4";
        this.labelParamName4.Size = new System.Drawing.Size(117, 13);
        this.labelParamName4.TabIndex = 17;
        this.labelParamName4.TextAlign = System.Drawing.ContentAlignment.TopRight;
        // 
        // labelParamValue4
        // 
        this.labelParamValue4.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelParamValue4.Location = new System.Drawing.Point(294, 256);
        this.labelParamValue4.Name = "labelParamValue4";
        this.labelParamValue4.Size = new System.Drawing.Size(67, 16);
        this.labelParamValue4.TabIndex = 18;
        this.labelParamValue4.Text = "Value";
        this.labelParamValue4.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // trackbarSlider4
        // 
        this.trackbarSlider4.Location = new System.Drawing.Point(426, 256);
        this.trackbarSlider4.Name = "trackbarSlider4";
        this.trackbarSlider4.Size = new System.Drawing.Size(195, 45);
        this.trackbarSlider4.TabIndex = 19;
        this.trackbarSlider4.Text = "Slider1";
        this.trackbarSlider4.TickStyle = System.Windows.Forms.TickStyle.None;
        this.trackbarSlider4.Scroll += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // labelParamMin4
        // 
        this.labelParamMin4.Location = new System.Drawing.Point(366, 256);
        this.labelParamMin4.Name = "labelParamMin4";
        this.labelParamMin4.Size = new System.Drawing.Size(60, 16);
        this.labelParamMin4.TabIndex = 20;
        this.labelParamMin4.Text = "min";
        this.labelParamMin4.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamMax4
        // 
        this.labelParamMax4.Location = new System.Drawing.Point(627, 256);
        this.labelParamMax4.Name = "labelParamMax4";
        this.labelParamMax4.Size = new System.Drawing.Size(52, 13);
        this.labelParamMax4.TabIndex = 21;
        this.labelParamMax4.Text = "max";
        this.labelParamMax4.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamName5
        // 
        this.labelParamName5.Location = new System.Drawing.Point(169, 304);
        this.labelParamName5.Name = "labelParamName5";
        this.labelParamName5.Size = new System.Drawing.Size(117, 13);
        this.labelParamName5.TabIndex = 22;
        this.labelParamName5.TextAlign = System.Drawing.ContentAlignment.TopRight;
        // 
        // labelParamValue5
        // 
        this.labelParamValue5.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelParamValue5.Location = new System.Drawing.Point(294, 304);
        this.labelParamValue5.Name = "labelParamValue5";
        this.labelParamValue5.Size = new System.Drawing.Size(67, 16);
        this.labelParamValue5.TabIndex = 23;
        this.labelParamValue5.Text = "Value";
        this.labelParamValue5.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // trackbarSlider5
        // 
        this.trackbarSlider5.Location = new System.Drawing.Point(426, 304);
        this.trackbarSlider5.Name = "trackbarSlider5";
        this.trackbarSlider5.Size = new System.Drawing.Size(195, 45);
        this.trackbarSlider5.TabIndex = 24;
        this.trackbarSlider5.Text = "Slider1";
        this.trackbarSlider5.TickStyle = System.Windows.Forms.TickStyle.None;
        this.trackbarSlider5.Scroll += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // labelParamMin5
        // 
        this.labelParamMin5.Location = new System.Drawing.Point(366, 304);
        this.labelParamMin5.Name = "labelParamMin5";
        this.labelParamMin5.Size = new System.Drawing.Size(60, 16);
        this.labelParamMin5.TabIndex = 25;
        this.labelParamMin5.Text = "min";
        this.labelParamMin5.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamMax5
        // 
        this.labelParamMax5.Location = new System.Drawing.Point(627, 312);
        this.labelParamMax5.Name = "labelParamMax5";
        this.labelParamMax5.Size = new System.Drawing.Size(52, 13);
        this.labelParamMax5.TabIndex = 26;
        this.labelParamMax5.Text = "max";
        this.labelParamMax5.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamName6
        // 
        this.labelParamName6.Location = new System.Drawing.Point(169, 352);
        this.labelParamName6.Name = "labelParamName6";
        this.labelParamName6.Size = new System.Drawing.Size(117, 13);
        this.labelParamName6.TabIndex = 27;
        this.labelParamName6.TextAlign = System.Drawing.ContentAlignment.TopRight;
        // 
        // labelParamValue6
        // 
        this.labelParamValue6.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelParamValue6.Location = new System.Drawing.Point(294, 352);
        this.labelParamValue6.Name = "labelParamValue6";
        this.labelParamValue6.Size = new System.Drawing.Size(67, 16);
        this.labelParamValue6.TabIndex = 28;
        this.labelParamValue6.Text = "Value";
        this.labelParamValue6.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // trackbarSlider6
        // 
        this.trackbarSlider6.Location = new System.Drawing.Point(426, 352);
        this.trackbarSlider6.Name = "trackbarSlider6";
        this.trackbarSlider6.Size = new System.Drawing.Size(195, 45);
        this.trackbarSlider6.TabIndex = 29;
        this.trackbarSlider6.Text = "Slider1";
        this.trackbarSlider6.TickStyle = System.Windows.Forms.TickStyle.None;
        this.trackbarSlider6.Scroll += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // labelParamMin6
        // 
        this.labelParamMin6.Location = new System.Drawing.Point(366, 352);
        this.labelParamMin6.Name = "labelParamMin6";
        this.labelParamMin6.Size = new System.Drawing.Size(60, 16);
        this.labelParamMin6.TabIndex = 30;
        this.labelParamMin6.Text = "min";
        this.labelParamMin6.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelParamMax6
        // 
        this.labelParamMax6.Location = new System.Drawing.Point(627, 352);
        this.labelParamMax6.Name = "labelParamMax6";
        this.labelParamMax6.Size = new System.Drawing.Size(52, 13);
        this.labelParamMax6.TabIndex = 31;
        this.labelParamMax6.Text = "max";
        this.labelParamMax6.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // radiobuttonTriangle
        // 
        this.radiobuttonTriangle.Location = new System.Drawing.Point(16, 16);
        this.radiobuttonTriangle.Name = "radiobuttonTriangle";
        this.radiobuttonTriangle.Size = new System.Drawing.Size(69, 16);
        this.radiobuttonTriangle.TabIndex = 32;
        this.radiobuttonTriangle.Text = "Triangle";
        this.radiobuttonTriangle.CheckedChanged += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // radiobuttonSquare
        // 
        this.radiobuttonSquare.Location = new System.Drawing.Point(88, 16);
        this.radiobuttonSquare.Name = "radiobuttonSquare";
        this.radiobuttonSquare.Size = new System.Drawing.Size(64, 16);
        this.radiobuttonSquare.TabIndex = 33;
        this.radiobuttonSquare.Text = "Square";
        this.radiobuttonSquare.CheckedChanged += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // radiobuttonRadioSine
        // 
        this.radiobuttonRadioSine.Location = new System.Drawing.Point(152, 16);
        this.radiobuttonRadioSine.Name = "radiobuttonRadioSine";
        this.radiobuttonRadioSine.Size = new System.Drawing.Size(48, 16);
        this.radiobuttonRadioSine.TabIndex = 34;
        this.radiobuttonRadioSine.Text = "Sine";
        this.radiobuttonRadioSine.CheckedChanged += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // groupboxFrameWaveform
        // 
        this.groupboxFrameWaveform.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                            this.radiobuttonSquare,
                                                                                            this.radiobuttonTriangle,
                                                                                            this.radiobuttonRadioSine});
        this.groupboxFrameWaveform.Location = new System.Drawing.Point(180, 400);
        this.groupboxFrameWaveform.Name = "groupboxFrameWaveform";
        this.groupboxFrameWaveform.Size = new System.Drawing.Size(225, 42);
        this.groupboxFrameWaveform.TabIndex = 35;
        this.groupboxFrameWaveform.TabStop = false;
        this.groupboxFrameWaveform.Text = "Waveform";
        // 
        // buttonOpen
        // 
        this.buttonOpen.Location = new System.Drawing.Point(12, 12);
        this.buttonOpen.Name = "buttonOpen";
        this.buttonOpen.TabIndex = 47;
        this.buttonOpen.Text = "&Open File";
        this.buttonOpen.Click += new System.EventHandler(this.buttonOpen_Click);
        // 
        // labelTextFilename
        // 
        this.labelTextFilename.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelTextFilename.Location = new System.Drawing.Point(94, 14);
        this.labelTextFilename.Name = "labelTextFilename";
        this.labelTextFilename.Size = new System.Drawing.Size(595, 20);
        this.labelTextFilename.TabIndex = 48;
        this.labelTextFilename.Text = "Filename";
        this.labelTextFilename.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
        // 
        // labelStatic2
        // 
        this.labelStatic2.Location = new System.Drawing.Point(19, 44);
        this.labelStatic2.Name = "labelStatic2";
        this.labelStatic2.Size = new System.Drawing.Size(67, 16);
        this.labelStatic2.TabIndex = 49;
        this.labelStatic2.Text = "Status";
        // 
        // labelTextStatus
        // 
        this.labelTextStatus.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelTextStatus.Location = new System.Drawing.Point(94, 44);
        this.labelTextStatus.Name = "labelTextStatus";
        this.labelTextStatus.Size = new System.Drawing.Size(595, 20);
        this.labelTextStatus.TabIndex = 50;
        this.labelTextStatus.Text = "No file loaded.";
        this.labelTextStatus.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
        // 
        // checkboxLoop
        // 
        this.checkboxLoop.Location = new System.Drawing.Point(42, 376);
        this.checkboxLoop.Name = "checkboxLoop";
        this.checkboxLoop.Size = new System.Drawing.Size(86, 16);
        this.checkboxLoop.TabIndex = 51;
        this.checkboxLoop.Text = "&Loop Sound";
        this.checkboxLoop.CheckedChanged += new System.EventHandler(this.checkboxLoop_CheckedChanged);
        // 
        // buttonPlay
        // 
        this.buttonPlay.Location = new System.Drawing.Point(10, 400);
        this.buttonPlay.Name = "buttonPlay";
        this.buttonPlay.Size = new System.Drawing.Size(67, 23);
        this.buttonPlay.TabIndex = 52;
        this.buttonPlay.Text = "&Play";
        this.buttonPlay.Click += new System.EventHandler(this.buttonPlay_Click);
        // 
        // buttonStop
        // 
        this.buttonStop.Location = new System.Drawing.Point(87, 400);
        this.buttonStop.Name = "buttonStop";
        this.buttonStop.Size = new System.Drawing.Size(67, 23);
        this.buttonStop.TabIndex = 53;
        this.buttonStop.Text = "&Stop";
        this.buttonStop.Enabled = false;
        this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
        // 
        // labelStatic3
        // 
        this.labelStatic3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelStatic3.Location = new System.Drawing.Point(372, 88);
        this.labelStatic3.Name = "labelStatic3";
        this.labelStatic3.Size = new System.Drawing.Size(52, 16);
        this.labelStatic3.TabIndex = 62;
        this.labelStatic3.Text = "Min";
        this.labelStatic3.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // labelStatic4
        // 
        this.labelStatic4.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.labelStatic4.Location = new System.Drawing.Point(627, 88);
        this.labelStatic4.Name = "labelStatic4";
        this.labelStatic4.Size = new System.Drawing.Size(52, 16);
        this.labelStatic4.TabIndex = 64;
        this.labelStatic4.Text = "Max";
        this.labelStatic4.TextAlign = System.Drawing.ContentAlignment.TopCenter;
        // 
        // radiobuttonRadioNeg180
        // 
        this.radiobuttonRadioNeg180.Location = new System.Drawing.Point(16, 16);
        this.radiobuttonRadioNeg180.Name = "radiobuttonRadioNeg180";
        this.radiobuttonRadioNeg180.Size = new System.Drawing.Size(45, 16);
        this.radiobuttonRadioNeg180.TabIndex = 65;
        this.radiobuttonRadioNeg180.Text = "-180";
        this.radiobuttonRadioNeg180.CheckedChanged += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // radiobuttonRadioNeg90
        // 
        this.radiobuttonRadioNeg90.Location = new System.Drawing.Point(72, 16);
        this.radiobuttonRadioNeg90.Name = "radiobuttonRadioNeg90";
        this.radiobuttonRadioNeg90.Size = new System.Drawing.Size(39, 16);
        this.radiobuttonRadioNeg90.TabIndex = 66;
        this.radiobuttonRadioNeg90.Text = "-90";
        this.radiobuttonRadioNeg90.CheckedChanged += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // radiobuttonRadioZero
        // 
        this.radiobuttonRadioZero.Location = new System.Drawing.Point(120, 16);
        this.radiobuttonRadioZero.Name = "radiobuttonRadioZero";
        this.radiobuttonRadioZero.Size = new System.Drawing.Size(30, 16);
        this.radiobuttonRadioZero.TabIndex = 67;
        this.radiobuttonRadioZero.Text = "0";
        this.radiobuttonRadioZero.CheckedChanged += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // radiobuttonRadio90
        // 
        this.radiobuttonRadio90.Location = new System.Drawing.Point(152, 16);
        this.radiobuttonRadio90.Name = "radiobuttonRadio90";
        this.radiobuttonRadio90.Size = new System.Drawing.Size(36, 16);
        this.radiobuttonRadio90.TabIndex = 68;
        this.radiobuttonRadio90.Text = "90";
        this.radiobuttonRadio90.CheckedChanged += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // radiobuttonRadio180
        // 
        this.radiobuttonRadio180.Location = new System.Drawing.Point(200, 16);
        this.radiobuttonRadio180.Name = "radiobuttonRadio180";
        this.radiobuttonRadio180.Size = new System.Drawing.Size(42, 16);
        this.radiobuttonRadio180.TabIndex = 69;
        this.radiobuttonRadio180.Text = "180";
        this.radiobuttonRadio180.CheckedChanged += new System.EventHandler(this.trackbarSliderScroll);
        // 
        // groupboxFramePhase
        // 
        this.groupboxFramePhase.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                         this.radiobuttonRadioNeg180,
                                                                                         this.radiobuttonRadioNeg90,
                                                                                         this.radiobuttonRadioZero,
                                                                                         this.radiobuttonRadio90,
                                                                                         this.radiobuttonRadio180});
        this.groupboxFramePhase.Location = new System.Drawing.Point(420, 400);
        this.groupboxFramePhase.Name = "groupboxFramePhase";
        this.groupboxFramePhase.Size = new System.Drawing.Size(247, 42);
        this.groupboxFramePhase.TabIndex = 63;
        this.groupboxFramePhase.TabStop = false;
        this.groupboxFramePhase.Text = "Phase (Degrees)";
        // 
        // groupboxEffects
        // 
        this.groupboxEffects.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                      this.buttonDelete,
                                                                                      this.listboxEffects,
                                                                                      this.comboEffects});
        this.groupboxEffects.Location = new System.Drawing.Point(8, 80);
        this.groupboxEffects.Name = "groupboxEffects";
        this.groupboxEffects.Size = new System.Drawing.Size(144, 280);
        this.groupboxEffects.TabIndex = 71;
        this.groupboxEffects.TabStop = false;
        this.groupboxEffects.Text = "Effects";
        // 
        // buttonDelete
        // 
        this.buttonDelete.Location = new System.Drawing.Point(40, 248);
        this.buttonDelete.Name = "buttonDelete";
        this.buttonDelete.Size = new System.Drawing.Size(64, 24);
        this.buttonDelete.TabIndex = 3;
        this.buttonDelete.Text = "Delete";
        this.buttonDelete.Click += new System.EventHandler(this.buttonDelete_Click);
        // 
        // listboxEffects
        // 
        this.listboxEffects.Location = new System.Drawing.Point(8, 48);
        this.listboxEffects.Name = "listboxEffects";
        this.listboxEffects.Size = new System.Drawing.Size(128, 186);
        this.listboxEffects.TabIndex = 2;
        this.listboxEffects.KeyUp += new System.Windows.Forms.KeyEventHandler(this.listboxEffects_KeyUp);
        this.listboxEffects.SelectedIndexChanged += new System.EventHandler(this.listboxEffects_SelectedIndexChanged);
        // 
        // comboEffects
        // 
        this.comboEffects.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
        this.comboEffects.Items.AddRange(new object[] {
                                                          "Chorus",
                                                          "Compressor",
                                                          "Distortion",
                                                          "Echo",
                                                          "Flanger",
                                                          "Gargle",
                                                          "Waves Reverb",
                                                          "ParamEq"});
        this.comboEffects.Location = new System.Drawing.Point(8, 16);
        this.comboEffects.Name = "comboEffects";
        this.comboEffects.Size = new System.Drawing.Size(128, 21);
        this.comboEffects.TabIndex = 1;
        this.comboEffects.SelectedValueChanged += new System.EventHandler(this.comboEffects_SelectedValueChanged);
        // 
        // timer1
        // 
        this.timer1.Interval = 50;
        this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
        // 
        // MainForm
        // 
        this.AcceptButton = this.buttonOk;
        this.ClientSize = new System.Drawing.Size(700, 472);
        this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                      this.groupboxEffects,
                                                                      this.buttonOk,
                                                                      this.labelParamName1,
                                                                      this.labelParamValue1,
                                                                      this.trackbarSlider1,
                                                                      this.labelParamMin1,
                                                                      this.labelParamMax1,
                                                                      this.labelParamName2,
                                                                      this.labelParamValue2,
                                                                      this.trackbarSlider2,
                                                                      this.labelParamMin2,
                                                                      this.labelParamMax2,
                                                                      this.labelParamName3,
                                                                      this.labelParamValue3,
                                                                      this.trackbarSlider3,
                                                                      this.labelParamMin3,
                                                                      this.labelParamMax3,
                                                                      this.labelParamName4,
                                                                      this.labelParamValue4,
                                                                      this.trackbarSlider4,
                                                                      this.labelParamMin4,
                                                                      this.labelParamMax4,
                                                                      this.labelParamName5,
                                                                      this.labelParamValue5,
                                                                      this.trackbarSlider5,
                                                                      this.labelParamMin5,
                                                                      this.labelParamMax5,
                                                                      this.labelParamName6,
                                                                      this.labelParamValue6,
                                                                      this.trackbarSlider6,
                                                                      this.labelParamMin6,
                                                                      this.labelParamMax6,
                                                                      this.buttonOpen,
                                                                      this.labelTextFilename,
                                                                      this.labelStatic2,
                                                                      this.labelTextStatus,
                                                                      this.checkboxLoop,
                                                                      this.buttonPlay,
                                                                      this.buttonStop,
                                                                      this.labelStatic3,
                                                                      this.labelStatic4,
                                                                      this.groupboxFrameWaveform,
                                                                      this.groupboxFramePhase,
                                                                      this.groupboxFrame});
        this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
        this.Location = new System.Drawing.Point(150, 160);
        this.MaximizeBox = false;
        this.Name = "MainForm";
        this.Text = "SoundFX - Sound effects applied to Device.SecondaryBuffer";
        this.Closing += new System.ComponentModel.CancelEventHandler(this.MainForm_Closing);
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider1)).EndInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider2)).EndInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider3)).EndInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider4)).EndInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider5)).EndInit();
        ((System.ComponentModel.ISupportInitialize)(this.trackbarSlider6)).EndInit();
        this.groupboxFrameWaveform.ResumeLayout(false);
        this.groupboxFramePhase.ResumeLayout(false);
        this.groupboxEffects.ResumeLayout(false);
        this.ResumeLayout(false);

    }
    #endregion
    private void InitDirectSound()
    {
        BufferDescription description = new BufferDescription();
        WaveFormat wfx = new WaveFormat();

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
    }
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            if (applicationBuffer != null)
                applicationBuffer.Dispose();
            if (applicationDevice != null)
                applicationDevice.Dispose();
        }
        base.Dispose (disposing);
    }

    private void buttonOpen_Click(object sender, System.EventArgs e)
    {
        BufferDescription description = new BufferDescription();
        OpenFileDialog ofd = new OpenFileDialog();

        labelTextStatus.Text = "Loading file...";
        // Get the default media path (something like C:\WINDOWS\MEDIA)
        if (string.Empty == path)
            path = Environment.SystemDirectory.Substring(0, Environment.SystemDirectory.LastIndexOf("\\")) + "\\media";

        ofd.DefaultExt = ".wav";
        ofd.Filter = "Wave Files|*.wav|All Files|*.*";
        ofd.FileName = fileName;
        ofd.InitialDirectory = path;

        if (null != applicationBuffer)
        {
            applicationBuffer.Stop();
            applicationBuffer.SetCurrentPosition(0);
        }

        // Display the OpenFileName dialog. Then, try to load the specified file
        if (DialogResult.Cancel == ofd.ShowDialog(this))
        {
            if(null != applicationBuffer)
                applicationBuffer.Dispose();

            labelTextStatus.Text = "No file loaded.";
            return;
        }
        fileName = string.Empty;

        description.ControlEffects = true;
    
        // Create a SecondaryBuffer using the file name.
        try
        {
            applicationBuffer = new SecondaryBuffer(ofd.FileName, description,  applicationDevice);
        }
        catch(BufferTooSmallException)
        {
            labelTextStatus.Text = "Wave file is too small to be used with effects.";
            return;
        }
        catch(FormatException)
        {
            // Invalid file was used. Managed DirectSound tries to convert any files less than
            // 8 bit to 8 bit. Some drivers don't support this conversion, so make sure to
            // catch the FormatException if it's thrown.
            labelTextStatus.Text = "Failed to create SecondaryBuffer from selected file.";
            return;
        }
        
        // Remember the file for next time
        if (null != applicationBuffer)
        {
            fileName = ofd.FileName;
            path =  fileName.Substring(0, fileName.LastIndexOf("\\"));
        }
        labelTextFilename.Text = fileName;
        labelTextStatus.Text = "File loaded.";
    }
    private void buttonPlay_Click(object sender, System.EventArgs e)
    {
        if (null != applicationBuffer)
        {
            applicationBuffer.Play(0, (shouldLoop == true) ? BufferPlayFlags.Looping : BufferPlayFlags.Default);
            timer1.Enabled = true;
            buttonStop.Enabled = true;
            buttonPlay.Enabled = false;
            comboEffects.Enabled = false;
            labelTextStatus.Text="Sound playing.";
        }
    }
    private void buttonStop_Click(object sender, System.EventArgs e)
    {
        if (null != applicationBuffer)
            if (applicationBuffer.Status.Playing == true)
            {
                applicationBuffer.Stop();
                applicationBuffer.SetCurrentPosition(0);
                timer1.Enabled = false;
                buttonStop.Enabled = false;
                buttonPlay.Enabled = true;
                comboEffects.Enabled = true;
                labelTextStatus.Text="Sound stopped.";
            }
    }
    private void comboEffects_SelectedValueChanged(object sender, System.EventArgs e)
    {
        string description  = string.Empty;

        if (null == applicationBuffer)
            return;

        EffectInfo[] temp = new EffectInfo[effectDescription.Count + 1];
        effectDescription.CopyTo(temp, 0);

        switch (comboEffects.SelectedIndex)
        {
            case 0:
                temp[temp.Length - 1].description.GuidEffectClass = DSoundHelper.StandardChorusGuid;
                description = "Chorus";
                break;
            case 1:
                temp[temp.Length - 1].description.GuidEffectClass = DSoundHelper.StandardCompressorGuid;
                description = "Compressor";
                break;
            case 2:
                temp[temp.Length - 1].description.GuidEffectClass = DSoundHelper.StandardDistortionGuid;
                description = "Distortion";
                break;
            case 3:
                temp[temp.Length - 1].description.GuidEffectClass = DSoundHelper.StandardEchoGuid;
                description = "Echo";
                break;
            case 4:
                temp[temp.Length - 1].description.GuidEffectClass = DSoundHelper.StandardFlangerGuid;
                description = "Flanger";
                break;
            case 5:
                temp[temp.Length - 1].description.GuidEffectClass = DSoundHelper.StandardGargleGuid;
                description = "Gargle";
                break;
            case 6:
                temp[temp.Length - 1].description.GuidEffectClass = DSoundHelper.StandardWavesReverbGuid;
                description = "Waves Reverb";
                break;
            case 7:
                temp[temp.Length - 1].description.GuidEffectClass = DSoundHelper.StandardParamEqGuid;
                description = "ParamEq";
                break;
        }       
        
        if (AddEffect(temp))
        {
            effectDescription.Clear();
            effectDescription.AddRange(temp);
            listboxEffects.Items.Add(description);
            listboxEffects.SelectedIndex = listboxEffects.Items.Count - 1;
        }
        
    }
    private bool AddEffect(EffectInfo[] temp)
    {
        EffectsReturnValue[] ret = null;
        EffectDescription[] fx = null;      
        bool WasPlaying = false;        
        int count = 0;

        if (null != temp)
        {
            fx = new EffectDescription[temp.Length];
            count = temp.Length;
        }

        if (true == applicationBuffer.Status.Playing)
            WasPlaying = true;
        
        applicationBuffer.Stop();
        
        // Store the current params for each effect.
        for (int i=0; i < count; i++)
            fx[i] = ((EffectInfo)temp[i]).description;

        try
        {
            ret = applicationBuffer.SetEffects(fx);
        }
        catch(DirectXException)
        {
            labelTextStatus.Text = "Unable to set effect on the buffer. Some effects can't be set on 8 bit wave files.";

            // Revert to the last valid effects.
            if(temp.Length <= 1)
                return false;

            fx = new EffectDescription[temp.Length -1];;
            for (int i=0; i < count - 1; i++)
                fx[i] = ((EffectInfo)temp[i]).description;

            try{ applicationBuffer.SetEffects(fx); }
            catch(DirectXException){}

            return false;
        }
        
        // Restore the params for each effect.
        for (int i=0; i < count; i++)
        {
            EffectInfo eff = new EffectInfo();

            eff.Effect = applicationBuffer.GetEffects(i);
            eff.EffectSettings = temp[i].EffectSettings;
            eff.description = temp[i].description;

            Type efftype = eff.Effect.GetType();

            if (typeof(ChorusEffect) == efftype)
                if (null != eff.EffectSettings)
                    ((ChorusEffect)eff.Effect).AllParameters = (EffectsChorus)eff.EffectSettings;
                else
                    eff.EffectSettings = ((ChorusEffect)eff.Effect).AllParameters;
            else if (typeof(CompressorEffect) ==efftype)
                if (null != eff.EffectSettings)
                    ((CompressorEffect)eff.Effect).AllParameters = (EffectsCompressor)eff.EffectSettings;
                else
                    eff.EffectSettings = ((CompressorEffect)eff.Effect).AllParameters;
            else if (typeof(DistortionEffect) == efftype)
                if (null != eff.EffectSettings)
                    ((DistortionEffect)eff.Effect).AllParameters = (EffectsDistortion)eff.EffectSettings;
                else
                    eff.EffectSettings = ((DistortionEffect)eff.Effect).AllParameters;
            else if (typeof(EchoEffect) == efftype)
                if (null != eff.EffectSettings)
                    ((EchoEffect)eff.Effect).AllParameters = (EffectsEcho)eff.EffectSettings;
                else
                    eff.EffectSettings = ((EchoEffect)eff.Effect).AllParameters;
            else if (typeof(FlangerEffect) == efftype)
                     if (null != eff.EffectSettings)
                         ((FlangerEffect)eff.Effect).AllParameters = (EffectsFlanger)eff.EffectSettings;
                     else
                         eff.EffectSettings = ((FlangerEffect)eff.Effect).AllParameters;
            else if (typeof(GargleEffect) == efftype)
                     if (null != eff.EffectSettings)
                         ((GargleEffect)eff.Effect).AllParameters = (EffectsGargle)eff.EffectSettings;
                     else
                         eff.EffectSettings = ((GargleEffect)eff.Effect).AllParameters;
            else if (typeof(ParamEqEffect) == efftype)
                     if (null != eff.EffectSettings)
                         ((ParamEqEffect)eff.Effect).AllParameters = (EffectsParamEq)eff.EffectSettings;
                     else
                         eff.EffectSettings = ((ParamEqEffect)eff.Effect).AllParameters;
            else if (typeof(WavesReverbEffect) == efftype)
                     if (null != eff.EffectSettings)
                         ((WavesReverbEffect)eff.Effect).AllParameters = (EffectsWavesReverb)eff.EffectSettings;
                     else
                         eff.EffectSettings = ((WavesReverbEffect)eff.Effect).AllParameters;

            temp[i] = eff;
        }
        
        if (WasPlaying)
            applicationBuffer.Play(0, (shouldLoop == true) ? BufferPlayFlags.Looping : BufferPlayFlags.Default);

        if (null != temp)
            if ((EffectsReturnValue.LocatedInHardware == ret[temp.Length - 1]) || (EffectsReturnValue.LocatedInSoftware == ret[temp.Length - 1]))
                return true;

        return false;
    }
    private void checkboxLoop_CheckedChanged(object sender, System.EventArgs e)
    {
        shouldLoop = checkboxLoop.Checked;
    }
    private void listboxEffects_SelectedIndexChanged(object sender, System.EventArgs e)
    {
        if (-1 == listboxEffects.SelectedIndex)
            return;

        currentIndex = listboxEffects.SelectedIndex;

        // Make sure we don't update any settings while updating the UI
        isIgnoringSettings = true;
        UpdateUI(true);
        isIgnoringSettings = false;
    }
    private void UpdateUI(bool MoveControls)
    {
        ClearUI(MoveControls);
        
        Type    efftype = ((EffectInfo)effectDescription[currentIndex]).Effect.GetType();
        object  eff     = ((EffectInfo)effectDescription[currentIndex]).Effect;

        if (typeof(ChorusEffect) == efftype)
        {
            EffectsChorus temp = ((ChorusEffect)eff).AllParameters;

            if (MoveControls)
            {
                trackbarSlider1.Minimum = (int)ChorusEffect.WetDryMixMin;
                trackbarSlider1.Maximum = (int)ChorusEffect.WetDryMixMax;
                trackbarSlider1.Value = (int)temp.WetDryMix;
                trackbarSlider2.Minimum = (int)ChorusEffect.DepthMin;
                trackbarSlider2.Maximum = (int)ChorusEffect.DepthMax;
                trackbarSlider2.Value = (int)temp.Depth;
                trackbarSlider3.Minimum = (int)ChorusEffect.FeedbackMin;
                trackbarSlider3.Maximum = (int)ChorusEffect.FeedbackMax;
                trackbarSlider3.Value = (int)temp.Feedback;
                trackbarSlider4.Minimum = (int)ChorusEffect.FrequencyMin;
                trackbarSlider4.Maximum = (int)ChorusEffect.FrequencyMax;
                trackbarSlider4.Value = (int)temp.Frequency;
                trackbarSlider5.Minimum = (int)ChorusEffect.DelayMin;
                trackbarSlider5.Maximum = (int)ChorusEffect.DelayMax;
                trackbarSlider5.Value = (int)temp.Delay;

                if (ChorusEffect.WaveSin == temp.Waveform)
                    radiobuttonRadioSine.Checked = true;
                else
                    radiobuttonTriangle.Checked = true;

                if (ChorusEffect.PhaseNegative180 == temp.Phase)
                    radiobuttonRadioNeg180.Checked = true;
                else if (ChorusEffect.PhaseNegative90 == temp.Phase)
                    radiobuttonRadioNeg90.Checked = true;
                else if (ChorusEffect.PhaseZero == temp.Phase)
                    radiobuttonRadioZero.Checked = true;
                else if (ChorusEffect.Phase90 == temp.Phase)
                    radiobuttonRadio90.Checked = true;
                else if (ChorusEffect.Phase180 == temp.Phase)
                    radiobuttonRadio180.Checked = true;

                groupboxFramePhase.Enabled = radiobuttonRadioNeg180.Enabled = radiobuttonRadioNeg90.Enabled = 
                radiobuttonRadioZero.Enabled = radiobuttonRadio90.Enabled = radiobuttonRadio180.Enabled = 
                groupboxFrameWaveform.Enabled = radiobuttonRadioSine.Enabled = 
                radiobuttonTriangle.Enabled = true;

                trackbarSlider1.Enabled = trackbarSlider2.Enabled = trackbarSlider3.Enabled = 
                trackbarSlider4.Enabled = trackbarSlider5.Enabled = true;
            }
            labelParamValue1.Text = temp.WetDryMix.ToString();
            labelParamName1.Text = "Wet/Dry Mix (%)";
            
            labelParamValue2.Text =  temp.Depth.ToString();
            labelParamName2.Text = "Depth (%)";

            labelParamValue3.Text = temp.Feedback.ToString();
            labelParamName3.Text = "Feedback (%)";

            labelParamValue4.Text = temp.Frequency.ToString();
            labelParamName4.Text = "Frequency (Hz)";

            labelParamValue5.Text = temp.Delay.ToString();
            labelParamName5.Text = "Delay (ms)" ;
        }
        else if (typeof(CompressorEffect) == efftype)
        {
            EffectsCompressor temp = ((CompressorEffect)eff).AllParameters;
            
            if (MoveControls)
            {
                trackbarSlider1.Minimum = (int)CompressorEffect.GainMin;
                trackbarSlider1.Maximum = (int)CompressorEffect.GainMax;
                trackbarSlider1.Value = (int)temp.Gain;
                trackbarSlider2.Minimum = (int)CompressorEffect.AttackMin;
                trackbarSlider2.Maximum = (int)CompressorEffect.AttackMax;
                trackbarSlider2.Value = (int)temp.Attack;
                trackbarSlider3.Minimum = (int)CompressorEffect.ReleaseMin;
                trackbarSlider3.Maximum = (int)CompressorEffect.ReleaseMax;
                trackbarSlider3.Value = (int)temp.Release;
                trackbarSlider4.Minimum = (int)CompressorEffect.ThresholdMin;
                trackbarSlider4.Maximum = (int)CompressorEffect.ThresholdMax;
                trackbarSlider4.Value = (int)temp.Threshold;
                trackbarSlider5.Minimum = (int)CompressorEffect.RatioMin;
                trackbarSlider5.Maximum = (int)CompressorEffect.RatioMax;
                trackbarSlider5.Value = (int)temp.Ratio;
                trackbarSlider6.Minimum = (int)CompressorEffect.PreDelayMin;
                trackbarSlider6.Maximum = (int)CompressorEffect.PreDelayMax;
                trackbarSlider6.Value = (int)temp.Predelay;

                trackbarSlider1.Enabled = trackbarSlider2.Enabled = trackbarSlider3.Enabled = 
                    trackbarSlider4.Enabled = trackbarSlider5.Enabled = trackbarSlider6.Enabled = true;
            }
            labelParamValue1.Text = temp.Gain.ToString();
            labelParamName1.Text = "Gain (dB)";

            labelParamName2.Text = "Attack (ms)";
            labelParamValue2.Text = temp.Attack.ToString();
            
            labelParamName3.Text = "Release (ms)";
            labelParamValue3.Text = temp.Release.ToString();
            
            labelParamName4.Text = "Threshold (dB)";
            labelParamValue4.Text = temp.Threshold.ToString();
            
            labelParamName5.Text = "Ratio (x:1)";
            labelParamValue5.Text = temp.Ratio.ToString();
            
            labelParamName6.Text = "Predelay (ms)";
            labelParamValue6.Text = temp.Predelay.ToString();
        }
        else if (typeof(DistortionEffect) == efftype)
        {
            EffectsDistortion temp = ((DistortionEffect)eff).AllParameters;
            
            if (MoveControls)
            {
                trackbarSlider1.Minimum = (int)DistortionEffect.GainMin;
                trackbarSlider1.Maximum = (int)DistortionEffect.GainMax;
                trackbarSlider1.Value = (int)temp.Gain;
                trackbarSlider2.Minimum = (int)DistortionEffect.EdgeMin;
                trackbarSlider2.Maximum = (int)DistortionEffect.EdgeMax;
                trackbarSlider2.Value = (int)temp.Edge;
                trackbarSlider3.Minimum = (int)DistortionEffect.PostEqCenterFrequencyMin;
                trackbarSlider3.Maximum = (int)DistortionEffect.PostEqCenterFrequencyMax;
                trackbarSlider3.Value = (int)temp.PostEqCenterFrequency;
                trackbarSlider4.Minimum = (int)DistortionEffect.PostEqBandwidthMin;
                trackbarSlider4.Maximum = (int)DistortionEffect.PostEqBandwidthMax;
                trackbarSlider4.Value = (int)temp.PostEqBandwidth;
                trackbarSlider5.Minimum = (int)DistortionEffect.PreLowPassCutoffMin;
                trackbarSlider5.Maximum = (int)DistortionEffect.PreLowPassCutoffMax;
                trackbarSlider5.Value = (int)temp.PreLowpassCutoff;

                trackbarSlider1.Enabled = trackbarSlider2.Enabled = trackbarSlider3.Enabled = 
                    trackbarSlider4.Enabled = trackbarSlider5.Enabled = true;
            }
            labelParamName1.Text = "Gain (dB)";
            labelParamValue1.Text = temp.Gain.ToString();
            
            labelParamName2.Text = "Edge (%)";
            labelParamValue2.Text = temp.Edge.ToString();
            
            labelParamName3.Text = "PostEQ Center Freq (Hz)";
            labelParamValue3.Text = temp.PostEqCenterFrequency.ToString();
            
            labelParamName4.Text = "PostEQ Bandwidth (Hz)";
            labelParamValue4.Text = temp.PostEqBandwidth.ToString();
            
            labelParamName5.Text = "PreLowpass Cutoff (Hz)";
            labelParamValue5.Text = temp.PreLowpassCutoff.ToString();
        }
        else if (typeof(EchoEffect) == efftype)
        {
            EffectsEcho temp = ((EchoEffect)eff).AllParameters;
            
            if (MoveControls)
            {
                trackbarSlider1.Minimum = (int)EchoEffect.WetDryMixMin;
                trackbarSlider1.Maximum = (int)EchoEffect.WetDryMixMax;
                trackbarSlider1.Value = (int)temp.WetDryMix;
                trackbarSlider2.Minimum = (int)EchoEffect.FeedbackMin;
                trackbarSlider2.Maximum = (int)EchoEffect.FeedbackMax;
                trackbarSlider2.Value = (int)temp.Feedback;
                trackbarSlider3.Minimum = (int)EchoEffect.LeftDelayMin;
                trackbarSlider3.Maximum = (int)EchoEffect.LeftDelayMax;
                trackbarSlider3.Value = (int)temp.LeftDelay;
                trackbarSlider4.Minimum = (int)EchoEffect.RightDelayMin;
                trackbarSlider4.Maximum = (int)EchoEffect.RightDelayMax;
                trackbarSlider4.Value = (int)temp.RightDelay;
                trackbarSlider5.Minimum = EchoEffect.PanDelayMin;
                trackbarSlider5.Maximum = EchoEffect.PanDelayMax;
                trackbarSlider5.Value = temp.PanDelay;

                trackbarSlider1.Enabled = trackbarSlider2.Enabled = trackbarSlider3.Enabled = 
                trackbarSlider4.Enabled = trackbarSlider5.Enabled = true;
            }
            labelParamName1.Text = "Wet/Dry Mix (%)";
            labelParamValue1.Text = temp.WetDryMix.ToString();
            
            labelParamName2.Text = "Feedback (%)";
            labelParamValue2.Text = temp.Feedback.ToString();
            
            labelParamName3.Text = "Left Delay (ms)";           
            labelParamValue3.Text = temp.LeftDelay.ToString();
                        
            labelParamName4.Text = "Right Delay (ms)";
            labelParamValue4.Text = temp.RightDelay.ToString();
            
            labelParamName5.Text = "Pan Delay (bool)";
            labelParamValue5.Text = temp.PanDelay.ToString();
        }
        else if (typeof(FlangerEffect) == efftype)
        {
            EffectsFlanger temp = ((FlangerEffect)eff).AllParameters;
            
            if (MoveControls)
            {
                trackbarSlider1.Minimum = (int)FlangerEffect.WetDryMixMin;
                trackbarSlider1.Maximum = (int)FlangerEffect.WetDryMixMax;
                trackbarSlider1.Value = (int)temp.WetDryMix;
                trackbarSlider2.Minimum = (int)FlangerEffect.DepthMin;
                trackbarSlider2.Maximum = (int)FlangerEffect.DepthMax;
                trackbarSlider2.Value = (int)temp.Depth;
                trackbarSlider3.Minimum = (int)FlangerEffect.FeedbackMin;
                trackbarSlider3.Maximum = (int)FlangerEffect.FeedbackMax;
                trackbarSlider3.Value = (int)temp.Feedback;
                trackbarSlider4.Minimum = (int)FlangerEffect.FrequencyMin;
                trackbarSlider4.Maximum = (int)FlangerEffect.FrequencyMax;
                trackbarSlider4.Value = (int)temp.Frequency;
                trackbarSlider5.Minimum = (int)FlangerEffect.DelayMin;
                trackbarSlider5.Maximum = (int)FlangerEffect.DelayMax;
                trackbarSlider5.Value = (int)temp.Delay;

                if (ChorusEffect.WaveSin == temp.Waveform)
                    radiobuttonRadioSine.Checked = true;
                else
                    radiobuttonTriangle.Checked = true;
            
                if (FlangerEffect.PhaseNeg180 == temp.Phase)
                    radiobuttonRadioNeg180.Checked = true;
                else if (FlangerEffect.PhaseNeg90 == temp.Phase)
                    radiobuttonRadioNeg90.Checked = true;
                else if (FlangerEffect.PhaseZero == temp.Phase)
                    radiobuttonRadioZero.Checked = true;
                else if (FlangerEffect.Phase90 == temp.Phase)
                    radiobuttonRadio90.Checked = true;
                else if (FlangerEffect.Phase180 == temp.Phase)
                    radiobuttonRadio180.Checked = true;

                groupboxFramePhase.Enabled = radiobuttonRadioNeg180.Enabled = radiobuttonRadioNeg90.Enabled = 
                radiobuttonRadioZero.Enabled = radiobuttonRadio90.Enabled = radiobuttonRadio180.Enabled = 
                groupboxFrameWaveform.Enabled = radiobuttonRadioSine.Enabled = radiobuttonTriangle.Enabled = true;

                trackbarSlider1.Enabled = trackbarSlider2.Enabled = trackbarSlider3.Enabled = 
                trackbarSlider4.Enabled = trackbarSlider5.Enabled = true;
            }
            labelParamName1.Text = "Wet/Dry Mix (%)";
            labelParamValue1.Text = temp.WetDryMix.ToString();
            
            labelParamName2.Text = "Depth (%)";
            labelParamValue2.Text = temp.Depth.ToString();
            
            labelParamName3.Text = "Feedback (%)";
            labelParamValue3.Text = temp.Feedback.ToString();
            
            labelParamName4.Text = "Frequency (Hz)";
            labelParamValue4.Text = temp.Frequency.ToString();
            
            labelParamName5.Text = "Delay (ms)";
            labelParamValue5.Text = temp.Delay.ToString();          
        }
        else if (typeof(GargleEffect) == efftype)
        {
            EffectsGargle temp = ((GargleEffect)eff).AllParameters;
            
            if (MoveControls)
            {
                trackbarSlider1.Minimum = GargleEffect.RateHzMin;
                trackbarSlider1.Maximum = GargleEffect.RateHzMax;
                trackbarSlider1.Value = temp.RateHz;

                if (GargleEffect.WaveSquare == temp.WaveShape)
                    radiobuttonSquare.Checked = true;
                else
                    radiobuttonTriangle.Checked = true;

                groupboxFrameWaveform.Enabled = radiobuttonSquare.Enabled = radiobuttonTriangle.Enabled = true;

                trackbarSlider1.Enabled = true;
            }
            labelParamName1.Text = "Rate (Hz)";
            labelParamValue1.Text = temp.RateHz.ToString();     
        }
        else if (typeof(ParamEqEffect) == efftype)
        {
            EffectsParamEq temp = ((ParamEqEffect)eff).AllParameters;
            
            if (MoveControls)
            {
                trackbarSlider1.Minimum = (int)ParamEqEffect.CenterMin;
                trackbarSlider1.Maximum = (int)ParamEqEffect.CenterMax;
                trackbarSlider1.Value = (int)temp.Center;
                trackbarSlider2.Minimum = (int)ParamEqEffect.BandwidthMin;
                trackbarSlider2.Maximum = (int)ParamEqEffect.BandwidthMax;
                trackbarSlider2.Value = (int)temp.Bandwidth;
                trackbarSlider3.Minimum = (int)ParamEqEffect.GainMin;
                trackbarSlider3.Maximum = (int)ParamEqEffect.GainMax;
                trackbarSlider3.Value = (int)temp.Gain;

                trackbarSlider1.Enabled = trackbarSlider2.Enabled = trackbarSlider3.Enabled = true;
            }
            labelParamName1.Text = "Center Freq (Hz)";
            labelParamValue1.Text = temp.Center.ToString();
            
            labelParamName2.Text = "Bandwidth (Hz)";
            labelParamValue2.Text = temp.Bandwidth.ToString();
            
            labelParamName3.Text = "Gain (dB)";
            labelParamValue3.Text = temp.Gain.ToString();
        }
        else if (typeof(WavesReverbEffect) == efftype)
        {
            EffectsWavesReverb temp = ((WavesReverbEffect)eff).AllParameters;
            
            if (MoveControls)
            {
                trackbarSlider1.Minimum = (int)WavesReverbEffect.InGainMin;
                trackbarSlider1.Maximum = (int)WavesReverbEffect.InGainMax;
                trackbarSlider1.Value = (int)temp.InGain;
                trackbarSlider2.Minimum = (int)WavesReverbEffect.ReverbMixMin;
                trackbarSlider2.Maximum = (int)WavesReverbEffect.ReverbMixMax;
                trackbarSlider2.Value = (int)temp.ReverbMix;
                trackbarSlider3.Minimum = (int)(1000 * WavesReverbEffect.ReverbTimeMin);
                trackbarSlider3.Maximum = (int)(1000 * WavesReverbEffect.ReverbTimeMax);
                trackbarSlider3.Value = (int)(1000 * temp.ReverbTime);
                trackbarSlider4.Minimum = (int)(1000 * WavesReverbEffect.HighFrequencyRtRatioMin);
                trackbarSlider4.Maximum = (int)(1000 * WavesReverbEffect.HighFrequencyRtRatioMax);
                trackbarSlider4.Value = (int)(1000 * temp.HighFrequencyRtRatio);

                trackbarSlider1.Enabled = trackbarSlider2.Enabled = trackbarSlider3.Enabled = trackbarSlider4.Enabled = true;
            }
            labelParamName1.Text = "In Gain (dB)";
            labelParamValue1.Text = temp.InGain.ToString();
            
            labelParamName2.Text = "Waves Reverb Mix (dB)";
            labelParamValue2.Text = temp.ReverbMix.ToString();
            
            labelParamName3.Text = "Waves Reverb Time (ms)";
            labelParamValue3.Text = temp.ReverbTime.ToString();
            
            labelParamName4.Text = "HighFreq RT Ratio (x:1)";
            labelParamValue4.Text = temp.HighFrequencyRtRatio.ToString();
        }
    }
    private void ClearUI(bool ClearControls)
    {
        labelParamName1.Text = labelParamValue1.Text = labelParamName2.Text = labelParamValue2.Text =  
        labelParamName3.Text = labelParamValue3.Text = labelParamName4.Text = labelParamValue4.Text =
        labelParamName5.Text = labelParamValue5.Text = labelParamName6.Text = labelParamName1.Text = 
        labelParamValue1.Text = labelParamName2.Text = labelParamValue2.Text = labelParamName3.Text = 
        labelParamValue3.Text = labelParamName4.Text = labelParamValue4.Text = labelParamName5.Text = 
        labelParamValue5.Text =labelParamName6.Text = labelParamValue6.Text = labelParamName1.Text = 
        labelParamValue1.Text = labelParamName2.Text = labelParamValue2.Text = labelParamName3.Text = 
        labelParamValue3.Text = labelParamName4.Text = labelParamValue4.Text = labelParamName5.Text = 
        labelParamValue5.Text = labelParamName6.Text = labelParamName1.Text = labelParamValue1.Text =
        labelParamName2.Text = labelParamValue2.Text = labelParamName3.Text = labelParamValue3.Text =
        labelParamName4.Text = labelParamValue4.Text = labelParamName5.Text = labelParamValue5.Text =
        labelParamName6.Text = labelParamName1.Text = labelParamValue1.Text = labelParamName2.Text = 
        labelParamValue2.Text = labelParamName3.Text = labelParamValue3.Text = labelParamName4.Text = 
        labelParamValue4.Text = labelParamName5.Text = labelParamValue5.Text = labelParamName6.Text =       
        labelParamName1.Text = labelParamValue1.Text = labelParamName2.Text = labelParamValue2.Text =
        labelParamName3.Text = labelParamName4.Text = labelParamValue4.Text = labelParamName5.Text = 
        labelParamValue5.Text = labelParamName6.Text = labelParamValue6.Text = labelParamName1.Text = 
        labelParamValue1.Text = labelParamName2.Text = labelParamValue2.Text = labelParamName3.Text = 
        labelParamValue3.Text = labelParamName4.Text = labelParamValue4.Text = labelParamName5.Text = 
        labelParamValue5.Text = labelParamName6.Text = labelParamValue6.Text = labelParamName1.Text = 
        labelParamValue1.Text = labelParamName2.Text = labelParamValue2.Text = labelParamName3.Text = 
        labelParamValue3.Text = labelParamName4.Text = labelParamValue4.Text = labelParamName5.Text = 
        labelParamValue5.Text = labelParamName6.Text = labelParamValue6.Text = string.Empty;

        if (ClearControls)
        {
            groupboxFrameWaveform.Enabled = radiobuttonTriangle.Enabled = radiobuttonTriangle.Enabled = radiobuttonRadioSine.Enabled =
            groupboxFramePhase.Enabled = radiobuttonRadioNeg180.Enabled = radiobuttonRadioNeg90.Enabled = radiobuttonRadioZero.Enabled =
            radiobuttonRadio90.Enabled = radiobuttonRadio180.Enabled = false;

            trackbarSlider1.Minimum = trackbarSlider2.Minimum = trackbarSlider3.Minimum = 
            trackbarSlider4.Minimum = trackbarSlider5.Minimum = trackbarSlider6.Minimum = 0;
            trackbarSlider1.Value = trackbarSlider2.Value = trackbarSlider3.Value = 
            trackbarSlider4.Value = trackbarSlider5.Value = trackbarSlider6.Value = 0;
            trackbarSlider1.Enabled = trackbarSlider2.Enabled = trackbarSlider3.Enabled = 
            trackbarSlider4.Enabled = trackbarSlider5.Enabled = trackbarSlider6.Enabled = false;
        }
    }

    private void trackbarSliderScroll(object sender, System.EventArgs e)
    {
        // We're ignoring settings right now
        if (isIgnoringSettings)
            return;

        EffectInfo eff = (EffectInfo)effectDescription[currentIndex];
        Type efftype = eff.Effect.GetType();

        if (typeof(ChorusEffect) == efftype)
        {
            EffectsChorus temp = new EffectsChorus();
            temp.WetDryMix = trackbarSlider1.Value;
            temp.Frequency = trackbarSlider4.Value;
            temp.Feedback = trackbarSlider3.Value;
            temp.Depth = trackbarSlider2.Value;
            temp.Delay = trackbarSlider5.Value;
            
            if (true == radiobuttonRadioSine.Checked)
                temp.Waveform = ChorusEffect.WaveSin;
            else
                temp.Waveform = ChorusEffect.WaveTriangle;
            
            if (true == radiobuttonRadioNeg180.Checked)
                temp.Phase = ChorusEffect.PhaseNegative180;
            else if (true == radiobuttonRadioNeg90.Checked)
                temp.Phase = ChorusEffect.PhaseNegative90;
            else if (true == radiobuttonRadioZero.Checked)
                temp.Phase = ChorusEffect.PhaseZero;
            else if (true == radiobuttonRadio90.Checked)
                temp.Phase = ChorusEffect.Phase90;
            else if (true == radiobuttonRadio180.Checked)
                temp.Phase = ChorusEffect.Phase180;

            eff.EffectSettings = temp;
            ((ChorusEffect)eff.Effect).AllParameters = temp;            
        }
        else if (typeof(CompressorEffect) == efftype)
        {
            EffectsCompressor temp = new EffectsCompressor();
            temp.Gain = trackbarSlider1.Value;
            temp.Attack = trackbarSlider2.Value;
            temp.Release = trackbarSlider3.Value;
            temp.Threshold = trackbarSlider4.Value;
            temp.Ratio = trackbarSlider5.Value;
            temp.Predelay = trackbarSlider6.Value;

            eff.EffectSettings = temp;
            ((CompressorEffect)eff.Effect).AllParameters = temp;
        }
        else if (typeof(DistortionEffect) == efftype)
        {
            EffectsDistortion temp = new EffectsDistortion();
            temp.Gain = trackbarSlider1.Value;
            temp.Edge = trackbarSlider2.Value;
            temp.PostEqCenterFrequency = trackbarSlider3.Value;
            temp.PostEqBandwidth = trackbarSlider4.Value;
            temp.PreLowpassCutoff = trackbarSlider5.Value;

            eff.EffectSettings = temp;
            ((DistortionEffect)eff.Effect).AllParameters = temp;
        }
        else if (typeof(EchoEffect) == efftype)
        {
            EffectsEcho temp = new EffectsEcho();
            temp.WetDryMix = trackbarSlider1.Value;
            temp.Feedback = trackbarSlider2.Value;
            temp.LeftDelay = trackbarSlider3.Value;
            temp.RightDelay = trackbarSlider4.Value;
            temp.PanDelay = trackbarSlider5.Value;

            eff.EffectSettings = temp;
            ((EchoEffect)eff.Effect).AllParameters = temp;
        }
        else if (typeof(FlangerEffect) == efftype)
        {
            EffectsFlanger temp = new EffectsFlanger();
            temp.WetDryMix = trackbarSlider1.Value;
            temp.Depth = trackbarSlider2.Value;
            temp.Feedback = trackbarSlider3.Value;
            temp.Frequency = trackbarSlider4.Value;
            temp.Delay = trackbarSlider5.Value;
        
            if (true == radiobuttonRadioSine.Checked)
                temp.Waveform = FlangerEffect.WaveSin;
            else
                temp.Waveform = FlangerEffect.WaveTriangle;

            if (true == radiobuttonRadioNeg180.Checked)
                temp.Phase = ChorusEffect.PhaseNegative180;
            else if (true == radiobuttonRadioNeg90.Checked)
                temp.Phase = ChorusEffect.PhaseNegative90;
            else if (true == radiobuttonRadioZero.Checked)
                temp.Phase = ChorusEffect.PhaseZero;
            else if (true == radiobuttonRadio90.Checked)
                temp.Phase = ChorusEffect.Phase90;
            else if (true == radiobuttonRadio180.Checked)
                temp.Phase = ChorusEffect.Phase180;

            eff.EffectSettings = temp;
            ((FlangerEffect)eff.Effect).AllParameters = temp;
        }
        else if (typeof(GargleEffect) == efftype)
        {
            EffectsGargle temp = new EffectsGargle();
            temp.RateHz = trackbarSlider1.Value;
            if (radiobuttonSquare.Checked)
                temp.WaveShape = GargleEffect.WaveSquare;
            else
                temp.WaveShape = GargleEffect.WaveTriangle;

            if (true == radiobuttonSquare.Checked)
                temp.WaveShape = GargleEffect.WaveSquare;
            else
                temp.WaveShape = GargleEffect.WaveTriangle;

            eff.EffectSettings = temp;
            ((GargleEffect)eff.Effect).AllParameters = temp;
        }
        else if (typeof(ParamEqEffect) == efftype)
        {
            EffectsParamEq temp = new EffectsParamEq();
            temp.Center = trackbarSlider1.Value;
            temp.Bandwidth = trackbarSlider2.Value;
            temp.Gain = trackbarSlider3.Value;

            eff.EffectSettings = temp;
            ((ParamEqEffect)eff.Effect).AllParameters = temp;
        }
        else if (typeof(WavesReverbEffect) == efftype)
        {
            EffectsWavesReverb temp = new EffectsWavesReverb();
            temp.InGain = trackbarSlider1.Value;
            temp.ReverbMix = trackbarSlider2.Value;
            temp.ReverbTime = (float)(.001 * trackbarSlider3.Value);
            temp.HighFrequencyRtRatio = (float)(.001 * trackbarSlider4.Value);

            eff.EffectSettings = temp;
            ((WavesReverbEffect)eff.Effect).AllParameters = temp;
        }       
        effectDescription[currentIndex] = eff;
        UpdateUI(false);
    }

    private void DeleteEffect()
    {
        EffectInfo[] temp = null;

        if (-1 == listboxEffects.SelectedIndex) 
            return;

        effectDescription.RemoveAt(listboxEffects.SelectedIndex);

        if (effectDescription.Count > 0)
        {
            temp = new EffectInfo[effectDescription.Count];
            effectDescription.CopyTo(temp, 0);
            AddEffect(temp);
            listboxEffects.Items.RemoveAt(listboxEffects.SelectedIndex);                
            listboxEffects.SelectedIndex = currentIndex = 0;
        }
        else
        {
            temp = null;
            AddEffect(temp);
            listboxEffects.Items.Clear();
            ClearUI(true);
        }
        effectDescription.Clear();
        if (null != temp)
            effectDescription.AddRange(temp);        
    }

    private void listboxEffects_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
    {
        if (e.KeyCode == Keys.Delete)
            DeleteEffect();
    }

    private void buttonOk_Click(object sender, System.EventArgs e)
    {
        this.Close();
    }

    private void buttonDelete_Click(object sender, System.EventArgs e)
    {
        DeleteEffect();
    }

    private void timer1_Tick(object sender, System.EventArgs e)
    {
        if(applicationBuffer.Status.Playing)
            return;
        else
        {
            timer1.Enabled = false;
            buttonStop.Enabled = false;
            buttonPlay.Enabled = true;
            comboEffects.Enabled = true;

            labelTextStatus.Text="Sound stopped.";
        }    
    }

    private void MainForm_Closing(object sender, System.ComponentModel.CancelEventArgs e)
    {
        if (null != applicationBuffer)
        {
            if(applicationBuffer.Status.Playing)
                applicationBuffer.Stop();
        }
    }
}
