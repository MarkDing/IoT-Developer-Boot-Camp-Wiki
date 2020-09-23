/***************************************************************************//**
 * @file main.c
 * @brief Bluetotoh over-the-air (OTA) device firmware update (DFU) example for
          x86 host using Network Co-Processor (NCP)
 * @version 3.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/**
 * This an example application that demonstrates how to make a Bluetooth Over-the-Air (OTA)
 * firmware update.
 *
 * To use this application you must have a WSTK configured into NCP mode connected to your
 * PC and it is used as a Bluetooth radio to push the OTA update to a remote device.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "sl_bt_api.h"
#include "sl_bt_ncp_host.h"
#include "uart.h"
#include "system.h"

SL_BT_API_DEFINE();

/**
 * Configurable parameters that can be modified to match the test setup.
 */

/** The serial port to use for BGAPI communication. */
static char* uart_port = NULL;

/** The baud rate to use. */
static uint32_t baud_rate = 0;

/** Usage string */
#define OTA_DFU_USAGE "OTA_DFU_Usage: %s ota [serial port] [baud rate] [ota file] [remote address] [(optional) force write without response: 0 / 1] [optional max mtu]\n\n"
/**/
/** dfu file to upload*/
FILE *dfu_file = NULL;
/** remote device address*/
static uint8_t remote_address_type;
static bd_addr remote_address;
static bd_addr remote_public_address;
static uint8_t addr_found = 0;
/** Force using write without response commands */
static uint32_t force_write_without_rsp = 1;
/*Bluetooth connection*/
uint8_t ble_connection;
/*found OTA descriptors*/
uint32_t ota_gatt_service_handle;
uint16_t ota_control_characteristic;
uint16_t ota_data_characteristic;
uint16_t ota_version_characteristic;
uint16_t apploader_version_characteristic;
uint16_t bootloader_version_characteristic;
uint16_t application_version_characteristic;
/*OTA UUIDS*/
uint8_t uuid_ota_service[] = { 0xf0, 0x19, 0x21, 0xb4, 0x47, 0x8f, 0xa4, 0xbf, 0xa1, 0x4f, 0x63, 0xfd, 0xee, 0xd6, 0x14, 0x1d }; //1d14d6ee-fd63-4fa1-bfa4-8f47b42119f0
uint8_t uuid_ota_control[] = { 0x63, 0x60, 0x32, 0xe0, 0x37, 0x5e, 0xa4, 0x88, 0x53, 0x4e, 0x6d, 0xfb, 0x64, 0x35, 0xbf, 0xf7 }; //F7BF3564-FB6D-4E53-88A4-5E37E0326063
uint8_t uuid_ota_data[] = { 0x53, 0xa1, 0x81, 0x1f, 0x58, 0x2c, 0xd0, 0xa5, 0x45, 0x40, 0xfc, 0x34, 0xf3, 0x27, 0x42, 0x98 }; //984227F3-34FC-4045-A5D0-2C581F81A153


uint8_t uuid_bootloader_version[] = { 0xfe, 0x5a, 0x24, 0xe1, 0x2b, 0xaa, 0xa5, 0xb2, 0xe9, 0x46, 0x17, 0xe9, 0x0a, 0x5c, 0xf0, 0x25 };//25f05c0a-e917-46e9-b2a5-aa2be1245afe
uint8_t uuid_apploader_version[] = { 0x9f, 0x3e, 0xe2, 0x2e, 0x0e, 0xcf, 0xff, 0xbf, 0x1e, 0x45, 0xca, 0x8c, 0x68, 0x23, 0x4a, 0x4f };//4f4a2368-8cca-451e-bfff-cf0e2ee23e9f
uint8_t uuid_ota_version[] = { 0x16, 0x53, 0x1e, 0xc4, 0x4c, 0xba, 0xad, 0x9d, 0x32, 0x4b, 0x68, 0x08, 0xcf, 0x7b, 0xc0, 0x4c };//4cc07bcf-0868-4b32-9dad-ba4cc41e5316
uint8_t uuid_application_version[] = { 0xf8, 0x92, 0x7a, 0xac, 0x96, 0xcd, 0xa9, 0xbf, 0xf2, 0x49, 0xc1, 0x4a, 0x11, 0xcc, 0x77, 0x0d };//0d77cc11-4ac1-49f2-bfa9-cd96ac7a92f8

/*Error macro*/
#define ERROR_EXIT(...) do { printf(__VA_ARGS__); exit(EXIT_FAILURE); } while (0)

#define GAP_ADDR_TYPE 0x1b

/*
   Enumeration of possible OTA states
 */
enum ota_states {
  OTA_INIT,   // default state
  OTA_CONNECT,  // connect to remote device
  OTA_FIND_SERVICES,  // find OTA control&data attributes
  OTA_FIND_CHARACTERISTICS,  // find OTA control&data attributes
  OTA_READ_OTA_DATA_PROPERTIES,  // Read OTA Data characteristic properties
  OTA_BEGIN,  // Begin OTA process
  OTA_UPLOAD_WITHOUT_RSP,  // Upload data to remote device using write without response
  OTA_UPLOAD_WITH_RSP,  // Upload data to remote device using write with response
  OTA_END,  // End current OTA process
  OTA_RESTART,  // End current OTA process
  OTA_RESET_TO_DFU,  // Reset to DFU mode
  OTA_SCAN, // Scan for device
  OTA_READ_BOOTLOADER_VERSION, //Read bootloader version
  OTA_READ_APPLOADER_VERSION, //Read apploader version
  OTA_READ_OTA_VERSION, //Read OTA version
  OTA_READ_APPLICATION_VERSION, //Read application version
} ota_state = OTA_INIT;
void ota_change_state(enum ota_states new_state);
///DFU
#define MAX_DFU_PACKET 256
uint8_t dfu_data[MAX_DFU_PACKET];
bool dfu_resync = false;
size_t dfu_toload = 0;
size_t dfu_total = 0;
size_t dfu_current_pos = 0;
time_t dfu_start_time;

#define MAX_MTU 247
#define MIN_MTU 23
uint32_t mtu = MIN_MTU;
uint32_t max_mtu = MAX_MTU;
uint8_t ota_data_properties = 0;
uint32_t bootloader_version = -1;
uint32_t application_version = -1;
uint16_t apploader_version[4] = { 0 };
uint8_t ota_version = 0;
//




/**
 * UART DFU Configuration
 */

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define MAXRETRANS 25

/** Usage string */
#define UART_DFU_USAGE "\nUART_DFU_Usage: %s uart [serial port] [baud rate] [dfu file] \n"

#define MAX_UART_DFU_PACKET 132
unsigned char txbuff[MAX_UART_DFU_PACKET];
uint8_t rxbuff; 

void app_init(int argc, char* argv[]);
void upload_dfu_file(void);
void sync_dfu_boot(void);
int dfu_read_size(void);
int xmodemTransmit(char *src, int srcsz);
static void on_message_send(uint32_t msg_len, uint8_t* msg_data);

static const unsigned short crc16tab[256]= {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};
  
unsigned short crc16_ccitt(const void *buf, int len)
{
	register int counter;
	register unsigned short crc = 0;
	for( counter = 0; counter < len; counter++)
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *(char *)buf++)&0x00FF];
	return crc;
}

static int _inbyte(void) // msec timeout
{
	int dataRead;
	dataRead = uartRxNonBlocking(1, &rxbuff);
	// dataRead = uartRx(1, &rxbuff);
	if(dataRead != 0)
		printf("inbyte is 0x%x, %c \n", rxbuff, rxbuff);
	
    return rxbuff ;
}

static void _outbyte(int c)
{
	uartTx(1, (uint8_t*)&c);
	// printf("outbyte is 0x%x, %c \n", c, c);
}

static void flushinput(void)
{
	while (_inbyte() >= 0)
		;
}

int get_option(void)
{
	int option;
	do{
		// scanf("%d", &option);
		option = getchar();

		// if(option < '1' || option > '3'){
		// 	printf("wrong value, please retry \n");
		// }
	}while(option < '1' || option > '3');

	return option;
}

int UART_hw_init(int argc, char* argv[])
{
  if (argc < 5) {
    printf(UART_DFU_USAGE, argv[0]);
    printf(OTA_DFU_USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }
  /**
   * Handle the command-line arguments.
   */

  //filename
  dfu_file = fopen(argv[4], "rb");
  if (dfu_file == NULL) {
    printf("cannot open file %s\n", argv[4]);
    exit(-1);
  }
  baud_rate = atoi(argv[3]);
  uart_port = argv[2];

  if (!uart_port || !baud_rate || !dfu_file) {
    printf(UART_DFU_USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }

  /**
   * Initialise the serial port.
   */
  return uartOpen((int8_t*)uart_port, baud_rate, 1, 100);
}

void menu_init(void)
{
	int option;
	printf("\r\nMenu\r\n");
	printf("1. upload gbl\r\n");
	printf("2. run\r\n");
	printf("3. ebl info\r\n");
	printf("BL >");

	option = get_option();
	switch(option){
		case '1':
			  	_outbyte('1'); // MENU start INIT_TRANSFER
				printf("begin upload \n");
				break;
		case '2':
				_outbyte('2'); // MENU start INIT_TRANSFER
        sl_bt_dfu_reset(0);
				printf("begin boot \n");
				break;
		case '3':
				_outbyte('3'); // MENU start INIT_TRANSFER
				printf("return to MENU \n");
				break;
		default:
				_outbyte('3'); // MENU start INIT_TRANSFER
				printf("return to MENU \n");
				break;			
	}

}



int main_uart(int argc, char* argv[])
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, processing task(s) will be created by
  // this call.
  sl_system_init();

  // Initialize the application. For example, create periodic timer(s) or
  // task(s) if the kernel is present.
  app_init(argc, argv);

  // Do not remove this call: Silicon Labs components process action routine
  // must be called from the super loop.
  // sl_system_process_action();

  upload_dfu_file();

}


void upload_dfu_file()
{
  int st;

  /** get dfu file size*/
  if (dfu_read_size()) {
    printf("Error, DFU file read failed\n");
    exit(EXIT_FAILURE);
  }

  /** move target to dfu mode*/
  sync_dfu_boot();

  printf ("Prepare your terminal emulator to receive data now...\n");

  uint8_t dfu_data[dfu_total];

  if (fread(dfu_data, 1, dfu_total, dfu_file) != dfu_total) {
    printf("File read failure\n");
    exit(EXIT_FAILURE);
  }
  printf ("Read the DFU file successfully.\n");

  st = xmodemTransmit((char *)dfu_data, dfu_total);
	if (st < 0) {
		printf ("Xmodem transmit error: status: %d\n", st); 
		fflush(stdout);
	}
	else  {
		printf ("Xmodem successfully transmitted %d bytes\n", st); 
		fflush(stdout);
	}
  printf("fclose, close the dfu file \r\n");
  fclose(dfu_file);

  menu_init();

  // sl_bt_dfu_reset(0);

  printf("Close the uart port \r\n");
  
  if(uartClose() != 0)		// close the uart handler
  {
  	printf("Close the uart port successfully\n");
  }


}


void sync_dfu_boot()
{
	printf("Syncing"); 
  	fflush(stdout);
	
  do{
		printf("."); 
    	fflush(stdout);

    	// reset the device, and make it enter bootloader mode.
		printf("->->->-> sl_bt_system_reset ...\n");
		sl_bt_system_reset(1);

		//get_action()
		menu_init();

		int receivechar[100];
		char transferInitStr[12] = "begin upload";
		int i;

		for(uint8_t try=0; try < 50; try++)
		{
			for(i = 0; i < 100; i++)
			{
				receivechar[i] = _inbyte();
			}

			for(int i = 0; i < 100; i++)
			{
				if (receivechar[i] == 'b')
				{
					if(memcmp(&receivechar[i], &transferInitStr[0],12) == 0)
					{
						printf("DFU OK\n");
						fflush(stdout);
						return;
					}
					else
						return;
				}
			}

			if(i == 15)
				return;

		}

		// sleep 1s, and then try to reset the device again
		sleep(1);

	} while(1);

}

/***************************************************************************//**
 * Application initialisation.
 ******************************************************************************/
void app_init(int argc, char* argv[])
{
  SL_BT_API_INITIALIZE_NONBLOCK(on_message_send, uartRx, uartRxPeek);

  //////////////////////////////////////////
  // Add your application init code here! //
  //////////////////////////////////////////

  if (UART_hw_init(argc, argv) < 0) {
    printf("HW init failure\n");
    exit(EXIT_FAILURE);
  }
}
/***************************************************************************//**
 * Application process actions.
 ******************************************************************************/
void app_process_action(void)
{
  //////////////////////////////////////////
  // Add your application tick code here! //
  //////////////////////////////////////////
}

int xmodemTransmit(char *src, int srcsz)
{
	// unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	int maxPayloadSize, crc = -1;
	// unsigned char packetno = 1;
	
	int i= 0;

	uint32_t packetno = 1;
	uint32_t packetLen = 0;
	static uint32_t sendOffset = 0;
	uint16_t retry;
	uint8_t rspVal = 0;		// The response from device after receiving one xmodem packet

	printf("---------------------\n");

	for(;;) {
		for( retry = 0; retry < 100; ++retry) {
			// printf("retry %d\n", retry);
			if ((rspVal = _inbyte()) >= 0) {
				switch (rspVal) {
					case 'C':
						crc = 1;
						printf("go to start_trans ->->->-  \n");
						goto start_trans;
					case NAK:
						crc = 0;
						goto start_trans;
					case CAN:
						if ((rspVal = _inbyte()) == CAN) {
							_outbyte(ACK);
							flushinput();
							return -1; /* canceled by remote */
						}
						break;
					default:
						break;
				}
			}
		}

		printf(">>>return and reset >>>\n");
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		// flushinput();
		return -2; /* no sync */


		for(;;) {
		start_trans:

			printf("\n\n>> start trans <<\n");
#ifdef TRANSMIT_XMODEM_1K
			txbuff[0] = STX; 
			maxPayloadSize = 1024;
#else
			txbuff[0] = SOH; 
			maxPayloadSize = 128;
#endif
			txbuff[1] = packetno;
			txbuff[2] = ~packetno;
			
			packetLen = srcsz - sendOffset;

			if (packetLen > maxPayloadSize) 
				packetLen = maxPayloadSize;

			printf("srcsz %d, sendOffset %d, packetLen %d \n", srcsz, sendOffset, packetLen);

			if (packetLen > 0) {

				memset (&txbuff[3], 0, maxPayloadSize);
				memcpy (&txbuff[3], &src[sendOffset], packetLen);
				if (packetLen < maxPayloadSize){
					txbuff[3+packetLen] = CTRLZ;
					printf("---- It's a short packet, packet length is %d---- \n", packetLen);
				}

				if (crc) {
					unsigned short ccrc = crc16_ccitt(&txbuff[3], maxPayloadSize);
					txbuff[maxPayloadSize + 3] = (ccrc >> 8) & 0xFF;
					txbuff[maxPayloadSize + 4] = ccrc & 0xFF;
					printf("CRC is 0x%04x\n", ccrc);
				}
				else {
					unsigned char ccks = 0;				// check sum
					for (i = 3; i < maxPayloadSize + 3; ++i) {
						ccks += txbuff[i];
					}
					txbuff[maxPayloadSize+3] = ccks;
				}

				for (retry = 0; retry < MAXRETRANS; ++retry) {			// If didn't get ACK from remote device, will try to re-transmit the packet up to MAXRETRANS times
					
					printf("retry %d, pakcet dump:\n", retry);
					for (i = 0; i < maxPayloadSize+4+(crc?1:0); ++i) 
					{
						_outbyte(txbuff[i]);
						
						if(i!=0 && i%16 == 0)
							printf("\n");
						printf("0x%02x ", txbuff[i]);
					}

					printf("\r\npacket number = %d\r\n", packetno);

					// if ((c = _inbyte()) >= 0 ) {
					rspVal = _inbyte();
					printf("response value is 0x%x (0x01 SOH; 0x04 EOT; 0x06 ACK; 0x15 NAK; 0x18 CAN)\n", rspVal);
					if (rspVal >= 0 ) {						
						switch (rspVal) {
							case ACK:
								++packetno;
								// sendOffset += maxPayloadSize;
								sendOffset += packetLen;		// 
								goto start_trans;
							case CAN:
								if ((rspVal = _inbyte()) == CAN) {
									_outbyte(ACK);
									// flushinput();	// cheng
									return -1; /* canceled by remote */
								}
								break;
							case NAK:
							default:
								break;
						}
					}
				}
				printf("Didn't get response from remote device after MAXRETRANS retry to the specified packet\r\n");
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				// flushinput();		// cheng
				return -4; /* xmit error */
			}
			else {				// end of transfer
					for (retry = 0; retry < 10; ++retry) 
					{
						_outbyte(EOT);
						if ((rspVal = _inbyte()) == ACK) 
							break;
					}
					//flushinput();
					return (rspVal == ACK)?sendOffset:-5;
				
			}
		}
	}
}





int dfu_read_size()
{
  if (fseek(dfu_file, 0L, SEEK_END)) {
    return -1;
  }
  dfu_total = dfu_toload = ftell(dfu_file);
  if (fseek(dfu_file, 0L, SEEK_SET)) {
    return -1;
  }
  printf("Bytes to send:%d\n", (int)dfu_toload); fflush(stdout);
  return 0;
}

void send_dfu_block()
{
  time_t ti;
  size_t dfu_size;
  sl_bt_msg_t evt;

  while (dfu_toload > 0) {
    sl_status_t sc;
    uint16_t sent_len;

    dfu_size = dfu_toload > (mtu - 3) ? (mtu - 3) : dfu_toload;
    if (fread(dfu_data, 1, dfu_size, dfu_file) != dfu_size) {
      printf("File read failure\n");
      exit(-1);
    }

    do {
      sc = sl_bt_pop_event(&evt);
      sc = sl_bt_gatt_write_characteristic_value_without_response(ble_connection, ota_data_characteristic, dfu_size, dfu_data, &sent_len);
    } while (sc != SL_STATUS_OK);

    if (sc) {
      ERROR_EXIT("Error,%s, characteristic write failed:0x%x", __FUNCTION__, sc);
    }

    dfu_current_pos += dfu_size;
    dfu_toload -= dfu_size;

    ti = time(NULL);
    if (ti != dfu_start_time && dfu_total > 0) {
      printf("\r%d%% %.2fkbit/s                   ",
             (int)(100 * dfu_current_pos / dfu_total),
             dfu_current_pos * 8.0 / 1000.0 / difftime(ti, dfu_start_time));
    }
  }

  printf("\n");
  printf("time: %.2fs", difftime(ti, dfu_start_time));

  printf("\n");
  ota_change_state(OTA_END);
}

void send_dfu_packet_with_confirmation()
{
  time_t ti;
  size_t dfu_size;

  if (dfu_toload > 0) {
    sl_status_t sc;

    dfu_size = dfu_toload > (mtu - 3) ? (mtu - 3) : dfu_toload;
    if (fread(dfu_data, 1, dfu_size, dfu_file) != dfu_size) {
      printf("File read failure\n");
      exit(-1);
    }

    sc = sl_bt_gatt_write_characteristic_value(ble_connection, ota_data_characteristic, dfu_size, dfu_data);

    if (sc) {
      ERROR_EXIT("Error,%s, characteristic write failed:0x%x", __FUNCTION__, sc);
    }

    dfu_current_pos += dfu_size;
    dfu_toload -= dfu_size;

    ti = time(NULL);
    if (ti != dfu_start_time && dfu_total > 0) {
      printf("\r%d%% %.2fkbit/s                   ",
             (int)(100 * dfu_current_pos / dfu_total),
             dfu_current_pos * 8.0 / 1000.0 / difftime(ti, dfu_start_time));
    }
  } else {
    ti = time(NULL);
    printf("\n");
    printf("time: %.2fs", difftime(ti, dfu_start_time));

    printf("\n");
    ota_change_state(OTA_END);
  }
}

/*
 */
void sync_boot()
{
  sl_status_t sc;
  sl_bt_msg_t evt;

  // Flush std output
  fflush(stdout);
  // Reset NCP to ensure it gets into a defined state.
  // Once the chip successfully boots, boot event should be received.
  sl_bt_system_reset(0);
  do {
    sc = sl_bt_pop_event(&evt);
    if (!sc) {
      switch (SL_BT_MSG_ID(evt.header)) {
        case sl_bt_evt_system_boot_id:
          printf("System rebooted\n"); fflush(stdout);
          sl_bt_evt_system_boot_t *p = \
            &evt.data.evt_system_boot;
          printf("NCP version: v%d.%d.%d-b%d\n",
                 p->major,
                 p->minor,
                 p->patch,
                 p->build);
          return;
      }
    }
  } while (1);
}
/*

 */
void ota_change_state(enum ota_states new_state)
{
  ota_state = new_state;
  switch (ota_state) {
    case OTA_END:
    {
      printf("Finishing DFU block..."); fflush(stdout);
      {
        sl_status_t sc;
        sc = sl_bt_gatt_write_characteristic_value(ble_connection, ota_control_characteristic, 1, (uint8_t*)"\x03");
        if (sc) {
          ERROR_EXIT("Error,%s, characteristic write failed,0x%x", __FUNCTION__, sc);
        }
      }
    }
    break;

    case OTA_UPLOAD_WITHOUT_RSP:
      send_dfu_block();
      break;

    case OTA_UPLOAD_WITH_RSP:
      send_dfu_packet_with_confirmation();
      break;

    case OTA_BEGIN:
    {
      sl_status_t sc;
      sc = sl_bt_gatt_write_characteristic_value(ble_connection, ota_control_characteristic, 1, (uint8_t*)"\0");
      if (sc) {
        ERROR_EXIT("Error,%s, characteristic write failed,0x%x", __FUNCTION__, sc);
      }
      printf("DFU mode..."); fflush(stdout);
      dfu_start_time = time(NULL);
    }
    break;

    case OTA_FIND_CHARACTERISTICS:
    {
      sl_status_t sc;
      sc = sl_bt_gatt_discover_characteristics(ble_connection, ota_gatt_service_handle);
      if (sc) {
        ERROR_EXIT("Error, characteristics discover failed,0x%x", sc);
      }
      printf("Discovering characteristics..."); fflush(stdout);
      ota_control_characteristic = 0xFFFF;
      ota_data_characteristic = 0xFFFF;
      bootloader_version_characteristic = 0xFFFF;
      apploader_version_characteristic = 0xFFFF;
      ota_version_characteristic = 0xFFFF;
    }
    break;

    case OTA_READ_OTA_DATA_PROPERTIES:
    {
      sl_status_t sc;
      sc = sl_bt_gatt_read_characteristic_value_from_offset(ble_connection, ota_data_characteristic - 1, 0, 1);
      if (sc) {
        ERROR_EXIT("Error,%s, characteristic read failed,0x%x", __FUNCTION__, sc);
      }
    }
    break;

    case OTA_FIND_SERVICES:
    {
      sl_status_t sc;
      sc = sl_bt_gatt_discover_primary_services_by_uuid(ble_connection, sizeof(uuid_ota_service), uuid_ota_service);
      if (sc) {
        ERROR_EXIT("Error, service discover failed,0x%x", sc);
      }
      printf("Discovering services..."); fflush(stdout);
      ota_gatt_service_handle = 0xFFFFFFFF;
    }
    break;

    case OTA_SCAN:
    {
      addr_found = 0;
      sl_bt_scanner_start(gap_1m_phy, scanner_discover_generic);
      printf("Scanning..."); fflush(stdout);
    }
    break;

    case OTA_CONNECT:
    {
      sl_status_t sc;
      uint16_t set_mtu;
      uint8_t connection;
      sc = sl_bt_gatt_set_max_mtu(max_mtu, &set_mtu);
      if (sc) {
        ERROR_EXIT("Error, set max MTU failed,0x%x", sc);
      }
      //set default connection parameters
      sc = sl_bt_connection_set_default_parameters(6, 6, 0, 300, 0, 0xffff);
      if (sc) {
        ERROR_EXIT("Error, set default connection parameters failed,0x%x", sc);
      }

      //move to connect state, connect to device address
      sc = sl_bt_connection_open(remote_address, remote_address_type, gap_1m_phy, &connection);
      if (sc) {
        ERROR_EXIT("Error, open failed,0x%x", sc);
      }
      ble_connection = connection;
      printf("Connecting..."); fflush(stdout);
    }
    break;

    case OTA_INIT:
      sync_boot();
      {
        sl_status_t sc;
        bd_addr address;
        uint8_t address_type;
        sc = sl_bt_system_get_identity_address(&address, &address_type);
        if (sc) {
          ERROR_EXIT("Error, failed to get Bluetooth address,0x%x", sc);
        }
        printf("Local %s address: %02x:%02x:%02x:%02x:%02x:%02x\n",
               address_type ? "static random" : "public device",
               address.addr[5],
               address.addr[4],
               address.addr[3],
               address.addr[2],
               address.addr[1],
               address.addr[0]);
      }
      if (dfu_read_size()) {
        ERROR_EXIT("Error, DFU file read failed\n");
      }
      ota_change_state(OTA_SCAN);
      break;

    case OTA_RESET_TO_DFU:
    {
      sl_status_t sc;
      sc = sl_bt_gatt_write_characteristic_value(ble_connection, ota_control_characteristic, 1, (uint8_t*)"\0");
      if (sc) {
        ERROR_EXIT("Error,%s, characteristic write failed,0x%x", __FUNCTION__, sc);
      }
    }
    break;

    case OTA_READ_OTA_VERSION:
    {
      sl_status_t sc;
      sc = sl_bt_gatt_read_characteristic_value(ble_connection, ota_version_characteristic);
      if (sc) {
        ERROR_EXIT("Error,%s, characteristic write failed,0x%x", __FUNCTION__, sc);
      }
    }
    break;

    case OTA_READ_APPLOADER_VERSION:
    {
      sl_status_t sc;
      sc = sl_bt_gatt_read_characteristic_value(ble_connection, apploader_version_characteristic);
      if (sc) {
        ERROR_EXIT("Error,%s, characteristic write failed,0x%x", __FUNCTION__, sc);
      }
    }
    break;

    case OTA_READ_BOOTLOADER_VERSION:
    {
      sl_status_t sc;
      sc = sl_bt_gatt_read_characteristic_value(ble_connection, bootloader_version_characteristic);
      if (sc) {
        ERROR_EXIT("Error,%s, characteristic write failed,0x%x", __FUNCTION__, sc);
      }
    }
    break;

    case OTA_READ_APPLICATION_VERSION:
    {
      sl_status_t sc;
      sc = sl_bt_gatt_read_characteristic_value(ble_connection, application_version_characteristic);
      if (sc) {
        ERROR_EXIT("Error,%s, characteristic write failed,0x%x", __FUNCTION__, sc);
      }
    }
    break;

    default:
      break;
  }
}

/**
 * Function called when a message needs to be written to the serial port.
 * @param msg_len Length of the message.
 * @param msg_data Message data, including the header.
 * @param data_len Optional variable data length.
 * @param data Optional variable data.
 */
static void on_message_send(uint32_t msg_len, uint8_t* msg_data)
{
  /** Variable for storing function return values. */
  int ret;

#if DEBUG
  printf("on_message_send()\n");
#endif /* DEBUG */

  ret = uartTx(msg_len, msg_data);
  if (ret < 0) {
    printf("on_message_send() - failed to write to serial port %s, ret: %d, errno: %d\n", uart_port, ret, errno);
    exit(EXIT_FAILURE);
  }
}

void print_address(bd_addr address)
{
  for (int i = 5; i >= 0; i--) {
    printf("%02x", address.addr[i]);
    if (i > 0) {
      printf(":");
    }
  }
}

static int parse_scan_data(uint8array *data, bd_addr *addr)
{
  uint8_t i = 0;
  while (i < data->len) {
    if (data->data[i + 1] == GAP_ADDR_TYPE) {
      memcpy(addr, &data->data[i + 3], sizeof(bd_addr));
      return 0;
    } else {
      i += data->data[i] + 1;
    }
  }

  return -1;
}

int parse_address(const char *str, bd_addr *addr)
{
  int a[6];
  int i;
  i = sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
             &a[5],
             &a[4],
             &a[3],
             &a[2],
             &a[1],
             &a[0]
             );
  if (i != 6) {
    return -1;
  }

  for (i = 0; i < 6; i++) {
    addr->addr[i] = (uint8_t)a[i];
  }

  return 0;
}
int hw_init(int argc, char* argv[])
{
  if (argc < 6) {
    printf(UART_DFU_USAGE, argv[0]);
    printf(OTA_DFU_USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }
  /**
   * Handle the command-line arguments.
   */

  //filename
  dfu_file = fopen(argv[4], "rb");
  if (dfu_file == NULL) {
    printf("cannot open file %s\n", argv[4]);
    exit(-1);
  }
  //baudrate
  baud_rate = atoi(argv[3]);
  //uart port
  uart_port = argv[2];
  //remote address
  if (parse_address(argv[5], &remote_public_address)) {
    printf("Unable to parse address %s", argv[5]);
    exit(EXIT_FAILURE);
  }

  if (argc >= 7) {
    force_write_without_rsp = atoi(argv[5]);
  }

  if (argc == 8) {
    max_mtu = atoi(argv[7]);
    if (max_mtu > MAX_MTU) {
      max_mtu = MAX_MTU;
    } else if (max_mtu < MIN_MTU) {
      max_mtu = MIN_MTU;
    }
    printf("MTU set to %u\n", max_mtu);
  }
  /**
   * Initialise the serial port.
   */
  return uartOpen((int8_t*)uart_port, baud_rate, 1, 100);
}

/**
 * The main program.
 */
int main_ota(int argc, char* argv[])
{
  sl_bt_msg_t evt;
  sl_bt_msg_t *p = &evt;
  /**
   * Initialize BGLIB with our output function for sending messages.
   */

  SL_BT_API_INITIALIZE_NONBLOCK(on_message_send, uartRx, uartRxPeek);

  if (hw_init(argc, argv) < 0) {
    printf("HW init failure\n");
    exit(EXIT_FAILURE);
  }
  ota_change_state(OTA_INIT);
  while (1) {
    sl_bt_wait_event(&evt);

    if (p && SL_BT_MSG_ID(p->header) == sl_bt_evt_gatt_mtu_exchanged_id) {
      mtu = p->data.evt_gatt_mtu_exchanged.mtu;
      printf("ATT MTU exchanged: %d\n", mtu);
      continue;
    }

    switch (ota_state) {
      case OTA_END:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_connection_closed_id:
            printf("OK\n"); fflush(stdout);
            exit(EXIT_SUCCESS);
            break;

          case sl_bt_evt_gatt_procedure_completed_id:
            if (p->data.evt_gatt_procedure_completed.result) {
              ERROR_EXIT("Error, OTA DFU failed,0x%x", p->data.evt_gatt_procedure_completed.result);
            }
            printf("OK\n"); fflush(stdout);
            printf("Closing connection..."); fflush(stdout);
            {
              sl_status_t sc;
              sc = sl_bt_connection_close(ble_connection);
              if (sc) {
                ERROR_EXIT("Error,%s, close failed,0x%x", __FUNCTION__, sc);
              }
            }
            break;

          default:

            break;
        }
        break;

      case OTA_UPLOAD_WITHOUT_RSP:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_procedure_completed_id:
            if (p->data.evt_gatt_procedure_completed.result) {
              ERROR_EXIT("procedure failed:0x%x\r\n", p->data.evt_gatt_procedure_completed.result);
            }
            send_dfu_block();
            break;

          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;

          default:
            break;
        }
        break;

      case OTA_UPLOAD_WITH_RSP:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_procedure_completed_id:
            if (p->data.evt_gatt_procedure_completed.result) {
              ERROR_EXIT("procedure failed:0x%x\r\n", p->data.evt_gatt_procedure_completed.result);
            }
            send_dfu_packet_with_confirmation();
            break;

          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;

          default:
            break;
        }
        break;

      case OTA_BEGIN:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_procedure_completed_id:
            printf("OK\n"); fflush(stdout);
            if ((ota_data_properties & 0x0C) == 0) {
              ERROR_EXIT("Wrong supported OTA Data properties\r\n");
            } else {
              if (force_write_without_rsp == 1 && ota_data_properties & 0x04) {     //Write without response is supported and forced
                printf("OTA DFU - write without response \n"); fflush(stdout);
                ota_change_state(OTA_UPLOAD_WITHOUT_RSP);
              } else {
                printf("OTA DFU - write with response \n"); fflush(stdout);
                ota_change_state(OTA_UPLOAD_WITH_RSP);
              }
            }
            break;

          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;

          default:
            break;
        }
        break;

      case OTA_CONNECT:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_connection_opened_id:
            printf("OK\n"); fflush(stdout);
            ota_change_state(OTA_FIND_SERVICES);
            break;

          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;

          default:
            break;
        }
        break;

      case OTA_FIND_SERVICES:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_procedure_completed_id:
            if (ota_gatt_service_handle == 0xFFFFFFFF) {
              ERROR_EXIT("Error, no valid OTA service found");
            }

            printf("OK\n"); fflush(stdout);
            ota_change_state(OTA_FIND_CHARACTERISTICS);
            break;

          case sl_bt_evt_gatt_service_id:
            ota_gatt_service_handle = p->data.evt_gatt_service.service;
            break;

          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;

          default:
            break;
        }
        break;

      case OTA_FIND_CHARACTERISTICS:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_procedure_completed_id:
            if (ota_control_characteristic == 0xFFFF) {
              ERROR_EXIT("Error, no valid OTA characteristics found");
            } else if (ota_data_characteristic == 0xFFFF) {
              printf("\nRestarting target.");
              ota_change_state(OTA_RESET_TO_DFU);
              break;
            }

            printf("OK\n    Control handle:%d\n    Data handle:%d\n", ota_control_characteristic, ota_data_characteristic); fflush(stdout);
            if (ota_version_characteristic != 0xFFFF) {
              ota_change_state(OTA_READ_OTA_VERSION);
            } else {
              ota_change_state(OTA_READ_OTA_DATA_PROPERTIES);
            }
            break;

          case sl_bt_evt_gatt_characteristic_id:
            if (p->data.evt_gatt_characteristic.uuid.len == sizeof(uuid_ota_control)
                && !memcmp(p->data.evt_gatt_characteristic.uuid.data, uuid_ota_control, sizeof(uuid_ota_control))) {
              ota_control_characteristic = p->data.evt_gatt_characteristic.characteristic;
            } else if (p->data.evt_gatt_characteristic.uuid.len == sizeof(uuid_ota_data)
                       && !memcmp(p->data.evt_gatt_characteristic.uuid.data, uuid_ota_data, sizeof(uuid_ota_data))) {
              ota_data_characteristic = p->data.evt_gatt_characteristic.characteristic;
            } else if (p->data.evt_gatt_characteristic.uuid.len == sizeof(uuid_bootloader_version)
                       && !memcmp(p->data.evt_gatt_characteristic.uuid.data, uuid_bootloader_version, sizeof(uuid_bootloader_version))) {
              bootloader_version_characteristic = p->data.evt_gatt_characteristic.characteristic;
            } else if (p->data.evt_gatt_characteristic.uuid.len == sizeof(uuid_apploader_version)
                       && !memcmp(p->data.evt_gatt_characteristic.uuid.data, uuid_apploader_version, sizeof(uuid_apploader_version))) {
              apploader_version_characteristic = p->data.evt_gatt_characteristic.characteristic;
            } else if (p->data.evt_gatt_characteristic.uuid.len == sizeof(uuid_ota_version)
                       && !memcmp(p->data.evt_gatt_characteristic.uuid.data, uuid_ota_version, sizeof(uuid_ota_version))) {
              ota_version_characteristic = p->data.evt_gatt_characteristic.characteristic;
            } else if (p->data.evt_gatt_characteristic.uuid.len == sizeof(uuid_application_version)
                       && !memcmp(p->data.evt_gatt_characteristic.uuid.data, uuid_application_version, sizeof(uuid_bootloader_version))) {
              application_version_characteristic = p->data.evt_gatt_characteristic.characteristic;
            }
            break;

          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;

          default:
            break;
        }
        break;

      case OTA_READ_OTA_DATA_PROPERTIES:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_characteristic_value_id:
            if (p->data.evt_gatt_characteristic_value.value.len == 1) {
              ota_data_properties = p->data.evt_gatt_characteristic_value.value.data[0];
            }
            printf("    OTA Data characteristic properties:0x%02x\n", ota_data_properties); fflush(stdout);
            break;

          case sl_bt_evt_gatt_procedure_completed_id:
            ota_change_state(OTA_BEGIN);
            break;

          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;

          default:
            break;
        }
        break;

      case OTA_RESET_TO_DFU:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_connection_closed_id:
            printf("\nConnection closed, retrying. (Remote device booting in DFU mode)\n"); fflush(stdout);
            ota_change_state(OTA_SCAN);
            break;
          default:
            break;
        }
        break;

      case OTA_SCAN:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_scanner_scan_report_id:
            if (!addr_found) {
              bd_addr addr;
              if (!memcmp(&p->data.evt_scanner_scan_report.address, &remote_public_address, sizeof(bd_addr))) {
                memcpy(&remote_address, &p->data.evt_scanner_scan_report.address, sizeof(bd_addr));
                remote_address_type = p->data.evt_scanner_scan_report.address_type;
                addr_found = 1;
              } else if (parse_scan_data(&p->data.evt_scanner_scan_report.data, &addr) == 0) {
                if (!memcmp(&addr, &remote_public_address, sizeof(bd_addr))) {
                  memcpy(&remote_address, &p->data.evt_scanner_scan_report.address, sizeof(bd_addr));
                  remote_address_type = p->data.evt_scanner_scan_report.address_type;
                  addr_found = 1;
                }
              }
              if (addr_found) {
                sl_bt_scanner_stop();
                printf("OK\n"); fflush(stdout);
                printf("Device address found, connecting.\n"); fflush(stdout);
                ota_change_state(OTA_CONNECT);
              }
            }
            break;
          default:
            break;
        }
        break;

      case OTA_READ_OTA_VERSION:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_characteristic_value_id:
            if (p->data.evt_gatt_characteristic_value.value.len == 1) {
              memcpy(&ota_version, p->data.evt_gatt_characteristic_value.value.data, p->data.evt_gatt_characteristic_value.value.len);
              printf("    OTA protocol version:0x%02x\n", ota_version); fflush(stdout);
            }
            break;
          case sl_bt_evt_gatt_procedure_completed_id:
            ota_change_state(OTA_READ_APPLOADER_VERSION);
            break;
          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;
          default:
            break;
        }
        break;

      case OTA_READ_APPLOADER_VERSION:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_characteristic_value_id:
            if (p->data.evt_gatt_characteristic_value.value.len == 8) {
              memcpy(&apploader_version, p->data.evt_gatt_characteristic_value.value.data, p->data.evt_gatt_characteristic_value.value.len);
              if (ota_version >= 3) {
                printf("    Apploader version:%d.%d.%d.%d\n", apploader_version[0], apploader_version[1], apploader_version[2], apploader_version[3]); fflush(stdout);
              } else {
                printf("    Bluetooth stack version:%d.%d.%d.%d\n", apploader_version[0], apploader_version[1], apploader_version[2], apploader_version[3]); fflush(stdout);
              }
            }
            break;
          case sl_bt_evt_gatt_procedure_completed_id:
            if (ota_version >= 3) {
              ota_change_state(OTA_READ_BOOTLOADER_VERSION);
            } else {
              ota_change_state(OTA_READ_OTA_DATA_PROPERTIES);
            }
            break;
          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;
          default:
            break;
        }
        break;

      case OTA_READ_BOOTLOADER_VERSION:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_characteristic_value_id:
            if (p->data.evt_gatt_characteristic_value.value.len == 4) {
              memcpy(&bootloader_version, p->data.evt_gatt_characteristic_value.value.data, p->data.evt_gatt_characteristic_value.value.len);
              printf("    Bootloader version:0x%08x\n", bootloader_version); fflush(stdout);
            }
            break;
          case sl_bt_evt_gatt_procedure_completed_id:
            if (bootloader_version == 0) {
              ERROR_EXIT("\nError, no bootloader present");
            }
            ota_change_state(OTA_READ_APPLICATION_VERSION);
            break;
          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;
          default:
            break;
        }
        break;

      case OTA_READ_APPLICATION_VERSION:
        switch (SL_BT_MSG_ID(p->header)) {
          case sl_bt_evt_gatt_characteristic_value_id:
            if (p->data.evt_gatt_characteristic_value.value.len == 4) {
              memcpy(&application_version, p->data.evt_gatt_characteristic_value.value.data, p->data.evt_gatt_characteristic_value.value.len);
              printf("    Application version:0x%08x\n", application_version); fflush(stdout);
            }
            break;
          case sl_bt_evt_gatt_procedure_completed_id:
            ota_change_state(OTA_READ_OTA_DATA_PROPERTIES);
            break;
          case sl_bt_evt_connection_closed_id:
            ERROR_EXIT("\nError, Connection closed, reason 0x%x", p->data.evt_connection_closed.reason);
            break;
          default:
            break;
        }
        break;

      default:
        break;
    }
  }

  fclose(dfu_file);
  return 0;
}




int main(int argc, char* argv[])
{

  if (strcmp(argv[1],"uart") == 0)
  {
      main_uart(argc, argv);
  }else if (strcmp(argv[1],"ota") == 0)
  {
      main_ota(argc, argv);
  }
  else{
    printf(UART_DFU_USAGE, argv[0]);
    printf(OTA_DFU_USAGE, argv[0]);
  }
  
}