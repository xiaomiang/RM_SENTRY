#include "main.h"
#include "task_ActionUpdate.h"

State_t Sentry_State;
block_disconnect_t block_disconnect;
int16_t Gimbal_init_flag,Shoot_init_flag = 0;
Debug_Mode_Type Debug_Mode;

static void update_state_with_RCdata(void);
static void sentry_state_reflect(uint8_t gimbal_r_mode,
                                 uint8_t gimbal_l_mode,
                                 uint8_t shoot_r_mode,
                                 uint8_t shoot_l_mode,
                                 uint8_t chassis_mode,
																 uint8_t chassisyaw_mode);

/**
 * @brief ״̬�����壬ִ��ÿ�ε�״̬ˢ��
 * @param ��
 * @retval ��
 */
void task_ActionUpdate(void)
{
    reset_remote();
		Rst_Debug();
    
    //����ͨ����أ�����������������ƣ�
    extern uint32_t chassis_offline_tick;
    extern uint8_t chassis_offline_flag;
    extern uint8_t Heat_ShootAbleFlag;
    
    while (1)
    {
				//������
				Robo_Disconnect.RemoteDisconnect ++;
				if(Robo_Disconnect.RemoteDisconnect > 50)
					reset_remote();
			
        update_state_with_RCdata(); //��ң�������µ�ǰ״
        vTaskDelay(10);
    }
}

/**
  * @brief  ��ң������Ϣ���¹���״̬
  * @param  None
  * @retval None
  */
static void update_state_with_RCdata(void)
{
    RC_Ctl_t RC_Ctl = getRCData();

    switch (RC_Ctl.rc.s1)
    {
    case 1:
        switch (RC_Ctl.rc.s2)
        {
        case 1: //�����ϳ�֮��ģʽ
            sentry_state_reflect(Gimbal_R_PC,
                                 Gimbal_L_PC,
                                 Shoot_R_PC,
                                 Shoot_L_PC,
                                 Chassis_Patrol,
																 ChassisYaw_PC);
            break;
        case 2://���̻������ټ�¼ģʽ
            sentry_state_reflect(Gimbal_R_SLEEP,
                                 Gimbal_L_SLEEP,
                                 Shoot_R_SLEEP,
                                 Shoot_L_SLEEP,
                                 Chassis_Patrol,
																 ChassisYaw_PC);
            break;
        case 3: //���ڶ̻����ϵĸ���ģʽ
            sentry_state_reflect(Gimbal_R_PC,
                                 Gimbal_L_PC,
                                 (abs(RC_Ctl.rc.ch3-1024)>300)?Shoot_R_RC:Shoot_R_SLEEP,
                                 Shoot_L_SLEEP, 
                                 Chassis_SLEEP,
																 ChassisYaw_PC);
            break;
        default:
            break;
        }
        break;
    case 2:
        switch (RC_Ctl.rc.s2)
        {
        case 1: //����̨��¼���ӵ�
            sentry_state_reflect(Gimbal_R_SLEEP,
                                 Gimbal_L_SLEEP,
                                 Shoot_R_SLEEP,
                                 Shoot_L_SLEEP,
                                 Chassis_RC,
																 ChassisYaw_RC);
            break;
        case 2: //����ģʽ
            sentry_state_reflect(Gimbal_R_SLEEP,
                                 Gimbal_L_SLEEP,
                                 Shoot_R_SLEEP,
                                 Shoot_L_SLEEP,
                                 Chassis_SLEEP,
																 ChassisYaw_SLEEP);
            break;
        case 3: //����̨��¼���ӵ�
            sentry_state_reflect(Gimbal_R_SLEEP,
//                                 Gimbal_L_RC,
																	Gimbal_L_RC,
                                 Shoot_R_SLEEP,
                                 Shoot_L_SLEEP,
                                 Chassis_SLEEP,
																 ChassisYaw_SLEEP);
            break;
        default:
            break;
        }
        break;
    case 3:
        switch (RC_Ctl.rc.s2)
        {
        case 1: //����̨ң��������
            sentry_state_reflect(Gimbal_R_SLEEP,
                                 Gimbal_L_SLEEP,
                                 Shoot_R_SLEEP,
                                 Shoot_L_SLEEP,
                                 Chassis_Protect,
																 ChassisYaw_RC);
            break;
        case 2: //����̨ң��������
            sentry_state_reflect(Gimbal_R_AIM,
                                 Gimbal_L_AIM,
                                 (abs(RC_Ctl.rc.ch3-1024)>300)?Shoot_R_RC:Shoot_R_SLEEP,
                                 Shoot_L_SLEEP,
                                 Chassis_SLEEP,
																 ChassisYaw_SLEEP);
               break;
        case 3: //����Ѳ��ģʽ�����ʵ��ԣ�
            sentry_state_reflect(Gimbal_R_RC,
//																	Gimbal_R_RC,
                                 Gimbal_L_SLEEP,
                                 (abs(RC_Ctl.rc.ch3-1024)>300)?Shoot_R_RC:Shoot_R_SLEEP,
                                 Shoot_L_SLEEP,
                                 Chassis_SLEEP,
																 ChassisYaw_SLEEP);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

static void sentry_state_reflect(uint8_t gimbal_r_mode,
                                 uint8_t gimbal_l_mode,
                                 uint8_t shoot_r_mode,
                                 uint8_t shoot_l_mode,
                                 uint8_t chassis_mode,
																 uint8_t chassisyaw_mode)
{
    Sentry_State.Gimbal_R_Mode = gimbal_r_mode;
    Sentry_State.Gimbal_L_Mode = gimbal_l_mode;
    Sentry_State.Shoot_R_Mode = shoot_r_mode;
    Sentry_State.Shoot_L_Mode = shoot_l_mode;
    Sentry_State.Chassis_Mode = chassis_mode;
	  Sentry_State.ChassisYaw_Mode = chassisyaw_mode;
	
		if(Debug_Mode.Shoot_Debug)
			Sentry_State.Shoot_R_Mode = Shoot_R_DEBUG;
		if(Debug_Mode.Gimbal_R_Debug)
			Sentry_State.Gimbal_R_Mode = Gimbal_R_DEBUG;
		if(Debug_Mode.Gimbal_L_Debug)
			Sentry_State.Gimbal_L_Mode = Gimbal_L_DEBUG;
}


/**
  * @brief  debug_mode ������ʼ��
  * @param  None
  * @retval None
  */
void Rst_Debug()
{
	Debug_Mode.Shoot_Debug = 0;
	Debug_Mode.Gimbal_R_Debug = 0;
	Debug_Mode.Gimbal_L_Debug = 0;
}