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

#include "LuaLCD.h"
#include "ui_LuaLCD.h"

#include "LuaEngine.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
}

#include "Lua_lrotable.h"

#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QFontMetrics>
#include <QFileInfo>
#include <QResizeEvent>
#include <assert.h>

#include <limits>

template<class t> inline t limit(t mi, t x, t ma) { return std::min(std::max(mi,x),ma); }

// ============================================================================

thread_local QPointer<CLuaLCD> CLuaLCD::g_luaLCD;

// ============================================================================

CLuaLCD::CLuaLCD(QWidget *parent) :
	QLabel(parent),
	m_pixmap(LCD_W*LCD_RES_SCALING, LCD_H*LCD_RES_SCALING),
	ui(new Ui::CLuaLCD)
{
	ui->setupUi(this);

	setPixmap(m_pixmap.scaled(width(), height(), Qt::KeepAspectRatio));

	setTheme(LCD_THEME_DEFAULT);

	// Set the Lua LCD on this thread to be this one:
	g_luaLCD = this;
}

CLuaLCD::~CLuaLCD()
{
	delete ui;
}

// ----------------------------------------------------------------------------

void CLuaLCD::setTheme(LCD_THEME_ENUM nTheme)
{
	switch (nTheme) {
		case LCD_THEME_DEFAULT:
			m_lcdColorTable[TEXT_COLOR_INDEX] = BLACK;
			m_lcdColorTable[TEXT_BGCOLOR_INDEX] = WHITE;
			m_lcdColorTable[TEXT_INVERTED_COLOR_INDEX] = WHITE;
			m_lcdColorTable[TEXT_INVERTED_BGCOLOR_INDEX] = RED;
			m_lcdColorTable[TEXT_STATUSBAR_COLOR_INDEX] = WHITE;
			m_lcdColorTable[LINE_COLOR_INDEX] = GREY;
			m_lcdColorTable[SCROLLBOX_COLOR_INDEX] = RED;
			m_lcdColorTable[MENU_TITLE_BGCOLOR_INDEX] = DARKGREY;
			m_lcdColorTable[MENU_TITLE_COLOR_INDEX] = WHITE;
			m_lcdColorTable[MENU_TITLE_DISABLE_COLOR_INDEX] = RGB32(GET_RED32(RED)>>1, GET_GREEN32(RED)>>1, GET_BLUE32(RED)>>1);
			m_lcdColorTable[HEADER_COLOR_INDEX] = DARKGREY;
			m_lcdColorTable[ALARM_COLOR_INDEX] = RED;
			m_lcdColorTable[WARNING_COLOR_INDEX] = YELLOW;
			m_lcdColorTable[TEXT_DISABLE_COLOR_INDEX] = GREY;
			m_lcdColorTable[CURVE_AXIS_COLOR_INDEX] = LIGHTGREY;
			m_lcdColorTable[CURVE_COLOR_INDEX] = RED;
			m_lcdColorTable[CURVE_CURSOR_COLOR_INDEX] = RED;
			m_lcdColorTable[TITLE_BGCOLOR_INDEX] = RED;
			m_lcdColorTable[TRIM_BGCOLOR_INDEX] = RED;
			m_lcdColorTable[TRIM_SHADOW_COLOR_INDEX] = BLACK;
			m_lcdColorTable[MAINVIEW_PANES_COLOR_INDEX] = WHITE;
			m_lcdColorTable[MAINVIEW_GRAPHICS_COLOR_INDEX] = RED;
			m_lcdColorTable[HEADER_BGCOLOR_INDEX] = DARKRED;
			m_lcdColorTable[HEADER_ICON_BGCOLOR_INDEX] = RED;
			m_lcdColorTable[HEADER_CURRENT_BGCOLOR_INDEX] = RED;
			m_lcdColorTable[OVERLAY_COLOR_INDEX] = BLACK;
			m_lcdColorTable[BARGRAPH1_COLOR_INDEX] = RED;
			m_lcdColorTable[BARGRAPH2_COLOR_INDEX] = RGB32(167, 167, 167);
			m_lcdColorTable[BARGRAPH_BGCOLOR_INDEX] = RGB32(222, 222, 222);
			break;
		case LCD_THEME_DARKBLUE:
			m_lcdColorTable[TEXT_COLOR_INDEX] = WHITE;
			m_lcdColorTable[TEXT_BGCOLOR_INDEX] = RGB32(10, 78, 121);
			m_lcdColorTable[TEXT_INVERTED_COLOR_INDEX] = WHITE;
			m_lcdColorTable[TEXT_INVERTED_BGCOLOR_INDEX] = RGB32(32, 34, 42);
			m_lcdColorTable[TEXT_STATUSBAR_COLOR_INDEX] = WHITE;
			m_lcdColorTable[LINE_COLOR_INDEX] = LIGHTGREY;
			m_lcdColorTable[SCROLLBOX_COLOR_INDEX] = WHITE;
			m_lcdColorTable[MENU_TITLE_BGCOLOR_INDEX] = DARKGREY;
			m_lcdColorTable[MENU_TITLE_COLOR_INDEX] = WHITE;
			m_lcdColorTable[MENU_TITLE_DISABLE_COLOR_INDEX] = BLACK;
			m_lcdColorTable[HEADER_COLOR_INDEX] = RGB32(32, 34, 42);
			m_lcdColorTable[ALARM_COLOR_INDEX] = RGB32(32, 34, 42);
			m_lcdColorTable[WARNING_COLOR_INDEX] = YELLOW;
			m_lcdColorTable[TEXT_DISABLE_COLOR_INDEX] = GREY;
			m_lcdColorTable[CURVE_AXIS_COLOR_INDEX] = LIGHTGREY;
			m_lcdColorTable[CURVE_COLOR_INDEX] = RGB32(32, 34, 42);
			m_lcdColorTable[CURVE_CURSOR_COLOR_INDEX] = RGB32(32, 34, 42);
			m_lcdColorTable[TITLE_BGCOLOR_INDEX] = RGB32(32, 34, 42);
			m_lcdColorTable[TRIM_BGCOLOR_INDEX] = RGB32(32, 34, 42);
			m_lcdColorTable[TRIM_SHADOW_COLOR_INDEX] = RGB32(100, 100, 100);
			m_lcdColorTable[MAINVIEW_PANES_COLOR_INDEX] = GREY;
			m_lcdColorTable[MAINVIEW_GRAPHICS_COLOR_INDEX] = BLUE;
			m_lcdColorTable[HEADER_BGCOLOR_INDEX] = BLACK;
			m_lcdColorTable[HEADER_ICON_BGCOLOR_INDEX] = BLACK;
			m_lcdColorTable[HEADER_CURRENT_BGCOLOR_INDEX] = RGB32(10, 78, 121);
			m_lcdColorTable[OVERLAY_COLOR_INDEX] = BLACK;
			m_lcdColorTable[BARGRAPH1_COLOR_INDEX] = RGB32(15, 150, 250);
			m_lcdColorTable[BARGRAPH2_COLOR_INDEX] = RGB32(167, 167, 167);
			m_lcdColorTable[BARGRAPH_BGCOLOR_INDEX] = RGB32(97, 97, 102);
			break;
		case LCD_THEME_FLEXI:
			m_lcdColorTable[TEXT_COLOR_INDEX] = BLACK;
			m_lcdColorTable[TEXT_BGCOLOR_INDEX] = LIGHTWHITE;
			m_lcdColorTable[TEXT_INVERTED_COLOR_INDEX] = LIGHTWHITE;
			m_lcdColorTable[TEXT_INVERTED_BGCOLOR_INDEX] = DARKBROWN;
			m_lcdColorTable[TEXT_STATUSBAR_COLOR_INDEX] = LIGHTWHITE;
			m_lcdColorTable[LINE_COLOR_INDEX] = GREY;
			m_lcdColorTable[SCROLLBOX_COLOR_INDEX] = DARKBROWN;
			m_lcdColorTable[MENU_TITLE_BGCOLOR_INDEX] = LIGHTWHITE;
			m_lcdColorTable[MENU_TITLE_COLOR_INDEX] = LIGHTWHITE;
			m_lcdColorTable[MENU_TITLE_DISABLE_COLOR_INDEX] = LIGHTWHITE;
			m_lcdColorTable[HEADER_COLOR_INDEX] = LIGHTGREY;
			m_lcdColorTable[ALARM_COLOR_INDEX] = RED;
			m_lcdColorTable[WARNING_COLOR_INDEX] = YELLOW;
			m_lcdColorTable[TEXT_DISABLE_COLOR_INDEX] = GREY;
			m_lcdColorTable[CURVE_AXIS_COLOR_INDEX] = LIGHTGREY;
			m_lcdColorTable[CURVE_COLOR_INDEX] = RED;
			m_lcdColorTable[CURVE_CURSOR_COLOR_INDEX] = DARKBROWN;
			m_lcdColorTable[TITLE_BGCOLOR_INDEX] = DARKBROWN;
			m_lcdColorTable[TRIM_BGCOLOR_INDEX] = LIGHTBROWN;
			m_lcdColorTable[TRIM_SHADOW_COLOR_INDEX] = BLACK;
			m_lcdColorTable[MAINVIEW_PANES_COLOR_INDEX] = LIGHTWHITE;
			m_lcdColorTable[MAINVIEW_GRAPHICS_COLOR_INDEX] = DARKBROWN;
			m_lcdColorTable[HEADER_BGCOLOR_INDEX] = DARKBROWN; // Top bar color
			m_lcdColorTable[HEADER_ICON_BGCOLOR_INDEX] = LIGHTBROWN; // background OPENTX icon
			m_lcdColorTable[HEADER_CURRENT_BGCOLOR_INDEX] = GREEN; // icons color
			m_lcdColorTable[OVERLAY_COLOR_INDEX] = LIGHTWHITE;
			m_lcdColorTable[BARGRAPH1_COLOR_INDEX] = ORANGE;
			m_lcdColorTable[BARGRAPH2_COLOR_INDEX] = BRIGHTGREEN;
			m_lcdColorTable[BARGRAPH_BGCOLOR_INDEX] = RGB32(220, 220, 220);
			break;
	}
}

void CLuaLCD::setColor(int ndx, const QColor &color)
{
	if ((ndx >= 0) && (ndx < LCD_COLOR_COUNT)) m_lcdColorTable[ndx] = color.value();
}

QColor CLuaLCD::color(int ndx)
{
	if ((ndx >= 0) && (ndx < LCD_COLOR_COUNT)) return m_lcdColorTable[ndx];
	return QColor();
}

// ----------------------------------------------------------------------------

void CLuaLCD::resizeEvent(QResizeEvent *event)
{
	assert(event != nullptr);
	setPixmap(m_pixmap.scaled(event->size().width(), event->size().height(), Qt::KeepAspectRatio));
}

void CLuaLCD::updateLCD()
{
	setPixmap(m_pixmap.scaled(width(), height(), Qt::KeepAspectRatio));
}

// ----------------------------------------------------------------------------

void CLuaLCD::clear(LcdFlags att)
{
	QPainter painter(&m_pixmap);
	painter.fillRect(0, 0, LCD_W*LCD_RES_SCALING, LCD_H*LCD_RES_SCALING, QColor(QRgb(m_lcdColorTable[COLOR_IDX(att)])));
	updateLCD();
}

void CLuaLCD::drawPoint(coord_t x, coord_t y, LcdFlags att)
{
	QPainter painter(&m_pixmap);
	painter.setBrush(Qt::NoBrush);
	painter.setPen(QPen(QColor(QRgb(m_lcdColorTable[COLOR_IDX(att)])), 1*LCD_RES_SCALING));
	painter.drawPoint(x*LCD_RES_SCALING, y*LCD_RES_SCALING);
	updateLCD();
}

void CLuaLCD::drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint8_t pat, LcdFlags att)
{
	QPainter painter(&m_pixmap);
	Qt::PenStyle ps = Qt::SolidLine;
	switch (pat) {
		case DOTTED:
			ps = Qt::DotLine;
			break;
		case STASHED:
			ps = Qt::DashLine;
			break;
	}
	painter.setBrush(Qt::NoBrush);
	painter.setPen(QPen(QBrush(QColor(QRgb(m_lcdColorTable[COLOR_IDX(att)]))), 1*LCD_RES_SCALING, ps));
	painter.drawLine(x1*LCD_RES_SCALING, y1*LCD_RES_SCALING, x2*LCD_RES_SCALING, y2*LCD_RES_SCALING);
	updateLCD();
}

static uint8_t getFontHeight(LcdFlags flags)
{
  static const uint8_t heightTable[16] = { 9, 13, 16, 24, 32, 64, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12 };
  return heightTable[FONTINDEX(flags)];
}

void CLuaLCD::drawText(coord_t x, coord_t y, const char * s, LcdFlags att)
{
	QPainter painter(&m_pixmap);
	painter.setBrush(Qt::NoBrush);
	painter.setPen(QPen(QBrush(QColor(QRgb(m_lcdColorTable[COLOR_IDX(att)]))), 1*LCD_RES_SCALING));
	QFont font = painter.font();
	font.setPixelSize(getFontHeight(att)*LCD_RES_SCALING);
	painter.setFont(font);
	QString strText = QString::fromUtf8(s);			// TODO: Should this be UTF8 or Latin1?
	QFontMetrics fm(font);
	QSize sz = fm.size(0, strText);
	Qt::Alignment align = Qt::AlignLeft;
	if (att & CENTERED) {
		align = Qt::AlignHCenter;
	} else if (att & RIGHT) {
		align = Qt::AlignRight;
	}
	painter.drawText(QRect(QPoint(x*LCD_RES_SCALING, y*LCD_RES_SCALING), sz*LCD_RES_SCALING), align, strText);
	updateLCD();
}

void CLuaLCD::drawBitmap(coord_t x, coord_t y, const QPixmap &bm, const QRect &src, float scale)
{
	QPainter painter(&m_pixmap);
	if (scale) {
		painter.scale(scale, scale);
	}
	painter.drawPixmap(x*LCD_RES_SCALING, y*LCD_RES_SCALING, bm, src.x(), src.y(), src.width(), src.height());
	updateLCD();
}

void CLuaLCD::drawRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness, uint8_t pat, LcdFlags att)
{
	QPainter painter(&m_pixmap);
	Qt::PenStyle ps = Qt::SolidLine;
	switch (pat) {
		case DOTTED:
			ps = Qt::DotLine;
			break;
		case STASHED:
			ps = Qt::DashLine;
			break;
	}
	painter.setBrush(Qt::NoBrush);
	painter.setPen(QPen(QBrush(QColor(QRgb(m_lcdColorTable[COLOR_IDX(att)]))), thickness*LCD_RES_SCALING, ps));
	painter.drawRect(x*LCD_RES_SCALING, y*LCD_RES_SCALING, w*LCD_RES_SCALING, h*LCD_RES_SCALING);
	updateLCD();
}

void CLuaLCD::drawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat, LcdFlags att)
{
	Q_UNUSED(pat);
	QPainter painter(&m_pixmap);
	painter.fillRect(x*LCD_RES_SCALING, y*LCD_RES_SCALING, w*LCD_RES_SCALING, h*LCD_RES_SCALING, QBrush(QColor(QRgb(m_lcdColorTable[COLOR_IDX(att)]))));
	updateLCD();
}


int CLuaLCD::loadBitmap(const QString &strFilename)
{
	// Convert common OpenTx SD Card path prefixes to relative paths:
	QString strFN = strFilename;
	if (strFN.startsWith("/SCRIPTS/TOOLS/", Qt::CaseInsensitive)) {
		strFN.replace("/SCRIPTS/TOOLS/", "./", Qt::CaseInsensitive);
	} else if (strFN.startsWith("/IMAGES/", Qt::CaseInsensitive)) {
		strFN.replace("/IMAGES", "./", Qt::CaseInsensitive);
	} else if (strFN.startsWith("/")) {
		strFN.prepend(".");
	}

	QFileInfo fi(CLuaEngine::currentStandaloneScriptPath());
	if (!fi.filePath().isEmpty()) {
		strFN.prepend(fi.absolutePath() + "/");
	}

	m_lstBitmaps.append(QPixmap(strFN));
	return m_lstBitmaps.size()-1;
}

void CLuaLCD::freeBitmap(int ndx)
{
	if ((m_lstBitmaps.size() > ndx) && (ndx >= 0)) {
		m_lstBitmaps[ndx] = QPixmap();
	}
}

const QPixmap &CLuaLCD::bitmap(int ndx)
{
	assert((m_lstBitmaps.size() > ndx) && (ndx >= 0));
	return m_lstBitmaps.at(ndx);
}

bool CLuaLCD::checkBitmap(int ndx)
{
	return ((m_lstBitmaps.size() > ndx) && (ndx >= 0));
}


// ============================================================================




// lcd.refresh()
//
// Refresh the LCD screen
static int luaLcdRefresh(lua_State *L)
{
	Q_UNUSED(L);
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
// TODO : Do we need to do anything here?
//	lcdRefresh();
	return 0;
}


// lcd.clear([color])
//
// Clear the LCD screen
//	[color] (optional, only on color screens)
static int luaLcdClear(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
	LcdFlags color = luaL_optunsigned(L, 1, TEXT_BGCOLOR);
	CLuaLCD::g_luaLCD->clear(color);
	return 0;
}


// lcd.resetBacklightTimeout()
//
// Reset the backlight timeout
static int luaLcdResetBacklightTimeout(lua_State * L)
{
	Q_UNUSED(L);
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
// TODO : Emulate backlight?
//	resetBacklightTimeout();
	return 0;
}


// lcd.drawPoint(x, y, [flags])
//
// Draw a single pixel at (x,y) position
//	x (positive number) x position
//	y (positive number) y position
//	[flags] (optional) lcdflags
static int luaLcdDrawPoint(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	LcdFlags att = luaL_optunsigned(L, 3, 0);
	CLuaLCD::g_luaLCD->drawPoint(x, y, att);
	return 0;
}


// lcd.drawLine(x1, y1, x2, y2, pattern, flags)
//
// Draw a straight line on LCD
//	x1,y1 (positive numbers) starting coordinate
//	x2,y2 (positive numbers) end coordinate
//	pattern SOLID or DOTTED
//	flags lcdflags
static int luaLcdDrawLine(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
	coord_t x1 = luaL_checkunsigned(L, 1);
	coord_t y1 = luaL_checkunsigned(L, 2);
	coord_t x2 = luaL_checkunsigned(L, 3);
	coord_t y2 = luaL_checkunsigned(L, 4);
	uint8_t pat = luaL_checkunsigned(L, 5);
	LcdFlags flags = luaL_checkunsigned(L, 6);

	if (x1 > LCD_W || y1 > LCD_H || x2 > LCD_W || y2 > LCD_H)
		return 0;

	CLuaLCD::g_luaLCD->drawLine(x1, y1, x2, y2, pat, flags);
	return 0;
}


// lcd.drawText(x, y, text [, flags])
//
// Draw a text beginning at (x,y)
//	x,y (positive numbers) starting coordinate
//	text (string) text to display
//	flags (unsigned number) drawing flags. All values can be
//		combined together using the + character. ie BLINK + DBLSIZE.
static int luaLcdDrawText(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	const char * s = luaL_checkstring(L, 3);
	unsigned int att = luaL_optunsigned(L, 4, 0);
	if ((att&SHADOWED) && !(att&INVERS)) CLuaLCD::g_luaLCD->drawText(x+1, y+1, s, att&0xFFFF);

	CLuaLCD::g_luaLCD->drawText(x, y, s, att);
	return 0;
}

constexpr int LEN_TIMER_STRING = 10;	// "-00:00:00"
constexpr int secondsPerDay = 24 * 3600;
constexpr int secondsPer99Hours = 99*3600 + 59*60 + 59;
constexpr int secondsPerYear = 365 * secondsPerDay;

static char * getTimerString(char * dest, int tme, uint8_t hours)
{
	char * s = dest;
	div_t qr;

	if (tme < 0) {
		tme = -tme;
		*s++ = '-';
	}

	if (tme < secondsPerDay) {
		qr = div((int) tme, 60);

		if (hours) {
			div_t qr2 = div(qr.quot, 60);
			*s++ = '0' + (qr2.quot / 10);
			*s++ = '0' + (qr2.quot % 10);
			*s++ = ':';
			qr.quot = qr2.rem;
		}

		if (!hours && qr.quot > 99) {
			*s++ = '0' + (qr.quot / 100);
			qr.quot = qr.quot % 100;
		}

		*s++ = '0' + (qr.quot / 10);
		*s++ = '0' + (qr.quot % 10);
		*s++ = ':';
		*s++ = '0' + (qr.rem / 10);
		*s++ = '0' + (qr.rem % 10);
		*s = '\0';
	} else if (tme < secondsPer99Hours) {
		qr = div(tme, 3600);
		div_t qr2 = div(qr.rem, 60);
		*s++ = '0' + (qr.quot / 10);
		*s++ = '0' + (qr.quot % 10);
		*s++ = 'H';
		*s++ = '0' + (qr2.quot / 10);
		*s++ = '0' + (qr2.quot % 10);
		*s = '\0';
	} else if (tme < secondsPerYear) {
		qr = div(tme, secondsPerDay);
		div_t qr2 = div(qr.rem, 60);
		*s++ = '0' + (qr.quot / 100);
		*s++ = '0' + (qr.quot / 10);
		*s++ = '0' + (qr.quot % 10);
		*s++ = 'D';
		*s++ = '0' + (qr2.quot / 10);
		*s++ = '0' + (qr2.quot % 10);
		*s++ = 'H';
		*s = '\0';
	} else {
		qr = div(tme, secondsPerYear);
		div_t qr2 = div(qr.rem, secondsPerDay);
		*s++ = '0' + (qr.quot / 10);
		*s++ = '0' + (qr.quot % 10);
		*s++ = 'Y';
		*s++ = 'Y';
		*s++ = '0' + (qr2.quot / 10);
		*s++ = '0' + (qr2.quot % 10);
		*s++ = 'D';
		*s = '\0';
	}
	return dest;
}

static void drawTimer(coord_t x, coord_t y, int32_t tme, LcdFlags flags)
{
	if (CLuaLCD::g_luaLCD.isNull()) return;
	char str[LEN_TIMER_STRING];
	getTimerString(str, tme, (flags & TIMEHOUR) != 0);
	CLuaLCD::g_luaLCD->drawText(x, y, str, flags);
}



// lcd.drawTimer(x, y, value [, flags])
//
// Display a value formatted as time at (x,y)
//	x,y (positive numbers) starting coordinate
//	value (number) time in seconds
//	flags (unsigned number) drawing flags:
//		`0 or not specified` normal representation (minutes and seconds)
//		`TIMEHOUR` display hours other general LCD flag also apply
//		`SHADOWED` Horus only, apply a shadow effect
static int luaLcdDrawTimer(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int seconds = luaL_checkinteger(L, 3);
	unsigned int att = luaL_optunsigned(L, 4, 0);
	if (att&SHADOWED) drawTimer(x+1, y+1, seconds, (att&0xFFFF)|LEFT);
	drawTimer(x, y, seconds, att|LEFT);
	return 0;
}


static void drawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags = 0, uint8_t len = 0, const char * prefix = nullptr, const char * suffix = nullptr)
{
	if (CLuaLCD::g_luaLCD.isNull()) return;
	char str[48+1]; // max=16 for the prefix, 16 chars for the number, 16 chars for the suffix
	char *s = str+32;
	*s = '\0';
	int idx = 0;
	int mode = MODE(flags);
	bool neg = false;

	if (val == INT_MAX) {
		flags &= ~(LEADING0 | PREC1 | PREC2);
		CLuaLCD::g_luaLCD->drawText(x, y, "INT_MAX", flags);
		return;
	}

	if (val < 0) {
		if (val == INT_MIN) {
			flags &= ~(LEADING0 | PREC1 | PREC2);
			CLuaLCD::g_luaLCD->drawText(x, y, "INT_MIN", flags);
			return;
		}
		val = -val;
		neg = true;
	}
	do {
		*--s = '0' + (val % 10);
		++idx;
		val /= 10;
		if (mode!=0 && idx==mode) {
			mode = 0;
			*--s = '.';
			if (val==0)
				*--s = '0';
		}
	} while (val!=0 || mode>0 || (mode==MODE(LEADING0) && idx<len));
	if (neg) *--s = '-';

	// TODO needs check on all string lengths ...
	if (prefix) {
		int len = strlen(prefix);
		if (len <= 16) {
			s -= len;
			strncpy(s, prefix, len);
		}
	}
	if (suffix) {
		strncpy(&str[32], suffix, 16);
	}
	flags &= ~LEADING0;
	CLuaLCD::g_luaLCD->drawText(x, y, s, flags);
}


// lcd.drawNumber(x, y, value [, flags])
//
// Display a number at (x,y)
//	x,y (positive numbers) starting coordinate
//	value (number) value to display
//	flags (unsigned number) drawing flags
static int luaLcdDrawNumber(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int val = luaL_checkinteger(L, 3);
	unsigned int att = luaL_optunsigned(L, 4, 0);
	if ((att&SHADOWED) && !(att&INVERS)) drawNumber(x, y, val, att&0xFFFF);
	drawNumber(x, y, val, att);
	return 0;
}


// lcd.drawChannel(x, y, source, flags)
//
// Display a telemetry value at (x,y)
//	x,y (positive numbers) starting coordinate
//	source can be a source identifier (number) or a source name (string).
//	flags (unsigned number) drawing flags
static int luaLcdDrawChannel(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int channel = -1;
	if (lua_isnumber(L, 3)) {
		channel = luaL_checkinteger(L, 3);
	} else {
//		const char * what = luaL_checkstring(L, 3);
//		LuaField field;
//		bool found = luaFindFieldByName(what, field);
//		if (found) {
//			channel = field.id;
//		}
	}
	unsigned int att = luaL_optunsigned(L, 4, 0);
// TODO : Finish This
//	getvalue_t value = getValue(channel);
//	drawSensorCustomValue(x, y, (channel-MIXSRC_FIRST_TELEM)/3, value, att);
	return 0;
}


// lcd.drawSwitch(x, y, switch, flags)
//
// Draw a text representation of switch at (x,y)
//	x,y (positive numbers) starting coordinate
//	switch (number) number of switch to display, negative number
//		displays negated switch
//	flags (unsigned number) drawing flags, only SMLSIZE, BLINK and INVERS.
static int luaLcdDrawSwitch(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int s = luaL_checkinteger(L, 3);
	unsigned int att = luaL_optunsigned(L, 4, 0);
// TODO : Finish this
//	drawSwitch(x, y, s, att);
	return 0;
}


// lcd.drawSource(x, y, source [, flags])
//
// Displays the name of the corresponding input as defined by the source at (x,y)
//	x,y (positive numbers) starting coordinate
//	source (number) source index
//	flags (unsigned number) drawing flags
static int luaLcdDrawSource(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int s = luaL_checkinteger(L, 3);
	unsigned int att = luaL_optunsigned(L, 4, 0);
// TODO : Finish this
//	drawSource(x, y, s, att);
	return 0;
}

#define LUA_BITMAPHANDLE          "BITMAP*"

// Bitmap.open(name)
//
//	Loads a bitmap in memory, for later use with lcd.drawBitmap(). Bitmaps should be loaded only
//	once, returned object should be stored and used for drawing. If loading fails for whatever
//	reason the resulting bitmap object will have width and height set to zero.
//
//	name (string) full path to the bitmap on SD card (i.e. “/IMAGES/test.bmp”)
//	retval: bitmap (object) a bitmap object that can be used with other bitmap functions
static int luaOpenBitmap(lua_State * L)
{
	const char * filename = luaL_checkstring(L, 1);

	int *b = (int *)lua_newuserdata(L, sizeof(int));
	if (!CLuaLCD::g_luaLCD.isNull()) {
		int nIndex = CLuaLCD::g_luaLCD->loadBitmap(QString::fromUtf8(filename));		// TODO : UTF8 or Latin1 here?
		*b = nIndex;
	} else {
		*b = -1;
	}

	luaL_getmetatable(L, LUA_BITMAPHANDLE);
	lua_setmetatable(L, -2);

	return 1;
}


static int checkBitmap(lua_State * L, int index)
{
	const int *b = (const int*)luaL_checkudata(L, index, LUA_BITMAPHANDLE);
	return *b;
}


// Bitmap.getSize(name)
//
// Return width, height of a bitmap object
//	bitmap (pointer) point to a bitmap previously opened with Bitmap.open()
//	retval: multiple returns 2 values:
//		(number) width in pixels
//		(number) height in pixels
static int luaGetBitmapSize(lua_State * L)
{
	const int b = checkBitmap(L, 1);

	if (!CLuaLCD::g_luaLCD.isNull() && CLuaLCD::g_luaLCD->checkBitmap(b)) {
		lua_pushinteger(L, CLuaLCD::g_luaLCD->bitmap(b).width());
		lua_pushinteger(L, CLuaLCD::g_luaLCD->bitmap(b).height());
	} else {
		lua_pushinteger(L, 0);
		lua_pushinteger(L, 0);
	}
	return 2;
}

static int luaDestroyBitmap(lua_State * L)
{
	int b = checkBitmap(L, 1);
	if (!CLuaLCD::g_luaLCD.isNull() && CLuaLCD::g_luaLCD->checkBitmap(b))
		CLuaLCD::g_luaLCD->freeBitmap(b);

	return 0;
}


const luaL_Reg bitmapFuncs[] = {
	{ "open", luaOpenBitmap },
	{ "getSize", luaGetBitmapSize },
	{ "__gc", luaDestroyBitmap },
	{ NULL, NULL }
};

void registerBitmapClass(lua_State * L)
{
	luaL_newmetatable(L, LUA_BITMAPHANDLE);
	luaL_setfuncs(L, bitmapFuncs, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_setglobal(L, "Bitmap");
}


// lcd.drawBitmap(bitmap, x, y [, scale])
//
// Displays a bitmap at (x,y)
//	bitmap (pointer) point to a bitmap previously opened with Bitmap.open()
//	x,y (positive numbers) starting coordinates
//	scale (positive numbers) scale in %, 50 divides size by two, 100 is unchanged, 200 doubles size.
//	Omitting scale draws image in 1:1 scale and is faster than specifying 100 for scale.
static int luaLcdDrawBitmap(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;

	int b = checkBitmap(L, 1);

	if (CLuaLCD::g_luaLCD->checkBitmap(b)) {
		const QPixmap &bm = CLuaLCD::g_luaLCD->bitmap(b);
		unsigned int x = luaL_checkunsigned(L, 2);
		unsigned int y = luaL_checkunsigned(L, 3);
		unsigned int scale = luaL_optunsigned(L, 4, 0);
		QRect rc(0, 0, bm.width(), bm.height());
		if (scale) {
			CLuaLCD::g_luaLCD->drawBitmap(x, y, bm, rc, scale/100.0f);
		} else {
			CLuaLCD::g_luaLCD->drawBitmap(x, y, bm, rc, 0);
		}
	}
	return 0;
}


// lcd.drawRectangle(x, y, w, h [, flags [, t]])
//
// Draw a rectangle from top left corner (x,y) of specified width and height
//	x,y (positive numbers) top left corner position
//	w (number) width in pixels
//	h (number) height in pixels
//	flags (unsigned number) drawing flags
//	t (number) thickness in pixels, defaults to 1
static int luaLcdDrawRectangle(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;

	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);
	unsigned int flags = luaL_optunsigned(L, 5, 0);
	unsigned int t = luaL_optunsigned(L, 6, 1);
	CLuaLCD::g_luaLCD->drawRect(x, y, w, h, t, 0xff, flags);
	return 0;
}


// lcd.drawFilledRectangle(x, y, w, h [, flags])
//
// Draw a solid rectangle from top left corner (x,y) of specified width and height
//	x,y (positive numbers) top left corner position
//	w (number) width in pixels
//	h (number) height in pixels
//	flags (unsigned number) drawing flags
static int luaLcdDrawFilledRectangle(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;

	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);
	unsigned int flags = luaL_optunsigned(L, 5, 0);
	CLuaLCD::g_luaLCD->drawFilledRect(x, y, w, h, SOLID, flags);
	return 0;
}


// lcd.drawGauge(x, y, w, h, fill, maxfill [, flags])
//
// Draw a simple gauge that is filled based upon fill value
//	x,y (positive numbers) top left corner position
//	w (number) width in pixels
//	h (number) height in pixels
//	fill (number) amount of fill to apply
//	maxfill (number) total value of fill
//	flags (unsigned number) drawing flags
static int luaLcdDrawGauge(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;

	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);
	int num = luaL_checkinteger(L, 5);
	int den = luaL_checkinteger(L, 6);
	unsigned int flags = luaL_optunsigned(L, 7, 0);
	CLuaLCD::g_luaLCD->drawRect(x, y, w, h, 1, 0xff, flags);
	uint8_t len = limit((uint8_t)1, uint8_t(w*num/den), uint8_t(w));
	CLuaLCD::g_luaLCD->drawFilledRect(x+1, y+1, len, h-2, SOLID, flags);
	return 0;
}


// lcd.setColor(area, color)
//
// Set a color for specific area
//	area (unsigned number) specific screen area
//	color (number) color in 5/6/5 rgb format.
static int luaLcdSetColor(lua_State *L)
{
	if (CLuaLCD::g_luaLCD.isNull()) return 0;

	unsigned int index = luaL_checkunsigned(L, 1) >> 16;
	unsigned int color = luaL_checkunsigned(L, 2);

	int r = GET_RED565(color);
	int g = GET_GREEN565(color);
	int b = GET_BLUE565(color);

	CLuaLCD::g_luaLCD->setColor(index, QColor(r, g, b));
	return 0;
}


// lcd.getColor(area)
//
// Get the color for specific area : see lcd.setColor for area list
static int luaLcdGetColor(lua_State *L)
{
	unsigned int index = luaL_checkunsigned(L, 1) >> 16;
	if (!CLuaLCD::g_luaLCD.isNull()) {
		lua_pushunsigned(L, RGB32toRGB565(CLuaLCD::g_luaLCD->color(index)));
	} else {
		lua_pushunsigned(L, 0);
	}
	return 1;
}


// lcd.RGB(r, g, b)
//
// Returns a 5/6/5 rgb color code, that can be used with lcd.setColor
//	r (integer) a number between 0x00 and 0xff that expresses te amount of red in the color
//	g (integer) a number between 0x00 and 0xff that expresses te amount of green in the color
//	b (integer) a number between 0x00 and 0xff that expresses te amount of blue in the color
//
// retval: number (integer) rgb color expressed in 5/6/5 format
static int luaRGB(lua_State *L)
{
	int r = luaL_checkinteger(L, 1);
	int g = luaL_checkinteger(L, 2);
	int b = luaL_checkinteger(L, 3);
	lua_pushinteger(L, RGB565(r, g, b));
	return 1;
}

// ----------------------------------------------------------------------------

const luaL_Reg lua_opentx_lcdLib[] = {
	{ "refresh", luaLcdRefresh },
	{ "clear", luaLcdClear },
	{ "resetBacklightTimeout", luaLcdResetBacklightTimeout },
	{ "drawPoint", luaLcdDrawPoint },
	{ "drawLine", luaLcdDrawLine },
	{ "drawRectangle", luaLcdDrawRectangle },
	{ "drawFilledRectangle", luaLcdDrawFilledRectangle },
	{ "drawText", luaLcdDrawText },
	{ "drawTimer", luaLcdDrawTimer },
	{ "drawNumber", luaLcdDrawNumber },
	{ "drawChannel", luaLcdDrawChannel },
	{ "drawSwitch", luaLcdDrawSwitch },
	{ "drawSource", luaLcdDrawSource },
	{ "drawGauge", luaLcdDrawGauge },
	{ "drawBitmap", luaLcdDrawBitmap },
	{ "setColor", luaLcdSetColor },
	{ "getColor", luaLcdGetColor },
	{ "RGB", luaRGB },
	// ----
	{ nullptr, nullptr }	// sentinel
};


const luaR_value_entry lua_opentx_const_lcd[] =
{
	{ "XXLSIZE", XXLSIZE },
	{ "DBLSIZE", DBLSIZE },
	{ "MIDSIZE", MIDSIZE },
	{ "SMLSIZE", SMLSIZE },
	{ "TINSIZE", TINSIZE },
	{ "INVERS", INVERS },
	{ "BOLD", BOLD },
	{ "BLINK", BLINK },
	{ "RIGHT", RIGHT },
	{ "LEFT", LEFT },
	{ "CENTER", CENTERED },
	{ "PREC1", PREC1 },
	{ "PREC2", PREC2 },
	{ "SHADOWED", SHADOWED },
	// ----
	{ "COLOR", ZoneOption::Color },
	{ "BOOL", ZoneOption::Bool },
	{ "STRING", ZoneOption::String },
	{ "TEXT_COLOR", TEXT_COLOR },
	{ "TEXT_BGCOLOR", TEXT_BGCOLOR },
	{ "TEXT_INVERTED_COLOR", TEXT_INVERTED_COLOR },
	{ "TEXT_INVERTED_BGCOLOR", TEXT_INVERTED_BGCOLOR },
	{ "LINE_COLOR", LINE_COLOR },
	{ "SCROLLBOX_COLOR", SCROLLBOX_COLOR },
	{ "MENU_TITLE_BGCOLOR", MENU_TITLE_BGCOLOR },
	{ "MENU_TITLE_COLOR", MENU_TITLE_COLOR },
	{ "MENU_TITLE_DISABLE_COLOR", MENU_TITLE_DISABLE_COLOR },
	{ "HEADER_COLOR", HEADER_COLOR },
	{ "ALARM_COLOR", ALARM_COLOR },
	{ "WARNING_COLOR", WARNING_COLOR },
	{ "TEXT_DISABLE_COLOR", TEXT_DISABLE_COLOR },
	{ "CURVE_AXIS_COLOR", CURVE_AXIS_COLOR },
	{ "CURVE_COLOR", CURVE_COLOR },
	{ "CURVE_CURSOR_COLOR", CURVE_CURSOR_COLOR },
	{ "TITLE_BGCOLOR", TITLE_BGCOLOR },
	{ "TRIM_BGCOLOR", TRIM_BGCOLOR },
	{ "TRIM_SHADOW_COLOR", TRIM_SHADOW_COLOR },
	{ "HEADER_BGCOLOR", HEADER_BGCOLOR },
	{ "HEADER_ICON_BGCOLOR", HEADER_ICON_BGCOLOR },
	{ "HEADER_CURRENT_BGCOLOR", HEADER_CURRENT_BGCOLOR },
	{ "MAINVIEW_PANES_COLOR", MAINVIEW_PANES_COLOR },
	{ "MAINVIEW_GRAPHICS_COLOR", MAINVIEW_GRAPHICS_COLOR },
	{ "OVERLAY_COLOR", OVERLAY_COLOR },
	{ "BARGRAPH1_COLOR", BARGRAPH1_COLOR },
	{ "BARGRAPH2_COLOR", BARGRAPH2_COLOR },
	{ "BARGRAPH_BGCOLOR", BARGRAPH_BGCOLOR },
	{ "CUSTOM_COLOR", CUSTOM_COLOR },
	{ "MENU_HEADER_HEIGHT", MENU_HEADER_HEIGHT },
	// ----
	{ "WHITE", (double)RGB32toRGB565(WHITE) },
	{ "GREY", (double)RGB32toRGB565(GREY) },
	{ "DARKGREY", (double)RGB32toRGB565(DARKGREY) },
	{ "BLACK", (double)RGB32toRGB565(BLACK) },
	{ "YELLOW", (double)RGB32toRGB565(YELLOW) },
	{ "BLUE", (double)RGB32toRGB565(BLUE) },
	{ "LIGHTGREY", (double)RGB32toRGB565(LIGHTGREY) },
	{ "RED", (double)RGB32toRGB565(RED) },
	{ "DARKRED", (double)RGB32toRGB565(DARKRED) },
	// ----
	{ "SOLID", SOLID },
	{ "DOTTED", DOTTED },
	{ "LCD_W", LCD_W },
	{ "LCD_H", LCD_H },

	// ----
	{ nullptr, 0 }	// sentinel
};

// ============================================================================

