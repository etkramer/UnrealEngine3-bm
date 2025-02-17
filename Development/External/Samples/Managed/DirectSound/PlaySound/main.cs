using System;
using System.Drawing;
using System.Windows.Forms;
using System.IO;
using Microsoft.DirectX;
using Microsoft.DirectX.DirectSound;
using Buffer = Microsoft.DirectX.DirectSound.SecondaryBuffer;
using Microsoft.Samples.DirectX.UtilityToolkit;


public class MainForm : Form
{
    private System.ComponentModel.Container components = null;
    private System.Windows.Forms.Button btnSoundfile;
    private Label lblFilename;
    private CheckBox cbLoopCheck;
    private System.Windows.Forms.Button btnPlay;
    private System.Windows.Forms.Button btnStop;
    private System.Windows.Forms.Button btnCancel;
        
    private SecondaryBuffer ApplicationBuffer = null;
    private Device ApplicationDevice = null;
    private string PathSoundFile = string.Empty;

    [STAThread]
    public static void Main()
    {
        using (MainForm form = new MainForm())
        {
            Application.Run(form);
        }
    }
    
    protected override void Dispose( bool disposing )
    {
        if(disposing)
        {
            if (null != components)
            {
                components.Dispose();
            }
        }
        base.Dispose(disposing);    
    }
    public MainForm()
    {
        //
        // Required for Windows Form Designer support
        //
        InitializeComponent();
    }
    #region InitializeComponent code
    private void InitializeComponent()
    {
        this.btnSoundfile = new System.Windows.Forms.Button();
        this.lblFilename = new System.Windows.Forms.Label();
        this.cbLoopCheck = new System.Windows.Forms.CheckBox();
        this.btnPlay = new System.Windows.Forms.Button();
        this.btnStop = new System.Windows.Forms.Button();
        this.btnCancel = new System.Windows.Forms.Button();
        this.SuspendLayout();
        // 
        // btnSoundfile
        // 
        this.btnSoundfile.Location = new System.Drawing.Point(10, 11);
        this.btnSoundfile.Name = "btnSoundfile";
        this.btnSoundfile.Size = new System.Drawing.Size(69, 21);
        this.btnSoundfile.TabIndex = 0;
        this.btnSoundfile.Text = "Sound &file...";
        this.btnSoundfile.Click += new System.EventHandler(this.btnSoundfile_Click);
        // 
        // lblFilename
        // 
        this.lblFilename.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.lblFilename.Location = new System.Drawing.Point(94, 11);
        this.lblFilename.Name = "lblFilename";
        this.lblFilename.Size = new System.Drawing.Size(345, 21);
        this.lblFilename.TabIndex = 1;
        this.lblFilename.Text = "No file loaded.";
        this.lblFilename.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
        // 
        // cbLoopCheck
        // 
        this.cbLoopCheck.Enabled = false;
        this.cbLoopCheck.Location = new System.Drawing.Point(9, 44);
        this.cbLoopCheck.Name = "cbLoopCheck";
        this.cbLoopCheck.Size = new System.Drawing.Size(87, 16);
        this.cbLoopCheck.TabIndex = 2;
        this.cbLoopCheck.Text = "&Loop sound";
        // 
        // btnPlay
        // 
        this.btnPlay.Enabled = false;
        this.btnPlay.Location = new System.Drawing.Point(104, 48);
        this.btnPlay.Name = "btnPlay";
        this.btnPlay.TabIndex = 3;
        this.btnPlay.Text = "&Play";
        this.btnPlay.Click += new System.EventHandler(this.btnPlay_Click);
        // 
        // btnStop
        // 
        this.btnStop.Enabled = false;
        this.btnStop.Location = new System.Drawing.Point(176, 48);
        this.btnStop.Name = "btnStop";
        this.btnStop.TabIndex = 4;
        this.btnStop.Text = "&Stop";
        this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
        // 
        // btnCancel
        // 
        this.btnCancel.Location = new System.Drawing.Point(364, 48);
        this.btnCancel.Name = "btnCancel";
        this.btnCancel.TabIndex = 5;
        this.btnCancel.Text = "E&xit";
        this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
        // 
        // MainForm
        // 
        this.AcceptButton = this.btnSoundfile;
        this.ClientSize = new System.Drawing.Size(450, 77);
        this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                      this.btnSoundfile,
                                                                      this.lblFilename,
                                                                      this.cbLoopCheck,
                                                                      this.btnPlay,
                                                                      this.btnStop,
                                                                      this.btnCancel});
        this.Name = "MainForm";
        this.Text = "PlaySound";
        this.Load += new System.EventHandler(this.MainForm_Load);
        this.ResumeLayout(false);

    }
    #endregion

    private void btnCancel_Click(object sender, System.EventArgs e)
    {
        Close();
    }

    private void btnStop_Click(object sender, System.EventArgs e)
    {
        if(null != ApplicationBuffer)
            ApplicationBuffer.Stop();
    }

    private void btnSoundfile_Click(object sender, System.EventArgs e)
    {
        OpenFileDialog ofd = new OpenFileDialog();

        ofd.InitialDirectory = PathSoundFile;
        ofd.Filter=  "Wave files(*.wav)|*.wav";

        // Stop the sound if it's already playing before you open the the dialog
        if (ApplicationBuffer != null)
        {
            ApplicationBuffer.Stop();
        }

        if( DialogResult.Cancel == ofd.ShowDialog() )
            return;
     
        if(LoadSoundFile(ofd.FileName))
        {
            PathSoundFile = Path.GetDirectoryName(ofd.FileName);
            lblFilename.Text = Path.GetFileName(ofd.FileName);
            EnablePlayUI(true);
        }
        else
        {
            lblFilename.Text = "Could not create sound buffer.";
            EnablePlayUI(false);
        }
    }

    private bool LoadSoundFile(string name)
    {
        try
        {
            ApplicationBuffer = new SecondaryBuffer(name, ApplicationDevice);
        }
        catch(Exception)
        {
            return false;
        }
        return true;
    }

    private void EnablePlayUI(bool enable)
    {
        if (enable)
        {
            cbLoopCheck.Enabled = true;
            btnPlay.Enabled = true;
            btnStop.Enabled = true;
        }
        else
        {
            cbLoopCheck.Enabled = false;
            btnPlay.Enabled = false;
            btnStop.Enabled = false;
        }
    }

    private void MainForm_Load(object sender, System.EventArgs e)
    {
        try
        {
            ApplicationDevice = new Device();
            ApplicationDevice.SetCooperativeLevel(this, CooperativeLevel.Priority);
        }
        catch
        {
            MessageBox.Show("Unable to create sound device. Sample will now exit.");
            this.Close();
        }
    }

    private void btnPlay_Click(object sender, System.EventArgs e)
    {
        if(null != ApplicationBuffer)
            ApplicationBuffer.Play(0, (cbLoopCheck.Checked ? BufferPlayFlags.Looping : BufferPlayFlags.Default));
    }
}
