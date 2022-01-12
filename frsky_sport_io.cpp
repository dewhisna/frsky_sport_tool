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

#include "frsky_sport_io.h"

#include "crc.h"

// ============================================================================

namespace {
	struct TDataIDNames {
		uint16_t m_nFirstID;
		uint16_t m_nLastID;
		QString m_strName;
	};
	static const TDataIDNames conarrDataIDNames[] =
	{
		{ DATA_ID_ALT_FIRST, DATA_ID_ALT_LAST, "ALT" },
		{ DATA_ID_VARIO_FIRST, DATA_ID_VARIO_LAST, "VARIO" },
		{ DATA_ID_CURR_FIRST, DATA_ID_CURR_LAST, "CURR" },
		{ DATA_ID_VFAS_FIRST, DATA_ID_VFAS_LAST, "VFAS" },
		{ DATA_ID_CELLS_FIRST, DATA_ID_CELLS_LAST, "LVSS" },
		{ DATA_ID_T1_FIRST, DATA_ID_T1_LAST, "T1" },
		{ DATA_ID_T2_FIRST, DATA_ID_T2_LAST, "T2" },
		{ DATA_ID_RPM_FIRST, DATA_ID_RPM_LAST, "RPM" },
		{ DATA_ID_FUEL_FIRST, DATA_ID_FUEL_LAST, "FUEL" },
		{ DATA_ID_ACCX_FIRST, DATA_ID_ACCX_LAST, "ACCX" },
		{ DATA_ID_ACCY_FIRST, DATA_ID_ACCY_LAST, "ACCY" },
		{ DATA_ID_ACCZ_FIRST, DATA_ID_ACCZ_LAST, "ACCZ" },
		{ DATA_ID_GPS_LONG_LATI_FIRST, DATA_ID_GPS_LONG_LATI_LAST, "GPS LAT/LNG" },
		{ DATA_ID_GPS_ALT_FIRST, DATA_ID_GPS_ALT_LAST, "GPS ALT" },
		{ DATA_ID_GPS_SPEED_FIRST, DATA_ID_GPS_SPEED_LAST, "GPS SPEED" },
		{ DATA_ID_GPS_COURS_FIRST, DATA_ID_GPS_COURS_LAST, "GPS COURS" },
		{ DATA_ID_GPS_TIME_DATE_FIRST, DATA_ID_GPS_TIME_DATE_LAST, "GPS TIME/DATE" },
		{ DATA_ID_A3_FIRST, DATA_ID_A3_LAST, "A3" },
		{ DATA_ID_A4_FIRST, DATA_ID_A4_LAST, "A4" },
		{ DATA_ID_AIR_SPEED_FIRST, DATA_ID_AIR_SPEED_LAST, "AIR SPEED" },
		{ DATA_ID_FUEL_QTY_FIRST, DATA_ID_FUEL_QTY_LAST, "FUEL QTY" },
		{ DATA_ID_RBOX_BATT1_FIRST, DATA_ID_RBOX_BATT1_LAST, "RBOX BATT1" },
		{ DATA_ID_RBOX_BATT2_FIRST, DATA_ID_RBOX_BATT2_LAST, "RBOX BATT2" },
		{ DATA_ID_RBOX_STATE_FIRST, DATA_ID_RBOX_STATE_LAST, "RBOX STATE" },
		{ DATA_ID_RBOX_CNSP_FIRST, DATA_ID_RBOX_CNSP_LAST, "RBOX CNSP" },
		{ DATA_ID_SD1_FIRST, DATA_ID_SD1_LAST, "SD1" },
		{ DATA_ID_ESC_POWER_FIRST, DATA_ID_ESC_POWER_LAST, "ESC PWR" },
		{ DATA_ID_ESC_RPM_CONS_FIRST, DATA_ID_ESC_RPM_CONS_LAST, "ESC RPM" },
		{ DATA_ID_ESC_TEMPERATURE_FIRST, DATA_ID_ESC_TEMPERATURE_LAST, "ESC TEMP" },
		{ DATA_ID_RB3040_OUTPUT_FIRST, DATA_ID_RB3040_OUTPUT_LAST, "RB3040 OUT" },
		{ DATA_ID_RB3040_CH1_2_FIRST, DATA_ID_RB3040_CH1_2_LAST, "RB3040 CH1/2" },
		{ DATA_ID_RB3040_CH3_4_FIRST, DATA_ID_RB3040_CH3_4_LAST, "RB3040 CH3/4" },
		{ DATA_ID_RB3040_CH5_6_FIRST, DATA_ID_RB3040_CH5_6_LAST, "RB3040 CH5/6" },
		{ DATA_ID_RB3040_CH7_8_FIRST, DATA_ID_RB3040_CH7_8_LAST, "RB3040 CH7/8" },
		{ DATA_ID_X8R_FIRST, DATA_ID_X8R_LAST, "X8R" },
		{ DATA_ID_SxR_FIRST, DATA_ID_SxR_LAST, "SxR" },
		{ DATA_ID_GASSUITE_TEMP1_FIRST, DATA_ID_GASSUITE_TEMP1_LAST, "GASSUITE TEMP1" },
		{ DATA_ID_GASSUITE_TEMP2_FIRST, DATA_ID_GASSUITE_TEMP2_LAST, "GASSUITE TEMP2" },
		{ DATA_ID_GASSUITE_SPEED_FIRST, DATA_ID_GASSUITE_SPEED_LAST, "GASSUITE SPEED" },
		{ DATA_ID_GASSUITE_RES_VOL_FIRST, DATA_ID_GASSUITE_RES_VOL_LAST, "GASSUITE RES VOL" },
		{ DATA_ID_GASSUITE_RES_PERC_FIRST, DATA_ID_GASSUITE_RES_PERC_LAST, "GASSUITE RES PERC" },
		{ DATA_ID_GASSUITE_FLOW_FIRST, DATA_ID_GASSUITE_FLOW_LAST, "GASSUITE FLOW" },
		{ DATA_ID_GASSUITE_MAX_FLOW_FIRST, DATA_ID_GASSUITE_MAX_FLOW_LAST, "GASSUITE MAX FLOW" },
		{ DATA_ID_GASSUITE_AVG_FLOW_FIRST, DATA_ID_GASSUITE_AVG_FLOW_LAST, "GASSUITE AVG FLOW" },
		{ DATA_ID_SBEC_POWER_FIRST, DATA_ID_SBEC_POWER_LAST, "SBEC POWER" },
		{ DATA_ID_DIY_STREAM_FIRST, DATA_ID_DIY_STREAM_LAST, "DIY STREAM" },
		{ DATA_ID_DIY_FIRST, DATA_ID_DIY_LAST, "DIY" },
		{ DATA_ID_SERVO_FIRST, DATA_ID_SERVO_LAST, "SERVO" },
		{ DATA_ID_FACT_TEST, DATA_ID_FACT_TEST, "FACT TEST" },
		{ DATA_ID_VALID_FRAME_RATE, DATA_ID_VALID_FRAME_RATE, "VALID FRAME RATE" },
		{ DATA_ID_RSSI, DATA_ID_RSSI, "RSSI" },
		{ DATA_ID_ADC1, DATA_ID_ADC1, "ADC1" },
		{ DATA_ID_ADC2, DATA_ID_ADC2, "ADC2" },
		{ DATA_ID_BATT, DATA_ID_BATT, "BATT" },
		{ DATA_ID_RAS, DATA_ID_RAS, "RAS" },
		{ DATA_ID_XJT_VERSION, DATA_ID_XJT_VERSION, "XJT VERSION" },
		{ DATA_ID_R9_PWR, DATA_ID_R9_PWR, "R9 PWR" },
		{ DATA_ID_SP2UART_A, DATA_ID_SP2UART_A, "SP2UARTA" },
		{ DATA_ID_SP2UART_B, DATA_ID_SP2UART_B, "SP2UARTB" },
		{ 0, 0, QString() },
	};

	// ------------------------------------------------------------------------

};

// ============================================================================

static constexpr uint8_t BIT(uint8_t x, int index) { return (((x) >> index) & 0x01); }
uint8_t physicalIdWithCRC(uint8_t physicalId)
{
	uint8_t result = (physicalId & 0x1F);
	result += (BIT(physicalId, 0) ^ BIT(physicalId, 1) ^ BIT(physicalId, 2)) << 5;
	result += (BIT(physicalId, 2) ^ BIT(physicalId, 3) ^ BIT(physicalId, 4)) << 6;
	result += (BIT(physicalId, 0) ^ BIT(physicalId, 2) ^ BIT(physicalId, 4)) << 7;
	return result;
}

uint8_t CSportTelemetryPacket::crc() const
{
	uint16_t crc = 0;
	for (size_t i=1; i<sizeof(m_raw); ++i) {	// no CRC on 1st byte (physicalId), it has its own CRC
		uint8_t byte = m_raw[i];
		crc += byte; // 0-1FE
		crc += crc >> 8; // 0-1FF
		crc &= 0x00ff;	// 0-FF
	}
	return (0xFF - crc);
}

QString CSportTelemetryPacket::logDetails() const
{
	QString strMsg;
	if (getDataId() != 0) {
		const TDataIDNames *pDataIDName = conarrDataIDNames;
		while (pDataIDName->m_nFirstID != 0) {
			if ((getDataId() >= pDataIDName->m_nFirstID) &&
				(getDataId() <= pDataIDName->m_nLastID)) {
				strMsg += pDataIDName->m_strName;
				if (getPhysicalId() < TELEMETRY_PHYS_ID_COUNT) {
					if (pDataIDName->m_nFirstID != pDataIDName->m_nLastID) {
						strMsg += QString("(%1:%2)").arg(getPhysicalId()).arg(getDataId()-pDataIDName->m_nFirstID);
					} else {
						strMsg += QString("(%1)").arg(getPhysicalId());
					}
				} else {
					if (pDataIDName->m_nFirstID != pDataIDName->m_nLastID) {
						strMsg += QString("(:%1)").arg(getDataId()-pDataIDName->m_nFirstID);
					}
				}
				strMsg += " : ";
				break;
			}
			++pDataIDName;
		}
		if (strMsg.isEmpty()) strMsg += "???Unknown Data ID : ";
	} else {
		if (getPhysicalId() < TELEMETRY_PHYS_ID_COUNT) {
			strMsg += QString("(%1) : ").arg(getPhysicalId());
		}
	}

	switch (getPrimId()) {
		case PRIM_ID_DEVICE_PRESENT_FRAME:
			if (getPhysicalId() < TELEMETRY_PHYS_ID_COUNT) {
				strMsg += QObject::tr("Device Present", "CSportTelemetryPacket");
			}
			if (getValue() != 0) {
				strMsg += QObject::tr(" (unexpected value of 0x%1 (%2)", "CSportTelemetryPacket").arg(getValue(), 4, 16, QChar('0')).arg(getValue());
			}
			break;
		case PRIM_ID_DATA_FRAME:
			strMsg += QObject::tr("Data: ", "CSportTelemetryPacket");
			break;
		case PRIM_ID_CONFIG_MODE_EXIT_FRAME:
			strMsg += QObject::tr("Exit Cal/Cfg Mode", "CSportTelemetryPacket");
			break;
		case PRIM_ID_CONFIG_MODE_ENTER_FRAME:
			strMsg += QObject::tr("Enter Cal/Cfg Mode", "CSportTelemetryPacket");
			break;
		case PRIM_ID_CLIENT_READ_CAL_FRAME:
			strMsg += QObject::tr("Read Cal", "CSportTelemetryPacket");
			break;
		case PRIM_ID_CLIENT_WRITE_CAL_FRAME:
			strMsg += QObject::tr("Write Cal", "CSportTelemetryPacket");
			break;
		case PRIM_ID_SERVER_RESP_CAL_FRAME:
			strMsg += QObject::tr("Cal Resp", "CSportTelemetryPacket");
			break;
	}

	uint32_t nValue = getValue();
	uint8_t nFieldId = nValue % 256;
	nValue = nValue / 256;

	if ((getPrimId() == PRIM_ID_CLIENT_READ_CAL_FRAME) ||
		(getPrimId() == PRIM_ID_CLIENT_WRITE_CAL_FRAME) ||
		(getPrimId() == PRIM_ID_SERVER_RESP_CAL_FRAME)) {
		// Note: the Frsky Calibration script ANDs the "value" here with 0xFFFF, the Config script does not (but it's always zero anyway)
		strMsg += ", ";
		if ((getDataId() >= DATA_ID_SxR_FIRST) &&
			(getDataId() <= DATA_ID_SxR_LAST)) {
			switch (nFieldId) {
				case 0x80:
					strMsg += "Wing type";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 0:
								strMsg += "Normal";
								break;
							case 1:
								strMsg += "Delta";
								break;
							case 2:
								strMsg += "VTail";
								break;
							default:
								strMsg += "VTail ??? (value out-of-range)";	// FrSky Config Script defaults invalid to last value
								break;
						}
					}
					break;
				case 0x81:
					strMsg += "Mounting type";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 0:
								strMsg += "Horz";
								break;
							case 1:
								strMsg += "Horz rev.";
								break;
							case 2:
								strMsg += "Vert";
								break;
							case 3:
								strMsg += "Vert rev.";
								break;
							default:
								strMsg += "Vert rev. ??? (value out-of-range)";	// FrSky Config Script defaults invalid to last value
								break;
						}
					}
					break;
				case 0x9C:
					strMsg += "SxR functions";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 0:
								strMsg += "Disable";
								break;
							case 1:
								strMsg += "Enable";
								break;
							default:
								strMsg += "Enable ??? (value out-of-range)";	// FrSky Config Script defaults invalid to last value
								break;
						}
					}
					break;
				case 0xAA:
					strMsg += "Quick Mode";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue & 0x0001) {		// Note: FrSky's scripts does a bit-and of the value so only 0 and 1 are considered options (but only on 0xAA, Quick Mode)
							case 0:
								strMsg += "Disable";
								break;
							case 1:
								strMsg += "Enable";
								break;
						}
					}
					break;
				case 0xA8:
					strMsg += "CH5 mode";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 0:
								strMsg += "AIL2";
								break;
							case 1:
								strMsg += "AUX1";
								break;
							default:
								strMsg += "AUX1 ??? (value out-of-range)";	// FrSky Config Script defaults invalid to last value
								break;
						}
					}
					break;
				case 0xA9:
					strMsg += "CH6 mode";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 0:
								strMsg += "ELE2";
								break;
							case 1:
								strMsg += "AUX2";
								break;
							default:
								strMsg += "AUX2 ??? (value out-of-range)";	// FrSky Config Script defaults invalid to last value
								break;
						}
					}
					break;
				case 0x82:
					strMsg += "AIL direction";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 255:
								strMsg += "Normal";
								break;
							case 0:
								strMsg += "Invers";
								break;
							default:
								strMsg += "Normal ??? (value out-of-range)";	// FrSky Config Script defaults invalid to "normal"
								break;
						}
					}
					break;
				case 0x83:
					strMsg += "ELE direction";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 255:
								strMsg += "Normal";
								break;
							case 0:
								strMsg += "Invers";
								break;
							default:
								strMsg += "Normal ??? (value out-of-range)";	// FrSky Config Script defaults invalid to "normal"
								break;
						}
					}
					break;
				case 0x84:
					strMsg += "RUD direction";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 255:
								strMsg += "Normal";
								break;
							case 0:
								strMsg += "Invers";
								break;
							default:
								strMsg += "Normal ??? (value out-of-range)";	// FrSky Config Script defaults invalid to "normal"
								break;
						}
					}
					break;
				case 0x9A:
					strMsg += "AIL2 direction";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 255:
								strMsg += "Normal";
								break;
							case 0:
								strMsg += "Invers";
								break;
							default:
								strMsg += "Normal ??? (value out-of-range)";	// FrSky Config Script defaults invalid to "normal"
								break;
						}
					}
					break;
				case 0x9B:
					strMsg += "ELE2 direction";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 255:
								strMsg += "Normal";
								break;
							case 0:
								strMsg += "Invers";
								break;
							default:
								strMsg += "Normal ??? (value out-of-range)";	// FrSky Config Script defaults invalid to "normal"
								break;
						}
					}
					break;
				case 0x85:
					strMsg += "AIL stab gain";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": " + QString("%1%").arg(nValue);
						if (nValue > 200) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x86:
					strMsg += "ELE stab gain";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": " + QString("%1%").arg(nValue);
						if (nValue > 200) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x87:
					strMsg += "RUD stab gain";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": " + QString("%1%").arg(nValue);
						if (nValue > 200) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x88:
					strMsg += "AIL autolvl gain";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": " + QString("%1%").arg(nValue);
						if (nValue > 200) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x89:
					strMsg += "ELE autolvl gain";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": " + QString("%1%").arg(nValue);
						if (nValue > 200) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x8C:
					strMsg += "ELE hover gain";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": " + QString("%1%").arg(nValue);
						if (nValue > 200) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x8D:
					strMsg += "RUD hover gain";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": " + QString("%1%").arg(nValue);
						if (nValue > 200) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x8E:
					strMsg += "AIL knife gain";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": " + QString("%1%").arg(nValue);
						if (nValue > 200) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x90:
					strMsg += "RUD knife gain";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": " + QString("%1%").arg(nValue);
						if (nValue > 200) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x91:
					strMsg += "AIL autolvl offset";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						int32_t nScaled = nValue;
						nScaled -= 0x6C;
						nScaled -= 20;
						strMsg += ": " + QString("%1%").arg(nScaled);
						if ((nScaled < -20) || (nScaled > 20)) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x92:
					strMsg += "ELE autolvl offset";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						int32_t nScaled = nValue;
						nScaled -= 0x6C;
						nScaled -= 20;
						strMsg += ": " + QString("%1%").arg(nScaled);
						if ((nScaled < -20) || (nScaled > 20)) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x95:
					strMsg += "ELE hover offset";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						int32_t nScaled = nValue;
						nScaled -= 0x6C;
						nScaled -= 20;
						strMsg += ": " + QString("%1%").arg(nScaled);
						if ((nScaled < -20) || (nScaled > 20)) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x96:
					strMsg += "RUD hover offset";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						int32_t nScaled = nValue;
						nScaled -= 0x6C;
						nScaled -= 20;
						strMsg += ": " + QString("%1%").arg(nScaled);
						if ((nScaled < -20) || (nScaled > 20)) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x97:
					strMsg += "AIL knife offset";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						int32_t nScaled = nValue;
						nScaled -= 0x6C;
						nScaled -= 20;
						strMsg += ": " + QString("%1%").arg(nScaled);
						if ((nScaled < -20) || (nScaled > 20)) strMsg += " ??? (value out-of-range)";
					}
					break;
				case 0x99:
					strMsg += "RUD knife offset";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						int32_t nScaled = nValue;
						nScaled -= 0x6C;
						nScaled -= 20;
						strMsg += ": " + QString("%1%").arg(nScaled);
						if ((nScaled < -20) || (nScaled > 20)) strMsg += " ??? (value out-of-range)";
					}
					break;

				case 0x9E:
					strMsg += "IMU X";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						int32_t nScaled = nValue;
						nScaled = (nScaled % 256) * 256 + (nScaled / 256);
						nScaled = nScaled - (nValue & 0x8000) * 2;
						strMsg += ": " + QString("%1").arg(double(nScaled)/1000, 0, 'f', 2);
					}
					break;
				case 0x9F:
					strMsg += "IMU Y";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						int32_t nScaled = nValue;
						nScaled = (nScaled % 256) * 256 + (nScaled / 256);
						nScaled = nScaled - (nValue & 0x8000) * 2;
						strMsg += ": " + QString("%1").arg(double(nScaled)/1000, 0, 'f', 2);
					}
					break;
				case 0xA0:
					strMsg += "IMU Z";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						int32_t nScaled = nValue;
						nScaled = (nScaled % 256) * 256 + (nScaled / 256);
						nScaled = nScaled - (nValue & 0x8000) * 2;
						strMsg += ": " + QString("%1").arg(double(nScaled)/1000, 0, 'f', 2);
					}
					break;
				case 0x9D:
					strMsg += "Cal Orientation";
					if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
						strMsg += ": ";
						switch (nValue) {
							case 0:
								strMsg += "up";
								break;
							case 1:
								strMsg += "down";
								break;
							case 2:
								strMsg += "left";
								break;
							case 3:
								strMsg += "right";
								break;
							case 4:
								strMsg += "forward";
								break;
							case 5:
								strMsg += "back";
								break;
							default:
								strMsg += "??? (unknown orientation)";
								break;
						}
					}
					break;

				default:
					strMsg += QObject::tr("** Unknown SxR Setting (0x%1: 0x%02)", "CSportTelemetryPacket").arg(nFieldId, 2, 16, QChar('0')).arg(nValue, 2, 16, QChar('0'));
					break;
			}
		} else if ((getDataId() >= DATA_ID_CELLS_FIRST) &&
				   (getDataId() <= DATA_ID_CELLS_LAST)) {
		} else if ((getDataId() >= DATA_ID_ALT_FIRST) &&
				   (getDataId() <= DATA_ID_ALT_LAST)) {
		} else if ((getDataId() >= DATA_ID_VARIO_FIRST) &&
				   (getDataId() <= DATA_ID_VARIO_LAST)) {
		} else if ((getDataId() >= DATA_ID_GPS_LONG_LATI_FIRST) &&
				   (getDataId() <= DATA_ID_GPS_LONG_LATI_LAST)) {
		} else if ((getDataId() >= DATA_ID_GPS_ALT_FIRST) &&
				   (getDataId() <= DATA_ID_GPS_ALT_LAST)) {
		} else if ((getDataId() >= DATA_ID_GPS_SPEED_FIRST) &&
				   (getDataId() <= DATA_ID_GPS_SPEED_LAST)) {
		} else if ((getDataId() >= DATA_ID_GPS_COURS_FIRST) &&
				   (getDataId() <= DATA_ID_GPS_COURS_LAST)) {
		} else if ((getDataId() >= DATA_ID_GPS_TIME_DATE_FIRST) &&
				   (getDataId() <= DATA_ID_GPS_TIME_DATE_LAST)) {
		}	// TODO : Add other sensors cal/cfg messages here
		// Common Sensor Messages:
		switch (nFieldId) {
			case 0x00:
				strMsg += "Set Defaults";
				break;
			case 0x01:
				strMsg += "PhysId";
				if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
					strMsg += QString("=%1").arg(nValue);
					if (nValue >= TELEMETRY_PHYS_ID_COUNT) strMsg += " ??? (value out-of-range)";
				}
				break;
			case 0x0C:
				strMsg += "Firmware Version";
				if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
					strMsg += QString("=%1.%2").arg(nValue >> 4).arg(nValue & 0x0F);
				}
				break;
			case 0x0D:
				strMsg += "AppId";
				if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
					strMsg += QString("=%1").arg(nValue);
					if (nValue >= 16) strMsg += " ??? (value out-of-range)";
				}
				break;
			case 0x22:
				strMsg += "DataRate";
				if (getPrimId() != PRIM_ID_CLIENT_READ_CAL_FRAME) {
					strMsg += QString("=%1ms").arg(nValue * 100);
					if (nValue >= 256) strMsg += " ??? (value out-of-range)";
				}
				break;
		}
	} else if (getPrimId() == PRIM_ID_DATA_FRAME) {
		if ((getDataId() >= DATA_ID_CELLS_FIRST) &&
			(getDataId() <= DATA_ID_CELLS_LAST)) {
			uint8_t nNumCells = (nFieldId >> 4) & 0x0F;
			uint8_t nCellIndex = nFieldId & 0x0F;
			strMsg += QString("#Cells=%1").arg(nNumCells);
			if (nCellIndex < nNumCells) {
				strMsg += QString(", Cell#%1=%2v").arg(nCellIndex+1).arg(double(nValue & 0x0FFF)/500, 0, 'f', 3);
				if ((nCellIndex+1) < nNumCells) {
					strMsg += QString(", Cell#%1=%2v").arg(nCellIndex+2).arg(double((nValue >> 12) & 0x0FFF)/500, 0, 'f', 3);
				}
			}
		} else if ((getDataId() >= DATA_ID_VARIO_FIRST) &&
				   (getDataId() <= DATA_ID_VARIO_LAST)) {
			int32_t nSValue = (int32_t)getValue();
			strMsg += QString("%1 (m/s)").arg(double(nSValue)/100, 0, 'f', 2);
		} else if (((getDataId() >= DATA_ID_ALT_FIRST) &&
					(getDataId() <= DATA_ID_ALT_LAST)) ||
				   ((getDataId() >= DATA_ID_GPS_ALT_FIRST) &&
					(getDataId() <= DATA_ID_GPS_ALT_LAST))) {
			int32_t nSValue = (int32_t)getValue();
			strMsg += QString("%1 (m)").arg(double(nSValue)/100, 0, 'f', 2);
		} else if ((getDataId() >= DATA_ID_GPS_SPEED_FIRST) &&
				   (getDataId() <= DATA_ID_GPS_SPEED_LAST)) {
			int32_t nSValue = (int32_t)getValue();
			strMsg += QString("%1 (kts)").arg(double(nSValue)/1000, 0, 'f', 3);
		} else if ((getDataId() >= DATA_ID_GPS_LONG_LATI_FIRST) &&
				   (getDataId() <= DATA_ID_GPS_LONG_LATI_LAST)) {
			int32_t nGPSValue = (getValue() & 0x3FFFFFFF);
			nGPSValue = (nGPSValue * 5) / 3;
			if (getValue() & (1 << 30)) nGPSValue = -nGPSValue;
			if (getValue() & (1 << 31)) {
				strMsg += QString("%1 Long.").arg(double(nGPSValue)/1000000);
			} else {
				strMsg += QString("%1 Lat.").arg(double(nGPSValue)/1000000);
			}
		} else if ((getDataId() >= DATA_ID_GPS_TIME_DATE_FIRST) &&
				   (getDataId() <= DATA_ID_GPS_TIME_DATE_LAST)) {
			if (getValue() & 0x000000FF) {
				static const QString arrMonths[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
				uint16_t nYear = (uint16_t) ((getValue() & 0xff000000) >> 24) + 2000;  // SPORT GPS year is only two digits
				uint8_t nMonth = (uint8_t) ((getValue() & 0x00ff0000) >> 16);
				uint8_t nDay = (uint8_t) ((getValue() & 0x0000ff00) >> 8);
				QString strMonth = ((nMonth >= 1) && (nMonth <= 12)) ? arrMonths[nMonth-1] : QString("%1").arg(nMonth);
				strMsg += QString("Date: %1 %2 %3").arg(nDay).arg(strMonth).arg(nYear);
			} else {
				uint8_t nHour = (uint8_t) ((getValue() & 0xff000000) >> 24);
				uint8_t nMin = (uint8_t) ((getValue() & 0x00ff0000) >> 16);
				uint8_t nSec = (uint8_t) ((getValue() & 0x0000ff00) >> 8);
				strMsg += QString("Time: %1:%2:%3 UTC").arg(nHour, 2, 10, QChar('0')).arg(nMin, 2, 10, QChar('0')).arg(nSec, 2, 10, QChar('0'));
			}
		} else {	// TODO : Add other sensors data messages here
		   strMsg += QObject::tr("0x%1 (%2)", "CSportTelemetryPacket").arg(getValue(), 4, 16, QChar('0')).arg(getValue());
		}
	}

	// TODO : Finish Implementing Telemetry Logging
	return (strMsg.isEmpty() ? QObject::tr("Unknown Telemetry Packet", "CSportTelemetryPacket") : strMsg);
}

// ============================================================================

QString CSportTelemetryPollPacket::logDetails() const
{
	return QObject::tr("Poll for PhysID: %1", "CSportTelemetryPollPacket").arg(getPhysicalId());
}

// ============================================================================

uint8_t CSportFirmwarePacket::crc() const
{
	uint16_t nCRC = crc16(CRC_1021, &m_raw[1], sizeof(m_raw)-1);	// no CRC of 1st byte itself (physicalId), it has its own CRC

	// Note: PHYS_ID_FIRMCMD uses the low-byte of the 16-bit CRC,
	//			PHYS_ID_FIRMRSP uses the high-byte of the 16-bit CRC
	return (m_physicalId == PHYS_ID_FIRMCMD) ? (nCRC & 0xFF) : ((nCRC >> 8) & 0xFF);
}

QString CSportFirmwarePacket::logDetails() const
{
	QString strTemp;

	switch (m_cmd) {
		case PRIM_REQ_FLASHMODE:			// Request to start flash mode
			return QObject::tr("Request Flash Mode", "CSportFirmwarePacket");

		case PRIM_REQ_VERSION:				// Request to send Version Info
			return QObject::tr("Request Version", "CSportFirmwarePacket");

		case PRIM_CMD_UPLOAD:				// Command upload mode ??
			strTemp = QObject::tr("Command Upload??", "CSportFirmwarePacket");
			strTemp += QObject::tr(": ?Addr: 0x%1", "CSportFirmwarePacket").arg(dataValue(), 8, 16, QChar('0'));	// Is this really the address?
			return strTemp;

		case PRIM_CMD_DOWNLOAD:				// Command download mode
			return QObject::tr("Command Download", "CSportFirmwarePacket");

		case PRIM_DATA_WORD:				// Receive Data Word Xfer
			strTemp = QObject::tr("Data Xfer", "CSportFirmwarePacket");
			strTemp += QString(": %1.%2.%3.%4: ndx %5")
					.arg(m_data[0], 2, 16, QChar('0'))
					.arg(m_data[1], 2, 16, QChar('0'))
					.arg(m_data[2], 2, 16, QChar('0'))
					.arg(m_data[3], 2, 16, QChar('0'))
					.arg(m_packet, 2, 16, QChar('0'));
			strTemp += QString(", or ?Addr: 0x%1").arg(dataValue(), 8, 16, QChar('0'));		// Can this really be an address?
			return strTemp;

		case PRIM_DATA_EOF:					// Data End-of-File
			return QObject::tr("Data EOF", "CSportFirmwarePacket");

		// ------------------------

		case PRIM_ACK_FLASHMODE:			// Device ACK Flash Mode and is present
			return QObject::tr("Flash Mode ACK", "CSportFirmwarePacket");

		case PRIM_ACK_VERSION:			// Device ACK Version Request
			strTemp = QObject::tr("Version ACK", "CSportFirmwarePacket");
			strTemp += QObject::tr(": Version=0x%1", "CSportFirmwarePacket").arg(dataValue(), 8, 16, QChar('0'));
			return strTemp;

		case PRIM_REQ_DATA_ADDR:		// Device requests specific file address from firmware image
			strTemp = QObject::tr("Req Data", "CSportFirmwarePacket");
			strTemp += QObject::tr(", Addr=0x%1", "CSportFirmwarePacket").arg(dataValue(), 8, 16, QChar('0'));
			strTemp += QObject::tr(", or ?Data Xfer: %1.%2.%3.%4: ndx %5", "CSportFirmwarePacket")
					.arg(m_data[0], 2, 16, QChar('0'))
					.arg(m_data[1], 2, 16, QChar('0'))
					.arg(m_data[2], 2, 16, QChar('0'))
					.arg(m_data[3], 2, 16, QChar('0'))
					.arg(m_packet, 2, 16, QChar('0'));
			return strTemp;

		case PRIM_END_DOWNLOAD:			// Device reports end-of-download (complete)
			return QObject::tr("End Download", "CSportFirmwarePacket");

		case PRIM_DATA_CRC_ERR:			// Device reports CRC failure
			return QObject::tr("Data Error Response (CRC?)", "CSportFirmwarePacket");

		// ------------------------

		default:
			return QObject::tr("*** Unexpected/Unknown Firmware Packet", "CSportFirmwarePacket");
	}
}

// ============================================================================

void CSportTxBuffer::pushByte(uint8_t byte)
{
	m_data.append(byte);
}

void CSportTxBuffer::pushByteWithByteStuffing(uint8_t byte)
{
	if (byte == 0x7E || byte == 0x7D) {
		pushByte(0x7D);
		pushByte(0x20 ^ byte);
	} else {
		pushByte(byte);
	}
}

// ----------------------------------------------------------------------------

QByteArray CSportRxBuffer::pushByte(uint8_t byte)
{
	QByteArray baExtraneous;

	if (byte == 0x7E) {			// Is this the start frame marker?
		// Note: since 0x7E is escaped and stuffed, there's no need
		//	to verify that we aren't in escapement or that we have data
		//	since we can only ever receive a 0x7E as the start frame byte
		baExtraneous = m_baExtraneous;
		reset();				// If so, reset our buffer for a new frame
		m_bHaveFrameStart = true;
	} else if (m_bHaveFrameStart) {
		m_baRaw.append(byte);
		if (m_size == 0) {
			m_data[m_size++] = byte;		// The physical ID is never byte stuffed
			m_bInEscape = false;			// m_bInEscape should already be false, but just in case
		} else {
			if (m_size < SPORT_BUFFER_SIZE) {
				if (m_bInEscape) {
					m_data[m_size++] = byte ^ 0x20;		// Unescape
					m_bInEscape = false;
				} else {
					if (byte == 0x7D) {					// Is this an escapement stuffing?
						m_bInEscape = true;
					} else {
						m_data[m_size++] = byte;
					}
				}
				if (haveCompletePacket()) {
					// Once we receive a complete packet, exit
					//	the frame to ignore extra bytes (which there
					//	shouldn't be any of) before next frame:
					m_bHaveFrameStart = false;
				}
			}
		}
	} else {
		// Note: ignore bytes we receive before a frame start, but
		//	keep them for logging so we know we had extraneous bytes:
		m_baExtraneous.append(byte);
	}

	return baExtraneous;
}

// ----------------------------------------------------------------------------

QString CSportRxBuffer::logDetails() const
{
	if (isFirmwarePacket()) {
		return firmwarePacket().logDetails();
	} else if (isTelemetryPacket()) {
		return telemetryPacket().logDetails();
	} else if (haveTelemetryPoll()) {
		return telemetryPollPacket().logDetails();
	}

	return QObject::tr("*** Unknown Packet", "CSportRxBuffer");
}

// ============================================================================

CFrskySportIO::CFrskySportIO(SPORT_ID_ENUM nSport, QObject *pParent)
	:	QObject(pParent),
		m_nSportID(nSport)
{
}

CFrskySportIO::~CFrskySportIO()
{
}

bool CFrskySportIO::openPort(const QString &strSerialPort, int nBaudRate, int nDataBits, char chParity, int nStopBits)
{
	closePort();

	QString strPortName = strSerialPort;
	if (strPortName.isEmpty()) strPortName = CPersistentSettings::instance()->getDeviceSerialPort(m_nSportID);

	if (strPortName.isEmpty()) {
		m_strLastError = tr("S.port #%1 device not selected.  Set configuration first!").arg(m_nSportID+1);
		return false;
	}

	m_serialPort.setPortName(strPortName);
	if (nBaudRate == 0) nBaudRate = CPersistentSettings::instance()->getDeviceBaudRate(m_nSportID);
	m_serialPort.setBaudRate(nBaudRate);
	m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
	if (chParity == 0) chParity = CPersistentSettings::instance()->getDeviceParity(m_nSportID);
	switch (chParity) {
		case 'N':
		case 'n':
			m_serialPort.setParity(QSerialPort::NoParity);
			break;
		case 'O':
		case 'o':
			m_serialPort.setParity(QSerialPort::OddParity);
			break;
		case 'E':
		case 'e':
			m_serialPort.setParity(QSerialPort::EvenParity);
			break;
		case 'S':
		case 's':
			m_serialPort.setParity(QSerialPort::SpaceParity);
			break;
		case 'M':
		case 'm':
			m_serialPort.setParity(QSerialPort::MarkParity);
			break;
		default:
			m_strLastError = tr("Invalid Parity Setting");
			return false;
	}
	if (nDataBits == 0) nDataBits = CPersistentSettings::instance()->getDeviceDataBits(m_nSportID);
	if ((nDataBits < 5) || (nDataBits > 8)) {
		m_strLastError = tr("Invalid Data Bits Setting");
		return false;
	}
	m_serialPort.setDataBits(static_cast<QSerialPort::DataBits>(nDataBits));
	if (nStopBits == 0) nStopBits = CPersistentSettings::instance()->getDeviceStopBits(m_nSportID);
	switch (nStopBits) {
		case 1:
			m_serialPort.setStopBits(QSerialPort::OneStop);
			break;
		case 2:
			m_serialPort.setStopBits(QSerialPort::TwoStop);
			break;
#ifdef Q_OS_WINDOWS		// as per Qt docs, this one is only supported on Windows
		case 3:
			m_serialPort.setStopBits(QSerialPort::OneAndHalfStop);
			break;
#endif
		default:
			m_strLastError = tr("Invalid StopBit Setting");
			return false;
	}

	if (!m_serialPort.open(QIODevice::ReadWrite)) {
		m_strLastError = m_serialPort.errorString();
		return false;
	}

	return true;
}

void CFrskySportIO::closePort()
{
	if (!m_serialPort.isOpen()) return;

	m_serialPort.close();
}

// ----------------------------------------------------------------------------

void CFrskySportIO::logMessage(LOG_TYPE nLT, const QByteArray &baMsg, const QString &strExtraMsg)
{
	QString strLogMsg;

	switch (nLT) {
		case LT_RX:
			strLogMsg += "Recv: ";
			break;
		case LT_TX:
			strLogMsg += "Send: ";
			break;
		case LT_TXECHO:
			strLogMsg += "Echo: ";
			break;
		case LT_TXPUSH:
			strLogMsg += "Push: ";
			break;
		case LT_TELEPOLL:
			strLogMsg += "Poll: ";
			break;
	}

	for (int i = 0; i < baMsg.size(); ++i) {
		if (i) {
			if ((nLT == LT_TXPUSH) && (i==2)) {
				strLogMsg += QChar('|');
			} else {
				strLogMsg += QChar('.');
			}
		}
		strLogMsg += QString("%1").arg((uint8_t)(baMsg.at(i)), 2, 16, QChar('0')).toUpper();
	}

	if (!strExtraMsg.isEmpty()) strLogMsg += "  " + strExtraMsg;

	emit writeLogString(m_nSportID, strLogMsg);
}

// ============================================================================

