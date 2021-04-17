
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <pspmoduleinfo.h>
#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <psputility_netmodules.h>
#include <pspctrl.h>
#include <pspsdk.h>

/* Define the module info section */
PSP_MODULE_INFO("MacFixer", 0, 1, 00);

/* Define printf, just to make typing easier */
#define printf  pspDebugScreenPrintf

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();

	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
    int cbid;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
	sceKernelStartThread(thid, 0, 0);
    return thid;
}

int AdhocInit()
{
	struct productStruct product;

	strcpy(product.product, "ULUS99999");
	product.unknown = 0;

    u32 err;
	pspDebugScreenPrintf("sceNetInit()\n");
    err = sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000);
    if (err != 0)
        return err;

	pspDebugScreenPrintf("sceNetAdhocInit()\n");
    err = sceNetAdhocInit();
    if (err != 0)
        return err;
	
	pspDebugScreenPrintf("sceNetAdhocctlInit()\n");
    err = sceNetAdhocctlInit(0x2000, 0x20, &product);
    if (err != 0)
        return err;

    // Connect
    err = sceNetAdhocctlConnect("");
    if (err != 0)
        return err;

    int stateLast = -1;

    while (1)
    {
        int state;
        err = sceNetAdhocctlGetState(&state);
        if (err != 0)
        {
            pspDebugScreenPrintf("sceNetApctlGetState returns $%x\n", err);
            sceKernelDelayThread(10*1000000); // 10sec to read before exit
			return -1;
        }
        if (state > stateLast)
        {
            stateLast = state;
        }
        if (state == 1)
            break;  // connected

        // wait a little before polling again
        sceKernelDelayThread(50*1000); // 50ms
    }

	// All the init functions have passed
	return 0;
}

int AdhocTerm()
{
    u32 err;
	
	err = sceNetAdhocctlTerm();
	if(err != 0)
	{
		printf("sceNetAdhocctlTerm returned %x\n", err);
	}

	err = sceNetAdhocTerm();
	if(err != 0)

	{
		printf("sceNetAdhocTerm returned %x\n", err);
	}

	err = sceNetTerm();
	if(err != 0)
	{
		printf("sceNetTerm returned %x\n", err);
	}

    return 0; // assume it worked
}

/* main routine */
int main(int argc, char *argv[])
{
	int rc;
	char mac[512];
	char current[512];
	char tempStr[40];

    //init screen and callbacks
    pspDebugScreenInit();
    pspDebugScreenClear();
    
    SetupCallbacks();
    
	rc = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	rc = sceUtilityLoadNetModule(PSP_NET_MODULE_ADHOC);

	AdhocInit();

	memset(mac, 0, 512);

	sceNetGetLocalEtherAddr(mac);

	AdhocTerm();

    SceUID mod = pspSdkLoadStartModule("idstorage.prx", PSP_MEMORY_PARTITION_KERNEL);
    if (mod < 0)
    {
        printf(" Error 0x%08X loading/starting idstorage.prx.\n", mod);
        sceKernelDelayThread(5*1000*1000);
        sceKernelExitGame();
    }

	// Ignore errors reading, and assume it is corrupt
	ReadKey(0x44, current);

	if(memcmp(mac, current, 512) == 0)
	{
		printf("MAC address is set correct\n");
	}
	else
	{		
		if(WriteKey(0x44, mac) < 0)
		{
			printf("Error writting updated MAC address\n");
		}
		else
		{
			printf("Updated the MAC address to match the WLAN module\n");
		}
	}

    sceKernelSleepThread();
    sceKernelExitGame();

    return 0;
}
