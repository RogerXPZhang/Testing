#ifndef _SETTING_H_
#define _SETTING_H_


#define TESTCASENUMBERMAX 24

#define SLIPTEST			1
#define FREQUENCYTEST		2
#define AMPLITUDETEST		3

typedef struct
{
    double motorFrequency;
    int targetCurrent;
	double coilInductance;
	double coilResistance;
	double maxVoltage;
	int singleTestCaseCycle;
	int sampleInterval;
	long sampleBuffer;
    long sampleNumber;
	int testcaseNumber;
	double defaultAmplitude;
	double defaultFrequency;
} configuration;

extern configuration config;
double freq[TESTCASENUMBERMAX];
double amplitude[TESTCASENUMBERMAX];



#endif