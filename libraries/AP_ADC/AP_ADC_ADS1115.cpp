#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/utility/sparse-endian.h>
#include <AP_Logger/AP_Logger.h>

#include "AP_ADC_ADS1115.h"

#define ADS1115_ADDRESS_ADDR_GND    0x48 // address pin low (GND)
#define ADS1115_ADDRESS_ADDR_VDD    0x49 // address pin high (VCC)
#define ADS1115_ADDRESS_ADDR_SDA    0x4A // address pin tied to SDA pin
#define ADS1115_ADDRESS_ADDR_SCL    0x4B // address pin tied to SCL pin

//#define ADS1115_I2C_ADDR            ADS1115_ADDRESS_ADDR_GND
#define ADS1115_I2C_BUS             1

#define ADS1115_RA_CONVERSION       0x00
#define ADS1115_RA_CONFIG           0x01
#define ADS1115_RA_LO_THRESH        0x02
#define ADS1115_RA_HI_THRESH        0x03

#define ADS1115_OS_SHIFT            15
#define ADS1115_OS_INACTIVE         0x00 << ADS1115_OS_SHIFT
#define ADS1115_OS_ACTIVE           0x01 << ADS1115_OS_SHIFT

#define ADS1115_MUX_SHIFT           12
#define ADS1115_MUX_P0_N1           0x00 << ADS1115_MUX_SHIFT /* default */
#define ADS1115_MUX_P0_N3           0x01 << ADS1115_MUX_SHIFT
#define ADS1115_MUX_P1_N3           0x02 << ADS1115_MUX_SHIFT
#define ADS1115_MUX_P2_N3           0x03 << ADS1115_MUX_SHIFT
#define ADS1115_MUX_P0_NG           0x04 << ADS1115_MUX_SHIFT
#define ADS1115_MUX_P1_NG           0x05 << ADS1115_MUX_SHIFT
#define ADS1115_MUX_P2_NG           0x06 << ADS1115_MUX_SHIFT
#define ADS1115_MUX_P3_NG           0x07 << ADS1115_MUX_SHIFT

#define ADS1115_PGA_SHIFT           9
#define ADS1115_PGA_6P144           0x00 << ADS1115_PGA_SHIFT
#define ADS1115_PGA_4P096           0x01 << ADS1115_PGA_SHIFT
#define ADS1115_PGA_2P048           0x02 << ADS1115_PGA_SHIFT // default
#define ADS1115_PGA_1P024           0x03 << ADS1115_PGA_SHIFT
#define ADS1115_PGA_0P512           0x04 << ADS1115_PGA_SHIFT
#define ADS1115_PGA_0P256           0x05 << ADS1115_PGA_SHIFT
#define ADS1115_PGA_0P256B          0x06 << ADS1115_PGA_SHIFT
#define ADS1115_PGA_0P256C          0x07 << ADS1115_PGA_SHIFT

#define ADS1115_MV_6P144            0.187500f
#define ADS1115_MV_4P096            0.125000f
#define ADS1115_MV_2P048            0.062500f // default
#define ADS1115_MV_1P024            0.031250f
#define ADS1115_MV_0P512            0.015625f
#define ADS1115_MV_0P256            0.007813f
#define ADS1115_MV_0P256B           0.007813f
#define ADS1115_MV_0P256C           0.007813f

#define ADS1115_MODE_SHIFT          8
#define ADS1115_MODE_CONTINUOUS     0x00 << ADS1115_MODE_SHIFT
#define ADS1115_MODE_SINGLESHOT     0x01 << ADS1115_MODE_SHIFT // default

#define ADS1115_RATE_SHIFT          5
#define ADS1115_RATE_8              0x00 << ADS1115_RATE_SHIFT
#define ADS1115_RATE_16             0x01 << ADS1115_RATE_SHIFT
#define ADS1115_RATE_32             0x02 << ADS1115_RATE_SHIFT
#define ADS1115_RATE_64             0x03 << ADS1115_RATE_SHIFT
#define ADS1115_RATE_128            0x04 << ADS1115_RATE_SHIFT // default
#define ADS1115_RATE_250            0x05 << ADS1115_RATE_SHIFT
#define ADS1115_RATE_475            0x06 << ADS1115_RATE_SHIFT
#define ADS1115_RATE_860            0x07 << ADS1115_RATE_SHIFT

#define ADS1115_COMP_MODE_SHIFT         4
#define ADS1115_COMP_MODE_HYSTERESIS    0x00 << ADS1115_COMP_MODE_SHIFT        // default
#define ADS1115_COMP_MODE_WINDOW        0x01 << ADS1115_COMP_MODE_SHIFT

#define ADS1115_COMP_POL_SHIFT          3
#define ADS1115_COMP_POL_ACTIVE_LOW     0x00 << ADS1115_COMP_POL_SHIFT     // default
#define ADS1115_COMP_POL_ACTIVE_HIGH    0x01 << ADS1115_COMP_POL_SHIFT

#define ADS1115_COMP_LAT_SHIFT          2
#define ADS1115_COMP_LAT_NON_LATCHING   0x00 << ADS1115_COMP_LAT_SHIFT    // default
#define ADS1115_COMP_LAT_LATCHING       0x01 << ADS1115_COMP_LAT_SHIFT

#define ADS1115_COMP_QUE_SHIFT      0
#define ADS1115_COMP_QUE_ASSERT1    0x00 << ADS1115_COMP_SHIFT
#define ADS1115_COMP_QUE_ASSERT2    0x01 << ADS1115_COMP_SHIFT
#define ADS1115_COMP_QUE_ASSERT4    0x02 << ADS1115_COMP_SHIFT
#define ADS1115_COMP_QUE_DISABLE    0x03 // default

#define ADS1115_DEBUG 0
#if ADS1115_DEBUG
#include <cstdio>
#define debug(fmt, args ...)  do {hal.console->printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); } while(0)
#define error(fmt, args ...)  do {fprintf(stderr,"%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); } while(0)
#else
#define debug(fmt, args ...)
#define error(fmt, args ...)
#endif

extern const AP_HAL::HAL &hal;

#define ADS1115_CHANNELS_COUNT           4
#define ADS1115_ADDRESS_COUNT           4

const uint8_t AP_ADC_ADS1115::_channels_number  = ADS1115_CHANNELS_COUNT;
const uint8_t AP_ADC_ADS1115::_addresses_number  = ADS1115_ADDRESS_COUNT;

/* No differential channels used */
static const uint16_t mux_table[ADS1115_CHANNELS_COUNT] = {
    ADS1115_MUX_P0_NG,
    ADS1115_MUX_P1_NG,
    ADS1115_MUX_P2_NG,
    ADS1115_MUX_P3_NG
};

static const uint8_t addr_table[ADS1115_ADDRESS_COUNT] = {
    ADS1115_ADDRESS_ADDR_GND,
	ADS1115_ADDRESS_ADDR_VDD,
	ADS1115_ADDRESS_ADDR_SDA,
	ADS1115_ADDRESS_ADDR_SCL
};


AP_ADC_ADS1115::AP_ADC_ADS1115()
    :
    _gain(ADS1115_PGA_6P144)
{
    _samples = new adc_report_s[_channels_number*_addresses_number];
}

AP_ADC_ADS1115::~AP_ADC_ADS1115()
{
    delete[] _samples;
}

bool AP_ADC_ADS1115::init(uint8_t boards)
{

	for(int i = 0; i < boards; i++){
		_dev.push_back(hal.i2c_mgr->get_device(ADS1115_I2C_BUS, addr_table[i], 400000));
		if (!_dev[i]) {
			return false;
		}
	}

    _gain = ADS1115_PGA_6P144;

    // 10000us -> 100Hz.
    _dev[0]->register_periodic_callback(10000, FUNCTOR_BIND_MEMBER(&AP_ADC_ADS1115::_update, void));

    return true;
}

bool AP_ADC_ADS1115::_start_conversion(uint8_t channel, uint8_t board)
{
    struct PACKED {
        uint8_t reg;
        be16_t val;
    } config;

    config.reg = ADS1115_RA_CONFIG;
    config.val = htobe16(ADS1115_OS_ACTIVE | _gain | mux_table[channel] |
                         ADS1115_MODE_SINGLESHOT | ADS1115_COMP_QUE_DISABLE |
                         ADS1115_RATE_860);

    return _dev[board]->transfer((uint8_t *)&config, sizeof(config), nullptr, 0);
}

float AP_ADC_ADS1115::_convert_register_data_to_mv(int16_t word) const
{

    float pga;

    switch (_gain) {
    case ADS1115_PGA_6P144:
        pga = ADS1115_MV_6P144;
        break;
    case ADS1115_PGA_4P096:
        pga = ADS1115_MV_4P096;
        break;
    case ADS1115_PGA_2P048:
        pga = ADS1115_MV_2P048;
        break;
    case ADS1115_PGA_1P024:
        pga = ADS1115_MV_1P024;
        break;
    case ADS1115_PGA_0P512:
        pga = ADS1115_MV_0P512;
        break;
    case ADS1115_PGA_0P256:
        pga = ADS1115_MV_0P256;
        break;
    case ADS1115_PGA_0P256B:
        pga = ADS1115_MV_0P256B;
        break;
    case ADS1115_PGA_0P256C:
        pga = ADS1115_MV_0P256C;
        break;
    default:
        pga = 0.0f;
        hal.console->printf("Wrong gain");
        AP_HAL::panic("ADS1115: wrong gain selected");
        break;
    }

    return (float) word * pga;
}

void AP_ADC_ADS1115::_update()
{

	be16_t val;

	for(int channel = 0; channel < _channels_number; channel++){

		for(int board = 0; board < _dev.size(); board++){

			// Small delay to battle the scrambling I found the day I added the NanoRPM measurements (there should be no correlation) Lowest working: 50us

			hal.scheduler->delay_microseconds(50);

			if (!_dev[board]->read_registers(ADS1115_RA_CONVERSION, (uint8_t *)&val,  sizeof(val))) {
				AP::logger().Write_MessageF("_dev->read_registers (CONV) failed in ADS1115");
				return;
			}

			float sample = _convert_register_data_to_mv(be16toh(val));

			_samples[board*_channels_number+channel].data = sample*0.001f; // mVolts to Volts

			_start_conversion((channel+1)%_channels_number, board);

		}

		// delay to not scramble results. This is against ardupilot official driver recommendations, but needed with one board to not scramble results.
		if(_dev.size()==1){
			hal.scheduler->delay_microseconds(1000);
		}
	}

	Log_Write_ADC(_samples);

}
