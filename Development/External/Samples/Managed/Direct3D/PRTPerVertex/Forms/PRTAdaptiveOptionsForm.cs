using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace PrtPerVertexSample
{
    /// <summary>
    /// Summary description for AdaptivePrtSettingsForm.
    /// </summary>
    public class PrtAdaptiveOptionsForm : System.Windows.Forms.Form
    {
        private System.Windows.Forms.GroupBox robustMeshRefineGB;
        private System.Windows.Forms.TextBox rmrMinEdgeEdit;
        private System.Windows.Forms.Label rmrMaxSubDText;
        private System.Windows.Forms.Label rmrMinEdgeText;
        private System.Windows.Forms.CheckBox enableRobustMeshRefineCB;
        private System.Windows.Forms.GroupBox adaptiveDirectLightingGB;
        private System.Windows.Forms.TextBox dlMinEdgeEdit;
        private System.Windows.Forms.TextBox dlSubDThresholdEdit;
        private System.Windows.Forms.Label dlMaxSubDText;
        private System.Windows.Forms.Label dlMinEdgeText;
        private System.Windows.Forms.Label dlSubDThresholdText;
        private System.Windows.Forms.CheckBox enableAdaptiveDirectLightingCB;
        private System.Windows.Forms.GroupBox adaptiveBounceGB;
        private System.Windows.Forms.TextBox abMinEdgeEdit;
        private System.Windows.Forms.TextBox abSubDThresholdEdit;
        private System.Windows.Forms.Label abMaxSubDText;
        private System.Windows.Forms.Label abMinEdgeText;
        private System.Windows.Forms.Label abSubDThresholdText;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.CheckBox enableAdaptiveBounceCB;
        private System.Windows.Forms.NumericUpDown rmrMaxSubDUD;
        private System.Windows.Forms.NumericUpDown dlMaxSubDUD;
        private System.Windows.Forms.NumericUpDown abMaxSubDUD;
        private System.Windows.Forms.ToolTip toolTip;
        private System.ComponentModel.IContainer components;

        public PrtAdaptiveOptionsForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            enableRobustMeshRefineCB.Checked = PrtOptions.GlobalOptions.IsRobustMeshRefine;
            enableAdaptiveDirectLightingCB.Checked = PrtOptions.GlobalOptions.IsAdaptiveDL;
            enableAdaptiveBounceCB.Checked = PrtOptions.GlobalOptions.IsAdaptiveBounce;

            UpdateUI();


            rmrMaxSubDUD.Maximum = 10;
            rmrMaxSubDUD.Minimum = 1;
            rmrMaxSubDUD.Value = PrtOptions.GlobalOptions.RobustMeshRefineMaxSubdiv;


            dlMaxSubDUD.Maximum = 10;
            dlMaxSubDUD.Minimum = 1;
            dlMaxSubDUD.Value = PrtOptions.GlobalOptions.AdaptiveBounceMaxSubdiv;

            abMaxSubDUD.Maximum = 10;
            abMaxSubDUD.Minimum = 1;
            abMaxSubDUD.Value = PrtOptions.GlobalOptions.AdaptiveBounceMaxSubdiv;

            
            rmrMinEdgeEdit.Text = string.Format("{0:f6}", PrtOptions.GlobalOptions.RobustMeshRefineMinEdgeLength);
            dlMinEdgeEdit.Text = string.Format("{0:f6}", PrtOptions.GlobalOptions.AdaptiveDLMinEdgeLength);
            abMinEdgeEdit.Text = string.Format("{0:f6}", PrtOptions.GlobalOptions.AdaptiveBounceMinEdgeLength);
            dlSubDThresholdEdit.Text = string.Format("{0:f6}", PrtOptions.GlobalOptions.AdaptiveDLThreshold);
            abSubDThresholdEdit.Text = string.Format("{0:f6}", PrtOptions.GlobalOptions.AdaptiveBounceThreshold);
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose( bool disposing )
        {
            if( disposing )
            {
                if(components != null)
                {
                    components.Dispose();
                }
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
            this.robustMeshRefineGB = new System.Windows.Forms.GroupBox();
            this.rmrMaxSubDUD = new System.Windows.Forms.NumericUpDown();
            this.rmrMinEdgeEdit = new System.Windows.Forms.TextBox();
            this.rmrMaxSubDText = new System.Windows.Forms.Label();
            this.rmrMinEdgeText = new System.Windows.Forms.Label();
            this.enableRobustMeshRefineCB = new System.Windows.Forms.CheckBox();
            this.adaptiveDirectLightingGB = new System.Windows.Forms.GroupBox();
            this.dlMaxSubDUD = new System.Windows.Forms.NumericUpDown();
            this.dlMinEdgeEdit = new System.Windows.Forms.TextBox();
            this.dlSubDThresholdEdit = new System.Windows.Forms.TextBox();
            this.dlMaxSubDText = new System.Windows.Forms.Label();
            this.dlMinEdgeText = new System.Windows.Forms.Label();
            this.dlSubDThresholdText = new System.Windows.Forms.Label();
            this.enableAdaptiveDirectLightingCB = new System.Windows.Forms.CheckBox();
            this.adaptiveBounceGB = new System.Windows.Forms.GroupBox();
            this.abMinEdgeEdit = new System.Windows.Forms.TextBox();
            this.abSubDThresholdEdit = new System.Windows.Forms.TextBox();
            this.abMaxSubDText = new System.Windows.Forms.Label();
            this.abMinEdgeText = new System.Windows.Forms.Label();
            this.abSubDThresholdText = new System.Windows.Forms.Label();
            this.enableAdaptiveBounceCB = new System.Windows.Forms.CheckBox();
            this.abMaxSubDUD = new System.Windows.Forms.NumericUpDown();
            this.okButton = new System.Windows.Forms.Button();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.robustMeshRefineGB.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.rmrMaxSubDUD)).BeginInit();
            this.adaptiveDirectLightingGB.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dlMaxSubDUD)).BeginInit();
            this.adaptiveBounceGB.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.abMaxSubDUD)).BeginInit();
            this.SuspendLayout();
            // 
            // robustMeshRefineGB
            // 
            this.robustMeshRefineGB.Controls.Add(this.rmrMaxSubDUD);
            this.robustMeshRefineGB.Controls.Add(this.rmrMinEdgeEdit);
            this.robustMeshRefineGB.Controls.Add(this.rmrMaxSubDText);
            this.robustMeshRefineGB.Controls.Add(this.rmrMinEdgeText);
            this.robustMeshRefineGB.Controls.Add(this.enableRobustMeshRefineCB);
            this.robustMeshRefineGB.Location = new System.Drawing.Point(8, 16);
            this.robustMeshRefineGB.Name = "robustMeshRefineGB";
            this.robustMeshRefineGB.Size = new System.Drawing.Size(288, 96);
            this.robustMeshRefineGB.TabIndex = 0;
            this.robustMeshRefineGB.TabStop = false;
            this.robustMeshRefineGB.Text = "Robust mesh refine";
            // 
            // rmrMaxSubDUD
            // 
            this.rmrMaxSubDUD.Location = new System.Drawing.Point(160, 64);
            this.rmrMaxSubDUD.Name = "rmrMaxSubDUD";
            this.rmrMaxSubDUD.Size = new System.Drawing.Size(80, 20);
            this.rmrMaxSubDUD.TabIndex = 13;
            // 
            // rmrMinEdgeEdit
            // 
            this.rmrMinEdgeEdit.Location = new System.Drawing.Point(160, 40);
            this.rmrMinEdgeEdit.Name = "rmrMinEdgeEdit";
            this.rmrMinEdgeEdit.Size = new System.Drawing.Size(80, 20);
            this.rmrMinEdgeEdit.TabIndex = 12;
            this.rmrMinEdgeEdit.Text = "Sample edit box";
            // 
            // rmrMaxSubDText
            // 
            this.rmrMaxSubDText.Location = new System.Drawing.Point(32, 64);
            this.rmrMaxSubDText.Name = "rmrMaxSubDText";
            this.rmrMaxSubDText.Size = new System.Drawing.Size(120, 16);
            this.rmrMaxSubDText.TabIndex = 10;
            this.rmrMaxSubDText.Text = "Max subdivision level:";
            this.rmrMaxSubDText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // rmrMinEdgeText
            // 
            this.rmrMinEdgeText.Location = new System.Drawing.Point(64, 40);
            this.rmrMinEdgeText.Name = "rmrMinEdgeText";
            this.rmrMinEdgeText.Size = new System.Drawing.Size(88, 16);
            this.rmrMinEdgeText.TabIndex = 9;
            this.rmrMinEdgeText.Text = "Min edge length:";
            this.rmrMinEdgeText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // enableRobustMeshRefineCB
            // 
            this.enableRobustMeshRefineCB.Location = new System.Drawing.Point(16, 16);
            this.enableRobustMeshRefineCB.Name = "enableRobustMeshRefineCB";
            this.enableRobustMeshRefineCB.Size = new System.Drawing.Size(160, 16);
            this.enableRobustMeshRefineCB.TabIndex = 0;
            this.enableRobustMeshRefineCB.Text = "Enable robust mesh refine";
            this.enableRobustMeshRefineCB.CheckedChanged += new System.EventHandler(this.enableRobustMeshRefineCB_CheckedChanged);
            // 
            // adaptiveDirectLightingGB
            // 
            this.adaptiveDirectLightingGB.Controls.Add(this.dlMaxSubDUD);
            this.adaptiveDirectLightingGB.Controls.Add(this.dlMinEdgeEdit);
            this.adaptiveDirectLightingGB.Controls.Add(this.dlSubDThresholdEdit);
            this.adaptiveDirectLightingGB.Controls.Add(this.dlMaxSubDText);
            this.adaptiveDirectLightingGB.Controls.Add(this.dlMinEdgeText);
            this.adaptiveDirectLightingGB.Controls.Add(this.dlSubDThresholdText);
            this.adaptiveDirectLightingGB.Controls.Add(this.enableAdaptiveDirectLightingCB);
            this.adaptiveDirectLightingGB.Location = new System.Drawing.Point(8, 120);
            this.adaptiveDirectLightingGB.Name = "adaptiveDirectLightingGB";
            this.adaptiveDirectLightingGB.Size = new System.Drawing.Size(288, 120);
            this.adaptiveDirectLightingGB.TabIndex = 1;
            this.adaptiveDirectLightingGB.TabStop = false;
            this.adaptiveDirectLightingGB.Text = "IsAdaptive direct lighting";
            // 
            // dlMaxSubDUD
            // 
            this.dlMaxSubDUD.Location = new System.Drawing.Point(160, 88);
            this.dlMaxSubDUD.Name = "dlMaxSubDUD";
            this.dlMaxSubDUD.Size = new System.Drawing.Size(80, 20);
            this.dlMaxSubDUD.TabIndex = 14;
            // 
            // dlMinEdgeEdit
            // 
            this.dlMinEdgeEdit.Location = new System.Drawing.Point(160, 64);
            this.dlMinEdgeEdit.Name = "dlMinEdgeEdit";
            this.dlMinEdgeEdit.Size = new System.Drawing.Size(80, 20);
            this.dlMinEdgeEdit.TabIndex = 6;
            this.dlMinEdgeEdit.Text = "Sample edit box";
            // 
            // dlSubDThresholdEdit
            // 
            this.dlSubDThresholdEdit.Location = new System.Drawing.Point(160, 40);
            this.dlSubDThresholdEdit.Name = "dlSubDThresholdEdit";
            this.dlSubDThresholdEdit.Size = new System.Drawing.Size(80, 20);
            this.dlSubDThresholdEdit.TabIndex = 5;
            this.dlSubDThresholdEdit.Text = "Sample edit box";
            // 
            // dlMaxSubDText
            // 
            this.dlMaxSubDText.Location = new System.Drawing.Point(32, 88);
            this.dlMaxSubDText.Name = "dlMaxSubDText";
            this.dlMaxSubDText.Size = new System.Drawing.Size(120, 16);
            this.dlMaxSubDText.TabIndex = 4;
            this.dlMaxSubDText.Text = "Max subdivision level:";
            this.dlMaxSubDText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // dlMinEdgeText
            // 
            this.dlMinEdgeText.Location = new System.Drawing.Point(64, 64);
            this.dlMinEdgeText.Name = "dlMinEdgeText";
            this.dlMinEdgeText.Size = new System.Drawing.Size(88, 16);
            this.dlMinEdgeText.TabIndex = 3;
            this.dlMinEdgeText.Text = "Min edge length:";
            this.dlMinEdgeText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // dlSubDThresholdText
            // 
            this.dlSubDThresholdText.Location = new System.Drawing.Point(32, 40);
            this.dlSubDThresholdText.Name = "dlSubDThresholdText";
            this.dlSubDThresholdText.Size = new System.Drawing.Size(120, 16);
            this.dlSubDThresholdText.TabIndex = 2;
            this.dlSubDThresholdText.Text = "Subdivision threshold:";
            this.dlSubDThresholdText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // enableAdaptiveDirectLightingCB
            // 
            this.enableAdaptiveDirectLightingCB.Location = new System.Drawing.Point(16, 16);
            this.enableAdaptiveDirectLightingCB.Name = "enableAdaptiveDirectLightingCB";
            this.enableAdaptiveDirectLightingCB.Size = new System.Drawing.Size(176, 16);
            this.enableAdaptiveDirectLightingCB.TabIndex = 1;
            this.enableAdaptiveDirectLightingCB.Text = "Enable adaptive direct lighting";
            this.enableAdaptiveDirectLightingCB.CheckedChanged += new System.EventHandler(this.enableAdaptiveDirectLightingCB_CheckedChanged);
            // 
            // adaptiveBounceGB
            // 
            this.adaptiveBounceGB.Controls.Add(this.abMinEdgeEdit);
            this.adaptiveBounceGB.Controls.Add(this.abSubDThresholdEdit);
            this.adaptiveBounceGB.Controls.Add(this.abMaxSubDText);
            this.adaptiveBounceGB.Controls.Add(this.abMinEdgeText);
            this.adaptiveBounceGB.Controls.Add(this.abSubDThresholdText);
            this.adaptiveBounceGB.Controls.Add(this.enableAdaptiveBounceCB);
            this.adaptiveBounceGB.Controls.Add(this.abMaxSubDUD);
            this.adaptiveBounceGB.Location = new System.Drawing.Point(8, 248);
            this.adaptiveBounceGB.Name = "adaptiveBounceGB";
            this.adaptiveBounceGB.Size = new System.Drawing.Size(288, 120);
            this.adaptiveBounceGB.TabIndex = 2;
            this.adaptiveBounceGB.TabStop = false;
            this.adaptiveBounceGB.Text = "IsAdaptive bounce";
            // 
            // abMinEdgeEdit
            // 
            this.abMinEdgeEdit.Location = new System.Drawing.Point(160, 64);
            this.abMinEdgeEdit.Name = "abMinEdgeEdit";
            this.abMinEdgeEdit.Size = new System.Drawing.Size(80, 20);
            this.abMinEdgeEdit.TabIndex = 12;
            this.abMinEdgeEdit.Text = "Sample edit box";
            // 
            // abSubDThresholdEdit
            // 
            this.abSubDThresholdEdit.Location = new System.Drawing.Point(160, 40);
            this.abSubDThresholdEdit.Name = "abSubDThresholdEdit";
            this.abSubDThresholdEdit.Size = new System.Drawing.Size(80, 20);
            this.abSubDThresholdEdit.TabIndex = 11;
            this.abSubDThresholdEdit.Text = "Sample edit box";
            // 
            // abMaxSubDText
            // 
            this.abMaxSubDText.Location = new System.Drawing.Point(32, 88);
            this.abMaxSubDText.Name = "abMaxSubDText";
            this.abMaxSubDText.Size = new System.Drawing.Size(120, 16);
            this.abMaxSubDText.TabIndex = 10;
            this.abMaxSubDText.Text = "Max subdivision level:";
            this.abMaxSubDText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // abMinEdgeText
            // 
            this.abMinEdgeText.Location = new System.Drawing.Point(64, 64);
            this.abMinEdgeText.Name = "abMinEdgeText";
            this.abMinEdgeText.Size = new System.Drawing.Size(88, 16);
            this.abMinEdgeText.TabIndex = 9;
            this.abMinEdgeText.Text = "Min edge length:";
            this.abMinEdgeText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // abSubDThresholdText
            // 
            this.abSubDThresholdText.Location = new System.Drawing.Point(32, 40);
            this.abSubDThresholdText.Name = "abSubDThresholdText";
            this.abSubDThresholdText.Size = new System.Drawing.Size(120, 16);
            this.abSubDThresholdText.TabIndex = 8;
            this.abSubDThresholdText.Text = "Subdivision threshold:";
            this.abSubDThresholdText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // enableAdaptiveBounceCB
            // 
            this.enableAdaptiveBounceCB.Location = new System.Drawing.Point(16, 16);
            this.enableAdaptiveBounceCB.Name = "enableAdaptiveBounceCB";
            this.enableAdaptiveBounceCB.Size = new System.Drawing.Size(152, 16);
            this.enableAdaptiveBounceCB.TabIndex = 2;
            this.enableAdaptiveBounceCB.Text = "Enable adaptive bounce";
            this.enableAdaptiveBounceCB.CheckedChanged += new System.EventHandler(this.enableAdaptiveBounceCB_CheckedChanged);
            // 
            // abMaxSubDUD
            // 
            this.abMaxSubDUD.Location = new System.Drawing.Point(160, 88);
            this.abMaxSubDUD.Name = "abMaxSubDUD";
            this.abMaxSubDUD.Size = new System.Drawing.Size(80, 20);
            this.abMaxSubDUD.TabIndex = 15;
            this.abMaxSubDUD.Value = new System.Decimal(new int[] {
                                                                      1,
                                                                      0,
                                                                      0,
                                                                      0});
            // 
            // okButton
            // 
            this.okButton.Location = new System.Drawing.Point(232, 376);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(64, 24);
            this.okButton.TabIndex = 3;
            this.okButton.Text = "OK";
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // toolTip
            // 
            this.toolTip.AutoPopDelay = 32000;
            this.toolTip.InitialDelay = 0;
            this.toolTip.ReshowDelay = 0;
            this.toolTip.ShowAlways = true;
            // 
            // PrtAdaptiveOptionsForm
            // 
            this.AcceptButton = this.okButton;
            this.ClientSize = new System.Drawing.Size(304, 406);
            this.Controls.Add(this.okButton);
            this.Controls.Add(this.adaptiveBounceGB);
            this.Controls.Add(this.robustMeshRefineGB);
            this.Controls.Add(this.adaptiveDirectLightingGB);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "PrtAdaptiveOptionsForm";
            this.Text = "IsAdaptive Prt Setttings";
            this.robustMeshRefineGB.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.rmrMaxSubDUD)).EndInit();
            this.adaptiveDirectLightingGB.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dlMaxSubDUD)).EndInit();
            this.adaptiveBounceGB.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.abMaxSubDUD)).EndInit();
            this.ResumeLayout(false);

        }
        #endregion

        private void okButton_Click(object sender, System.EventArgs e)
        {
            PrtOptions.GlobalOptions.IsRobustMeshRefine = enableRobustMeshRefineCB.Checked;
            PrtOptions.GlobalOptions.IsAdaptiveDL = enableAdaptiveDirectLightingCB.Checked;
            PrtOptions.GlobalOptions.IsAdaptiveBounce = enableAdaptiveBounceCB.Checked;

            PrtOptions.GlobalOptions.RobustMeshRefineMaxSubdiv = Decimal.ToInt32(rmrMaxSubDUD.Value);
            PrtOptions.GlobalOptions.AdaptiveDLMaxSubdiv = Decimal.ToInt32(dlMaxSubDUD.Value);
            PrtOptions.GlobalOptions.AdaptiveBounceMaxSubdiv = Decimal.ToInt32(abMaxSubDUD.Value);

            PrtOptions.GlobalOptions.RobustMeshRefineMinEdgeLength = float.Parse(rmrMinEdgeEdit.Text);
            PrtOptions.GlobalOptions.AdaptiveDLMinEdgeLength = float.Parse(dlMinEdgeEdit.Text);
            PrtOptions.GlobalOptions.AdaptiveBounceMinEdgeLength = float.Parse(abMinEdgeEdit.Text);
            PrtOptions.GlobalOptions.AdaptiveDLThreshold = float.Parse(dlSubDThresholdEdit.Text);
            PrtOptions.GlobalOptions.AdaptiveBounceThreshold = float.Parse(abSubDThresholdEdit.Text);

            DialogResult = DialogResult.OK;
        }

        private void enableRobustMeshRefineCB_CheckedChanged(object sender, System.EventArgs e)
        {
            PrtOptions.GlobalOptions.IsRobustMeshRefine = enableRobustMeshRefineCB.Checked;
            UpdateUI();
        }

        private void enableAdaptiveDirectLightingCB_CheckedChanged(object sender, System.EventArgs e)
        {
            PrtOptions.GlobalOptions.IsAdaptiveDL = enableAdaptiveDirectLightingCB.Checked;
            UpdateUI();
        }

        private void enableAdaptiveBounceCB_CheckedChanged(object sender, System.EventArgs e)
        {
            PrtOptions.GlobalOptions.IsAdaptiveBounce = enableAdaptiveBounceCB.Checked;
            UpdateUI();
        }

        #region Manually Written Code

        void UpdateUI()
        {
            rmrMinEdgeEdit.Enabled = 
                rmrMinEdgeText.Enabled = 
                rmrMaxSubDUD.Enabled = 
                rmrMaxSubDText.Enabled =
                PrtOptions.GlobalOptions.IsRobustMeshRefine;

            dlMaxSubDUD.Enabled = 
                dlMaxSubDText.Enabled = 
                dlMinEdgeEdit.Enabled = 
                dlMinEdgeText.Enabled = 
                dlSubDThresholdEdit.Enabled = 
                dlSubDThresholdText.Enabled = 
                PrtOptions.GlobalOptions.IsAdaptiveDL;

            abMaxSubDUD.Enabled = 
                abMaxSubDText.Enabled = 
                abMinEdgeEdit.Enabled = 
                abMinEdgeText.Enabled = 
                abSubDThresholdEdit.Enabled = 
                abSubDThresholdText.Enabled = 
                PrtOptions.GlobalOptions.IsAdaptiveBounce;
        }

        #endregion
    }
}
