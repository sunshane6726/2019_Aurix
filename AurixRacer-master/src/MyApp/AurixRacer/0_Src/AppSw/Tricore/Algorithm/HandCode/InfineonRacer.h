#ifndef INFINEONRACER_H_
#define INFINEONRACER_H_


/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include <Ifx_Types.h>
#include "Configuration.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define IR_getLs0Margin()		IR_Ctrl.Ls0Margin
#define IR_getLs1Margin()		IR_Ctrl.Ls1Margin
#define LINESIZE 600


/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/



/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef struct{
	sint32 Ls0Margin;
	sint32 Ls1Margin;
	sint32 CrossReadCnt;
	sint32 RightLine[LINESIZE];
	sint32 LeftLine[LINESIZE];
	sint32 LeftAvg;
	sint32 RightAvg;
	uint32 L1,L2,LT,R1,R2,RT,W1,W2,WT;
	uint32 LDot,RDot;
	float32 mid, rad;
	uint32 SumLine[200];
	uint32 FLine[196];
	uint32 DLine[190];
	uint32 SmalLine[39];
	boolean basicTest;
	boolean SpeedFlag;
	boolean CrossFlag;
	boolean LeftDotFlag;
	boolean RightDotFlag;
	boolean PositionFix;
	uint32 DotObstaclFlag;
	uint32 conv;
	float32 GolVol;
	sint32 Dist;
	boolean SlowTest;
}InfineonRacer_t;

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
IFX_EXTERN InfineonRacer_t IR_Ctrl;

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/
IFX_EXTERN void InfineonRacer_init(void);
IFX_EXTERN void InfineonRacer_detectLane(void);
IFX_EXTERN void InfineonRacer_control(void);

#endif
