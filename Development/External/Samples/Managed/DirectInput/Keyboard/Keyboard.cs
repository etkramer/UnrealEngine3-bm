//-----------------------------------------------------------------------------
// File: Keyboard.cs
//
// Desc: The Keyboard sample obtains and displays Keyboard data.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
using System;
using System.Drawing;
using System.Windows.Forms;
using Microsoft.DirectX.DirectInput;
using Microsoft.DirectX;

public class KeyboardForm : Form
{
    private System.ComponentModel.IContainer components;
    private GroupBox staticGroupBox1;
    private GroupBox staticGroupBox2;
    private GroupBox staticGroupBox3;
    private Label staticDataLabel;
    private Label dataLabel;
    private System.Windows.Forms.Button createDevice;
    private System.Windows.Forms.Button cancel;
    private Label behaviorLabel;
    private System.Windows.Forms.Timer grabData;
    private System.Windows.Forms.RadioButton immediate;
    private System.Windows.Forms.RadioButton buffered;
    private System.Windows.Forms.RadioButton exclusive;
    private System.Windows.Forms.RadioButton nonexclusive;
    private System.Windows.Forms.CheckBox windowsKey;
    private System.Windows.Forms.Panel panel1;
    private System.Windows.Forms.RadioButton foreground;
    private System.Windows.Forms.RadioButton background;

    int sizeSampleBuffer  = 8;
    Device applicationDevice = null;


    
    
    public static void Main()
    {
        using (KeyboardForm form = new KeyboardForm())
        {
            Application.Run(form);
        }
    }
    



    public KeyboardForm()
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
            catch {}
        }
        //
        // Required for Windows Form Designer support.
        //
        InitializeComponent();
    }
    #region InitializeComponent code
    
    
    
    
    private void InitializeComponent()
    {
        this.components = new System.ComponentModel.Container();
        System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(KeyboardForm));
        this.staticGroupBox1 = new System.Windows.Forms.GroupBox();
        this.exclusive = new System.Windows.Forms.RadioButton();
        this.nonexclusive = new System.Windows.Forms.RadioButton();
        this.windowsKey = new System.Windows.Forms.CheckBox();
        this.panel1 = new System.Windows.Forms.Panel();
        this.background = new System.Windows.Forms.RadioButton();
        this.foreground = new System.Windows.Forms.RadioButton();
        this.staticGroupBox2 = new System.Windows.Forms.GroupBox();
        this.immediate = new System.Windows.Forms.RadioButton();
        this.buffered = new System.Windows.Forms.RadioButton();
        this.staticDataLabel = new System.Windows.Forms.Label();
        this.dataLabel = new System.Windows.Forms.Label();
        this.createDevice = new System.Windows.Forms.Button();
        this.cancel = new System.Windows.Forms.Button();
        this.staticGroupBox3 = new System.Windows.Forms.GroupBox();
        this.behaviorLabel = new System.Windows.Forms.Label();
        this.grabData = new System.Windows.Forms.Timer(this.components);
        this.staticGroupBox1.SuspendLayout();
        this.panel1.SuspendLayout();
        this.staticGroupBox2.SuspendLayout();
        this.SuspendLayout();
        // 
        // staticGroupBox1
        // 
        this.staticGroupBox1.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                               this.exclusive,
                                                                               this.nonexclusive,
                                                                               this.windowsKey,
                                                                               this.panel1});
        this.staticGroupBox1.Location = new System.Drawing.Point(10, 11);
        this.staticGroupBox1.Name = "staticGroupBox1";
        this.staticGroupBox1.Size = new System.Drawing.Size(247, 79);
        this.staticGroupBox1.TabIndex = 0;
        this.staticGroupBox1.TabStop = false;
        this.staticGroupBox1.Text = "Cooperative Level";
        // 
        // exclusive
        // 
        this.exclusive.Checked = true;
        this.exclusive.Location = new System.Drawing.Point(15, 13);
        this.exclusive.Name = "exclusive";
        this.exclusive.Size = new System.Drawing.Size(72, 16);
        this.exclusive.TabIndex = 6;
        this.exclusive.TabStop = true;
        this.exclusive.Text = "Exclusive";
        this.exclusive.Click += new System.EventHandler(this.ChangeProperties);
        // 
        // nonexclusive
        // 
        this.nonexclusive.Location = new System.Drawing.Point(15, 31);
        this.nonexclusive.Name = "nonexclusive";
        this.nonexclusive.Size = new System.Drawing.Size(96, 16);
        this.nonexclusive.TabIndex = 7;
        this.nonexclusive.Text = "Nonexclusive";
        this.nonexclusive.Click += new System.EventHandler(this.ChangeProperties);
        // 
        // windowsKey
        // 
        this.windowsKey.Location = new System.Drawing.Point(15, 50);
        this.windowsKey.Name = "windowsKey";
        this.windowsKey.Size = new System.Drawing.Size(105, 16);
        this.windowsKey.TabIndex = 8;
        this.windowsKey.Text = "Disable Windows Key";
        // 
        // panel1
        // 
        this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                             this.background,
                                                                             this.foreground});
        this.panel1.Location = new System.Drawing.Point(136, 8);
        this.panel1.Name = "panel1";
        this.panel1.Size = new System.Drawing.Size(104, 48);
        this.panel1.TabIndex = 15;
        // 
        // background
        // 
        this.background.Location = new System.Drawing.Point(8, 24);
        this.background.Name = "background";
        this.background.Size = new System.Drawing.Size(90, 16);
        this.background.TabIndex = 12;
        this.background.Text = "Background";
        this.background.Click += new System.EventHandler(this.ChangeProperties);
        // 
        // foreground
        // 
        this.foreground.Checked = true;
        this.foreground.Location = new System.Drawing.Point(8, 8);
        this.foreground.Name = "foreground";
        this.foreground.Size = new System.Drawing.Size(82, 16);
        this.foreground.TabIndex = 11;
        this.foreground.TabStop = true;
        this.foreground.Text = "Foreground";
        this.foreground.Click += new System.EventHandler(this.ChangeProperties);
        // 
        // staticGroupBox2
        // 
        this.staticGroupBox2.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                                this.immediate,
                                                                                this.buffered});
        this.staticGroupBox2.Location = new System.Drawing.Point(267, 11);
        this.staticGroupBox2.Name = "staticGroupBox2";
        this.staticGroupBox2.Size = new System.Drawing.Size(105, 79);
        this.staticGroupBox2.TabIndex = 6;
        this.staticGroupBox2.TabStop = false;
        this.staticGroupBox2.Text = "Data Style";
        // 
        // immediate
        // 
        this.immediate.Checked = true;
        this.immediate.Location = new System.Drawing.Point(13, 22);
        this.immediate.Name = "immediate";
        this.immediate.Size = new System.Drawing.Size(78, 16);
        this.immediate.TabIndex = 9;
        this.immediate.TabStop = true;
        this.immediate.Text = "Immediate";
        this.immediate.Click += new System.EventHandler(this.ChangeProperties);
        // 
        // buffered
        // 
        this.buffered.Location = new System.Drawing.Point(13, 40);
        this.buffered.Name = "buffered";
        this.buffered.Size = new System.Drawing.Size(70, 16);
        this.buffered.TabIndex = 10;
        this.buffered.Text = "Buffered";
        this.buffered.Click += new System.EventHandler(this.ChangeProperties);
        // 
        // staticDataLabel
        // 
        this.staticDataLabel.Location = new System.Drawing.Point(16, 347);
        this.staticDataLabel.Name = "staticDataLabel";
        this.staticDataLabel.Size = new System.Drawing.Size(32, 13);
        this.staticDataLabel.TabIndex = 9;
        this.staticDataLabel.Text = "Data:";
        // 
        // dataLabel
        // 
        this.dataLabel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
        this.dataLabel.Location = new System.Drawing.Point(48, 344);
        this.dataLabel.Name = "dataLabel";
        this.dataLabel.Size = new System.Drawing.Size(504, 18);
        this.dataLabel.TabIndex = 10;
        this.dataLabel.Text = "Device not created";
        // 
        // createDevice
        // 
        this.createDevice.Location = new System.Drawing.Point(10, 376);
        this.createDevice.Name = "createDevice";
        this.createDevice.Size = new System.Drawing.Size(102, 23);
        this.createDevice.TabIndex = 11;
        this.createDevice.Text = "Create Device";
        this.createDevice.Click += new System.EventHandler(this.btnCreatedevice_Click);
        // 
        // cancel
        // 
        this.cancel.Location = new System.Drawing.Point(477, 376);
        this.cancel.Name = "cancel";
        this.cancel.TabIndex = 12;
        this.cancel.Text = "Exit";
        this.cancel.Click += new System.EventHandler(this.btnCancel_Click);
        // 
        // staticGroupBox3
        // 
        this.staticGroupBox3.Location = new System.Drawing.Point(10, 91);
        this.staticGroupBox3.Name = "staticGroupBox3";
        this.staticGroupBox3.Size = new System.Drawing.Size(541, 245);
        this.staticGroupBox3.TabIndex = 13;
        this.staticGroupBox3.TabStop = false;
        this.staticGroupBox3.Text = "Expected Behavior";
        // 
        // behaviorLabel
        // 
        this.behaviorLabel.Location = new System.Drawing.Point(21, 108);
        this.behaviorLabel.Name = "behaviorLabel";
        this.behaviorLabel.Size = new System.Drawing.Size(519, 216);
        this.behaviorLabel.TabIndex = 14;
        this.behaviorLabel.Text = "Behavior";
        // 
        // grabData
        // 
        this.grabData.Interval = 83;
        this.grabData.Tick += new System.EventHandler(this.GrabData_Tick);
        // 
        // KeyboardForm
        // 
        this.AcceptButton = this.createDevice;
        this.ClientSize = new System.Drawing.Size(562, 410);
        this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                      this.behaviorLabel,
                                                                      this.staticDataLabel,
                                                                      this.dataLabel,
                                                                      this.createDevice,
                                                                      this.cancel,
                                                                      this.staticGroupBox1,
                                                                      this.staticGroupBox2,
                                                                      this.staticGroupBox3});
        this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
        this.KeyPreview = true;
        this.MaximizeBox = false;
        this.Name = "KeyboardForm";
        this.Text = "DirectInput Keyboard Sample";
        this.Closing += new System.ComponentModel.CancelEventHandler(this.KeyboardForm_Closing);
        this.Activated += new System.EventHandler(this.KeyboardForm_Activated);
        this.staticGroupBox1.ResumeLayout(false);
        this.panel1.ResumeLayout(false);
        this.staticGroupBox2.ResumeLayout(false);
        this.ResumeLayout(false);

    }
    #endregion

    
    
    
    private void ChangeProperties(object sender, System.EventArgs e)
    {
        UpdateUI();
    }

    
    
    
    private void btnCreatedevice_Click(object sender, System.EventArgs e)
    {
        if (null != applicationDevice)
        {
            FreeDirectInput();
            UpdateUI();
            return;
        }

        CooperativeLevelFlags coopFlags;
        
        // Cleanup any previous call first.
        grabData.Enabled = false;    
        FreeDirectInput();
    
        if (true == exclusive.Checked)
            coopFlags = CooperativeLevelFlags.Exclusive;
        else
            coopFlags = CooperativeLevelFlags.NonExclusive;

        if (true == foreground.Checked)
            coopFlags |= CooperativeLevelFlags.Foreground;
        else
            coopFlags |= CooperativeLevelFlags.Background;

        // Disabling the windows key is only allowed only if we are in foreground nonexclusive.
        if (true == windowsKey.Checked && !exclusive.Checked && foreground.Checked)
            coopFlags |= CooperativeLevelFlags.NoWindowsKey;

        try
        {
            // Obtain an instantiated system keyboard device.
            applicationDevice = new Device(SystemGuid.Keyboard);
        }
        catch(InputException)
        {
            FreeDirectInput();
            MessageBox.Show("Unable to create device.");
            return;
        }

        // Set the cooperative level to let DirectInput know how
        // this device should interact with the system and with other
        // DirectInput applications.
        try
        {
            applicationDevice.SetCooperativeLevel(this, coopFlags);
        }
        catch(InputException dx)
        {
            if (dx is UnsupportedException && !foreground.Checked)
            {
                FreeDirectInput();
                MessageBox.Show("SetCooperativeLevel() returned UnsupportedException.\n" +
                    "For security reasons, background keyboard\n" +
                        "access is not allowed.", "Keyboard");
                return;
            }
        }

        if (false == immediate.Checked)
        {
            // IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
            //
            // DirectInput uses unbuffered I/O (buffer size = 0) by default.
            // If you want to read buffered data, you need to set a nonzero
            // buffer size.
            //
            // The buffer size is an int property associated with the device.

            try
            {
                applicationDevice.Properties.BufferSize = sizeSampleBuffer;
            }
            catch(DirectXException)
            {
                return;
            }
        }

        applicationDevice.Acquire();
        grabData.Enabled = true;
        UpdateUI();
    }

    
    
    
    private void btnCancel_Click(object sender, System.EventArgs e)
    {
        FreeDirectInput();
        this.Close();
    }

    
    
    
    private void KeyboardForm_Activated(object sender, System.EventArgs e)
    {
        if (null != applicationDevice)
        {
            try{applicationDevice.Acquire();}
            catch (DirectXException){}
        }
    }

    
    
    
    private void GrabData_Tick(object sender, System.EventArgs e)
    {
        // Update the input device every timer message.
        bool isImmediate = immediate.Checked;

        if (true == isImmediate)
        {
            if (!ReadImmediateData())
            {
                grabData.Enabled = false;
                MessageBox.Show("Error reading input state. The sample will now exit.","Keyboard");
                this.Close();
            }
        }
        else
        {
            if (!ReadBufferedData())
            {
                grabData.Enabled = false;
                MessageBox.Show("Error reading input state. The sample will now exit.","Keyboard");
                this.Close();
            }
        }       
    }

    
    
    
    private void KeyboardForm_Closing(object sender, System.ComponentModel.CancelEventArgs e)
    {
        grabData.Enabled = false;
        FreeDirectInput();
    }

    
    
    
    private void FreeDirectInput()
    {
        // Unacquire the device one last time just in case 
        // the app tried to exit while the device is still acquired.
        if (null != applicationDevice) 
        {
            applicationDevice.Unacquire();
            applicationDevice.Dispose();
            applicationDevice = null;
        }
    }

    
    
    
    ///<summary>
    /// Enables/disables the UI, and sets the dialog behavior text based on the UI
    ///</summary>
    private void UpdateUI()
    {
        string strExpected = String.Empty;
        bool isExclusive;
        bool isForeground;
        bool isImmediate;
        bool DisableWindowsKey;

        // Determine where the buffer would like to be allocated.
        isExclusive = exclusive.Checked;
        isForeground = foreground.Checked;
        isImmediate = immediate.Checked;
        DisableWindowsKey = windowsKey.Checked;

        if (null != applicationDevice)
        {
            createDevice.Text ="Release Device";
            dataLabel.Text = String.Empty;

            exclusive.Enabled = false;
            foreground.Enabled = false;
            nonexclusive.Enabled = false;
            background.Enabled = false;
            immediate.Enabled = false;
            buffered.Enabled = false;
            windowsKey.Enabled = false;
        }
        else
        {
            createDevice.Text ="&Create Device";
            dataLabel.Text = "Device not created. Choose settings and click 'Create Device' then type to see results";

            exclusive.Enabled = true;
            foreground.Enabled = true;
            nonexclusive.Enabled = true;
            background.Enabled = true;
            immediate.Enabled = true;
            buffered.Enabled = true;          

            if (!isExclusive && isForeground)
                windowsKey.Enabled = true;
            else
                windowsKey.Enabled = false;
        }

        // Figure what the user should expect based on the dialog choice.
        if (!isForeground && isExclusive)
        {
            strExpected = "For security reasons, background exclusive " +
                "keyboard access is not allowed.\n\n";
        }
        else
        {
            if (isForeground)
            {
                strExpected =  "Foreground cooperative level means that the " +
                    "application has access to data only when in the " +
                    "foreground or, in other words, has the input focus. " +
                    "If the application moves to the background, " +
                    "the device is automatically unacquired, or made " +
                    "unavailable.\n\n";
            }
            else
            {
                strExpected = "Background cooperative level really means " +
                    "foreground and background. A device with a " +
                    "background cooperative level can be acquired " +
                    "and used by an application at any time.\n\n";
            }

            if (isExclusive)
            {
                strExpected += "Exclusive mode prevents other applications from " +
                    "also acquiring the device exclusively. The fact " +
                    "that your application is using a device at the " +
                    "exclusive level does not mean that other " +
                    "applications cannot get data from the device. " +
                    "When an application has exclusive access to the " +
                    "keyboard, DirectInput suppresses all keyboard " +
                    "messages including the Windows key except " +
                    "CTRL+ALT+DEL and ALT+TAB\n\n";
            }
            else
            {
                strExpected += "Nonexclusive mode means that other applications " +
                    "can acquire device in exclusive or nonexclusive mode. ";

                if (true == DisableWindowsKey)
                {
                    strExpected += "The Windows key will also be disabled so that " +
                        "users cannot inadvertently break out of the " +
                        "application. ";
                }
                strExpected += "\n\n";
            }
            if (true ==  isImmediate)
            {
                strExpected += "immediate data is a snapshot of the current " +
                    "state of a device. It provides no data about " +
                    "what has happened with the device since the " +
                    "last call, apart from implicit information that " +
                    "you can derive by comparing the current state with " +
                    "the last one. Events in between calls are lost.\n\n";
            }
            else
            {
                strExpected += "Buffered data is a record of events that are stored " +
                    "until an application retrieves them. With buffered " +
                    "data, events are stored until you are ready to deal " +
                    "with them. If the buffer overflows, new data is lost.\n\n";                             
            }

            strExpected += "The sample will read the keyboard 12 times a second. " +
                "Typically an application would poll the keyboard " +
                "much faster than this, but this slow rate is simply " +
                "for the purposes of demonstration.";
        }
        behaviorLabel.Text = strExpected;
    }

    
    
    
    ///<summary>
    /// Read the input device's state when in buffered mode and display it.
    ///</summary>
    bool ReadBufferedData()
    {
        BufferedDataCollection dataCollection = null;
        string textNew = String.Empty;
    
        if (null == applicationDevice) 
            return true;
    
        InputException ie = null;
        try
        {
            dataCollection = applicationDevice.GetBufferedData();
        }
        catch(InputException)
        {
            // We got an error 
            //
            // It means that continuous contact with the
            // device has been lost, either due to an external
            // interruption, or because the buffer overflowed
            // and some events were lost.
            //
            // Consequently, if a button was pressed at the time
            // the buffer overflowed or the connection was broken,
            // the corresponding "up" message might have been lost.
            //
            // But since our simple sample doesn't actually have
            // any state associated with button up or down events,
            // there is no state to reset.  (In a real game, ignoring
            // the buffer overflow would result in the game thinking
            // a key was held down when in fact it isn't; it's just
            // that the "up" event got lost because the buffer
            // overflowed.)
            //
            // If we want to be more clever, we could do a
            // GetBufferedData() and compare the current state
            // against the state we think the device is in,
            // and process all the states that are currently
            // different from our private state.
        
            bool loop = true;
            do
            {
                try
                {
                    applicationDevice.Acquire();
                }
                catch(InputLostException)
                {
                    loop = true;
                }
                catch(InputException inputException)
                {
                    ie = inputException;
                    loop = false;
                }
            }while (loop);

            // Update the dialog text.
            if (ie is OtherApplicationHasPriorityException || ie is NotAcquiredException) 
                dataLabel.Text = "Unacquired";

            // This
            // may occur when the app is minimized or in the process of 
            // switching, so just try again later.
            return true;
        }

        if (null != ie)
            return false;

        if (null == dataCollection)
            return true;

        // Study each of the buffer elements and process them.
        //
        // Since we really don't do anything, our "processing"
        // consists merely of squirting the name into our
        // local buffer.
        foreach (BufferedData d in dataCollection) 
        {
            // this will display then scan code of the key
            // plus a 'D' - meaning the key was pressed 
            //   or a 'U' - meaning the key was released
            textNew += String.Format("0x{0:X}", d.Offset);
            textNew += (0 != (d.Data & 0x80)) ? "D " : "U ";
        }

        // If nothing changed then don't repaint - avoid flicker.
        if (dataLabel.Text != textNew)
            dataLabel.Text = textNew;

        return true;
    }

    
    
    
    ///<summary>
    /// Read the input device's state when in immediate mode and display it.
    ///</summary>
    bool ReadImmediateData()
    {
        string textNew = String.Empty;
        KeyboardState state = null;

        if (null == applicationDevice) 
            return true;
    
        // Get the input's device state, and store it.
        InputException ie = null;
        try
        {
            state = applicationDevice.GetCurrentKeyboardState();
        }
        catch (DirectXException)
        {
            // DirectInput may be telling us that the input stream has been
            // interrupted.  We aren't tracking any state between polls, so
            // we don't have any special reset that needs to be done.
            // We just re-acquire and try again.
        
            // If input is lost then acquire and keep trying.
            
            bool loop = true;
            do
            {
                try
                {
                    applicationDevice.Acquire();
                }
                catch(InputLostException)
                {
                    loop = true;
                }
                catch(InputException inputException)
                {
                    ie = inputException;
                    loop = false;
                }
            }while (loop);

            // Update the dialog text 
            if (ie is OtherApplicationHasPriorityException || ie is NotAcquiredException) 
                dataLabel.Text = "Unacquired";

            // Exception may be OtherApplicationHasPriorityException or other exceptions.
            // This may occur when the app is minimized or in the process of 
            // switching, so just try again later.
            return true; 
        }
    
        // Make a string of the index values of the keys that are down.
        for (Key k = Key.Escape; k <= Key.MediaSelect; k++) 
        {
            if (state[k])
                textNew += k.ToString() + " ";
        }
    
        // If nothing changed then don't repaint - avoid flicker.
        if (dataLabel.Text != textNew)
            dataLabel.Text = textNew;
    
        return true;
    }
}
