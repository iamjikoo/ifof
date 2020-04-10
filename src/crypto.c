#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "aes_local.h"

#define MIN(x,y) (((x)>(y))?(y):(x))
#define MAX(x,y) (((x)>(y))?(x):(y))

#define AES256_KEY "conestoga.capstone.team7.ifof."
#define AES_BLOCK_SIZE	16

/*
** AES-256 encrypt
** Return the length of the encrypted data (a multiple of AES block size).
*/
int aes256_enc( unsigned char *plain, int plain_len, unsigned char *crypt, int crypt_len )
{
	AES_KEY encrypt_key;
	char in[AES_BLOCK_SIZE];
	int len;
	int pad;
	
	memset( crypt, 0x00, crypt_len );
	len = plain_len;
	if ( len == 0 )
		return 0;

	//* Padding size and also padding value.
	pad = AES_BLOCK_SIZE - (len % AES_BLOCK_SIZE);
	
	//* Set the encryption key.
	if ( AES_set_encrypt_key( (unsigned char*)AES256_KEY, 256, &encrypt_key ) != 0 )
		return -1;

	//* Encrypt with block cipher.
	int offset;
	for( offset = 0; offset < len; offset+=AES_BLOCK_SIZE ) {
		if ( len - offset < AES_BLOCK_SIZE )
			memset( in, pad, sizeof(in) );
		memcpy( in, plain+offset, MIN(AES_BLOCK_SIZE,len-offset) );
		AES_encrypt( (unsigned char*)in, (unsigned char*)crypt+offset, &encrypt_key );
	}

	//* Whole padding block if plain length is a multiple of the block size.
	if ( pad == AES_BLOCK_SIZE ) {
		memset( in, pad, sizeof(in) );
		AES_encrypt( (unsigned char*)in, (unsigned char*)crypt+offset, &encrypt_key );
		offset += AES_BLOCK_SIZE;
	}

	//* Return the cipher data and the length of the cipher data.
	return offset;
}


/*
** AES-256 decrypt
** Return the length of plain (decrypted) data.
*/
int aes256_dec( unsigned char *crypt, int crypt_len, char *plain)
{
    AES_KEY decrypt_key;

	if ( AES_set_decrypt_key( (unsigned char*)AES256_KEY, 256, &decrypt_key ) != 0 )
		return -1;

	int block_num = crypt_len / AES_BLOCK_SIZE;
	int offset = 0, i;
	for( i = 0; i < block_num; i++, offset+=AES_BLOCK_SIZE )
		AES_decrypt( (unsigned char*)crypt+offset, (unsigned char*)plain+offset, &decrypt_key );

	//* Remove padding and add EOS.
	int pad = plain[offset-1];
	plain[offset-pad] = 0;

	//* Return the plain length.
	return (offset-pad);
}
