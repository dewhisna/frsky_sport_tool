---- #########################################################################
---- #                                                                       #
---- # Copyright (C) OpenTX                                                  #
---- #                                                                       #
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

local version = "v2.01 Reworked for S.port Tool"

local VALUE = 0
local COMBO = 1

local COLUMN_2 = 300

local edit = false
local page = 1
local current = 1
local refreshState = 0
local refreshIndex = 0
local pageOffset = 0
local configEnabled = false
local configEnableSent = false
local bWriteSuccess = true
local readOnly = true
local pages = {}
local fields = {}
local modifications = {}
local wingBitmaps = {}
local mountBitmaps = {}
local margin = 1
local spacing = 8
local numberPerPage = 7
local counter = 0

local configFields = {
    { "Wing type", COMBO, 0x80, nil, { "Normal", "Delta", "VTail" } },
    { "Mounting type", COMBO, 0x81, nil, { "Horz", "Horz rev.", "Vert", "Vert rev." } },
}

local wingBitmapsFile = { "/SCRIPTS/TOOLS/bmp/plane.bmp", "/SCRIPTS/TOOLS/bmp/delta.bmp", "/SCRIPTS/TOOLS/bmp/vtail.bmp" }
local mountBitmapsFile = { "/SCRIPTS/TOOLS/bmp/horz.bmp", "/SCRIPTS/TOOLS/bmp/horz-r.bmp", "/SCRIPTS/TOOLS/bmp/vert.bmp", "/SCRIPTS/TOOLS/bmp/vert-r.bmp" }

local settingsFields = {
    {"SxR functions", COMBO, 0x9C, nil, { "Disable", "Enable" } },
    {"Quick Mode:", COMBO, 0xAA, nil, { "Disable", "Enable" } },
    {"CH5 mode", COMBO, 0xA8, nil, { "AIL2", "AUX1" } },
    {"CH6 mode", COMBO, 0xA9, nil, { "ELE2", "AUX2" } },
    {"AIL direction", COMBO, 0x82, nil, { "Normal", "Invers" }, { 255, 0 } },
    {"ELE direction", COMBO, 0x83, nil, { "Normal", "Invers" }, { 255, 0 } },
    {"RUD direction", COMBO, 0x84, nil, { "Normal", "Invers" }, { 255, 0 } },
    {"AIL2 direction", COMBO, 0x9A, nil, { "Normal", "Invers" }, { 255, 0 } },
    {"ELE2 direction", COMBO, 0x9B, nil, { "Normal", "Invers" }, { 255, 0 } },
    {"AIL stab gain", VALUE, 0x85, nil, 0, 200, "%"},
    {"ELE stab gain", VALUE, 0x86, nil, 0, 200, "%"},
    {"RUD stab gain", VALUE, 0x87, nil, 0, 200, "%"},
    {"AIL autolvl gain", VALUE, 0x88, nil, 0, 200, "%"},
    {"ELE autolvl gain", VALUE, 0x89, nil, 0, 200, "%"},
    {"ELE hover gain", VALUE, 0x8C, nil, 0, 200, "%"},
    {"RUD hover gain", VALUE, 0x8D, nil, 0, 200, "%"},
    {"AIL knife gain", VALUE, 0x8E, nil, 0, 200, "%"},
    {"RUD knife gain", VALUE, 0x90, nil, 0, 200, "%"},
    {"AIL autolvl offset", VALUE, 0x91, nil, -20, 20, "%", 0x6C},
    {"ELE autolvl offset", VALUE, 0x92, nil, -20, 20, "%", 0x6C},
    {"ELE hover offset", VALUE, 0x95, nil, -20, 20, "%", 0x6C},
    {"RUD hover offset", VALUE, 0x96, nil, -20, 20, "%", 0x6C},
    {"AIL knife offset", VALUE, 0x97, nil, -20, 20, "%", 0x6C},
    {"RUD knife offset", VALUE, 0x99, nil, -20, 20, "%", 0x6C},
}

local calibrationFields = {
    { "X:", VALUE, 0x9E, 0, -100, 100, "%" },
    { "Y:", VALUE, 0x9F, 0, -100, 100, "%" },
    { "Z:", VALUE, 0xA0, 0, -100, 100, "%" }
}

local totalFields = #configFields + #settingsFields

local function telemetryRead(field)
    return sportTelemetryPush(0x1A, 0x30, 0x0C30, field)
end

local function telemetryWrite(field, value)
    return sportTelemetryPush(0x1A, 0x31, 0x0C30, field + value * 256)
end

local function enableConfig()
    return sportTelemetryPush(0x1A, 0x21, 0x0C30, 0)
end

local function disableConfig()
    return sportTelemetryPush(0x1A, 0x20, 0x0C30, 0)
end

local function drawScreenTitle(title, page, pages)
    if math.fmod(math.floor(getTime() / 500), 2) == 0 then
        title = version
    end
    if LCD_W == 480 then
        lcd.drawFilledRectangle(0, 0, LCD_W, 30, TITLE_BGCOLOR)
        lcd.drawText(1, 5, title, MENU_TITLE_COLOR)
        lcd.drawText(LCD_W - 40, 5, page .. "/" .. pages, MENU_TITLE_COLOR)
    else
        lcd.drawScreenTitle(title, page, pages)
    end
end

-- Change display attribute to current field
local function addField(step)
    local field = fields[current]
    local min, max
    if field[2] == VALUE then
        min = field[5]
        max = field[6]
    elseif field[2] == COMBO then
        min = 0
        max = #(field[5]) - 1
    end
    if (step < 0 and field[4] > min) or (step > 0 and field[4] < max) then
        field[4] = field[4] + step
    end
end

-- Select the next or previous page
local function selectPage(step)
    page = 1 + ((page + step - 1 + #pages) % #pages)
    pageOffset = 0
    current = 1

    -- Changing pages means we will have to start
    --  reading or writing all over, so reset refresh
    --  logic when switching pages and disable config:
    if (configEnabled or configEnableSent) then
        disableConfig()
    end
    configEnabled = false
    configEnableSent = false
    refreshIndex = 0
    refreshState = 0
    bWriteSuccess = true
end

-- Select the next or previous editable field
local function selectField(step)
    current = 1 + ((current + step - 1 + #fields) % #fields)
    if current > numberPerPage + pageOffset then
        pageOffset = current - numberPerPage
    elseif current <= pageOffset then
        pageOffset = current - 1
    end
end

local function drawProgressBar()
    if LCD_W == 480 then
        local width = math.floor((300 * refreshIndex) / totalFields)
        lcd.drawRectangle(88, 126, 304, 20)
        lcd.drawFilledRectangle(90, 128, width, 16)
    else
        local width = math.floor((60 * refreshIndex) / totalFields)
        lcd.drawRectangle(32, 22, 64, 20)
        lcd.drawFilledRectangle(34, 24, width, 16)
    end
end

-- Redraw the current page
local function redrawFieldPage()
    lcd.clear()
    drawScreenTitle("SxR/R9S", page, #pages)

    for index = 1, numberPerPage, 1 do
        local field = fields[pageOffset + index]
        if field == nil then
            break
        end

        local attr = current == (pageOffset + index) and ((edit == true and BLINK or 0) + INVERS) or 0

        lcd.drawText(1, margin + spacing * index, field[1], attr)

        if field[4] == nil then
            lcd.drawText(LCD_W, margin + spacing * index, "---", RIGHT + attr)
        else
            if field[2] == VALUE then
                lcd.drawNumber(LCD_W, margin + spacing * index, field[4], RIGHT + attr)
            elseif field[2] == COMBO then
                if field[4] >= 0 and field[4] < #(field[5]) then
                    lcd.drawText(LCD_W, margin + spacing * index, field[5][1 + field[4]], RIGHT + attr)
                end
            end
        end
    end
end

local telemetryPopTimeout = 0
local function resetTimeout(nTime)    -- nTime in 10msec increments (as that's the unit for getTime)
    telemetryPopTimeout = getTime() + nTime
end

local nCurModValue = 0
local nWriteRetry = 0
local function refreshNext()
    local rwFields = {}
    local rwFieldOffset = 0
    if (refreshIndex >= 1) and (refreshIndex <= totalFields) then
        if (refreshIndex <= #configFields) then
            rwFields = configFields
            rwFieldOffset = 0
        else
            rwFields = settingsFields
            rwFieldOffset = #configFields
        end
        if (configEnabled == false) and (refreshState == 0) then
            if (configEnableSent == false) then
                if (enableConfig() == true) then
                    configEnableSent = true
                    -- Note: we aren't waiting for a response, but we need a delay to get the enable sent:
                    resetTimeout(50)
                end
            elseif getTime() > telemetryPopTimeout then
                configEnabled = true
                resetTimeout(80)
            end
        else
            local rwField = rwFields[refreshIndex - rwFieldOffset]
            -- refreshState definitions:
            --    0 = Issuing Read Command for value acquisition or Write Command for update
            --    1 = Waiting on value from read acquisition
            --    2 = Waiting on write to complete (since there's no device response on this, we wait one timeout cycle)
            --    3 = Read Verify of Write (like 1, but compares value at the end and reissues write if it doesn't match)
            if (refreshState == 0) then
                if (not readOnly) then
                    -- For write, see if this field needs writing:
                    for index = 1, #modifications, 1 do
                        if (modifications[index][1] == rwField[3]) then
                            nCurModValue = modifications[index][2]
                            telemetryWrite(modifications[index][1], modifications[index][2])
                            refreshState = 2
                            resetTimeout(40)
                            break
                        end
                    end
                    if (refreshState == 0) then
                        -- If this field didn't need writing (i.e. we didn't advance the
                        --  state above), then skip to the next one:
                        refreshIndex = refreshIndex + 1
                        if (refreshIndex > totalFields) then
                            -- When we are done, disable the config mode and
                            --  advance to settings page:
                            selectPage(2)
                        end
                    end
                else
                    -- For read only, read and advance:
                    if (telemetryRead(rwField[3]) == true) then
                        refreshState = 1
                        resetTimeout(80)
                    end
                end
            elseif (refreshState == 1) or (refreshState == 3) then
                local physicalId, primId, dataId, value = sportTelemetryPop()
                if physicalId == 0x1A and primId == 0x32 and dataId == 0x0C30 then
                    local fieldId = value % 256
                    if (fieldId == rwField[3]) then
                        value = math.floor(value / 256)
                        if (rwField[3] == 0xAA) then
                            value = bit32.band(value, 0x0001)           -- kept for Lua 5.2 compat, update to:  value = value & 0x0001
                        end
                        if (refreshState == 3) then
                            -- Verify written data
                            if (nCurModValue == value) then
                                -- If it's good, remove it from the list and move on to next value:
                                for index = 1, #modifications, 1 do
                                    if (modifications[index][1] == rwField[3]) then
                                        table.remove(modifications, index)
                                        break
                                    end
                                end
                                refreshIndex = refreshIndex + 1
                                if (refreshIndex > totalFields) then
                                    -- When we are done, disable the config mode and
                                    --  advance to settings page:
                                    selectPage(2)
                                end
                                nWriteRetry = 3
                            else
                                -- If bad, try retry until we run out of retries.
                                --  Loop back to refreshState 0 so that we will
                                --  resend it
                                nWriteRetry = nWriteRetry - 1
                                if (nWriteRetry == 0) then
                                    bWriteSuccess = false
                                    refreshIndex = 0
                                end
                            end
                            refreshState = 0
                        elseif (refreshState == 1) then
                            if rwField[3] >= 0x9E and rwField[3] <= 0xA0 then
                                local b1 = value % 256
                                local b2 = math.floor(value / 256)
                                value = b1 * 256 + b2
                                value = value - bit32.band(value, 0x8000) * 2     -- kept for Lua 5.2 compat, update to:  value = value - (value & 0x8000) * 2
                            end
                            if rwField[2] == COMBO and #rwField == 6 then
                                for index = 1, #(rwField[6]), 1 do
                                    if value == rwField[6][index] then
                                        value = index - 1
                                        break
                                    else
                                        value = 0
                                    end
                                end
                            elseif rwField[2] == COMBO and #rwField == 5 then
                                if value >= #rwField[5] then
                                    value = #rwField[5] - 1
                                end
                            elseif rwField[2] == VALUE and #rwField == 8 then
                                value = value - rwField[8] + rwField[5]
                            end

                            -- Save value and advance to next field:
                            rwFields[refreshIndex - rwFieldOffset][4] = value
                            refreshIndex = refreshIndex + 1
                            refreshState = 0
                            if (refreshIndex > totalFields) then
                                -- When we are done reading, disable the config mode and
                                --  advance to settings page:
                                selectPage(1)
                            end
                        end
                    end
                elseif getTime() > telemetryPopTimeout then
                    if (refreshState == 1) then
                        -- Timeout on read, clear data and retry:
                        rwFields[refreshIndex - rwFieldOffset][4] = nil
                    elseif (refreshState == 3) then
                        -- Timeout on verify, try a write retry or fail completely:
                        nWriteRetry = nWriteRetry - 1
                        if (nWriteRetry == 0) then
                            bWriteSuccess = false
                            refreshIndex = 0
                        end
                    end
                    refreshState = 0
                end
            elseif (refreshState == 2) then
                if getTime() > telemetryPopTimeout then
                    -- After the timeout delay for the write operation, issue a
                    --   read for the data:
                    if (telemetryRead(rwField[3]) == true) then
                        refreshState = 3
                        resetTimeout(80)
                    end
                end
            end    -- end of refreshState
        end    -- end of configEnabled
    end    -- end of doing readIndexes
end

local function updateField(field)
    local value = field[4]
    local bWritten = false
    if field[2] == COMBO and #field == 6 then
        value = field[6][1 + value]
    elseif field[2] == VALUE and #field == 8 then
        value = value + field[8] - field[5]
    end
    for index = 1, #modifications, 1 do
        if (modifications[index][1] == field[3]) then
            modifications[index][2] = value
            bWritten = true
            break
        end
    end
    if (not bWritten) then
        modifications[#modifications + 1] = { field[3], value }
    end
end

-- Main
local function runFieldsPage(event)
    if event == EVT_VIRTUAL_EXIT then -- exit script
        return 2
    elseif event == EVT_VIRTUAL_ENTER then -- toggle editing/selecting current field
        if fields[current][4] ~= nil then
            edit = not edit
            if edit == false then
                updateField(fields[current])
            end
        end
    elseif edit then
        if event == EVT_VIRTUAL_INC or event == EVT_VIRTUAL_INC_REPT then
            addField(1)
        elseif event == EVT_VIRTUAL_DEC or event == EVT_VIRTUAL_DEC_REPT then
            addField(-1)
        end
    else
        if event == EVT_VIRTUAL_NEXT then
            selectField(1)
        elseif event == EVT_VIRTUAL_PREV then
            selectField(-1)
        end
    end
    redrawFieldPage()
    return 0
end

local function runConfigPage(event)
    fields = configFields
    local result = runFieldsPage(event)
    if LCD_W == 128 then
        local mountText = { "Label is facing the sky", "Label is facing ground", "Label is left when", "Label is right when" }
        if fields[2][4] ~= nil then
            lcd.drawText(1, 30, "Pins toward tail")
            lcd.drawText(1, 40, mountText[1 + fields[2][4]])
            if fields[2][4] > 1 then
                lcd.drawText(1, 50, "looking from the tail")
            end
        end
    else
        if fields[1][4] ~= nil then
            if LCD_W == 480 then
                if wingBitmaps[1 + fields[1][4]] == nil then
                    wingBitmaps[1 + fields[1][4]] = Bitmap.open(wingBitmapsFile[1 + fields[1][4]])
                end
                lcd.drawBitmap(wingBitmaps[1 + fields[1][4]], 10, 90)
            else
                lcd.drawPixmap(20, 28, wingBitmapsFile[1 + fields[1][4]])
            end
        end
        if fields[2][4] ~= nil then
            if LCD_W == 480 then
                if mountBitmaps[1 + fields[2][4]] == nil then
                    mountBitmaps[1 + fields[2][4]] = Bitmap.open(mountBitmapsFile[1 + fields[2][4]])
                end
                lcd.drawBitmap(mountBitmaps[1 + fields[2][4]], 190, 110)
            else
                lcd.drawPixmap(128, 28, mountBitmapsFile[1 + fields[2][4]])
            end
        end
    end
    return result
end

local function runSettingsPage(event)
    fields = settingsFields
    return runFieldsPage(event)
end

local function runReadDevicePage(event)
    readOnly = true

    lcd.clear()
    drawScreenTitle("SxR/R9S", page, #pages)

    if (refreshIndex == 0) and (refreshState == 0) then
        if LCD_W == 480 then
            lcd.drawText(160, 126, "Press [Enter] to read settings", 0)
        else
            lcd.drawText(0, 10, "Press [Enter] to read settings", 0)
        end
        if event == EVT_VIRTUAL_ENTER then
            modifications = {}
            refreshIndex = 1
        end
    else
        if LCD_W == 480 then
            lcd.drawText(180, 68, "Reading Device...")
        else
            lcd.drawText(8, 10, "Reading Device...")
        end

        if refreshIndex <= totalFields then
            drawProgressBar()
        end
    end
    if event == EVT_VIRTUAL_EXIT then -- exit script
        return 2
    end

    refreshNext()
    return 0
end

local function runWriteDevicePage(event)
    readOnly = false

    lcd.clear()
    drawScreenTitle("SxR/R9S", page, #pages)

    if (refreshIndex == 0) and (refreshState == 0) then
        if LCD_W == 480 then
            if (bWriteSuccess == true) then
                lcd.drawText(160, 100, #modifications .. " uncommitted change(s)", 0)
                lcd.drawText(160, 126, "Press [Enter] to write settings", 0)
            else
                lcd.drawText(208, 100, "Write failed", 0)
                lcd.drawText(160, 126, "Press [Enter] to retry writing", 0)
            end
        else
            if (bWriteSuccess == true) then
                lcd.drawText(0, 10, #modifications .. " uncommitted change(s)", 0)
                lcd.drawText(0, 20, "Press [Enter] to write settings", 0)
            else
                lcd.drawText(0, 10, "Write failed", 0)
                lcd.drawText(0, 20, "Press [Enter] to retry writing", 0)
            end
        end
        if event == EVT_VIRTUAL_ENTER then
            bWriteSuccess = true
            nWriteRetry = 3
            refreshIndex = 1
        end
    else
        if LCD_W == 480 then
            lcd.drawText(180, 68, "Writing Device...")
        else
            lcd.drawText(8, 10, "Writing Device...")
        end

        if refreshIndex <= totalFields then
            drawProgressBar()
        end
    end
    if event == EVT_VIRTUAL_EXIT then -- exit script
        return 2
    end

    refreshNext()
    return 0
end

-- Init
local function init()
    current, edit, refreshState, refreshIndex = 1, false, 0, 1
    if LCD_W == 480 then
        margin = 10
        spacing = 20
        numberPerPage = 12
        wingBitmapsFile = { "/SCRIPTS/TOOLS/img/plane_b.png", "/SCRIPTS/TOOLS/img/delta_b.png", "/SCRIPTS/TOOLS/img/planev_b.png" }
        mountBitmapsFile = { "/SCRIPTS/TOOLS/img/up.png", "/SCRIPTS/TOOLS/img/down.png", "/SCRIPTS/TOOLS/img/vert.png", "/SCRIPTS/TOOLS/img/vert-r.png" }
    end
    pages = {
        runReadDevicePage,
        runConfigPage,
        runSettingsPage,
        runWriteDevicePage
    }
end

-- Main
local function run(event)
    if event == nil then
        error("Cannot be run as a model script!")
        return 2
    elseif event == EVT_VIRTUAL_NEXT_PAGE then
        killEvents(event)
        selectPage(1)
    elseif event == EVT_VIRTUAL_PREV_PAGE then
        killEvents(event)
        selectPage(-1)
    end

    local result = pages[page](event)

    return result
end

return { init = init, run = run }

