using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using UnrealControls;

namespace UnrealConsole
{
	/// <summary>
	/// Dialog box for finding text within a <see cref="RichTextBox"/>.
	/// </summary>
	public partial class FindDialog : Form
	{
		OutputWindowView TxtBox;

		/// <summary>
		/// Gets/Sets the text box associated with the find dialog box.
		/// </summary>
		public OutputWindowView TextBox
		{
			get { return TxtBox; }
			set
			{
				if(value != TxtBox)
				{
					DisconnectEvents();

					TxtBox = value;

					ConnectEvents();
				}
			}
		}

		/// <summary>
		/// Constructor.
		/// </summary>
		public FindDialog()
		{
			InitializeComponent();

			if(Properties.Settings.Default.FindHistory != null && Properties.Settings.Default.FindHistory.Length > 0)
			{
				Combo_SearchString.Items.AddRange(Properties.Settings.Default.FindHistory);
			}
		}

		/// <summary>
		/// Event handler for when the close button has been clicked.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Button_Close_Click(object sender, EventArgs e)
		{
			Visible = false;
		}

		/// <summary>
		/// Event handler for when the Find Next button has been clicked.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Button_FindNext_Click(object sender, EventArgs e)
		{
			ExecuteFindNext();
		}

		/// <summary>
		/// Executes a find operation.
		/// </summary>
		/// <param name="bSearchDown">True if the search is to be performed down from the current location, otherwise the search is performed up.</param>
		public void ExecuteFindNext(bool bSearchDown)
		{
			if(this.Combo_SearchString.Text.Length > 0)
			{
				List<string> FindHistory = new List<string>();
				bool bFoundDuplicate = false;

				for(int i = 0; i < Combo_SearchString.Items.Count && !bFoundDuplicate; ++i)
				{
					string Item = Combo_SearchString.Items[i] as string;
					FindHistory.Add(Item);

					if(!bFoundDuplicate && Item != null && Item == Combo_SearchString.Text)
					{
						string Temp = Combo_SearchString.Text;

						// NOTE: This will nuke the contents of Combo_SearchString.Text so we store it in Temp
						Combo_SearchString.Items.RemoveAt(i);

						// Restore the text
						Combo_SearchString.Text = Temp;
						Combo_SearchString.SelectionStart = Temp.Length;

						bFoundDuplicate = true;
					}
				}

				Combo_SearchString.Items.Insert(0, Combo_SearchString.Text);

				if(!bFoundDuplicate)
				{
					FindHistory.Add(Combo_SearchString.Text);
				}

				Properties.Settings.Default.FindHistory = FindHistory.ToArray();

				if(bSearchDown)
				{
					FindDown();
				}
				else
				{
					FindUp();
				}
			}
		}

		/// <summary>
		/// Executes a find operation.
		/// </summary>
		public void ExecuteFindNext()
		{
			ExecuteFindNext(Radio_Down.Checked);
		}

		/// <summary>
		/// Searches down within the text box for the specified search string.
		/// </summary>
		void FindDown()
		{
			if(TxtBox != null && TxtBox.Document != null)
			{
				FindResult Result;
				TextLocation DownSearchStart = TxtBox.SelectionStart >= TxtBox.SelectionEnd ? TxtBox.SelectionStart : TxtBox.SelectionEnd;

				if(!TxtBox.Find(Combo_SearchString.Text, DownSearchStart, TxtBox.Document.EndOfDocument, GetSearchFlags(), out Result))
				{
					if(!TxtBox.Find(Combo_SearchString.Text, TxtBox.Document.BeginningOfDocument, TxtBox.Document.EndOfDocument, GetSearchFlags(), out Result))
					{
						MessageBox.Show(this, "The specified search string does not exist!", Combo_SearchString.Text);
					}
				}
			}
		}

		/// <summary>
		/// Searches up within the text box for the specified search string.
		/// </summary>
		void FindUp()
		{
			if(TxtBox != null && TxtBox.Document != null)
			{
				FindResult Result;
				TextLocation UpSearchStart = TxtBox.SelectionStart <= TxtBox.SelectionEnd ? TxtBox.SelectionStart : TxtBox.SelectionEnd;

				if(!TxtBox.Find(Combo_SearchString.Text, UpSearchStart, TxtBox.Document.BeginningOfDocument, GetSearchFlags() | RichTextBoxFinds.Reverse, out Result))
				{
					if(!TxtBox.Find(Combo_SearchString.Text, TxtBox.Document.EndOfDocument, TxtBox.Document.BeginningOfDocument, GetSearchFlags() | RichTextBoxFinds.Reverse, out Result))
					{
						MessageBox.Show(this, "The specified search string does not exist!", Combo_SearchString.Text);
					}
				}
			}
		}

		/// <summary>
		/// Returns the search flags for the find operation.
		/// </summary>
		/// <returns>The flags used to narrow the searching.</returns>
		RichTextBoxFinds GetSearchFlags()
		{
			RichTextBoxFinds Flags = RichTextBoxFinds.None;

			if(CheckBox_MatchCase.Checked)
			{
				Flags |= RichTextBoxFinds.MatchCase;
			}

			if(CheckBox_MatchWord.Checked)
			{
				Flags |= RichTextBoxFinds.WholeWord;
			}

			return Flags;
		}

		/// <summary>
		/// Connects event handlers to the text box being associated with the find dialog.
		/// </summary>
		void ConnectEvents()
		{
			if(TxtBox != null)
			{
				TxtBox.Disposed += new EventHandler(TxtBox_Disposed);
			}
		}

		/// <summary>
		/// Disconnects event handlers from the text box previously associated with the find dialog.
		/// </summary>
		void DisconnectEvents()
		{
			if(TxtBox != null)
			{
				TxtBox.Disposed -= new EventHandler(TxtBox_Disposed);
			}
		}

		/// <summary>
		/// Event handler for when the text box associated with the find dialog has been disposed.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		void TxtBox_Disposed(object sender, EventArgs e)
		{
				DisconnectEvents();
				this.TextBox = null;
		}

		/// <summary>
		/// Event handler for the closing event.
		/// </summary>
		/// <remarks>Cancels the closing event and hides the window instead of destroying it.</remarks>
		/// <param name="e">Information about the event.</param>
		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			
			// Cancel the closing event and make the dialog invisible
			e.Cancel = true;
			this.Visible = false;
		}

		/// <summary>
		/// Updates the search string from the currently selected text in text box.
		/// </summary>
		public void UpdateFindTextFromSelectedText()
		{
			if(TxtBox != null)
			{
				string Txt = TxtBox.SelectedText;

				if(Txt.Length > 0 && !Txt.Contains(Environment.NewLine))
				{
					this.Combo_SearchString.Text = Txt;
					this.Combo_SearchString.SelectAll();
				}
			}
		}

		/// <summary>
		/// Combo Boxes do some extra weirdness with the enter key so intercept it here before Combo_SearchString gets a chance to process the message.
		/// </summary>
		/// <param name="m">The key message to process.</param>
		/// <returns>True if the parent handled the message.</returns>
		protected override bool ProcessKeyPreview(ref Message m)
		{
			const int WM_KEYDOWN = 0x100;
			const int WM_KEYUP = 0x101;
			const int WM_CHAR = 0x102;

			// as amazing as this sounds the m.HWnd for Combo_SearchString is != to its Handle
			// apparantly Handle's refer to some internal handle or who knows what
			// all I know is I have to call this function instead of checking against m.HWnd directly
			Control Ctrl = Control.FromChildHandle(m.HWnd);

			if(Ctrl != null && Ctrl.Handle == Combo_SearchString.Handle)
			{
				switch(m.Msg)
				{
					case WM_KEYDOWN:
						{
							KeyEventArgs KeyPressed = new KeyEventArgs((Keys)m.WParam.ToInt64());

							if(KeyPressed.KeyCode == Keys.Enter)
							{
								ExecuteFindNext();
								return true;
							}

							break;
						}
					case WM_KEYUP:
						{
							KeyEventArgs KeyPressed = new KeyEventArgs((Keys)m.WParam.ToInt64());

							if(KeyPressed.KeyCode == Keys.Enter)
							{
								return true;
							}
							break;
						}
					case WM_CHAR:
						{
							if((char)m.WParam.ToInt64() == '\r')
							{
								return true;
							}

							break;
						}
				}
			}

			return base.ProcessKeyPreview(ref m);
		}
	}
}