#ifndef _XLCD_SENSORS_CHAMBER_TEMP
#define _XLCD_SENSORS_CHAMBER_TEMP

#include <OneWire.h>
#include <DallasTemperature.h>
#include "ui/ui_msgs.h"

#if defined(__XPTOUCH_SCREEN_S3_028__)
#define XPTOUCH_CHAMBER_TEMP_PIN 44
#elif defined(__XPTOUCH_PLATFORM_S3__)
#define XPTOUCH_CHAMBER_TEMP_PIN 18
#else
#define XPTOUCH_CHAMBER_TEMP_PIN 22
#endif

// Setup a temperatureSensorsOneWire instance to communicate with any temperatureSensorsOneWire devices
OneWire temperatureSensorsOneWire(XPTOUCH_CHAMBER_TEMP_PIN);

// Pass our temperatureSensorsOneWire reference to Dallas Temperature sensor
DallasTemperature xptouch_chamber_sensors(&temperatureSensorsOneWire);

lv_timer_t *xptouch_chambertemp_requestTemperaturesTimer = NULL;

void xptouch_chamber_requestTemperatures(lv_timer_t *timer);

void xptouch_chamber_timer_create()
{
    xptouch_chambertemp_requestTemperaturesTimer = lv_timer_create(xptouch_chamber_requestTemperatures, 2500, NULL);
    lv_timer_set_repeat_count(xptouch_chambertemp_requestTemperaturesTimer, 1);
}

void xptouch_chamber_requestTemperatures(lv_timer_t *timer)
{
    int temperatureC = xptouch_chamber_sensors.getTempCByIndex(0) + xPTouchConfig.xTouchChamberSensorReadingDiff;
    bambuStatus.chamber_temper = temperatureC;
    ui_msg_send(XPTOUCH_ON_CHAMBER_TEMP, (unsigned long long)(int64_t)temperatureC, 0);
    xptouch_chamber_sensors.requestTemperatures();
    xptouch_chamber_timer_create();
}

bool xptouch_chamber_started = false;
void xptouch_chamber_timer_start()
{
    if (!xptouch_chamber_started)
    {
        xptouch_chamber_sensors.begin();
        xptouch_chamber_sensors.setWaitForConversion(false);
        xptouch_chamber_started = true;
    }
    xptouch_chamber_timer_create();
}

void xptouch_chamber_timer_stop()
{
    if (xptouch_chambertemp_requestTemperaturesTimer == NULL)
    {
        return;
    }
    lv_timer_pause(xptouch_chambertemp_requestTemperaturesTimer);
}

void xptouch_chamber_timer_init()
{
    if (!xptouch_bblp_is_p1Series())
    {
        return;
    }

    if (xPTouchConfig.xTouchChamberSensorEnabled)
    {
        xptouch_chamber_timer_start();
    }
    else
    {
        if (xptouch_chamber_started)
        {

            xptouch_chamber_timer_stop();
        }
    }
}

#endif
