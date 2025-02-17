//----------------------------------------------------------------------------
// File: Main.cs
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.IO;
using System.Drawing;
using System.Windows.Forms;
using System.Threading;
using Microsoft.DirectX;
using Microsoft.DirectX.DirectSound;

public class MainForm : Form
{
	private Label labelMainInputformatText;
	private CheckBox checkboxRecord;
	private Button buttonSoundfile;
	private Label labelFilename;
	private Label labelStatic;
    
	public BufferPositionNotify[] PositionNotify = new BufferPositionNotify[NumberRecordNotifications + 1];  
	public const int NumberRecordNotifications	= 16;
	public AutoResetEvent NotificationEvent	= null;
	public CaptureBuffer applicationBuffer = null;
	public Guid CaptureDeviceGuid = Guid.Empty;
	public Capture applicationDevice = null;
	private string FileName = string.Empty;
	public Notify applicationNotify = null;
	private Thread NotifyThread = null;
	private FileStream WaveFile = null;
	private BinaryWriter Writer = null;
	private string Path = string.Empty;
	public int CaptureBufferSize = 0;
	public int NextCaptureOffset = 0;
	private bool Recording = false;
	public WaveFormat InputFormat;
	private int SampleCount = 0;
	public int NotifySize = 0;
    private bool Capturing = false;

    [STAThread]
	public static void Main()
	{
        try
        {
            using(MainForm form = new MainForm())
            {
                form.ShowDialog();
            }
        }
        catch{}
	}
    
	private void MainForm_Closing(object sender, System.ComponentModel.CancelEventArgs e)
	{
        if (null != NotificationEvent)
        {
            Capturing = false;
            NotificationEvent.Set();
        }
		if (null != applicationBuffer)
			if (applicationBuffer.Capturing)
				StartOrStopRecord(false);
	}

	public MainForm()
	{
		//
		// Required for Windows Form Designer support
		//
		InitializeComponent();
		
		DevicesForm devices = new DevicesForm(this);
		devices.ShowDialog(this);
		InitDirectSound();
		if (null == applicationDevice)
			Close();
		else
		{
			FormatsForm	formats = new FormatsForm(this);
            if (formats.ShowDialog(this) == DialogResult.OK)
            {
		
                labelMainInputformatText.Text = string.Format("{0} Hz, {1}-bit ", 
                    InputFormat.SamplesPerSecond, 
                    InputFormat.BitsPerSample) +
                    ((1 == InputFormat.Channels) ? "Mono" : "Stereo");

                CreateCaptureBuffer();
            }
            else
            {
                this.Close();
            }
		}
	}
    #region InitializeComponent code
	private void InitializeComponent()
	{
		this.labelStatic = new System.Windows.Forms.Label();
		this.labelMainInputformatText = new System.Windows.Forms.Label();
		this.checkboxRecord = new System.Windows.Forms.CheckBox();
		this.buttonSoundfile = new System.Windows.Forms.Button();
		this.labelFilename = new System.Windows.Forms.Label();
		this.SuspendLayout();
		// 
		// labelStatic
		// 
		this.labelStatic.Location = new System.Drawing.Point(10, 46);
		this.labelStatic.Name = "labelStatic";
		this.labelStatic.Size = new System.Drawing.Size(75, 13);
		this.labelStatic.TabIndex = 0;
		this.labelStatic.Text = "Input Format:";
		// 
		// labelMainInputformatText
		// 
		this.labelMainInputformatText.Location = new System.Drawing.Point(90, 46);
		this.labelMainInputformatText.Name = "labelMainInputformatText";
		this.labelMainInputformatText.Size = new System.Drawing.Size(262, 13);
		this.labelMainInputformatText.TabIndex = 1;
		// 
		// checkboxRecord
		// 
		this.checkboxRecord.Appearance = System.Windows.Forms.Appearance.Button;
		this.checkboxRecord.Enabled = false;
		this.checkboxRecord.FlatStyle = System.Windows.Forms.FlatStyle.System;
		this.checkboxRecord.Location = new System.Drawing.Point(369, 40);
		this.checkboxRecord.Name = "checkboxRecord";
		this.checkboxRecord.Size = new System.Drawing.Size(75, 23);
		this.checkboxRecord.TabIndex = 2;
		this.checkboxRecord.Text = "&Record";
		this.checkboxRecord.CheckedChanged += new System.EventHandler(this.checkboxRecord_CheckedChanged);
		// 
		// buttonSoundfile
		// 
		this.buttonSoundfile.Location = new System.Drawing.Point(10, 11);
		this.buttonSoundfile.Name = "buttonSoundfile";
		this.buttonSoundfile.Size = new System.Drawing.Size(78, 21);
		this.buttonSoundfile.TabIndex = 3;
		this.buttonSoundfile.Text = "Sound &file...";
		this.buttonSoundfile.Click += new System.EventHandler(this.buttonSoundfile_Click);
		// 
		// labelFilename
		// 
		this.labelFilename.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
		this.labelFilename.Location = new System.Drawing.Point(96, 11);
		this.labelFilename.Name = "labelFilename";
		this.labelFilename.Size = new System.Drawing.Size(348, 21);
		this.labelFilename.TabIndex = 4;
		this.labelFilename.Text = "No file loaded.";
		this.labelFilename.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
		// 
		// MainForm
		// 
		this.AcceptButton = this.buttonSoundfile;
		this.ClientSize = new System.Drawing.Size(454, 77);
		this.Controls.AddRange(new System.Windows.Forms.Control[] {
																	  this.labelStatic,
																	  this.labelMainInputformatText,
																	  this.checkboxRecord,
																	  this.buttonSoundfile,
																	  this.labelFilename});
		this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
		this.Name = "MainForm";
		this.Text = "CaptureSound";
		this.Closing += new System.ComponentModel.CancelEventHandler(this.MainForm_Closing);
		this.ResumeLayout(false);

	}
    #endregion
	private void InitDirectSound()
	{
		CaptureBufferSize = 0;
		NotifySize = 0;

		// Create DirectSound.Capture using the preferred capture device
		try
		{
			applicationDevice = new Capture(CaptureDeviceGuid);
		}
		catch {}
	}
	void CreateCaptureBuffer()
	{
		//-----------------------------------------------------------------------------
		// Name: CreateCaptureBuffer()
		// Desc: Creates a capture buffer and sets the format 
		//-----------------------------------------------------------------------------
		CaptureBufferDescription dscheckboxd = new CaptureBufferDescription(); 

		if (null != applicationNotify)
		{
			applicationNotify.Dispose();
			applicationNotify = null;
		}
		if (null != applicationBuffer)
		{
			applicationBuffer.Dispose();
			applicationBuffer = null;
		}

		if (0 == InputFormat.Channels)
			return;

		// Set the notification size
		NotifySize = (1024 > InputFormat.AverageBytesPerSecond / 8) ? 1024 : (InputFormat.AverageBytesPerSecond / 8);
		NotifySize -= NotifySize % InputFormat.BlockAlign;   

		// Set the buffer sizes
		CaptureBufferSize = NotifySize * NumberRecordNotifications;

		// Create the capture buffer
		dscheckboxd.BufferBytes = CaptureBufferSize;
		InputFormat.FormatTag = WaveFormatTag.Pcm;
		dscheckboxd.Format = InputFormat; // Set the format during creatation
		
		applicationBuffer = new CaptureBuffer(dscheckboxd, applicationDevice);
		NextCaptureOffset = 0;

		InitNotifications();
	}
	void InitNotifications()
	{
		//-----------------------------------------------------------------------------
		// Name: InitNotifications()
		// Desc: Inits the notifications on the capture buffer which are handled
		//       in the notify thread.
		//-----------------------------------------------------------------------------

		if (null == applicationBuffer)
			throw new NullReferenceException();
		
		// Create a thread to monitor the notify events
		if (null == NotifyThread)
		{
			NotifyThread = new Thread(new ThreadStart(WaitThread));
            Capturing = true;
			NotifyThread.Start();

			// Create a notification event, for when the sound stops playing
			NotificationEvent = new AutoResetEvent(false);
		}


		// Setup the notification positions
		for (int i = 0; i < NumberRecordNotifications; i++)
		{
			PositionNotify[i].Offset = (NotifySize * i) + NotifySize - 1;
			PositionNotify[i].EventNotifyHandle = NotificationEvent.Handle;
		}
		
		applicationNotify = new Notify(applicationBuffer);

		// Tell DirectSound when to notify the app. The notification will come in the from 
		// of signaled events that are handled in the notify thread.
		applicationNotify.SetNotificationPositions(PositionNotify, NumberRecordNotifications);
	}
	private void checkboxRecord_CheckedChanged(object sender, System.EventArgs e)
	{
		Recording = !Recording;
		StartOrStopRecord(Recording);

		if (!Recording)
			checkboxRecord.Enabled = false;
	}
	void StartOrStopRecord(bool StartRecording)
	{
		//-----------------------------------------------------------------------------
		// Name: StartOrStopRecord()
		// Desc: Starts or stops the capture buffer from recording
		//-----------------------------------------------------------------------------

		if (StartRecording)
		{
			// Create a capture buffer, and tell the capture 
			// buffer to start recording   
			CreateCaptureBuffer();
			applicationBuffer.Start(true);
		}
		else
		{
			// Stop the buffer, and read any data that was not 
			// caught by a notification
			applicationBuffer.Stop();

			RecordCapturedData();
			
			Writer.Seek(4, SeekOrigin.Begin); // Seek to the length descriptor of the RIFF file.
			Writer.Write((int)(SampleCount + 36));	// Write the file length, minus first 8 bytes of RIFF description.
			Writer.Seek(40, SeekOrigin.Begin); // Seek to the data length descriptor of the RIFF file.
			Writer.Write(SampleCount); // Write the length of the sample data in bytes.
			
			Writer.Close();	// Close the file now.
			Writer = null;	// Set the writer to null.
			WaveFile = null; // Set the FileStream to null.
		}
	}
	void CreateRIFF()
	{
		/**************************************************************************
			 
			Here is where the file will be created. A
			wave file is a RIFF file, which has chunks
			of data that describe what the file contains.
			A wave RIFF file is put together like this:
			 
			The 12 byte RIFF chunk is constructed like this:
			Bytes 0 - 3 :	'R' 'I' 'F' 'F'
			Bytes 4 - 7 :	Length of file, minus the first 8 bytes of the RIFF description.
							(4 bytes for "WAVE" + 24 bytes for format chunk length +
							8 bytes for data chunk description + actual sample data size.)
			Bytes 8 - 11:	'W' 'A' 'V' 'E'
			
			The 24 byte FORMAT chunk is constructed like this:
			Bytes 0 - 3 :	'f' 'm' 't' ' '
			Bytes 4 - 7 :	The format chunk length. This is always 16.
			Bytes 8 - 9 :	File padding. Always 1.
			Bytes 10- 11:	Number of channels. Either 1 for mono,  or 2 for stereo.
			Bytes 12- 15:	Sample rate.
			Bytes 16- 19:	Number of bytes per second.
			Bytes 20- 21:	Bytes per sample. 1 for 8 bit mono, 2 for 8 bit stereo or
							16 bit mono, 4 for 16 bit stereo.
			Bytes 22- 23:	Number of bits per sample.
			
			The DATA chunk is constructed like this:
			Bytes 0 - 3 :	'd' 'a' 't' 'a'
			Bytes 4 - 7 :	Length of data, in bytes.
			Bytes 8 -...:	Actual sample data.
			
		***************************************************************************/

		// Open up the wave file for writing.
		WaveFile = new FileStream(FileName, FileMode.Create);
		Writer = new BinaryWriter(WaveFile);

		// Set up file with RIFF chunk info.
		char[] ChunkRiff = {'R','I','F','F'};
		char[] ChunkType = {'W','A','V','E'};
		char[] ChunkFmt	= {'f','m','t',' '};
		char[] ChunkData = {'d','a','t','a'};
			
		short shPad = 1; // File padding
		int nFormatChunkLength = 0x10; // Format chunk length.
		int nLength = 0; // File length, minus first 8 bytes of RIFF description. This will be filled in later.
		short shBytesPerSample = 0; // Bytes per sample.

		// Figure out how many bytes there will be per sample.
		if (8 == InputFormat.BitsPerSample && 1 == InputFormat.Channels)
			shBytesPerSample = 1;
		else if ((8 == InputFormat.BitsPerSample && 2 == InputFormat.Channels) || (16 == InputFormat.BitsPerSample && 1 == InputFormat.Channels))
			shBytesPerSample = 2;
		else if (16 == InputFormat.BitsPerSample && 2 == InputFormat.Channels)
			shBytesPerSample = 4;

		// Fill in the riff info for the wave file.
		Writer.Write(ChunkRiff);
		Writer.Write(nLength);
		Writer.Write(ChunkType);

		// Fill in the format info for the wave file.
		Writer.Write(ChunkFmt);
		Writer.Write(nFormatChunkLength);
		Writer.Write(shPad);
		Writer.Write(InputFormat.Channels);
		Writer.Write(InputFormat.SamplesPerSecond);
		Writer.Write(InputFormat.AverageBytesPerSecond);
		Writer.Write(shBytesPerSample);
		Writer.Write(InputFormat.BitsPerSample);
			
		// Now fill in the data chunk.
		Writer.Write(ChunkData);
		Writer.Write((int)0);	// The sample length will be written in later.
	}

	void RecordCapturedData() 
	{
		//-----------------------------------------------------------------------------
		// Name: RecordCapturedData()
		// Desc: Copies data from the capture buffer to the output buffer 
		//-----------------------------------------------------------------------------
		byte[] CaptureData = null;
		int ReadPos;
		int CapturePos;
		int LockSize;

		applicationBuffer.GetCurrentPosition(out CapturePos, out ReadPos);
		LockSize = ReadPos - NextCaptureOffset;
		if (LockSize < 0)
			LockSize += CaptureBufferSize;

		// Block align lock size so that we are always write on a boundary
		LockSize -= (LockSize % NotifySize);

		if (0 == LockSize)
			return;

		// Read the capture buffer.
		CaptureData = (byte[])applicationBuffer.Read(NextCaptureOffset, typeof(byte), LockFlag.None, LockSize);

		// Write the data into the wav file
		Writer.Write(CaptureData, 0, CaptureData.Length);
		
		// Update the number of samples, in bytes, of the file so far.
		SampleCount += CaptureData.Length;

		// Move the capture offset along
		NextCaptureOffset += CaptureData.Length; 
		NextCaptureOffset %= CaptureBufferSize; // Circular buffer
	}

	private void buttonSoundfile_Click(object sender, System.EventArgs e)
	{
        OnCreateSoundFile();
	}

    private void OnCreateSoundFile()
    {
        //-----------------------------------------------------------------------------
        // Name: OnCreateSoundFile()
        // Desc: Called when the user requests to save to a sound file
        //-----------------------------------------------------------------------------

        SaveFileDialog ofd = new SaveFileDialog();
        WaveFormat wf = new WaveFormat();
        DialogResult result;

        // Get the default media path (something like C:\WINDOWS\MEDIA)
        if (string.Empty == Path)
            Path = Environment.SystemDirectory.Substring(0, Environment.SystemDirectory.LastIndexOf("\\")) + "\\media";

        if (Recording)
        {
            // Stop the capture and read any data that 
            // was not caught by a notification
            StartOrStopRecord(false);
            Recording = false;
        }

        ofd.DefaultExt = ".wav";
        ofd.Filter = "Wave Files|*.wav|All Files|*.*";
        ofd.FileName = FileName;
        ofd.InitialDirectory = Path;
		
        // Update the UI controls to show the sound as loading a file
        checkboxRecord.Enabled = false;
        labelFilename.Text = "Saving file...";

        // Display the OpenFileName dialog. Then, try to load the specified file
        result = ofd.ShowDialog(this);

        if (DialogResult.Abort == result || DialogResult.Cancel == result)
        {
            labelFilename.Text = "Save aborted.";
            return;
        }
        FileName = ofd.FileName;		

        try
        {
            CreateRIFF();
        }
        catch
        {
            labelFilename.Text = "Could not create wave file.";
            return;
        }

        // Update the UI controls to show the sound as the file is loaded
        labelFilename.Text = FileName;
        checkboxRecord.Enabled = true;

        // Remember the path for next time
        Path = FileName.Substring(0, FileName.LastIndexOf("\\"));
    }

	private void WaitThread()
	{
		while(Capturing)
		{
			//Sit here and wait for a message to arrive
			NotificationEvent.WaitOne(Timeout.Infinite, true);
			RecordCapturedData();
		}
	}

}
