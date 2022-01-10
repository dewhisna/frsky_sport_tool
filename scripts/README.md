# Frsky Lua Scripts

Description
-----------

This folder contains Frsky Lua Scripts for calibrating and configuring Lua devices.  It currently only has scripts for the SxR/R9S receivers.

Note that there are S.port communications protocol differences when communication is done over a radio telemetry link vs directly on the S.port of the device.  As such, this folder contains both the original "official" Frsky Lua Scripts that you'd run on OpenTx and a modified/improved version designed to run on `frsky_sport_tool` directly on the S.port connection.

File naming is as follows:
- `SxR_R9S_Calibrate_tele.lua` : original unmodified SxR/R9S Frsky Receiver Calibration Lua script [download from https://www.frsky-rc.com/lua-script/](https://www.frsky-rc.com/lua-script/).  Version 200706, Dated 2020-07-06.  Designed to run on an OpenTx radio and communicate over the telemetry link.  While it may run on `frsky_sport_tool`, it won't communicate with the receiver correctly over the S.port link.
- `SxR_R9S_Configure_tele.lua` : original unmodified SxR/R9S Frsky Receiver Configuration Lua script [download from https://www.frsky-rc.com/lua-script/](https://www.frsky-rc.com/lua-script/).  Version 200706, Dated 2020-07-06.  Designed to run on an OpenTx radio and communicate over the telemetry link.  While it may run on `frsky_sport_tool`, it won't communicate with the receiver correctly over the S.port link.
- `SxR_R9S_Calibrate_sport.lua` : Reworked SxR/R9S Frsky Receiver Calibration Lua script. Designed to communicate directly with the receiver on the S.port when running on `frsky_sport_tool`. It's also been fixed to have an improved state-machine and prompt the user to wait while the device actually calibrates itself so that you don't move the receiver and mess up the calibration. I'm not sure what happens if you try to run this one on a radio over the telemetry link, but it probably won't work since the S.port protocol is slightly different. However, it might work on the radio if using a direct S.port link with the receiver.
- `SxR_R9S_Configure_sport.lua` : Reworked SxR/R9S Frsky Receiver Configuration Lua script. Designed to communicate directly with the receiver on the S.port when running on `frsky_sport_tool`. Adds pages for reading/writing the device, displays the number of changes to commit, verifies the data was written correctly by reading it back out (rather than just losing your change), adds retry logic, notifies the user when there's a problem writing parameters, revamps the modification tracking and state-machine logic, doesn't try to make changes on-the-fly, but instead lets the user decide when to commit them. I'm not sure what happens if you try to run this one on a radio over the telemetry link, but it probably won't work since the S.port protocol is slightly different. However, it might work on the radio if using a direct S.port link with the receiver.


License
-------

Frsky Sport Tool

Copyright(c) 2021 Donna Whisnant, a.k.a. Dewtronics.

Contact: <http://www.dewtronics.com/>

GNU General Public License Usage

This content may be used under the terms of the GNU General Public License
version 3.0 as published by the Free Software Foundation and appearing
in the file gpl-3.0.txt included in the packaging of this app. Please
review the following information to ensure the GNU General Public License
version 3.0 requirements will be met:

<http://www.gnu.org/copyleft/gpl.html>


Other Usage:

Alternatively, this repository may be used in accordance with the terms
and conditions contained in a signed written agreement between you and
Dewtronics, a.k.a. Donna Whisnant.

See '[LICENSE](../LICENSE.txt)' for the full content of the license.