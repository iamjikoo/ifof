#include <string.h>
#include <ctype.h>

//#include "main.h"
#include "common.h"

#include "hal_nrf.h"

/*******************************************************/
//	rfPDU type 
/*
	 |<------------- 32 bytes ----------------->|
	   1B        16B     1B       14B
	 +--------+--------+--------+---------------+
	 | opcode |  id    | length | message       |
	 +--------+--------+----+-------------------+ */

typedef struct {
	uint8_t opcode; // command type
#define OP_COMMAND	1
#define OP_RELAY    2
	char    id[16]; // device id
	uint8_t length; // length of the message
	char    message[14]; // message contents
} rfPDU_t;

/*******************************************************/

static const uint8_t address[HAL_NRF_AW_5BYTES] = {0x22,0x33, 0x44,0x55,0x01};
char device_id[16] = {"OF1_123_456_789"};
int fRfReading= 0;

static radio_status_t status;

uint32_t deviceID()
{
   uint32_t firstBit= 0;
   firstBit = HAL_GetUIDw0();
   printf(".FIRTBIT VAL - %04lx\n", firstBit);

	 //printf("halver=%lx\n", HAL_GetHalVersion());
	 //printf("hal_RevId=%lx\n", HAL_GetREVID());
	 //printf("hal_DevId=%lx\n", HAL_GetDEVID());
	 printf("hal_UIDw0=%lx\n", HAL_GetUIDw0());
	 printf("hal_UIDw1=%lx\n", HAL_GetUIDw1());
	 printf("hal_UIDw2=%lx\n", HAL_GetUIDw2());

   return firstBit;
}

void radio_set_status (radio_status_t new_status)
{
	if (status != new_status) {
	  //printf("RF state:%d->%d\n\n", status, new_status);
    status = new_status;
	}
}

void radio_init_mode(const uint8_t *address, hal_nrf_operation_mode_t operational_mode)
{
  hal_nrf_close_pipe(HAL_NRF_ALL);               // First close all radio pipes
                                                 // Pipe 0 and 1 open by default
  hal_nrf_open_pipe(HAL_NRF_PIPE0, false);       // Open pipe0, without/autoack
  hal_nrf_set_crc_mode(HAL_NRF_CRC_16BIT);       // Operates in 16bits CRC mode
  hal_nrf_set_auto_retr(0, 250);                  // Disables auto retransmit

  hal_nrf_set_address_width(HAL_NRF_AW_5BYTES);  // 5 bytes address width
  hal_nrf_set_address(HAL_NRF_TX, address);      // Set device's addresses
  hal_nrf_set_address(HAL_NRF_PIPE0, address);   // Sets recieving address on 
                                                 // pipe0  

  if(operational_mode == HAL_NRF_PTX) {          // Mode depentant settings
    hal_nrf_set_operation_mode(HAL_NRF_PTX);     // Enter TX mode
		printf("nRF: TX mode\r\n");
  } else {
		int pload = 32;
    hal_nrf_set_operation_mode(HAL_NRF_PRX);     // Enter RX mode
    hal_nrf_set_rx_pload_width((uint8_t)HAL_NRF_PIPE0, pload);
		printf("nRF: RX mode, pload=%d\r\n", pload);
                                                 // Pipe0 expect 
                                                 // PAYLOAD_LENGTH byte payload
                                                 // PAYLOAD_LENGTH in radio.h
  }

  hal_nrf_set_rf_channel(40);                    // Operating on static channel 
                                                 // Defined in radio.h. 
                                                 // Frequenzy = 
                                                 //        2400 + RF_CHANNEL

  hal_nrf_set_power_mode(HAL_NRF_PWR_UP);        // Power up device

//hal_nrf_set_datarate(HAL_NRF_1MBPS);           // Uncomment this line for 
                                                 // compatibility with nRF2401 
                                                 // and nRF24E1

  radio_set_status (RF_IDLE);                    // Radio now ready

  if(operational_mode == HAL_NRF_PRX){
    CE_HIGH();
  } else {
    CE_LOW();
	}
}

void radio_set_rx_mode(void)
{
  hal_nrf_set_operation_mode(HAL_NRF_PRX);
  hal_nrf_set_rx_pload_width((uint8_t)HAL_NRF_PIPE0, 32);
  CE_HIGH();
}

void radio_set_tx_mode(void)
{
  hal_nrf_set_operation_mode(HAL_NRF_PTX);
  CE_LOW();
}

void radio_init(void) 
{
  radio_init_mode(address, HAL_NRF_PRX); /* set default mode */
}

int radio_send_data(char *data, uint8_t length) 
{

  CE_LOW();

	/* set rf mode to tx */

	if (hal_nrf_get_operation_mode() != HAL_NRF_PTX) {
    radio_set_tx_mode();
		//printf("changed mode: %d -> %d\r\n", hal_nrf_get_operation_mode(), HAL_NRF_PTX);
	}

	/* build packet */
	rfPDU_t data_tx;

	data_tx.opcode = 1;
	sprintf(data_tx.id, device_id);
	data_tx.length = length;

	for (int i = 0; i < 14; i++) {
    if (i >= length) {
		  data_tx.message[i] = ' '; // enter space
		} else {
      data_tx.message[i] = data[i];
		}
	}

	hal_nrf_write_tx_pload((uint8_t*)&data_tx, 32);
	CE_PULSE();

	//printf("\r\nsend to (%s) len(%d) msg(%s)\n\r", data_r.id, length, data);

	return length;
}

static uint16_t _radio_recv_packet(uint8_t *packet)
{
   uint16_t payload_length = 0;
   uint16_t PLL = hal_nrf_read_rx_pload(packet);
   payload_length = (PLL & 0x00FF); // take the bottom 8 bit
   //printf("%d -- payload\n",payload_length);
   return payload_length;
}

#if 0
int radio_is_active() {
   if( (reg = hal_nrf_get_rx_data_source()) < 7)
}
#endif

int radio_recv_data(char *rdata, uint8_t length)
{
	uint8_t buffer[32];

  rfPDU_t *pRf;

  CE_HIGH();

  /* set rf mode to rx */
	if (hal_nrf_get_operation_mode() != HAL_NRF_PRX) {
    radio_set_rx_mode();
	}

  length = _radio_recv_packet((uint8_t*)buffer);
	if (length > 0) {

#if 0
	  /* parse packet */
		printf("Data in hex:\n");
		for (uint8_t i = 1; i <= length; i++){
				printf("%02X  ", buffer[i-1]);
				if (i % 16 == 0) printf("\n");
		}
		//printf("\n");
#endif

		pRf = (rfPDU_t *)buffer;

		//printf("id=%s op=%d msg=", pRf->id, pRf->opcode);
		int i;
		for (i = 0; i < pRf->length && i < length; i++) {
			//printf("%c", pRf->message[i]);
			rdata[i] = pRf->message[i];
		}
		rdata[i] = '\0';
		//printf("\n");
		//printf("Data Received - %s\n", rxData_P);
	}

	//return length;
	return pRf->length;
}

void rfReadingPacket()
{
	if (!fRfReading)
		return;

	char buffer[32];

  radio_recv_data(buffer, sizeof(buffer));
}
//ADD_TASK(rfReadingPacket, NULL, NULL, 100, "receive rf data");

ParserReturnVal_t CmdStop(int mode)
{
  if(mode != CMD_INTERACTIVE) return CmdReturnOk;

	fRfReading = 0;

	return CmdReturnOk;
}
ADD_CMD("q",CmdStop,"quit receive packet")

#if 0
ParserReturnVal_t CmdReadReg(int mode)
{
  int rc;
  uint16_t length;
  uint8_t val;
  
  if(mode != CMD_INTERACTIVE) return CmdReturnOk;
  rc = fetch_uint16_arg(&length);
  if(rc) {
    printf("Must specify data value to the user\n");
    return CmdReturnBadParameter1;
  }

  val = hal_nrf_read_reg(length);
  printf("Read Reg: %X, VAL= %X\n", length, val);  
  
  return CmdReturnOk;
}

ADD_CMD("radioReg",CmdReadReg,"Initialise Radio, set 0 for transmission or 1 for receive")

ParserReturnVal_t CmdRadioVerify(int mode)
{
  
  uint8_t val;
  for (int i = 0; i< 0x7f; i++)
  {
    hal_nrf_write_reg(RF_CH, i);  
    val = hal_nrf_read_reg(RF_CH);  
    if(i != val)
    {
      printf("mismatch data exp :%02x  real ; %02x\r\n", i, val);
    }

    else 
    printf(".");
  }
  
  printf("\n");
  return CmdReturnOk;
}

ADD_CMD("radioverify",CmdRadioVerify,"Initialise Radio, set 0 for transmission or 1 for receive")

ParserReturnVal_t CmdDecode11(int mode)
{
  deviceID();
  return CmdReturnOk;
}

ADD_CMD("DecDevice",CmdDecode11,"Initialise Radio, set 0 for transmission or 1 for receive")
#endif

ParserReturnVal_t CmdRadio(int mode)
{
  int rc;
  char *n;
  char *subCommand;

  char *helpString = 
    "rf init rx | tx\n"
    "rf send <data> | recv\n"
    "rf reg | verify\n";
   
  
  if (mode == CMD_SHORT_HELP) {
    return CmdReturnOk;
  }

  // type 'help rf'
  if (mode == CMD_LONG_HELP) {
    rc = fetch_string_arg(&n);
    if (rc) {
      printf("%s", helpString);
    }
    return CmdReturnOk;
  }

  /* fetch sub command string */
  rc = fetch_string_arg(&subCommand);
  if (rc) {
    printf("Must specify subcommand one of these: init | reg | vfy | send | recv\n");
    return CmdReturnBadParameter2;
  }

  /* identify the sub commands and run the corresponding functions */
  if (strcasecmp(subCommand, "help")==0 || strcasecmp(subCommand, "?")==0) {
      printf("%s", helpString);
      return CmdReturnOk;
  } else if (strcasecmp(subCommand, "init")==0) {

    char *para;
    char mode;

    rc = fetch_string_arg(&para);
    if(rc) {
      radio_init();
      return CmdReturnOk;
    }
    
    if (strcasecmp(para, "rx")==0) {
      mode = HAL_NRF_PRX;
      radio_init_mode(address, mode);
    } else if (strcasecmp(para, "tx")==0) {
      mode = HAL_NRF_PTX;
      radio_init_mode(address, mode);
    } else {
      printf("Invalid mode '%s': set as default(rx)", para);
      radio_init();
    }

    return CmdReturnOk;

  } else if (strcasecmp(subCommand, "reg")==0) {
  } else if (strcasecmp(subCommand, "recv")==0) {
    fRfReading = 1;
  } else if (strcasecmp(subCommand, "send")==0) {

    char rc, *data;
    uint8_t length;

    rc = fetch_string_arg(&data);
    if(rc) {
      printf("Must specify data to send\n");
      return CmdReturnBadParameter3;
    }

    length = strlen(data);

		if (length > 14) {
			printf("Message too long: limited to 14 bytes\n");
      return CmdReturnBadParameter3;
		}

    radio_send_data(data, length);
    return CmdReturnOk;

  } else if (strcasecmp(subCommand, "set")==0) {
    char *para;

    rc = fetch_string_arg(&para);
    if(rc) {
      printf("Must specify mode: pload | etc\n");
      return CmdReturnBadParameter3;
    }
    
		/* set payload length */
    if (strcasecmp(para, "pload")==0) {
		  uint16_t size;

      rc = fetch_uint16_arg(&size);
      if(rc) {
        printf("Must specify payload length (4 ~ 32)\n");
        return CmdReturnBadParameter4;
      }
			if (size > 32) {
        printf("Too long, limited to max 32 bytes\n");
        return CmdReturnBadParameter4;
			}

      hal_nrf_set_rx_pload_width((uint8_t)HAL_NRF_PIPE0, size);

      return CmdReturnOk;

		/* set device id */
		} else if (strcasecmp(para, "id")==0) {

      char *data;

      rc = fetch_string_arg(&data);
      if(rc) {
        printf("Must specify id, e.g) OF2_123_456_789\n");
        return CmdReturnBadParameter3;
      }
			
		  if (strlen(data) > 15) {
        printf("Different format, e.g) OF2_123_456_789\n");
        return CmdReturnBadParameter3;
			}

			for (int i = 0; i <strlen(data); i++) {
				device_id[i] = toupper(data[i]);
			}
			device_id[strlen(data)-1] = '\0';
			//snprintf(device_id, sizeof(device_id), "%s", data);

      return CmdReturnOk;

		} else {
      printf("Command '%s' not found.\n", para);
      return CmdReturnOk;
		}

	/* show command */
  } else if (strcasecmp(subCommand, "show")==0) {
		int mode = hal_nrf_get_operation_mode();

		printf("\n");
		printf("----------------+-----------------------\n");
	  printf(" Operation Mode | %s\n", (mode==HAL_NRF_PRX)? "RX":"TX");
		//printf(" Payload length | %d bytes\n", hal_nrf_read_rx_pl_w());
		printf(" Payload length | %d bytes\n",
				hal_nrf_get_rx_pload_width(HAL_NRF_PIPE0));
		printf(" ID             | %s\n", device_id);
		printf("----------------+-----------------------\n");

  } else if (strcasecmp(subCommand, "enc")==0) {

  } else {
    printf("Command '%s' not found.\n", subCommand);
    return CmdReturnOk;
  }

  return CmdReturnOk;
}

ADD_CMD("rf",CmdRadio,"                Set and show rf module")
