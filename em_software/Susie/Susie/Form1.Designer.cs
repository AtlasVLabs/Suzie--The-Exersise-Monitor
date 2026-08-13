namespace Susie
{
    partial class Susie
    {
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            pnl_display = new Panel();
            tbx_statusBar = new TextBox();
            button1 = new Button();
            pnl_display.SuspendLayout();
            SuspendLayout();
            // 
            // pnl_display
            // 
            pnl_display.Controls.Add(button1);
            pnl_display.Location = new Point(12, 12);
            pnl_display.Name = "pnl_display";
            pnl_display.Size = new Size(750, 575);
            pnl_display.TabIndex = 0;
            // 
            // tbx_statusBar
            // 
            tbx_statusBar.Location = new Point(12, 593);
            tbx_statusBar.Name = "tbx_statusBar";
            tbx_statusBar.Size = new Size(750, 23);
            tbx_statusBar.TabIndex = 0;
            // 
            // button1
            // 
            button1.Location = new Point(3, 540);
            button1.Name = "button1";
            button1.Size = new Size(115, 32);
            button1.TabIndex = 2;
            button1.Text = "button1";
            button1.UseVisualStyleBackColor = true;
            // 
            // Susie
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            BackColor = SystemColors.ControlLight;
            ClientSize = new Size(774, 628);
            Controls.Add(tbx_statusBar);
            Controls.Add(pnl_display);
            HelpButton = true;
            Name = "Susie";
            Text = "Susie";
            pnl_display.ResumeLayout(false);
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Panel pnl_display;
        private TextBox tbx_statusBar;
        private Button button1;
    }
}
