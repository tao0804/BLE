
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "panip_config.h"     // SW configuration

#include "app_proj_template.h"
#include "proj_template_server.h"
#include "proj_template_server_task.h"
#include "ota_server_task.h"

#include "gapm_task.h" 
#include "att.h" 
#include "co_utils.h" 
#include "gattc_task.h"
#include "prf_types.h" 
#include "string.h"
#include "app.h"
#include "stack_svc_api.h"
#include "mcu_hal.h"
#include "temperature.h"

static struct app_proj_template_env_tag app_proj_template_env;
void app_proj_template_init(void)
{
	memset(&app_proj_template_env, 0, sizeof(app_proj_template_env));
	temp_relate_init();	//init
	// 首次上电定时器set,间断性定时器再设置一次
	((ke_timer_set_handler)SVC_ke_timer_set)(TEMPTIMECNT_ENOUGH_TIMEOUT_TIMER, TASK_APP, 400);	//400 * 10ms
}


void app_proj_template_add_server(void)
{

	struct proj_template_server_db_cfg* db_cfg;
	
  // Allocate the BASS_CREATE_DB_REQ
  struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                TASK_GAPM, TASK_APP,
                                                gapm_profile_task_add_cmd,sizeof(struct proj_template_server_db_cfg));
	// Fill message
	req->operation	 = GAPM_PROFILE_TASK_ADD;
	req->sec_lvl	 = (PERM(SVC_AUTH, DISABLE)| PERM(SVC_UUID_LEN, UUID_16)| PERM(SVC_EKS, DISABLE)|PERM(SVC_DIS, DISABLE));

	req->prf_task_id = TASK_ID_PROJ_TEMPLATE_SERVER;
	req->app_task	 = TASK_APP;
	req->start_hdl	 = 0;

	// Set parameters
	db_cfg = (struct proj_template_server_db_cfg* ) req->param;
	db_cfg->features = 0x1fff;

	// Send the message
	((ke_msg_send_handler)SVC_ke_msg_send)(req);
}


void app_proj_template_enable_server_prf(uint8_t conidx)
{
	app_proj_template_env.conidx = conidx;

  // Allocate the message
  struct proj_template_server_enable_req * req = KE_MSG_ALLOC(PROJ_TEMPLATE_SERVER_ENABLE_REQ,
                                              prf_get_task_from_id(TASK_ID_PROJ_TEMPLATE_SERVER),
                                              TASK_APP,
                                              proj_template_server_enable_req);
  // Fill in the parameter structure
  req->conidx             = conidx;

  // NTF initial status - Disabled
  req->ntf_cfg           = PRF_CLI_STOP_NTFIND;

  // Send the message
  ((ke_msg_send_handler)SVC_ke_msg_send)(req);
}

void app_proj_template_send_value(uint8_t att_idx,uint8_t *buf,uint8_t len)
{
    // Allocate the message
	struct proj_template_server_write_cmd * cmd = KE_MSG_ALLOC_DYN(PROJ_TEMPLATE_SERVER_WRITE_CMD,
													  prf_get_task_from_id(TASK_ID_PROJ_TEMPLATE_SERVER),
													  TASK_APP,
													  proj_template_server_write_cmd,
													  len);
	
	cmd->conidx  = app_proj_template_env.conidx;
	//now fill report
	cmd->write_type = NOTIFICATION;		//NOTIFICATION
	cmd->att_idx = att_idx; 
	if(len > PROJ_TEMPLATE_SERVER_PACKET_SIZE)
	{
		len = PROJ_TEMPLATE_SERVER_PACKET_SIZE;
	}
	cmd->packet_size = len;
	memcpy(cmd->packet, buf, len);

	((ke_msg_send_handler)SVC_ke_msg_send)(cmd);
}

static int proj_template_server_peer_write_data_ind_handler(ke_msg_id_t const msgid,
                                               struct proj_template_server_peer_write_data_ind const *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
#if 0
    printf("recv %d\n", param->packet_size);
#else
	// uint8_t Sendata[PROJ_TEMPLATE_SERVER_PACKET_SIZE] = {0};
	show_reg3(param->packet,param->packet_size);
	if(0xf3 == param->packet[0])
	{
		int8 t = temp_temporary_sampling();
		printf("int8:0x%x,temp:%.2f", t, TEMP_VALUE_TO_C(t));
		// sprintf成功,则返回写入的字符总数
		// uint8 len = sprintf((char*)Sendata, "ch2:%02X", temp_temporary_sampling());
		// app_proj_template_send_value(PROJ_TEMPLATE_IDX_CTRL_VAL,Sendata, len);
	}
	// app_proj_template_send_value(PROJ_TEMPLATE_IDX_CTRL_VAL,Sendata,param->packet_size);

#endif
	return (KE_MSG_CONSUMED);
}

static int proj_template_server_enable_rsp_handler(ke_msg_id_t const msgid,
                                    struct proj_template_server_enable_rsp const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}

static int app_proj_template_msg_dflt_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    // Drop the message
    return (KE_MSG_CONSUMED);
}


/// Default State handlers definition
const struct ke_msg_handler app_proj_template_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t)app_proj_template_msg_dflt_handler},
    {PROJ_TEMPLATE_SERVER_PEER_WRITE_DATA_IND,   (ke_msg_func_t)proj_template_server_peer_write_data_ind_handler},
    {PROJ_TEMPLATE_SERVER_ENABLE_RSP,       (ke_msg_func_t)proj_template_server_enable_rsp_handler},
};

const struct ke_state_handler app_proj_template_table_handler =
    {&app_proj_template_msg_handler_list[0], (sizeof(app_proj_template_msg_handler_list)/sizeof(struct ke_msg_handler))};

