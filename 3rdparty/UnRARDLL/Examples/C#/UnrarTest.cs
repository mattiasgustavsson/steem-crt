using System;
using System.IO;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace Schematrix
{
    /// <summary>
    /// Test driver form for unrar class
    /// </summary>
    public class UnrarTest : System.Windows.Forms.Form
    {
        private System.Windows.Forms.TextBox rarFileName;
        private System.Windows.Forms.Button selectFile;
        private System.Windows.Forms.Label rarFileNameLabel;
        private System.Windows.Forms.ListView fileList;
        private System.Windows.Forms.Button testButton;
        private System.Windows.Forms.StatusBar statusBar;
        private System.Windows.Forms.Button extractButton;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.FolderBrowserDialog folderBrowser;
        private System.Windows.Forms.OpenFileDialog openFile;
        private System.Windows.Forms.ColumnHeader fileNameColumnHeader;
        private System.Windows.Forms.ColumnHeader unpackedSizeColumnHeader;
        private System.Windows.Forms.ColumnHeader packedSizeColumnHeader;
        private System.Windows.Forms.ColumnHeader fileTimestampColumnHeader;
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;

        private Unrar unrar;

        public UnrarTest()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
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

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.Run(new UnrarTest());
        }

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.rarFileName = new System.Windows.Forms.TextBox();
            this.selectFile = new System.Windows.Forms.Button();
            this.rarFileNameLabel = new System.Windows.Forms.Label();
            this.fileList = new System.Windows.Forms.ListView();
            this.fileNameColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.unpackedSizeColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.packedSizeColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.fileTimestampColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.testButton = new System.Windows.Forms.Button();
            this.statusBar = new System.Windows.Forms.StatusBar();
            this.extractButton = new System.Windows.Forms.Button();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.folderBrowser = new System.Windows.Forms.FolderBrowserDialog();
            this.openFile = new System.Windows.Forms.OpenFileDialog();
            this.SuspendLayout();
            //
            // rarFileName
            //
            this.rarFileName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
            this.rarFileName.Location = new System.Drawing.Point(6, 26);
            this.rarFileName.Name = "rarFileName";
            this.rarFileName.ReadOnly = true;
            this.rarFileName.Size = new System.Drawing.Size(590, 20);
            this.rarFileName.TabIndex = 0;
            this.rarFileName.Text = "";
            //
            // selectFile
            //
            this.selectFile.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.selectFile.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.selectFile.Location = new System.Drawing.Point(603, 26);
            this.selectFile.Name = "selectFile";
            this.selectFile.Size = new System.Drawing.Size(28, 20);
            this.selectFile.TabIndex = 1;
            this.selectFile.Text = "...";
            this.selectFile.Click += new System.EventHandler(this.selectFile_Click);
            //
            // rarFileNameLabel
            //
            this.rarFileNameLabel.Location = new System.Drawing.Point(6, 8);
            this.rarFileNameLabel.Name = "rarFileNameLabel";
            this.rarFileNameLabel.Size = new System.Drawing.Size(208, 23);
            this.rarFileNameLabel.TabIndex = 2;
            this.rarFileNameLabel.Text = "RAR File Name";
            //
            // fileList
            //
            this.fileList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
            this.fileList.CheckBoxes = true;
            this.fileList.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
                                                                                                                                                             this.fileNameColumnHeader,
                                                                                                                                                             this.unpackedSizeColumnHeader,
                                                                                                                                                             this.packedSizeColumnHeader,
                                                                                                                                                             this.fileTimestampColumnHeader});
            this.fileList.Location = new System.Drawing.Point(6, 52);
            this.fileList.Name = "fileList";
            this.fileList.Size = new System.Drawing.Size(626, 304);
            this.fileList.TabIndex = 3;
            this.fileList.View = System.Windows.Forms.View.Details;
            //
            // fileNameColumnHeader
            //
            this.fileNameColumnHeader.Text = "File Name";
            this.fileNameColumnHeader.Width = 250;
            //
            // unpackedSizeColumnHeader
            //
            this.unpackedSizeColumnHeader.Text = "Unpacked Size";
            this.unpackedSizeColumnHeader.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.unpackedSizeColumnHeader.Width = 100;
            //
            // packedSizeColumnHeader
            //
            this.packedSizeColumnHeader.Text = "Packed Size";
            this.packedSizeColumnHeader.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.packedSizeColumnHeader.Width = 100;
            //
            // fileTimestampColumnHeader
            //
            this.fileTimestampColumnHeader.Text = "File Timestamp";
            this.fileTimestampColumnHeader.Width = 150;
            //
            // testButton
            //
            this.testButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.testButton.Enabled = false;
            this.testButton.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.testButton.Location = new System.Drawing.Point(12, 364);
            this.testButton.Name = "testButton";
            this.testButton.Size = new System.Drawing.Size(66, 26);
            this.testButton.TabIndex = 4;
            this.testButton.Text = "Test";
            this.testButton.Click += new System.EventHandler(this.testButton_Click);
            //
            // statusBar
            //
            this.statusBar.Location = new System.Drawing.Point(0, 398);
            this.statusBar.Name = "statusBar";
            this.statusBar.Size = new System.Drawing.Size(640, 22);
            this.statusBar.TabIndex = 5;
            this.statusBar.Text = "Ready";
            //
            // extractButton
            //
            this.extractButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.extractButton.Enabled = false;
            this.extractButton.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.extractButton.Location = new System.Drawing.Point(87, 365);
            this.extractButton.Name = "extractButton";
            this.extractButton.Size = new System.Drawing.Size(66, 26);
            this.extractButton.TabIndex = 6;
            this.extractButton.Text = "Extract";
            this.extractButton.Click += new System.EventHandler(this.extractButton_Click);
            //
            // progressBar
            //
            this.progressBar.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBar.Location = new System.Drawing.Point(162, 370);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(470, 16);
            this.progressBar.TabIndex = 7;
            //
            // folderBrowser
            //
            this.folderBrowser.Description = "Please select a destination folder for the extracted files.";
            //
            // openFile
            //
            this.openFile.DefaultExt = "rar";
            this.openFile.Filter = "RAR Files (*.rar)|*.rar|All Files (*.*)|*.*";
            this.openFile.Title = "Select RAR File";
            //
            // UnrarTest
            //
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(640, 420);
            this.Controls.Add(this.progressBar);
            this.Controls.Add(this.extractButton);
            this.Controls.Add(this.statusBar);
            this.Controls.Add(this.testButton);
            this.Controls.Add(this.fileList);
            this.Controls.Add(this.selectFile);
            this.Controls.Add(this.rarFileName);
            this.Controls.Add(this.rarFileNameLabel);
            this.Name = "UnrarTest";
            this.Text = "Unrar Class Tester";
            this.ResumeLayout(false);

        }
        #endregion

        private void selectFile_Click(object sender, System.EventArgs e)
        {
            if(openFile.ShowDialog()==DialogResult.OK)
            {
                rarFileName.Text = openFile.FileName;
                OpenRarFile(rarFileName.Text);
            }
        }

        private void OpenRarFile(string fileName)
        {
            try
            {
                this.Cursor=Cursors.WaitCursor;
                testButton.Enabled=false;
                extractButton.Enabled=false;
                fileList.BeginUpdate();
                fileList.Items.Clear();

                // Create new unrar class and open archive for listing files
                unrar=new Unrar();
                unrar.Open(fileName, Unrar.OpenMode.List);

                // Read each header, skipping directory entries
                while(unrar.ReadHeader())
                {
                    if(!unrar.CurrentFile.IsDirectory)
                    {
                        ListViewItem item=new ListViewItem(unrar.CurrentFile.FileName);
                        item.SubItems.Add(unrar.CurrentFile.UnpackedSize.ToString());
                        item.SubItems.Add(unrar.CurrentFile.PackedSize.ToString());
                        item.SubItems.Add(unrar.CurrentFile.FileTime.ToString());
                        item.Checked=true;
                        fileList.Items.Add(item);
                    }
                    unrar.Skip();
                }

                // Cleanup and enable buttons if no exception was thrown
                unrar.Close();
                this.unrar=null;
                testButton.Enabled=true;
                extractButton.Enabled=true;
            }
            catch(Exception ex)
            {
                testButton.Enabled=false;
                extractButton.Enabled=false;
                MessageBox.Show(ex.Message);
            }
            finally
            {
                this.Cursor=Cursors.Default;
                fileList.EndUpdate();
                if(this.unrar!=null)
                    unrar.Close();
            }
        }

        private Hashtable GetSelectedFiles()
        {
            // Build hashtable of selected entries
            Hashtable ht=new Hashtable(fileList.CheckedItems.Count);
            foreach(ListViewItem item in fileList.CheckedItems)
                ht.Add(item.Text, item);
            return ht;
        }

        private void testButton_Click(object sender, System.EventArgs e)
        {
            // Get hashtable of selected files
            Hashtable selectedFiles=GetSelectedFiles();
            try
            {
                this.Cursor=Cursors.WaitCursor;

                // Create new unrar class and attach event handlers for
                // progress, missing volumes, and password
                unrar=new Unrar();
                AttachHandlers(unrar);

                // Open archive for extraction
                unrar.Open(rarFileName.Text, Unrar.OpenMode.Extract);

                // Test each file found in hashtable
                while(unrar.ReadHeader())
                {
                    if(selectedFiles.ContainsKey(unrar.CurrentFile.FileName))
                    {
                        this.progressBar.Value=0;
                        unrar.Test();
                    }
                    else
                    {
                        unrar.Skip();
                    }
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
            finally
            {
                this.Cursor=Cursors.Default;
                this.statusBar.Text = "Ready";
                this.progressBar.Value=0;
                if(this.unrar!=null)
                    unrar.Close();
            }
        }

        private void extractButton_Click(object sender, System.EventArgs e)
        {
            // Get hashtable of selected files
            Hashtable selectedFiles=GetSelectedFiles();
            try
            {
                // Get destination from user
                string directory=Path.GetDirectoryName(rarFileName.Text);
                if(Directory.Exists(directory))
                    folderBrowser.SelectedPath=directory;
                if(folderBrowser.ShowDialog()==DialogResult.OK)
                {
                    this.Cursor=Cursors.WaitCursor;

                    // Create new unrar class and attach event handlers for
                    // progress, missing volumes, and password
                    unrar=new Unrar();
                    AttachHandlers(unrar);

                    // Set destination path for all files
                    unrar.DestinationPath=folderBrowser.SelectedPath;

                    // Open archive for extraction
                    unrar.Open(rarFileName.Text, Unrar.OpenMode.Extract);

                    // Extract each file found in hashtable
                    while(unrar.ReadHeader())
                    {
                        if(selectedFiles.ContainsKey(unrar.CurrentFile.FileName))
                        {
                            this.progressBar.Value=0;
                            unrar.Extract();
                        }
                        else
                        {
                            unrar.Skip();
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
            finally
            {
                this.Cursor=Cursors.Default;
                this.statusBar.Text = "Ready";
                this.progressBar.Value=0;
                if(this.unrar!=null)
                    unrar.Close();
            }
        }

        private void AttachHandlers(Unrar unrar)
        {
            unrar.ExtractionProgress+=new ExtractionProgressHandler(unrar_ExtractionProgress);
            unrar.MissingVolume+=new MissingVolumeHandler(unrar_MissingVolume);
            unrar.PasswordRequired+=new PasswordRequiredHandler(unrar_PasswordRequired);
        }

        private void unrar_ExtractionProgress(object sender, ExtractionProgressEventArgs e)
        {
            statusBar.Text = "Testing " + e.FileName;
            progressBar.Value=(int)e.PercentComplete;
        }

        private void unrar_MissingVolume(object sender, MissingVolumeEventArgs e)
        {
            TextInputDialog dialog=new TextInputDialog();
            dialog.Value=e.VolumeName;
            dialog.Prompt=string.Format("Volume is missing.  Correct or cancel.");
            if(dialog.ShowDialog()==DialogResult.OK)
            {
                e.VolumeName=dialog.Value;
                e.ContinueOperation=true;
            }
            else
                e.ContinueOperation=false;
        }

        private void unrar_PasswordRequired(object sender, PasswordRequiredEventArgs e)
        {
            TextInputDialog dialog=new TextInputDialog();
            dialog.Prompt=string.Format("Password is required for extraction.");
            dialog.PasswordChar='*';
            if(dialog.ShowDialog()==DialogResult.OK)
            {
                e.Password=dialog.Value;
                e.ContinueOperation=true;
            }
            else
                e.ContinueOperation=false;
        }
    }
}

