/*
 * This file has been automatically generated by the WICED 20719-B1 Designer.
 * Bluetooth Application.
 *
 */

/** key_ClassicSpp.c
 *
 */

#include "wiced.h"
#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_uuid.h"
#include "wiced_bt_gatt.h"
#include "wiced_hal_nvram.h"
#include "wiced_hal_gpio.h"
#include "wiced_bt_app_hal_common.h"
#include "wiced_hal_platform.h"
#include "wiced_bt_trace.h"
#include "sparcommon.h"
#include "hci_control_api.h"
#include "wiced_transport.h"
#include "wiced_hal_pspi.h"
#include "key_SppConfirm_sdp_db.h"
#include "wiced_bt_cfg.h"
#include "wiced_bt_stack.h"
#include "wiced_bt_sdp.h"
#include "wiced_hal_wdog.h"
#include "wiced_bt_app_common.h"
#include "spp.h"
#include "wiced_hal_puart.h"


/*******************************************************************
 * Constant Definitions
 ******************************************************************/
#define TRANS_UART_BUFFER_SIZE  1024
#define TRANS_UART_BUFFER_COUNT 2

/*******************************************************************
 * Variable Definitions
 ******************************************************************/
extern const wiced_bt_cfg_settings_t wiced_bt_cfg_settings;
extern const wiced_bt_cfg_buf_pool_t wiced_bt_cfg_buf_pools[WICED_BT_CFG_NUM_BUF_POOLS];
// Transport pool for sending RFCOMM data to host
static wiced_transport_buffer_pool_t* transport_pool = NULL;

wiced_bt_device_address_t confirmAddr;

/*******************************************************************
 * Function Prototypes
 ******************************************************************/
static void                  key_classicspp_app_init            ( void );
static wiced_bt_dev_status_t key_classicspp_management_callback ( wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data );
static void                  key_classicspp_reset_device        ( void );
static uint32_t              hci_control_process_rx_cmd         ( uint8_t* p_data, uint32_t len );
#ifdef HCI_TRACE_OVER_TRANSPORT
static void                  key_classicspp_trace_callback      ( wiced_bt_hci_trace_type_t type, uint16_t length, uint8_t* p_data );
#endif
int key_classicspp_read_link_keys( wiced_bt_device_link_keys_t *keys );
int key_classicspp_write_link_keys( wiced_bt_device_link_keys_t *keys );
void rx_cback( void *data );
void button_cback( void *data, uint8_t port_pin );


/*******************************************************************
 * Macro Definitions
 ******************************************************************/
// Macro to extract uint16_t from little-endian byte array
#define LITTLE_ENDIAN_BYTE_ARRAY_TO_UINT16(byte_array) \
        (uint16_t)( ((byte_array)[0] | ((byte_array)[1] << 8)) )

/*******************************************************************
 * Transport Configuration
 ******************************************************************/
wiced_transport_cfg_t transport_cfg =
{
    WICED_TRANSPORT_UART,              /**< Wiced transport type. */
    {
        WICED_TRANSPORT_UART_HCI_MODE, /**<  UART mode, HCI or Raw */
        HCI_UART_DEFAULT_BAUD          /**<  UART baud rate */
    },
    {
        TRANS_UART_BUFFER_SIZE,        /**<  Rx Buffer Size */
        TRANS_UART_BUFFER_COUNT        /**<  Rx Buffer Count */
    },
    NULL,                              /**< Wiced transport status handler.*/
    hci_control_process_rx_cmd,        /**< Wiced transport receive data handler. */
    NULL                               /**< Wiced transport tx complete callback. */
};

/*******************************************************************
 * Function Definitions
 ******************************************************************/

/*
 * Entry point to the application. Set device configuration and start BT
 * stack initialization.  The actual application initialization will happen
 * when stack reports that BT device is ready
 */
void application_start(void)
{
    /* Initialize the transport configuration */
    wiced_transport_init( &transport_cfg );

    /* Initialize Transport Buffer Pool */
    transport_pool = wiced_transport_create_buffer_pool ( TRANS_UART_BUFFER_SIZE, TRANS_UART_BUFFER_COUNT );

#if ((defined WICED_BT_TRACE_ENABLE) || (defined HCI_TRACE_OVER_TRANSPORT))
    /* Set the Debug UART as WICED_ROUTE_DEBUG_NONE to get rid of prints */
    //  wiced_set_debug_uart( WICED_ROUTE_DEBUG_NONE );

    /* Set Debug UART as WICED_ROUTE_DEBUG_TO_PUART to see debug traces on Peripheral UART (PUART) */
    wiced_set_debug_uart( WICED_ROUTE_DEBUG_TO_PUART );

    /* Set the Debug UART as WICED_ROUTE_DEBUG_TO_WICED_UART to send debug strings over the WICED debug interface */
    //wiced_set_debug_uart( WICED_ROUTE_DEBUG_TO_WICED_UART );
#endif

    /* Initialize Bluetooth Controller and Host Stack */
    wiced_bt_stack_init(key_classicspp_management_callback, &wiced_bt_cfg_settings, wiced_bt_cfg_buf_pools);
}

/*
 * This function is executed in the BTM_ENABLED_EVT management callback.
 */
void key_classicspp_app_init(void)
{
    /* Initialize Application */
    wiced_bt_app_init();

    /* Allow peer to pair */
    wiced_bt_set_pairable_mode(WICED_TRUE, 0);

    /* Initialize SDP Database */
    wiced_bt_sdp_db_init( (uint8_t*)sdp_database, sdp_database_len );

    /* Make device connectable (enables page scan) using default connectability window/interval.
     * The corresponding parameters are contained in 'wiced_bt_cfg.c' */
    /* TODO: Make sure that this is the desired behavior. */
    wiced_bt_dev_set_connectability(BTM_CONNECTABLE, BTM_DEFAULT_CONN_WINDOW, BTM_DEFAULT_CONN_INTERVAL);

    /* Make device discoverable (enables inquiry scan) over BR/EDR using default discoverability window/interval.
     * The corresponding parameters are contained in 'wiced_bt_cfg.c' */
    /* TODO: Make sure that this is the desired behavior. */
    wiced_bt_dev_set_discoverability(BTM_GENERAL_DISCOVERABLE, BTM_DEFAULT_DISC_WINDOW, BTM_DEFAULT_DISC_INTERVAL);

    /* Start the SPP server */
    spp_start();

    /* Setup UART to receive characters */
    wiced_hal_puart_init( );
    wiced_hal_puart_flow_off( );
    wiced_hal_puart_set_baudrate( 115200 );
    wiced_hal_puart_enable_tx( );

    /* Enable receive and the interrupt */
    wiced_hal_puart_register_interrupt( rx_cback );

    /* Set watermark level to 1 to receive interrupt up on receiving each byte */
    wiced_hal_puart_set_watermark_level( 1 );
    wiced_hal_puart_enable_rx();
}

/* TODO: This function should be called when the device needs to be reset */
void key_classicspp_reset_device( void )
{
    /* TODO: Clear any additional persistent values used by the application from NVRAM */

    // Reset the device
    wiced_hal_wdog_reset_system( );
}

/* Bluetooth Management Event Handler */
wiced_bt_dev_status_t key_classicspp_management_callback( wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data )
{
    wiced_bt_dev_status_t status = WICED_BT_SUCCESS;
    wiced_bt_device_address_t bda = { 0 };
    wiced_bt_dev_br_edr_pairing_info_t *p_br_edr_info = NULL;
    wiced_bt_ble_advert_mode_t *p_adv_mode = NULL;

    switch (event)
    {
    case BTM_ENABLED_EVT:
        /* Bluetooth Controller and Host Stack Enabled */

#ifdef HCI_TRACE_OVER_TRANSPORT
        // There is a virtual HCI interface between upper layers of the stack and
        // the controller portion of the chip with lower layers of the BT stack.
        // Register with the stack to receive all HCI commands, events and data.
        wiced_bt_dev_register_hci_trace(key_classicspp_trace_callback);
#endif

        WICED_BT_TRACE("Bluetooth Enabled (%s)\n",
                ((WICED_BT_SUCCESS == p_event_data->enabled.status) ? "success" : "failure"));

        if (WICED_BT_SUCCESS == p_event_data->enabled.status)
        {
            /* Bluetooth is enabled */
            wiced_bt_dev_read_local_addr(bda);
            WICED_BT_TRACE("Local Bluetooth Address: [%B]\n", bda);

            /* Perform application-specific initialization */
            key_classicspp_app_init();
        }
        break;
    case BTM_DISABLED_EVT:
        /* Bluetooth Controller and Host Stack Disabled */
        WICED_BT_TRACE("Bluetooth Disabled\n");
        break;
    case BTM_PASSKEY_NOTIFICATION_EVT: /* Print passkey to the screen so that the user can enter it. */
        WICED_BT_TRACE( "Passkey Notification\n\r");
        WICED_BT_TRACE(">>>>>>>>>>>>>>>>>>>>>>>> PassKey Required for BDA %B, Enter Key: %06d \n\r", p_event_data->user_passkey_notification.bd_addr, p_event_data->user_passkey_notification.passkey );
        break;
    case BTM_SECURITY_REQUEST_EVT:
        /* Security Request */
        WICED_BT_TRACE("Security Request\n");
        wiced_bt_ble_security_grant(p_event_data->security_request.bd_addr, WICED_BT_SUCCESS);
        break;
    case BTM_PAIRING_IO_CAPABILITIES_BR_EDR_RESPONSE_EVT:
         WICED_BT_TRACE(
                         "IO_CAPABILITIES_BR_EDR_RESPONSE peer_bd_addr: %B, peer_io_cap: %d, peer_oob_data: %d, peer_auth_req: %d\n",
                         p_event_data->pairing_io_capabilities_br_edr_response.bd_addr,
                         p_event_data->pairing_io_capabilities_br_edr_response.io_cap,
                         p_event_data->pairing_io_capabilities_br_edr_response.oob_data,
                         p_event_data->pairing_io_capabilities_br_edr_response.auth_req);
         break;
    case BTM_PAIRING_IO_CAPABILITIES_BR_EDR_REQUEST_EVT:
        /* Request for Pairing IO Capabilities (BR/EDR) */
        WICED_BT_TRACE("BR/EDR Pairing IO Capabilities Request\n");
        p_event_data->pairing_io_capabilities_br_edr_request.oob_data = BTM_OOB_NONE;
        p_event_data->pairing_io_capabilities_br_edr_request.auth_req = BTM_AUTH_SINGLE_PROFILE_GENERAL_BONDING_YES;
        p_event_data->pairing_io_capabilities_br_edr_request.is_orig = WICED_FALSE;
        p_event_data->pairing_io_capabilities_br_edr_request.local_io_cap = BTM_IO_CAPABILITIES_DISPLAY_AND_YES_NO_INPUT;
        break;
    case BTM_PAIRING_COMPLETE_EVT:
        /* Pairing is Complete */
        p_br_edr_info = &p_event_data->pairing_complete.pairing_complete_info.br_edr;
        WICED_BT_TRACE("Pairing Complete %d.\n", p_br_edr_info->status);
        break;
    case BTM_ENCRYPTION_STATUS_EVT:
        /* Encryption Status Change */
        WICED_BT_TRACE("Encryption Status event: bd ( %B ) res %d\n", p_event_data->encryption_status.bd_addr, p_event_data->encryption_status.result);
        break;
    case BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
        /* Paired Device Link Keys Request */
        WICED_BT_TRACE("Paired Device Link Request Keys Event\n");
        /* Device/app-specific TODO: HANDLE PAIRED DEVICE LINK REQUEST KEY - retrieve from NVRAM, etc */
#if 1
        if (key_classicspp_read_link_keys( &p_event_data->paired_device_link_keys_request ))
        {
            WICED_BT_TRACE("Key Retrieval Success\n");
        }
        else
#endif
        /* Until key retrieval implemented above, just fail the request - will cause re-pairing */
        {
            WICED_BT_TRACE("Key Retrieval Failure\n");
            status = WICED_BT_ERROR;
        }
        break;
    case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
        WICED_BT_TRACE("BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT\n");
        key_classicspp_write_link_keys( &p_event_data->paired_device_link_keys_update);
        break;
    case BTM_USER_CONFIRMATION_REQUEST_EVT:
        /* Pairing request, TODO: handle confirmation of numeric compare here if desired */
        WICED_BT_TRACE("numeric_value: %d\n", p_event_data->user_confirmation_request.numeric_value);
        WICED_BT_TRACE("Press MB1 to accept the connection or MB2 to reject\n");

        /* Save the BDADDR */
        memcpy(confirmAddr,p_event_data->user_confirmation_request.bd_addr,sizeof(confirmAddr));

        /* Turn on button interrupts */
        wiced_hal_gpio_register_pin_for_interrupt( WICED_GPIO_PIN_BUTTON_1, button_cback, NULL );
        wiced_hal_gpio_configure_pin( WICED_GPIO_PIN_BUTTON_1, ( GPIO_INPUT_ENABLE | GPIO_PULL_UP | GPIO_EN_INT_FALLING_EDGE ), GPIO_PIN_OUTPUT_HIGH );
        wiced_hal_gpio_register_pin_for_interrupt( WICED_GPIO_PIN_BUTTON_2, button_cback, NULL );
        wiced_hal_gpio_configure_pin( WICED_GPIO_PIN_BUTTON_2, ( GPIO_INPUT_ENABLE | GPIO_PULL_UP | GPIO_EN_INT_FALLING_EDGE ), GPIO_PIN_OUTPUT_HIGH );

        /* This is done in the interrupt callback based on the button press */
        //wiced_bt_dev_confirm_req_reply( WICED_BT_SUCCESS , p_event_data->user_confirmation_request.bd_addr);
        break;
    default:
        WICED_BT_TRACE("Unhandled Bluetooth Management Event: 0x%x (%d)\n", event, event);
        break;
    }

    return status;
}


/* Handle Command Received over Transport */
uint32_t hci_control_process_rx_cmd( uint8_t* p_data, uint32_t len )
{
    uint8_t status = 0;
    uint8_t cmd_status = HCI_CONTROL_STATUS_SUCCESS;
    uint8_t opcode = 0;
    uint8_t* p_payload_data = NULL;
    uint8_t payload_length = 0;

    WICED_BT_TRACE("hci_control_process_rx_cmd : Data Length '%d'\n", len);

    // At least 4 bytes are expected in WICED Header
    if ((NULL == p_data) || (len < 4))
    {
        WICED_BT_TRACE("Invalid Parameters\n");
        status = HCI_CONTROL_STATUS_INVALID_ARGS;
    }
    else
    {
        // Extract OpCode and Payload Length from little-endian byte array
        opcode = LITTLE_ENDIAN_BYTE_ARRAY_TO_UINT16(p_data);
        payload_length = LITTLE_ENDIAN_BYTE_ARRAY_TO_UINT16(&p_data[sizeof(uint16_t)]);
        p_payload_data = &p_data[sizeof(uint16_t)*2];

        // TODO : Process received HCI Command based on its Control Group
        // (see 'hci_control_api.h' for additional details)
        switch ( HCI_CONTROL_GROUP(opcode) )
        {
        default:
            // HCI Control Group was not handled
            cmd_status = HCI_CONTROL_STATUS_UNKNOWN_GROUP;
            wiced_transport_send_data(HCI_CONTROL_EVENT_COMMAND_STATUS, &cmd_status, sizeof(cmd_status));
            break;
        }
    }

    // When operating in WICED_TRANSPORT_UART_HCI_MODE or WICED_TRANSPORT_SPI,
    // application has to free buffer in which data was received
    wiced_transport_free_buffer( p_data );
    p_data = NULL;

    return status;
}

#ifdef HCI_TRACE_OVER_TRANSPORT
/* Handle Sending of Trace over the Transport */
void key_classicspp_trace_callback( wiced_bt_hci_trace_type_t type, uint16_t length, uint8_t* p_data )
{
    wiced_transport_send_hci_trace( transport_pool, type, length, p_data );
}
#endif


// This function reads the first VSID row into the link keys
int key_classicspp_read_link_keys( wiced_bt_device_link_keys_t *keys )
{

    wiced_result_t result;
    int bytes_read;

    bytes_read = wiced_hal_read_nvram(WICED_NVRAM_VSID_START,sizeof(wiced_bt_device_link_keys_t),
            (uint8_t *)keys,&result);
    WICED_BT_TRACE("NVRAM ID:%d read :%d bytes result:%d\n", WICED_NVRAM_VSID_START, bytes_read, result);

    return bytes_read;

}

// This function write the link keys into the first VSID row
int key_classicspp_write_link_keys( wiced_bt_device_link_keys_t *keys )
{

    wiced_result_t  result;
    int   bytes_written = wiced_hal_write_nvram(WICED_NVRAM_VSID_START, sizeof(wiced_bt_device_link_keys_t),
            (uint8_t*)keys, &result);

       WICED_BT_TRACE("NVRAM ID:%d written :%d bytes result:%d\n", WICED_NVRAM_VSID_START, bytes_written, result);
       return (bytes_written);
}


/* Interrupt callback function for UART */
void rx_cback( void *data )
{
    uint8_t  readbyte;

    /* Read one byte from the buffer and (unlike GPIO) reset the interrupt */
    wiced_hal_puart_read( &readbyte );
    wiced_hal_puart_reset_puart_interrupt();

    /* Transmit the data using the Bluetooth SPP */
    spp_tx_data(&readbyte, sizeof(readbyte));
}


/* Interrupt callback function for BUTTONS */
void button_cback( void *data, uint8_t port_pin )
{
    /* Turn off pin interrupts */
    wiced_hal_gpio_configure_pin( WICED_GPIO_PIN_BUTTON_1, ( GPIO_INPUT_ENABLE | GPIO_PULL_UP  ), GPIO_PIN_OUTPUT_HIGH );
    wiced_hal_gpio_configure_pin( WICED_GPIO_PIN_BUTTON_2, ( GPIO_INPUT_ENABLE | GPIO_PULL_UP  ), GPIO_PIN_OUTPUT_HIGH );

    /* Send appropriate response */
    if(port_pin == WICED_GPIO_PIN_BUTTON_1) /* Yes */
    {
        wiced_bt_dev_confirm_req_reply( WICED_BT_SUCCESS , confirmAddr);
        WICED_BT_TRACE("Confirm Accepted\n");
    }
    if(port_pin == WICED_GPIO_PIN_BUTTON_2) /* No */
    {
        wiced_bt_dev_confirm_req_reply( WICED_BT_ERROR , confirmAddr);
        WICED_BT_TRACE("Confirm Failed\n");

    }
}
