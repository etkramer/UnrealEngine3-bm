namespace CDKeyEntry
{
    partial class CDKeyEntry
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( CDKeyEntry ) );
            this.CDEntry0 = new System.Windows.Forms.TextBox();
            this.CDEntry1 = new System.Windows.Forms.TextBox();
            this.CDEntry2 = new System.Windows.Forms.TextBox();
            this.CDEntry3 = new System.Windows.Forms.TextBox();
            this.CDKeyGroup = new System.Windows.Forms.GroupBox();
            this.ValidateCDKey = new System.Windows.Forms.Button();
            this.MainDescription = new System.Windows.Forms.Label();
            this.ExitButton = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.BannerImage = new System.Windows.Forms.PictureBox();
            this.CDKeyGroup.SuspendLayout();
            ( ( System.ComponentModel.ISupportInitialize )( this.BannerImage ) ).BeginInit();
            this.SuspendLayout();
            // 
            // CDEntry0
            // 
            this.CDEntry0.AccessibleDescription = null;
            this.CDEntry0.AccessibleName = null;
            resources.ApplyResources( this.CDEntry0, "CDEntry0" );
            this.CDEntry0.BackgroundImage = null;
            this.CDEntry0.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.CDEntry0.Name = "CDEntry0";
            this.CDEntry0.TextChanged += new System.EventHandler( this.CDEntry0_TextChanged );
            // 
            // CDEntry1
            // 
            this.CDEntry1.AccessibleDescription = null;
            this.CDEntry1.AccessibleName = null;
            resources.ApplyResources( this.CDEntry1, "CDEntry1" );
            this.CDEntry1.BackgroundImage = null;
            this.CDEntry1.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.CDEntry1.Name = "CDEntry1";
            this.CDEntry1.TextChanged += new System.EventHandler( this.CDEntry0_TextChanged );
            // 
            // CDEntry2
            // 
            this.CDEntry2.AccessibleDescription = null;
            this.CDEntry2.AccessibleName = null;
            resources.ApplyResources( this.CDEntry2, "CDEntry2" );
            this.CDEntry2.BackgroundImage = null;
            this.CDEntry2.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.CDEntry2.Name = "CDEntry2";
            this.CDEntry2.TextChanged += new System.EventHandler( this.CDEntry0_TextChanged );
            // 
            // CDEntry3
            // 
            this.CDEntry3.AccessibleDescription = null;
            this.CDEntry3.AccessibleName = null;
            resources.ApplyResources( this.CDEntry3, "CDEntry3" );
            this.CDEntry3.BackgroundImage = null;
            this.CDEntry3.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.CDEntry3.Name = "CDEntry3";
            this.CDEntry3.TextChanged += new System.EventHandler( this.CDEntry0_TextChanged );
            // 
            // CDKeyGroup
            // 
            this.CDKeyGroup.AccessibleDescription = null;
            this.CDKeyGroup.AccessibleName = null;
            resources.ApplyResources( this.CDKeyGroup, "CDKeyGroup" );
            this.CDKeyGroup.BackgroundImage = null;
            this.CDKeyGroup.Controls.Add( this.CDEntry2 );
            this.CDKeyGroup.Controls.Add( this.CDEntry0 );
            this.CDKeyGroup.Controls.Add( this.CDEntry1 );
            this.CDKeyGroup.Controls.Add( this.CDEntry3 );
            this.CDKeyGroup.Font = null;
            this.CDKeyGroup.Name = "CDKeyGroup";
            this.CDKeyGroup.TabStop = false;
            // 
            // ValidateCDKey
            // 
            this.ValidateCDKey.AccessibleDescription = null;
            this.ValidateCDKey.AccessibleName = null;
            resources.ApplyResources( this.ValidateCDKey, "ValidateCDKey" );
            this.ValidateCDKey.BackgroundImage = null;
            this.ValidateCDKey.Name = "ValidateCDKey";
            this.ValidateCDKey.UseVisualStyleBackColor = true;
            this.ValidateCDKey.Click += new System.EventHandler( this.ValidateCDKey_Click );
            // 
            // MainDescription
            // 
            this.MainDescription.AccessibleDescription = null;
            this.MainDescription.AccessibleName = null;
            resources.ApplyResources( this.MainDescription, "MainDescription" );
            this.MainDescription.BackColor = System.Drawing.Color.White;
            this.MainDescription.Name = "MainDescription";
            // 
            // ExitButton
            // 
            this.ExitButton.AccessibleDescription = null;
            this.ExitButton.AccessibleName = null;
            resources.ApplyResources( this.ExitButton, "ExitButton" );
            this.ExitButton.BackgroundImage = null;
            this.ExitButton.Name = "ExitButton";
            this.ExitButton.UseVisualStyleBackColor = true;
            this.ExitButton.Click += new System.EventHandler( this.ExitButton_Click );
            // 
            // groupBox1
            // 
            this.groupBox1.AccessibleDescription = null;
            this.groupBox1.AccessibleName = null;
            resources.ApplyResources( this.groupBox1, "groupBox1" );
            this.groupBox1.BackgroundImage = null;
            this.groupBox1.Font = null;
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
            // 
            // groupBox2
            // 
            this.groupBox2.AccessibleDescription = null;
            this.groupBox2.AccessibleName = null;
            resources.ApplyResources( this.groupBox2, "groupBox2" );
            this.groupBox2.BackgroundImage = null;
            this.groupBox2.Font = null;
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // BannerImage
            // 
            this.BannerImage.AccessibleDescription = null;
            this.BannerImage.AccessibleName = null;
            resources.ApplyResources( this.BannerImage, "BannerImage" );
            this.BannerImage.BackgroundImage = null;
            this.BannerImage.Font = null;
            this.BannerImage.Image = global::CDKeyEntry.Properties.Resources.banner;
            this.BannerImage.ImageLocation = null;
            this.BannerImage.InitialImage = null;
            this.BannerImage.Name = "BannerImage";
            this.BannerImage.TabStop = false;
            // 
            // CDKeyEntry
            // 
            this.AccessibleDescription = null;
            this.AccessibleName = null;
            resources.ApplyResources( this, "$this" );
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = null;
            this.Controls.Add( this.groupBox2 );
            this.Controls.Add( this.groupBox1 );
            this.Controls.Add( this.ExitButton );
            this.Controls.Add( this.ValidateCDKey );
            this.Controls.Add( this.CDKeyGroup );
            this.Controls.Add( this.MainDescription );
            this.Controls.Add( this.BannerImage );
            this.Font = null;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Name = "CDKeyEntry";
            this.CDKeyGroup.ResumeLayout( false );
            this.CDKeyGroup.PerformLayout();
            ( ( System.ComponentModel.ISupportInitialize )( this.BannerImage ) ).EndInit();
            this.ResumeLayout( false );
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox CDEntry0;
        private System.Windows.Forms.TextBox CDEntry1;
        private System.Windows.Forms.TextBox CDEntry2;
        private System.Windows.Forms.TextBox CDEntry3;
        private System.Windows.Forms.GroupBox CDKeyGroup;
        private System.Windows.Forms.Button ValidateCDKey;
        private System.Windows.Forms.PictureBox BannerImage;
        private System.Windows.Forms.Label MainDescription;
        private System.Windows.Forms.Button ExitButton;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
    }
}

