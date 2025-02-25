﻿using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;


using IServiceProvider = System.IServiceProvider;
using IOleServiceProvider = Microsoft.VisualStudio.OLE.Interop.IServiceProvider;

namespace Epic.UnrealDebugger2005
{
    /// <summary>
    /// Summary description for MyControl.
    /// </summary>
    public partial class UnrealWatch : UserControl
    {
        public UnrealWatch()
        {
            InitializeComponent();
            timer1.Start();
        }
       private void OnResize(object sender, System.EventArgs e)
        {
            treeView1.Size = Size;
        }
        private void ReadWatchFile(string fpath)
        {
            if (fpath.EndsWith(".txt"))
            {
                try
                {
                    ArrayList nodeList = new ArrayList();
                    treeView1.Nodes.Clear();
                    TextReader tr = new StreamReader(fpath, System.Text.Encoding.Unicode);
                    TreeNode root = new TreeNode();

                    int watch = 'x';

                    string line = " ";
                    try
                    {
                        while (line != null && line.Length > 0)
                        {
                            int w = tr.Read();
                            if (w != watch)
                            {
                                if (w == (int)'0')
                                    root = treeView1.Nodes.Add("Locals");
                                if (w == (int)'1')
                                {
                                    if (root != null)
                                        root.Expand();
                                    root = treeView1.Nodes.Add("Global");
                                }
                                if (w == (int)'2')
                                    root = treeView1.Nodes.Add("Watches");
                                watch = w;
                            }
                            int space = tr.Read();
                            int level = tr.Read();
                            line = tr.ReadLine();
                            if (line != null)
                            {
                                if (level == 0)
                                    nodeList.Add(root.Nodes.Add(line));
                                else
                                {
                                    TreeNode node = (TreeNode)nodeList[level - 2];
                                    nodeList.Add(node.Nodes.Add(line));
                                }
                            }
                        }
                    }
                    catch (System.Exception /*e2*/)
                    {

                    }
                    tr.Close();
                }
                catch (System.Exception /*e3*/)
                {

                }
            }
        }
        private void OnFileChanged(object sender, System.IO.FileSystemEventArgs e)
        {
            if (e.Name.EndsWith(".txt"))
            {
               timer1.Stop();
               timer1.Start();
            }
 
            // 			LogOutput();
        }

        /// <summary> 
        /// Let this control process the mnemonics.
        /// </summary>
        protected override bool ProcessDialogChar(char charCode)
        {
              // If we're the top-level form or control, we need to do the mnemonic handling
              if (charCode != ' ' && ProcessMnemonic(charCode))
              {
                    return true;
              }
              return base.ProcessDialogChar(charCode);
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            timer1.Stop();
            ReadWatchFile(WTGlobals.WT_WATCHFILE);

        }
    }
}
