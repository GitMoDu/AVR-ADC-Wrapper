// AvrAdc.h
// Static wrapper for AVR ADC functionality.
// Made possible by Nick Gammon (www.gammon.com.au/adc).
// TODO: Finish implementation for ATTiny85.
// TODO: Validate channel count.

#if !defined(_AVRADC_h) && defined(ARDUINO_ARCH_AVR)
#define _AVRADC_h

#include <avr/sleep.h> 
#include <Arduino.h>

enum PrescalerEnum
{
	P2,
	P4,
	P8,
	P16,
	P32,
	P64,
	P128
};

const uint16_t AdcRange = 1024;
const uint16_t AdcChannelCount = 5; //TODO: Check if more available?
const uint32_t AdcSettlePeriodDefault = 15;
const uint32_t AdcSamplingPeriodDefault = 1;

#if F_CPU > 8000000
const PrescalerEnum PrescalerDefault = PrescalerEnum::P16;
#elif F_CPU > 1000000
const PrescalerEnum PrescalerDefault = PrescalerEnum::P8;
#else
const PrescalerEnum PrescalerDefault = PrescalerEnum::P1;
#endif

#if defined(ATTINY_CORE) 	// TODO: Validate for ATTiny85.
class BaseAvrAdc
{
public:
	static void SetReferenceExternal()
	{
		ADMUX &= ~(bit(REFS0) | bit(REFS1)); // External AREF reference.
	}

	static void SetReferenceInternal1100()
	{
		ADMUX |= bit(REFS0) | bit(REFS1); // Internal 1100 mV reference.
	}

	static void SetReferenceAvcc()
	{
		ADMUX = (ADMUX | bit(REFS0)) & ~bit(REFS1); // Internal AVcc reference.
	}

	static void SetReferenceInternal2560()
	{
		ADMUX = bit(REFS0); // Internal 2560 mV reference.
	}
};

enum PositiveNegativePairEnum : uint8_t
{
	ADC2ADC0 = 100,
	ADC2ADC1 = 110,
	ADC2ADC3 = 1000,
	ADC0ADC1,
	ADC0ADC2,
	ADC0ADC3,
};

enum DifferentialGainEnum
{
	X1,
	X20
};

class DifferentialAvrAdc : public BaseAvrAdc
{
public:
	static void SetChannels(PositiveNegativePairEnum pair)
	{
		switch (pair)
		{
		case ADC0ADC1:
			ADMUX = bit(MUX2);
			break;
		case ADC0ADC2:
			ADMUX = bit(MUX2) | bit(MUX3);
			break;
		case ADC0ADC3:
			break;
		case ADC2ADC0:
			break;
		case ADC2ADC1:
			break;
		case ADC2ADC3:
			break;
		default:
			break;
		}
	}

	static void SetChannels(const uint8_t channel)
	{
		ADMUX |= channel << 2;
	}

	static void SetDifferentialGain(DifferentialGainEnum gain)
	{
		switch (gain)
		{
		case DifferentialGainEnum::X1:
			ADMUX &= ~bit(MUX0);
			break;
		case DifferentialGainEnum::X20:
			ADMUX |= bit(MUX0);
			break;
		default:
			break;
		}
	}
};
#else
class BaseAvrAdc
{
public:
	static void AdcOn()
	{
		ADCSRA |= bit(ADEN) | bit(ADIF);
	}

	static void AdcOff()
	{
		ADCSRA &= ~bit(ADEN);
	}

	static void SetReferenceExternal()
	{
		ADMUX &= ~(bit(REFS0) | bit(REFS1)); // External AREF reference.
	}

	static void SetReferenceInternal1100()
	{
		ADMUX |= bit(REFS0) | bit(REFS1); // Internal 1100 mV reference.
	}

	static void SetReferenceAvcc()
	{
		ADMUX &= ~bit(REFS1); // Internal AVcc reference.
		ADMUX |= bit(REFS0);
	}

	static bool IsSampleDone()
	{
		return bit_is_clear(ADCSRA, ADSC); // ADC clears bit when sample is done.
	}

	static void StartSampleWithSleep()
	{
		noInterrupts();
		set_sleep_mode(SLEEP_MODE_ADC);    // Sleep during sample.
		sleep_enable();

		// Start the conversion with interrupt.
		ADCSRA |= bit(ADSC) | bit(ADIE);
		interrupts();
		sleep_cpu();
		sleep_disable();
	}

	static void StartSample()
	{
		ADCSRA |= bit(ADSC);
	}

	static void SetPrescaler(const PrescalerEnum prescalar)
	{
		ADCSRA &= ~(bit(ADPS0) | bit(ADPS1) | bit(ADPS2)); // Clear prescalar.

		switch (prescalar)
		{
		case PrescalerEnum::P2:
			ADCSRA |= bit(ADPS0);
			break;
		case PrescalerEnum::P4:
			ADCSRA |= bit(ADPS1);
			break;
		case PrescalerEnum::P8:
			ADCSRA |= bit(ADPS0) | bit(ADPS1);
			break;
		case PrescalerEnum::P16:
			ADCSRA |= bit(ADPS2);
			break;
		case PrescalerEnum::P32:
			ADCSRA |= bit(ADPS0) | bit(ADPS2);
			break;
		case PrescalerEnum::P64:
			ADCSRA |= bit(ADPS1) | bit(ADPS2);
			break;
		case PrescalerEnum::P128:
			ADCSRA |= bit(ADPS0) | bit(ADPS1) | bit(ADPS2);
			break;
		default:
			break;
		}
	}
};

class HalfScaleAvrAdc : public BaseAvrAdc
{
public:
	static const uint16_t AdcMax = UINT8_MAX;

public:
	static void SetScale()
	{
		ADMUX |= bit(ADLAR);
	}

	static uint8_t GetSample()
	{
		return ADCL;
	}
};

class FullScaleAvrAdc : public BaseAvrAdc
{
public:
	static const uint16_t AdcMax = 0x3FF;

public:
	static void SetScale()
	{
		ADMUX &= ~bit(ADLAR);
	}

	static uint16_t GetSample()
	{
		return ADC;
	}

	static void SetChannel(const uint8_t channel)
	{
		ADMUX &= ~0x0F;
		ADMUX |= channel;
	}
};
#endif
#endif