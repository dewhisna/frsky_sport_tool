---- #########################################################################
---- #                                                                       #
---- # Copyright (C) OpenTX                                                  #
-----#                                                                       #
---- # License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html               #
---- #                                                                       #
---- # This program is free software; you can redistribute it and/or modify  #
---- # it under the terms of the GNU General Public License version 2 as     #
---- # published by the Free Software Foundation.                            #
---- #                                                                       #
---- # This program is distributed in the hope that it will be useful        #
---- # but WITHOUT ANY WARRANTY; without even the implied warranty of        #
---- # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
---- # GNU General Public License for more details.                          #
---- #                                                                       #
---- #########################################################################

local isHorus = (LCD_W == 480)
local isX9 = (LCD_W == 212)
local VALUE = 0

local page = 1
local refreshState = 0
local refreshIndex = 0
local calibrationState = 0
local calibrationStep = 0
local configEnabled = false
local configEnableSent = false
local pages = {}
local fields = {}
local modifications = {}

local calibrationPositions = { "up", "down", "left", "right", "forward", "back" }
-- only for x7
local positionConfirmed = 0
local orientationAutoSense = 0

-- -- only for horus
local calibBitmaps = {}
local calibBitmapsFile = {"/SCRIPTS/TOOLS/img/up.png", "/SCRIPTS/TOOLS/img/down.png", "/SCRIPTS/TOOLS/img/left.png", "/SCRIPTS/TOOLS/img/right.png", "/SCRIPTS/TOOLS/img/forward.png", "/SCRIPTS/TOOLS/img/back.png"}



local calibrationFields = {
  {"X:", VALUE, 0x9E, 0, -100, 100, "%"},
  {"Y:", VALUE, 0x9F, 0, -100, 100, "%"},
  {"Z:", VALUE, 0xA0, 0, -100, 100, "%"}
}

-- common
-- Select the next or previous page
local function selectPage(step)
  page = 1 + ((page + step - 1 + #pages) % #pages)
  refreshIndex = 0
  calibrationStep = 0
  pageOffset = 0
end


local function telemetryRead(field)
  return sportTelemetryPush(0x1A, 0x30, 0x0C30, field)
end

local function telemetryWrite(field, value)
  return sportTelemetryPush(0x1A, 0x31, 0x0C30, field + value*256)
end

local function enableConfig()
  return sportTelemetryPush(0x1A, 0x21, 0x0C30, 0)
end

local function disableConfig()
  return sportTelemetryPush(0x1A, 0x20, 0x0C30, 0)
end

local telemetryPopTimeout = 0
local function resetTimeout(nTime)    -- nTime in 10msec increments (as that's the unit for getTime)
  telemetryPopTimeout = getTime() + nTime
end

local function refreshNext()
  if (configEnabled == false) and (calibrationState == 0) and (calibrationStep < 6) then
    if (configEnableSent == false) then
      if (enableConfig() == true) then
        configEnableSent = true
        -- Note: we aren't waiting for a response, but we need a delay to get the enable sent:
        resetTimeout(25)
      end
    elseif getTime() > telemetryPopTimeout then
      configEnabled = true
      resetTimeout(80)
    end
  else
    if refreshState == 0 then
      if calibrationState == 1 then
        if telemetryWrite(0x9D, calibrationStep) == true then
          refreshState = 1
          calibrationState = 2
          resetTimeout(80)
        end
      elseif #modifications > 0 then
        -- telemetryWrite(modifications[1][1], modifications[1][2])
        -- modifications[1] = nil
      elseif refreshIndex < #fields then
        local field = fields[refreshIndex + 1]
        if telemetryRead(field[3]) == true then
          refreshState = 1
          resetTimeout(80)
        end
      end
    elseif refreshState == 1 then
      local physicalId, primId, dataId, value = sportTelemetryPop()
      if physicalId == 0x1A and primId == 0x32 and dataId == 0x0C30 then
        local fieldId = value % 256
        if calibrationState == 2 then
          if fieldId == 0x9D then
            calibrationState = 3
            disableConfig()
            resetTimeout(25)
            -- Short timeout to enter "disable" mode.  Let timeout handler restart us by taking us to calibrationState 0
          end
        elseif calibrationState == 3 then
          calibrationStep = (calibrationStep + 1) % 7
          calibrationState = 0
        else
          local field = fields[refreshIndex + 1]
          if fieldId == field[3] then
            local value = math.floor(value / 256)
            value =  bit32.band(value, 0xffff)        -- kept for Lua 5.2 compat, update to:  value =  (value & 0xffff)
            if field[3] >= 0x9E and field[3] <= 0xA0 then
              local b1 = value % 256
              local b2 = math.floor(value / 256)
              value = b1*256 + b2
              value = value - bit32.band(value, 0x8000) * 2      -- kept for Lua 5.2 compat, update to:  value = value - (value & 0x8000) * 2
            end
            if field[2] == VALUE and #field == 8 then
              value = value - field[8] + field[5]
            end
            fields[refreshIndex + 1][4] = value
            refreshIndex = refreshIndex + 1
            refreshState = 0
          end
        end
      elseif getTime() > telemetryPopTimeout then
        refreshState = 0
        if (calibrationState < 3) then
          calibrationState = 0
        end
        configEnabled = false
        configEnableSent = false
      end
    end
  end
end

-- horus

local function drawScreenTitle(title,page, pages)
  lcd.drawFilledRectangle(0, 0, LCD_W, 30, TITLE_BGCOLOR)
  lcd.drawText(1, 5, title, MENU_TITLE_COLOR)
  lcd.drawText(LCD_W-40, 5, page.."/"..pages, MENU_TITLE_COLOR)
end

local function drawProgressBar()
  if (isHorus) then -- horus
    local width = (300 * refreshIndex) / #fields
    lcd.drawRectangle(100, 10, 300, 6)
    lcd.drawFilledRectangle(102, 13, width, 2);
  else                   -- taranis-x9/x7
    local width = (140 * refreshIndex) / #fields
    lcd.drawRectangle(30, 1, 144, 6)
    lcd.drawFilledRectangle(32, 3, width, 2);
  end
end

local function runCalibrationPageForHorus(event)
  local attr = calibrationState == 0 and INVERS or 0
  fields = calibrationFields
  if refreshIndex == #fields then
    refreshIndex = 0
  end
  lcd.clear()
  drawScreenTitle("SxR", page, #pages)
  if(calibrationStep < 6) then
    local position = calibrationPositions[1 + calibrationStep]
    lcd.drawText(100, 50, "Place the SxR/R9S in the following position", TEXT_COLOR)
    if calibBitmaps[calibrationStep + 1] == nil then
      calibBitmaps[calibrationStep + 1] = Bitmap.open(calibBitmapsFile[calibrationStep + 1])
    end
    lcd.drawBitmap(calibBitmaps[calibrationStep + 1], 200, 70)
    for index = 1, 3, 1 do
      local field = fields[index]
      lcd.drawText(70, 80+20*index, field[1]..":", TEXT_COLOR)
      lcd.drawNumber(90, 80+20*index, math.floor(field[4]/10), LEFT+PREC2)
    end

    if calibrationState == 0 then
      lcd.drawText(160, 220, "Press [Enter] when ready", attr)
    else
      lcd.drawText(193, 220, "Please Wait...", 0)
    end
  else
    lcd.drawText(160, 50, "Calibration completed", 0)
    lcd.drawBitmap(Bitmap.open("/SCRIPTS/TOOLS/img/done.bmp"),200, 100)
    lcd.drawText(160, 220, "Press [RTN] when ready", attr)
  end
  if calibrationStep >= 6 and (event == EVT_VIRTUAL_ENTER or event == EVT_VIRTUAL_EXIT) then
    return 2
  elseif event == EVT_VIRTUAL_ENTER then
    calibrationState = 1
  elseif event == EVT_VIRTUAL_EXIT then
    if calibrationStep > 0 then
      calibrationStep = 0
    end
  end
  return 0
end

-- only for taranis x9/x7
-- Draw initial warning page
local function runWarningPage(event)
  lcd.clear()
  lcd.drawScreenTitle("SxR Calibration", page, #pages)
  lcd.drawText(0, 10, "You only need to calibrate", SMLSIZE)
  lcd.drawText(0, 20, "once. You will need the SxR,", SMLSIZE)
  lcd.drawText(0, 30, "power, and a level surface.", SMLSIZE)
  lcd.drawText(0, 40, "Press [Enter] when ready", SMLSIZE)
  lcd.drawText(0, 50, "Press [Exit] to cancel", SMLSIZE)
  if event == EVT_VIRTUAL_ENTER then
    selectPage(1)
    return 0
  elseif event == EVT_VIRTUAL_EXIT then
    return 2
  end
  return 0
end

-- taranis x9
local calibrationPositionsBitmaps = { "/SCRIPTS/TOOLS/bmp/up.bmp", "/SCRIPTS/TOOLS/bmp/down.bmp", "/SCRIPTS/TOOLS/bmp/left.bmp", "/SCRIPTS/TOOLS/bmp/right.bmp", "/SCRIPTS/TOOLS/bmp/forward.bmp", "/SCRIPTS/TOOLS/bmp/back.bmp"  }
local function runCalibrationPageForX9(event)
  local attr = calibrationState == 0 and INVERS or 0
  fields = calibrationFields
  if refreshIndex == #fields then
    refreshIndex = 0
  end
  lcd.clear()
  lcd.drawScreenTitle("SxR", page, #pages)
  if(calibrationStep < 6) then
    lcd.drawText(0, 9, "Turn the SxR/R9S as shown", 0)
    lcd.drawPixmap(10, 19, calibrationPositionsBitmaps[1 + calibrationStep])
    for index = 1, 3, 1 do
      local field = fields[index]
      lcd.drawText(80, 12+10*index, field[1], 0)
      lcd.drawNumber(90, 12+10*index, math.floor(field[4]/10), LEFT+PREC2)
    end

    if calibrationState == 0 then
      lcd.drawText(0, 56, "Press [Enter] when ready", attr)
    else
      lcd.drawText(0, 56, "Please Wait...", 0)
    end
  else
    lcd.drawText(0, 9, "Calibration completed", 0)
    lcd.drawPixmap(10, 19, "/SCRIPTS/TOOLS/bmp/done.bmp")
    lcd.drawText(0, 56, "Press [Exit] when ready", attr)
  end
  if calibrationStep >= 6 and (event == EVT_VIRTUAL_ENTER or event == EVT_VIRTUAL_EXIT) then
    return 2
  elseif event == EVT_VIRTUAL_ENTER then
    calibrationState = 1
  elseif event == EVT_VIRTUAL_EXIT then
    if calibrationStep > 0 then
      calibrationStep = 0
    end
  end
  return 0
end

-- taranis x7
local function drawCalibrationOrientation(x, y, step)
  local orientation = { {"Label up.", "", 0, 0, 1000, 0, 0, 1000},
                          {"Label down.", "", 0, 0, -1000, 0, 0, -1000},
                          {"Pins Up.", "", -1000, 0, 0, 1000, 0, 0},
                          {"Pins Down.", "", 1000, 0, 0, -1000, 0, 0},
                          {"Label facing you", "Pins Right", 0, 1000, 0, 0, -1000, 0},
                          {"Label facing you", "Pins Left", 0, -1000 , 0, 0, 1000, 0} }

  lcd.drawText(0, 9, "Place the SxR/R9S as follows:", 0)
  lcd.drawText(x-9, y, orientation[step][1])
  lcd.drawText(x-9, y+10, orientation[step][2])
  local positionStatus = 0
  for index = 1, 3, 1 do
    local field = fields[index]
    lcd.drawText(90, 12+10*index, field[1], 0)
    if math.abs(field[4] - orientation[step][2+index+orientationAutoSense]) < 200 then
      lcd.drawNumber(100, 12+10*index, math.floor(field[4]/10), LEFT+PREC2)
      positionStatus = positionStatus + 1
    else
      lcd.drawNumber(100, 12+10*index, math.floor(field[4]/10), LEFT+PREC2+INVERS)
    end
  end
  if step == 3 and positionStatus == 2 then -- orientation auto sensing
    orientationAutoSense = 3 - orientationAutoSense
  end
  if positionStatus == 3 then
    if calibrationState == 0 then
      lcd.drawText(0, 56, " [Enter] to validate   ", INVERS)
      positionConfirmed = 1
    else
      lcd.drawText(0, 56, "    Please Wait...     ", 0)
    end
  end
end

local function runCalibrationPageForX7(event)
  local attr = calibrationState == 0 and INVERS or 0
  fields = calibrationFields
  if refreshIndex == #fields then
    refreshIndex = 0
  end
  lcd.clear()
  lcd.drawScreenTitle("SxR Calibration", page, #pages)
  if(calibrationStep < 6) then
    drawCalibrationOrientation(10, 24, 1 + calibrationStep)

    --lcd.drawText(0, 56, "[Enter] to validate", attr)
  else
    lcd.drawText(0, 19, "Calibration completed", 0)
--    lcd.drawText(10, 19, "Done",0)
    lcd.drawText(0, 56, "Press [Exit] when ready", attr)
  end
  if calibrationStep >= 6 and (event == EVT_VIRTUAL_ENTER or event == EVT_VIRTUAL_EXIT) then
    return 2
  elseif event == EVT_VIRTUAL_ENTER and positionConfirmed  then
    calibrationState = 1
    positionConfirmed = 0
  end
  return 0
end

-- Init
local function init()
  current, edit, refreshState, refreshIndex = 1, false, 0, 0
  if (isHorus) then
    pages = {
    runCalibrationPageForHorus
    }
  elseif (isX9) then
    pages = {
    runWarningPage,
    runCalibrationPageForX9
    }
  else
    pages = {
      runWarningPage,
      runCalibrationPageForX7
    }
  end
end

-- Main
local function run(event)
  if event == nil then
    error("Cannot be run as a model script!")
    return 2
  elseif event == EVT_VIRTUAL_NEXT_PAGE then
    selectPage(1)
  elseif event == EVT_VIRTUAL_PREV_PAGE then
    killEvents(event);
    selectPage(-1)
  end

  local result = pages[page](event)
  refreshNext()

  return result
end

return { init=init, run=run }
