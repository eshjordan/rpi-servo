using Eto.Drawing;
using Eto.Forms;
using System;

namespace ServoApp
{
    partial class MainForm : Form
    {
        private void InitializeComponent()
        {
            Title = "My Eto Form";
            ClientSize = new Size(600, 600);
            Padding = 10;

            var textBox = new TextBox();
            textBox.Size = new Size(200, 50);
            var buttonSend = new Button();
            buttonSend.Text = "Send";

            //buttonSend.Command;
            //var myCommand = new Command { };
            //myCommand.Executed += (sender, e) => sendClick();

            var buttonClose = new Button();
            buttonClose.Text = "Close";

            var myStackLayout = new StackLayout
            {
                Items =
                {
                    "Hello World!",
					// add more controls here
                    textBox,
                    buttonSend,
                    buttonClose,
                }
            };

            Content = myStackLayout;

            /*
            PixelFormat pxfm = Eto.Drawing.PixelFormat.Format32bppRgba;
            Bitmap bmp = new Eto.Drawing.Bitmap(600, 600, pxfm);
            Graphics grap = new Eto.Drawing.Graphics(bmp);

            RectangleF rectF = new RectangleF(Content.Bounds);

            grap.DrawRectangle(new Color(255, 0, 0, 0), rectF);
            */
            
            Content.MouseMove += (source, e) => OnMouseMove(source, e);
            buttonSend.Click += (sender, e) => sendClick(mySocket, textBox);
            buttonClose.Click += (sender, e) => closeClick(mySocket);

            // create a few commands that can be used for the menu and toolbar
            var clickMe = new Command { MenuText = "Click Me!", ToolBarText = "Click Me!" };
            clickMe.Executed += (sender, e) => MessageBox.Show(this, "I was clicked!");

            var quitCommand = new Command { MenuText = "Quit", Shortcut = Application.Instance.CommonModifier | Keys.Q };
            quitCommand.Executed += (sender, e) => Application.Instance.Quit();

            var aboutCommand = new Command { MenuText = "About..." };
            aboutCommand.Executed += (sender, e) => new AboutDialog().ShowDialog(this);

            // create menu
            Menu = new MenuBar
            {
                Items =
                {
					// File submenu
					new ButtonMenuItem { Text = "&File", Items = { clickMe } },
					// new ButtonMenuItem { Text = "&Edit", Items = { /* commands/items */ } },
					// new ButtonMenuItem { Text = "&View", Items = { /* commands/items */ } },
				},
                ApplicationItems =
                {
					// application (OS X) or file menu (others)
					new ButtonMenuItem { Text = "&Preferences..." },
                },
                QuitItem = quitCommand,
                AboutItem = aboutCommand
            };

            // create toolbar
            ToolBar = new ToolBar { Items = { clickMe } };

            Content.Load += (sender, e) => OnLoad(sender, e);

            void OnLoad(object sender, EventArgs e)
            {
                MyInit(textBox, buttonSend, buttonClose);
            }
        }
    }
}
