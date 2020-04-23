'**************************************************************************************
'
'  Trivial example using the Wrapper class for unrar.dll
'  If anyone finds any problems, I'd appreciate an email.
'
'  marcel_madonna@ajmsoft.com
'
'  This code is meant as an example only.  It is not robust and does not handle 
'  many potential error conditions.  This is not production ready code.
'
'**************************************************************************************

Imports RARNET

Public Class Form1
    Inherits System.Windows.Forms.Form

#Region " Windows Form Designer generated code "

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        'Add any initialization after the InitializeComponent() call

    End Sub

    'Form overrides dispose to clean up the component list.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    Friend WithEvents lv As System.Windows.Forms.ListView
    Friend WithEvents btnOpen As System.Windows.Forms.Button
    Friend WithEvents btnClose As System.Windows.Forms.Button
    Friend WithEvents btnUnpackAll As System.Windows.Forms.Button
    Friend WithEvents btnUnpack As System.Windows.Forms.Button
    Friend WithEvents btnComment As System.Windows.Forms.Button
    Friend WithEvents StatusBar1 As System.Windows.Forms.StatusBar
    Friend WithEvents mainPanel As System.Windows.Forms.StatusBarPanel
    Friend WithEvents datepanel As System.Windows.Forms.StatusBarPanel
    Friend WithEvents progressPanel As System.Windows.Forms.StatusBarPanel
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Me.btnOpen = New System.Windows.Forms.Button
        Me.lv = New System.Windows.Forms.ListView
        Me.btnClose = New System.Windows.Forms.Button
        Me.btnUnpackAll = New System.Windows.Forms.Button
        Me.btnUnpack = New System.Windows.Forms.Button
        Me.btnComment = New System.Windows.Forms.Button
        Me.StatusBar1 = New System.Windows.Forms.StatusBar
        Me.mainPanel = New System.Windows.Forms.StatusBarPanel
        Me.datepanel = New System.Windows.Forms.StatusBarPanel
        Me.progressPanel = New System.Windows.Forms.StatusBarPanel
        CType(Me.mainPanel, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.datepanel, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.progressPanel, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.SuspendLayout()
        '
        'btnOpen
        '
        Me.btnOpen.Location = New System.Drawing.Point(336, 224)
        Me.btnOpen.Name = "btnOpen"
        Me.btnOpen.Size = New System.Drawing.Size(64, 24)
        Me.btnOpen.TabIndex = 0
        Me.btnOpen.Text = "Open"
        '
        'lv
        '
        Me.lv.Location = New System.Drawing.Point(8, 16)
        Me.lv.Name = "lv"
        Me.lv.Size = New System.Drawing.Size(608, 200)
        Me.lv.TabIndex = 1
        '
        'btnClose
        '
        Me.btnClose.Location = New System.Drawing.Point(552, 224)
        Me.btnClose.Name = "btnClose"
        Me.btnClose.Size = New System.Drawing.Size(64, 24)
        Me.btnClose.TabIndex = 2
        Me.btnClose.Text = "Close"
        '
        'btnUnpackAll
        '
        Me.btnUnpackAll.Location = New System.Drawing.Point(480, 224)
        Me.btnUnpackAll.Name = "btnUnpackAll"
        Me.btnUnpackAll.Size = New System.Drawing.Size(64, 24)
        Me.btnUnpackAll.TabIndex = 3
        Me.btnUnpackAll.Text = "Unp All"
        '
        'btnUnpack
        '
        Me.btnUnpack.Location = New System.Drawing.Point(408, 224)
        Me.btnUnpack.Name = "btnUnpack"
        Me.btnUnpack.Size = New System.Drawing.Size(64, 24)
        Me.btnUnpack.TabIndex = 4
        Me.btnUnpack.Text = "Unpack"
        '
        'btnComment
        '
        Me.btnComment.Location = New System.Drawing.Point(8, 224)
        Me.btnComment.Name = "btnComment"
        Me.btnComment.Size = New System.Drawing.Size(64, 24)
        Me.btnComment.TabIndex = 5
        Me.btnComment.Text = "Comment"
        '
        'StatusBar1
        '
        Me.StatusBar1.Location = New System.Drawing.Point(0, 256)
        Me.StatusBar1.Name = "StatusBar1"
        Me.StatusBar1.Panels.AddRange(New System.Windows.Forms.StatusBarPanel() {Me.mainPanel, Me.progressPanel, Me.datepanel})
        Me.StatusBar1.ShowPanels = True
        Me.StatusBar1.Size = New System.Drawing.Size(624, 16)
        Me.StatusBar1.TabIndex = 6
        '
        'mainPanel
        '
        Me.mainPanel.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Spring
        Me.mainPanel.Text = "StatusBarPanel1"
        Me.mainPanel.Width = 499
        '
        'datepanel
        '
        Me.datepanel.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents
        Me.datepanel.Text = "StatusBarPanel1"
        Me.datepanel.Width = 99
        '
        'progressPanel
        '
        Me.progressPanel.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents
        Me.progressPanel.Width = 10
        '
        'Form1
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(624, 272)
        Me.Controls.Add(Me.StatusBar1)
        Me.Controls.Add(Me.btnComment)
        Me.Controls.Add(Me.btnUnpack)
        Me.Controls.Add(Me.btnUnpackAll)
        Me.Controls.Add(Me.btnClose)
        Me.Controls.Add(Me.lv)
        Me.Controls.Add(Me.btnOpen)
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow
        Me.Name = "Form1"
        Me.Text = "Trivial UnRAR Sample"
        CType(Me.mainPanel, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.datepanel, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.progressPanel, System.ComponentModel.ISupportInitialize).EndInit()
        Me.ResumeLayout(False)

    End Sub

#End Region

    Dim d As Decompressor

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        lv.View = View.Details
        lv.GridLines = True
        ' Create some columns 
        lv.Columns.Add("File", 200, HorizontalAlignment.Left)
        lv.Columns.Add("Attr", 30, HorizontalAlignment.Left)
        lv.Columns.Add("Packed Size", 100, HorizontalAlignment.Left)
        lv.Columns.Add("Unpacked Size", 100, HorizontalAlignment.Center)
        lv.Columns.Add("CRC", 120, HorizontalAlignment.Center)

        'Initialize the status bar
        StatusBar1.Panels.Item(0).Text = ""
        StatusBar1.Panels.Item(2).Text = Now.ToShortDateString.ToString
    End Sub

    Private Sub btnOpen_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnOpen.Click

        Dim szFile As String = GetFile
        Me.Cursor = Cursors.WaitCursor
        lv.Items.Clear()
        Try
            d = New Decompressor(szFile)
            AddHandler d.OnUnpack, AddressOf Unpacked
            AddHandler Decompressor.Unpacking, AddressOf InProgress

            For Each r As Decompressor.RAREntry In d.RARFiles
                Dim lvitem As New ListViewItem(r.FileName)
                ' Place a check mark next to the item.
                lvitem.SubItems.Add(r.FileAttr)
                lvitem.SubItems.Add(r.PackSize)
                lvitem.SubItems.Add(r.UnpSize)
                lvitem.SubItems.Add(r.FileCRC)
                lv.Items.Add(lvitem)
        Next
        Catch ex As Exception
            MsgBox(ex.Message)
            d = Nothing
            Exit Sub
        Finally
            Me.Cursor = Cursors.Default
        End Try
        StatusBar1.Panels.Item(2).Text = d.TotalArchiveSize.ToString

    End Sub

    Private Sub btnClose_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnClose.Click
        Me.Close()
    End Sub

    Private Sub btnComment_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnComment.Click
        If d Is Nothing Then Exit Sub
        MsgBox("Comment: " & d.RARHeader.CmtBuf)
    End Sub

    Private Sub btnUnpack_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnUnpack.Click
        Me.Cursor = Cursors.WaitCursor
        StatusBar1.Panels.Item(0).Text = "Working..."
        Try
            Select Case lv.SelectedItems.Count
                Case 0
                    Me.Cursor = Cursors.Default
                    MsgBox("Select a file to unpack")
                Case 1
                    d.UnPack(lv.SelectedItems(0).Text, GetPath())
                Case Else
                    Dim szFiles(lv.SelectedItems.Count - 1) As String
                    For i As Integer = 0 To lv.SelectedItems.Count - 1
                        szFiles(i) = lv.SelectedItems(i).Text
                    Next
                    d.UnPack(szFiles, GetPath())

            End Select
        Catch ex As Exception
            MsgBox(ex.Message)
        Finally
            StatusBar1.Panels.Item(0).Text = "Done"
            Me.Cursor = Cursors.Default
        End Try

    End Sub

    Private Sub btnUnpackAll_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnUnpackAll.Click
        Me.Cursor = Cursors.WaitCursor
        StatusBar1.Panels.Item(0).Text = "Working..."
        Try
            d.UnPackAll(GetPath())
        Catch ex As Exception
            MsgBox(ex.Message)
        Finally
            StatusBar1.Panels.Item(0).Text = "Done"
            Me.Cursor = Cursors.Default
        End Try
    End Sub

    Private Function GetFile() As String

        Dim f As New OpenFileDialog
        f.Title = "Unpack RAR Archive"
        f.CheckFileExists = True
        f.DefaultExt = "RAR"
        f.Filter = "RAR files (*.rar)|*.rar|All files (*.*)|*.*"
        f.RestoreDirectory = True
        If f.ShowDialog() = DialogResult.OK Then
            Return f.FileName
        End If
        Return ""
    End Function

    Private Function GetPath() As String
        Dim f As New FolderBrowserDialog
        f.Description = "Unpack RAR Archive"
        f.ShowNewFolderButton = True
        If f.ShowDialog() = DialogResult.OK Then
            Return f.SelectedPath
        End If
        Return ""
    End Function

    Public Sub Unpacked(ByVal r As Decompressor.RAREntry)
        StatusBar1.Panels.Item(0).Text = r.FileName & " Unpacked"
    End Sub

    'Public Sub InProgress(ByVal TotalFileSize As Long, ByVal CurrentFileSize As Long, ByRef bDisposition As Decompressor.RarDisposition)
    Public Sub InProgress(ByVal TotalFileSize As Long, ByVal CurrentFileSize As Long, ByRef bDisposition As Decompressor.RarDisposition)
        If TotalFileSize = 0 Then Exit Sub
        StatusBar1.Panels.Item(1).Text = Format(CurrentFileSize / TotalFileSize, "percent")
        Application.DoEvents()
    End Sub


End Class
