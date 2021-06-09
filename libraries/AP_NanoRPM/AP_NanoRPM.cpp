#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/utility/sparse-endian.h>
#include <AP_Logger/AP_Logger.h>

#include "AP_NanoRPM.h"

#define NANORPM_I2C_ADDR    0x29 // Default address
#define NANORPM_I2C_BUS     1

// Scaling factor for number of magnets / pairs of poles for a Brushless RPM sensor

#define SCALING_FACTOR     2

#define NANORPM_DEBUG 		0
#if NANORPM_DEBUG
#include <cstdio>
#define debug(fmt, args ...)  do {hal.console->printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); } while(0)
#define error(fmt, args ...)  do {fprintf(stderr,"%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); } while(0)
#else
#define debug(fmt, args ...)
#define error(fmt, args ...)
#endif

extern const AP_HAL::HAL &hal;


AP_NanoRPM::AP_NanoRPM():
    _scaling_factor(SCALING_FACTOR)
{
    _samples = new NanoRPM_report_s[1];
}

AP_NanoRPM::~AP_NanoRPM()
{
    delete[] _samples;
}

bool AP_NanoRPM::init()
{

	_dev = hal.i2c_mgr->get_device(NANORPM_I2C_BUS, NANORPM_I2C_ADDR, 400000);
	if (!_dev) {
		return false;
	}

    // 100000us -> 10Hz.
    _dev->register_periodic_callback(100000, FUNCTOR_BIND_MEMBER(&AP_NanoRPM::_update, void));

    return true;
}

void AP_NanoRPM::_update()
{

	static union {
	    uint32_t long_value;
	    uint8_t bytes[2];
	} bytes_union;

	if (!_dev->read_registers(NANORPM_I2C_ADDR, bytes_union.bytes,  sizeof(bytes_union.bytes))) {
		AP::logger().Write_MessageF("_dev->read_registers failed in NanoRPM");
		return;
	}

	_samples[0].data = bytes_union.long_value/_scaling_factor;

	Log_Write_NanoRPM(_samples);

}
