using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace packer
{
	public class Log : System.Windows.Forms.Form
	{
		public System.Windows.Forms.RichTextBox logRtfBox;
		private System.ComponentModel.Container components = null;

		public Log()
		{
			InitializeComponent();
		}

		public void DisposeLog(bool disposing)
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

		protected override void Dispose( bool disposing )
		{
			this.Hide();
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.logRtfBox = new System.Windows.Forms.RichTextBox();
			this.SuspendLayout();
			// 
			// logRtfBox
			// 
			this.logRtfBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.logRtfBox.Location = new System.Drawing.Point(0, 0);
			this.logRtfBox.Name = "logRtfBox";
			this.logRtfBox.ReadOnly = true;
			this.logRtfBox.Size = new System.Drawing.Size(264, 309);
			this.logRtfBox.TabIndex = 0;
			this.logRtfBox.Text = "";
			// 
			// Log
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(264, 309);
			this.Controls.Add(this.logRtfBox);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
			this.Name = "Log";
			this.Text = "log";
			this.ResumeLayout(false);

		}
		#endregion
	}
}
