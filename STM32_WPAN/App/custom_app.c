/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    App/custom_app.c
 * @author  MCD Application Team
 * @brief   Custom Example Application (Server)
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "custom_app.h"
#include "custom_stm.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef struct {
	/* Mvt_def */
	uint8_t Mvt_rep_Notification_Status;
	/* USER CODE BEGIN CUSTOM_APP_Context_t */
	uint8_t rep_status;

	/* USER CODE END CUSTOM_APP_Context_t */

	uint16_t ConnectionHandle;
} Custom_App_Context_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_APP_CONTEXT
 */

static Custom_App_Context_t Custom_App_Context;

/**
 * END of Section BLE_APP_CONTEXT
 */

uint8_t UpdateCharData[247];
uint8_t NotifyCharData[247];

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* Mvt_def */
static void Custom_Mvt_rep_Update_Char(void);
static void Custom_Mvt_rep_Send_Notification(void);

/* USER CODE BEGIN PFP */
static void Custom_Appli_Update_Char();
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void Custom_STM_App_Notification(
		Custom_STM_App_Notification_evt_t *pNotification) {
	/* USER CODE BEGIN CUSTOM_STM_App_Notification_1 */

	/* USER CODE END CUSTOM_STM_App_Notification_1 */
	switch (pNotification->Custom_Evt_Opcode) {
	/* USER CODE BEGIN CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

	/* USER CODE END CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

	/* Mvt_def */
	case CUSTOM_STM_MVT_REP_NOTIFY_ENABLED_EVT:
		/* USER CODE BEGIN CUSTOM_STM_MVT_REP_NOTIFY_ENABLED_EVT */
		Custom_App_Context.Mvt_rep_Notification_Status = 1;
		Custom_Mvt_rep_Send_Notification();
		/* USER CODE END CUSTOM_STM_MVT_REP_NOTIFY_ENABLED_EVT */
		break;

	case CUSTOM_STM_MVT_REP_NOTIFY_DISABLED_EVT:
		/* USER CODE BEGIN CUSTOM_STM_MVT_REP_NOTIFY_DISABLED_EVT */
		Custom_App_Context.Mvt_rep_Notification_Status = 0;
		/* USER CODE END CUSTOM_STM_MVT_REP_NOTIFY_DISABLED_EVT */
		break;

	case CUSTOM_STM_APPEL_APP_READ_EVT:
		/* USER CODE BEGIN CUSTOM_STM_APPEL_APP_READ_EVT */

		/* USER CODE END CUSTOM_STM_APPEL_APP_READ_EVT */
		break;

	case CUSTOM_STM_APPEL_APP_WRITE_NO_RESP_EVT:
		/* USER CODE BEGIN CUSTOM_STM_APPEL_APP_WRITE_NO_RESP_EVT */
		memcpy(UpdateCharData, pNotification->DataTransfered.pPayload,
				pNotification->DataTransfered.Length);
		Custom_Appli_Update_Char();
		/* USER CODE END CUSTOM_STM_APPEL_APP_WRITE_NO_RESP_EVT */
		break;

	case CUSTOM_STM_NOTIFICATION_COMPLETE_EVT:
		/* USER CODE BEGIN CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */

		/* USER CODE END CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */
		break;

	default:
		/* USER CODE BEGIN CUSTOM_STM_App_Notification_default */

		/* USER CODE END CUSTOM_STM_App_Notification_default */
		break;
	}
	/* USER CODE BEGIN CUSTOM_STM_App_Notification_2 */

	/* USER CODE END CUSTOM_STM_App_Notification_2 */
	return;
}

void Custom_APP_Notification(Custom_App_ConnHandle_Not_evt_t *pNotification) {
	/* USER CODE BEGIN CUSTOM_APP_Notification_1 */

	/* USER CODE END CUSTOM_APP_Notification_1 */

	switch (pNotification->Custom_Evt_Opcode) {
	/* USER CODE BEGIN CUSTOM_APP_Notification_Custom_Evt_Opcode */

	/* USER CODE END P2PS_CUSTOM_Notification_Custom_Evt_Opcode */
	case CUSTOM_CONN_HANDLE_EVT:
		/* USER CODE BEGIN CUSTOM_CONN_HANDLE_EVT */

		/* USER CODE END CUSTOM_CONN_HANDLE_EVT */
		break;

	case CUSTOM_DISCON_HANDLE_EVT:
		/* USER CODE BEGIN CUSTOM_DISCON_HANDLE_EVT */

		/* USER CODE END CUSTOM_DISCON_HANDLE_EVT */
		break;

	default:
		/* USER CODE BEGIN CUSTOM_APP_Notification_default */

		/* USER CODE END CUSTOM_APP_Notification_default */
		break;
	}

	/* USER CODE BEGIN CUSTOM_APP_Notification_2 */

	/* USER CODE END CUSTOM_APP_Notification_2 */

	return;
}

void Custom_APP_Init(void) {
	/* USER CODE BEGIN CUSTOM_APP_Init */
	Custom_App_Context.rep_status = 0;

	UTIL_SEQ_RegTask(1 << CFG_TASK_ACC_rep_ID, UTIL_SEQ_RFU,
			Custom_Mvt_rep_Send_Notification);

	Custom_Mvt_rep_Update_Char();

	/* USER CODE END CUSTOM_APP_Init */
	return;
}

/* USER CODE BEGIN FD */
void FSVC_MVT_REF_fin(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 1;

	return;
}
void FSVC_MVT_REF_vide(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 2;

	return;
}
void FSVC_MVT_REF_validation_succes(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 3;

	return;
}
void FSVC_MVT_REF_validation_fail(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 4;

	return;
}
void FSVC_MVT_REF_validation_vide(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 5;

	return;
}
void FSVC_MVT_vide(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 6;

	return;
}

void FSVC_MVT_BRAS_GAUCHE_valide(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 7;

	return;
}
void FSVC_MVT_BRAS_DROIT_valide(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 8;

	return;
}
void FSVC_MVT_BRAS_CORRECT_valide(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 9;

	return;
}
void FSVC_MVT_BRAS_GAUCHE_non_valide(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 10;

	return;
}
void FSVC_MVT_BRAS_DROIT_non_valide(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 11;

	return;
}
void FSVC_MVT_BRAS_CORRECT_non_valide(void) {
	UTIL_SEQ_SetTask(1 << CFG_TASK_ACC_rep_ID, CFG_SCH_PRIO_0);
	Custom_App_Context.rep_status = 12;

	return;
}

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/* Mvt_def */
void Custom_Mvt_rep_Update_Char(void) /* Property Read */
{
	uint8_t updateflag = 0;

	/* USER CODE BEGIN Mvt_rep_UC_1*/

	/* USER CODE END Mvt_rep_UC_1*/

	if (updateflag != 0) {
		Custom_STM_App_Update_Char(CUSTOM_STM_MVT_REP,
				(uint8_t*) UpdateCharData);
	}

	/* USER CODE BEGIN Mvt_rep_UC_Last*/

	/* USER CODE END Mvt_rep_UC_Last*/
	return;
}

void Custom_Mvt_rep_Send_Notification(void) /* Property Notification */
{
	uint8_t updateflag = 0;

	/* USER CODE BEGIN Mvt_rep_NS_1*/
	if (Custom_App_Context.Mvt_rep_Notification_Status == 1) {
		updateflag = 1;

		if (Custom_App_Context.rep_status == 1) {

			char rep_ref_fin[] = "finr";
			memcpy(&NotifyCharData[0], &rep_ref_fin, sizeof(rep_ref_fin));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT rep_ref_fin good \n")
			;
		}
		if (Custom_App_Context.rep_status == 2) {

			char rep_ref_vide[] = "refv";
			memcpy(&NotifyCharData[0], &rep_ref_vide, sizeof(rep_ref_vide));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT rep_ref_vide good \n")
			;
		}
		if (Custom_App_Context.rep_status == 3) {

			char validation_succes[] = "vals";
			memcpy(&NotifyCharData[0], &validation_succes,
					sizeof(validation_succes));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT validation_succes good \n")
			;
		}
		if (Custom_App_Context.rep_status == 4) {

			char validation_fail[] = "valf";
			memcpy(&NotifyCharData[0], &validation_fail,
					sizeof(validation_fail));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT validation_fail good \n")
			;
		}
		if (Custom_App_Context.rep_status == 5) {

			char validation_vide[] = "valv";
			memcpy(&NotifyCharData[0], &validation_vide,
					sizeof(validation_vide));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT validation_vide good \n")
			;
		}
		if (Custom_App_Context.rep_status == 6) {

			char rep_vide[] = "repv";
			memcpy(&NotifyCharData[0], &rep_vide, sizeof(rep_vide));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT rep_vide good \n")
			;
		}
		if (Custom_App_Context.rep_status == 7) {

			char BRAS_GAUCHE_valide[] = "Gval";
			memcpy(&NotifyCharData[0], &BRAS_GAUCHE_valide,
					sizeof(BRAS_GAUCHE_valide));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT rep_ref_vide good \n")
			;
		}
		if (Custom_App_Context.rep_status == 8) {

			char BRAS_DROIT_valide[] = "Dval";
			memcpy(&NotifyCharData[0], &BRAS_DROIT_valide,
					sizeof(BRAS_DROIT_valide));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT validation_succes good \n")
			;
		}
		if (Custom_App_Context.rep_status == 9) {

			char BRAS_CORRECT_valide[] = "Cval";
			memcpy(&NotifyCharData[0], &BRAS_CORRECT_valide,
					sizeof(BRAS_CORRECT_valide));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT validation_succes good \n")
			;
		}
		if (Custom_App_Context.rep_status == 10) {

			char BRAS_GAUCHE_non_valide[] = "Gnva";
			memcpy(&NotifyCharData[0], &BRAS_GAUCHE_non_valide,
					sizeof(BRAS_GAUCHE_non_valide));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT validation_succes good \n")
			;
		}
		if (Custom_App_Context.rep_status == 11) {

			char BRAS_DROIT_non_valide[] = "Dnva";
			memcpy(&NotifyCharData[0], &BRAS_DROIT_non_valide,
					sizeof(BRAS_DROIT_non_valide));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT validation_succes good \n")
			;
		}
		if (Custom_App_Context.rep_status == 12) {

			char BRAS_CORRECT_non_valide[] = "Cnva";
			memcpy(&NotifyCharData[0], &BRAS_CORRECT_non_valide,
					sizeof(BRAS_CORRECT_non_valide));
			APP_DBG_MSG("-- CUSTOM APPLICATION SERVER  : INFORM CLIENT validation_succes good \n")
			;
		}
	} else {
		APP_DBG_MSG("-- CUSTOM APPLICATION : CAN'T INFORM CLIENT -  NOTIFICATION DISABLED\n")
		;
	}
	/* USER CODE END Mvt_rep_NS_1*/

	if (updateflag != 0) {
		Custom_STM_App_Update_Char(CUSTOM_STM_MVT_REP,
				(uint8_t*) NotifyCharData);
	}

	/* USER CODE BEGIN Mvt_rep_NS_Last*/

	/* USER CODE END Mvt_rep_NS_Last*/

	return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/
void Custom_Appli_Update_Char()/* Property writed */
{
	Custom_STM_App_Update_Char(CUSTOM_STM_APPEL_APP, (uint8_t*) UpdateCharData);
	return;
}
/* USER CODE END FD_LOCAL_FUNCTIONS*/
