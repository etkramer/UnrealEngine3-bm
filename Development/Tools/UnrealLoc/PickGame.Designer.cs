namespace UnrealLoc
{
    partial class PickGame
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
            this.Label_PickGame_Title = new System.Windows.Forms.Label();
            this.Button_Example = new System.Windows.Forms.Button();
            this.Button_GearGame = new System.Windows.Forms.Button();
            this.Button_UTGame = new System.Windows.Forms.Button();
            this.Button_Options = new System.Windows.Forms.Button();
            this.Button_Engine = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // Label_PickGame_Title
            // 
            this.Label_PickGame_Title.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.Label_PickGame_Title.Font = new System.Drawing.Font( "Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ( ( byte )( 0 ) ) );
            this.Label_PickGame_Title.Location = new System.Drawing.Point( -86, 9 );
            this.Label_PickGame_Title.Name = "Label_PickGame_Title";
            this.Label_PickGame_Title.Size = new System.Drawing.Size( 752, 33 );
            this.Label_PickGame_Title.TabIndex = 0;
            this.Label_PickGame_Title.Text = "Pick a game to process";
            this.Label_PickGame_Title.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // Button_Example
            // 
            this.Button_Example.AutoSize = true;
            this.Button_Example.Location = new System.Drawing.Point( 217, 68 );
            this.Button_Example.Name = "Button_Example";
            this.Button_Example.Size = new System.Drawing.Size( 150, 23 );
            this.Button_Example.TabIndex = 1;
            this.Button_Example.Text = "ExampleGame";
            this.Button_Example.UseVisualStyleBackColor = true;
            this.Button_Example.Click += new System.EventHandler( this.Button_Example_Click );
            // 
            // Button_GearGame
            // 
            this.Button_GearGame.AutoSize = true;
            this.Button_GearGame.Location = new System.Drawing.Point( 217, 98 );
            this.Button_GearGame.Name = "Button_GearGame";
            this.Button_GearGame.Size = new System.Drawing.Size( 150, 23 );
            this.Button_GearGame.TabIndex = 2;
            this.Button_GearGame.Text = "GearGame";
            this.Button_GearGame.UseVisualStyleBackColor = true;
            this.Button_GearGame.Click += new System.EventHandler( this.Button_GearGame_Click );
            // 
            // Button_UTGame
            // 
            this.Button_UTGame.AutoSize = true;
            this.Button_UTGame.Location = new System.Drawing.Point( 217, 128 );
            this.Button_UTGame.Name = "Button_UTGame";
            this.Button_UTGame.Size = new System.Drawing.Size( 150, 23 );
            this.Button_UTGame.TabIndex = 3;
            this.Button_UTGame.Text = "UTGame";
            this.Button_UTGame.UseVisualStyleBackColor = true;
            this.Button_UTGame.Click += new System.EventHandler( this.Button_UTGame_Click );
            // 
            // Button_Options
            // 
            this.Button_Options.Location = new System.Drawing.Point( 254, 184 );
            this.Button_Options.Name = "Button_Options";
            this.Button_Options.Size = new System.Drawing.Size( 75, 23 );
            this.Button_Options.TabIndex = 4;
            this.Button_Options.Text = "Options";
            this.Button_Options.UseVisualStyleBackColor = true;
            this.Button_Options.Click += new System.EventHandler( this.Button_Options_Click );
            // 
            // Button_Engine
            // 
            this.Button_Engine.AutoSize = true;
            this.Button_Engine.Location = new System.Drawing.Point( 217, 38 );
            this.Button_Engine.Name = "Button_Engine";
            this.Button_Engine.Size = new System.Drawing.Size( 150, 23 );
            this.Button_Engine.TabIndex = 5;
            this.Button_Engine.Text = "Engine";
            this.Button_Engine.UseVisualStyleBackColor = true;
            this.Button_Engine.Click += new System.EventHandler( this.Button_Engine_Click );
            // 
            // PickGame
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size( 581, 221 );
            this.Controls.Add( this.Button_Engine );
            this.Controls.Add( this.Button_Options );
            this.Controls.Add( this.Button_UTGame );
            this.Controls.Add( this.Button_GearGame );
            this.Controls.Add( this.Button_Example );
            this.Controls.Add( this.Label_PickGame_Title );
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "PickGame";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "PickGame";
            this.ResumeLayout( false );
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label Label_PickGame_Title;
        private System.Windows.Forms.Button Button_Example;
        private System.Windows.Forms.Button Button_GearGame;
        private System.Windows.Forms.Button Button_UTGame;
        private System.Windows.Forms.Button Button_Options;
        private System.Windows.Forms.Button Button_Engine;
    }
}