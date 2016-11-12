#ifndef __PS2SDK_1_1__
#include <stdio.h>
#endif
#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#include <loadfile.h>
#include <fileio.h>
#include <errno.h>
#include <iopheap.h>
#include <debug.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <libmc.h>
#include <libcdvd.h>
#include <fileXio_rpc.h>

#include "cdvd_rpc.h"

//Declare usbd module //
extern unsigned char usbd[]; /**< TODO */
extern unsigned int size_usbd; /**< TODO */
//Declare usbhdfsd module //
extern unsigned char usbhdfsd[]; /**< TODO */
extern unsigned int size_usbhdfsd; /**< TODO */
// cdfs IRX
extern unsigned char cdvd[]; /**< TODO */
extern unsigned int size_cdvd; /**< TODO */

//Declare fileXio module //
extern unsigned char fileXio[]; /**< TODO */
extern unsigned int size_fileXio; /**< TODO */


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
	int ret;
	SifExecModuleBuffer(usbd, size_usbd, 0, NULL, &ret);
	SifExecModuleBuffer(usbhdfsd, size_usbhdfsd, 0, NULL, &ret);
}


void PS2_load_CDROM()
{
    int ret;
    SifExecModuleBuffer(cdvd, size_cdvd, 0, NULL, &ret);
    // @IURI: was crashing, removed it fow now
    cdInit(CDVD_INIT_INIT);
    CDVD_Init();
}

void PS2_load_MC()
{
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


void PS2_load_xio() {
    int ret;

    /*
    std::cout << ">>>>>>>>> PREPARE TO INIT fileXio..." << std::endl;
    ret = SifExecModuleBuffer(fileXio, size_fileXio, 0, NULL, &ret);
    if (ret < 0) {
        std::cout << ">>>>>>>>> LOAD fileXio FAILED!" << std::endl;
        exit(-1);
    }
    std::cout << ">>>>>>>>> fileXio module loaded, ret: " << ret << ". Calling init..." << std::endl;

    fileXioInit();
    std::cout << ">>>>>>>>> fileXioInit() DONE!" << std::endl;
    */

}

void PS2_init() {
    #ifndef PS2LINK
    SifIopReset(NULL, 0); // clean previous loading of irx by apps like ulaunchElf. Comment this line to get cout on ps2link
    #endif

    printf("DEBUG.PS2 #1.1\n");

    // SP193: Being creative (Do something while waiting for the slow IOP to be reset). =D
    int main_id = GetThreadId();
    ChangeThreadPriority(main_id, 72);
    std::cout << "PS2.DEBUG #1.1" << std::endl; std::fflush(stdout);
    printf("DEBUG.PS2 #1.2\n");


    #ifndef PS2LINK
    while(SifIopSync()) {
        std::cout << "PS2.SifIopSync()" << std::endl;
    }
    #endif
    // Initialize and connect to all SIF services on the IOP.
    SifInitRpc(0);
    SifInitIopHeap();
    SifLoadFileInit();
    fioInit();
    printf("DEBUG.PS2 #1.3\n");

    // Apply the SBV LMB patch to allow modules to be loaded from a buffer in EE RAM.
    sbv_patch_enable_lmb();
}


