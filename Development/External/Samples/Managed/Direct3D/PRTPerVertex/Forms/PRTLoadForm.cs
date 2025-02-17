using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace PrtPerVertexSample
{
    /// <summary>
    /// Summary description for LoadPrtBufferForm.
    /// </summary>
    class PrtLoadForm : System.Windows.Forms.Form
    {
        private System.Windows.Forms.GroupBox resultsGB;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button simResultsBrowseButton;
        private System.Windows.Forms.Button inputMeshBrowseButton;
        private System.Windows.Forms.TextBox simResultsEdit;
        private System.Windows.Forms.TextBox inputMeshEdit;
        private System.Windows.Forms.Label simResultsText;
        private System.Windows.Forms.Label inputMeshText;
        private System.Windows.Forms.ToolTip toolTip;
        private System.ComponentModel.IContainer components;

        public PrtLoadForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            string mesh;

            if(!PrtOptions.GlobalOptions.IsAdaptive)
                mesh = PrtOptions.GlobalOptions.InputMesh;
            else
                mesh = PrtOptions.GlobalOptions.OutputMesh;
            inputMeshEdit.Text = mesh;
            inputMeshEdit.SelectionStart = 0;
            inputMeshEdit.SelectionLength = inputMeshEdit.Text.Length;

            simResultsEdit.Text = PrtOptions.GlobalOptions.ResultsFileName;
            simResultsEdit.SelectionStart = 0;
            simResultsEdit.SelectionLength = simResultsEdit.Text.Length;

            Directory.SetCurrentDirectory( PrtOptions.GlobalOptions.InitialDir );
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
            this.resultsGB = new System.Windows.Forms.GroupBox();
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.simResultsBrowseButton = new System.Windows.Forms.Button();
            this.inputMeshBrowseButton = new System.Windows.Forms.Button();
            this.simResultsEdit = new System.Windows.Forms.TextBox();
            this.inputMeshEdit = new System.Windows.Forms.TextBox();
            this.simResultsText = new System.Windows.Forms.Label();
            this.inputMeshText = new System.Windows.Forms.Label();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.resultsGB.SuspendLayout();
            this.SuspendLayout();
            // 
            // resultsGB
            // 
            this.resultsGB.Controls.Add(this.okButton);
            this.resultsGB.Controls.Add(this.cancelButton);
            this.resultsGB.Controls.Add(this.simResultsBrowseButton);
            this.resultsGB.Controls.Add(this.inputMeshBrowseButton);
            this.resultsGB.Controls.Add(this.simResultsEdit);
            this.resultsGB.Controls.Add(this.inputMeshEdit);
            this.resultsGB.Controls.Add(this.simResultsText);
            this.resultsGB.Controls.Add(this.inputMeshText);
            this.resultsGB.Location = new System.Drawing.Point(8, 8);
            this.resultsGB.Name = "resultsGB";
            this.resultsGB.Size = new System.Drawing.Size(392, 120);
            this.resultsGB.TabIndex = 0;
            this.resultsGB.TabStop = false;
            this.resultsGB.Text = "View the results from a previously saved Prt simulation";
            // 
            // okButton
            // 
            this.okButton.Location = new System.Drawing.Point(24, 88);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(72, 20);
            this.okButton.TabIndex = 7;
            this.okButton.Text = "OK";
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(296, 88);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(72, 20);
            this.cancelButton.TabIndex = 6;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // simResultsBrowseButton
            // 
            this.simResultsBrowseButton.Location = new System.Drawing.Point(296, 56);
            this.simResultsBrowseButton.Name = "simResultsBrowseButton";
            this.simResultsBrowseButton.Size = new System.Drawing.Size(72, 20);
            this.simResultsBrowseButton.TabIndex = 5;
            this.simResultsBrowseButton.Text = "Browse";
            this.simResultsBrowseButton.Click += new System.EventHandler(this.simResultsBrowseButton_Click);
            // 
            // inputMeshBrowseButton
            // 
            this.inputMeshBrowseButton.Location = new System.Drawing.Point(296, 32);
            this.inputMeshBrowseButton.Name = "inputMeshBrowseButton";
            this.inputMeshBrowseButton.Size = new System.Drawing.Size(72, 20);
            this.inputMeshBrowseButton.TabIndex = 4;
            this.inputMeshBrowseButton.Text = "Browse";
            this.inputMeshBrowseButton.Click += new System.EventHandler(this.inputMeshBrowseButton_Click);
            // 
            // simResultsEdit
            // 
            this.simResultsEdit.Location = new System.Drawing.Point(128, 56);
            this.simResultsEdit.Name = "simResultsEdit";
            this.simResultsEdit.Size = new System.Drawing.Size(152, 20);
            this.simResultsEdit.TabIndex = 3;
            this.simResultsEdit.Text = "Sample edit box";
            this.toolTip.SetToolTip(this.simResultsEdit, "Name of the simulator results file");
            // 
            // inputMeshEdit
            // 
            this.inputMeshEdit.Location = new System.Drawing.Point(128, 32);
            this.inputMeshEdit.Name = "inputMeshEdit";
            this.inputMeshEdit.Size = new System.Drawing.Size(152, 20);
            this.inputMeshEdit.TabIndex = 2;
            this.inputMeshEdit.Text = "Sample edit box";
            this.toolTip.SetToolTip(this.inputMeshEdit, "Name of the mesh file");
            // 
            // simResultsText
            // 
            this.simResultsText.Location = new System.Drawing.Point(16, 56);
            this.simResultsText.Name = "simResultsText";
            this.simResultsText.Size = new System.Drawing.Size(112, 16);
            this.simResultsText.TabIndex = 1;
            this.simResultsText.Text = "Simulator results file: ";
            this.simResultsText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // inputMeshText
            // 
            this.inputMeshText.Location = new System.Drawing.Point(72, 32);
            this.inputMeshText.Name = "inputMeshText";
            this.inputMeshText.Size = new System.Drawing.Size(56, 16);
            this.inputMeshText.TabIndex = 0;
            this.inputMeshText.Text = "Mesh file: ";
            this.inputMeshText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // toolTip
            // 
            this.toolTip.AutoPopDelay = 32000;
            this.toolTip.InitialDelay = 0;
            this.toolTip.ReshowDelay = 0;
            this.toolTip.ShowAlways = true;
            // 
            // PrtLoadForm
            // 
            this.AcceptButton = this.okButton;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(408, 134);
            this.Controls.Add(this.resultsGB);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "PrtLoadForm";
            this.Text = "Precomputed Radiance Transfer";
            this.resultsGB.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion

        #region Manually Written Code

        // Manual code
        public SimulatorOptions GetOptions()
        {
            return PrtOptions.GlobalOptions;
        }

        #endregion

        private void inputMeshBrowseButton_Click(object sender, System.EventArgs e)
        {
            string mesh;
            if( PrtOptions.GlobalOptions.IsAdaptive )
                mesh = PrtOptions.GlobalOptions.InputMesh;
            else
                mesh = PrtOptions.GlobalOptions.OutputMesh;

            // Display the OpenFileName dialog
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.InitialDirectory = PrtOptions.GlobalOptions.InitialDir;
            ofd.Filter = ".X Files (.x)|*.x|All Files|*.*";
            ofd.FilterIndex = 1;
            ofd.FileName = mesh;
            ofd.Title = "Open Mesh File";
            ofd.CheckFileExists = true;

            if(ofd.ShowDialog() == DialogResult.OK)
            {
                PrtOptions.GlobalOptions.InitialDir = System.IO.Path.GetDirectoryName(ofd.FileName);
                string meshFileName = System.IO.Path.GetFileName(ofd.FileName);
                meshFileName = System.IO.Path.GetFileNameWithoutExtension(meshFileName);

                string[] split = meshFileName.Split(new char[] { '_' });
                string resultFile = split[0];

                resultFile += "_prtresults";
                if(PrtOptions.GlobalOptions.IsSaveCompressedResults)
                    resultFile += ".pca";
                else
                    resultFile += ".prt";
                PrtOptions.GlobalOptions.ResultsFileName = resultFile;
                simResultsEdit.Text = PrtOptions.GlobalOptions.ResultsFileName;

                Directory.SetCurrentDirectory( PrtOptions.GlobalOptions.InitialDir );
                inputMeshEdit.Text = ofd.FileName;
                inputMeshEdit.SelectionStart = 0;
                inputMeshEdit.SelectionLength = inputMeshEdit.Text.Length;
            }
        }

        private void simResultsBrowseButton_Click(object sender, System.EventArgs e)
        {
            // Display the OpenFileName dialog
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.InitialDirectory = PrtOptions.GlobalOptions.InitialDir;
            ofd.Filter = "Prt buffer files (.pca;*.prt)|*.pca;*.prt|Compressed Prt buffer files (.pca)|*.pca|Uncompressed Prt buffer files (.prt)|*.prt|All Files|*.*";
            ofd.FilterIndex = 1;
            ofd.FileName = PrtOptions.GlobalOptions.ResultsFileName;
            ofd.Title = "Load Results File";
            ofd.CheckFileExists = true;
            ofd.DefaultExt = "";

            if(ofd.ShowDialog() == DialogResult.OK)
            {
                PrtOptions.GlobalOptions.ResultsFileName = ofd.FileName;
                PrtOptions.GlobalOptions.InitialDir = System.IO.Path.GetDirectoryName(PrtOptions.GlobalOptions.ResultsFileName);

                Directory.SetCurrentDirectory( PrtOptions.GlobalOptions.InitialDir );
                simResultsEdit.Text = PrtOptions.GlobalOptions.ResultsFileName;
                simResultsEdit.SelectionStart = 0;
                simResultsEdit.SelectionLength = simResultsEdit.Text.Length;
            }
        }

        private void okButton_Click(object sender, System.EventArgs e)
        {
            PrtOptions.GlobalOptions.InputMesh = inputMeshEdit.Text;
            PrtOptions.GlobalOptions.ResultsFileName = simResultsEdit.Text;

            Directory.SetCurrentDirectory( PrtOptions.GlobalOptions.InitialDir );
            
            try
            {
                string results = Utility.FindMediaFile(PrtOptions.GlobalOptions.ResultsFileName);
            }
            catch(MediaNotFoundException)
            {
                System.Windows.Forms.MessageBox.Show("Couldn't find the simulator results file.  Run the simulator first to precompute the transfer vectors for the mesh.");
                return;
            }
            
            try
            {
                string mesh = Utility.FindMediaFile(PrtOptions.GlobalOptions.InputMesh);
            }
            catch(MediaNotFoundException)
            {
                System.Windows.Forms.MessageBox.Show("Couldn't find the mesh file.  Be sure the mesh file exists.");
                return;
            }

            PrtOptions.GlobalOptionsFile.SaveOptions(string.Empty);
            
            DialogResult = DialogResult.OK;
        }

        private void cancelButton_Click(object sender, System.EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
        }
    }
}
