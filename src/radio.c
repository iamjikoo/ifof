#include <string.h>
#include <ctype.h>

//#include "main.h"
#include "common.h"

#include "hal_nrf.h"

/** Defines the payload length the radio should use */
#define RF_PAYLOAD_LENGTH 32                           

/*******************************************************/
//	rfPDU type 
/*
	 |<------------- 32 bytes ----------------->|
	   1B        mlen | 14B           16 B
	 +--------+--------+--------+---------------+
	 | opcode |  len  | id       | message       |
	 +--------+--------+----+-------------------+ */

typedef struct {
	uint8_t opcode:4; // command type
#define OP_COMMAND	1
#define OP_RELAY    2
	uint8_t crypto:1; // 1:enable 0; disable 
	uint8_t reserved1:1; 
	uint8_t reserved2:2;
	char    id[14]; // device id
	uint8_t length; // length of the message
	char    message[16]; // message contents
} rfPDU_t;


typedef struct {
  char auto_manual;  
#define RF_OPER_AUTO   0
#define RF_OPER_MANUAL 1 
#define RF_OPER_DEFAULT RF_OPER_AUTO

  char log_level;  /* log level */
	char dev_id[5]; /* device id */

	char crypto_enable; 
#define RF_CRYPTO_OFF 0
#define RF_CRYPTO_ON  1 
//#define RF_CRYPTO_DEFAULT RF_CRYPTO_ON
#define RF_CRYPTO_DEFAULT RF_CRYPTO_OFF

} radio_info_t;

radio_info_t *pRF; // global variable

/*******************************************************/

static const uint8_t address[HAL_NRF_AW_5BYTES] = {0x22,0x33, 0x44,0x55,0x01};
char device_id[14] = {"OF1_123_456_7"};
//int fRfReading= 0;


bool is_crypto_on()
{
  return (pRF? pRF->crypto_enable:0);
}

void crypto_onoff(int en)
{
	pRF->crypto_enable = en;
}

static radio_status_t status;

#if 0
uint32_t deviceID()
{
   uint32_t firstBit= 0;
   firstBit = HAL_GetUIDw0();
   printf(".FIRTBIT VAL - %04lx\n", firstBit);

	 printf("halver=%lx\n", HAL_GetHalVersion());
	 printf("hal_RevId=%lx\n", HAL_GetREVID());
	 printf("hal_DevId=%lx\n", HAL_GetDEVID());
	 printf("hal_UIDw0=%lx\n", HAL_GetUIDw0());
	 printf("hal_UIDw1=%lx\n", HAL_GetUIDw1());
	 printf("hal_UIDw2=%lx\n", HAL_GetUIDw2());

   return firstBit;
}
#endif

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
		LOG(1, "nRF TX mode");
  } else {
		int pload = RF_PAYLOAD_LENGTH;
    hal_nrf_set_operation_mode(HAL_NRF_PRX);     // Enter RX mode
    hal_nrf_set_rx_pload_width((uint8_t)HAL_NRF_PIPE0, pload);
		LOG(1, "nRF RX mode, pload=%d", pload);
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

  //deviceID();

}

void radio_set_rx_mode(void)
{
	uint8_t mode = hal_nrf_get_operation_mode();

	if (mode != HAL_NRF_PRX) {
    hal_nrf_set_operation_mode(HAL_NRF_PRX);
    hal_nrf_set_rx_pload_width((uint8_t)HAL_NRF_PIPE0, 32);
		LOG(1, "nRF mode:tx->%s",(hal_nrf_get_operation_mode())?"rx":"tx");
	}
  CE_HIGH();
}

void radio_set_tx_mode(void)
{
	uint8_t mode = hal_nrf_get_operation_mode();

  if (mode != HAL_NRF_PTX) {
    hal_nrf_set_operation_mode(HAL_NRF_PTX);
		LOG(1, "nRF mode:rx->%s",(hal_nrf_get_operation_mode())?"rx":"tx");
	}
  CE_LOW();
}

void radio_init()
{
  /* init RF variables */
  pRF = (radio_info_t *)malloc(sizeof(radio_info_t));

	pRF->auto_manual = RF_OPER_DEFAULT;
	pRF->log_level = 0;
	pRF->crypto_enable = RF_CRYPTO_DEFAULT;

  radio_init_mode(address, HAL_NRF_PRX); /* set default mode */
}

int radio_send_data(char *data, uint8_t length) 
{

	if (length >= 16) {
	  LOG(0, "length is limited to max 15 bytes");
		length = 15; /* adjust length */
	}

	/* set rf mode to tx */
  radio_set_tx_mode();
  CE_LOW();

	/* build packet */
	rfPDU_t data_tx;
	memset(&data_tx, 0x00, sizeof(data_tx));

	data_tx.opcode = 1;

	sprintf(data_tx.id, device_id);

	/* crypto data */
	unsigned char enc[128];
	int  enclen;
	if (is_crypto_on()) {
		enclen = aes256_enc((unsigned char*)data, length, enc, sizeof(enc));
		hexDump("crypto_enc", enc, enclen);
		data_tx.crypto = 1;
	  data_tx.length = enclen;
		memcpy(data_tx.message, enc, enclen);
		//ptx = (const char *)enc;
	} else {
		data_tx.crypto = 0;
	  data_tx.length = length;
		memcpy(data_tx.message, data, length);
		//ptx = (const char *)data;
	}

	hexDump("radio_send", (const void*)&data_tx, sizeof(data_tx));

	hal_nrf_write_tx_pload((uint8_t*)&data_tx, RF_PAYLOAD_LENGTH);
	CE_PULSE();

	LOG(1, "send to(%s) c(%d) len(%d) msg(%s)", data_tx.id, data_tx.crypto, length, data);

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

int radio_recv_data(char *rdata, uint8_t rsize)
{

	uint8_t buffer[32];

  rfPDU_t *pRx;

  /* set rf mode to rx */
  radio_set_rx_mode();

  int nread = _radio_recv_packet((uint8_t*)buffer);
	if (nread > 0) {
		printf("nread=%d\n", nread);

		hexDump("radio_recv", buffer, nread);

		/* parse pasckt */
		pRx = (rfPDU_t *)buffer;

		char dec[256];
		char *pRDATA;
		int  len;

		if (pRx->crypto) {
			len = aes256_dec((unsigned char *)pRx->message, pRx->length, dec);
	    hexDump("crypto_dec", dec, len);
			pRDATA = dec;
		} else {
			len = pRx->length;
			pRDATA = pRx->message;
		}

		int i;
		for (i = 0; i < len && i < rsize; i++) {
		  rdata[i] = pRDATA[i];
		}
		rdata[i] = '\0';

#if 0
		int i;
		for (i = 0; i < pRf->length && i < rsize; i++) {
			rdata[i] = pRf->message[i];
		}
		rdata[i] = '\0';
#endif

		LOG(1, "recv fr(%s) len(%d) msg(%s)", pRx->id, len, rdata);

	  //return pRf->length;
	  return len;
	}

	return 0;
}

void radio_run()
{
	if (pRF->auto_manual == RF_OPER_MANUAL) 
		return;

	char buffer[32];

  radio_recv_data(buffer, sizeof(buffer));
}
ADD_TASK(radio_run, radio_init, NULL, 1000, "nrf module task");

#if 0
ParserReturnVal_t CmdStop(int mode)
{
  if(mode != CMD_INTERACTIVE) return CmdReturnOk;

	fRfReading = 0;

	return CmdReturnOk;
}
ADD_CMD("q",CmdStop,"quit receive packet")
#endif

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
    //fRfReading = 1;
  } else if (strcasecmp(subCommand, "send")==0) {

    char rc, *data;
    uint8_t length;

    rc = fetch_string_arg(&data);
    if(rc) {
      printf("Must specify data to send\n");
      return CmdReturnBadParameter3;
    }

    length = strlen(data);

		if (length > 20) {
			printf("Message too long: limited to 20 bytes\n");
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
		printf("-------------------+------------------------------------\n");
	  printf(" Operational  Mode | %s\n", (mode==HAL_NRF_PRX)? "RX":"TX");
	  printf(" Transmission Mode | %s\n", (mode==HAL_NRF_PRX)? "RX":"TX");
		//printf(" Payload length  | %d bytes\n", hal_nrf_read_rx_pl_w());
		printf(" Payload length    | %d bytes\n",
				hal_nrf_get_rx_pload_width(HAL_NRF_PIPE0));
		printf(" ID                | %s\n", device_id);
		printf(" Encryption        | %s\n", (is_crypto_on()?"yes (aes256)":"no"));
		printf("-------------------+------------------------------------\n");
  } else if (strcasecmp(subCommand, "neighbour")==0) {

  } else if (strcasecmp(subCommand, "enc")==0) {
		char *para;
		
		rc = fetch_string_arg(&para);
		if(rc) {
			printf("Please set on | off\r\n");
			return CmdReturnBadParameter2;
		}

		if (strcasecmp(para, "on")==0) {
			crypto_onoff(RF_CRYPTO_ON);
		} else if (strcasecmp(para, "off")==0) {
			crypto_onoff(RF_CRYPTO_OFF);
		} else {
			printf("Invalid options '%s'", para);
			return CmdReturnBadParameter2;
		}

		return CmdReturnOk;

	} else {
		printf("Command '%s' not found.\n", subCommand);
		return CmdReturnOk;
	}

	return CmdReturnOk;
}

ADD_CMD("rf",CmdRadio,"                Set and show rf module")
