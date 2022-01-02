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

#ifndef LUA_LCD_H
#define LUA_LCD_H

#include <QWidget>
#include <QPixmap>
#include <QPointer>
#include <QColor>
#include <QList>
#include <QBitmap>
#include <QPixmap>

// Forward Declarations
extern "C" struct luaR_value_entry;
extern "C" struct luaL_Reg;
struct lua_State;

// ============================================================================

// -------------------
// GUI Implementation:
// -------------------

#define MENU_TOOLTIPS
#define MENU_HEADER_HEIGHT             45
#define MENU_TITLE_TOP                 48
#define MENU_TITLE_HEIGHT              21
#define MENU_BODY_TOP                  (MENU_TITLE_TOP+MENU_TITLE_HEIGHT)
#define MENU_CONTENT_TOP               (MENU_BODY_TOP+1)
#define MENU_FOOTER_HEIGHT             21
#define MENU_FOOTER_TOP                (LCD_H-MENU_FOOTER_HEIGHT)
#define MENU_BODY_HEIGHT               (MENU_FOOTER_TOP-MENU_BODY_TOP)
#define MENUS_MARGIN_LEFT              6

// ----------------------------------------------------------------------------

// ---------
// Core LCD:
// ---------

#define LCD_W                          480
#define LCD_H                          272
#define LCD_DEPTH                      16

// ----------------------------------------------------------------------------

// lcd common flags
#define BLINK                          0x01

// drawText flags
#define INVERS                         0x02
#define LEFT                           0x00 /* align left */
#define CENTERED                       0x04 /* align center */
#define RIGHT                          0x08 /* align right */
#define SHADOWED                       0x80 /* black copy at +1 +1 */

// drawNumber flags
#define LEADING0                       0x10
#define PREC1                          0x20
#define PREC2                          0x30
#define MODE(flags)                    ((((int8_t)(flags) & 0x30) - 0x10) >> 4)

#define ZCHAR                          0x10

// rect, square flags
#define ROUND                          0x04

// telemetry flags
#define NO_UNIT                        0x40

enum FontSizeIndex {
	STDSIZE_INDEX,
	TINSIZE_INDEX,
	SMLSIZE_INDEX,
	MIDSIZE_INDEX,
	DBLSIZE_INDEX,
	XXLSIZE_INDEX,
	SPARE6_INDEX,
	SPARE7_INDEX,
	STDSIZE_BOLD_INDEX,
	SPARE9_INDEX,
	SPAREa_INDEX,
	SPAREb_INDEX,
	SPAREc_INDEX,
	SPAREd_INDEX,
	SPAREe_INDEX,
	SPAREf_INDEX,
};

#define STDSIZE                        (STDSIZE_INDEX << 8)
#define TINSIZE                        (TINSIZE_INDEX << 8)
#define SMLSIZE                        (SMLSIZE_INDEX << 8)
#define MIDSIZE                        (MIDSIZE_INDEX << 8)
#define DBLSIZE                        (DBLSIZE_INDEX << 8)
#define XXLSIZE                        (XXLSIZE_INDEX << 8)
#define BOLD                           (STDSIZE_BOLD_INDEX << 8)
#define FONTSIZE_MASK                  0x0f00

#define FONTSIZE(flags)                ((flags) & FONTSIZE_MASK)
#define FONTINDEX(flags)               (FONTSIZE(flags) >> 8)

#define SOLID   0xff
#define DOTTED  0x55
#define STASHED 0x33

#define TIMEBLINK                      0x1000
#define TIMEHOUR                       0x2000
#define EXPANDED                       0x2000
#define VERTICAL                       0x4000
#define NO_FONTCACHE                   0x8000

// ----------------------------------------------------------------------------

// -----------
// LCD Colors:
// -----------

// NOTE: We will internally be using all 32-bit RGB format and
//	only convert to the 16-bit 565 format as needed to/from Lua
//	functions.

#define RGB565(r, g, b)				(uint16_t)((((r) & 0xF8) << 8) + (((g) & 0xFC) << 3) + (((b) & 0xF8) >> 3))
#define ARGB565(a, r, g, b)			(uint16_t)((((a) & 0xF0) << 8) + (((r) & 0xF0) << 4) + (((g) & 0xF0) << 0) + (((b) & 0xF0) >> 4))

//#define RGB_JOIN(r, g, b) \
//  (((r) << 11) + ((g) << 5) + (b))

#define GET_RED565(color) \
  (((color) & 0xF800) >> 8)

#define GET_GREEN565(color) \
  (((color) & 0x07E0) >> 3)

#define GET_BLUE565(color) \
  (((color) & 0x001F) << 3)

#define RGB32(r, g, b)				(uint32_t)((((r) & 0xFF) << 16) + (((g) & 0xFF) << 8) + ((b) & 0xFF))
#define ARGB32(a, r, g, b)			(uint32_t)((((a) & 0xFF) << 24) + (((r) & 0xFF) << 16) + (((g) & 0xFF) << 8) + ((b) & 0xFF))

#define GET_RED32(color) \
  (((color) & 0x00FF0000) >> 16)

#define GET_GREEN32(color) \
  (((color) & 0x0000FF00) >> 8)

#define GET_BLUE32(color) \
  ((color) & 0x000000FF)

#define WHITE                          RGB32(0xFF, 0xFF, 0xFF)
#define LIGHTWHITE                     RGB32(238, 234, 238)
#define BLACK                          RGB32(0, 0, 0)
#define YELLOW                         RGB32(0xF0, 0xD0, 0x10)
#define BLUE                           RGB32(0x30, 0xA0, 0xE0)
#define GREY                           RGB32(96, 96, 96)
#define DARKGREY                       RGB32(64, 64, 64)
#define LIGHTGREY                      RGB32(180, 180, 180)
#define RED                            RGB32(229, 32, 30)
#define DARKRED                        RGB32(160, 0, 6)
#define GREEN                          RGB32(25, 150, 50)
#define LIGHTBROWN                     RGB32(156, 109, 32)
#define DARKBROWN                      RGB32(106, 72, 16)
#define BRIGHTGREEN                    RGB32(0, 180, 60)
#define ORANGE                         RGB32(229, 100, 30)

inline uint16_t RGB32toRGB565(uint32_t nClr)
{
	QColor clr(nClr);
	return RGB565(clr.red(), clr.green(), clr.blue());
}
inline uint16_t RGB32toRGB565(const QColor &clr)
{
	return RGB32toRGB565(clr.value());
}


#define OPACITY_MAX                    0x0F
#define OPACITY(x)                     ((x)<<24)

enum LcdColorIndex
{
	TEXT_COLOR_INDEX,
	TEXT_BGCOLOR_INDEX,
	TEXT_INVERTED_COLOR_INDEX,
	TEXT_INVERTED_BGCOLOR_INDEX,
	TEXT_STATUSBAR_COLOR_INDEX,
	LINE_COLOR_INDEX,
	SCROLLBOX_COLOR_INDEX,
	MENU_TITLE_BGCOLOR_INDEX,
	MENU_TITLE_COLOR_INDEX,
	MENU_TITLE_DISABLE_COLOR_INDEX,
	HEADER_COLOR_INDEX,
	ALARM_COLOR_INDEX,
	WARNING_COLOR_INDEX,
	TEXT_DISABLE_COLOR_INDEX,
	CURVE_AXIS_COLOR_INDEX,
	CURVE_COLOR_INDEX,
	CURVE_CURSOR_COLOR_INDEX,
	HEADER_BGCOLOR_INDEX,
	HEADER_ICON_BGCOLOR_INDEX,
	HEADER_CURRENT_BGCOLOR_INDEX,
	TITLE_BGCOLOR_INDEX,
	TRIM_BGCOLOR_INDEX,
	TRIM_SHADOW_COLOR_INDEX,
	MAINVIEW_PANES_COLOR_INDEX,
	MAINVIEW_GRAPHICS_COLOR_INDEX,
	OVERLAY_COLOR_INDEX,
	CUSTOM_COLOR_INDEX,
	BARGRAPH1_COLOR_INDEX,
	BARGRAPH2_COLOR_INDEX,
	BARGRAPH_BGCOLOR_INDEX,
	LCD_COLOR_COUNT
};

typedef int coord_t;
typedef uint32_t LcdFlags;

// ----------------------------------------------------------------------------

#define COLOR(index)                   ((index) << 16)
#define COLOR_IDX(att)                 uint8_t((att) >> 16)

#define TEXT_COLOR                     COLOR(TEXT_COLOR_INDEX)
#define TEXT_BGCOLOR                   COLOR(TEXT_BGCOLOR_INDEX)
#define TEXT_INVERTED_COLOR            COLOR(TEXT_INVERTED_COLOR_INDEX)
#define TEXT_INVERTED_BGCOLOR          COLOR(TEXT_INVERTED_BGCOLOR_INDEX)
#define TEXT_STATUSBAR_COLOR           COLOR(TEXT_STATUSBAR_COLOR_INDEX)
#define LINE_COLOR                     COLOR(LINE_COLOR_INDEX)
#define SCROLLBOX_COLOR                COLOR(SCROLLBOX_COLOR_INDEX)
#define HEADER_SEPARATOR_COLOR         COLOR(HEADER_SEPARATOR_COLOR_INDEX)
#define MENU_TITLE_BGCOLOR             COLOR(MENU_TITLE_BGCOLOR_INDEX)
#define MENU_TITLE_COLOR               COLOR(MENU_TITLE_COLOR_INDEX)
#define MENU_TITLE_DISABLE_COLOR       COLOR(MENU_TITLE_DISABLE_COLOR_INDEX)
#define HEADER_COLOR                   COLOR(HEADER_COLOR_INDEX)
#define ALARM_COLOR                    COLOR(ALARM_COLOR_INDEX)
#define WARNING_COLOR                  COLOR(WARNING_COLOR_INDEX)
#define TEXT_DISABLE_COLOR             COLOR(TEXT_DISABLE_COLOR_INDEX)
#define CURVE_AXIS_COLOR               COLOR(CURVE_AXIS_COLOR_INDEX)
#define CURVE_COLOR                    COLOR(CURVE_COLOR_INDEX)
#define CURVE_CURSOR_COLOR             COLOR(CURVE_CURSOR_COLOR_INDEX)
#define TITLE_BGCOLOR                  COLOR(TITLE_BGCOLOR_INDEX)
#define TRIM_BGCOLOR                   COLOR(TRIM_BGCOLOR_INDEX)
#define TRIM_SHADOW_COLOR              COLOR(TRIM_SHADOW_COLOR_INDEX)
#define HEADER_BGCOLOR                 COLOR(HEADER_BGCOLOR_INDEX)
#define HEADER_ICON_BGCOLOR            COLOR(HEADER_ICON_BGCOLOR_INDEX)
#define HEADER_CURRENT_BGCOLOR         COLOR(HEADER_CURRENT_BGCOLOR_INDEX)
#define MAINVIEW_PANES_COLOR           COLOR(MAINVIEW_PANES_COLOR_INDEX)
#define MAINVIEW_GRAPHICS_COLOR        COLOR(MAINVIEW_GRAPHICS_COLOR_INDEX)
#define OVERLAY_COLOR                  COLOR(OVERLAY_COLOR_INDEX)
#define BARGRAPH1_COLOR                COLOR(BARGRAPH1_COLOR_INDEX)
#define BARGRAPH2_COLOR                COLOR(BARGRAPH2_COLOR_INDEX)
#define BARGRAPH_BGCOLOR               COLOR(BARGRAPH_BGCOLOR_INDEX)
#define CUSTOM_COLOR                   COLOR(CUSTOM_COLOR_INDEX)

// ----------------------------------------------------------------------------

struct ZoneOption
{
	enum Type {
		Integer,
		Source,
		Bool,
		String,
		File,
		TextSize,
		Timer,
		Switch,
		Color
	};
};

// ----------------------------------------------------------------------------

extern "C" const luaR_value_entry lua_opentx_const_lcd[];			// Lua OpenTx Constants for LCD
extern "C" const luaL_Reg lua_opentx_lcdLib[];						// Lua OpenTx LCD functions

extern void registerBitmapClass(lua_State * L);

// ============================================================================

namespace Ui {
	class CLuaLCD;
}

class CLuaLCD : public QWidget
{
	Q_OBJECT

public:
	explicit CLuaLCD(QWidget *parent = nullptr);
	~CLuaLCD();

	virtual QSize sizeHint() const override;

	enum LCD_THEME_ENUM {
		LCD_THEME_DEFAULT,
		LCD_THEME_DARKBLUE,
		LCD_THEME_FLEXI,
	};

	void setTheme(LCD_THEME_ENUM nTheme);
	void setColor(int ndx, const QColor &color);
	QColor color(int ndx);

	void clear(LcdFlags att);
	void drawPoint(coord_t x, coord_t y, LcdFlags att=0);
	void drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint8_t pat=SOLID, LcdFlags att=0);
	void drawText(coord_t x, coord_t y, const char * s, LcdFlags att=0);
	void drawBitmap(coord_t x, coord_t y, const QPixmap &bm, const QRect &src, float scale = 0);
	void drawRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t thickness=1, uint8_t pat=SOLID, LcdFlags att=0);
	void drawFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t pat, LcdFlags att);

	int loadBitmap(const QString &strFilename);
	void freeBitmap(int ndx);
	const QPixmap &bitmap(int ndx);
	bool checkBitmap(int ndx);

protected:
    virtual void paintEvent(QPaintEvent *event) override;

protected:
	uint32_t m_lcdColorTable[LCD_COLOR_COUNT] = {};
	QPixmap m_pixmap;
	QList<QPixmap> m_lstBitmaps;

private:
	Ui::CLuaLCD *ui;

public:
	// Per thread Lua LCD -- one LCD screen on each thread running Lua:
	static thread_local QPointer<CLuaLCD> g_luaLCD;
};

// ============================================================================

#endif	// LUA_LCD_H
