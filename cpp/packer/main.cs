using System;
using System.IO;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

using System.Diagnostics;
using System.Drawing.Imaging;

namespace packer
{
	public class mainForm : System.Windows.Forms.Form
	{
		public PhysicsApp phys;
		private System.Windows.Forms.TextBox logTxtBox;
		private System.Windows.Forms.Button showLogBtn;
		private System.Windows.Forms.OpenFileDialog dlgOpenLow;
		private System.Windows.Forms.FolderBrowserDialog dlgFolderIcon;
		private System.ComponentModel.IContainer components = null;	
		private System.Windows.Forms.Label lblFileMedium;
		private System.Windows.Forms.Label lblFileHigh;
		private System.Windows.Forms.TextBox txtPath;
		private System.Windows.Forms.Label lblOutPath;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.TextBox txtInputHigh;
		private System.Windows.Forms.TextBox txtInputMedium;
		private System.Windows.Forms.Button btnBrowseMed;
		private System.Windows.Forms.Label lblFileLow;
		private System.Windows.Forms.GroupBox grpFileQuality;
		private System.Windows.Forms.Button btnBrowseOutpath;
		private System.Windows.Forms.GroupBox grpPath;
		private System.Windows.Forms.OpenFileDialog dlgOpenMedium;
		private System.Windows.Forms.Button btnBrowseLow;
		private System.Windows.Forms.OpenFileDialog dlgOpenHigh;
		private System.Windows.Forms.FolderBrowserDialog dlgFolderPath;
		private System.Windows.Forms.GroupBox grpOutput;
		private System.Windows.Forms.Label objLbl;
		private System.Windows.Forms.TextBox objectNameTxtBox;
		private System.Windows.Forms.RichTextBox commentRtfBox;
		private System.Windows.Forms.Label commentLbl;
		private System.Windows.Forms.Label typeLbl;
		private System.Windows.Forms.ComboBox objectTypeCmbBox;
		private System.Windows.Forms.Panel panelStepOne;
		private System.Windows.Forms.OpenFileDialog dlgOpenConverter;
		private System.Windows.Forms.Panel panel3;
		private System.Windows.Forms.TabPage tabInformation;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtProjectName;
		private System.Windows.Forms.TextBox txtInputLow;
		private System.Windows.Forms.Button btnBrowseHigh;
		
		/*
		 * Added members
		 * /
		/*---------------------------------------------------*/
		
		private enum ObjectType : int
		{
			Static,
			Usable,
			Programmable,
		}
		
		private const string binExtension = ".3dbin";
		private const string txtExtension = ".txt";
		private const string ascExtension = ".3dasc";

		private const int nameLength = 32;
		private const int commentLength = 128;

		private Log logFrm;
		private Stream logFile;
		private StreamWriter outFile;
		private System.Windows.Forms.TabPage tabFiles;
		private System.Windows.Forms.TabPage tabAtributes;
		private System.Windows.Forms.TabPage tabConversion;
		private System.Windows.Forms.TabPage tabVerify;
		private System.Windows.Forms.GroupBox grpConfigurations;
		private System.Windows.Forms.Label lblCommand;
		private System.Windows.Forms.TextBox txtCommandToRun;
		private System.Windows.Forms.Button btnConverter;
		private System.Windows.Forms.Label lblConverter;
		private System.Windows.Forms.TextBox txtConverter;
		private System.Windows.Forms.Button btnGenerateRaw;
		private System.Windows.Forms.GroupBox grpSelectIcon;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button folderBtn;
		private System.Windows.Forms.TextBox inputFolderTxtBox;
		private System.Windows.Forms.GroupBox previewBox;
		private System.Windows.Forms.PictureBox picPreview;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.ListBox imageList;
		private System.Windows.Forms.GroupBox grpPropertiesAdvanced;
		private System.Windows.Forms.TabControl tabctrlMain;
		private System.Windows.Forms.TabControl tabctrlAdvancedAttributes;
		private System.Windows.Forms.TabPage tabObjectDynamics;
		private System.Windows.Forms.TabPage tabObjectPhysics;
		private System.Windows.Forms.TabPage tabObjectScripts;
		private System.Windows.Forms.Label lblAuthor;
		private System.Windows.Forms.TextBox txtAuthor;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox txtVerifyFile;
		private System.Windows.Forms.Button btnVerifyFile;
		private System.Windows.Forms.GroupBox grpVerifyInput;
		private System.Windows.Forms.GroupBox grpVerifyContents;
		private System.Windows.Forms.Button btnVerifyEnact;
		private System.Windows.Forms.TextBox txtVerifyObjectName;
		private System.Windows.Forms.TextBox textBox2;
		private System.Windows.Forms.TextBox textBox3;
		private System.Windows.Forms.TextBox txtVerifyTextureList;
		private System.Windows.Forms.Label lblVerifyObjectName;
		private System.Windows.Forms.Label lblVerifyCount;
		private System.Windows.Forms.Label lblVerifyFileSize;
		private System.Windows.Forms.Label lblVerifyTextures;
		private System.Windows.Forms.Label lblInfoVRUPL;
		private System.Windows.Forms.GroupBox grpInformationAuthors;
		private System.Windows.Forms.Label lblInformationAuthors;
		private System.Windows.Forms.PictureBox tempPb2;
		private System.Windows.Forms.PictureBox tempPb1;
		private System.Windows.Forms.PictureBox pb1;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Button cmdOpenPhys;
		private BinaryWriter binFile;
		//initialize the filter for the 3d file dialog
		//private string filter3D = "Maya Binary|*.mb|Maya Ascii|*.ma";
		
		/*
		 * Constructor, initializer, and dispose
		 * /		
		
		/*---------------------------------------------------*/

		public mainForm()
		{
			InitializeComponent();
			Init();
			DefaultInit();			
		}

		/*---------------------------------------------------*/
		
		public void Init()
		{			
			logFrm = new Log();
			logFrm.Hide();
			logFile = File.Open("log.txt", FileMode.OpenOrCreate, FileAccess.Write, FileShare.Read);
		}

		/*---------------------------------------------------*/
		
		public void DefaultInit()
		{
			this.txtProjectName.Text = "DefaultObject";
			this.txtPath.Text = "D:\\Program Files\\Microsoft Visual Studio .NET 2003\\My Projects\\Vrupl\\holodeck\\data";
			this.txtInputLow.Text = "D:\\Program Files\\Microsoft Visual Studio .NET 2003\\My Projects\\Vrupl\\converter\\data\\in.mb";
			this.txtInputMedium.Text = "D:\\Program Files\\Microsoft Visual Studio .NET 2003\\My Projects\\Vrupl\\converter\\data\\in.mb";
			this.txtConverter.Text = "D:\\Program Files\\Microsoft Visual Studio .NET 2003\\My Projects\\Vrupl\\converter\\Debug\\converter.exe";
			DisplayCommand();
			this.btnGenerateRaw.Enabled = true;
		}

		/*---------------------------------------------------*/
		
		protected override void Dispose( bool disposing )
		{
			logFrm.DisposeLog(disposing);

			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			
			base.Dispose( disposing );
		}

		/*---------------------------------------------------*/

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.logTxtBox = new System.Windows.Forms.TextBox();
			this.showLogBtn = new System.Windows.Forms.Button();
			this.tabctrlMain = new System.Windows.Forms.TabControl();
			this.tabFiles = new System.Windows.Forms.TabPage();
			this.panelStepOne = new System.Windows.Forms.Panel();
			this.grpSelectIcon = new System.Windows.Forms.GroupBox();
			this.label1 = new System.Windows.Forms.Label();
			this.folderBtn = new System.Windows.Forms.Button();
			this.inputFolderTxtBox = new System.Windows.Forms.TextBox();
			this.previewBox = new System.Windows.Forms.GroupBox();
			this.picPreview = new System.Windows.Forms.PictureBox();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.imageList = new System.Windows.Forms.ListBox();
			this.grpFileQuality = new System.Windows.Forms.GroupBox();
			this.btnBrowseHigh = new System.Windows.Forms.Button();
			this.btnBrowseMed = new System.Windows.Forms.Button();
			this.lblFileLow = new System.Windows.Forms.Label();
			this.btnBrowseLow = new System.Windows.Forms.Button();
			this.txtInputLow = new System.Windows.Forms.TextBox();
			this.txtInputMedium = new System.Windows.Forms.TextBox();
			this.lblFileMedium = new System.Windows.Forms.Label();
			this.txtInputHigh = new System.Windows.Forms.TextBox();
			this.lblFileHigh = new System.Windows.Forms.Label();
			this.tabAtributes = new System.Windows.Forms.TabPage();
			this.panel3 = new System.Windows.Forms.Panel();
			this.grpPropertiesAdvanced = new System.Windows.Forms.GroupBox();
			this.tabctrlAdvancedAttributes = new System.Windows.Forms.TabControl();
			this.tabObjectDynamics = new System.Windows.Forms.TabPage();
			this.typeLbl = new System.Windows.Forms.Label();
			this.objectTypeCmbBox = new System.Windows.Forms.ComboBox();
			this.tabObjectPhysics = new System.Windows.Forms.TabPage();
			this.tabObjectScripts = new System.Windows.Forms.TabPage();
			this.grpOutput = new System.Windows.Forms.GroupBox();
			this.txtAuthor = new System.Windows.Forms.TextBox();
			this.lblAuthor = new System.Windows.Forms.Label();
			this.objLbl = new System.Windows.Forms.Label();
			this.objectNameTxtBox = new System.Windows.Forms.TextBox();
			this.commentRtfBox = new System.Windows.Forms.RichTextBox();
			this.commentLbl = new System.Windows.Forms.Label();
			this.tabConversion = new System.Windows.Forms.TabPage();
			this.grpConfigurations = new System.Windows.Forms.GroupBox();
			this.pb1 = new System.Windows.Forms.PictureBox();
			this.lblCommand = new System.Windows.Forms.Label();
			this.txtCommandToRun = new System.Windows.Forms.TextBox();
			this.btnConverter = new System.Windows.Forms.Button();
			this.lblConverter = new System.Windows.Forms.Label();
			this.txtConverter = new System.Windows.Forms.TextBox();
			this.btnGenerateRaw = new System.Windows.Forms.Button();
			this.tabVerify = new System.Windows.Forms.TabPage();
			this.grpVerifyContents = new System.Windows.Forms.GroupBox();
			this.lblVerifyTextures = new System.Windows.Forms.Label();
			this.lblVerifyFileSize = new System.Windows.Forms.Label();
			this.lblVerifyCount = new System.Windows.Forms.Label();
			this.lblVerifyObjectName = new System.Windows.Forms.Label();
			this.txtVerifyTextureList = new System.Windows.Forms.TextBox();
			this.textBox3 = new System.Windows.Forms.TextBox();
			this.textBox2 = new System.Windows.Forms.TextBox();
			this.txtVerifyObjectName = new System.Windows.Forms.TextBox();
			this.grpVerifyInput = new System.Windows.Forms.GroupBox();
			this.btnVerifyEnact = new System.Windows.Forms.Button();
			this.label3 = new System.Windows.Forms.Label();
			this.txtVerifyFile = new System.Windows.Forms.TextBox();
			this.btnVerifyFile = new System.Windows.Forms.Button();
			this.tabInformation = new System.Windows.Forms.TabPage();
			this.tempPb1 = new System.Windows.Forms.PictureBox();
			this.tempPb2 = new System.Windows.Forms.PictureBox();
			this.grpInformationAuthors = new System.Windows.Forms.GroupBox();
			this.lblInformationAuthors = new System.Windows.Forms.Label();
			this.lblInfoVRUPL = new System.Windows.Forms.Label();
			this.grpPath = new System.Windows.Forms.GroupBox();
			this.txtProjectName = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.txtPath = new System.Windows.Forms.TextBox();
			this.btnBrowseOutpath = new System.Windows.Forms.Button();
			this.lblOutPath = new System.Windows.Forms.Label();
			this.dlgOpenLow = new System.Windows.Forms.OpenFileDialog();
			this.dlgFolderIcon = new System.Windows.Forms.FolderBrowserDialog();
			this.panel1 = new System.Windows.Forms.Panel();
			this.panel2 = new System.Windows.Forms.Panel();
			this.dlgOpenMedium = new System.Windows.Forms.OpenFileDialog();
			this.dlgOpenHigh = new System.Windows.Forms.OpenFileDialog();
			this.dlgFolderPath = new System.Windows.Forms.FolderBrowserDialog();
			this.dlgOpenConverter = new System.Windows.Forms.OpenFileDialog();
			this.label4 = new System.Windows.Forms.Label();
			this.cmdOpenPhys = new System.Windows.Forms.Button();
			this.tabctrlMain.SuspendLayout();
			this.tabFiles.SuspendLayout();
			this.panelStepOne.SuspendLayout();
			this.grpSelectIcon.SuspendLayout();
			this.previewBox.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.grpFileQuality.SuspendLayout();
			this.tabAtributes.SuspendLayout();
			this.panel3.SuspendLayout();
			this.grpPropertiesAdvanced.SuspendLayout();
			this.tabctrlAdvancedAttributes.SuspendLayout();
			this.tabObjectDynamics.SuspendLayout();
			this.tabObjectPhysics.SuspendLayout();
			this.grpOutput.SuspendLayout();
			this.tabConversion.SuspendLayout();
			this.grpConfigurations.SuspendLayout();
			this.tabVerify.SuspendLayout();
			this.grpVerifyContents.SuspendLayout();
			this.grpVerifyInput.SuspendLayout();
			this.tabInformation.SuspendLayout();
			this.grpInformationAuthors.SuspendLayout();
			this.grpPath.SuspendLayout();
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// logTxtBox
			// 
			this.logTxtBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.logTxtBox.Location = new System.Drawing.Point(0, 0);
			this.logTxtBox.Name = "logTxtBox";
			this.logTxtBox.Size = new System.Drawing.Size(616, 20);
			this.logTxtBox.TabIndex = 0;
			this.logTxtBox.TabStop = false;
			this.logTxtBox.Text = "";
			// 
			// showLogBtn
			// 
			this.showLogBtn.Dock = System.Windows.Forms.DockStyle.Right;
			this.showLogBtn.Location = new System.Drawing.Point(616, 0);
			this.showLogBtn.Name = "showLogBtn";
			this.showLogBtn.Size = new System.Drawing.Size(48, 24);
			this.showLogBtn.TabIndex = 1;
			this.showLogBtn.TabStop = false;
			this.showLogBtn.Text = "Log";
			this.showLogBtn.Click += new System.EventHandler(this.showLogBtn_Click);
			// 
			// tabctrlMain
			// 
			this.tabctrlMain.Controls.Add(this.tabFiles);
			this.tabctrlMain.Controls.Add(this.tabAtributes);
			this.tabctrlMain.Controls.Add(this.tabConversion);
			this.tabctrlMain.Controls.Add(this.tabVerify);
			this.tabctrlMain.Controls.Add(this.tabInformation);
			this.tabctrlMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabctrlMain.Location = new System.Drawing.Point(0, 80);
			this.tabctrlMain.Name = "tabctrlMain";
			this.tabctrlMain.SelectedIndex = 0;
			this.tabctrlMain.Size = new System.Drawing.Size(664, 477);
			this.tabctrlMain.TabIndex = 0;
			// 
			// tabFiles
			// 
			this.tabFiles.Controls.Add(this.panelStepOne);
			this.tabFiles.Controls.Add(this.grpFileQuality);
			this.tabFiles.Location = new System.Drawing.Point(4, 22);
			this.tabFiles.Name = "tabFiles";
			this.tabFiles.Size = new System.Drawing.Size(656, 451);
			this.tabFiles.TabIndex = 0;
			this.tabFiles.Text = "Files";
			// 
			// panelStepOne
			// 
			this.panelStepOne.Controls.Add(this.grpSelectIcon);
			this.panelStepOne.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panelStepOne.Location = new System.Drawing.Point(0, 112);
			this.panelStepOne.Name = "panelStepOne";
			this.panelStepOne.Size = new System.Drawing.Size(656, 339);
			this.panelStepOne.TabIndex = 22;
			// 
			// grpSelectIcon
			// 
			this.grpSelectIcon.Controls.Add(this.label1);
			this.grpSelectIcon.Controls.Add(this.folderBtn);
			this.grpSelectIcon.Controls.Add(this.inputFolderTxtBox);
			this.grpSelectIcon.Controls.Add(this.previewBox);
			this.grpSelectIcon.Controls.Add(this.groupBox1);
			this.grpSelectIcon.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpSelectIcon.Location = new System.Drawing.Point(0, 0);
			this.grpSelectIcon.Name = "grpSelectIcon";
			this.grpSelectIcon.Size = new System.Drawing.Size(656, 339);
			this.grpSelectIcon.TabIndex = 7;
			this.grpSelectIcon.TabStop = false;
			this.grpSelectIcon.Text = "Select Object Bitmap (*.bmp) Icon";
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(32, 40);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(80, 24);
			this.label1.TabIndex = 2;
			this.label1.Text = "Image Folder:";
			this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// folderBtn
			// 
			this.folderBtn.Location = new System.Drawing.Point(584, 40);
			this.folderBtn.Name = "folderBtn";
			this.folderBtn.Size = new System.Drawing.Size(48, 20);
			this.folderBtn.TabIndex = 1;
			this.folderBtn.Text = "...";
			this.folderBtn.Click += new System.EventHandler(this.folderBtn_Click);
			// 
			// inputFolderTxtBox
			// 
			this.inputFolderTxtBox.Location = new System.Drawing.Point(128, 40);
			this.inputFolderTxtBox.Name = "inputFolderTxtBox";
			this.inputFolderTxtBox.Size = new System.Drawing.Size(440, 20);
			this.inputFolderTxtBox.TabIndex = 0;
			this.inputFolderTxtBox.Text = "";
			// 
			// previewBox
			// 
			this.previewBox.Controls.Add(this.picPreview);
			this.previewBox.Location = new System.Drawing.Point(32, 80);
			this.previewBox.Name = "previewBox";
			this.previewBox.Size = new System.Drawing.Size(144, 160);
			this.previewBox.TabIndex = 5;
			this.previewBox.TabStop = false;
			this.previewBox.Text = "Preview";
			// 
			// picPreview
			// 
			this.picPreview.Dock = System.Windows.Forms.DockStyle.Fill;
			this.picPreview.Location = new System.Drawing.Point(3, 16);
			this.picPreview.Name = "picPreview";
			this.picPreview.Size = new System.Drawing.Size(138, 141);
			this.picPreview.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
			this.picPreview.TabIndex = 4;
			this.picPreview.TabStop = false;
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.imageList);
			this.groupBox1.Location = new System.Drawing.Point(192, 80);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(440, 160);
			this.groupBox1.TabIndex = 3;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Images Found";
			// 
			// imageList
			// 
			this.imageList.Location = new System.Drawing.Point(8, 16);
			this.imageList.Name = "imageList";
			this.imageList.Size = new System.Drawing.Size(424, 134);
			this.imageList.TabIndex = 0;
			this.imageList.SelectedIndexChanged += new System.EventHandler(this.imageList_SelectedIndexChanged);
			// 
			// grpFileQuality
			// 
			this.grpFileQuality.Controls.Add(this.btnBrowseHigh);
			this.grpFileQuality.Controls.Add(this.btnBrowseMed);
			this.grpFileQuality.Controls.Add(this.lblFileLow);
			this.grpFileQuality.Controls.Add(this.btnBrowseLow);
			this.grpFileQuality.Controls.Add(this.txtInputLow);
			this.grpFileQuality.Controls.Add(this.txtInputMedium);
			this.grpFileQuality.Controls.Add(this.lblFileMedium);
			this.grpFileQuality.Controls.Add(this.txtInputHigh);
			this.grpFileQuality.Controls.Add(this.lblFileHigh);
			this.grpFileQuality.Dock = System.Windows.Forms.DockStyle.Top;
			this.grpFileQuality.Location = new System.Drawing.Point(0, 0);
			this.grpFileQuality.Name = "grpFileQuality";
			this.grpFileQuality.Size = new System.Drawing.Size(656, 112);
			this.grpFileQuality.TabIndex = 17;
			this.grpFileQuality.TabStop = false;
			this.grpFileQuality.Text = "Input File [Quality]";
			// 
			// btnBrowseHigh
			// 
			this.btnBrowseHigh.Location = new System.Drawing.Point(592, 80);
			this.btnBrowseHigh.Name = "btnBrowseHigh";
			this.btnBrowseHigh.Size = new System.Drawing.Size(48, 20);
			this.btnBrowseHigh.TabIndex = 5;
			this.btnBrowseHigh.Text = "...";
			this.btnBrowseHigh.Click += new System.EventHandler(this.btnBrowseHigh_Click);
			// 
			// btnBrowseMed
			// 
			this.btnBrowseMed.Location = new System.Drawing.Point(592, 48);
			this.btnBrowseMed.Name = "btnBrowseMed";
			this.btnBrowseMed.Size = new System.Drawing.Size(48, 20);
			this.btnBrowseMed.TabIndex = 3;
			this.btnBrowseMed.Text = "...";
			this.btnBrowseMed.Click += new System.EventHandler(this.btnBrowseMed_Click);
			// 
			// lblFileLow
			// 
			this.lblFileLow.Location = new System.Drawing.Point(16, 16);
			this.lblFileLow.Name = "lblFileLow";
			this.lblFileLow.Size = new System.Drawing.Size(74, 20);
			this.lblFileLow.TabIndex = 2;
			this.lblFileLow.Text = "3D File [Low]";
			this.lblFileLow.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// btnBrowseLow
			// 
			this.btnBrowseLow.Location = new System.Drawing.Point(592, 16);
			this.btnBrowseLow.Name = "btnBrowseLow";
			this.btnBrowseLow.Size = new System.Drawing.Size(48, 20);
			this.btnBrowseLow.TabIndex = 1;
			this.btnBrowseLow.Text = "...";
			this.btnBrowseLow.Click += new System.EventHandler(this.btnBrowseLow_Click);
			// 
			// txtInputLow
			// 
			this.txtInputLow.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(255)), ((System.Byte)(255)), ((System.Byte)(192)));
			this.txtInputLow.Location = new System.Drawing.Point(112, 16);
			this.txtInputLow.Name = "txtInputLow";
			this.txtInputLow.Size = new System.Drawing.Size(464, 20);
			this.txtInputLow.TabIndex = 0;
			this.txtInputLow.Text = "";
			this.txtInputLow.TextChanged += new System.EventHandler(this.inputFileTextChanged);
			// 
			// txtInputMedium
			// 
			this.txtInputMedium.Location = new System.Drawing.Point(120, 48);
			this.txtInputMedium.Name = "txtInputMedium";
			this.txtInputMedium.Size = new System.Drawing.Size(456, 20);
			this.txtInputMedium.TabIndex = 2;
			this.txtInputMedium.Text = "";
			this.txtInputMedium.TextChanged += new System.EventHandler(this.inputFileTextChanged);
			// 
			// lblFileMedium
			// 
			this.lblFileMedium.Location = new System.Drawing.Point(16, 48);
			this.lblFileMedium.Name = "lblFileMedium";
			this.lblFileMedium.Size = new System.Drawing.Size(96, 20);
			this.lblFileMedium.TabIndex = 11;
			this.lblFileMedium.Text = "3D File [Medium]";
			this.lblFileMedium.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// txtInputHigh
			// 
			this.txtInputHigh.Location = new System.Drawing.Point(112, 80);
			this.txtInputHigh.Name = "txtInputHigh";
			this.txtInputHigh.Size = new System.Drawing.Size(464, 20);
			this.txtInputHigh.TabIndex = 4;
			this.txtInputHigh.Text = "";
			this.txtInputHigh.TextChanged += new System.EventHandler(this.inputFileTextChanged);
			// 
			// lblFileHigh
			// 
			this.lblFileHigh.Location = new System.Drawing.Point(16, 80);
			this.lblFileHigh.Name = "lblFileHigh";
			this.lblFileHigh.Size = new System.Drawing.Size(80, 20);
			this.lblFileHigh.TabIndex = 12;
			this.lblFileHigh.Text = "3D File [High]";
			this.lblFileHigh.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// tabAtributes
			// 
			this.tabAtributes.Controls.Add(this.panel3);
			this.tabAtributes.Controls.Add(this.grpOutput);
			this.tabAtributes.Location = new System.Drawing.Point(4, 22);
			this.tabAtributes.Name = "tabAtributes";
			this.tabAtributes.Size = new System.Drawing.Size(656, 451);
			this.tabAtributes.TabIndex = 1;
			this.tabAtributes.Text = "Attributes";
			// 
			// panel3
			// 
			this.panel3.Controls.Add(this.grpPropertiesAdvanced);
			this.panel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel3.Location = new System.Drawing.Point(0, 168);
			this.panel3.Name = "panel3";
			this.panel3.Size = new System.Drawing.Size(656, 283);
			this.panel3.TabIndex = 22;
			// 
			// grpPropertiesAdvanced
			// 
			this.grpPropertiesAdvanced.Controls.Add(this.tabctrlAdvancedAttributes);
			this.grpPropertiesAdvanced.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpPropertiesAdvanced.Location = new System.Drawing.Point(0, 0);
			this.grpPropertiesAdvanced.Name = "grpPropertiesAdvanced";
			this.grpPropertiesAdvanced.Size = new System.Drawing.Size(656, 283);
			this.grpPropertiesAdvanced.TabIndex = 0;
			this.grpPropertiesAdvanced.TabStop = false;
			this.grpPropertiesAdvanced.Text = "Advanced Object Properties";
			// 
			// tabctrlAdvancedAttributes
			// 
			this.tabctrlAdvancedAttributes.Controls.Add(this.tabObjectDynamics);
			this.tabctrlAdvancedAttributes.Controls.Add(this.tabObjectPhysics);
			this.tabctrlAdvancedAttributes.Controls.Add(this.tabObjectScripts);
			this.tabctrlAdvancedAttributes.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabctrlAdvancedAttributes.Location = new System.Drawing.Point(3, 16);
			this.tabctrlAdvancedAttributes.Name = "tabctrlAdvancedAttributes";
			this.tabctrlAdvancedAttributes.SelectedIndex = 0;
			this.tabctrlAdvancedAttributes.Size = new System.Drawing.Size(650, 264);
			this.tabctrlAdvancedAttributes.TabIndex = 0;
			// 
			// tabObjectDynamics
			// 
			this.tabObjectDynamics.Controls.Add(this.typeLbl);
			this.tabObjectDynamics.Controls.Add(this.objectTypeCmbBox);
			this.tabObjectDynamics.Location = new System.Drawing.Point(4, 22);
			this.tabObjectDynamics.Name = "tabObjectDynamics";
			this.tabObjectDynamics.Size = new System.Drawing.Size(642, 238);
			this.tabObjectDynamics.TabIndex = 0;
			this.tabObjectDynamics.Text = "Dynamics";
			// 
			// typeLbl
			// 
			this.typeLbl.Location = new System.Drawing.Point(8, 8);
			this.typeLbl.Name = "typeLbl";
			this.typeLbl.Size = new System.Drawing.Size(72, 20);
			this.typeLbl.TabIndex = 9;
			this.typeLbl.Text = "Object Type:";
			this.typeLbl.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// objectTypeCmbBox
			// 
			this.objectTypeCmbBox.Items.AddRange(new object[] {
																  "Static",
																  "Usable",
																  "Programmable"});
			this.objectTypeCmbBox.Location = new System.Drawing.Point(96, 8);
			this.objectTypeCmbBox.Name = "objectTypeCmbBox";
			this.objectTypeCmbBox.Size = new System.Drawing.Size(208, 21);
			this.objectTypeCmbBox.TabIndex = 8;
			this.objectTypeCmbBox.Text = "Static";
			// 
			// tabObjectPhysics
			// 
			this.tabObjectPhysics.Controls.Add(this.cmdOpenPhys);
			this.tabObjectPhysics.Location = new System.Drawing.Point(4, 22);
			this.tabObjectPhysics.Name = "tabObjectPhysics";
			this.tabObjectPhysics.Size = new System.Drawing.Size(642, 238);
			this.tabObjectPhysics.TabIndex = 1;
			this.tabObjectPhysics.Text = "Physics";
			// 
			// tabObjectScripts
			// 
			this.tabObjectScripts.Location = new System.Drawing.Point(4, 22);
			this.tabObjectScripts.Name = "tabObjectScripts";
			this.tabObjectScripts.Size = new System.Drawing.Size(642, 238);
			this.tabObjectScripts.TabIndex = 2;
			this.tabObjectScripts.Text = "Scripts";
			// 
			// grpOutput
			// 
			this.grpOutput.Controls.Add(this.txtAuthor);
			this.grpOutput.Controls.Add(this.lblAuthor);
			this.grpOutput.Controls.Add(this.objLbl);
			this.grpOutput.Controls.Add(this.objectNameTxtBox);
			this.grpOutput.Controls.Add(this.commentRtfBox);
			this.grpOutput.Controls.Add(this.commentLbl);
			this.grpOutput.Dock = System.Windows.Forms.DockStyle.Top;
			this.grpOutput.Location = new System.Drawing.Point(0, 0);
			this.grpOutput.Name = "grpOutput";
			this.grpOutput.Size = new System.Drawing.Size(656, 168);
			this.grpOutput.TabIndex = 20;
			this.grpOutput.TabStop = false;
			this.grpOutput.Text = "Basic Object Properties";
			// 
			// txtAuthor
			// 
			this.txtAuthor.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(255)), ((System.Byte)(255)), ((System.Byte)(192)));
			this.txtAuthor.Location = new System.Drawing.Point(104, 64);
			this.txtAuthor.Name = "txtAuthor";
			this.txtAuthor.Size = new System.Drawing.Size(520, 20);
			this.txtAuthor.TabIndex = 1;
			this.txtAuthor.Text = "";
			// 
			// lblAuthor
			// 
			this.lblAuthor.Location = new System.Drawing.Point(16, 64);
			this.lblAuthor.Name = "lblAuthor";
			this.lblAuthor.Size = new System.Drawing.Size(80, 24);
			this.lblAuthor.TabIndex = 8;
			this.lblAuthor.Text = "Author:";
			this.lblAuthor.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// objLbl
			// 
			this.objLbl.Location = new System.Drawing.Point(16, 32);
			this.objLbl.Name = "objLbl";
			this.objLbl.Size = new System.Drawing.Size(80, 24);
			this.objLbl.TabIndex = 5;
			this.objLbl.Text = "Object Name:";
			this.objLbl.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// objectNameTxtBox
			// 
			this.objectNameTxtBox.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(255)), ((System.Byte)(255)), ((System.Byte)(192)));
			this.objectNameTxtBox.Location = new System.Drawing.Point(104, 32);
			this.objectNameTxtBox.Name = "objectNameTxtBox";
			this.objectNameTxtBox.Size = new System.Drawing.Size(520, 20);
			this.objectNameTxtBox.TabIndex = 0;
			this.objectNameTxtBox.Text = "";
			// 
			// commentRtfBox
			// 
			this.commentRtfBox.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(255)), ((System.Byte)(255)), ((System.Byte)(192)));
			this.commentRtfBox.Location = new System.Drawing.Point(104, 96);
			this.commentRtfBox.Name = "commentRtfBox";
			this.commentRtfBox.Size = new System.Drawing.Size(520, 56);
			this.commentRtfBox.TabIndex = 2;
			this.commentRtfBox.Text = "";
			// 
			// commentLbl
			// 
			this.commentLbl.Location = new System.Drawing.Point(16, 96);
			this.commentLbl.Name = "commentLbl";
			this.commentLbl.Size = new System.Drawing.Size(80, 24);
			this.commentLbl.TabIndex = 6;
			this.commentLbl.Text = "Comments:";
			this.commentLbl.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// tabConversion
			// 
			this.tabConversion.Controls.Add(this.grpConfigurations);
			this.tabConversion.Location = new System.Drawing.Point(4, 22);
			this.tabConversion.Name = "tabConversion";
			this.tabConversion.Size = new System.Drawing.Size(656, 451);
			this.tabConversion.TabIndex = 3;
			this.tabConversion.Text = "Conversion";
			// 
			// grpConfigurations
			// 
			this.grpConfigurations.Controls.Add(this.label4);
			this.grpConfigurations.Controls.Add(this.pb1);
			this.grpConfigurations.Controls.Add(this.lblCommand);
			this.grpConfigurations.Controls.Add(this.txtCommandToRun);
			this.grpConfigurations.Controls.Add(this.btnConverter);
			this.grpConfigurations.Controls.Add(this.lblConverter);
			this.grpConfigurations.Controls.Add(this.txtConverter);
			this.grpConfigurations.Controls.Add(this.btnGenerateRaw);
			this.grpConfigurations.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpConfigurations.Location = new System.Drawing.Point(0, 0);
			this.grpConfigurations.Name = "grpConfigurations";
			this.grpConfigurations.Size = new System.Drawing.Size(656, 451);
			this.grpConfigurations.TabIndex = 22;
			this.grpConfigurations.TabStop = false;
			this.grpConfigurations.Text = "Raw Binary 3D Converter Program Settings";
			// 
			// pb1
			// 
			this.pb1.Location = new System.Drawing.Point(64, 248);
			this.pb1.Name = "pb1";
			this.pb1.Size = new System.Drawing.Size(280, 184);
			this.pb1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
			this.pb1.TabIndex = 5;
			this.pb1.TabStop = false;
			// 
			// lblCommand
			// 
			this.lblCommand.Location = new System.Drawing.Point(32, 72);
			this.lblCommand.Name = "lblCommand";
			this.lblCommand.Size = new System.Drawing.Size(96, 48);
			this.lblCommand.TabIndex = 4;
			this.lblCommand.Text = "Running Command";
			this.lblCommand.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// txtCommandToRun
			// 
			this.txtCommandToRun.Location = new System.Drawing.Point(152, 64);
			this.txtCommandToRun.Multiline = true;
			this.txtCommandToRun.Name = "txtCommandToRun";
			this.txtCommandToRun.ReadOnly = true;
			this.txtCommandToRun.Size = new System.Drawing.Size(456, 136);
			this.txtCommandToRun.TabIndex = 3;
			this.txtCommandToRun.TabStop = false;
			this.txtCommandToRun.Text = "";
			// 
			// btnConverter
			// 
			this.btnConverter.Location = new System.Drawing.Point(560, 24);
			this.btnConverter.Name = "btnConverter";
			this.btnConverter.Size = new System.Drawing.Size(48, 20);
			this.btnConverter.TabIndex = 1;
			this.btnConverter.Text = "...";
			this.btnConverter.Enter += new System.EventHandler(this.btnConverter_Click);
			// 
			// lblConverter
			// 
			this.lblConverter.Location = new System.Drawing.Point(32, 24);
			this.lblConverter.Name = "lblConverter";
			this.lblConverter.Size = new System.Drawing.Size(104, 20);
			this.lblConverter.TabIndex = 1;
			this.lblConverter.Text = "Converter";
			this.lblConverter.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// txtConverter
			// 
			this.txtConverter.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(255)), ((System.Byte)(255)), ((System.Byte)(192)));
			this.txtConverter.Location = new System.Drawing.Point(152, 24);
			this.txtConverter.Name = "txtConverter";
			this.txtConverter.Size = new System.Drawing.Size(384, 20);
			this.txtConverter.TabIndex = 0;
			this.txtConverter.Text = "";
			// 
			// btnGenerateRaw
			// 
			this.btnGenerateRaw.Enabled = false;
			this.btnGenerateRaw.Location = new System.Drawing.Point(472, 208);
			this.btnGenerateRaw.Name = "btnGenerateRaw";
			this.btnGenerateRaw.Size = new System.Drawing.Size(136, 72);
			this.btnGenerateRaw.TabIndex = 2;
			this.btnGenerateRaw.Text = "Generate Raw";
			this.btnGenerateRaw.Click += new System.EventHandler(this.btnGenerateRaw_Click);
			this.btnGenerateRaw.MouseHover += new System.EventHandler(this.btnGenerateRaw_MouseHover);
			// 
			// tabVerify
			// 
			this.tabVerify.Controls.Add(this.grpVerifyContents);
			this.tabVerify.Controls.Add(this.grpVerifyInput);
			this.tabVerify.Location = new System.Drawing.Point(4, 22);
			this.tabVerify.Name = "tabVerify";
			this.tabVerify.Size = new System.Drawing.Size(656, 451);
			this.tabVerify.TabIndex = 4;
			this.tabVerify.Text = "Verify";
			// 
			// grpVerifyContents
			// 
			this.grpVerifyContents.Controls.Add(this.lblVerifyTextures);
			this.grpVerifyContents.Controls.Add(this.lblVerifyFileSize);
			this.grpVerifyContents.Controls.Add(this.lblVerifyCount);
			this.grpVerifyContents.Controls.Add(this.lblVerifyObjectName);
			this.grpVerifyContents.Controls.Add(this.txtVerifyTextureList);
			this.grpVerifyContents.Controls.Add(this.textBox3);
			this.grpVerifyContents.Controls.Add(this.textBox2);
			this.grpVerifyContents.Controls.Add(this.txtVerifyObjectName);
			this.grpVerifyContents.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grpVerifyContents.Location = new System.Drawing.Point(0, 112);
			this.grpVerifyContents.Name = "grpVerifyContents";
			this.grpVerifyContents.Size = new System.Drawing.Size(656, 339);
			this.grpVerifyContents.TabIndex = 4;
			this.grpVerifyContents.TabStop = false;
			this.grpVerifyContents.Text = "Contents";
			// 
			// lblVerifyTextures
			// 
			this.lblVerifyTextures.Location = new System.Drawing.Point(16, 88);
			this.lblVerifyTextures.Name = "lblVerifyTextures";
			this.lblVerifyTextures.Size = new System.Drawing.Size(80, 24);
			this.lblVerifyTextures.TabIndex = 7;
			this.lblVerifyTextures.Text = "Texture List";
			this.lblVerifyTextures.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lblVerifyFileSize
			// 
			this.lblVerifyFileSize.Location = new System.Drawing.Point(232, 48);
			this.lblVerifyFileSize.Name = "lblVerifyFileSize";
			this.lblVerifyFileSize.Size = new System.Drawing.Size(72, 24);
			this.lblVerifyFileSize.TabIndex = 6;
			this.lblVerifyFileSize.Text = "File Size";
			this.lblVerifyFileSize.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lblVerifyCount
			// 
			this.lblVerifyCount.Location = new System.Drawing.Point(16, 48);
			this.lblVerifyCount.Name = "lblVerifyCount";
			this.lblVerifyCount.Size = new System.Drawing.Size(80, 24);
			this.lblVerifyCount.TabIndex = 5;
			this.lblVerifyCount.Text = "Triangle Count";
			// 
			// lblVerifyObjectName
			// 
			this.lblVerifyObjectName.Location = new System.Drawing.Point(16, 16);
			this.lblVerifyObjectName.Name = "lblVerifyObjectName";
			this.lblVerifyObjectName.Size = new System.Drawing.Size(80, 16);
			this.lblVerifyObjectName.TabIndex = 4;
			this.lblVerifyObjectName.Text = "Object Name:";
			this.lblVerifyObjectName.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// txtVerifyTextureList
			// 
			this.txtVerifyTextureList.Location = new System.Drawing.Point(104, 88);
			this.txtVerifyTextureList.Multiline = true;
			this.txtVerifyTextureList.Name = "txtVerifyTextureList";
			this.txtVerifyTextureList.ReadOnly = true;
			this.txtVerifyTextureList.Size = new System.Drawing.Size(408, 96);
			this.txtVerifyTextureList.TabIndex = 3;
			this.txtVerifyTextureList.Text = "";
			// 
			// textBox3
			// 
			this.textBox3.Location = new System.Drawing.Point(312, 48);
			this.textBox3.Name = "textBox3";
			this.textBox3.ReadOnly = true;
			this.textBox3.Size = new System.Drawing.Size(200, 20);
			this.textBox3.TabIndex = 2;
			this.textBox3.Text = "";
			// 
			// textBox2
			// 
			this.textBox2.Location = new System.Drawing.Point(112, 48);
			this.textBox2.Name = "textBox2";
			this.textBox2.ReadOnly = true;
			this.textBox2.Size = new System.Drawing.Size(112, 20);
			this.textBox2.TabIndex = 1;
			this.textBox2.Text = "";
			// 
			// txtVerifyObjectName
			// 
			this.txtVerifyObjectName.Location = new System.Drawing.Point(112, 16);
			this.txtVerifyObjectName.Name = "txtVerifyObjectName";
			this.txtVerifyObjectName.ReadOnly = true;
			this.txtVerifyObjectName.Size = new System.Drawing.Size(400, 20);
			this.txtVerifyObjectName.TabIndex = 0;
			this.txtVerifyObjectName.Text = "";
			// 
			// grpVerifyInput
			// 
			this.grpVerifyInput.Controls.Add(this.btnVerifyEnact);
			this.grpVerifyInput.Controls.Add(this.label3);
			this.grpVerifyInput.Controls.Add(this.txtVerifyFile);
			this.grpVerifyInput.Controls.Add(this.btnVerifyFile);
			this.grpVerifyInput.Dock = System.Windows.Forms.DockStyle.Top;
			this.grpVerifyInput.Location = new System.Drawing.Point(0, 0);
			this.grpVerifyInput.Name = "grpVerifyInput";
			this.grpVerifyInput.Size = new System.Drawing.Size(656, 112);
			this.grpVerifyInput.TabIndex = 3;
			this.grpVerifyInput.TabStop = false;
			this.grpVerifyInput.Text = "Verify File";
			// 
			// btnVerifyEnact
			// 
			this.btnVerifyEnact.Location = new System.Drawing.Point(136, 72);
			this.btnVerifyEnact.Name = "btnVerifyEnact";
			this.btnVerifyEnact.TabIndex = 3;
			this.btnVerifyEnact.Text = "Verify !";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(16, 32);
			this.label3.Name = "label3";
			this.label3.TabIndex = 0;
			this.label3.Text = "3DBIN File";
			this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// txtVerifyFile
			// 
			this.txtVerifyFile.Location = new System.Drawing.Point(136, 32);
			this.txtVerifyFile.Name = "txtVerifyFile";
			this.txtVerifyFile.Size = new System.Drawing.Size(448, 20);
			this.txtVerifyFile.TabIndex = 1;
			this.txtVerifyFile.Text = "";
			// 
			// btnVerifyFile
			// 
			this.btnVerifyFile.Location = new System.Drawing.Point(608, 32);
			this.btnVerifyFile.Name = "btnVerifyFile";
			this.btnVerifyFile.Size = new System.Drawing.Size(40, 23);
			this.btnVerifyFile.TabIndex = 2;
			this.btnVerifyFile.Text = "...";
			// 
			// tabInformation
			// 
			this.tabInformation.Controls.Add(this.tempPb1);
			this.tabInformation.Controls.Add(this.tempPb2);
			this.tabInformation.Controls.Add(this.grpInformationAuthors);
			this.tabInformation.Controls.Add(this.lblInfoVRUPL);
			this.tabInformation.Location = new System.Drawing.Point(4, 22);
			this.tabInformation.Name = "tabInformation";
			this.tabInformation.Size = new System.Drawing.Size(656, 451);
			this.tabInformation.TabIndex = 2;
			this.tabInformation.Text = "Information";
			// 
			// tempPb1
			// 
			this.tempPb1.Location = new System.Drawing.Point(392, 8);
			this.tempPb1.Name = "tempPb1";
			this.tempPb1.Size = new System.Drawing.Size(256, 152);
			this.tempPb1.TabIndex = 5;
			this.tempPb1.TabStop = false;
			this.tempPb1.Visible = false;
			// 
			// tempPb2
			// 
			this.tempPb2.Location = new System.Drawing.Point(8, 8);
			this.tempPb2.Name = "tempPb2";
			this.tempPb2.Size = new System.Drawing.Size(248, 168);
			this.tempPb2.TabIndex = 4;
			this.tempPb2.TabStop = false;
			this.tempPb2.Visible = false;
			// 
			// grpInformationAuthors
			// 
			this.grpInformationAuthors.Controls.Add(this.lblInformationAuthors);
			this.grpInformationAuthors.Location = new System.Drawing.Point(408, 216);
			this.grpInformationAuthors.Name = "grpInformationAuthors";
			this.grpInformationAuthors.Size = new System.Drawing.Size(208, 152);
			this.grpInformationAuthors.TabIndex = 3;
			this.grpInformationAuthors.TabStop = false;
			this.grpInformationAuthors.Text = "Authors";
			// 
			// lblInformationAuthors
			// 
			this.lblInformationAuthors.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lblInformationAuthors.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblInformationAuthors.Location = new System.Drawing.Point(3, 16);
			this.lblInformationAuthors.Name = "lblInformationAuthors";
			this.lblInformationAuthors.Size = new System.Drawing.Size(202, 133);
			this.lblInformationAuthors.TabIndex = 0;
			this.lblInformationAuthors.Text = "\"3D to Holodeck Packer\" created by Maksim Fridberg, Wiktor Kopec, and Mark Tulewi" +
				"cz for the Virtual Reality Undergraduate reaserch Project Laboratories";
			this.lblInformationAuthors.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lblInfoVRUPL
			// 
			this.lblInfoVRUPL.Font = new System.Drawing.Font("Microsoft Sans Serif", 72F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblInfoVRUPL.Location = new System.Drawing.Point(16, 240);
			this.lblInfoVRUPL.Name = "lblInfoVRUPL";
			this.lblInfoVRUPL.Size = new System.Drawing.Size(376, 96);
			this.lblInfoVRUPL.TabIndex = 2;
			this.lblInfoVRUPL.Text = "VRUPL";
			this.lblInfoVRUPL.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// grpPath
			// 
			this.grpPath.Controls.Add(this.txtProjectName);
			this.grpPath.Controls.Add(this.label2);
			this.grpPath.Controls.Add(this.txtPath);
			this.grpPath.Controls.Add(this.btnBrowseOutpath);
			this.grpPath.Controls.Add(this.lblOutPath);
			this.grpPath.Dock = System.Windows.Forms.DockStyle.Top;
			this.grpPath.Location = new System.Drawing.Point(0, 0);
			this.grpPath.Name = "grpPath";
			this.grpPath.Size = new System.Drawing.Size(664, 80);
			this.grpPath.TabIndex = 20;
			this.grpPath.TabStop = false;
			this.grpPath.Text = "Global Settings";
			// 
			// txtProjectName
			// 
			this.txtProjectName.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(255)), ((System.Byte)(255)), ((System.Byte)(192)));
			this.txtProjectName.Location = new System.Drawing.Point(104, 24);
			this.txtProjectName.Name = "txtProjectName";
			this.txtProjectName.Size = new System.Drawing.Size(464, 20);
			this.txtProjectName.TabIndex = 0;
			this.txtProjectName.Text = "";
			this.txtProjectName.TextChanged += new System.EventHandler(this.reqTextChanged);
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(16, 24);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(80, 20);
			this.label2.TabIndex = 17;
			this.label2.Text = "Project Name";
			this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// txtPath
			// 
			this.txtPath.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(255)), ((System.Byte)(255)), ((System.Byte)(192)));
			this.txtPath.Location = new System.Drawing.Point(104, 48);
			this.txtPath.Name = "txtPath";
			this.txtPath.Size = new System.Drawing.Size(464, 20);
			this.txtPath.TabIndex = 1;
			this.txtPath.Text = "";
			this.txtPath.TextChanged += new System.EventHandler(this.reqTextChanged);
			// 
			// btnBrowseOutpath
			// 
			this.btnBrowseOutpath.Location = new System.Drawing.Point(584, 48);
			this.btnBrowseOutpath.Name = "btnBrowseOutpath";
			this.btnBrowseOutpath.Size = new System.Drawing.Size(48, 20);
			this.btnBrowseOutpath.TabIndex = 2;
			this.btnBrowseOutpath.Text = "...";
			this.btnBrowseOutpath.Click += new System.EventHandler(this.btnBrowseOutpath_Click);
			// 
			// lblOutPath
			// 
			this.lblOutPath.Location = new System.Drawing.Point(16, 48);
			this.lblOutPath.Name = "lblOutPath";
			this.lblOutPath.Size = new System.Drawing.Size(72, 20);
			this.lblOutPath.TabIndex = 16;
			this.lblOutPath.Text = "Project Path";
			this.lblOutPath.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.logTxtBox);
			this.panel1.Controls.Add(this.showLogBtn);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel1.Location = new System.Drawing.Point(0, 557);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(664, 24);
			this.panel1.TabIndex = 3;
			// 
			// panel2
			// 
			this.panel2.Controls.Add(this.tabctrlMain);
			this.panel2.Controls.Add(this.grpPath);
			this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel2.Location = new System.Drawing.Point(0, 0);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(664, 557);
			this.panel2.TabIndex = 4;
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(40, 216);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(77, 16);
			this.label4.TabIndex = 6;
			this.label4.Text = "Sample Image";
			// 
			// cmdOpenPhys
			// 
			this.cmdOpenPhys.Location = new System.Drawing.Point(280, 96);
			this.cmdOpenPhys.Name = "cmdOpenPhys";
			this.cmdOpenPhys.TabIndex = 0;
			this.cmdOpenPhys.Text = "Beta Run";
			this.cmdOpenPhys.Click += new System.EventHandler(this.cmdOpenPhys_Click);
			// 
			// mainForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(664, 581);
			this.Controls.Add(this.panel2);
			this.Controls.Add(this.panel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.Name = "mainForm";
			this.Text = "3D to Holodeck Packer";
			this.Load += new System.EventHandler(this.mainForm_Load);
			this.tabctrlMain.ResumeLayout(false);
			this.tabFiles.ResumeLayout(false);
			this.panelStepOne.ResumeLayout(false);
			this.grpSelectIcon.ResumeLayout(false);
			this.previewBox.ResumeLayout(false);
			this.groupBox1.ResumeLayout(false);
			this.grpFileQuality.ResumeLayout(false);
			this.tabAtributes.ResumeLayout(false);
			this.panel3.ResumeLayout(false);
			this.grpPropertiesAdvanced.ResumeLayout(false);
			this.tabctrlAdvancedAttributes.ResumeLayout(false);
			this.tabObjectDynamics.ResumeLayout(false);
			this.tabObjectPhysics.ResumeLayout(false);
			this.grpOutput.ResumeLayout(false);
			this.tabConversion.ResumeLayout(false);
			this.grpConfigurations.ResumeLayout(false);
			this.tabVerify.ResumeLayout(false);
			this.grpVerifyContents.ResumeLayout(false);
			this.grpVerifyInput.ResumeLayout(false);
			this.tabInformation.ResumeLayout(false);
			this.grpInformationAuthors.ResumeLayout(false);
			this.grpPath.ResumeLayout(false);
			this.panel1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		[STAThread]
		static void Main()
		{

			Application.Run(new mainForm());
		}
		
		/*
		 * Main functions
		 * /

		/*---------------------------------------------------*/
		
		private void GenerateX3DML(string infile, string type, string name, string comment)
		{			
			this.outFile = new StreamWriter(this.txtPath.Text + "\\" + name + ".x3dml");

			this.outFile.WriteLine("<filename>" + name + "x3dml" + "</filename>");
			this.outFile.WriteLine("<object type=\"" + type + "\"</object>");
			this.outFile.WriteLine("<objectname>" + name + "</objectname>");
			this.outFile.WriteLine("<comment>" + comment + "</comment>");
			this.outFile.Flush();
			this.outFile.Close();
		}
		
		/*---------------------------------------------------*/

		private void GenerateHeader(string path, string name, string objectName, int lodCount, string comments, ObjectType objectType)
		{
			this.binFile = new BinaryWriter(File.Open(path + "\\" + name + binExtension, FileMode.Create, FileAccess.Write, FileShare.ReadWrite));

			/*Magic*/
			this.binFile.Write(4);
			this.binFile.Write(2);

			/*Header*/
			if ( comments == "" )
			{
				comments = " ";
			}
			
			if (objectName == "" )
			{
				objectName = "x";
			}

			this.binFile.Write(GenerateName(objectName, nameLength));
			this.binFile.Write(GenerateName(comments, commentLength));
			this.binFile.Write(lodCount);
			this.binFile.Write((int)objectType);
			
			this.binFile.Flush();
			this.binFile.Close();
		}
		
		/*---------------------------------------------------*/
		
		private void Generate3DBin(string processName, string args)
		{																
			ProcessStartInfo info = new ProcessStartInfo(processName, args);

			info.UseShellExecute = false;
			info.RedirectStandardOutput = true;
			info.RedirectStandardError = true;
				
			Process process = Process.Start(info);			
			
			while (true)
			{
				if (!process.HasExited)
				{
					process.WaitForExit(100);
				}
				else
				{
					break;
				}
			}								
							
			ProcessStream(process.StandardError);
			ProcessStream(process.StandardOutput);
				
			if (process.ExitCode != 0)
			{
				MessageBox.Show("Failed");					
			}
			else
			{
				MessageBox.Show("Succeeded");
			}										
		}
		
		/*---------------------------------------------------*/
		
		private void GenerateTextures(string path, string projectName, int lodCount)
		{
			this.binFile = new BinaryWriter(File.Open(path + "\\" + projectName + binExtension, FileMode.Append, FileAccess.Write, FileShare.None));

			StreamReader [] reader = new StreamReader[3];	
			string [] extList = new string[3]{".lo", ".med", ".hi"};
												
			int readerCount = 0;			

			for (int i = 0; i < lodCount; i++)
			{
				string textureFile = path + "\\" + projectName + txtExtension + extList[i];
				
				if (File.Exists(textureFile))
				{
					reader[i] = new StreamReader(textureFile);
					readerCount++;
				}
				else
				{
					MessageBox.Show("Inconsistency between requested levels of detail and output texture files");
				}
			}
			
			binFile.Write((float)3.14f);
			
			for (int i = 0; i < readerCount; i++)
			{				
				if (reader[i] != null)
				{
					int count = 0;
					string texture = null;
										
					count = int.Parse(reader[i].ReadLine());
					
					if (count > 0)
					{						
						binFile.Write(count);
						binFile.Flush();
						
						for (int j = 0; j < count; j++)
						{							
							texture = reader[i].ReadLine();
							
							int firstq = texture.IndexOf('\"');
 
							int secondq = texture.IndexOf('\"', firstq + 1, texture.Length - firstq - 1);

							string tindex = texture.Substring(firstq + 1, secondq - firstq - 1);

							int tid = int.Parse(tindex);
							
							int firstindex = texture.IndexOf(">");
							
							if (firstindex > -1)
							{
								int lastindex = texture.IndexOf("</texture>");
								string textureFile = texture.Substring(firstindex + 2, lastindex - firstindex - 3);
								int ind = txtInputLow.Text.LastIndexOf("\\");
								string textureLoc = txtInputLow.Text.Substring(0,ind);
								
								if ( textureFile != "" )
								{
									GenerateTextureData(tid, textureLoc + "\\" + textureFile, projectName);
								}
							}
							else
							{
								MessageBox.Show("Error, texture table is missing entries");
							}
						}
					}
					else
					{
						MessageBox.Show("Error, texture count is <= 0");
					}
				}
				else
				{
					MessageBox.Show("Sanity check");
				}
			}
			
			binFile.Flush();
			binFile.Close();
		}
		

		/*---------------------------------------------------*/
		
		private void GenerateTextureData(int id, string textureIn, string textureOut)
		{
			Bitmap bitmap;

			try
			{
				bitmap = (Bitmap)Bitmap.FromFile(textureIn);				
			}
			catch
			{
				MessageBox.Show("Could not find texture file: " + textureIn);
				return;
			}

			Graphics g = Graphics.FromImage(bitmap);
			
//			int newWidth = GenerateNewSize(bitmap.Width);
//			int newHeight = GenerateNewSize(bitmap.Height);
			
			int newWidth = bitmap.Width;
			int newHeight = bitmap.Height;


			//g.ScaleTransform((float)newWidth / (float)bitmap.Width, (float)newHeight / (float)bitmap.Height, System.Drawing.Drawing2D.MatrixOrder.Append);
			g.ScaleTransform((float)bitmap.Width, (float)bitmap.Height, System.Drawing.Drawing2D.MatrixOrder.Append);
			g.DrawImage(bitmap, 0, 0, (float)bitmap.Width, (float)bitmap.Height);
			
			BitmapData bitmapData = bitmap.LockBits(new Rectangle(0, 0,bitmap.Width, bitmap.Height), ImageLockMode.ReadWrite, PixelFormat.Format32bppArgb);
			
			
			//System.Runtime.InteropServices.Marshal.Copy(bitmapData.Scan0, buffer, 0, bitmap.Width * bitmap.Height * 4);
			
			Bitmap newBitmap = new Bitmap(newWidth, newHeight, PixelFormat.Format32bppArgb);			
			
			IntPtr source = bitmapData.Scan0;
			int sourceNum = source.ToInt32();

			byte [] buffer = new byte[newHeight * newWidth * 4];
			int offset = (bitmap.Height-1) * bitmapData.Stride;						

			//	If the height is positive the DIB are stored upside down. That means that the uppest row 
			//	which appears on the screen actually is the lowest row stored in the bitmap 					
			//	if it is negative, if it stored in the way arround.			
			if ( bitmap.Height > 0 ) 
			{				
				offset = (bitmap.Height - 1) * bitmapData.Stride;														
				while(offset >= 0)
				{									
					System.Runtime.InteropServices.Marshal.Copy(source, buffer, offset, bitmapData.Stride);						
					offset -= bitmapData.Stride;	
					source = (IntPtr)(sourceNum + bitmapData.Stride);
					sourceNum += bitmapData.Stride;
				}
			}										
			else 
			{
				offset = 0;
				for (int lines = 0; lines < bitmap.Height ; lines++)
				{
					System.Runtime.InteropServices.Marshal.Copy(bitmapData.Scan0, buffer, offset, bitmapData.Stride);							
					offset += bitmapData.Stride;
				}										
			}			

			int counter = 0;
			
			ProcessTextureBuffer(buffer, newWidth, newHeight);
					
			for (int j = 0; j < newBitmap.Height; j++)			
			{
				for (int i = 0; i < newBitmap.Width; i++)
				{
					newBitmap.SetPixel(i, j, Color.FromArgb(buffer[counter + 3], buffer[counter], buffer[counter + 1], buffer[counter + 2]));
					counter += 4;
				}
			}

			if (id == 1)
			{
				this.tempPb1.Image = newBitmap;
				this.pb1.Image = newBitmap;
			}
			else
			{
				this.tempPb2.Image = newBitmap;
			}															
			
			binFile.Write(id);
			binFile.Write(bitmapData.Width);
			binFile.Write(bitmapData.Height);
			binFile.Write(buffer);

			binFile.Flush();
			
			//this.picPreview.Image = newBitmap;
							
			bitmap.UnlockBits(bitmapData);
		}
		
		/*
		 * Utility functions
		 * /

		/*---------------------------------------------------*/
		
		private void ProcessTextureBuffer(byte [] buffer, int width, int height)
		{
			byte tmp = 0;
			int counter = 0;

			for (int i = 0; i < width * height; i++)
			{
				tmp = buffer[counter];
				buffer[counter] = buffer[counter + 2];
				buffer[counter + 2] = tmp;
				counter += 4;
			}						
		}

		/*---------------------------------------------------*/
		
		private int GenerateNewSize(int size)
		{
			int [] available = new int[11]{1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1};

			for (int j = size; j >= 1; j--)
			{
				for (int i = 0; i < available.Length; i++)
				{
					if (j == available[i])
					{
						return available[i];
					}
				}
			}

			return 0;
		}

		/*---------------------------------------------------*/
		
		private void ProcessStream(StreamReader stream)
		{
			if (stream.Peek() > 0)
			{
				string streamData = stream.ReadToEnd();
				this.logFrm.logRtfBox.AppendText(streamData);

				string [] stdv = streamData.Split('\n');
				for (int i = stdv.Length - 1; i >= 0; i--)
				{
					if (stdv[i] != "")
					{							
						this.logTxtBox.Text = stdv[i].Trim();
						break;
					}
				}
			}
		}				
		
		/*---------------------------------------------------*/

		byte [] GenerateName(string name, int maxLen)
		{
			char [] cname = name.ToCharArray();
			byte [] bname = new byte[maxLen];

			for (int i = 0; ( (i < maxLen) && (i < cname.Length) ); i++)
			{
				bname[i] = (byte)cname[i];
			}
			return bname;
		}

		/*---------------------------------------------------*/

		private string GenerateArgs(params string [] args)
		{
			string arguments = "";
			for (int i = 0; i < args.Length; i++)
			{
				if (args[i] != "")
				{
					arguments += "\"" + args[i] + "\"" + ((i != args.Length - 1) ? " " : "");
				}
			}
			return arguments.Replace('\\', '/');	
		}				

		
		/*---------------------------------------------------*/
		
		private void CheckOptions(Button button, params string [] text)
		{			
			for (int i = 0; i < text.Length; i++)
			{
				if ( (text[i] == null) || (text[i] == "") )
				{
					button.Enabled = false;
					return;
				}
			}
			button.Enabled = true;
		}
		
		/*---------------------------------------------------*/
		
		private string DisplayCommand()
		{
			string converter = this.txtConverter.Text;						

			string lo = "";
			string med = "";
			string hi = "";
		
			if ( (this.txtInputLow.Text != null) && (this.txtInputLow.Text != "") )
			{
				lo = "-l" + this.txtInputLow.Text;
			}

			if ( (this.txtInputMedium.Text != null) && (this.txtInputMedium.Text != "") )
			{
				med = "-m" + this.txtInputMedium.Text;
			}

			if ( (this.txtInputHigh.Text != null) && (this.txtInputHigh.Text != "") )
			{
				hi = "-h" + this.txtInputHigh.Text;
			}			

			string outputPath = "-d" + this.txtPath.Text;
		
			string name = "-o" + this.txtProjectName.Text;

			string args = this.GenerateArgs(lo, med, hi, outputPath, name);
			
			this.txtCommandToRun.Text = "\"" + this.txtConverter.Text + "\"" + " " + args;

			return args;
		}
		
		/*
		 * Event handlers
		 * /

		/*---------------------------------------------------*/

		private void mainForm_Load(object sender, System.EventArgs e)
		{
			try
			{
				StreamReader textIn = new StreamReader(Application.StartupPath + "\\lastUsedInfo.dat");
				txtPath.Text = textIn.ReadLine();
				txtInputLow.Text = textIn.ReadLine();
				txtInputMedium.Text = textIn.ReadLine();
				txtInputHigh.Text = textIn.ReadLine();
				txtConverter.Text = textIn.ReadLine();
				objectNameTxtBox.Text = textIn.ReadLine();
				txtPath.Text = textIn.ReadLine();
				txtAuthor.Text = textIn.ReadLine();
				commentRtfBox.Text = textIn.ReadLine();
				textIn.Close();
			}
			catch{ }
		}

		/*---------------------------------------------------*/

		private void btnGenerateRaw_Click(object sender, System.EventArgs e)
		{		
	
			if ( commentRtfBox.Text == "" )
			{
				commentRtfBox.Text = "NULL";
			}

			if ( objectNameTxtBox.Text == "" )
			{
				objectNameTxtBox.Text = "X";
			}


			//Save textInputs.
			StreamWriter textOut = new StreamWriter(Application.StartupPath + "\\lastUsedInfo.dat",false);
			textOut.WriteLine(txtPath.Text);
			textOut.WriteLine(txtInputLow.Text);
			textOut.WriteLine(txtInputMedium.Text);
			textOut.WriteLine(txtInputHigh.Text);
			textOut.WriteLine(txtConverter.Text);
			textOut.WriteLine(objectNameTxtBox.Text);
			textOut.WriteLine(txtPath.Text);
			textOut.WriteLine(txtAuthor.Text);
			textOut.WriteLine(commentRtfBox.Text);



			textOut.Close();

			for (int i = 0; i < Controls.Count; i++)
			{
				this.Controls[i].Enabled = false;
			}		

			//GenerateX3DML(this.txtInputLow.Text, this.objectTypeCmbBox.SelectedItem.ToString(), this.objectNameTxtBox.Text, this.commentRtfBox.Text);
									
			int lodCount = 0;

			if ( (this.txtInputLow.Text != null) && (this.txtInputLow.Text != "") )
			{
				lodCount++;
			}

			if ( (this.txtInputMedium.Text != null) && (this.txtInputMedium.Text != "") )
			{
				lodCount++;
			}

			if ( (this.txtInputHigh.Text != null) && (this.txtInputHigh.Text != "") )
			{
				lodCount++;
			}			

			if (lodCount == 0)
			{
				MessageBox.Show("Error, no input files");				
			}
			else
			{
				GenerateHeader(this.txtPath.Text, this.txtProjectName.Text, this.objectNameTxtBox.Text, lodCount, this.commentRtfBox.Text, ( ((string)this.objectTypeCmbBox.SelectedItem == "Static") ? ObjectType.Static : ( ((string)this.objectTypeCmbBox.SelectedItem == "Usable" ? ObjectType.Usable : ObjectType.Programmable) ) ) );
				Generate3DBin("\"" + this.txtConverter.Text + "\"", DisplayCommand());
				GenerateTextures(this.txtPath.Text, this.txtProjectName.Text, lodCount);
			}
												
			for (int i = 0; i < Controls.Count; i++)
			{
				this.Controls[i].Enabled = true;
			}
		}
		
		/*---------------------------------------------------*/

		private void folderBtn_Click(object sender, System.EventArgs e)
		{
			DialogResult result = this.dlgFolderIcon.ShowDialog();
			if (result == DialogResult.OK)
			{
				this.inputFolderTxtBox.Text = dlgFolderIcon.SelectedPath;
				string [] fileList = Directory.GetFiles(this.inputFolderTxtBox.Text, "*.bmp");
				for (int i = 0;	i < fileList.Length; i++)
				{
					imageList.Items.Add(fileList[i]);
				}
			}
		}
		
		/*---------------------------------------------------*/

		private void btnBrowseOutpath_Click(object sender, System.EventArgs e)
		{			
			DialogResult result = this.dlgFolderPath.ShowDialog();
			if (result == DialogResult.OK)
			{
				this.txtPath.Text = this.dlgFolderPath.SelectedPath;			
			}
		}

		/*---------------------------------------------------*/

		private void imageList_SelectedIndexChanged(object sender, System.EventArgs e)
		{			
			picPreview.Image = Image.FromFile((string)imageList.Items[imageList.SelectedIndex]);
		}

		/*---------------------------------------------------*/
		
		private void showLogBtn_Click(object sender, System.EventArgs e)
		{						
			logFrm.Show();
		}

		/*---------------------------------------------------*/

		private void btnBrowseLow_Click(object sender, System.EventArgs e)
		{
			DialogResult result = this.dlgOpenLow.ShowDialog();
			if (result == DialogResult.OK)
			{
				this.txtInputLow.Text = dlgOpenLow.FileName;
			}			
		}

		/*---------------------------------------------------*/

		private void btnBrowseMed_Click(object sender, System.EventArgs e)
		{
			DialogResult result = this.dlgOpenMedium.ShowDialog();
			if (result == DialogResult.OK)
			{
				this.txtInputMedium.Text = this.dlgOpenMedium.FileName;
			}			
		}

		/*---------------------------------------------------*/

		private void btnBrowseHigh_Click(object sender, System.EventArgs e)
		{
			DialogResult result = this.dlgOpenHigh.ShowDialog();
			if (result == DialogResult.OK)
			{
				this.txtInputHigh.Text = this.dlgOpenHigh.FileName;
			}
		}

		/*---------------------------------------------------*/

		private void btnConverter_Click(object sender, System.EventArgs e)
		{
			DialogResult result = this.dlgOpenConverter.ShowDialog();
			if (result == DialogResult.OK)
			{
				this.txtConverter.Text = this.dlgOpenConverter.FileName;				
			}
		}
		
		/*---------------------------------------------------*/

		private void inputFileTextChanged(object sender, System.EventArgs e)
		{
			if ( ( (this.txtInputLow.Text == null) || (this.txtInputLow.Text == "") ) && ( ( (this.txtInputMedium.Text == null) || (this.txtInputMedium.Text == "") ) && ( ( (this.txtInputHigh.Text == null) || (this.txtInputMedium.Text == "") ) ) ) )
			{
				this.btnGenerateRaw.Enabled = false;
			}
			else
			{
				this.btnGenerateRaw.Enabled = true;
			}
			DisplayCommand();
		}

		/*---------------------------------------------------*/

		private void reqTextChanged(object sender, System.EventArgs e)
		{
			CheckOptions(this.btnGenerateRaw, this.txtPath.Text, this.txtProjectName.Text, this.txtConverter.Text);
			DisplayCommand();
		}

		private void btnGenerateRaw_MouseHover(object sender, System.EventArgs e)
		{
			ToolTip a = new ToolTip();
			a.SetToolTip(this.btnGenerateRaw, "\nThis will create the new *.3dbin file\nbased on all your input\n");

		}

		private void cmdOpenPhys_Click(object sender, System.EventArgs e)
		{
			try
			{
				phys.Activate();
				phys.Visible = true;
			}
			catch
			{
				phys = new PhysicsApp(this);
				phys.Activate();
				phys.Visible = true;
			}
		}

	}
}
