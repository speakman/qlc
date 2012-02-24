/*
  Q Light Controller
  main.cpp

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QApplication>
#include <QTextStream>
#include <QTranslator>
#include <QMetaType>
#include <QtGlobal>
#include <QLocale>
#include <QString>
#include <QDebug>
#include <QTimer>
#include <QHash>
#include <QDir>

#include "qlcconfig.h"
#include "qlci18n.h"

#include "app.h"
#include "doc.h"

#ifndef WIN32
#include <X11/Xlib.h>
#endif

/* Use this namespace for command-line arguments so that we don't pollute
   the global namespace. */
namespace QLCArgs
{
    /**
     * If true, switch to operate mode after ALL initialization is done.
     */
    bool operate = false;

    /**
     * Specifies a workspace file name to load after all initialization
     * has been done, but before switching to operate mode (if applicable)
     */
    QString workspace;

    /** If true, enables kiosk-mode (Operate mode locked, only virtual console) */
    bool kioskMode = false;

    /** If true, opens the application in full screen mode */
    bool fullScreen = false;

    /** If not null, defines the place for a close button that in virtual console */
    QRect closeButtonRect = QRect();

    /**
     * Debug output level
     */
    QtMsgType debugLevel = QtSystemMsg;
}

/**
 * Suppresses debug messages
 */
void qlcMessageHandler(QtMsgType type, const char* msg)
{
    if (type >= QLCArgs::debugLevel)
    {
        fprintf(stderr, msg);
        fprintf(stderr, "\n");
        fflush(stderr);
    }
}

/**
 * Prints the application version
 */
void printVersion()
{
    QTextStream cout(stdout, QIODevice::WriteOnly);

    cout << endl;
    cout << APPNAME << " " << "version " << APPVERSION << endl;
    cout << "This program is licensed under the terms of the GNU ";
    cout << "General Public License v2." << endl;
    cout << "Copyright (c) Heikki Junnila (hjunnila@users.sf.net)." << endl;
    cout << endl;
}

/**
 * Prints possible command-line options
 */
void printUsage()
{
    QTextStream cout(stdout, QIODevice::WriteOnly);

    cout << "Usage:";
    cout << "  qlc [options]" << endl;
    cout << "Options:" << endl;
    cout << "  -c or --closebutton <x,y,w,h>\t\tPlace a close button in virtual console (only when -k is specified)" << endl;
    cout << "  -d or --debug <level>\t\tSet debug output level (0-3, see QtMsgType)" << endl;
    cout << "  -f or --fullscreen\t\tStart the application in fullscreen mode" << endl;
    cout << "  -h or --help\t\t\tPrint this help" << endl;
    cout << "  -k or --kiosk\t\t\tEnable kiosk mode (only virtual console in forced operate mode)" << endl;
    cout << "  -l or --locale <locale>\tForce a locale for translation" << endl;
    cout << "  -o or --open <file>\t\tOpen the specified workspace file" << endl;
    cout << "  -p or --operate\t\tStart in operate mode" << endl;
    cout << "  -v or --version\t\tPrint version information" << endl;
    cout << endl;
}

/**
 * Parse command line arguments
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 *
 * @return true to continue with application launch; otherwise false
 */
bool parseArgs(int argc, char **argv)
{
    QLCArgs::operate = false;

    for (int i = 1; i < argc; i++)
    {
        if (::strcmp(argv[i], "-v") == 0 ||
                ::strcmp(argv[i], "--version") == 0)
        {
            /* Don't print anything, since version is always
               printed before anything else. Just make the app
               exit by returning false. */
            return false;
        }
        else if (::strcmp(argv[i], "-h") == 0 ||
                 ::strcmp(argv[i], "--help") == 0)
        {
            printUsage();
            return false;
        }
        else if (::strcmp(argv[i], "-p") == 0 ||
                 ::strcmp(argv[i], "--operate") == 0)
        {
            QLCArgs::operate = true;
        }
        else if (::strcmp(argv[i], "-o") == 0 ||
                 ::strcmp(argv[i], "--open") == 0)
        {
            QLCArgs::workspace = QString(argv[++i]);
        }
        else if (::strcmp(argv[i], "-l") == 0 ||
                 ::strcmp(argv[i], "--locale") == 0)
        {
            QLCi18n::setDefaultLocale(QString(argv[++i]));
        }
        else if (::strcmp(argv[i], "-d") == 0 ||
                 ::strcmp(argv[i], "--debug") == 0)
        {
            QLCArgs::debugLevel = QtMsgType(QString(argv[++i]).toInt());
        }
        else if (::strcmp(argv[i], "-k") == 0 ||
                 ::strcmp(argv[i], "--kiosk") == 0)
        {
            QLCArgs::kioskMode = true;
        }
        else if (::strcmp(argv[i], "-f") == 0 ||
                 ::strcmp(argv[i], "--fullscreen") == 0)
        {
            QLCArgs::fullScreen = true;
        }
        else if (::strcmp(argv[i], "-c") == 0 ||
                 ::strcmp(argv[i], "--closebutton") == 0 && argc >= i + 2)
        {
            QString str(argv[i+1]);
            QStringList parts = str.split(",");
            if (parts.size() == 4)
            {
                QRect rect(parts[0].toInt(), parts[1].toInt(), parts[2].toInt(), parts[3].toInt());
                if (rect.isValid() == true)
                    QLCArgs::closeButtonRect = rect;
            }
        }
    }

    return true;
}

/**
 * THE entry point for the application
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 */
int main(int argc, char** argv)
{
    /* Create the Qt core application object */
    QApplication qapp(argc, argv);

#ifdef __APPLE__
    /* Load plugins from within the bundle ONLY */
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

    QLCi18n::init();

    /* Let the world know... */
    printVersion();

    /* Parse command-line arguments */
    if (parseArgs(argc, argv) == false)
        return 0;

    /* Load translation for main application */
    QLCi18n::loadTranslation("qlc");

    /* Handle debug messages */
    qInstallMsgHandler(qlcMessageHandler);

    /* Create and initialize the QLC application object */
    App app;
    app.show();

    if (QLCArgs::workspace.isEmpty() == false)
        app.loadXML(QLCArgs::workspace);
    if (QLCArgs::operate == true)
        app.slotModeOperate();
    if (QLCArgs::kioskMode == true)
        app.enableKioskMode();
    if (QLCArgs::fullScreen == true)
        QTimer::singleShot(0, &app, SLOT(slotControlFullScreen()));
    if (QLCArgs::kioskMode == true && QLCArgs::closeButtonRect.isValid() == true)
        app.createKioskCloseButton(QLCArgs::closeButtonRect);

    return qapp.exec();
}
