/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/moduleinfo.h>

#include <psplib/pl_psp.h>
#include <psplib/pl_snd.h>
#include <psplib/image.h>
#include <psplib/video.h>
#include <psplib/pl_perf.h>
#include <psplib/pl_file.h>
#include <psplib/ctrl.h>
#include <psplib/pl_ini.h>
#include <psplib/pl_rewind.h>
#include <psplib/pl_util.h>
#include <psplib/ui.h>

PSP2_MODULE_INFO(0, 0, "vita2dsample");

static PspImage *Background;

int main()
{
  /* Initialize PSP */
  //pl_psp_init(argv[0]);
  printf("RUNNING");
  pl_psp_init("cache0:/VitaDefilerClient/Documents/");
  printf("PSP_INIT");
  pl_snd_init(512, 0);
  printf("SND_INIT");
  pspCtrlInit();
  printf("CTRL_INIT");
  pspVideoInit();
  printf("VIDEO_INIT");

  printf("START_CALLBACK");
	pl_file_path background;
	snprintf(background, sizeof(background) - 1, "%sbackground.png",
					pl_psp_get_app_directory());
	Background = pspImageLoadPng(background);
	printf("%p",background);

	SceCtrlData pad;
	memset(&pad, 0, sizeof(pad));

	while (1) {


		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons & PSP2_CTRL_START) break;

		pspVideoBegin();
	  pspVideoClearScreen();
		pspVideoPutImage(Background, 0, 0,
			SCR_WIDTH, SCR_HEIGHT);
    pspVideoFillRect(0,0,10, 13,PSP_COLOR_GREEN);

    pspVideoDrawLine(0,0,SCR_WIDTH, SCR_HEIGHT,PSP_COLOR_GREEN);

    pspVideoFillRect(100,100,SCR_WIDTH, SCR_HEIGHT,PSP_COLOR_GREEN);

    pspVideoPrint(&PspStockFont, 0, 10, "W",PSP_COLOR_BLUE);

    vita2d_draw_line(480,272,480,272,PSP_COLOR_YELLOW);

    pspVideoEnd();

		/* Swap buffers */
		pspVideoWaitVSync();
		pspVideoSwapBuffers();
	}

	/* Release PSP resources */
	pl_snd_shutdown();
	pspVideoShutdown();
	pl_psp_shutdown();

	return(0);

}
