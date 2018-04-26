#ifndef _EV1527_H_
#define _EV1527_H_
#include "stm32l1xx.h"

#define 	EV1527_Enable()			GPIO_ResetBits(GPIOC,GPIO_Pin_4)
#define		EV1527_Disable()		GPIO_SetBits(GPIOC,GPIO_Pin_4)
 

	


#define REMOTE_ID 0      		   

extern uint8_t RmtCnt;			
extern int key_num;
extern int key_daaress;

void 	EV1527_Init(void);   
uint8_t key_rmote(void);
uint8_t Get_KeyState(void);

#endif

/********************************************** end of file *********************************************/

