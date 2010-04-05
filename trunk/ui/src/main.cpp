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
#include <QLocale>
#include <QString>
#include <QDebug>
#include <QHash>

#include "qlcconfig.h"
#include "qlctypes.h"

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
	bool operate;

	/**
	 * Specifies a workspace file name to load after all initialization
	 * has been done, but before switching to operate mode (if applicable)
	 */
	QString workspace;

	/**
	 * Specifies a locale for forced translation
	 */
	QString locale;
}

/**
 * Prints the application version
 */
void printVersion()
{
	QTextStream cout(stdout, QIODevice::WriteOnly);

	cout << endl;
	cout << App::longName() << " " << App::version() << endl;
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
	cout << "  -o <file> or --open <file>\tOpen the specified workspace file" << endl;
	cout << "  -p or --operate\t\tStart in operate mode" << endl;
	cout << "  -l <locale> or --locale <locale>\tForce QLC to use the given locale for translation" << endl;
	cout << "  -h or --help\t\t\tPrint this help" << endl;
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
			QLCArgs::locale = QString(argv[++i]);
		}
	}

	return true;
}

void loadTranslation(const QString& locale, QApplication& app)
{
	QString lc;
	if (QLCArgs::locale.isEmpty() == true)
		lc = locale;
	else
		lc = QLCArgs::locale;
	QString file(QString("qlc_%1").arg(lc));

#ifdef __APPLE__
	QString path(QString("%1/../%2").arg(QApplication::applicationDirPath())
					.arg(TRANSLATIONDIR));
#else
	QString path(TRANSLATIONDIR);
#endif
	QTranslator* translator = new QTranslator(&app);
	if (translator->load(file, path) == true)
	{
		qDebug() << "Using translation for" << lc;
		QCoreApplication::installTranslator(translator);
	}
	else
	{
		qDebug() << "Unable to find translation for" << lc;
	}
}

/**
 * THE entry point for the application
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 */
int main(int argc, char** argv)
{
	/* Let the world know... */
	printVersion();

	/* Parse command-line arguments */
	if (parseArgs(argc, argv) == false)
		return 0;

	/* Create the Qt core application object */
	QApplication qapp(argc, argv);

	/* Load translation for current locale */
	loadTranslation(QLocale::system().name(), qapp);

	/* Registering needed to pass signals with these types between
	   different contexts (threads) */
	qRegisterMetaType <t_function_id>("t_function_id");
	qRegisterMetaType <QHash<t_channel,t_value> >("QHash<t_channel,t_value>");

	/* Create and initialize the QLC application object */
	App app;
	if (QLCArgs::operate == true)
		app.slotModeOperate();
	if (QLCArgs::workspace.isEmpty() == false)
		app.loadXML(QLCArgs::workspace);

	/* Show and execute the application */
	app.show();

	return qapp.exec();
}
