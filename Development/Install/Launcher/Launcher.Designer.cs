namespace EpicGames
{
    partial class Launcher
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose( bool disposing )
        {
            if( disposing && ( components != null ) )
            {
                components.Dispose();
            }
            base.Dispose( disposing );
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( Launcher ) );
            this.DescLabel1 = new System.Windows.Forms.Label();
            this.buttonUpdateTimer = new System.Windows.Forms.Timer( this.components );
            this.ButtonUninstall = new EpicGames.ButtonPictureBox();
            this.ButtonInstall = new EpicGames.ButtonPictureBox();
            this.ButtonLaunch = new EpicGames.ButtonPictureBox();
            ( ( System.ComponentModel.ISupportInitialize )( this.ButtonUninstall ) ).BeginInit();
            ( ( System.ComponentModel.ISupportInitialize )( this.ButtonInstall ) ).BeginInit();
            ( ( System.ComponentModel.ISupportInitialize )( this.ButtonLaunch ) ).BeginInit();
            this.SuspendLayout();
            // 
            // DescLabel1
            // 
            this.DescLabel1.AccessibleDescription = null;
            this.DescLabel1.AccessibleName = null;
            resources.ApplyResources( this.DescLabel1, "DescLabel1" );
            this.DescLabel1.BackColor = System.Drawing.Color.Transparent;
            this.DescLabel1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.DescLabel1.ForeColor = System.Drawing.Color.White;
            this.DescLabel1.Name = "DescLabel1";
            // 
            // buttonUpdateTimer
            // 
            this.buttonUpdateTimer.Interval = 1000;
            // 
            // ButtonUninstall
            // 
            this.ButtonUninstall.AccessibleDescription = null;
            this.ButtonUninstall.AccessibleName = null;
            resources.ApplyResources( this.ButtonUninstall, "ButtonUninstall" );
            this.ButtonUninstall.BackColor = System.Drawing.Color.Transparent;
            this.ButtonUninstall.BackgroundImage = null;
            this.ButtonUninstall.ForeColor = System.Drawing.Color.White;
            this.ButtonUninstall.ImageLocation = null;
            this.ButtonUninstall.Name = "ButtonUninstall";
            this.ButtonUninstall.TabStop = false;
            this.ButtonUninstall.Click += new System.EventHandler( this.Uninstall_Click );
            // 
            // ButtonInstall
            // 
            this.ButtonInstall.AccessibleDescription = null;
            this.ButtonInstall.AccessibleName = null;
            resources.ApplyResources( this.ButtonInstall, "ButtonInstall" );
            this.ButtonInstall.BackColor = System.Drawing.Color.Transparent;
            this.ButtonInstall.BackgroundImage = null;
            this.ButtonInstall.ForeColor = System.Drawing.Color.White;
            this.ButtonInstall.ImageLocation = null;
            this.ButtonInstall.Name = "ButtonInstall";
            this.ButtonInstall.TabStop = false;
            this.ButtonInstall.Click += new System.EventHandler( this.Install_Click );
            // 
            // ButtonLaunch
            // 
            this.ButtonLaunch.AccessibleDescription = null;
            this.ButtonLaunch.AccessibleName = null;
            resources.ApplyResources( this.ButtonLaunch, "ButtonLaunch" );
            this.ButtonLaunch.BackColor = System.Drawing.Color.Transparent;
            this.ButtonLaunch.BackgroundImage = null;
            this.ButtonLaunch.ForeColor = System.Drawing.Color.White;
            this.ButtonLaunch.ImageLocation = null;
            this.ButtonLaunch.Name = "ButtonLaunch";
            this.ButtonLaunch.TabStop = false;
            this.ButtonLaunch.Click += new System.EventHandler( this.Launch_Click );
            // 
            // Launcher
            // 
            this.AccessibleDescription = null;
            this.AccessibleName = null;
            resources.ApplyResources( this, "$this" );
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = global::EpicGames.Properties.Resources.ut3_install;
            this.Controls.Add( this.DescLabel1 );
            this.Controls.Add( this.ButtonUninstall );
            this.Controls.Add( this.ButtonInstall );
            this.Controls.Add( this.ButtonLaunch );
            this.DoubleBuffered = true;
            this.Font = null;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "Launcher";
            ( ( System.ComponentModel.ISupportInitialize )( this.ButtonUninstall ) ).EndInit();
            ( ( System.ComponentModel.ISupportInitialize )( this.ButtonInstall ) ).EndInit();
            ( ( System.ComponentModel.ISupportInitialize )( this.ButtonLaunch ) ).EndInit();
            this.ResumeLayout( false );
            this.PerformLayout();

        }

        #endregion

		private ButtonPictureBox ButtonLaunch;
		private ButtonPictureBox ButtonInstall;
		private ButtonPictureBox ButtonUninstall;
        private System.Windows.Forms.Label DescLabel1;
		private System.Windows.Forms.Timer buttonUpdateTimer;
    }
}

