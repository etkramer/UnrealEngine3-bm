using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace UnrealLoc
{
    public partial class PickGame : Form
    {
        private UnrealLoc Main = null;
        public string GameName = "";

        public PickGame( UnrealLoc InMain )
        {
            Main = InMain;
            InitializeComponent();
        }

        private void Button_Engine_Click( object sender, EventArgs e )
        {
            GameName = "Engine";
            Close();
        }

        private void Button_Example_Click( object sender, EventArgs e )
        {
            GameName = "Example";
            Close();
        }

        private void Button_GearGame_Click( object sender, EventArgs e )
        {
            GameName = "Gear";
            Close();
        }

        private void Button_UTGame_Click( object sender, EventArgs e )
        {
            GameName = "UT";
            Close();
        }

        private void Button_Options_Click( object sender, EventArgs e )
        {
            OptionsDialog DisplayOptions = new OptionsDialog( Main, Main.Options );
            DisplayOptions.ShowDialog();
        }
    }
}