using Eto.Drawing;
using Eto.Forms;
using System;
using System.Net;
using System.Net.Sockets;
using System.Collections.ObjectModel;
using System.Diagnostics;

namespace ServoApp
{
    public partial class MainForm : Form
    {
        public Stopwatch stopwatch { get; set; }
        private Socket mySocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        private bool connected = false;
        private int port;
        private IPAddress iPAddress;
        private byte[] buffer = new byte[32];
        private String str;

        public MainForm()
        {
            stopwatch = Stopwatch.StartNew();
            InitializeComponent();
        }

        public void MyInit(TextBox textBox, Button buttonSend, Button buttonClose)
        {
            Debug.WriteLine("DONE");
            iPAddress = IPAddress.Parse("127.0.0.1");
            connect(textBox);
            textBox.Text = "Enter an IP address or 'exit'";
        }

        private void connect(TextBox textBox)
        {
            try
            {
                mySocket.Connect(iPAddress, 5001);
                mySocket.Receive(buffer);
                str = buffToStr(buffer);
                Debug.WriteLine(str);
                textBox.Text = str;
                connected = true;
            }
            catch
            {
                textBox.Text = "Could Not Connect to " + iPAddress.ToString();
                connected = false;
            }
        }

        private void sendClick(Socket s, TextBox box)
        {
            if (connected)
            {
                var usrText = box.Text;
                buffer = strToBuff(usrText);
                if (usrText.Equals("exit"))
                {
                    s.Send(buffer);
                    s.Close(3);
                }
                else
                {
                    s.Send(buffer);
                }
            }
            else
            {
                try
                {
                    iPAddress = IPAddress.Parse(box.Text);
                    connect(box);
                }
                catch
                {
                    IPAddress[] iPAddresses = Dns.GetHostAddresses(box.Text);
                    if (iPAddresses.Length > 0)
                    {
                        iPAddress = iPAddresses[0];
                        connect(box);
                    }
                    else
                    {
                        box.Text = "Invalid Address!";
                    }
                }
            }
        }

        private void closeClick(Socket s)
        {
            if (connected)
            {
                buffer = strToBuff("exit");
                s.Send(buffer);
                s.Close(3);
            }
            
            Application.Instance.Quit();
        }

        private void OnMouseMove(object sender, MouseEventArgs e)
        {
            if (connected && stopwatch.ElapsedMilliseconds > 15)
            {
                stopwatch = Stopwatch.StartNew();
                writeToSocket(mySocket, new Point(((int)e.Location.X), ((int)e.Location.Y)));
            }
        }

        public void writeToSocket(Socket s, Point coordinates)
        {
            if (connected)
            {
                string frmx;
                string frmy;
                int xcoords = (int)Math.Round(((double)(coordinates.X)) / ((double)(Size.Width / 180)));
                int ycoords = (int)Math.Round(((double)(coordinates.Y)) / ((double)(Size.Height / 180)));

                if (xcoords < 10)
                {
                    frmx = "00" + xcoords;
                }
                else if (xcoords < 100)
                {
                    frmx = "0" + xcoords;
                }
                else
                {
                    frmx = xcoords.ToString();
                }

                if (ycoords < 10)
                {
                    frmy = "00" + ycoords;
                }
                else if (ycoords < 100)
                {
                    frmy = "0" + ycoords;
                }
                else
                {
                    frmy = ycoords.ToString();
                }

                var str = String.Format("X{0}Y{1}",
                frmx,
                frmy);

                Debug.Print("Hello");
                Debug.Print(str);

                buffer = strToBuff(str);

                mySocket.Send(buffer);
            }
        }

        private string buffToStr(byte[] inBuff)
        {
            string retVal = System.Text.Encoding.ASCII.GetString(inBuff);
            retVal = string.Join("", retVal.Split(System.IO.Path.GetInvalidFileNameChars()));
            return retVal;
        }

        private byte[] strToBuff(String inStr)
        {
            inStr = string.Join("", inStr.Split(System.IO.Path.GetInvalidFileNameChars()));
            byte[] retVal = System.Text.Encoding.ASCII.GetBytes(inStr);
            return retVal;
        }
    }
}