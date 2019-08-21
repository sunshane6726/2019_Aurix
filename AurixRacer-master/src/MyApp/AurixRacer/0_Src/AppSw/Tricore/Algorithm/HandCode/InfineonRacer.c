/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "InfineonRacer.h"
#include "Basic.h"

#define DELTA 50
#define LEFTDOT 1
#define RIGHTDOT 2
#define NODOTLINE 3
#define MOVINGAVG 6
#define FIXDIST 2000

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
boolean CrosswalkFlag;
boolean EnableCrossL;
boolean EnableCrossR;
int Idx_LMAX, Idx_RMAX;
InfineonRacer_t IR_Ctrl  /**< \brief  global data */
      /*={64, 64, FALSE  }*/;
/******************************************************************* ***********/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void InfineonRacer_init(void){
	IR_Ctrl.SpeedFlag = TRUE;
	IR_Ctrl.CrossFlag = FALSE;
	IR_Ctrl.LeftDotFlag = FALSE;
	IR_Ctrl.RightDotFlag = FALSE;
	IR_Ctrl.DotObstaclFlag = NODOTLINE;
	IR_Ctrl.PositionFix = FALSE;
	IR_Ctrl.LeftAvg = 0;
	IR_Ctrl.RightAvg = 0;
	IR_Ctrl.conv = 0;
	IR_Ctrl.CrossReadCnt = 0;
	IR_Ctrl.GolVol = 0.0;
	IR_Ctrl.LDot = 0;
	IR_Ctrl.RDot = 0;
	IR_Ctrl.Dist = 0;
	CrosswalkFlag = FALSE;
	EnableCrossL = FALSE;
	EnableCrossR = FALSE;

	IR_Ctrl.SlowTest = FALSE;
}

boolean CrossFind(void){
	int i,pos=0,neg=0,conv=0;
	int avg=0;

	boolean HFlag = FALSE, LFlag = FALSE;

	IR_Ctrl.conv = 0;

	for(i=0;i<190;i++){
		avg += IR_Ctrl.DLine[i];
	}
	avg /= 190;

	for(i=0;i<190;i++){
		if((avg - (int)IR_Ctrl.DLine[i]) > DELTA){
			LFlag = TRUE;
		}
		if(LFlag && (((int)IR_Ctrl.DLine[i] - avg) > DELTA)){
			HFlag = TRUE;
		}

		if(LFlag && HFlag){
			IR_Ctrl.conv++;
			LFlag = FALSE;
			HFlag = FALSE;
		}
	}

	if(IR_Ctrl.conv>=3){
		return TRUE;
	}
	else
		return FALSE;
}


//TRUE = Left / FALSE = Right
void LineDataGather(void){
	int i,j;
	int Lavg=0,Ravg=0;
	int mAvg;

	mAvg = (int)(MOVINGAVG/2);

	for(i=LINESIZE-1;i>0;i--){
		IR_Ctrl.LeftLine[i] = IR_Ctrl.LeftLine[i-1];
		IR_Ctrl.RightLine[i] = IR_Ctrl.RightLine[i-1];
	}
	IR_Ctrl.LeftLine[0] = 0;
	IR_Ctrl.RightLine[0] = 0;
	for(i=0;i<50;i++){
		IR_Ctrl.LeftLine[0] += (int)IR_Ctrl.SumLine[i];
		IR_Ctrl.RightLine[0] += (int)IR_Ctrl.SumLine[200-i];
	}

	for(i=mAvg;i<LINESIZE-mAvg;i++){
		Lavg =0 ;
		Ravg = 0;
		for(j=-mAvg;j<mAvg;j++){
			Lavg += IR_Ctrl.LeftLine[i+j];
			Ravg += IR_Ctrl.RightLine[i+j];
		}
		IR_Ctrl.LeftLine[i] = Lavg/MOVINGAVG;
		IR_Ctrl.RightLine[i] = Ravg/MOVINGAVG;
	}
	for(i=0;i<mAvg;i++){
		IR_Ctrl.LeftLine[i] = IR_Ctrl.LeftLine[mAvg];
		IR_Ctrl.RightLine[i] = IR_Ctrl.RightLine[mAvg];
		IR_Ctrl.LeftLine[LINESIZE-1-i] = IR_Ctrl.LeftLine[LINESIZE-1-mAvg];
		IR_Ctrl.RightLine[LINESIZE-1-i] = IR_Ctrl.RightLine[LINESIZE-1-mAvg];
	}

	//IR_Ctrl.LeftLine[0] /= 1000;
	//IR_Ctrl.RightLine[0] /= 1000;
}

int DotLine(void){
	int i,LeftAvg=0,RightAvg=0;
	int DotMode = NODOTLINE;
	int DotDelta = 5000;
	int LeftConv = 0, RightConv = 0;
	boolean LeftLowFlag=FALSE, LeftHighFlag=FALSE,RightLowFlag=FALSE, RightHighFlag=FALSE;

	IR_Ctrl.LeftAvg = 0;
	IR_Ctrl.RightAvg = 0;

	for(i=0;i<LINESIZE;i++){
		IR_Ctrl.LeftAvg += IR_Ctrl.LeftLine[i];
		IR_Ctrl.RightAvg += IR_Ctrl.RightLine[i];
		LeftAvg += IR_Ctrl.LeftLine[i];
		RightAvg += IR_Ctrl.RightLine[i];
	}
	IR_Ctrl.LeftAvg /= LINESIZE;
	IR_Ctrl.RightAvg /= LINESIZE;
	LeftAvg /= LINESIZE;
	RightAvg /= LINESIZE;

	//IR_Ctrl.LeftAvg = IR_Ctrl.LeftLine[0];

	for(i=0;i<LINESIZE;i++){
		if((LeftAvg - IR_Ctrl.LeftLine[i]) > DotDelta){
			LeftLowFlag = TRUE;
		}
		if(LeftLowFlag && ((IR_Ctrl.LeftLine[i] - LeftAvg) > DotDelta)){
			LeftHighFlag = TRUE;
		}

		if(LeftLowFlag && LeftHighFlag){
			LeftConv++;
		}


		if((RightAvg - IR_Ctrl.RightLine[i]) > DotDelta){
			RightLowFlag = TRUE;
		}
		if(RightLowFlag && ((IR_Ctrl.RightLine[i] - RightAvg) > DotDelta)){
			RightHighFlag = TRUE;
		}

		if(RightLowFlag && RightHighFlag){
			RightConv++;
		}
	}

	if(LeftConv > 3) DotMode = LEFTDOT;
	if(RightConv > 3) DotMode = RIGHTDOT;

	return DotMode;
}


void InfineonRacer_detectLane(void){
	int i,j,CrossL = 0, CrossR=0;
	//int L1=0,L2=0,LT=0,R1=0,R2=0,RT=0,W1=0,W2=0,WT=0;
	//int LThr,RThr;
	//boolean LLineflag = TRUE;
	//boolean RLineflag = TRUE;
	boolean LFlag = TRUE;
	int LDot =0,RDot=0;

	//boolean RFlag = TRUE;

	IR_Ctrl.L1=0;
	IR_Ctrl.R1=0;
	IR_Ctrl.W1=0;
	IR_Ctrl.L2=0;
	IR_Ctrl.R2=0;
	IR_Ctrl.W2=0;
	IR_Ctrl.LT=0;
	IR_Ctrl.RT=0;
	IR_Ctrl.WT=0;

	for(i=0 ; i<200 ; i++){
		if(i<100){
	    	IR_Ctrl.SumLine[i]=4096-IR_LineScan.adcResult[0][i+6];
		}
	    else
	    	IR_Ctrl.SumLine[i]=4096-IR_LineScan.adcResult[1][i-88];
	}

	/////카메라 필터.
	for(i=2;i<198;i++){
		IR_Ctrl.FLine[i-2] =(((float32)IR_Ctrl.SumLine[i-2]*0.2 + (float32)IR_Ctrl.SumLine[i-1]*0.6 + (float32)IR_Ctrl.SumLine[i]*1.0 + (float32)IR_Ctrl.SumLine[i+1]*0.6 + (float32)IR_Ctrl.SumLine[i+2]*0.2)/2.6);
		IR_Ctrl.FLine[i-2] =(IR_Ctrl.FLine[i-2]*2)-4096;
		//IR_Ctrl.DLine[i-2] = (((float32)IR_Ctrl.SumLine[i-2] + (float32)IR_Ctrl.SumLine[i-1] + (float32)IR_Ctrl.SumLine[i] + (float32)IR_Ctrl.SumLine[i+1] + (float32)IR_Ctrl.SumLine[i+2])/5);

		if(IR_Ctrl.FLine[i-2]<0 || IR_Ctrl.FLine[i-2]>4096) IR_Ctrl.FLine[i-2] = 0;
		else	IR_Ctrl.FLine[i-2] = 1;
		//IR_Ctrl.DLine[i-2] = IR_Ctrl.SumLine[i]/200;

		/*if((i-2)%5 == 0){
			IR_Ctrl.SmalLine[(i-2)/5] = IR_Ctrl.SumLine[i];
		}*/
	}

	//DataSmothing//Moving AVG 취한 카메라
	for(i=5;i<195;i++){
		int data=0;
		for(j=0;j<10;j++){
			data += IR_Ctrl.SumLine[j+i-5];
		}
		IR_Ctrl.DLine[i-5] = data/10;
	}


	//가장 굵은 검은선 2개 검출 L1R1 / L2R2 선이 없을경우 양 끝을 선으로 고려
	for(i=0;i<196;i++){
		if(i<98)
			CrossL += IR_Ctrl.FLine[i];
		else
			CrossR += IR_Ctrl.FLine[i];
		if(IR_Ctrl.FLine[i] == 1 && LFlag ){//&& i - IR_Ctrl.R2 > 20){
			IR_Ctrl.LT = i;
			LFlag = FALSE;
		}
		else if(IR_Ctrl.FLine[i] == 0 && LFlag == FALSE){
			IR_Ctrl.RT = i-1;
			IR_Ctrl.WT = IR_Ctrl.RT - IR_Ctrl.LT;
			if(IR_Ctrl.W2 < IR_Ctrl.WT){
				if(IR_Ctrl.W1 < IR_Ctrl.WT){
					IR_Ctrl.W2 = IR_Ctrl.W1;
					IR_Ctrl.L1 = IR_Ctrl.L2;
					IR_Ctrl.R1 = IR_Ctrl.R2;
					IR_Ctrl.W1 = IR_Ctrl.WT;
				}
				else{
					IR_Ctrl.W2 = IR_Ctrl.WT;
				}
				IR_Ctrl.L2 = IR_Ctrl.LT;
				IR_Ctrl.R2 = IR_Ctrl.RT;
			}
			LFlag = TRUE;
		}
	}

	if(IR_Ctrl.FLine[195] == 1 && LFlag == FALSE){
		IR_Ctrl.RT = 195;
		IR_Ctrl.WT = IR_Ctrl.RT - IR_Ctrl.LT;
		if(IR_Ctrl.W2 < IR_Ctrl.WT){
			if(IR_Ctrl.W1 < IR_Ctrl.WT){
				IR_Ctrl.W2 = IR_Ctrl.W1;
				IR_Ctrl.L1 = IR_Ctrl.L2;
				IR_Ctrl.R1 = IR_Ctrl.R2;
				IR_Ctrl.W1 = IR_Ctrl.WT;
			}
			else{
				IR_Ctrl.W2 = IR_Ctrl.WT;
			}
			IR_Ctrl.L2 = IR_Ctrl.LT;
			IR_Ctrl.R2 = IR_Ctrl.RT;
		}
		LFlag = TRUE;
	}

	if(IR_Ctrl.R1 == 0){
		if((IR_Ctrl.RT+IR_Ctrl.L2)/2 < 195/2){
			IR_Ctrl.L1 = IR_Ctrl.L2;
			IR_Ctrl.R1 = IR_Ctrl.R2;
			IR_Ctrl.L2 = 195;
			IR_Ctrl.R2 = 195;
		}
	}



	/*
	 *             점선 검출 & 장애물회피
	 */

	if(IR_Ctrl.SpeedFlag == FALSE){
		if((IR_InfraRed.LowObstacleFlag == FALSE) && (IR_Ctrl.DotObstaclFlag == NODOTLINE)){
			if(IR_Ctrl.R1<10){
				if(IR_Ctrl.LeftDotFlag == FALSE){
					IR_Ctrl.LDot++;
					IR_Ctrl.LeftDotFlag = TRUE;
				}
				IR_Ctrl.R1 = 10;
				//IR_Ctrl.RightDotFlag = FALSE;
			}
			else{
				IR_Ctrl.LeftDotFlag = FALSE;
			}

			if(IR_Ctrl.L2>185){
				if(IR_Ctrl.RightDotFlag == FALSE){
					IR_Ctrl.RDot++;
					IR_Ctrl.RightDotFlag = TRUE;
				}
				IR_Ctrl.L2 = 185;
				//IR_Ctrl.LeftDotFlag = FALSE;
			}
			else{
				IR_Ctrl.RightDotFlag = FALSE;
			}
		}

		else if(IR_Ctrl.DotObstaclFlag == NODOTLINE){
			if(IR_Ctrl.LDot>IR_Ctrl.RDot){
				IR_Ctrl.rad = -0.4;
				//IR_setSrvAngle(IR_Ctrl.rad);
				IR_Ctrl.DotObstaclFlag = LEFTDOT;
				//LDot = 0;
				//RDot = 0;

			}
			if(IR_Ctrl.LDot<IR_Ctrl.RDot){
				IR_Ctrl.rad = 0.6;
				//IR_setSrvAngle(IR_Ctrl.rad);
				IR_Ctrl.DotObstaclFlag = RIGHTDOT;
				//LDot = 0;
				//RDot = 0;

			}
			IR_Ctrl.Dist = -(2*IR_Encoder.speed);
		}
	}
///////////////////////////////////////////////////////////////


/////////////////////////////////////////////
/*	if(IR_Ctrl.SpeedFlag == FALSE && IR_InfraRed.ObstacleFlag == TRUE){
		//IR_Ctrl.SlowTest = TRUE;
		IR_Ctrl.DotObstaclFlag = LEFTDOT;
		IR_Ctrl.Dist = -(2 * IR_Encoder.speed);
	}*/


	//////////////////////////////////////////////////// 장애물 회피시 무시 rad 값 수정 안함
	if((CrossFind() == FALSE) && (IR_Ctrl.DotObstaclFlag == NODOTLINE)){
		IR_Ctrl.mid = (float32) (IR_Ctrl.L2+IR_Ctrl.R1)/2;
		IR_Ctrl.rad = (IR_Ctrl.mid-97.5) /43.5;
	}

	////////////////////////////////////////////////////////

	if((IR_Ctrl.SpeedFlag == FALSE)&&(IR_Ctrl.DotObstaclFlag != NODOTLINE) && (IR_Ctrl.PositionFix == FALSE)){
		if(IR_Ctrl.DotObstaclFlag == LEFTDOT){
			IR_Ctrl.rad = -0.4;
		}
		if(IR_Ctrl.DotObstaclFlag == RIGHTDOT){
			IR_Ctrl.rad = 0.6;
		}
		IR_Ctrl.Dist += IR_Encoder.speed;
		if(IR_Ctrl.Dist >= FIXDIST){
			IR_Ctrl.Dist = -(2 * IR_Encoder.speed);
			if(IR_Ctrl.DotObstaclFlag == LEFTDOT) IR_Ctrl.DotObstaclFlag = RIGHTDOT;
			else if(IR_Ctrl.DotObstaclFlag == RIGHTDOT) IR_Ctrl.DotObstaclFlag = LEFTDOT;

			IR_Ctrl.PositionFix = TRUE;
		}
	}

	if(IR_Ctrl.PositionFix == TRUE){
		if(IR_Ctrl.DotObstaclFlag == LEFTDOT){
			IR_Ctrl.rad = -0.4;
		}
		if(IR_Ctrl.DotObstaclFlag == RIGHTDOT){
			IR_Ctrl.rad = 0.6;
		}
		IR_Ctrl.Dist += IR_Encoder.speed;

		if(IR_Ctrl.Dist >= FIXDIST){
			IR_Ctrl.Dist = 0;
			IR_Ctrl.PositionFix = FALSE;
			IR_Ctrl.DotObstaclFlag = NODOTLINE;
			IR_Ctrl.LDot = 0;
			IR_Ctrl.RDot = 0;
		}
	}


/*
	if(CrossL < 49 && CrossR < 49){
		IR_Ctrl.mid = (float32) (IR_Ctrl.L2+IR_Ctrl.R1)/2;
	//    a = ((mid - 87.5)/43.5) * 100;
		IR_Ctrl.rad = (IR_Ctrl.mid-97.5) /43.5;
		IR_Ctrl.CrossFlag = FALSE;
	}
	else if(IR_Ctrl.CrossFlag == FALSE){
		IR_Ctrl.SpeedFlag = !IR_Ctrl.SpeedFlag;
		IR_Ctrl.CrossFlag = TRUE;
	}*/




	///////횡단보도 검출. 20ms * 30프레임(고속) / *80프레임(저속) 단위로 2번 검출시 횡단보도로 인식 최초 검출 후 0.3(고)/0.8(저) 초동안 검출되지 않으면 초기화/////////////////////////////////////////
	if(CrossFind() && (IR_Ctrl.CrossFlag == FALSE) && (IR_Ctrl.DotObstaclFlag == NODOTLINE)){
		IR_Ctrl.CrossReadCnt++;
		if(IR_Ctrl.CrossReadCnt > 2){
			IR_Ctrl.SpeedFlag = !IR_Ctrl.SpeedFlag;
			IR_Ctrl.CrossFlag = TRUE;
			IR_Ctrl.CrossReadCnt= 0;
		}
	}


/*
	if(IR_Ctrl.SpeedFlag == FALSE){
		if(DotLine() == LEFTDOT){
			IR_Ctrl.rad = -1;
		}
		if(DotLine() == RIGHTDOT){
			IR_Ctrl.rad = 1;
		}
	}*/

//	if((IR_Ctrl.SpeedFlag == FALSE) && (CrossFind() == FALSE)) IR_Ctrl.rad *= 2;

	if(IR_Ctrl.rad >= 1.0) IR_Ctrl.rad = 1.0;
	else if(IR_Ctrl.rad <= -1.0) IR_Ctrl.rad = -1.0;

    /*
	if(mid>87)
		rad = (float32)   (mid-87)*((mid-87)+43)/44;
	else
		rad = (float32)   -((mid-87)*(mid-87)/1936);
*/

	//InfineonRacer_control();

	IR_setSrvAngle(IR_Ctrl.rad);
}

void InfineonRacer_control(void){

	//if(IR_InfraRed.ObstacleFlag == TRUE){
	//	IR_Ctrl.rad = -0.4;
	//}
	//IR_setSrvAngle(IR_Ctrl.rad);

}
