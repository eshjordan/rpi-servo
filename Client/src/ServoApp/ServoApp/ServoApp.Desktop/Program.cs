﻿using Eto.Forms;
using System;

namespace ServoApp.Desktop
{
    internal class Program
    {
        [STAThread]
        private static void Main(string[] args)
        {
            new Application(Eto.Platform.Detect).Run(new MainForm());
        }
    }
}