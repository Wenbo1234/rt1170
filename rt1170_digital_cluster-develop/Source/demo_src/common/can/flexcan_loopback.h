

/*! @brief FlexCAN Rx FIFO transfer. */
typedef struct _flexcan_data_buff
{
    uint32_t DATA[16];
} flexcan_data_buff_t;

#define EXAMPLE_CAN CAN1
#define EXAMPLE_FLEXCAN_IRQn CAN1_IRQn
#define EXAMPLE_FLEXCAN_IRQHandler CAN1_IRQHandler
#define TX_MESSAGE_BUFFER_NUM (0)
#define RX_MESSAGE_BUFFER_NUM (1)


#define TX_ID   0x100U
#define RX_ID   0x100U

#include "fsl_flexcan.h"
#include "semphr.h"

void CANFD_Init(void);

void CANFD_RX_MailBox_Config(uint8_t mbIdx, uint32_t rx_id);

void CANFD_TX_MailBox_Config(uint8_t mbIdx, uint32_t tx_id);

void CANFD_TX_SendData(uint8_t mbIdx, uint32_t tx_id, flexcan_data_buff_t send_data);

