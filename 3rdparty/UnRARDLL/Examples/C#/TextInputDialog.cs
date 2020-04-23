using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace Schematrix
{
	/// <summary>
	/// Summary description for TextInputDialog.
	/// </summary>
	public class TextInputDialog : System.Windows.Forms.Form
	{
		private System.Windows.Forms.TextBox InputValue;
		private System.Windows.Forms.Button OKButton;
		private System.Windows.Forms.Button ResetButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.Button CanxButton;
		private string defaultValue=string.Empty;

		public TextInputDialog()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
		}

		public string Value
		{
			get
			{
				return(InputValue.Text);
			}
			set
			{
				if(value!=InputValue.Text)
				{
					InputValue.Text=value;
					InputValue.SelectionStart=value.Length;
					InputValue.SelectionLength=0;
					defaultValue=value;
				}
			}
		}

		public string Prompt
		{
			get
			{
				return(this.Text);
			}
			set
			{
				this.Text=value;
			}
		}

		public char PasswordChar
		{
			get
			{
				return InputValue.PasswordChar;
			}
			set
			{
				InputValue.PasswordChar=value;
			}
		}

		public void SelectAll()
		{
			InputValue.SelectAll();
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
			this.InputValue = new System.Windows.Forms.TextBox();
			this.OKButton = new System.Windows.Forms.Button();
			this.CanxButton = new System.Windows.Forms.Button();
			this.ResetButton = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// InputValue
			// 
			this.InputValue.Dock = System.Windows.Forms.DockStyle.Top;
			this.InputValue.Location = new System.Drawing.Point(0, 0);
			this.InputValue.Name = "InputValue";
			this.InputValue.Size = new System.Drawing.Size(304, 20);
			this.InputValue.TabIndex = 1;
			this.InputValue.Text = "";
			// 
			// OKButton
			// 
			this.OKButton.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left)));
			this.OKButton.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.OKButton.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.OKButton.Location = new System.Drawing.Point(40, 26);
			this.OKButton.Name = "OKButton";
			this.OKButton.Size = new System.Drawing.Size(64, 24);
			this.OKButton.TabIndex = 2;
			this.OKButton.Text = "OK";
			// 
			// CanxButton
			// 
			this.CanxButton.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left)));
			this.CanxButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.CanxButton.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.CanxButton.Location = new System.Drawing.Point(184, 26);
			this.CanxButton.Name = "CanxButton";
			this.CanxButton.Size = new System.Drawing.Size(64, 24);
			this.CanxButton.TabIndex = 3;
			this.CanxButton.Text = "Cancel";
			// 
			// ResetButton
			// 
			this.ResetButton.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left)));
			this.ResetButton.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.ResetButton.Location = new System.Drawing.Point(112, 26);
			this.ResetButton.Name = "ResetButton";
			this.ResetButton.Size = new System.Drawing.Size(64, 24);
			this.ResetButton.TabIndex = 4;
			this.ResetButton.Text = "Reset";
			this.ResetButton.Click += new System.EventHandler(this.ResetButton_Click);
			// 
			// TextInputDialog
			// 
			this.AcceptButton = this.OKButton;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(304, 54);
			this.ControlBox = false;
			this.Controls.Add(this.ResetButton);
			this.Controls.Add(this.CanxButton);
			this.Controls.Add(this.OKButton);
			this.Controls.Add(this.InputValue);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.KeyPreview = true;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "TextInputDialog";
			this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
			this.Text = "TextInputDialog";
			this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.TextInputDialog_KeyDown);
			this.ResumeLayout(false);

		}
		#endregion

		private void ResetButton_Click(object sender, System.EventArgs e)
		{
			InputValue.Text=defaultValue;
		}

		private void TextInputDialog_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
		{
			if(e.KeyCode==Keys.Escape)
				this.DialogResult=DialogResult.Cancel;
		}
	}
}
