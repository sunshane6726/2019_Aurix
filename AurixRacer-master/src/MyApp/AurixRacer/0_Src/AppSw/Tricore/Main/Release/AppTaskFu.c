#include "AppTaskFu.h"

static sint32 task_cnt_1m = 0;
static sint32 task_cnt_10m = 0;
static sint32 task_cnt_100m = 0;
static sint32 task_cnt_1000m = 0;
static sint32 CrossFrame = 0;
static sint32 DotObstaclCnt = 0;

boolean task_flag_1m = FALSE;
boolean task_flag_10m = FALSE;
boolean task_flag_100m = FALSE;
boolean task_flag_1000m = FALSE;

void appTaskfu_init(void){

	BasicLineScan_init();
	BasicPort_init();
    BasicGtmTom_init();
    BasicVadcBgScan_init();
    BasicGpt12Enc_init();
    //AsclinShellInterface_init();

#if BOARD == APPLICATION_KIT_TC237
    tft_app_init(1);
    perf_meas_init();
#elif BOARD == SHIELD_BUDDY

#endif

#if CODE == CODE_HAND
    InfineonRacer_init();


#elif CODE == CODE_ERT
    IR_Controller_initialize();
#else

#endif
}

void appTaskfu_1ms(void)
{
	task_cnt_1m++;
	if(task_cnt_1m == 1000){
		task_cnt_1m = 0;
	}

}


void appTaskfu_10ms(void)
{
	task_cnt_10m++;
	if(task_cnt_10m == 1000){
		task_cnt_10m = 0;
	}

	if(task_cnt_10m%2 == 0){
		BasicLineScan_run();
		InfineonRacer_detectLane();
		InfineonRacer_control();
		BasicGtmTom_run();
		BasicVadcBgScan_run();
		if(IR_Ctrl.basicTest == FALSE){
			#if CODE == CODE_HAND
				//InfineonRacer_control();
			#elif CODE == CODE_ERT
				IR_Controller_step();
			#else

			#endif
		}
		BasicPort_run();
		//AsclinShellInterface_runLineScan();

		//일정 프레임 단위로 횡단보도 검출시 카운트
		if(IR_Ctrl.CrossFlag){
			CrossFrame++;
		}
		if(IR_Ctrl.SpeedFlag && CrossFrame > 30){
			CrossFrame = 0;
			IR_Ctrl.CrossFlag = FALSE;
		}
		else if(IR_Ctrl.SpeedFlag == FALSE && CrossFrame > 80){
			CrossFrame = 0;
			IR_Ctrl.CrossFlag = FALSE;
		}
	}

	/*if(task_cnt_10m%2 == 0){
		LineDataGather();
	}*/

}

void appTaskfu_100ms(void)
{
	task_cnt_100m++;
	/*if(IR_Ctrl.DotObstaclFlag == TRUE){
		DotObstaclCnt++;
		if(DotObstaclCnt>=4){
			DotObstaclCnt=0;
			IR_Ctrl.DotObstaclFlag = FALSE;
			IR_Ctrl.LDot = 0;
			IR_Ctrl.RDot = 0;
		}
	}*/

	if(task_cnt_100m == 1000){
		task_cnt_100m = 0;
	}

	///일정시간 횡단보도 검출 불가 시 초기화
	if((task_cnt_100m %3 == 0) && IR_Ctrl.SpeedFlag){
		IR_Ctrl.CrossReadCnt = 0;
	}
	else if((task_cnt_100m %8 == 0) && (IR_Ctrl.SpeedFlag == FALSE)){
		IR_Ctrl.CrossReadCnt = 0;
	}
#if BOARD == APPLICATION_KIT_TC237
	if(task_cnt_100m % REFRESH_TFT == 0){
		tft_app_run();
	}

#elif BOARD == SHIELD_BUDDY

#endif
}

void appTaskfu_1000ms(void)
{
	task_cnt_1000m++;
	if(task_cnt_1000m == 1000){
		task_cnt_1000m = 0;
	}

}

void appTaskfu_idle(void){
//	AsclinShellInterface_run();
#if BOARD == APPLICATION_KIT_TC237
	perf_meas_idle();
#elif BOARD == SHIELD_BUDDY

#endif

}

void appIsrCb_1ms(void){
	BasicGpt12Enc_run();
}

