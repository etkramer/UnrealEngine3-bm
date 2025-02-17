using System;
using System.IO;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace PrtPerVertexSample
{
    class PrtOptionsForm : System.Windows.Forms.Form
    {
        private bool IsComboBoxSelChange = false;
        private bool IsShowToolTips = true;

        private System.Windows.Forms.GroupBox prtSettingsGB;
        private System.Windows.Forms.Button adaptiveSettingsButton;
        private System.Windows.Forms.CheckBox adaptiveCB;
        private System.Windows.Forms.CheckBox spectralCB;
        private System.Windows.Forms.TextBox lengthScaleEdit;
        private System.Windows.Forms.Label lengthScaleText;
        private System.Windows.Forms.Label numBouncesText;
        private System.Windows.Forms.Label numRaysText;
        private System.Windows.Forms.TrackBar orderSlider;
        private System.Windows.Forms.Label orderText;
        private System.Windows.Forms.Button inputBrowseButton;
        private System.Windows.Forms.TextBox inputMeshNameTB;
        private System.Windows.Forms.Label inputMeshText;
        private System.Windows.Forms.CheckBox subsurfCB;
        private System.Windows.Forms.GroupBox materialSettingsGB;
        private System.Windows.Forms.Label greenText;
        private System.Windows.Forms.Label redText;
        private System.Windows.Forms.ComboBox predefCombo;
        private System.Windows.Forms.TextBox absorptionBEdit;
        private System.Windows.Forms.TextBox absorptionGEdit;
        private System.Windows.Forms.TextBox absorptionREdit;
        private System.Windows.Forms.TextBox scatteringBEdit;
        private System.Windows.Forms.TextBox scatteringGEdit;
        private System.Windows.Forms.TextBox scatteringREdit;
        private System.Windows.Forms.TextBox reflectanceBEdit;
        private System.Windows.Forms.TextBox reflectanceGEdit;
        private System.Windows.Forms.TextBox refractionEdit;
        private System.Windows.Forms.TextBox reflectanceREdit;
        private System.Windows.Forms.Label absorptionText;
        private System.Windows.Forms.Label scatteringText;
        private System.Windows.Forms.Label reflectanceText;
        private System.Windows.Forms.Label refractionText;
        private System.Windows.Forms.Label predefText;
        private System.Windows.Forms.Label blueText;
        private System.Windows.Forms.GroupBox outputSettingsGB;
        private System.Windows.Forms.RadioButton meshSaveTextRB;
        private System.Windows.Forms.RadioButton meshSaveBinaryRB;
        private System.Windows.Forms.CheckBox compressedCB;
        private System.Windows.Forms.Button outputBrowse;
        private System.Windows.Forms.Button outputMeshBrowseButton;
        private System.Windows.Forms.TextBox outputMeshEdit;
        private System.Windows.Forms.Label outputMeshText;
        private System.Windows.Forms.TextBox outputEdit;
        private System.Windows.Forms.Label outputText;
        private System.Windows.Forms.Button goButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.CheckBox showTooltipsCB;
        private System.Windows.Forms.Label minStatic;
        private System.Windows.Forms.Label maxStatic;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.MenuItem settingsMM;
        private System.Windows.Forms.MenuItem resetMI;
        private System.Windows.Forms.NumericUpDown numBouncesUD;
        private System.Windows.Forms.NumericUpDown numRaysUD;
        private System.Windows.Forms.ToolTip toolTip;
        private System.ComponentModel.IContainer components;

        public PrtOptionsForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            
            UpdateControlsWithSettings();
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
            this.mainMenu = new System.Windows.Forms.MainMenu();
            this.settingsMM = new System.Windows.Forms.MenuItem();
            this.resetMI = new System.Windows.Forms.MenuItem();
            this.prtSettingsGB = new System.Windows.Forms.GroupBox();
            this.numBouncesUD = new System.Windows.Forms.NumericUpDown();
            this.maxStatic = new System.Windows.Forms.Label();
            this.minStatic = new System.Windows.Forms.Label();
            this.adaptiveSettingsButton = new System.Windows.Forms.Button();
            this.adaptiveCB = new System.Windows.Forms.CheckBox();
            this.spectralCB = new System.Windows.Forms.CheckBox();
            this.lengthScaleEdit = new System.Windows.Forms.TextBox();
            this.lengthScaleText = new System.Windows.Forms.Label();
            this.numBouncesText = new System.Windows.Forms.Label();
            this.numRaysText = new System.Windows.Forms.Label();
            this.orderSlider = new System.Windows.Forms.TrackBar();
            this.orderText = new System.Windows.Forms.Label();
            this.inputBrowseButton = new System.Windows.Forms.Button();
            this.inputMeshNameTB = new System.Windows.Forms.TextBox();
            this.inputMeshText = new System.Windows.Forms.Label();
            this.subsurfCB = new System.Windows.Forms.CheckBox();
            this.numRaysUD = new System.Windows.Forms.NumericUpDown();
            this.materialSettingsGB = new System.Windows.Forms.GroupBox();
            this.greenText = new System.Windows.Forms.Label();
            this.redText = new System.Windows.Forms.Label();
            this.predefCombo = new System.Windows.Forms.ComboBox();
            this.absorptionBEdit = new System.Windows.Forms.TextBox();
            this.absorptionGEdit = new System.Windows.Forms.TextBox();
            this.absorptionREdit = new System.Windows.Forms.TextBox();
            this.scatteringBEdit = new System.Windows.Forms.TextBox();
            this.scatteringGEdit = new System.Windows.Forms.TextBox();
            this.scatteringREdit = new System.Windows.Forms.TextBox();
            this.reflectanceBEdit = new System.Windows.Forms.TextBox();
            this.reflectanceGEdit = new System.Windows.Forms.TextBox();
            this.refractionEdit = new System.Windows.Forms.TextBox();
            this.reflectanceREdit = new System.Windows.Forms.TextBox();
            this.absorptionText = new System.Windows.Forms.Label();
            this.scatteringText = new System.Windows.Forms.Label();
            this.reflectanceText = new System.Windows.Forms.Label();
            this.refractionText = new System.Windows.Forms.Label();
            this.predefText = new System.Windows.Forms.Label();
            this.blueText = new System.Windows.Forms.Label();
            this.outputSettingsGB = new System.Windows.Forms.GroupBox();
            this.meshSaveTextRB = new System.Windows.Forms.RadioButton();
            this.meshSaveBinaryRB = new System.Windows.Forms.RadioButton();
            this.compressedCB = new System.Windows.Forms.CheckBox();
            this.outputBrowse = new System.Windows.Forms.Button();
            this.outputMeshBrowseButton = new System.Windows.Forms.Button();
            this.outputMeshEdit = new System.Windows.Forms.TextBox();
            this.outputMeshText = new System.Windows.Forms.Label();
            this.outputEdit = new System.Windows.Forms.TextBox();
            this.outputText = new System.Windows.Forms.Label();
            this.goButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.showTooltipsCB = new System.Windows.Forms.CheckBox();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.prtSettingsGB.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numBouncesUD)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.orderSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numRaysUD)).BeginInit();
            this.materialSettingsGB.SuspendLayout();
            this.outputSettingsGB.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                     this.settingsMM});
            // 
            // settingsMM
            // 
            this.settingsMM.Index = 0;
            this.settingsMM.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                       this.resetMI});
            this.settingsMM.Text = "Settings";
            // 
            // resetMI
            // 
            this.resetMI.Index = 0;
            this.resetMI.Text = "Reset";
            this.resetMI.Click += new System.EventHandler(this.resetMI_Click);
            // 
            // prtSettingsGB
            // 
            this.prtSettingsGB.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
                | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.prtSettingsGB.Controls.Add(this.numBouncesUD);
            this.prtSettingsGB.Controls.Add(this.maxStatic);
            this.prtSettingsGB.Controls.Add(this.minStatic);
            this.prtSettingsGB.Controls.Add(this.adaptiveSettingsButton);
            this.prtSettingsGB.Controls.Add(this.adaptiveCB);
            this.prtSettingsGB.Controls.Add(this.spectralCB);
            this.prtSettingsGB.Controls.Add(this.lengthScaleEdit);
            this.prtSettingsGB.Controls.Add(this.lengthScaleText);
            this.prtSettingsGB.Controls.Add(this.numBouncesText);
            this.prtSettingsGB.Controls.Add(this.numRaysText);
            this.prtSettingsGB.Controls.Add(this.orderSlider);
            this.prtSettingsGB.Controls.Add(this.orderText);
            this.prtSettingsGB.Controls.Add(this.inputBrowseButton);
            this.prtSettingsGB.Controls.Add(this.inputMeshNameTB);
            this.prtSettingsGB.Controls.Add(this.inputMeshText);
            this.prtSettingsGB.Controls.Add(this.subsurfCB);
            this.prtSettingsGB.Controls.Add(this.numRaysUD);
            this.prtSettingsGB.Location = new System.Drawing.Point(16, 16);
            this.prtSettingsGB.Name = "prtSettingsGB";
            this.prtSettingsGB.Size = new System.Drawing.Size(480, 198);
            this.prtSettingsGB.TabIndex = 0;
            this.prtSettingsGB.TabStop = false;
            this.prtSettingsGB.Text = "Prt settings";
            // 
            // numBouncesUD
            // 
            this.numBouncesUD.Location = new System.Drawing.Point(120, 112);
            this.numBouncesUD.Name = "numBouncesUD";
            this.numBouncesUD.Size = new System.Drawing.Size(104, 20);
            this.numBouncesUD.TabIndex = 17;
            this.toolTip.SetToolTip(this.numBouncesUD, @"This controls the number of bounces simulated. If this is non-zero then inter-reflections are calculated. Inter-reflections are, for example, when a light shines on a red wall and bounces on a white wall. The white wall even though it contains no red in the material will reflect some red do to the bouncing of the light off the red wall.");
            // 
            // maxStatic
            // 
            this.maxStatic.Location = new System.Drawing.Point(408, 80);
            this.maxStatic.Name = "maxStatic";
            this.maxStatic.Size = new System.Drawing.Size(40, 20);
            this.maxStatic.TabIndex = 16;
            this.maxStatic.Text = "Max: 6";
            this.maxStatic.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // minStatic
            // 
            this.minStatic.Location = new System.Drawing.Point(248, 80);
            this.minStatic.Name = "minStatic";
            this.minStatic.Size = new System.Drawing.Size(40, 20);
            this.minStatic.TabIndex = 15;
            this.minStatic.Text = "Min: 2";
            this.minStatic.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // adaptiveSettingsButton
            // 
            this.adaptiveSettingsButton.Location = new System.Drawing.Point(288, 128);
            this.adaptiveSettingsButton.Name = "adaptiveSettingsButton";
            this.adaptiveSettingsButton.Size = new System.Drawing.Size(152, 24);
            this.adaptiveSettingsButton.TabIndex = 14;
            this.adaptiveSettingsButton.Text = "IsAdaptive Mesh Settings...";
            this.adaptiveSettingsButton.Click += new System.EventHandler(this.adaptiveSettingsButton_Click);
            // 
            // adaptiveCB
            // 
            this.adaptiveCB.Location = new System.Drawing.Point(264, 104);
            this.adaptiveCB.Name = "adaptiveCB";
            this.adaptiveCB.Size = new System.Drawing.Size(208, 24);
            this.adaptiveCB.TabIndex = 13;
            this.adaptiveCB.Text = "Enable IsAdaptive mesh tessellation";
            this.adaptiveCB.CheckedChanged += new System.EventHandler(this.adaptiveCB_CheckedChanged);
            // 
            // spectralCB
            // 
            this.spectralCB.Enabled = false;
            this.spectralCB.Location = new System.Drawing.Point(256, 160);
            this.spectralCB.Name = "spectralCB";
            this.spectralCB.Size = new System.Drawing.Size(176, 24);
            this.spectralCB.TabIndex = 12;
            this.spectralCB.Text = "3 color channels (RGB)";
            this.toolTip.SetToolTip(this.spectralCB, @"If checked then the simulator will process 3 channels: red, green, and blue and return order^2 spherical harmonic transfer coefficients for each of these channels in a single ID3DXBuffer* buffer. Otherwise it use values of only one channel (the red channel) and return the transfer coefficients for just that single channel. A single channel is useful for lighting environments that don't need to have the whole spectrum of light such as shadows");
            this.spectralCB.CheckedChanged += new System.EventHandler(this.spectralCB_CheckedChanged);
            // 
            // lengthScaleEdit
            // 
            this.lengthScaleEdit.Location = new System.Drawing.Point(120, 136);
            this.lengthScaleEdit.Name = "lengthScaleEdit";
            this.lengthScaleEdit.Size = new System.Drawing.Size(104, 20);
            this.lengthScaleEdit.TabIndex = 11;
            this.lengthScaleEdit.Text = "";
            this.toolTip.SetToolTip(this.lengthScaleEdit, "When subsurface scattering is used the object is mapped to a cube of length scale" +
                " mm per side. For example, if length scale is 10, then the object is mapped to a" +
                " 10mm x 10mm x 10mm cube.  The smaller the cube the more light penetrates the ob" +
                "ject.");
            // 
            // lengthScaleText
            // 
            this.lengthScaleText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.lengthScaleText.Location = new System.Drawing.Point(48, 136);
            this.lengthScaleText.Name = "lengthScaleText";
            this.lengthScaleText.Size = new System.Drawing.Size(72, 20);
            this.lengthScaleText.TabIndex = 7;
            this.lengthScaleText.Text = "Length scale:";
            this.lengthScaleText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.lengthScaleText, "When subsurface scattering is used the object is mapped to a cube of length scale" +
                " mm per side. For example, if length scale is 10, then the object is mapped to a" +
                " 10mm x 10mm x 10mm cube.  The smaller the cube the more light penetrates the ob" +
                "ject.");
            // 
            // numBouncesText
            // 
            this.numBouncesText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.numBouncesText.Location = new System.Drawing.Point(8, 112);
            this.numBouncesText.Name = "numBouncesText";
            this.numBouncesText.Size = new System.Drawing.Size(112, 20);
            this.numBouncesText.TabIndex = 6;
            this.numBouncesText.Text = "Number of bounces:";
            this.numBouncesText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.numBouncesText, @"This controls the number of bounces simulated. If this is non-zero then inter-reflections are calculated. Inter-reflections are, for example, when a light shines on a red wall and bounces on a white wall. The white wall even though it contains no red in the material will reflect some red do to the bouncing of the light off the red wall.");
            // 
            // numRaysText
            // 
            this.numRaysText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.numRaysText.Location = new System.Drawing.Point(32, 88);
            this.numRaysText.Name = "numRaysText";
            this.numRaysText.Size = new System.Drawing.Size(88, 20);
            this.numRaysText.TabIndex = 5;
            this.numRaysText.Text = "Number of rays:";
            this.numRaysText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.numRaysText, "This controls the number of rays to shoot at each sample. The more rays the more " +
                "accurate the final result will be, but it will increase time it takes to precomp" +
                "ute the transfer coefficients.");
            // 
            // orderSlider
            // 
            this.orderSlider.Location = new System.Drawing.Point(248, 48);
            this.orderSlider.Name = "orderSlider";
            this.orderSlider.Size = new System.Drawing.Size(200, 45);
            this.orderSlider.TabIndex = 4;
            this.toolTip.SetToolTip(this.orderSlider, @"This controls the number of spherical harmonic basis functions used. The simulator generates order^2 coefficients per channel. Higher order allows for higher frequency lighting environments which allow for sharper shadows with the tradeoff of more coefficients per vertex that need to be processed by the vertex shader. For convex objects (no shadows), 3rd order has very little approximation error.  For more detailed information, see ""Spherical Harmonic Lighting: The Gritty Details"" by Robin Green, GDC 2003 and ""An Efficient Representation of Irradiance Environment Maps"" by Ravi Ramamoorthi, and Pat Hanrahan, SIGGRAPH 2001.");
            // 
            // orderText
            // 
            this.orderText.Location = new System.Drawing.Point(16, 48);
            this.orderText.Name = "orderText";
            this.orderText.Size = new System.Drawing.Size(224, 20);
            this.orderText.TabIndex = 3;
            this.orderText.Text = "Order of spherical harmonic approximation:";
            this.orderText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.orderText, @"This controls the number of spherical harmonic basis functions used. The simulator generates order^2 coefficients per channel. Higher order allows for higher frequency lighting environments which allow for sharper shadows with the tradeoff of more coefficients per vertex that need to be processed by the vertex shader. For convex objects (no shadows), 3rd order has very little approximation error.  For more detailed information, see ""Spherical Harmonic Lighting: The Gritty Details"" by Robin Green, GDC 2003 and ""An Efficient Representation of Irradiance Environment Maps"" by Ravi Ramamoorthi, and Pat Hanrahan, SIGGRAPH 2001.");
            // 
            // inputBrowseButton
            // 
            this.inputBrowseButton.Location = new System.Drawing.Point(336, 24);
            this.inputBrowseButton.Name = "inputBrowseButton";
            this.inputBrowseButton.Size = new System.Drawing.Size(56, 20);
            this.inputBrowseButton.TabIndex = 2;
            this.inputBrowseButton.Text = "Browse";
            this.inputBrowseButton.Click += new System.EventHandler(this.inputBrowseButton_Click);
            // 
            // inputMeshNameTB
            // 
            this.inputMeshNameTB.Location = new System.Drawing.Point(88, 24);
            this.inputMeshNameTB.Name = "inputMeshNameTB";
            this.inputMeshNameTB.Size = new System.Drawing.Size(240, 20);
            this.inputMeshNameTB.TabIndex = 1;
            this.inputMeshNameTB.Text = "Sample edit box";
            this.toolTip.SetToolTip(this.inputMeshNameTB, @"This is the file that is loaded as a Mesh and passed into SphericalHarmonics.PrtSimulation so that it can compute and return spherical harmonic transfer coefficients for each vertex in the mesh. It returns these coefficients in a ID3DXBuffer*. This process takes some time and should be precomputed, however the results can be used in real time. For more detailed information, see ""Precomputed Radiance Transfer for Real-Time Rendering in Dynamic, Low-Frequency Lighting Environments"" by Peter-Pike Sloan, Jan Kautz, and John Snyder, SIGGRAPH 2002.");
            // 
            // inputMeshText
            // 
            this.inputMeshText.Location = new System.Drawing.Point(16, 24);
            this.inputMeshText.Name = "inputMeshText";
            this.inputMeshText.Size = new System.Drawing.Size(64, 20);
            this.inputMeshText.TabIndex = 0;
            this.inputMeshText.Text = "Input mesh:";
            this.inputMeshText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.inputMeshText, @"This is the file that is loaded as a Mesh and passed into SphericalHarmonics.PrtSimulation so that it can compute and return spherical harmonic transfer coefficients for each vertex in the mesh. It returns these coefficients in a ID3DXBuffer*. This process takes some time and should be precomputed, however the results can be used in real time. For more detailed information, see ""Precomputed Radiance Transfer for Real-Time Rendering in Dynamic, Low-Frequency Lighting Environments"" by Peter-Pike Sloan, Jan Kautz, and John Snyder, SIGGRAPH 2002.");
            // 
            // subsurfCB
            // 
            this.subsurfCB.Location = new System.Drawing.Point(80, 160);
            this.subsurfCB.Name = "subsurfCB";
            this.subsurfCB.Size = new System.Drawing.Size(176, 24);
            this.subsurfCB.TabIndex = 6;
            this.subsurfCB.Text = "Enable subsurface scattering";
            this.toolTip.SetToolTip(this.subsurfCB, @"If checked then subsurface scattering will be done in the simulator. Subsurface scattering is when light penetrates a translucent surface and comes out the other side. For example, a jade sculpture or a flashlight shining through skin exhibits subsurface scattering. The simulator assumes the mesh is made of a homogenous material. If subsurface scattering is not used, then the length scale, the relative index of refraction, the reduced scattering coefficients, and the absorption coefficients are not used.");
            this.subsurfCB.CheckedChanged += new System.EventHandler(this.subsurfCB_CheckedChanged);
            // 
            // numRaysUD
            // 
            this.numRaysUD.Location = new System.Drawing.Point(120, 88);
            this.numRaysUD.Name = "numRaysUD";
            this.numRaysUD.Size = new System.Drawing.Size(104, 20);
            this.numRaysUD.TabIndex = 6;
            this.toolTip.SetToolTip(this.numRaysUD, "This controls the number of rays to shoot at each sample. The more rays the more " +
                "accurate the final result will be, but it will increase time it takes to precomp" +
                "ute the transfer coefficients.");
            // 
            // materialSettingsGB
            // 
            this.materialSettingsGB.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
                | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.materialSettingsGB.Controls.Add(this.greenText);
            this.materialSettingsGB.Controls.Add(this.redText);
            this.materialSettingsGB.Controls.Add(this.predefCombo);
            this.materialSettingsGB.Controls.Add(this.absorptionBEdit);
            this.materialSettingsGB.Controls.Add(this.absorptionGEdit);
            this.materialSettingsGB.Controls.Add(this.absorptionREdit);
            this.materialSettingsGB.Controls.Add(this.scatteringBEdit);
            this.materialSettingsGB.Controls.Add(this.scatteringGEdit);
            this.materialSettingsGB.Controls.Add(this.scatteringREdit);
            this.materialSettingsGB.Controls.Add(this.reflectanceBEdit);
            this.materialSettingsGB.Controls.Add(this.reflectanceGEdit);
            this.materialSettingsGB.Controls.Add(this.refractionEdit);
            this.materialSettingsGB.Controls.Add(this.reflectanceREdit);
            this.materialSettingsGB.Controls.Add(this.absorptionText);
            this.materialSettingsGB.Controls.Add(this.scatteringText);
            this.materialSettingsGB.Controls.Add(this.reflectanceText);
            this.materialSettingsGB.Controls.Add(this.refractionText);
            this.materialSettingsGB.Controls.Add(this.predefText);
            this.materialSettingsGB.Controls.Add(this.blueText);
            this.materialSettingsGB.Location = new System.Drawing.Point(16, 216);
            this.materialSettingsGB.Name = "materialSettingsGB";
            this.materialSettingsGB.Size = new System.Drawing.Size(480, 206);
            this.materialSettingsGB.TabIndex = 1;
            this.materialSettingsGB.TabStop = false;
            this.materialSettingsGB.Text = "Material settings";
            // 
            // greenText
            // 
            this.greenText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.greenText.Location = new System.Drawing.Point(312, 88);
            this.greenText.Name = "greenText";
            this.greenText.Size = new System.Drawing.Size(40, 16);
            this.greenText.TabIndex = 30;
            this.greenText.Text = "Green";
            this.greenText.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.toolTip.SetToolTip(this.greenText, "The values below are the green coefficients");
            // 
            // redText
            // 
            this.redText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.redText.Location = new System.Drawing.Point(240, 88);
            this.redText.Name = "redText";
            this.redText.Size = new System.Drawing.Size(32, 16);
            this.redText.TabIndex = 29;
            this.redText.Text = "Red";
            this.redText.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.toolTip.SetToolTip(this.redText, "The values below are the red coefficients. If spectral is turned off, then this i" +
                "s the channel that will be used.");
            // 
            // predefCombo
            // 
            this.predefCombo.Location = new System.Drawing.Point(224, 16);
            this.predefCombo.Name = "predefCombo";
            this.predefCombo.Size = new System.Drawing.Size(192, 21);
            this.predefCombo.TabIndex = 28;
            this.toolTip.SetToolTip(this.predefCombo, @"These are some example materials. Choosing one of these materials with change the all the material values below. The parameters for these materials are from ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001. The relative index of refraction is with respect the material immersed in air.");
            this.predefCombo.SelectedIndexChanged += new System.EventHandler(this.predefCombo_SelectedIndexChanged);
            // 
            // absorptionBEdit
            // 
            this.absorptionBEdit.Location = new System.Drawing.Point(368, 168);
            this.absorptionBEdit.Name = "absorptionBEdit";
            this.absorptionBEdit.Size = new System.Drawing.Size(64, 20);
            this.absorptionBEdit.TabIndex = 27;
            this.absorptionBEdit.Text = "";
            this.toolTip.SetToolTip(this.absorptionBEdit, @"The absorption coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001");
            this.absorptionBEdit.TextChanged += new System.EventHandler(this.absorptionBEdit_TextChanged);
            // 
            // absorptionGEdit
            // 
            this.absorptionGEdit.Location = new System.Drawing.Point(296, 168);
            this.absorptionGEdit.Name = "absorptionGEdit";
            this.absorptionGEdit.Size = new System.Drawing.Size(64, 20);
            this.absorptionGEdit.TabIndex = 26;
            this.absorptionGEdit.Text = "";
            this.toolTip.SetToolTip(this.absorptionGEdit, @"The absorption coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001");
            this.absorptionGEdit.TextChanged += new System.EventHandler(this.absorptionGEdit_TextChanged);
            // 
            // absorptionREdit
            // 
            this.absorptionREdit.Location = new System.Drawing.Point(224, 168);
            this.absorptionREdit.Name = "absorptionREdit";
            this.absorptionREdit.Size = new System.Drawing.Size(64, 20);
            this.absorptionREdit.TabIndex = 25;
            this.absorptionREdit.Text = "";
            this.toolTip.SetToolTip(this.absorptionREdit, @"The absorption coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001");
            this.absorptionREdit.TextChanged += new System.EventHandler(this.absorptionREdit_TextChanged);
            // 
            // scatteringBEdit
            // 
            this.scatteringBEdit.Location = new System.Drawing.Point(368, 136);
            this.scatteringBEdit.Name = "scatteringBEdit";
            this.scatteringBEdit.Size = new System.Drawing.Size(64, 20);
            this.scatteringBEdit.TabIndex = 24;
            this.scatteringBEdit.Text = "";
            this.toolTip.SetToolTip(this.scatteringBEdit, @"The reduced scattering coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001");
            this.scatteringBEdit.TextChanged += new System.EventHandler(this.scatteringBEdit_TextChanged);
            // 
            // scatteringGEdit
            // 
            this.scatteringGEdit.Location = new System.Drawing.Point(296, 136);
            this.scatteringGEdit.Name = "scatteringGEdit";
            this.scatteringGEdit.Size = new System.Drawing.Size(64, 20);
            this.scatteringGEdit.TabIndex = 23;
            this.scatteringGEdit.Text = "";
            this.toolTip.SetToolTip(this.scatteringGEdit, @"The reduced scattering coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001");
            this.scatteringGEdit.TextChanged += new System.EventHandler(this.scatteringGEdit_TextChanged);
            // 
            // scatteringREdit
            // 
            this.scatteringREdit.Location = new System.Drawing.Point(224, 136);
            this.scatteringREdit.Name = "scatteringREdit";
            this.scatteringREdit.Size = new System.Drawing.Size(64, 20);
            this.scatteringREdit.TabIndex = 22;
            this.scatteringREdit.Text = "";
            this.toolTip.SetToolTip(this.scatteringREdit, @"The reduced scattering coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001");
            this.scatteringREdit.TextChanged += new System.EventHandler(this.scatteringREdit_TextChanged);
            // 
            // reflectanceBEdit
            // 
            this.reflectanceBEdit.Location = new System.Drawing.Point(368, 104);
            this.reflectanceBEdit.Name = "reflectanceBEdit";
            this.reflectanceBEdit.Size = new System.Drawing.Size(64, 20);
            this.reflectanceBEdit.TabIndex = 21;
            this.reflectanceBEdit.Text = "";
            this.toolTip.SetToolTip(this.reflectanceBEdit, "The diffuse reflectance coefficient is the fraction of diffuse light reflected ba" +
                "ck. This value is typically between 0 and 1.");
            this.reflectanceBEdit.TextChanged += new System.EventHandler(this.reflectanceBEdit_TextChanged);
            // 
            // reflectanceGEdit
            // 
            this.reflectanceGEdit.Location = new System.Drawing.Point(296, 104);
            this.reflectanceGEdit.Name = "reflectanceGEdit";
            this.reflectanceGEdit.Size = new System.Drawing.Size(64, 20);
            this.reflectanceGEdit.TabIndex = 20;
            this.reflectanceGEdit.Text = "";
            this.toolTip.SetToolTip(this.reflectanceGEdit, "The diffuse reflectance coefficient is the fraction of diffuse light reflected ba" +
                "ck. This value is typically between 0 and 1.");
            this.reflectanceGEdit.TextChanged += new System.EventHandler(this.reflectanceGEdit_TextChanged);
            // 
            // refractionEdit
            // 
            this.refractionEdit.Location = new System.Drawing.Point(224, 48);
            this.refractionEdit.Name = "refractionEdit";
            this.refractionEdit.Size = new System.Drawing.Size(104, 20);
            this.refractionEdit.TabIndex = 19;
            this.refractionEdit.Text = "";
            this.toolTip.SetToolTip(this.refractionEdit, "Relative index of refraction is the ratio between two absolute indexes of refract" +
                "ion. An index of refraction is ratio of the sine of the angle of incidence to th" +
                "e sine of the angle of refraction.");
            // 
            // reflectanceREdit
            // 
            this.reflectanceREdit.Location = new System.Drawing.Point(224, 104);
            this.reflectanceREdit.Name = "reflectanceREdit";
            this.reflectanceREdit.Size = new System.Drawing.Size(64, 20);
            this.reflectanceREdit.TabIndex = 15;
            this.reflectanceREdit.Text = "";
            this.toolTip.SetToolTip(this.reflectanceREdit, "The diffuse reflectance coefficient is the fraction of diffuse light reflected ba" +
                "ck. This value is typically between 0 and 1.");
            this.reflectanceREdit.TextChanged += new System.EventHandler(this.reflectanceREdit_TextChanged);
            // 
            // absorptionText
            // 
            this.absorptionText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.absorptionText.Location = new System.Drawing.Point(96, 168);
            this.absorptionText.Name = "absorptionText";
            this.absorptionText.Size = new System.Drawing.Size(120, 16);
            this.absorptionText.TabIndex = 14;
            this.absorptionText.Text = "Absorption coefficient:";
            this.absorptionText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.absorptionText, @"The absorption coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001");
            // 
            // scatteringText
            // 
            this.scatteringText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.scatteringText.Location = new System.Drawing.Point(56, 136);
            this.scatteringText.Name = "scatteringText";
            this.scatteringText.Size = new System.Drawing.Size(160, 16);
            this.scatteringText.TabIndex = 13;
            this.scatteringText.Text = "Reduced scattering coefficient:";
            this.scatteringText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.scatteringText, @"The reduced scattering coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001");
            // 
            // reflectanceText
            // 
            this.reflectanceText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.reflectanceText.Location = new System.Drawing.Point(56, 104);
            this.reflectanceText.Name = "reflectanceText";
            this.reflectanceText.Size = new System.Drawing.Size(160, 16);
            this.reflectanceText.TabIndex = 12;
            this.reflectanceText.Text = "Diffuse reflectance coefficient:";
            this.reflectanceText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.reflectanceText, "The diffuse reflectance coefficient is the fraction of diffuse light reflected ba" +
                "ck. This value is typically between 0 and 1.");
            // 
            // refractionText
            // 
            this.refractionText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.refractionText.Location = new System.Drawing.Point(80, 48);
            this.refractionText.Name = "refractionText";
            this.refractionText.Size = new System.Drawing.Size(144, 16);
            this.refractionText.TabIndex = 11;
            this.refractionText.Text = "Relative index of refraction:";
            this.refractionText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.refractionText, "Relative index of refraction is the ratio between two absolute indexes of refract" +
                "ion. An index of refraction is ratio of the sine of the angle of incidence to th" +
                "e sine of the angle of refraction.");
            // 
            // predefText
            // 
            this.predefText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.predefText.Location = new System.Drawing.Point(112, 16);
            this.predefText.Name = "predefText";
            this.predefText.Size = new System.Drawing.Size(112, 16);
            this.predefText.TabIndex = 10;
            this.predefText.Text = "Predefined Material:";
            this.predefText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.predefText, @"These are some example materials. Choosing one of these materials with change the all the material values below. The parameters for these materials are from ""A Practical Model for Subsurface Light Transport"" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001. The relative index of refraction is with respect the material immersed in air.");
            // 
            // blueText
            // 
            this.blueText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.blueText.Location = new System.Drawing.Point(384, 88);
            this.blueText.Name = "blueText";
            this.blueText.Size = new System.Drawing.Size(32, 16);
            this.blueText.TabIndex = 31;
            this.blueText.Text = "Blue";
            this.blueText.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.toolTip.SetToolTip(this.blueText, "The values below are the blue coefficients");
            // 
            // outputSettingsGB
            // 
            this.outputSettingsGB.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
                | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.outputSettingsGB.Controls.Add(this.meshSaveTextRB);
            this.outputSettingsGB.Controls.Add(this.meshSaveBinaryRB);
            this.outputSettingsGB.Controls.Add(this.compressedCB);
            this.outputSettingsGB.Controls.Add(this.outputBrowse);
            this.outputSettingsGB.Controls.Add(this.outputMeshBrowseButton);
            this.outputSettingsGB.Controls.Add(this.outputMeshEdit);
            this.outputSettingsGB.Controls.Add(this.outputMeshText);
            this.outputSettingsGB.Controls.Add(this.outputEdit);
            this.outputSettingsGB.Controls.Add(this.outputText);
            this.outputSettingsGB.ForeColor = System.Drawing.SystemColors.ControlText;
            this.outputSettingsGB.Location = new System.Drawing.Point(16, 424);
            this.outputSettingsGB.Name = "outputSettingsGB";
            this.outputSettingsGB.Size = new System.Drawing.Size(480, 94);
            this.outputSettingsGB.TabIndex = 2;
            this.outputSettingsGB.TabStop = false;
            this.outputSettingsGB.Text = "Output settings";
            // 
            // meshSaveTextRB
            // 
            this.meshSaveTextRB.Location = new System.Drawing.Point(400, 56);
            this.meshSaveTextRB.Name = "meshSaveTextRB";
            this.meshSaveTextRB.Size = new System.Drawing.Size(48, 16);
            this.meshSaveTextRB.TabIndex = 34;
            this.meshSaveTextRB.Text = "Text";
            // 
            // meshSaveBinaryRB
            // 
            this.meshSaveBinaryRB.Location = new System.Drawing.Point(344, 56);
            this.meshSaveBinaryRB.Name = "meshSaveBinaryRB";
            this.meshSaveBinaryRB.Size = new System.Drawing.Size(72, 16);
            this.meshSaveBinaryRB.TabIndex = 33;
            this.meshSaveBinaryRB.Text = "Binary";
            // 
            // compressedCB
            // 
            this.compressedCB.Location = new System.Drawing.Point(344, 24);
            this.compressedCB.Name = "compressedCB";
            this.compressedCB.Size = new System.Drawing.Size(88, 24);
            this.compressedCB.TabIndex = 32;
            this.compressedCB.Text = "Compressed";
            this.compressedCB.CheckedChanged += new System.EventHandler(this.compressedCB_CheckedChanged);
            // 
            // outputBrowse
            // 
            this.outputBrowse.Location = new System.Drawing.Point(272, 24);
            this.outputBrowse.Name = "outputBrowse";
            this.outputBrowse.Size = new System.Drawing.Size(56, 20);
            this.outputBrowse.TabIndex = 31;
            this.outputBrowse.Text = "Browse";
            this.toolTip.SetToolTip(this.outputBrowse, "Select the output buffer file name");
            this.outputBrowse.Click += new System.EventHandler(this.outputBrowse_Click);
            // 
            // outputMeshBrowseButton
            // 
            this.outputMeshBrowseButton.Location = new System.Drawing.Point(272, 56);
            this.outputMeshBrowseButton.Name = "outputMeshBrowseButton";
            this.outputMeshBrowseButton.Size = new System.Drawing.Size(56, 20);
            this.outputMeshBrowseButton.TabIndex = 30;
            this.outputMeshBrowseButton.Text = "Browse";
            this.toolTip.SetToolTip(this.outputMeshBrowseButton, "Select the output mesh file name");
            this.outputMeshBrowseButton.Click += new System.EventHandler(this.outputMeshBrowseButton_Click);
            // 
            // outputMeshEdit
            // 
            this.outputMeshEdit.Location = new System.Drawing.Point(112, 56);
            this.outputMeshEdit.Name = "outputMeshEdit";
            this.outputMeshEdit.Size = new System.Drawing.Size(152, 20);
            this.outputMeshEdit.TabIndex = 29;
            this.outputMeshEdit.Text = "";
            this.toolTip.SetToolTip(this.outputMeshEdit, "If adaptive tessellation is on, then this sample will save the resulting tessella" +
                "ted mesh to this file.");
            // 
            // outputMeshText
            // 
            this.outputMeshText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.outputMeshText.Location = new System.Drawing.Point(40, 56);
            this.outputMeshText.Name = "outputMeshText";
            this.outputMeshText.Size = new System.Drawing.Size(72, 16);
            this.outputMeshText.TabIndex = 28;
            this.outputMeshText.Text = "Output mesh: ";
            this.outputMeshText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.outputMeshText, "If adaptive tessellation is on, then this sample will save the resulting tessella" +
                "ted mesh to this file.");
            // 
            // outputEdit
            // 
            this.outputEdit.Location = new System.Drawing.Point(112, 24);
            this.outputEdit.Name = "outputEdit";
            this.outputEdit.Size = new System.Drawing.Size(152, 20);
            this.outputEdit.TabIndex = 27;
            this.outputEdit.Text = "";
            this.toolTip.SetToolTip(this.outputEdit, "This sample will save a binary buffer of spherical harmonic transfer coefficients" +
                " to a file which the sample can read in later.  This is the file name of the whe" +
                "re the resulting binary buffer will be saved");
            // 
            // outputText
            // 
            this.outputText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right)));
            this.outputText.Location = new System.Drawing.Point(32, 24);
            this.outputText.Name = "outputText";
            this.outputText.Size = new System.Drawing.Size(80, 16);
            this.outputText.TabIndex = 26;
            this.outputText.Text = "Output buffer: ";
            this.outputText.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.outputText, "This sample will save a binary buffer of spherical harmonic transfer coefficients" +
                " to a file which the sample can read in later.  This is the file name of the whe" +
                "re the resulting binary buffer will be saved");
            // 
            // goButton
            // 
            this.goButton.Location = new System.Drawing.Point(16, 520);
            this.goButton.Name = "goButton";
            this.goButton.Size = new System.Drawing.Size(72, 24);
            this.goButton.TabIndex = 3;
            this.goButton.Text = "Go";
            this.toolTip.SetToolTip(this.goButton, @"This will start the simulator based on the options selected above. This process takes some time and should be precomputed, however the results can be used in real time. When the simulator is done calculating the spherical harmonic transfer coefficients for each vertex, the sample will use D3DXSHPRTCompress() to compress the data based on the number of PCA vectors chosen. This sample will then save the binary data of coefficients to a file. This sample will also allow you to view the results by passing these coefficients to a vertex shader for real time rendering (if the Direct3D device has hardware accelerated programmable vertex shader support). This sample also stores the settings of this dialog to the registry so it remembers them for next time.");
            this.goButton.Click += new System.EventHandler(this.goButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(424, 520);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(72, 24);
            this.cancelButton.TabIndex = 4;
            this.cancelButton.Text = "Cancel";
            this.toolTip.SetToolTip(this.cancelButton, "This cancels the dialog, does not save the settings, and does not run the simulat" +
                "or.");
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // showTooltipsCB
            // 
            this.showTooltipsCB.Location = new System.Drawing.Point(216, 520);
            this.showTooltipsCB.Name = "showTooltipsCB";
            this.showTooltipsCB.Size = new System.Drawing.Size(96, 24);
            this.showTooltipsCB.TabIndex = 5;
            this.showTooltipsCB.Text = "Show Tooltips";
            this.showTooltipsCB.CheckedChanged += new System.EventHandler(this.showTooltipsCB_CheckedChanged);
            // 
            // toolTip
            // 
            this.toolTip.AutoPopDelay = 32000;
            this.toolTip.InitialDelay = 0;
            this.toolTip.ReshowDelay = 0;
            this.toolTip.ShowAlways = true;
            // 
            // PrtOptionsForm
            // 
            this.AcceptButton = this.goButton;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(511, 555);
            this.Controls.Add(this.showTooltipsCB);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.goButton);
            this.Controls.Add(this.outputSettingsGB);
            this.Controls.Add(this.materialSettingsGB);
            this.Controls.Add(this.prtSettingsGB);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Menu = this.mainMenu;
            this.MinimizeBox = false;
            this.Name = "PrtOptionsForm";
            this.Text = "Precomputed Radiance Transfer";
            this.prtSettingsGB.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.numBouncesUD)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.orderSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numRaysUD)).EndInit();
            this.materialSettingsGB.ResumeLayout(false);
            this.outputSettingsGB.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion

        #region Manually Written Code

        // Manual code
        public SimulatorOptions GetOptions()
        {
            return PrtOptions.GlobalOptions;
        }

        public void LoadOptions(string file)
        {
            PrtOptions.GlobalOptionsFile.LoadOptions(file);
        }

        public void SaveOptions(string file)
        {
            PrtOptions.GlobalOptionsFile.SaveOptions(file);
        }

        public void ResetSettings()
        {
            PrtOptions.GlobalOptionsFile.ResetSettings();
        }

        public void UpdateControlsWithSettings()
        {
            Directory.SetCurrentDirectory( PrtOptions.GlobalOptions.InitialDir );

            inputMeshNameTB.Text = PrtOptions.GlobalOptions.InputMesh;
            outputMeshEdit.Text = PrtOptions.GlobalOptions.OutputMesh;
            outputMeshEdit.SelectionStart = 0;
            outputMeshEdit.SelectionLength = outputMeshEdit.Text.Length;

            orderSlider.SetRange(2, 6);
            orderSlider.TickFrequency = 1;
            orderSlider.Value = PrtOptions.GlobalOptions.Order;

            numBouncesUD.Maximum = 10;
            numBouncesUD.Minimum = 1;
            numBouncesUD.Value = PrtOptions.GlobalOptions.NumberBounces;

            numRaysUD.Maximum = 30000;
            numRaysUD.Minimum = 10;
            numRaysUD.Value = PrtOptions.GlobalOptions.NumberRays;

            subsurfCB.Checked = PrtOptions.GlobalOptions.IsSubsurfaceScattering;
            adaptiveCB.Checked = PrtOptions.GlobalOptions.IsAdaptive;
            spectralCB.Checked = PrtOptions.GlobalOptions.NumberChannels == 3;

            for( int i = 0; i < PrtOptions.PredefinedMaterials.Length; i++ )
            {
                predefCombo.Items.Add(PrtOptions.PredefinedMaterials[i].Name);
            }
            predefCombo.SelectedIndex = PrtOptions.GlobalOptions.PredefinedMaterialIndex; 

            IsComboBoxSelChange     = true;
            refractionEdit.Text     = string.Format("{0}", PrtOptions.GlobalOptions.RelativeIndexOfRefraction);
            absorptionREdit.Text    = string.Format("{0}", PrtOptions.GlobalOptions.Absorption.Red);
            absorptionGEdit.Text    = string.Format("{0}", PrtOptions.GlobalOptions.Absorption.Green);
            absorptionBEdit.Text    = string.Format("{0}", PrtOptions.GlobalOptions.Absorption.Blue);
            reflectanceREdit.Text   = string.Format("{0}", PrtOptions.GlobalOptions.Diffuse.Red);
            reflectanceGEdit.Text   = string.Format("{0}", PrtOptions.GlobalOptions.Diffuse.Green);
            reflectanceBEdit.Text   = string.Format("{0}", PrtOptions.GlobalOptions.Diffuse.Blue);
            scatteringREdit.Text    = string.Format("{0}", PrtOptions.GlobalOptions.ReducedScattering.Red);
            scatteringGEdit.Text    = string.Format("{0}", PrtOptions.GlobalOptions.ReducedScattering.Green);
            scatteringBEdit.Text    = string.Format("{0}", PrtOptions.GlobalOptions.ReducedScattering.Blue);
            IsComboBoxSelChange     = false;

            lengthScaleEdit.Text = String.Format("{0:f1}", PrtOptions.GlobalOptions.LengthScale);

            outputEdit.Text = PrtOptions.GlobalOptions.ResultsFileName;
            outputEdit.SelectionStart = 0;
            outputEdit.SelectionLength = outputEdit.Text.Length;

            if(PrtOptions.GlobalOptions.IsBinaryOutputXFile)
            {
                meshSaveBinaryRB.Checked = true;
                meshSaveTextRB.Checked = false;
            }
            else
            {
                meshSaveBinaryRB.Checked = false;
                meshSaveTextRB.Checked = true;
            }

            compressedCB.Checked = PrtOptions.GlobalOptions.IsSaveCompressedResults;
            showTooltipsCB.Checked = IsShowToolTips;
        }
        #endregion

        #region Auto-Generated Event Handlers

        private void inputBrowseButton_Click(object sender, System.EventArgs e)
        {
            // Display the OpenFileName dialog
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.InitialDirectory = PrtOptions.GlobalOptions.InitialDir;
            ofd.Filter = ".X Files (.x)|*.x|All Files|*.*";
            ofd.FilterIndex = 1;
            ofd.FileName = PrtOptions.GlobalOptions.InputMesh;
            ofd.Title = "Open Mesh File";
            ofd.CheckFileExists = true;
            if(ofd.ShowDialog() == DialogResult.OK)
            {
                PrtOptions.GlobalOptions.InputMesh = ofd.FileName;
                PrtOptions.GlobalOptions.InitialDir = System.IO.Path.GetDirectoryName(ofd.FileName);

                ofd.FileName = System.IO.Path.GetFileName(ofd.FileName);
                string resultFile = System.IO.Path.GetFileNameWithoutExtension(ofd.FileName);
                resultFile += "_prtresults";
                if(compressedCB.Checked)
                    resultFile += ".pca";
                else
                    resultFile += ".prt";
                PrtOptions.GlobalOptions.ResultsFileName = resultFile;
                outputEdit.Text = PrtOptions.GlobalOptions.ResultsFileName;

                resultFile = System.IO.Path.GetFileNameWithoutExtension(ofd.FileName) + "_adaptive.x";
                PrtOptions.GlobalOptions.OutputMesh = resultFile;
                outputMeshEdit.Text = PrtOptions.GlobalOptions.OutputMesh;


                Directory.SetCurrentDirectory( PrtOptions.GlobalOptions.InitialDir );
                inputMeshNameTB.Text = PrtOptions.GlobalOptions.InputMesh;
                inputMeshNameTB.SelectionStart = 0;
                inputMeshNameTB.SelectionLength = inputMeshNameTB.Text.Length;
            }
        }

        private void adaptiveSettingsButton_Click(object sender, System.EventArgs e)
        {
            PrtAdaptiveOptionsForm form = new PrtAdaptiveOptionsForm();
            form.ShowDialog();
        }

        private void outputBrowse_Click(object sender, System.EventArgs e)
        {
            // Display the SaveFileDialog
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.InitialDirectory = PrtOptions.GlobalOptions.InitialDir;
            sfd.FilterIndex = 1;
            sfd.FileName = outputEdit.Text;
            sfd.Title = "Save Results File";
            sfd.CheckPathExists = true;
            sfd.OverwritePrompt = true;

            if(compressedCB.Checked)
            {
                sfd.Filter = "Compressed Prt buffer Files (.pca)|*.pca|All Files|*.*";
                sfd.DefaultExt = ".pca";
            }
            else
            {
                sfd.Filter = "Uncompressed Prt buffer Files (.prt)|*.prt|All Files|*.*";
                sfd.DefaultExt = ".prt";
            }

            if(sfd.ShowDialog() == DialogResult.OK)
            {
                PrtOptions.GlobalOptions.InitialDir = System.IO.Path.GetDirectoryName(sfd.FileName);

                Directory.SetCurrentDirectory( PrtOptions.GlobalOptions.InitialDir );
                outputEdit.Text = sfd.FileName;
                outputEdit.SelectionStart = 0;
                outputEdit.SelectionLength = outputEdit.Text.Length;
            }
        }

        private void outputMeshBrowseButton_Click(object sender, System.EventArgs e)
        {
            // Display the SaveFileDialog
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.InitialDirectory = PrtOptions.GlobalOptions.InitialDir;
            sfd.Filter = "X Files (.x)|*.x|All Files|*.*";
            sfd.FilterIndex = 1;
            sfd.DefaultExt = ".x";
            sfd.FileName = PrtOptions.GlobalOptions.OutputMesh;
            sfd.Title = "Save Output Mesh";
            sfd.CheckPathExists = true;
            sfd.OverwritePrompt = true;
            
            if(sfd.ShowDialog() == DialogResult.OK)
            {
                PrtOptions.GlobalOptions.OutputMesh = sfd.FileName;
                PrtOptions.GlobalOptions.InitialDir = System.IO.Path.GetDirectoryName(sfd.FileName);
                Directory.SetCurrentDirectory( PrtOptions.GlobalOptions.InitialDir );
                outputMeshEdit.Text = PrtOptions.GlobalOptions.OutputMesh;
                outputMeshEdit.SelectionStart = 0;
                outputMeshEdit.SelectionLength = outputMeshEdit.Text.Length;
            }
        }

        private void goButton_Click(object sender, System.EventArgs e)
        {
            inputMeshNameTB.Text = PrtOptions.GlobalOptions.InputMesh;
            outputMeshEdit.Text = PrtOptions.GlobalOptions.OutputMesh;

            PrtOptions.GlobalOptions.Order = orderSlider.Value;
            PrtOptions.GlobalOptions.NumberBounces = decimal.ToInt32(numBouncesUD.Value);
            PrtOptions.GlobalOptions.NumberRays = decimal.ToInt32(numRaysUD.Value);
            PrtOptions.GlobalOptions.IsSubsurfaceScattering = subsurfCB.Checked;
            PrtOptions.GlobalOptions.IsAdaptive = adaptiveCB.Checked;
            PrtOptions.GlobalOptions.NumberChannels = spectralCB.Checked? 3 : 0;
            PrtOptions.GlobalOptions.PredefinedMaterialIndex = predefCombo.SelectedIndex;
            PrtOptions.GlobalOptions.IsSaveCompressedResults = compressedCB.Checked;
            PrtOptions.GlobalOptions.IsBinaryOutputXFile = meshSaveBinaryRB.Checked;

            if(PrtOptions.GlobalOptions.IsAdaptive)
            {
                if( !PrtOptions.GlobalOptions.IsRobustMeshRefine &&
                    !PrtOptions.GlobalOptions.IsAdaptiveDL &&
                    !PrtOptions.GlobalOptions.IsAdaptiveBounce )
                    PrtOptions.GlobalOptions.IsAdaptive = false;
            }

            try
            {
                PrtOptions.GlobalOptions.RelativeIndexOfRefraction = float.Parse(refractionEdit.Text);
                PrtOptions.GlobalOptions.Absorption = new ColorValue(float.Parse(absorptionREdit.Text),
                    float.Parse(absorptionGEdit.Text),
                    float.Parse(absorptionBEdit.Text));
                PrtOptions.GlobalOptions.Diffuse = new ColorValue(float.Parse(reflectanceREdit.Text),
                    float.Parse(reflectanceGEdit.Text),
                    float.Parse(reflectanceBEdit.Text));
                PrtOptions.GlobalOptions.ReducedScattering = new ColorValue(float.Parse(scatteringREdit.Text),
                    float.Parse(scatteringGEdit.Text),
                    float.Parse(scatteringBEdit.Text));
            }
            catch (FormatException)
            {
                MessageBox.Show("You must use a valid floating point value for the fields which require it.");
                DialogResult = DialogResult.Retry;
                return;
            }

            outputEdit.Text = PrtOptions.GlobalOptions.ResultsFileName;

            PrtOptions.GlobalOptionsFile.SaveOptions(String.Empty);

            DialogResult = DialogResult.OK;
        }

        private void cancelButton_Click(object sender, System.EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
        }

        private void reflectanceREdit_TextChanged(object sender, System.EventArgs e)
        {
            if(!IsComboBoxSelChange)
                predefCombo.SelectedIndex = PrtOptions.PredefinedMaterials.Length - 1;
        }

        private void reflectanceGEdit_TextChanged(object sender, System.EventArgs e)
        {
            reflectanceREdit_TextChanged(sender, e);
        }

        private void reflectanceBEdit_TextChanged(object sender, System.EventArgs e)
        {
            reflectanceREdit_TextChanged(sender, e);
        }

        private void scatteringREdit_TextChanged(object sender, System.EventArgs e)
        {
            reflectanceREdit_TextChanged(sender, e);
        }

        private void scatteringGEdit_TextChanged(object sender, System.EventArgs e)
        {
            reflectanceREdit_TextChanged(sender, e);
        }

        private void scatteringBEdit_TextChanged(object sender, System.EventArgs e)
        {
            reflectanceREdit_TextChanged(sender, e);
        }

        private void absorptionREdit_TextChanged(object sender, System.EventArgs e)
        {
            reflectanceREdit_TextChanged(sender, e);
        }

        private void absorptionGEdit_TextChanged(object sender, System.EventArgs e)
        {
            reflectanceREdit_TextChanged(sender, e);
        }

        private void absorptionBEdit_TextChanged(object sender, System.EventArgs e)
        {
            reflectanceREdit_TextChanged(sender, e);
        }

        private void adaptiveCB_CheckedChanged(object sender, System.EventArgs e)
        {
            subsurfCB_CheckedChanged(sender, e);
        }

        private void spectralCB_CheckedChanged(object sender, System.EventArgs e)
        {
            subsurfCB_CheckedChanged(sender, e);
        }

        private void subsurfCB_CheckedChanged(object sender, System.EventArgs e)
        {
            outputMeshText.Enabled = 
                outputMeshEdit.Enabled = 
                outputMeshBrowseButton.Enabled = 
                adaptiveSettingsButton.Enabled = 
                meshSaveBinaryRB.Enabled = 
                meshSaveTextRB.Enabled = adaptiveCB.Checked;

            refractionText.Enabled = 
                absorptionText.Enabled = 
                scatteringText.Enabled = subsurfCB.Checked;

            reflectanceREdit.Enabled = true;
            reflectanceGEdit.Enabled = 
                reflectanceBEdit.Enabled = spectralCB.Checked;

            absorptionREdit.Enabled = subsurfCB.Checked;
            absorptionGEdit.Enabled = 
                absorptionBEdit.Enabled = subsurfCB.Checked && spectralCB.Checked;

            scatteringREdit.Enabled = subsurfCB.Checked;
            scatteringGEdit.Enabled = 
                scatteringBEdit.Enabled = subsurfCB.Checked && spectralCB.Checked;

            refractionEdit.Enabled = 
                lengthScaleText.Enabled =
                lengthScaleEdit.Enabled = subsurfCB.Checked;
        }

        private void compressedCB_CheckedChanged(object sender, System.EventArgs e)
        {
            string result = System.IO.Path.GetFileNameWithoutExtension(outputEdit.Text);

            if(compressedCB.Checked)
                result += ".pca";
            else
                result += ".prt";

            outputEdit.Text = result;
            outputEdit.SelectionStart = 0;
            outputEdit.SelectionLength = outputEdit.Text.Length;
        }

        private void predefCombo_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            if(predefCombo.SelectedIndex == PrtOptions.PredefinedMaterials.Length - 1) return;

            PredefinedMaterial selectedData = (PredefinedMaterial) PrtOptions.PredefinedMaterialsHT[(string)predefCombo.SelectedItem];
            IsComboBoxSelChange     = true;
            absorptionREdit.Text    = string.Format("{0}",  selectedData.Absorption.Red );
            absorptionGEdit.Text    = string.Format("{0}",  selectedData.Absorption.Green );
            absorptionBEdit.Text    = string.Format("{0}",  selectedData.Absorption.Blue );
            reflectanceREdit.Text   = string.Format("{0}",  selectedData.Diffuse.Red );
            reflectanceGEdit.Text   = string.Format("{0}",  selectedData.Diffuse.Green );
            reflectanceBEdit.Text   = string.Format("{0}",  selectedData.Diffuse.Blue );
            scatteringREdit.Text    = string.Format("{0}",  selectedData.ReducedScattering.Red );
            scatteringGEdit.Text    = string.Format("{0}",  selectedData.ReducedScattering.Green );
            scatteringBEdit.Text    = string.Format("{0}",  selectedData.ReducedScattering.Blue );
            refractionEdit.Text     = string.Format("{0:f2}", selectedData.RelativeIndexOfRefraction );
            IsComboBoxSelChange     = false;
        }

        private void resetMI_Click(object sender, System.EventArgs e)
        {
            ResetSettings();
            UpdateControlsWithSettings();
        }

        #endregion

        private void showTooltipsCB_CheckedChanged(object sender, System.EventArgs e)
        {
            toolTip.Active = IsShowToolTips = showTooltipsCB.Checked;
        }
    }
}
