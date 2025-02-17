//----------------------------------------------------------------------------
// File: Formats.cs
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Drawing;
using System.Windows.Forms;
using System.Collections;
using Microsoft.DirectX;
using Microsoft.DirectX.DirectSound;

public class FormatsForm : Form
{
	private struct FormatInfo
	{
		public WaveFormat format;
		public override string ToString()
		{
			return ConvertWaveFormatToString(format);
		}
	};
	private Button buttonOk;
	private Button buttonCancel;
	private ListBox lbFormatsInputListbox;
	private Label labelStatic;
	
	private MainForm	mf = null;
	private ArrayList	formats = new ArrayList();
	private bool[]		InputFormatSupported = new bool[20];
	public FormatsForm(MainForm mf)
	{
		//
		// Required for Windows Form Designer support
		//
		InitializeComponent();
		this.mf = mf;

		ScanAvailableInputFormats();
		FillFormatListBox();
	}
    #region InitializeComponent code
	private void InitializeComponent()
	{
		this.buttonOk = new System.Windows.Forms.Button();
		this.buttonCancel = new System.Windows.Forms.Button();
		this.lbFormatsInputListbox = new System.Windows.Forms.ListBox();
		this.labelStatic = new System.Windows.Forms.Label();
		this.SuspendLayout();
		// 
		// buttonOk
		// 
		this.buttonOk.Enabled = false;
        this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.OK;
		this.buttonOk.Location = new System.Drawing.Point(10, 128);
		this.buttonOk.Name = "buttonOk";
		this.buttonOk.TabIndex = 0;
		this.buttonOk.Text = "OK";
		this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
		// 
		// buttonCancel
		// 
		this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
		this.buttonCancel.Location = new System.Drawing.Point(97, 128);
		this.buttonCancel.Name = "buttonCancel";
		this.buttonCancel.TabIndex = 1;
		this.buttonCancel.Text = "Cancel";
		// 
		// lbFormatsInputListbox
		// 
		this.lbFormatsInputListbox.Location = new System.Drawing.Point(10, 24);
		this.lbFormatsInputListbox.Name = "lbFormatsInputListbox";
		this.lbFormatsInputListbox.Size = new System.Drawing.Size(162, 95);
		this.lbFormatsInputListbox.TabIndex = 2;
		this.lbFormatsInputListbox.SelectedIndexChanged += new System.EventHandler(this.lbFormatsInputListbox_SelectedIndexChanged);
		// 
		// labelStatic
		// 
		this.labelStatic.Location = new System.Drawing.Point(10, 11);
		this.labelStatic.Name = "labelStatic";
		this.labelStatic.Size = new System.Drawing.Size(75, 13);
		this.labelStatic.TabIndex = 3;
		this.labelStatic.Text = "Input Format:";
		// 
		// FormatsForm
		// 
		this.AcceptButton = this.buttonOk;
		this.CancelButton = this.buttonCancel;
		this.ClientSize = new System.Drawing.Size(183, 170);
		this.Controls.AddRange(new System.Windows.Forms.Control[] {
																	  this.buttonOk,
																	  this.buttonCancel,
																	  this.lbFormatsInputListbox,
																	  this.labelStatic});
		this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
		this.Name = "FormatsForm";
		this.Text = "Select Capture Format";
		this.ResumeLayout(false);

	}
    #endregion
	private void ScanAvailableInputFormats()
	{
		//-----------------------------------------------------------------------------
		// Name: ScanAvailableInputFormats()
		// Desc: Tests to see if 20 different standard wave formats are supported by
		//       the capture device 
		//-----------------------------------------------------------------------------
		WaveFormat format = new WaveFormat();
		CaptureBufferDescription dscheckboxd = new CaptureBufferDescription();
		CaptureBuffer pDSCaptureBuffer = null;
    
		// This might take a second or two, so throw up the hourglass
		Cursor = Cursors.WaitCursor;
    
		format.FormatTag = WaveFormatTag.Pcm;

		// Try 20 different standard formats to see if they are supported
		for (int iIndex = 0; iIndex < 20; iIndex++)
		{
			GetWaveFormatFromIndex(iIndex, ref format);

			// To test if a capture format is supported, try to create a 
			// new capture buffer using a specific format.  If it works
			// then the format is supported, otherwise not.
			dscheckboxd.BufferBytes = format.AverageBytesPerSecond;
			dscheckboxd.Format = format;
        
			try
			{
				pDSCaptureBuffer = new CaptureBuffer(dscheckboxd, mf.applicationDevice);
                InputFormatSupported[ iIndex ] = true;
            }
			catch
            {
                InputFormatSupported[ iIndex ] = false;
            }
            if (pDSCaptureBuffer != null)
			    pDSCaptureBuffer.Dispose();
		}
		Cursor = Cursors.Default;
	}
	private void GetWaveFormatFromIndex(int Index, ref WaveFormat format)
	{
		//-----------------------------------------------------------------------------
		// Name: GetWaveFormatFromIndex()
		// Desc: Returns 20 different wave formats based on Index
		//-----------------------------------------------------------------------------
		int SampleRate = Index / 4;
		int iType = Index % 4;

		switch (SampleRate)
		{
			case 0: format.SamplesPerSecond = 48000; break;
			case 1: format.SamplesPerSecond = 44100; break;
			case 2: format.SamplesPerSecond = 22050; break;
			case 3: format.SamplesPerSecond = 11025; break;
			case 4: format.SamplesPerSecond =  8000; break;
		}

		switch (iType)
		{
			case 0: format.BitsPerSample =  8; format.Channels = 1; break;
			case 1: format.BitsPerSample = 16; format.Channels = 1; break;
			case 2: format.BitsPerSample =  8; format.Channels = 2; break;
			case 3: format.BitsPerSample = 16; format.Channels = 2; break;
		}

		format.BlockAlign = (short)(format.Channels * (format.BitsPerSample / 8));
		format.AverageBytesPerSecond = format.BlockAlign * format.SamplesPerSecond;
	}
	void FillFormatListBox()
	{
		//-----------------------------------------------------------------------------
		// Name: FillFormatListBox()
		// Desc: Fills the format list box based on the availible formats
		//-----------------------------------------------------------------------------
		FormatInfo		info			= new FormatInfo();		
		string			strFormatName	= string.Empty;
		WaveFormat	format				= new WaveFormat();

		for (int iIndex = 0; iIndex < InputFormatSupported.Length; iIndex++)
		{
			if (true == InputFormatSupported[iIndex])
			{
				// Turn the index into a WaveFormat then turn that into a
				// string and put the string in the listbox
				GetWaveFormatFromIndex(iIndex, ref format);
				info.format = format;
				formats.Add(info);
			}
		}
		lbFormatsInputListbox.DataSource = formats;
	}
	private static string ConvertWaveFormatToString(WaveFormat format)
	{
		//-----------------------------------------------------------------------------
		// Name: ConvertWaveFormatToString()
		// Desc: Converts a wave format to a text string
		//-----------------------------------------------------------------------------
		return format.SamplesPerSecond + " Hz, " + 
			format.BitsPerSample + "-bit " + 
			((format.Channels == 1) ? "Mono" : "Stereo");
	}
	private void FormatsOK()
	{
		//-----------------------------------------------------------------------------
		// Name: FormatsOK()
		// Desc: Stores the capture buffer format based on what was selected
		//-----------------------------------------------------------------------------
		
		mf.InputFormat = ((FormatInfo)formats[lbFormatsInputListbox.SelectedIndex]).format;
        this.DialogResult = DialogResult.OK;
		Close();
	}
	private void buttonOk_Click(object sender, System.EventArgs e)
	{
		FormatsOK();
	}

	private void lbFormatsInputListbox_SelectedIndexChanged(object sender, System.EventArgs e)
	{
		buttonOk.Enabled = true;
	}
}
