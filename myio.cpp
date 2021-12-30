/****************************************************************************
**
** Copyright (C) 2021 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the frsky_sport_tool Application.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

//
// myio functionality for getch/getche, etc.
//

#include "myio.h"

#include <QtGlobal>

// ============================================================================

CConsoleReader::CConsoleReader(bool bEcho, QObject *pParent)
	:	QThread(pParent),
		m_bEcho(bEcho)
{
	connect(this, SIGNAL(finished()), this, SLOT(deleteLater()), Qt::QueuedConnection);
}

CConsoleReader::~CConsoleReader()
{
}

void CConsoleReader::run()
{
//	forever
//    {
        char key = getch1(m_bEcho);
        emit KeyPressed(key);
//    }
}

// ============================================================================


#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)

	#include <stdio.h>
	#include <unistd.h>
	#include <termios.h>
	#include <fcntl.h>

	int my_kbhit(void)
	{
		struct termios oldt, newt;
		int ch;
		int oldf;

		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
		fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

		ch = getchar();

		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		fcntl(STDIN_FILENO, F_SETFL, oldf);

		if(ch != EOF) {
			ungetc(ch, stdin);
			return 1;
		}

		return 0;
	}

	#ifdef USING_NCURSES

		//
		// Linux ncurses mode:
		//

		#include <ncurses.h>

		static struct Tncurses {
			Tncurses()
			{
				initscr();
				endwin();

				savetty();
				nonl();

#ifdef USE_MOUSE_INPUT
				keypad(stdscr, true);
				mmask_t oldMouseMask;
				mousemask(ALL_MOUSE_EVENTS, &oldMouseMask);			// Hook and unhook the mouse to force load and initialize the driver so that has_mouse() reports correct status
				mousemask(oldMouseMask, nullptr);
#endif

				setvbuf(stdout, nullptr, _IOLBF, 0);
				setvbuf(stderr, nullptr, _IONBF, 0);
			}
			~Tncurses()
			{
				resetty();
				clear();
				endwin();
			}

		} g_ncurses;

		/* Read 1 character - echo defines echo mode */
		static char local_getch(bool bEcho)
		{
			cbreak();
			noecho();
			char ch = static_cast<char>(getchar());
			if (bEcho) putchar(ch);
			resetty();
			return ch;
		}

	#else

		//
		// Linux non-ncurses mode:
		//
		// Based On : https://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux

		class Ttermios {
		public:
			Ttermios(bool bEcho, bool bICANON = false)
			{
				/* Initialize new terminal i/o settings */
				tcgetattr(0, &m_old);							/* grab old terminal i/o settings */
				m_new = m_old;									/* make new settings same as old settings */
				if (bICANON) {
					m_new.c_lflag |= ICANON;
				} else {
					m_new.c_lflag &= static_cast<tcflag_t>(~ICANON);	/* disable buffered i/o */
				}
				if (bEcho) {									/* set echo mode */
					m_new.c_lflag |= ECHO;
				} else {
					m_new.c_lflag &= static_cast<tcflag_t>(~ECHO);
				}
				m_new.c_iflag |= ICRNL;
				m_new.c_iflag &= static_cast<tcflag_t>(~INLCR & ~IGNCR);
				tcsetattr(0, TCSANOW, &m_new);					/* use these new terminal i/o settings now */
			}
			~Ttermios()
			{
				/* Restore old terminal i/o settings */
				tcsetattr(0, TCSANOW, &m_old);
			}

		private:
			termios m_old;
			termios m_new;
		};

		static Ttermios g_termios(true, true);

		/* Read 1 character - echo defines echo mode */
		static char local_getch(bool bEcho)
		{
			Ttermios aTermios(bEcho);
			return static_cast<char>(getchar());
		}

	#endif

#elif defined(Q_OS_WINDOWS)

	//
	// Windows console mode:
	//

	#include <conio.h>

	int my_kbhit(void)
	{
		return kbhit();
	}

	/* Read 1 character - echo defines echo mode */
	static char local_getch(bool bEcho)
	{
		return (bEcho ? getche() : getch());
	}

#else
	#error Unsupported OS for console mode -- add support here
#endif


// ============================================================================

/* Read 1 character without echo */
char my_getch(void)
{
	return local_getch(false);
}

/* Read 1 character with echo */
char my_getche(void)
{
	return local_getch(true);
}

char getch1(bool bEcho)
{
	return ((bEcho) ? my_getche() : my_getch());
}

// ============================================================================

char thread_local g_inputBuf[INPUT_BUF_SIZE];

bool getDouble(double *aDouble)
{
	bool bHaveValue = (my_scanf("%lf", aDouble) >= 1);
	if (!bHaveValue) *aDouble = 0.0;
	return bHaveValue;
}

bool getInt(int *anInt)
{
	bool bHaveValue = (my_scanf("%d", anInt) >= 1);
	if (!bHaveValue) *anInt = 0;
	return bHaveValue;
}

bool getUInt(unsigned int *anUInt)
{
	bool bHaveValue = (my_scanf("%u", anUInt) >= 1);
	if (!bHaveValue) *anUInt = 0;
	return bHaveValue;
}

bool getUIntHex(unsigned int *anUInt)
{
	bool bHaveValue = (my_scanf("%x", anUInt) >= 1);
	if (!bHaveValue) *anUInt = 0;
	return bHaveValue;
}

bool getString(char *pBuf, int nBufSize)
{
	if ((nBufSize < INPUT_BUF_SIZE) || (pBuf == nullptr)) return false;
	bool bHaveValue (fgets(g_inputBuf, sizeof(g_inputBuf), stdin) != nullptr);
	if (!bHaveValue) {
		*pBuf = 0;
		return false;
	}
	bHaveValue = (sscanf(g_inputBuf, "%s", pBuf) >= 1);
	return bHaveValue;
}

// ============================================================================
