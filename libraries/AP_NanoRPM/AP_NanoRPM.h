#pragma once

#include <inttypes.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/I2CDevice.h>

struct NanoRPM_report_s {
    //uint64_t sampletime; // sample time in us for debugging
    float data;
};

class AP_NanoRPM
{
public:
    AP_NanoRPM();
    ~AP_NanoRPM();

    bool init();

private:

    AP_HAL::OwnPtr<AP_HAL::I2CDevice> _dev;

    uint16_t            _scaling_factor;
    NanoRPM_report_s        *_samples;

    void _update();
    void Log_Write_NanoRPM(NanoRPM_report_s* rep);
};
