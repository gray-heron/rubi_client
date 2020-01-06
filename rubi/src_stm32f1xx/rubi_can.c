#include <stm32f1xx_hal.h>

#include "rubi.h"
#include "rubi_can.h"

extern CAN_HandleTypeDef hcan;
uint32_t                 rubi_flocked = false;

HAL_StatusTypeDef
    rubi_can_recv_msg_from_fifo(rubi_can_msg_t* msg, uint8_t fifo);

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan
                                       __attribute__((unused)))
{
    rubi_can_msg_t msg;
    RUBI_ASSERT(rubi_can_recv_msg_from_fifo(&msg, 0) == HAL_OK);
    rubi_event_inbound(msg);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef* hcan
                                       __attribute__((unused)))
{
    rubi_can_msg_t msg;
    RUBI_ASSERT(rubi_can_recv_msg_from_fifo(&msg, 1) == HAL_OK);
    rubi_event_inbound(msg);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef* hcan
                                        __attribute__((unused)))
{
    rubi_event_continue_tx();
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef* hcan
                                        __attribute__((unused)))
{
    rubi_event_continue_tx();
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef* hcan
                                        __attribute__((unused)))
{
    rubi_event_continue_tx();
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef* hcan __attribute__((unused)))
{
    if ((hcan->ErrorCode & HAL_CAN_ERROR_TX_ALST0) ||
        (hcan->ErrorCode & HAL_CAN_ERROR_TX_ALST1) ||
        (hcan->ErrorCode & HAL_CAN_ERROR_TX_ALST2))
    {
        HAL_CAN_ResetError(hcan);
        return;
    }

    rubi_die();
}

void rubi_can_init(void)
{
    HAL_CAN_ActivateNotification(
        &hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING |
                   CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_OVERRUN |
                   CAN_IT_RX_FIFO1_OVERRUN);

    HAL_CAN_Start(&hcan);
}

void rubi_flock(void)
{
    HAL_CAN_DeactivateNotification(
        &hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING |
                   CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_OVERRUN |
                   CAN_IT_RX_FIFO1_OVERRUN);

    rubi_flocked += 1;
}

void rubi_funlock(void)
{
    RUBI_ASSERT(rubi_flocked == 1);

    rubi_flocked -= 1;
    HAL_CAN_ActivateNotification(
        &hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING |
                   CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_OVERRUN |
                   CAN_IT_RX_FIFO1_OVERRUN);
}

void rubi_can_add_filter(
    uint16_t id, uint16_t mask, uint8_t fifo, uint8_t number)
{
    CAN_FilterTypeDef CAN_FilterInitStructure;
    CAN_FilterInitStructure.FilterBank   = number;
    CAN_FilterInitStructure.FilterMode   = CAN_FILTERMODE_IDMASK;
    CAN_FilterInitStructure.FilterScale  = CAN_FILTERSCALE_32BIT;
    CAN_FilterInitStructure.FilterIdHigh = id << 5; // 11-bit ID, in top bits
    CAN_FilterInitStructure.FilterIdLow  = 0x0000;
    CAN_FilterInitStructure.FilterMaskIdHigh = mask << 5; // resolves as 0x01xx
    CAN_FilterInitStructure.FilterMaskIdLow  = 0x0000;
    CAN_FilterInitStructure.FilterFIFOAssignment = fifo;
    CAN_FilterInitStructure.FilterActivation     = ENABLE;
    HAL_CAN_ConfigFilter(&hcan, &CAN_FilterInitStructure);
}

HAL_StatusTypeDef rubi_can_recv_msg_from_fifo(rubi_can_msg_t* msg, uint8_t fifo)
{
    CAN_RxHeaderTypeDef rx_msg;
    uint8_t             rx_data[8];

    HAL_StatusTypeDef status =
        HAL_CAN_GetRxMessage(&hcan, fifo, &rx_msg, rx_data);

    if(status != HAL_OK)
    {
        return HAL_ERROR;
    }

    if(rx_msg.RTR == CAN_RTR_REMOTE)
    {
        RUBI_ASSERT(0);
        return HAL_OK;
    }

    msg->data_length = rx_msg.DLC;
    msg->id          = rx_msg.StdId;
    memcpy(msg->data, rx_data, rx_msg.DLC);

    return HAL_OK;
}

bool rubi_can_send_array(int16_t id, uint8_t data_length, uint8_t* data)
{
    CAN_TxHeaderTypeDef tx_msg;
    uint32_t            mailbox;

    tx_msg.StdId = id;
    tx_msg.IDE   = CAN_ID_STD;
    tx_msg.DLC   = data_length;
    tx_msg.RTR   = CAN_RTR_DATA;

    return HAL_CAN_AddTxMessage(&hcan, &tx_msg, (uint8_t*)data, &mailbox) ==
           HAL_OK;
}
