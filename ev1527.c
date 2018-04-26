#include "ev1527.h"


#define 	REMOTE   GPIO_SetBits(GPIOB, GPIO_Pin_4); 
#define  	RDATA 	 GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)		

int daaress= 0;
char daaress_tmp[12];
int key_daaress = 0;
int key_num = 0;

uint32_t  key_value = 0;
uint32_t  key_value2 = 0;
uint8_t 	RmtSta=0;	  	  
uint32_t 	Dval;		
uint32_t 	Dval2;		
int 			RmtRec=0;	
uint8_t  	num = 0;

uint8_t 	Key_State = 1; 
uint8_t   Key_Flag	= 0; //是否有按键按下标志位


void 	EV1527_Init(void)    			  
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;  
	
	//射频模块使能端口RF_SDN
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);		 
	

	
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				
	 GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);
 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); 		
		
	TIM_TimeBaseStructure.TIM_Period = 50000; 
	TIM_TimeBaseStructure.TIM_Prescaler =(16-1);	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;	
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;  
	TIM_ICInitStructure.TIM_ICFilter = 0x03;
	TIM_ICInit(TIM2, &TIM_ICInitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 14;	//高于系统滴答时钟
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);  
	
	//TIM_ITConfig(TIM2, TIM_IT_Update|TIM_IT_CC1,ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_CC1,ENABLE);
	
	TIM_Cmd(TIM2,ENABLE );
	EV1527_Enable();
	Key_State = 1; //默认状态位
}





void TIM2_IRQHandler(void)
{ 		    	 	
	if(TIM_GetITStatus(TIM2,TIM_IT_CC1)!=RESET)
	{ 
		if(RDATA)
		{
  		TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Falling);						
			Dval2=TIM_GetCapture1(TIM2);					
			TIM_SetCounter(TIM2,0);							
			RmtSta|=0X10;							
		}
		else 
		{
			Dval=TIM_GetCapture1(TIM2);					
			TIM_SetCounter(TIM2,0);							
			TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Rising);				
			RmtSta&=~(1<<4);
		}	
		if(RmtSta&0X10)							
		{
			if(RmtSta&0X80)
			{
				if(Dval2>250&&Dval2<600&&Dval>600&&Dval<2000&&Dval2<2000)			
				//if(Dval2>200&&Dval2<350&&Dval>775&&Dval<975&&Dval2<975)	
				{
					RmtRec<<=1;					
					RmtRec|=1;							
					num ++;

				}
				else if(Dval>250&&Dval<600&&Dval2>600&&Dval<2000&&Dval2<2000)	
				//else if(Dval>250&&Dval<400&&Dval2>715&&Dval<915&&Dval2<915)
				{
					RmtRec<<=1;					
					RmtRec|=0;					
					num ++;

				}
					else if(Dval>1200||Dval2>1200)	
				//else if(Dval>1000||Dval2>1000)	
				{
					num = 0;
					RmtRec = 0;
					RmtSta&=0XF0;				
					RmtSta&=~(1<<7);//RmtSta|=0<<7;					
				}
			}
			else 
			{
			if(Dval2>6000&&Dval2<20000)
			//	if(Dval2>8700&&Dval2<8850)		
				{
					RmtSta|=1<<7;					
					RmtRec = 0;
					num = 0;
				} 	
	
		}
		
		if(num == 24)
		{
			key_rmote();
		}

	}	
}

TIM_ClearITPendingBit(TIM2,TIM_IT_Update|TIM_IT_CC1);
TIM2->SR =	0;
}



uint8_t key_rmote(void)
{
	daaress =	RmtRec>>4;
	key_daaress = daaress;
	key_num = RmtRec;
	key_num &= 0x0000000F; 
	
	RmtRec = 0;

	//u2_printf("key_daaress = %d\n",key_daaress);
	//u2_printf("key_num = %d\n",key_num);
	
	if((key_daaress == 82822))
	{
			key_daaress = 0;
			Key_Flag = 1;
			if(key_num == 1)    // I
			{
				key_num = 0;
				Key_State = 0;
				//close_lock_control();
			}
			
			if(key_num == 4)
			{
				key_num = 0;
				Key_State = 1;
				//open_lock_control();  //-o
			}	
		}
	
	return 1;
}

/*
****************************************************
*Function: Get_KeyState
*
*
*Describtion: 获取红外按键状态
*
*Parameter: NONE
*
*Return: 按键值，当没有按键按下时返回0xff
*					
*Author:	RenBin
*
*Date: 2018.3.8
****************************************************
*/
uint8_t Get_KeyState(void)
{
	if(Key_Flag)
	{
		Key_Flag = 0;
		return Key_State;
	}
	else
	{
		return 0xff;
	}
	
	
}
