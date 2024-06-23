/**
  ******************************************************************************
  * @file    timer.c
  * @author  const-zpc
  * @date    2020-12-1
  * @brief   ���ļ��ṩ����ģ�������ܣ��Թ��� TIMER ���������¹��ܣ�
  *           + timer�ײ���������������
  *           + timer���жϷ�����
  *
  *
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "timer.h"
#include "sys.h"
#include <string.h>

/**
  * @brief ��ע��Ķ�ʱ�ص����������Ŀ
  */
#define SUPPORT_FUN_MAX_NUM      10

/* Private typedef -----------------------------------------------------------*/

/**
  * @brief TIME �����Ϣ�ṹ�嶨��
  */
typedef struct{
    TimeFun pTimeFun;                   /*!< �ص����� */
    
    uint32_t ui1msTic;                  /*!< 1ms��ʱ��ʱ */
    
    uint32_t ui1msMaxTime;              /*!< �ص�����ʱ�� */
} TIME_FunType;

/**
  * @brief TIME �����Ϣ�ṹ�嶨��
  */
typedef struct{
    uint16_t ucSupportCnt;                           /*!< ע��Ķ�ʱ�ص���������Ŀ */
    
    TIME_FunType tCallFun[SUPPORT_FUN_MAX_NUM];     /*!< ע��Ķ�ʱ�ص����� */
    
    uint32_t ui1msTic;                              /*!< 1ms��ʱ */
    
    uint32_t uimsDelayTic;                              /*!< 1ms��ʱ */
} TIME_HandleType;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** ��ʱ��־ */
Tim_TimeSignType g_tTimeSign = {0};

static TIME_HandleType sg_tTIME_Handle = {0};

/* Private function prototypes -----------------------------------------------*/
/* Private function ----------------------------------------------------------*/

/**
  * @brief      PIT CH1 �ĳ�ʼ������
  * @note       ����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
  * @param[in]  arr �Զ���װֵ
  * @param[in]  psc ʱ��Ԥ��Ƶ��
  * @retval     None
  */
void TIME_Init(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


	TIM_Cmd(TIM3, ENABLE);	 
}

/**
  * @brief  PIT CH1 �ĳ�ʼ������.
  * @retval None
  */
void FML_TIME_Init(void)
{
    //100Khz�ļ���Ƶ�ʣ�������100Ϊ1ms  
    TIME_Init(99, 719);
    
    memset(&sg_tTIME_Handle, 0, sizeof(sg_tTIME_Handle));
}

/**
  * @brief  ��ʱ��3�жϷ������
  * @retval None
  */
void TIM3_IRQHandler(void)
{
    uint8_t i;
    
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );
        
        for (i = 0; i < sg_tTIME_Handle.ucSupportCnt && i < SUPPORT_FUN_MAX_NUM; i++)
        {
            sg_tTIME_Handle.tCallFun[i].ui1msTic++;
            
            if (sg_tTIME_Handle.tCallFun[i].ui1msTic >= sg_tTIME_Handle.tCallFun[i].ui1msMaxTime)
            {
                sg_tTIME_Handle.tCallFun[i].ui1msTic = 0;
                
                if (sg_tTIME_Handle.tCallFun[i].pTimeFun != NULL)
                {
                    sg_tTIME_Handle.tCallFun[i].pTimeFun();
                }
            }
        }
        
        sg_tTIME_Handle.uimsDelayTic++;
        sg_tTIME_Handle.ui1msTic++;
        
        sg_tTIME_Handle.ui1msTic % 1 == 0 ? g_tTimeSign.bTic1msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 10 == 0 ? g_tTimeSign.bTic10msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 20 == 0 ? g_tTimeSign.bTic20msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 100 == 0 ? g_tTimeSign.bTic100msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 500 == 0 ? g_tTimeSign.bTic500msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 1000 == 0 ? (g_tTimeSign.bTic1secSign = TRUE, sg_tTIME_Handle.ui1msTic = 0) : 0;
    }
}

/**
  * @brief      ע�ᶨʱ�ص�����.
  * @note       ע��Ķ�ʱ����ִ��ʱ�����ܶ�
  * @param[in]  pTimeFun �ص�����
  * @param[in]  time     �ص�����ʱ��, ��λ����
  * @retval     0,�ɹ�; -1,ʧ��
  */
int FML_TIME_Register(TimeFun pTimeFun, uint32_t time)
{
    if (sg_tTIME_Handle.ucSupportCnt < SUPPORT_FUN_MAX_NUM)
    {
        sg_tTIME_Handle.tCallFun[sg_tTIME_Handle.ucSupportCnt].ui1msMaxTime = time;
        sg_tTIME_Handle.tCallFun[sg_tTIME_Handle.ucSupportCnt].pTimeFun = pTimeFun;
        sg_tTIME_Handle.ucSupportCnt++;
        return 0;
    }
    
    return -1;
}

/**
  * @brief      ע����ʱ�ص�����.
  * @param[in]  pTimeFun �ص�����
  * @retval     None
  */
void FML_TIME_UnRegister(TimeFun pTimeFun)
{
    uint16_t i;
    uint8_t ucSupportCnt = 0;
    TIME_FunType tCallFun[SUPPORT_FUN_MAX_NUM] = {0};
    
    for (i = 0; i < sg_tTIME_Handle.ucSupportCnt && i < SUPPORT_FUN_MAX_NUM; i++)
    {
        if (sg_tTIME_Handle.tCallFun[i].pTimeFun != pTimeFun)
        {
            tCallFun[ucSupportCnt] = sg_tTIME_Handle.tCallFun[i];
            ucSupportCnt++;
        }
    }
    
    for (i = 0; i < SUPPORT_FUN_MAX_NUM; i++)
    {
        sg_tTIME_Handle.tCallFun[i] = tCallFun[i];
    }
    
    sg_tTIME_Handle.ucSupportCnt = ucSupportCnt;
}

