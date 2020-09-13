using System;
using System.Windows.Forms;

namespace Mandelbrot
{
    partial class MandelbrotForm
    {
        private OpenTK.GLControl glControl;
        
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }

            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.glControl = new OpenTK.GLControl();
            this.trackBar1 = new System.Windows.Forms.TrackBar();
            this.trackBar2 = new System.Windows.Forms.TrackBar();
            this.trackBar3 = new System.Windows.Forms.TrackBar();
            this.trackBar4 = new System.Windows.Forms.TrackBar();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize) (this.trackBar1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize) (this.trackBar2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize) (this.trackBar3)).BeginInit();
            ((System.ComponentModel.ISupportInitialize) (this.trackBar4)).BeginInit();
            this.SuspendLayout();
            // 
            // glControl
            // 
            this.glControl.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.glControl.BackColor = System.Drawing.Color.Black;
            this.glControl.Location = new System.Drawing.Point(0, 0);
            this.glControl.Margin = new System.Windows.Forms.Padding(0);
            this.glControl.Name = "glControl";
            this.glControl.Size = new System.Drawing.Size(800, 800);
            this.glControl.TabIndex = 0;
            this.glControl.VSync = false;
            this.glControl.Load += new System.EventHandler(this.OnGLControlLoad);
            this.glControl.Paint += new System.Windows.Forms.PaintEventHandler(this.OnGLControlRender);
            this.glControl.MouseDown += new System.Windows.Forms.MouseEventHandler(this.glControl_MouseDown);
            this.glControl.MouseMove += new System.Windows.Forms.MouseEventHandler(this.glControl_MouseMove);
            this.glControl.MouseUp += new System.Windows.Forms.MouseEventHandler(this.glControl_MouseUp);
            this.glControl.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.glControl_MouseWheel);
            this.glControl.Resize += new System.EventHandler(this.OnGLControlResize);
            // 
            // trackBar1
            // 
            this.trackBar1.LargeChange = 1000;
            this.trackBar1.Location = new System.Drawing.Point(803, 0);
            this.trackBar1.Maximum = 1000;
            this.trackBar1.Name = "trackBar1";
            this.trackBar1.Size = new System.Drawing.Size(533, 114);
            this.trackBar1.SmallChange = 100;
            this.trackBar1.TabIndex = 1;
            this.trackBar1.Value = 500;
            this.trackBar1.ValueChanged += new System.EventHandler(this.trackBar1_ValueChanged);
            // 
            // trackBar2
            // 
            this.trackBar2.Location = new System.Drawing.Point(803, 189);
            this.trackBar2.Maximum = 100;
            this.trackBar2.Name = "trackBar2";
            this.trackBar2.Size = new System.Drawing.Size(533, 114);
            this.trackBar2.TabIndex = 2;
            this.trackBar2.Value = 50;
            this.trackBar2.ValueChanged += new System.EventHandler(this.trackBar2_ValueChanged);
            // 
            // trackBar3
            // 
            this.trackBar3.Location = new System.Drawing.Point(803, 433);
            this.trackBar3.Maximum = 100;
            this.trackBar3.Minimum = -100;
            this.trackBar3.Name = "trackBar3";
            this.trackBar3.Size = new System.Drawing.Size(533, 114);
            this.trackBar3.TabIndex = 3;
            this.trackBar3.ValueChanged += new System.EventHandler(this.trackBar3_ValueChanged);
            // 
            // trackBar4
            // 
            this.trackBar4.LargeChange = 1;
            this.trackBar4.Location = new System.Drawing.Point(803, 607);
            this.trackBar4.Maximum = 100;
            this.trackBar4.Minimum = -100;
            this.trackBar4.Name = "trackBar4";
            this.trackBar4.Size = new System.Drawing.Size(533, 114);
            this.trackBar4.TabIndex = 4;
            this.trackBar4.ValueChanged += new System.EventHandler(this.trackBar4_ValueChanged);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(1395, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(181, 114);
            this.label1.TabIndex = 5;
            this.label1.Text = "500";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(1395, 189);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(181, 114);
            this.label2.TabIndex = 6;
            this.label2.Text = "50%";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(1395, 433);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(181, 114);
            this.label3.TabIndex = 7;
            this.label3.Text = "0";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(1395, 607);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(181, 114);
            this.label4.TabIndex = 8;
            this.label4.Text = "0";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(803, 84);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(533, 51);
            this.label5.TabIndex = 9;
            this.label5.Text = "Iterations";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(803, 297);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(533, 51);
            this.label6.TabIndex = 10;
            this.label6.Text = "Depth Coloring";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(803, 518);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(533, 51);
            this.label7.TabIndex = 11;
            this.label7.Text = "z0.X";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(803, 700);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(533, 51);
            this.label8.TabIndex = 12;
            this.label8.Text = "z0.Y";
            this.label8.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // MandelbrotForm
            // 
            this.ClientSize = new System.Drawing.Size(1600, 800);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.trackBar4);
            this.Controls.Add(this.trackBar3);
            this.Controls.Add(this.trackBar2);
            this.Controls.Add(this.trackBar1);
            this.Controls.Add(this.glControl);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Name = "MandelbrotForm";
            this.Text = "Form1";
            ((System.ComponentModel.ISupportInitialize) (this.trackBar1)).EndInit();
            ((System.ComponentModel.ISupportInitialize) (this.trackBar2)).EndInit();
            ((System.ComponentModel.ISupportInitialize) (this.trackBar3)).EndInit();
            ((System.ComponentModel.ISupportInitialize) (this.trackBar4)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;

        private System.Windows.Forms.TrackBar trackBar4;

        private System.Windows.Forms.TrackBar trackBar3;

        private System.Windows.Forms.TrackBar trackBar2;

        private System.Windows.Forms.TrackBar trackBar1;

        #endregion
    }
}