#ifndef __PS2SDK_1_1__
#include <stdio.h>
#endif

#include <string.h>
#include <string>
#include <kernel.h>
#include <delaythread.h>
#include <sifrpc.h>
#include <loadfile.h>

// RPC server ids
#define PS2_RPC_FILEIO 0x80000001
#define PS2_RPC_AUDSRV 0x870884e
#define PS2_RPC_PAD_NEW 0x80000100
#define PS2_RPC_PAD_OLD 0x80000101

/**
 * @brief Check if an RPC server is already registered on the IOP.
 *
 * Used to avoid loading a second copy of a module the launcher (ps2link,
 * wLaunchELF, ...) or the BIOS already provided.
 */
static bool PS2_rpc_available(unsigned int sid)
{
    SifRpcClientData_t client;
    memset(&client, 0, sizeof(client));
    if (SifBindRpc(&client, sid, 0) < 0) {
        return false;
    }
    for (int i = 0; i < 20 && client.server == NULL; i++) {
        DelayThread(10000); // 10ms, up to 200ms
    }
    return client.server != NULL;
}

/**
 * @brief Load an IOP module shipped next to the game data.
 *
 * Modules are loaded from the game directory (host:, mass:, cdfs:, ...)
 * instead of being embedded in the ELF: modules embedded in EE RAM and
 * loaded with SifExecModuleBuffer() are accepted by the IOP but never
 * register their RPC servers with this SDK, while SifLoadModule() works.
 */
static int PS2_load_module(const char *name)
{
    std::string path = FILEPATH + "modules/" + name;
    return SifLoadModule(path.c_str(), 0, NULL);
}


//#include <tamtypes.h>
//#include <kernel.h>
//#include <sifrpc.h>
//#include <loadfile.h>
//#include <fileio.h>
//#include <stdlib.h>
//#include <libmc.h>
//#include <stdio.h>
//#include <string.h>


//#include "cdvd_rpc.h"

//Declare usbd module //
//extern unsigned char usbd[]; /**< TODO */
//extern unsigned int size_usbd; /**< TODO */
//Declare usbhdfsd module //
//extern unsigned char usbhdfsd[]; /**< TODO */
//extern unsigned int size_usbhdfsd; /**< TODO */
// cdfs IRX
//extern unsigned char cdvd[]; /**< TODO */
//extern unsigned int size_cdvd; /**< TODO */

// adds support for basic I/O such as MC
#if 0
void PS2loadMinimal() {
	int ret;
	SifLoadModule("rom0:SIO2MAN", 0, NULL);
	ret = SifLoadModule("rom0:MCMAN", 0, NULL);
	#ifdef DEBUG_PS2
		if (ret < 0) {
			printf("Error '%d' loading module rom0:MCMAN\n", ret);
		} else {
			printf("Module rom0:MCMAN loaded\n");
		}
	#endif
	ret = SifLoadModule("rom0:MCSERV", 0, NULL);
	ret = SifLoadModule("rom0:PADMAN", 0, NULL);

	//mcInit(MC_TYPE_MC);
}
#endif

// usb mass support
/**
 * @brief
 *
 */
void PS2_load_USB()
{
    //int ret;
    //SifExecModuleBuffer(usbd, size_usbd, 0, NULL, &ret);
    //SifExecModuleBuffer(usbhdfsd, size_usbhdfsd, 0, NULL, &ret);
}


void PS2_load_CDROM()
{
    //int ret;
    //SifExecModuleBuffer(cdvd, size_cdvd, 0, NULL, &ret);
    //// @IURI: was crashing, removed it fow now
    //cdInit(CDVD_INIT_INIT);
    //CDVD_Init();
}

void PS2_load_MC()
{
    /*
    int ret = 0;

    ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL);
    if (ret < 0) {
        std::cout << "Failed to load module: XSIO2MAN" << std::endl; std::fflush(stdout);
        exit(-1);
    } else {
        std::cout << "XSIO2MAN loaded" << std::endl; std::fflush(stdout);
    }

    ret = SifLoadModule("rom0:XMCMAN", 0, NULL);
    if (ret < 0) {
        std::cout << "Failed to load module: XMCMAN" << std::endl; std::fflush(stdout);
        exit(-1);
    } else {
        std::cout << "XMCMAN loaded" << std::endl; std::fflush(stdout);
    }
    ret = SifLoadModule("rom0:XMCSERV", 0, NULL);
    if (ret < 0) {
        std::cout << "Failed to load module: XMCSERV" << std::endl;
    } else {
        std::cout << "XMCSERV loaded" << std::endl;
    }


    int mc_Type, mc_Free, mc_Format;
    // Since this is the first call, -1 should be returned.
    mcGetInfo(0, 0, &mc_Type, &mc_Free, &mc_Format);
    mcSync(0, NULL, &ret);
    printf("mcGetInfo returned %d\n",ret);
    printf("Type: %d Free: %d Format: %d\n\n", mc_Type, mc_Free, mc_Format);
    */

    /*
    // Assuming that the same memory card is connected, this should return 0
    mcGetInfo(0,0,&mc_Type,&mc_Free,&mc_Format);
    mcSync(0, NULL, &ret);
    printf("mcGetInfo returned %d\n",ret);
    printf("Type: %d Free: %d Format: %d\n\n", mc_Type, mc_Free, mc_Format);
    if (ret != 0) {
        printf("mc0 trouble... should save to other device... To implement\n");  /// TBD
    }
    */


}


void PS2_init() {
    /*
    #ifndef PS2LINK
    SifIopReset(NULL, 0); // clean previous loading of irx by apps like ulaunchElf. Comment this line to get cout on ps2link
    #endif

    // SP193: Being creative (Do something while waiting for the slow IOP to be reset). =D
    int main_id = GetThreadId();
    ChangeThreadPriority(main_id, 72);

    #ifndef PS2LINK
    while(SifIopSync()) {
        //std::cout << "PS2.SifIopSync(): Waiting Iop Sync" << std::endl;
    }
    #endif
    // Initialize and connect to all SIF services on the IOP.
    SifInitRpc(0);
    SifInitIopHeap();
    SifLoadFileInit();
    //fioInit();

    // Apply the SBV LMB patch to allow modules to be loaded from a buffer in EE RAM.
    sbv_patch_enable_lmb();
    */

    SifInitRpc(0);
}

/**
 * @brief Load the IOP modules the game needs, from FILEPATH/modules/.
 *
 * Called after FILEPATH has been resolved to its final device (host:, mass:,
 * cdfs:, ...).
 */
void PS2_load_modules() {
    // newlib's file I/O goes through the "fileio" RPC server. ps2link and
    // similar launchers already provide it; when booting the ELF directly
    // (e.g. PCSX2, or a disc/USB launcher) we have to load it ourselves.
    if (!PS2_rpc_available(PS2_RPC_FILEIO)) {
        PS2_load_module("fileio.irx");
    }

    // stdio is imported by padman/freesd/audsrv; harmless if already loaded.
    PS2_load_module("stdio.irx");

    // Controller: sio2man + padman. Skip when the launcher/BIOS already
    // registered the pad RPC server.
    if (!PS2_rpc_available(PS2_RPC_PAD_NEW) && !PS2_rpc_available(PS2_RPC_PAD_OLD)) {
        PS2_load_module("sio2man.irx");
        PS2_load_module("padman.irx");
    }

    // Audio: audsrv_init() spins forever when its RPC server is missing, and
    // the game calls it during boot. audsrv imports cdvdman and libsd; the
    // ps2sdk freesd.irx provides libsd.
    if (!PS2_rpc_available(PS2_RPC_AUDSRV)) {
        SifLoadModule("rom0:CDVDMAN", 0, NULL);
        PS2_load_module("freesd.irx");
        PS2_load_module("audsrv.irx");
    }

    // Memory card services for saves.
    SifLoadModule("rom0:MCMAN", 0, NULL);
    SifLoadModule("rom0:MCSERV", 0, NULL);
}


