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
            SuspendLayout();
            // 
            // pnl_display
            // 
            pnl_display.BackColor = SystemColors.Control;
            pnl_display.Location = new Point(12, 12);
            pnl_display.Name = "pnl_display";
            pnl_display.Size = new Size(851, 604);
            pnl_display.TabIndex = 0;
            // 
            // Susie
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            BackColor = SystemColors.ControlLight;
            ClientSize = new Size(875, 628);
            Controls.Add(pnl_display);
            HelpButton = true;
            Name = "Susie";
            Text = "Susie";
            ResumeLayout(false);
        }

        #endregion

        private Panel pnl_display;
    }
}
