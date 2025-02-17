using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace AutoReporter
{
    public partial class Form1 : Form
    {
        public string crashDesc;
        public string summary;

        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            crashDesc = textBox1.Text;
            summary = textBox2.Text;
            Close();
        }

        public void SetCallStack(string CallStack)
        {
            label3.Text = CallStack;

            int LabelHeightIncrease = (CallStack.Length / 80) * 20;
            label3.Height += LabelHeightIncrease;

            int FormHeightIncrease = (CallStack.Length / 100) * 15;
            Height += FormHeightIncrease;

        }

        public void SetServiceError(string ErrorMsg)
        {
            textBox1.Visible = false;
            label4.Visible = false;
            textBox2.Visible = false;

            label1.Text = "An error occured connecting to the Report Service: \n" + ErrorMsg;
            label1.Height += 20;
        }
    }
}