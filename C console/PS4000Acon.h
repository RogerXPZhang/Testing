#ifndef _PS4000ACON_H_
#define _PS4000ACON_H_



typedef enum
{
	MODEL_NONE = 0,
	MODEL_PS4824 = 0x12d8,
	MODEL_PS4225 = 0x1081,
	MODEL_PS4425 = 0x1149
} MODEL_TYPE;

typedef enum
{
	SIGGEN_NONE = 0,
	SIGGEN_FUNCTGEN = 1,
	SIGGEN_AWG = 2
} SIGGEN_TYPE;

typedef struct
{
	int16_t DCcoupled;
	int16_t range;
	int16_t enabled;
	float analogueOffset;
}CHANNEL_SETTINGS;

typedef struct
{
	int16_t handle;
	MODEL_TYPE					model;
	int8_t						modelString[8];
	int8_t						serial[10];
	int16_t						complete;
	int16_t						openStatus;
	int16_t						openProgress;
	PS4000A_RANGE				firstRange;
	PS4000A_RANGE				lastRange;
	int16_t						channelCount;
	int16_t						maxADCValue;
	SIGGEN_TYPE					sigGen;
	int16_t						hasETS;
	uint16_t					AWGFileSize;
	CHANNEL_SETTINGS			channelSettings [PS4000A_MAX_CHANNELS];
}UNIT;

extern PICO_STATUS OpenDevice(UNIT *unit, int8_t *serial);
extern void CollectBlockTriggered(UNIT * unit);
extern void SetDefaults(UNIT * unit);
extern void CollectBlockImmediate(UNIT * unit);
extern void set_info(UNIT * unit);
extern PICO_STATUS HandleDevice(UNIT * unit);
extern void CollectStreamingImmediate(UNIT * unit);
extern void CollectStreamingTriggered(UNIT * unit);
extern void StreamDataHandler(UNIT * unit, uint32_t preTrigger);
extern void SetStreamingTrigger(UNIT * unit);
extern void TriggerCallback(void);
extern int g_testCycle;
extern int g_singleTestCycle;
extern int g_redoFlag;









#endif


