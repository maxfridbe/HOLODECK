using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using CsGL.OpenGL;

namespace packer
{
	/// <summary>
	/// Summary description for PhysicsApp.
	/// </summary>
	public class PhysicsApp : System.Windows.Forms.Form
	{
		public  mainForm main;
		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button cmdFilePick;
		private System.Windows.Forms.OpenFileDialog dlgFile;
		private System.Windows.Forms.StatusBar stats;
		private System.Windows.Forms.Splitter splitter1;
		private CsGL.OpenGL.OpenGLControl ogl;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public PhysicsApp( mainForm parent)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			this.main = parent;
			stats.Text = "I am Ready.";
			//
			// TODO: Add any constructor code after InitializeComponent call
			//
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
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.cmdFilePick = new System.Windows.Forms.Button();
			this.dlgFile = new System.Windows.Forms.OpenFileDialog();
			this.stats = new System.Windows.Forms.StatusBar();
			this.splitter1 = new System.Windows.Forms.Splitter();
			this.ogl = new CsGL.OpenGL.OpenGLControl();
			this.SuspendLayout();
			// 
			// textBox1
			// 
			this.textBox1.Location = new System.Drawing.Point(80, 16);
			this.textBox1.Name = "textBox1";
			this.textBox1.Size = new System.Drawing.Size(464, 20);
			this.textBox1.TabIndex = 0;
			this.textBox1.Text = "textBox1";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(16, 16);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(58, 16);
			this.label1.TabIndex = 1;
			this.label1.Text = "3dBin File:";
			// 
			// cmdFilePick
			// 
			this.cmdFilePick.Location = new System.Drawing.Point(552, 16);
			this.cmdFilePick.Name = "cmdFilePick";
			this.cmdFilePick.Size = new System.Drawing.Size(24, 23);
			this.cmdFilePick.TabIndex = 2;
			this.cmdFilePick.Text = "...";
			this.cmdFilePick.Click += new System.EventHandler(this.cmdFilePick_Click);
			// 
			// dlgFile
			// 
			this.dlgFile.Filter = "3dBin (*.3dbin)|*.3dbin";
			this.dlgFile.FileOk += new System.ComponentModel.CancelEventHandler(this.dlgFile_FileOk);
			// 
			// stats
			// 
			this.stats.Location = new System.Drawing.Point(0, 495);
			this.stats.Name = "stats";
			this.stats.Size = new System.Drawing.Size(600, 22);
			this.stats.TabIndex = 3;
			// 
			// splitter1
			// 
			this.splitter1.Location = new System.Drawing.Point(0, 0);
			this.splitter1.Name = "splitter1";
			this.splitter1.Size = new System.Drawing.Size(3, 495);
			this.splitter1.TabIndex = 4;
			this.splitter1.TabStop = false;
			// 
			// ogl
			// 
			this.ogl.Location = new System.Drawing.Point(16, 48);
			this.ogl.Name = "ogl";
			this.ogl.Size = new System.Drawing.Size(272, 224);
			this.ogl.TabIndex = 5;
			this.ogl.Text = "openGLControl1";
			// 
			// PhysicsApp
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(600, 517);
			this.Controls.Add(this.ogl);
			this.Controls.Add(this.splitter1);
			this.Controls.Add(this.stats);
			this.Controls.Add(this.cmdFilePick);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.textBox1);
			this.Name = "PhysicsApp";
			this.Text = "PhysicsApp";
			this.Load += new System.EventHandler(this.PhysicsApp_Load);
			this.ResumeLayout(false);

		}
		#endregion

		private void cmdFilePick_Click(object sender, System.EventArgs e)
		{
			dlgFile.ShowDialog();
		}

		private void dlgFile_FileOk(object sender, System.ComponentModel.CancelEventArgs e)
		{
			stats.Text = "Loading Model: " + dlgFile.FileName;
			

		}

		private void PhysicsApp_Load(object sender, System.EventArgs e)
		{
			GL.glClearColor(1,1,1,1);
			GL.glClear( GL.GL_COLOR_BUFFER_BIT );
			
		}
	}
}
