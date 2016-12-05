#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>
#include <conio.h>
#include "ps4000aApi.h"
#include "PS4000Acon.h"
#include "Setting.h"
#include <math.h>
#include <string.h>
#include "ini.h"


HANDLE hCom;
DCB CommDCB;
COMMTIMEOUTS TimeOuts;
#define TESTCASENUMBERMAX 24
#define M_PI 3.14159265358979323846

const double slip[TESTCASENUMBERMAX] = 
{ -1.0000, -0.9000, -0.8000, -0.7000, -0.6000, -0.5000, -0.4000, -0.3000, -0.2000, -0.1000, 
   0.0000,  0.1000,  0.2000,  0.3000,  0.4000,  0.5000,  0.6000,  0.7000,  0.8000,  0.8200,
   0.8400,  0.8600,  0.8800,  0.9000 
};

const double frequency[TESTCASENUMBERMAX] =
{	10.00, 15.00, 20.00, 25.00, 30.00, 35.00, 40.00, 45.00,
	50.00, 55.00, 60.00, 65.00, 70.00, 75.00, 80.00, 85.00,
	90.00, 95.00, 100.00, 105.00, 110.00, 115.00, 120.00,
	125.00
};

const double ratio[TESTCASENUMBERMAX] =
{	0.20, 0.22, 0.24, 0.26, 0.28, 0.30, 0.32, 0.34, 
    0.36, 0.38, 0.4, 0.42, 0.44, 0.46, 0.48, 0.5, 
	0.52, 0.54, 0.56, 0.58, 0.6, 0.62, 0.64, 0.66
};

//const double ratio[TESTCASENUMBERMAX] =
//{	0.54, 0.56, 0.58, 0.6, 0.62, 0.64, 0.66, 0.68, 
//	0.70, 0.72, 0.74, 0.76, 0.78, 0.8
//};



double syncSpeed[TESTCASENUMBERMAX];
double freq[TESTCASENUMBERMAX];
double amplitude[TESTCASENUMBERMAX];
PICO_STATUS picostatus = PICO_OK;
UNIT allUnits;



configuration config;

void errorHandle(char* text)
{
	printf(text);
	for(;;);
}

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if(MATCH("motor", "motorFrequency")) {
		pconfig->motorFrequency =atof(value);
	} else if (MATCH("coil", "targetCurrent")) {
		pconfig->targetCurrent =atoi(value);
	} else if (MATCH("coil", "coilInductance")) {
		pconfig->coilInductance =atof(value);
	} else if (MATCH("coil", "coilResistance")) {
		pconfig->coilResistance =atof(value);
	} else if (MATCH("coil", "maxVoltage")) {
		pconfig->maxVoltage =atof(value);
	} else if (MATCH("testcase", "singleTestCaseCycle")) {
		pconfig->singleTestCaseCycle =atoi(value);
	} else if (MATCH("testcase", "sampleInterval")) {
		pconfig->sampleInterval =atoi(value);
	} else if (MATCH("testcase", "sampleBuffer")) {
		pconfig->sampleBuffer =atoi(value);
	} else if (MATCH("testcase", "sampleNumber")) {
		pconfig->sampleNumber =atoi(value);
	} else if (MATCH("testcase", "defaultAmplitude")) {
		pconfig->defaultAmplitude =(atof(value)<=1?atof(value):1);
	} else if (MATCH("testcase", "defaultFrequency")) {
		pconfig->defaultFrequency = atof(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}



int makeAmp(double d, char *c)
{
	int ret=0;
	c[0] = 'a';
	sprintf(&c[1],"%.2f",d);
	while(d/10.0 > 1.0) {
		ret++;
		d/=10.0;
	}
	return ret+5;
}

int makeFreq(double d, char *c)
{
	int ret=0;
	c[0] = 'f';
	sprintf(&c[1],"%.2f",d);
	while(d/10.0 > 1.0){
		ret++;
		d/=10.0;
	}
	return ret+5;
}

void initParameter(int type)
{
	int i;
	int error;
	if (ini_parse("setup.ini", handler, &config) < 0) {
		errorHandle("Can't load 'setup.ini'\n");
		return;
	}
	if(type == SLIPTEST)
	{
		for(i=0;i<TESTCASENUMBERMAX;i++)	{
			syncSpeed[i] = config.motorFrequency/(1.0-slip[i]);
			freq[i] = 15.0/2.0*syncSpeed[i];
			amplitude[i] = 2.0 * config.targetCurrent * sqrt((config.coilInductance*2.0*M_PI*freq[i])*(config.coilInductance*2.0*M_PI*freq[i])+config.coilResistance*config.coilResistance)/3.0/config.maxVoltage;
			if(amplitude[i]<1.0){
				config.testcaseNumber = i;
			}
		}
	}
	else if(type == FREQUENCYTEST){
		for(i=0;i<TESTCASENUMBERMAX;i++){
			freq[i] = frequency[i];
			amplitude [i] = config.defaultAmplitude;
		}
		config.testcaseNumber = TESTCASENUMBERMAX;
	}
	else if(type == AMPLITUDETEST){
		for(i=0;i<TESTCASENUMBERMAX;i++){
			freq[i] = config.defaultFrequency;
			amplitude [i] = ratio[i];
		}
		config.testcaseNumber = TESTCASENUMBERMAX;
	}

}

int initCOM(void)
{
	hCom = CreateFile(TEXT("\\\\.\\COM20"), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hCom==(HANDLE)-1)
	{
		errorHandle("PORT can not open!");
		return 1;
	}
	else
	{
		printf("COM connected!\n");
		GetCommState( hCom, &CommDCB );
		CommDCB.BaudRate = CBR_115200;
		CommDCB.Parity = NOPARITY;
		CommDCB.StopBits = ONESTOPBIT;
		CommDCB.ByteSize = 8;
		CommDCB.fDtrControl = 0;
		CommDCB.fRtsControl =0;
		SetCommState( hCom, &CommDCB );
		SetupComm( hCom, 1024, 1024 );


		//set read timeout
		TimeOuts.ReadIntervalTimeout=100;
		TimeOuts.ReadTotalTimeoutMultiplier=1;
		TimeOuts.ReadTotalTimeoutConstant=500;
		//set write timeout
		TimeOuts.WriteTotalTimeoutMultiplier=1;
		TimeOuts.WriteTotalTimeoutConstant=500;
		SetCommTimeouts(hCom,&TimeOuts); //set timeout
		return 0;
	}
}



int initPicoScope(void)
{
	int i;
	
	picostatus = OpenDevice(&allUnits,NULL);
	if (picostatus == PICO_OK || picostatus == PICO_USB3_0_DEVICE_NON_USB3_0_PORT)
	{
		allUnits.openStatus = (int16_t) picostatus;
		printf("Found one device, opening...\n\n");
		picostatus = allUnits.openStatus;

		if (picostatus == PICO_OK || picostatus == PICO_POWER_SUPPLY_NOT_CONNECTED || picostatus == PICO_USB3_0_DEVICE_NON_USB3_0_PORT)
		{
			set_info(&allUnits);
			picostatus = HandleDevice(&allUnits);
		}
	}
	else
		errorHandle("Fail to init PicoScope!\n");

	allUnits.channelCount = 8;
	allUnits.channelSettings[0].enabled = FALSE;
	allUnits.channelSettings[1].enabled = TRUE;
	allUnits.channelSettings[2].enabled = TRUE;
	allUnits.channelSettings[3].enabled = TRUE;
	allUnits.channelSettings[4].enabled = FALSE;
	allUnits.channelSettings[5].enabled = TRUE;
	allUnits.channelSettings[6].enabled = TRUE;
	allUnits.channelSettings[7].enabled = TRUE;
	for(i=0;i<8;i++)
	{
		allUnits.channelSettings[i].DCcoupled = TRUE;
		allUnits.channelSettings[i].range = 9;
		allUnits.channelSettings[i].analogueOffset = 0;
	}
	return picostatus;
}

BOOLEAN ReadAllCom(unsigned char *buffer, int len)
{
	DWORD wRead = 1;
	BOOLEAN ret;
	while(wRead)
	{
		ret = ReadFile(hCom,buffer,len,&wRead,NULL);
		if(wRead)
		{
			buffer[len-1]='\0';
			printf("%s\n",buffer);
				}
	}
	PurgeComm(hCom,PURGE_RXCLEAR);

	return ret;
}


void TriggerCallback(void)
{
	DWORD wWrite;
	unsigned char str[64]={0};
	const unsigned char s[5] = "n150";
	int ii;
	
	PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR); 
	for(ii=0;ii<4;ii++)
	{
		WriteFile(hCom,&s[ii],1,&wWrite,NULL);
	}
}

void mainmenu(int* mode)
{
	int ch = '.';
	while (ch != 'X')
	{
		printf("\n");
		printf("Please select an operation:\n\n");

		printf("1 - Slip Test\n");
		printf("2 - Frequency Test\n");
		printf("3 - Amplitude Test\n");

		printf("Operation:");

		ch = toupper(_getch());

		printf("\n\n");

		switch (ch) 
		{
			case '1':
				*mode = SLIPTEST;
				return;

			case '2':
				*mode = FREQUENCYTEST;
				return;

			case '3':
				*mode = AMPLITUDETEST;
				return;
			default:
				printf("Invalid operation\n");
				break;
		}
	}	
}

int main(void)
{
	DWORD wRead = 1;
	DWORD wWrite = 0;
	char inputCmd[32]={0};
	unsigned char str[64]={0};
	int i;
	int mode;

	mainmenu(&mode);
	initParameter(mode);            //read setup.ini and put them into config structure


	if(initCOM())
		errorHandle("Fail to init COM!\n");

	initPicoScope();



	for(g_testCycle = 0;g_testCycle<config.testcaseNumber;g_testCycle++)
	{
		int l,ii;
		//int retry = 3;

		/* wait for pressing button*/
		printf("\n\n======== Test: %d ========\nAmptude: %.2f\nFreq: %.2f\nPress a key to start, 'x' to end\n",g_testCycle+1,amplitude[g_testCycle],freq[g_testCycle]);
	 //   if(_getch()=='x'){
		//	return 0;
		//}

		/* write amplitude */
		PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
		l = makeAmp(amplitude[g_testCycle],inputCmd);
		for(ii=0;ii<l;ii++)
		{
			WriteFile(hCom,&inputCmd[ii],1,&wWrite,NULL);
		}
		ReadAllCom(str, sizeof(str));
		Sleep(100);

		/* write frequency */
		l = makeFreq(freq[g_testCycle],inputCmd);
		for(ii=0;ii<l;ii++)
		{
			WriteFile(hCom,&inputCmd[ii],1,&wWrite,NULL);
		}
		
		ReadAllCom(str, sizeof(str));
		Sleep(100);
		for(g_singleTestCycle=0;g_singleTestCycle<config.singleTestCaseCycle;g_singleTestCycle++)
		{
			printf("<====== Test: %d     Cycle: %d =======>\n",g_testCycle+1,g_singleTestCycle+1);
			CollectStreamingImmediate(&allUnits);
			if(g_redoFlag == 1)
			{
				ps4000aCloseUnit(allUnits.handle);
				while(initPicoScope())
				{
					printf("press any button to reconnect PICOscope.\n");
					_getch();
				}
				
				g_singleTestCycle--;
				g_redoFlag = 0;
			}
			printf("next test case will start after:\n");
			for(i=8;i>0;i--)
			{
				Sleep(1000);
				printf("%d\n",i);
			}
			
		}
	}

	
	return 0;
}
