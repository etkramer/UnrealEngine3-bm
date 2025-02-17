using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.ComponentModel;

namespace EpicGames
{
    class ButtonPictureBox : PictureBox
    {
		[Browsable(true)]
		public override Color ForeColor
		{
			get
			{
				return base.ForeColor;
			}
			set
			{
				base.ForeColor = value;
			}
		}

        [Browsable( true )]
        public override string Text
        {
            get
            {
                return base.Text;
            }
            set
            {
                base.Text = value;
            }
        }

        protected override void OnPaint(PaintEventArgs pe)
        {
            base.OnPaint(pe);

            SizeF size = pe.Graphics.MeasureString(this.Text, this.Font);

			using(SolidBrush brush = new SolidBrush(this.ForeColor))
			{
				pe.Graphics.DrawString(this.Text, this.Font, brush, this.Width / 2f - size.Width / 2f, this.Height / 2f - size.Height / 2f);
			}
        }
    }
}
