// AWTRIX Controller
// Copyright (C) 2020
// by Blueforcer & Mazze2000

#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
#include <Fonts/TomThumb.h>
#include <LightDependentResistor.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include "SoftwareSerial.h"

#include <WiFiManager.h>
#include <DoubleResetDetect.h>
#include <Wire.h>
#include <BME280_t.h>
#include "Adafruit_HTU21DF.h"
#include <Adafruit_BMP280.h>

#include <DFMiniMp3.h>

#include "MenueControl/MenueControl.h"

// instantiate temp sensor
BME280<> BMESensor;
Adafruit_BMP280 BMPSensor; // use I2C interface
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

enum MsgType
{
	MsgType_Wifi,
	MsgType_Host,
	MsgType_Temp,
	MsgType_Audio,
	MsgType_Gest,
	MsgType_LDR,
	MsgType_Other
};
enum TempSensor
{
	TempSensor_None,
	TempSensor_BME280,
	TempSensor_HTU21D,
	TempSensor_BMP280
}; // None = 0

TempSensor tempState = TempSensor_None;

int ldrState = 1000;		// 0 = None
bool USBConnection = false; // true = usb...
bool WIFIConnection = false;
bool notify=false;
int connectionTimout;
int matrixTempCorrection = 0;

String version = "0.43";
char awtrix_server[16] = "0.0.0.0";
char Port[6] = "7001"; // AWTRIX Host Port, default = 7001
int matrixType = 0;

IPAddress Server;
WiFiClient espClient;
PubSubClient client(espClient);

WiFiManager wifiManager;

MenueControl myMenue;

//update
ESP8266WebServer server(80);
const char *serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

//resetdetector
#define DRD_TIMEOUT 5.0
#define DRD_ADDRESS 0x00
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

bool firstStart = true;
int myTime;	 //need for loop
int myTime2; //need for loop
int myTime3; //need for loop3
int myCounter;
int myCounter2;
//boolean getLength = true;
//int prefix = -5;

bool ignoreServer = false;
int menuePointer;

//Taster_mid
int tasterPin[] = {D0, D4, D8};
int tasterCount = 3;

int timeoutTaster[] = {0, 0, 0, 0};
bool pushed[] = {false, false, false, false};
int blockTimeTaster[] = {0, 0, 0, 0};
bool blockTaster[] = {false, false, false, false};
bool blockTaster2[] = {false, false, false, false};
bool tasterState[3];
bool allowTasterSendToServer = true;
int pressedTaster = 0;

//Reset time (Touch Taster)
int resetTime = 6000; //in milliseconds

boolean awtrixFound = false;
int myPointer[14];
uint32_t messageLength = 0;
uint32_t SavemMessageLength = 0;

//USB Connection:
byte myBytes[1000];
int bufferpointer;

//Zum speichern...
int cfgStart = 0;

//flag for saving data
bool shouldSaveConfig = false;

/// LDR Config
#define LDR_RESISTOR 1000 //ohms
#define LDR_PIN A0
#define LDR_PHOTOCELL LightDependentResistor::GL5516
LightDependentResistor photocell(LDR_PIN, ldrState, LDR_PHOTOCELL);

// Gesture Sensor
#define APDS9960_INT D6
#define I2C_SDA D3
#define I2C_SCL D1
SparkFun_APDS9960 apds = SparkFun_APDS9960();
volatile bool isr_flag = 0;

#ifndef ICACHE_RAM_ATTR
#define ICACHE_RAM_ATTR IRAM_ATTR
#endif

bool updating = false;

// Audio
//DFPlayerMini_Fast myMP3;

// forward declare the notify class, just the name
//
class Mp3Notify; 

// define a handy type using serial and our notify class
//


// instance a DfMp3 object, 
//

SoftwareSerial mySoftwareSerial(D7, D5); // RX, TX
typedef DFMiniMp3<SoftwareSerial, Mp3Notify> DfMp3; 
DfMp3 dfmp3(mySoftwareSerial);

class Mp3Notify
{

};

// Matrix Settings
#define Width 63
#define Heigh 14
#define LED_NUM Width*Heigh
#define MaxBrithness 255
#define MiniBrithness 155
#define COLOR_ORDER RGB

boolean testing = false;

CRGB leds[LED_NUM];
FastLED_NeoMatrix *matrix;

// Params for width and height
const uint8_t kMatrixWidth = Width;
const uint8_t kMatrixHeight = Heigh;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define LAST_VISIBLE_LED 761

uint16_t XY (uint16_t x, uint16_t y) {
  // any out of bounds address maps to the first hidden pixel
  if ( (x >= kMatrixWidth) || (y >= kMatrixHeight) ) {
    return (LAST_VISIBLE_LED + 1);
  }

  const uint16_t XYTable[] = {
     0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
   125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100,  99,  98,  97,  96,  95,  94,  93,  92,  91,  90,  89,  88,  87,  86,  85,  84,  83,  82,  81,  80,  79,  78,  77,  76,  75,  74,  73,  72,  71,  70,  69,  68,  67,  66,  65,  64,  63,
   126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
   227, 226, 225, 224, 223, 222, 221, 220, 219, 785, 784, 783, 782, 781, 780, 779, 778, 777, 776, 775, 774, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 773, 772, 771, 770, 769, 768, 767, 766, 765, 764, 763, 762, 197, 196, 195, 194, 193, 192, 191, 190, 189,
   228, 229, 230, 231, 232, 233, 234, 235, 236, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 258, 259, 260, 261, 262, 263, 264, 265, 266,
   305, 304, 303, 302, 301, 300, 299, 298, 297, 833, 832, 831, 830, 829, 828, 827, 826, 825, 824, 823, 822, 296, 295, 294, 293, 292, 291, 290, 289, 288, 287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 821, 820, 819, 818, 817, 816, 815, 814, 813, 812, 811, 810, 275, 274, 273, 272, 271, 270, 269, 268, 267,
   306, 307, 308, 309, 310, 311, 312, 313, 314, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 336, 337, 338, 339, 340, 341, 342, 343, 344,
   383, 382, 381, 380, 379, 378, 377, 376, 375, 881, 880, 879, 878, 877, 876, 875, 874, 873, 872, 871, 870, 374, 373, 372, 371, 370, 369, 368, 367, 366, 365, 364, 363, 362, 361, 360, 359, 358, 357, 356, 355, 354, 869, 868, 867, 866, 865, 864, 863, 862, 861, 860, 859, 858, 353, 352, 351, 350, 349, 348, 347, 346, 345,
   384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446,
   509, 508, 507, 506, 505, 504, 503, 502, 501, 500, 499, 498, 497, 496, 495, 494, 493, 492, 491, 490, 489, 488, 487, 486, 485, 484, 483, 482, 481, 480, 479, 478, 477, 476, 475, 474, 473, 472, 471, 470, 469, 468, 467, 466, 465, 464, 463, 462, 461, 460, 459, 458, 457, 456, 455, 454, 453, 452, 451, 450, 449, 448, 447,
   510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572,
   635, 634, 633, 632, 631, 630, 629, 628, 627, 626, 625, 624, 623, 622, 621, 620, 619, 618, 617, 616, 615, 614, 613, 612, 611, 610, 609, 608, 607, 606, 605, 604, 603, 602, 601, 600, 599, 598, 597, 596, 595, 594, 593, 592, 591, 590, 589, 588, 587, 586, 585, 584, 583, 582, 581, 580, 579, 578, 577, 576, 575, 574, 573,
   636, 637, 638, 639, 640, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698,
   761, 760, 759, 758, 757, 756, 755, 754, 753, 752, 751, 750, 749, 748, 747, 746, 745, 744, 743, 742, 741, 740, 739, 738, 737, 736, 735, 734, 733, 732, 731, 730, 729, 728, 727, 726, 725, 724, 723, 722, 721, 720, 719, 718, 717, 716, 715, 714, 713, 712, 711, 710, 709, 708, 707, 706, 705, 704, 703, 702, 701, 700, 699
  };

  uint16_t i = (y * kMatrixWidth) + x;
  uint16_t j = XYTable[i];
  return j;
}



static byte c1; // Last character buffer
byte utf8ascii(byte ascii)
{
	if (ascii < 128) // Standard ASCII-set 0..0x7F handling
	{
		c1 = 0;
		return (ascii);
	}
	// get previous input
	byte last = c1; // get last char
	c1 = ascii;		// remember actual character
	switch (last)	// conversion depending on first UTF8-character
	{
	case 0xC2:
		return (ascii)-34;
		break;
	case 0xC3:
		return (ascii | 0xC0) - 34;
		break;
	case 0x82:
		if (ascii == 0xAC)
			return (0xEA);
	}
	return (0);
}

bool saveConfig()
{
	DynamicJsonBuffer jsonBuffer;
	JsonObject &json = jsonBuffer.createObject();
	json["awtrix_server"] = awtrix_server;
	json["matrixType"] = matrixType;
	json["matrixCorrection"] = matrixTempCorrection;
	json["Port"] = Port;

	//json["temp"] = tempState;
	//json["usbWifi"] = USBConnection;
	//json["ldr"] = ldrState;
	//json["gesture"] = gestureState;
	//json["audio"] = audioState;

	File configFile = LittleFS.open("/awtrix.json", "w");
	if (!configFile)
	{
		if (!USBConnection)
		{
			Serial.println("failed to open config file for writing");
		}

		return false;
	}

	json.printTo(configFile);
	configFile.close();
	//end save
	return true;
}

void debuggingWithMatrix(String text)
{
	matrix->setCursor(7, 6);
	matrix->clear();
	matrix->print(text);
	matrix->show();
}

void sendToServer(String s)
{
	if (USBConnection)
	{
		uint32_t laenge = s.length();
		Serial.printf("%c%c%c%c%s", (laenge & 0xFF000000) >> 24, (laenge & 0x00FF0000) >> 16, (laenge & 0x0000FF00) >> 8, (laenge & 0x000000FF), s.c_str());
	}
	else
	{
		client.publish("matrixClient", s.c_str());
	}
}

void logToServer(String s)
{
	StaticJsonBuffer<400> jsonBuffer;
	JsonObject &root = jsonBuffer.createObject();
	root["type"] = "log";
	root["msg"] = s;
	String JS;
	root.printTo(JS);
	sendToServer(JS);
}

int checkTaster(int nr)
{
	tasterState[0] = !digitalRead(tasterPin[0]);
	tasterState[1] = digitalRead(tasterPin[1]);
	tasterState[2] = !digitalRead(tasterPin[2]);

	switch (nr)
	{
	case 0:
		if (tasterState[0] == LOW && !pushed[nr] && !blockTaster2[nr] && tasterState[1] && tasterState[2])
		{
			pushed[nr] = true;
			timeoutTaster[nr] = millis();
		}
		break;
	case 1:
		if (tasterState[1] == LOW && !pushed[nr] && !blockTaster2[nr] && tasterState[0] && tasterState[2])
		{
			pushed[nr] = true;
			timeoutTaster[nr] = millis();
		}
		break;
	case 2:
		if (tasterState[2] == LOW && !pushed[nr] && !blockTaster2[nr] && tasterState[0] && tasterState[1])
		{
			pushed[nr] = true;
			timeoutTaster[nr] = millis();
		}
		break;
	case 3:
		if (tasterState[0] == LOW && tasterState[2] == LOW && !pushed[nr] && !blockTaster2[nr] && tasterState[1])
		{
			pushed[nr] = true;
			timeoutTaster[nr] = millis();
		}
		break;
	}

	if (pushed[nr] && (millis() - timeoutTaster[nr] < 2000) && tasterState[nr] == HIGH)
	{
		if (!blockTaster2[nr])
		{
			StaticJsonBuffer<400> jsonBuffer;
			JsonObject &root = jsonBuffer.createObject();
			root["type"] = "button";

			switch (nr)
			{
			case 0:
				root["left"] = "short";
				pressedTaster = 1;
				//Serial.println("LEFT: normaler Tastendruck");
				break;
			case 1:
				root["middle"] = "short";
				pressedTaster = 2;
				//Serial.println("MID: normaler Tastendruck");
				break;
			case 2:
				root["right"] = "short";
				pressedTaster = 3;
				//Serial.println("RIGHT: normaler Tastendruck");
				break;
			}

			String JS;
			root.printTo(JS);
			if (allowTasterSendToServer)
			{
				sendToServer(JS);
			}
			pushed[nr] = false;
			return 1;
		}
	}

	if (pushed[nr] && (millis() - timeoutTaster[nr] > 2000))
	{
		if (!blockTaster2[nr])
		{
			StaticJsonBuffer<400> jsonBuffer;
			JsonObject &root = jsonBuffer.createObject();
			root["type"] = "button";
			switch (nr)
			{
			case 0:
				root["left"] = "long";
				//Serial.println("LEFT: langer Tastendruck");
				break;
			case 1:
				root["middle"] = "long";
				//Serial.println("MID: langer Tastendruck");
				break;
			case 2:
				root["right"] = "long";
				//Serial.println("RIGHT: langer Tastendruck");
				break;
			case 3:
				if (allowTasterSendToServer)
				{
					allowTasterSendToServer = false;
					ignoreServer = true;
				}
				else
				{
					allowTasterSendToServer = true;
					ignoreServer = false;
					menuePointer = 0;
				}
				break;
			}
			String JS;
			root.printTo(JS);
			if (allowTasterSendToServer)
			{
				sendToServer(JS);
			}

			blockTaster[nr] = true;
			blockTaster2[nr] = true;
			pushed[nr] = false;
			return 2;
		}
	}
	if (nr == 3)
	{
		if (blockTaster[nr] && tasterState[0] == HIGH && tasterState[2] == HIGH)
		{
			blockTaster[nr] = false;
			blockTimeTaster[nr] = millis();
		}
	}
	else
	{
		if (blockTaster[nr] && tasterState[nr] == HIGH)
		{
			blockTaster[nr] = false;
			blockTimeTaster[nr] = millis();
		}
	}

	if (!blockTaster[nr] && (millis() - blockTimeTaster[nr] > 500))
	{
		blockTaster2[nr] = false;
	}
	return 0;
}

String utf8ascii(String s)
{
	String r = "";
	char c;
	for (unsigned int i = 0; i < s.length(); i++)
	{
		c = utf8ascii(s.charAt(i));
		if (c != 0)
			r += c;
	}
	return r;
}

void hardwareAnimatedUncheck(int typ, int x, int y)
{
	int wifiCheckTime = millis();
	int wifiCheckPoints = 0;
	while (millis() - wifiCheckTime < 2000)
	{
		while (wifiCheckPoints < 10)
		{
			matrix->clear();
			switch (typ)
			{
			case 0:
				matrix->setCursor(7, 6);
				matrix->print("WiFi");
				break;
			case 1:
				matrix->setCursor(1, 6);
				matrix->print("Server");
				break;
			case 2:
				matrix->setCursor(7, 6);
				matrix->print("Temp");
				break;
			case 4:
				matrix->setCursor(3, 6);
				matrix->print("Gest.");
				break;
			}

			switch (wifiCheckPoints)
			{
			case 9:
				matrix->drawPixel(x, y + 4, 0xF800);
			case 8:
				matrix->drawPixel(x - 1, y + 3, 0xF800);
			case 7:
				matrix->drawPixel(x - 2, y + 2, 0xF800);
			case 6:
				matrix->drawPixel(x - 3, y + 1, 0xF800);
			case 5:
				matrix->drawPixel(x - 4, y, 0xF800);
			case 4:
				matrix->drawPixel(x - 4, y + 4, 0xF800);
			case 3:
				matrix->drawPixel(x - 3, y + 3, 0xF800);
			case 2:
				matrix->drawPixel(x - 2, y + 2, 0xF800);
			case 1:
				matrix->drawPixel(x - 1, y + 1, 0xF800);
			case 0:
				matrix->drawPixel(x, y, 0xF800);
				break;
			}
			wifiCheckPoints++;
			matrix->show();
			delay(100);
		}
	}
}

void hardwareAnimatedCheck(MsgType typ, int x, int y)
{
	int wifiCheckTime = millis();
	int wifiCheckPoints = 0;
	while (millis() - wifiCheckTime < 2000)
	{
		while (wifiCheckPoints < 7)
		{
			matrix->clear();
			switch (typ)
			{
			case MsgType_Wifi:
				matrix->setCursor(7, 6);
				matrix->print("WiFi");
				break;
			case MsgType_Host:
				matrix->setCursor(5, 6);
				matrix->print("Host");
				break;
			case MsgType_Temp:
				matrix->setCursor(7, 6);
				matrix->print("Temp");
				break;
			case MsgType_Audio:
				matrix->setCursor(3, 6);
				matrix->print("Audio");
				break;
			case MsgType_Gest:
				matrix->setCursor(3, 6);
				matrix->print("Gest.");
				break;
			case MsgType_LDR:
				matrix->setCursor(7, 6);
				matrix->print("LDR");
				break;
			}

			switch (wifiCheckPoints)
			{
			case 6:
				matrix->drawPixel(x, y, 0x07E0);
			case 5:
				matrix->drawPixel(x - 1, y + 1, 0x07E0);
			case 4:
				matrix->drawPixel(x - 2, y + 2, 0x07E0);
			case 3:
				matrix->drawPixel(x - 3, y + 3, 0x07E0);
			case 2:
				matrix->drawPixel(x - 4, y + 4, 0x07E0);
			case 1:
				matrix->drawPixel(x - 5, y + 3, 0x07E0);
			case 0:
				matrix->drawPixel(x - 6, y + 2, 0x07E0);
				break;
			}
			wifiCheckPoints++;
			matrix->show();
			delay(100);
		}
	}
}

void serverSearch(int rounds, int typ, int x, int y)
{
	matrix->clear();
	matrix->setTextColor(0xFFFF);
	matrix->setCursor(5, 6);
	matrix->print("Host");

	if (typ == 0)
	{
		switch (rounds)
		{
		case 3:
			matrix->drawPixel(x, y, 0x22ff);
			matrix->drawPixel(x + 1, y + 1, 0x22ff);
			matrix->drawPixel(x + 2, y + 2, 0x22ff);
			matrix->drawPixel(x + 3, y + 3, 0x22ff);
			matrix->drawPixel(x + 2, y + 4, 0x22ff);
			matrix->drawPixel(x + 1, y + 5, 0x22ff);
			matrix->drawPixel(x, y + 6, 0x22ff);
		case 2:
			matrix->drawPixel(x - 1, y + 2, 0x22ff);
			matrix->drawPixel(x, y + 3, 0x22ff);
			matrix->drawPixel(x - 1, y + 4, 0x22ff);
		case 1:
			matrix->drawPixel(x - 3, y + 3, 0x22ff);
		case 0:
			break;
		}
	}
	else if (typ == 1)
	{

		switch (rounds)
		{
		case 12:
			//matrix->drawPixel(x+3, y+2, 0x22ff);
			matrix->drawPixel(x + 3, y + 3, 0x22ff);
			//matrix->drawPixel(x+3, y+4, 0x22ff);
			matrix->drawPixel(x + 3, y + 5, 0x22ff);
			//matrix->drawPixel(x+3, y+6, 0x22ff);
		case 11:
			matrix->drawPixel(x + 2, y + 2, 0x22ff);
			matrix->drawPixel(x + 2, y + 3, 0x22ff);
			matrix->drawPixel(x + 2, y + 4, 0x22ff);
			matrix->drawPixel(x + 2, y + 5, 0x22ff);
			matrix->drawPixel(x + 2, y + 6, 0x22ff);
		case 10:
			matrix->drawPixel(x + 1, y + 3, 0x22ff);
			matrix->drawPixel(x + 1, y + 4, 0x22ff);
			matrix->drawPixel(x + 1, y + 5, 0x22ff);
		case 9:
			matrix->drawPixel(x, y + 4, 0x22ff);
		case 8:
			matrix->drawPixel(x - 1, y + 4, 0x22ff);
		case 7:
			matrix->drawPixel(x - 2, y + 4, 0x22ff);
		case 6:
			matrix->drawPixel(x - 3, y + 4, 0x22ff);
		case 5:
			matrix->drawPixel(x - 3, y + 5, 0x22ff);
		case 4:
			matrix->drawPixel(x - 3, y + 6, 0x22ff);
		case 3:
			matrix->drawPixel(x - 3, y + 7, 0x22ff);
		case 2:
			matrix->drawPixel(x - 4, y + 7, 0x22ff);
		case 1:
			matrix->drawPixel(x - 5, y + 7, 0x22ff);
		case 0:
			break;
		}
	}
	matrix->show();
}

void hardwareAnimatedSearch(int typ, int x, int y)
{
	for (int i = 0; i < 4; i++)
	{
		matrix->clear();
		matrix->setTextColor(0xFFFF);
		if (typ == 0)
		{
			matrix->setCursor(7, 6);
			matrix->print("WiFi");
		}
		else if (typ == 1)
		{
			matrix->setCursor(5, 6);
			matrix->print("Host");
		}
		switch (i)
		{
		case 3:
			matrix->drawPixel(x, y, 0x22ff);
			matrix->drawPixel(x + 1, y + 1, 0x22ff);
			matrix->drawPixel(x + 2, y + 2, 0x22ff);
			matrix->drawPixel(x + 3, y + 3, 0x22ff);
			matrix->drawPixel(x + 2, y + 4, 0x22ff);
			matrix->drawPixel(x + 1, y + 5, 0x22ff);
			matrix->drawPixel(x, y + 6, 0x22ff);
		case 2:
			matrix->drawPixel(x - 1, y + 2, 0x22ff);
			matrix->drawPixel(x, y + 3, 0x22ff);
			matrix->drawPixel(x - 1, y + 4, 0x22ff);
		case 1:
			matrix->drawPixel(x - 3, y + 3, 0x22ff);
		case 0:
			break;
		}
		matrix->show();
		delay(100);
	}
}

void utf8ascii(char *s)
{
	int k = 0;
	char c;
	for (unsigned int i = 0; i < strlen(s); i++)
	{
		c = utf8ascii(s[i]);
		if (c != 0)
			s[k++] = c;
	}
	s[k] = 0;
}

String GetChipID()
{
	return String(ESP.getChipId());
}

int hexcolorToInt(char upper, char lower)
{
	int uVal = (int)upper;
	int lVal = (int)lower;
	uVal = uVal > 64 ? uVal - 55 : uVal - 48;
	uVal = uVal << 4;
	lVal = lVal > 64 ? lVal - 55 : lVal - 48;
	//  Serial.println(uVal+lVal);
	return uVal + lVal;
}

int GetRSSIasQuality(int rssi)
{
	int quality = 0;

	if (rssi <= -100)
	{
		quality = 0;
	}
	else if (rssi >= -50)
	{
		quality = 100;
	}
	else
	{
		quality = 2 * (rssi + 100);
	}
	return quality;
}

void updateMatrix(byte payload[], int length)
{
	if (!ignoreServer)
	{
		int y_offset = 5;
		if (firstStart)
		{
			//hardwareAnimatedCheck(1, 30, 2);
			firstStart = false;
		}

		connectionTimout = millis();

		switch (payload[0])
		{
		case 0:
		{
			//Command 0: DrawText

			//Prepare the coordinates
			uint16_t x_coordinate = int(payload[1] << 8) + int(payload[2]);
			uint16_t y_coordinate = int(payload[3] << 8) + int(payload[4]);

			matrix->setCursor(x_coordinate + 1, y_coordinate + y_offset);
			matrix->setTextColor(matrix->Color(payload[5], payload[6], payload[7]));
			String myText = "";
			for (int i = 8; i < length; i++)
			{
				char c = payload[i];
				myText += c;
			}

			matrix->print(utf8ascii(myText));
			break;
		}
		case 1:
		{
			//Command 1: DrawBMP

			//Prepare the coordinates
			uint16_t x_coordinate = int(payload[1] << 8) + int(payload[2]);
			uint16_t y_coordinate = int(payload[3] << 8) + int(payload[4]);

			int16_t width = payload[5];
			int16_t height = payload[6];

			unsigned short colorData[width * height];

			for (int i = 0; i < width * height * 2; i++)
			{
				colorData[i / 2] = (payload[i + 7] << 8) + payload[i + 1 + 7];
				i++;
			}

			for (int16_t j = 0; j < height; j++, y_coordinate++)
			{
				for (int16_t i = 0; i < width; i++)
				{
					matrix->drawPixel(x_coordinate + i, y_coordinate, (uint16_t)colorData[j * width + i]);
				}
			}
			break;
		}

		case 2:
		{
			//Command 2: DrawCircle

			//Prepare the coordinates
			uint16_t x0_coordinate = int(payload[1] << 8) + int(payload[2]);
			uint16_t y0_coordinate = int(payload[3] << 8) + int(payload[4]);
			uint16_t radius = payload[5];
			matrix->drawCircle(x0_coordinate, y0_coordinate, radius, matrix->Color(payload[6], payload[7], payload[8]));
			break;
		}
		case 3:
		{
			//Command 3: FillCircle

			//Prepare the coordinates
			uint16_t x0_coordinate = int(payload[1] << 8) + int(payload[2]);
			uint16_t y0_coordinate = int(payload[3] << 8) + int(payload[4]);
			uint16_t radius = payload[5];
			matrix->fillCircle(x0_coordinate, y0_coordinate, radius, matrix->Color(payload[6], payload[7], payload[8]));
			break;
		}
		case 4:
		{
			//Command 4: DrawPixel

			//Prepare the coordinates
			uint16_t x0_coordinate = int(payload[1] << 8) + int(payload[2]);
			uint16_t y0_coordinate = int(payload[3] << 8) + int(payload[4]);
			matrix->drawPixel(x0_coordinate, y0_coordinate, matrix->Color(payload[5], payload[6], payload[7]));
			break;
		}
		case 5:
		{
			//Command 5: DrawRect

			//Prepare the coordinates
			uint16_t x0_coordinate = int(payload[1] << 8) + int(payload[2]);
			uint16_t y0_coordinate = int(payload[3] << 8) + int(payload[4]);
			int16_t width = payload[5];
			int16_t height = payload[6];
			matrix->drawRect(x0_coordinate, y0_coordinate, width, height, matrix->Color(payload[7], payload[8], payload[9]));
			break;
		}
		case 6:
		{
			//Command 6: DrawLine

			//Prepare the coordinates
			uint16_t x0_coordinate = int(payload[1] << 8) + int(payload[2]);
			uint16_t y0_coordinate = int(payload[3] << 8) + int(payload[4]);
			uint16_t x1_coordinate = int(payload[5] << 8) + int(payload[6]);
			uint16_t y1_coordinate = int(payload[7] << 8) + int(payload[8]);
			matrix->drawLine(x0_coordinate, y0_coordinate, x1_coordinate, y1_coordinate, matrix->Color(payload[9], payload[10], payload[11]));
			break;
		}

		case 7:
		{
			//Command 7: FillMatrix

			matrix->fillScreen(matrix->Color(payload[1], payload[2], payload[3]));
			break;
		}

		case 8:
		{
			//Command 8: Show
			if (notify){
				matrix->drawPixel(31, 0, matrix->Color(200,0, 0));
			}
			matrix->show();
			break;
		}
		case 9:
		{
			//Command 9: Clear
			matrix->clear();
			break;
		}
		case 10:
		{
			//deprecated
			//Command 10: Play
			
  
			dfmp3.setVolume(payload[2]);
			delay(10);
			dfmp3.playMp3FolderTrack(payload[1]);
		
			break;
		}
		case 11:
		{
			//Command 11: reset
			ESP.reset();
			break;
		}
		case 12:
		{
			//Command 12: GetMatrixInfo
			StaticJsonBuffer<400> jsonBuffer;
			JsonObject &root = jsonBuffer.createObject();
			root["type"] = "MatrixInfo";
			root["version"] = version;
			root["wifirssi"] = String(WiFi.RSSI());
			root["wifiquality"] = GetRSSIasQuality(WiFi.RSSI());
			root["wifissid"] = WiFi.SSID();
			root["IP"] = WiFi.localIP().toString();
			if (ldrState != 0)
			{
				root["LUX"] = photocell.getCurrentLux();
			}
			else
			{
				root["LUX"] = 0;
			}

			switch (tempState)
			{
			case TempSensor_BME280:
				BMESensor.refresh();
				root["Temp"] = BMESensor.temperature;
				root["Hum"] = BMESensor.humidity;
				root["hPa"] = BMESensor.pressure;
				break;
			case TempSensor_HTU21D:
				root["Temp"] = htu.readTemperature();
				root["Hum"] = htu.readHumidity();
				root["hPa"] = 0;
				break;
			case TempSensor_BMP280:
				sensors_event_t temp_event, pressure_event;
				BMPSensor.getTemperatureSensor()->getEvent(&temp_event);
				BMPSensor.getPressureSensor()->getEvent(&pressure_event);

				root["Temp"] = temp_event.temperature;
				root["Hum"] = 0;
				root["hPa"] = pressure_event.pressure;
				break;
			default:
				root["Temp"] = 0;
				root["Hum"] = 0;
				root["hPa"] = 0;
				break;
			}

			String JS;
			root.printTo(JS);
			sendToServer(JS);
			break;
		}
		case 13:
		{
			matrix->setBrightness(MaxBrithness);
			break;
		}
		case 14:
		{
			//tempState = (int)payload[1];
			//audioState = (int)payload[2];
			//gestureState = (int)payload[3];
			ldrState = int(payload[1] << 8) + int(payload[2]);
			matrixTempCorrection = (int)payload[3];
			matrix->clear();
			matrix->setCursor(6, 6);
			matrix->setTextColor(matrix->Color(0, 255, 50));
			matrix->print("SAVED!");
			matrix->show();
			delay(2000);
			if (saveConfig())
			{
				ESP.reset();
			}
			break;
		}
		case 15:
		{

			matrix->clear();
			matrix->setTextColor(matrix->Color(255, 0, 0));
			matrix->setCursor(6, 6);
			matrix->print("RESET!");
			matrix->show();
			delay(1000);
			if (LittleFS.begin())
			{
				delay(1000);
				LittleFS.remove("/awtrix.json");

				LittleFS.end();
				delay(1000);
			}
			wifiManager.resetSettings();
			ESP.reset();
			break;
		}
		case 16:
		{
			sendToServer("ping");
			break;
		}
		case 17:
		{
			
			//Command 17: Volume
			dfmp3.setVolume(payload[1]);
			break;
		}
		case 18:
		{
			//Command 18: Play
			
			dfmp3.playMp3FolderTrack(payload[1]);
			break;
		}
		case 19:
		{
			//Command 18: Stop
			dfmp3.stopAdvertisement();
			delay(50);
			dfmp3.stop();
			break;
		}
		case 20:
		{
			//change the connection...
			USBConnection = false;
			WIFIConnection = false;
			firstStart = true;
			break;
		}
		case 21:
		{
			//multicolor...
			uint16_t x_coordinate = int(payload[1] << 8) + int(payload[2]);
			uint16_t y_coordinate = int(payload[3] << 8) + int(payload[4]);
			matrix->setCursor(x_coordinate + 1, y_coordinate + y_offset);

			String myJSON = "";
			for (int i = 5; i < length; i++)
			{
				myJSON += (char)payload[i];
			}
			//Serial.println("myJSON: " + myJSON + " ENDE");
			DynamicJsonBuffer jsonBuffer;
			JsonArray &array = jsonBuffer.parseArray(myJSON);
			if (array.success())
			{
				//Serial.println("Array erfolgreich geöffnet... =)");
				for (int i = 0; i < (int)array.size(); i++)
				{
					String tempString = array[i]["t"];
					String colorString = array[i]["c"];
					JsonArray &color = jsonBuffer.parseArray(colorString);
					if (color.success())
					{
						//Serial.println("Color erfolgreich geöffnet... =)");
						String myText = "";
						int r = color[0];
						int g = color[1];
						int b = color[2];
						//Serial.println("Test: " + tempString + " / Color: " + r + "/" + g + "/" + b);
						matrix->setTextColor(matrix->Color(r, g, b));
						for (int y = 0; y < (int)tempString.length(); y++)
						{
							myText += (char)tempString[y];
						}
						matrix->print(utf8ascii(myText));
					}
				}
			}
			break;
		}
		case 22:
		{
			//Text
			//scrollSpeed
			//icon
			//color
			//multicolor (textData?)
			//moveIcon
			//repeatIcon
			//duration
			//repeat
			//rainbow
			//progress
			//progresscolor
			//progressBackgroundColor
			//soundfile

			String myJSON = "";
			for (int i = 1; i < length; i++)
			{
				myJSON += (char)payload[i];
			}
			DynamicJsonBuffer jsonBuffer;
			JsonObject &json = jsonBuffer.parseObject(myJSON);

			String tempString = json["text"];
			String colorString = json["color"];

			JsonArray &color = jsonBuffer.parseArray(colorString);
			int r = color[0];
			int g = color[1];
			int b = color[2];
			int scrollSpeed = (int)json["scrollSpeed"];

			Serial.println("Scrollspeed: " + (String)(scrollSpeed));

			int textlaenge;
			while (true)
			{
				matrix->setCursor(32, 6);
				matrix->print(utf8ascii(tempString));
				textlaenge = (int)matrix->getCursorX() - 32;
				for (int i = 31; i > (-textlaenge); i--)
				{
					int starzeit = millis();
					matrix->clear();
					matrix->setCursor(i, 6);
					matrix->setTextColor(matrix->Color(r, g, b));
					matrix->print(utf8ascii(tempString));
					matrix->show();
					client.loop();
					int endzeit = millis();
					if ((scrollSpeed + starzeit - endzeit) > 0)
					{
						delay(scrollSpeed + starzeit - endzeit);
					}
				}
				connectionTimout = millis();
				break;
			}
			Serial.println("Textlänge auf Matrix: " + (String)(textlaenge));
			Serial.println("Test: " + tempString + " / Color: " + r + "/" + g + "/" + b);
			break;
		}
		case 23:
		{
			//Command 23: DrawFilledRect

			//Prepare the coordinates
			uint16_t x0_coordinate = int(payload[1] << 8) + int(payload[2]);
			uint16_t y0_coordinate = int(payload[3] << 8) + int(payload[4]);
			int16_t width = payload[5];
			int16_t height = payload[6];
			matrix->fillRect(x0_coordinate, y0_coordinate, width, height, matrix->Color(payload[7], payload[8], payload[9]));
			break;
		}
		case 24:
		{
			
			dfmp3.loopGlobalTrack(payload[1]);
			break;
		}
		case 25:
		{
			dfmp3.playAdvertisement(payload[1]);
			break;
		}
		case 26:
		{
			notify=payload[1];
			break;
		}
		}
	}
}

void callback(char *topic, byte *payload, unsigned int length)
{
	WIFIConnection = true;
	updateMatrix(payload, length);
}

void reconnect()
{
	//Serial.println("reconnecting to " + String(awtrix_server));
	String clientId = "AWTRIXController-";
	clientId += String(random(0xffff), HEX);
	hardwareAnimatedSearch(1, 28, 0);
	if (client.connect(clientId.c_str()))
	{
		//Serial.println("connected to server!");
		client.subscribe("awtrixmatrix/#");
		client.publish("matrixClient", "connected");
		matrix->fillScreen(matrix->Color(0, 0, 0));
		matrix->show();
	}
}

void ICACHE_RAM_ATTR interruptRoutine()
{
	isr_flag = 1;
}

void handleGesture()
{
	String control;
	if (apds.isGestureAvailable())
	{
		switch (apds.readGesture())
		{
		case DIR_UP:
			control = "UP";
			break;
		case DIR_DOWN:
			control = "DOWN";
			break;
		case DIR_LEFT:
			control = "LEFT";
			break;
		case DIR_RIGHT:
			control = "RIGHT";
			break;
		case DIR_NEAR:
			control = "NEAR";
			break;
		case DIR_FAR:
			control = "FAR";
			break;
		default:
			control = "NONE";
		}
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject &root = jsonBuffer.createObject();
		root["type"] = "gesture";
		root["gesture"] = control;
		String JS;
		root.printTo(JS);
		sendToServer(JS);
	}
}

uint32_t Wheel(byte WheelPos, int pos)
{
	if (WheelPos < 85)
	{
		return matrix->Color((WheelPos * 3) - pos, (255 - WheelPos * 3) - pos, 0);
	}
	else if (WheelPos < 170)
	{
		WheelPos -= 85;
		return matrix->Color((255 - WheelPos * 3) - pos, 0, (WheelPos * 3) - pos);
	}
	else
	{
		WheelPos -= 170;
		return matrix->Color(0, (WheelPos * 3) - pos, (255 - WheelPos * 3) - pos);
	}
}

void flashProgress(unsigned int progress, unsigned int total)
{
	matrix->setBrightness(MaxBrithness);
	long num = 32 * 8 * progress / total;
	for (unsigned char y = 0; y < 8; y++)
	{
		for (unsigned char x = 0; x < 32; x++)
		{
			if (num-- > 0)
				matrix->drawPixel(x, 8 - y - 1, Wheel((num * 16) & 255, 0));
		}
	}
	matrix->setCursor(1, 6);
	matrix->setTextColor(matrix->Color(200, 200, 200));
	matrix->print("FLASHING");
	matrix->show();
}

void saveConfigCallback()
{
	if (!USBConnection)
	{
		Serial.println("Should save config");
	}
	shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager)
{

	if (!USBConnection)
	{
		Serial.println("Entered config mode");
		Serial.println(WiFi.softAPIP());
		Serial.println(myWiFiManager->getConfigPortalSSID());
	}
	matrix->clear();
	matrix->setCursor(3, 6);
	matrix->setTextColor(matrix->Color(0, 255, 50));
	matrix->print("Hotspot");
	matrix->show();
}

void setup()
{
	delay(2000);

	for (int i = 0; i < tasterCount; i++)
	{
		pinMode(tasterPin[i], INPUT_PULLUP);
	}


	Serial.setRxBufferSize(1024);
	Serial.begin(115200);
	mySoftwareSerial.begin(9600);

	if (LittleFS.begin())
	{
		//if file not exists
		if (!(LittleFS.exists("/awtrix.json")))
		{
			LittleFS.open("/awtrix.json", "w+");
		}

		File configFile = LittleFS.open("/awtrix.json", "r");
		if (configFile)
		{
			size_t size = configFile.size();
			// Allocate a buffer to store contents of the file.
			std::unique_ptr<char[]> buf(new char[size]);
			configFile.readBytes(buf.get(), size);
			DynamicJsonBuffer jsonBuffer;
			JsonObject &json = jsonBuffer.parseObject(buf.get());
			if (json.success())
			{

				strcpy(awtrix_server, json["awtrix_server"]);

				if (json.containsKey("matrixType"))
				{
					matrixType = json["matrixType"].as<int>();
				}

				matrixTempCorrection = json["matrixCorrection"].as<int>();

				if (json.containsKey("Port"))
				{
					strcpy(Port, json["Port"]);
				}
			}
			configFile.close();
		}
	}
	else
	{
		//error
	}
	Serial.println("matrixType");
	Serial.println(matrixType);

	if (testing)
	{
		/* code */
		matrixType = 1;
	}
	
	switch (matrixType)
	{
	case 0:
		matrix = new FastLED_NeoMatrix(leds, Width, Heigh, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG);
		break;
	case 1:
		matrix = new FastLED_NeoMatrix(leds, Width, Heigh, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);
		break;
	default:
		matrix = new FastLED_NeoMatrix(leds, Width, Heigh, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG);
		break;
	}

	switch (matrixTempCorrection)
	{
	case 0:
		FastLED.addLeds<WS2812B, D2, COLOR_ORDER>(leds, LED_NUM).setCorrection(TypicalLEDStrip);
		break;
	case 1:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(Candle);
		break;
	case 2:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(Tungsten40W);
		break;
	case 3:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(Tungsten100W);
		break;
	case 4:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(Halogen);
		break;
	case 5:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(CarbonArc);
		break;
	case 6:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(HighNoonSun);
		break;
	case 7:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(DirectSunlight);
		break;
	case 8:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(OvercastSky);
		break;
	case 9:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(ClearBlueSky);
		break;
	case 10:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(WarmFluorescent);
		break;
	case 11:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(StandardFluorescent);
		break;
	case 12:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(CoolWhiteFluorescent);
		break;
	case 13:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(FullSpectrumFluorescent);
		break;
	case 14:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(GrowLightFluorescent);
		break;
	case 15:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(BlackLightFluorescent);
		break;
	case 16:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(MercuryVapor);
		break;
	case 17:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(SodiumVapor);
		break;
	case 18:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(MetalHalide);
		break;
	case 19:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(HighPressureSodium);
		break;
	case 20:
		FastLED.addLeds<NEOPIXEL, D2>(leds, LED_NUM).setTemperature(UncorrectedTemperature);
		break;
	default:
		FastLED.addLeds<WS2812B, D2, COLOR_ORDER>(leds, LED_NUM).setCorrection(TypicalLEDStrip);
		break;
	}

	matrix->begin();
	matrix->setRemapFunction(XY);	//call the remapFunction to map the pixels
	matrix->setTextWrap(false);
	matrix->setBrightness(MaxBrithness);
	matrix->setFont(&TomThumb);
	//Reset with Tasters...
	int zeit = millis();
	int zahl = 5;
	int zahlAlt = 6;
	matrix->clear();

	/*
		starting test animation
	*/
	//set full screen to red
	matrix->fillScreen(matrix->Color(255, 0, 0));
	matrix->show();
	delay(1000);
	matrix->clear();
	//set full screen to green
	matrix->fillScreen(matrix->Color(0, 255, 0));
	matrix->show();
	delay(1000);
	matrix->clear();
	//set full screen to blue
	matrix->fillScreen(matrix->Color(0, 0, 255));
	matrix->show();
	delay(1000);
	matrix->clear();
	//set full screen to white
	matrix->fillScreen(matrix->Color(255, 255, 255));
	matrix->show();
	delay(1000);
	matrix->clear();


	matrix->setTextColor(matrix->Color(255, 0, 255));
	matrix->setCursor(9, 6);
	matrix->print("BOOT");
	matrix->show();
	delay(2000);
	while (!digitalRead(D4))
	{
		if (zahl != zahlAlt)
		{
			matrix->clear();
			matrix->setTextColor(matrix->Color(255, 0, 0));
			matrix->setCursor(6, 6);
			matrix->print("RESET ");
			matrix->print(zahl);
			matrix->show();
			zahlAlt = zahl;
		}
		zahl = 5 - ((millis() - zeit) / 1000);
		if (zahl == 0)
		{
			matrix->clear();
			matrix->setTextColor(matrix->Color(255, 0, 0));
			matrix->setCursor(6, 6);
			matrix->print("RESET!");
			matrix->show();
			delay(1000);
			if (LittleFS.begin())
			{
				delay(1000);
				LittleFS.remove("/awtrix.json");

				LittleFS.end();
				delay(1000);
			}
			wifiManager.resetSettings();
			ESP.reset();
		}
	}
	/*
		if (drd.detect())
		{
			//Serial.println("** Double reset boot **");
			matrix->clear();
			matrix->setTextColor(matrix->Color(255, 0, 0));
			matrix->setCursor(6, 6);
			matrix->print("RESET!");
			matrix->show();
			delay(1000);
			if (LittleFS.begin())
			{
				delay(1000);
				LittleFS.remove("/awtrix.json");

				LittleFS.end();
				delay(1000);
			}
			wifiManager.resetSettings();
			ESP.reset();
		}
		*/

	wifiManager.setAPStaticIPConfig(IPAddress(172, 217, 28, 1), IPAddress(172, 217, 28, 1), IPAddress(255, 255, 255, 0));
	WiFiManagerParameter custom_awtrix_server("server", "AWTRIX Host", awtrix_server, 16);
	WiFiManagerParameter custom_port("Port", "Matrix Port", Port, 6);
	WiFiManagerParameter custom_matrix_type("matrixType", "MatrixType", "0", 1);
	// Just a quick hint
	WiFiManagerParameter host_hint("<small>AWTRIX Host IP (without Port)<br></small><br><br>");
	WiFiManagerParameter port_hint("<small>Communication Port (default: 7001)<br></small><br><br>");
	WiFiManagerParameter matrix_hint("<small>0: Columns; 1: Tiles; 2: Rows <br></small><br><br>");
	WiFiManagerParameter p_lineBreak_notext("<p></p>");

	wifiManager.setSaveConfigCallback(saveConfigCallback);
	wifiManager.setAPCallback(configModeCallback);

	wifiManager.addParameter(&p_lineBreak_notext);
	wifiManager.addParameter(&host_hint);
	wifiManager.addParameter(&custom_awtrix_server);
	wifiManager.addParameter(&port_hint);
	wifiManager.addParameter(&custom_port);
	wifiManager.addParameter(&matrix_hint);
	wifiManager.addParameter(&custom_matrix_type);
	wifiManager.addParameter(&p_lineBreak_notext);

	//wifiManager.setCustomHeadElement("<style>html{ background-color: #607D8B;}</style>");

	hardwareAnimatedSearch(0, 24, 0);

	if (!wifiManager.autoConnect("AWTRIX Controller", "awtrixxx"))
	{
		//reset and try again, or maybe put it to deep sleep
		ESP.reset();
		delay(5000);
	}

	//is needed for only one hotpsot!
	WiFi.mode(WIFI_STA);

	server.on("/", HTTP_GET, []() {
		server.sendHeader("Connection", "close");
		server.send(200, "text/html", serverIndex);
	});

	server.on("/reset", HTTP_GET, []() {
		server.send(200, "text/html", serverIndex);
		wifiManager.resetSettings();
		ESP.reset();
	});
	
	server.on(
		"/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart(); }, []() {
      HTTPUpload& upload = server.upload();

      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);

        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
		  matrix->clear();
		  flashProgress((int)upload.currentSize,(int)upload.buf);
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
		  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");


        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield(); });

	server.begin();

	if (shouldSaveConfig)
	{

		strcpy(awtrix_server, custom_awtrix_server.getValue());
		matrixType =  atoi(custom_matrix_type.getValue());
		strcpy(Port, custom_port.getValue());
		saveConfig();
		ESP.reset();
	}

	hardwareAnimatedCheck(MsgType_Wifi, 27, 2);

	delay(1000); //is needed for the dfplayer to startup

	//Checking periphery
	Wire.begin(I2C_SDA, I2C_SCL);
	if (BMESensor.begin())
	{
		//temp OK
		tempState = TempSensor_BME280;
		hardwareAnimatedCheck(MsgType_Temp, 29, 2);
	}
	else if (htu.begin())
	{
		tempState = TempSensor_HTU21D;
		hardwareAnimatedCheck(MsgType_Temp, 29, 2);
	}
	else if (BMPSensor.begin(BMP280_ADDRESS_ALT) || BMPSensor.begin(BMP280_ADDRESS))
	{

		/* Default settings from datasheet. */
		BMPSensor.setSampling(Adafruit_BMP280::MODE_NORMAL,		/* Operating Mode. */
							  Adafruit_BMP280::SAMPLING_X2,		/* Temp. oversampling */
							  Adafruit_BMP280::SAMPLING_X16,	/* Pressure oversampling */
							  Adafruit_BMP280::FILTER_X16,		/* Filtering. */
							  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
		tempState = TempSensor_BMP280;
		hardwareAnimatedCheck(MsgType_Temp, 29, 2);
	}

	dfmp3.begin();

	if (0)
	{ //Use softwareSerial to communicate with mp3.
		hardwareAnimatedCheck(MsgType_Audio, 29, 2);
	}

	attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);
	apds.enableGestureSensor(true);
	if (apds.init())
	{
		hardwareAnimatedCheck(MsgType_Gest, 29, 2);
		pinMode(APDS9960_INT, INPUT);
	}

	photocell.setPhotocellPositionOnGround(false);
	if (photocell.getCurrentLux() > 1)
	{
		hardwareAnimatedCheck(MsgType_LDR, 29, 2);
	}

	ArduinoOTA.onStart([&]() {
		updating = true;
		matrix->clear();
	});

	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		flashProgress(progress, total);
	});

	ArduinoOTA.begin();

	matrix->clear();
	matrix->setCursor(7, 6);

	bufferpointer = 0;

	myTime = millis() - 500;
	myTime2 = millis() - 1000;
	myTime3 = millis() - 500;
	myCounter = 0;
	myCounter2 = 0;

	for (int x = 32; x >= -90; x--)
	{
		matrix->clear();
		matrix->setCursor(x, 6);
		matrix->print("Host-IP: " + String(awtrix_server) + ":" + String(Port));
		matrix->setTextColor(matrix->Color(0, 255, 50));
		matrix->show();
		delay(40);
	}

	client.setServer(awtrix_server, atoi(Port));
	client.setCallback(callback);

	ignoreServer = false;

	connectionTimout = millis();
}

void loop()
{
	server.handleClient();
	ArduinoOTA.handle();

	//is needed for the server search animation
	if (firstStart && !ignoreServer)
	{
		if (millis() - myTime > 500)
		{
			serverSearch(myCounter, 0, 28, 0);
			myCounter++;
			if (myCounter == 4)
			{
				myCounter = 0;
			}
			myTime = millis();
		}
	}

	//not during the falsh process
	if (!updating)
	{
		if (USBConnection || firstStart)
		{
			int x = 100;
			while (x >= 0)
			{
				x--;
				//USB
				if (Serial.available() > 0)
				{
					//read and fill in ringbuffer
					myBytes[bufferpointer] = Serial.read();
					messageLength--;
					for (int i = 0; i < 14; i++)
					{
						if ((bufferpointer - i) < 0)
						{
							myPointer[i] = 1000 + bufferpointer - i;
						}
						else
						{
							myPointer[i] = bufferpointer - i;
						}
					}
					//prefix from "awtrix" == 6?
					if (myBytes[myPointer[13]] == 0 && myBytes[myPointer[12]] == 0 && myBytes[myPointer[11]] == 0 && myBytes[myPointer[10]] == 6)
					{
						//"awtrix" ?
						if (myBytes[myPointer[9]] == 97 && myBytes[myPointer[8]] == 119 && myBytes[myPointer[7]] == 116 && myBytes[myPointer[6]] == 114 && myBytes[myPointer[5]] == 105 && myBytes[myPointer[4]] == 120)
						{
							messageLength = (int(myBytes[myPointer[3]]) << 24) + (int(myBytes[myPointer[2]]) << 16) + (int(myBytes[myPointer[1]]) << 8) + int(myBytes[myPointer[0]]);
							SavemMessageLength = messageLength;
							awtrixFound = true;
						}
					}

					if (awtrixFound && messageLength == 0)
					{
						byte tempData[SavemMessageLength];
						int up = 0;
						for (int i = SavemMessageLength - 1; i >= 0; i--)
						{
							if ((bufferpointer - i) >= 0)
							{
								tempData[up] = myBytes[bufferpointer - i];
							}
							else
							{
								tempData[up] = myBytes[1000 + bufferpointer - i];
							}
							up++;
						}
						USBConnection = true;
						updateMatrix(tempData, SavemMessageLength);
						awtrixFound = false;
					}
					bufferpointer++;
					if (bufferpointer == 1000)
					{
						bufferpointer = 0;
					}
				}
				else
				{
					break;
				}
			}
		}
		//Wifi
		if (WIFIConnection || firstStart)
		{
			//Serial.println("wifi oder first...");
			if (!client.connected())
			{
				//Serial.println("nicht verbunden...");
				reconnect();
				if (WIFIConnection)
				{
					USBConnection = false;
					WIFIConnection = false;
					firstStart = true;
				}
			}
			else
			{
				client.loop();
			}
		}
		//check gesture sensor
		if (isr_flag == 1)
		{
			detachInterrupt(APDS9960_INT);
			handleGesture();
			isr_flag = 0;
			attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);
		}

		if (millis() - connectionTimout > 20000)
		{
			USBConnection = false;
			WIFIConnection = false;
			firstStart = true;
		}
	}

	checkTaster(0);
	checkTaster(1);
	checkTaster(2);
	//checkTaster(3);

	//is needed for the menue...
	if (ignoreServer)
	{
		if (pressedTaster > 0)
		{
			matrix->clear();
			matrix->setCursor(0, 6);
			matrix->setTextColor(matrix->Color(0, 255, 50));
			//matrix->print(myMenue.getMenueString(&menuePointer, &pressedTaster, &minBrightness, &maxBrightness));
			matrix->show();
		}

		//get data and ignore
		if (Serial.available() > 0)
		{
			Serial.read();
		}
	}
}
