/* 
	KIRK ENGINE CODE
	Thx for coyotebean, Davee, hitchhikr, kgsws, Mathieulh, SilverSpring
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "crypto.h"

#include "internal_keys.h"

/* ------------------------- INTERNAL STUFF ------------------------- */

typedef struct header_keys
{
    u8 AES[16];
    u8 CMAC[16];
}header_keys;  //small struct for temporary keeping AES & CMAC key from CMD1 header

u8 fuseID[16]; //Emulate FUSEID	

AES_ctx aes_kirk1; //global

char is_kirk_initialized; //"init" emulation

/* ------------------------- INTERNAL STUFF END ------------------------- */


/* ------------------------- IMPLEMENTATION ------------------------- */

int kirk_CMD0(void* outbuff, void* inbuff, int size, int generate_trash)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
	
    KIRK_CMD1_HEADER* header = (KIRK_CMD1_HEADER*)outbuff;
    
    memcpy(outbuff, inbuff, size);
    
	if(header->mode != KIRK_MODE_CMD1) return KIRK_INVALID_MODE;
	
	header_keys *keys = (header_keys *)outbuff; //0-15 AES key, 16-31 CMAC key
	
	//FILL PREDATA WITH RANDOM DATA
	if(generate_trash) kirk_CMD14(outbuff+sizeof(KIRK_CMD1_HEADER), header->data_offset);
	
	//Make sure data is 16 aligned
	int chk_size = header->data_size;
	if(chk_size % 16) chk_size += 16 - (chk_size % 16);
	
	//ENCRYPT DATA
	AES_ctx k1;
	AES_set_key(&k1, keys->AES, 128);
	
	AES_cbc_encrypt(&k1, inbuff+sizeof(KIRK_CMD1_HEADER)+header->data_offset, outbuff+sizeof(KIRK_CMD1_HEADER)+header->data_offset, chk_size);
	
	//CMAC HASHES
	AES_ctx cmac_key;
	AES_set_key(&cmac_key, keys->CMAC, 128);
	    
	u8 cmac_header_hash[16];
	u8 cmac_data_hash[16];
		
	AES_CMAC(&cmac_key, outbuff+0x60, 0x30, cmac_header_hash);
	
	AES_CMAC(&cmac_key, outbuff+0x60, 0x30 + chk_size + header->data_offset, cmac_data_hash);
	
	memcpy(header->CMAC_header_hash, cmac_header_hash, 16);
	memcpy(header->CMAC_data_hash, cmac_data_hash, 16);
	
	//ENCRYPT KEYS
	
	AES_cbc_encrypt(&aes_kirk1, inbuff, outbuff, 16*2);
	return KIRK_OPERATION_SUCCESS;
}

int kirk_CMD1(void* outbuff, void* inbuff, int size, int do_check)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
	
    KIRK_CMD1_HEADER* header = (KIRK_CMD1_HEADER*)inbuff;
	if(header->mode != KIRK_MODE_CMD1) return KIRK_INVALID_MODE;
	
	header_keys keys; //0-15 AES key, 16-31 CMAC key
	
	AES_cbc_decrypt(&aes_kirk1, inbuff, (u8*)&keys, 16*2); //decrypt AES & CMAC key to temp buffer
	
	// HOAX WARRING! I have no idea why the hash check on last IPL block fails, so there is an option to disable checking
	if(do_check)
	{
       int ret = kirk_CMD10(inbuff, size);
       if(ret != KIRK_OPERATION_SUCCESS) return ret;
    }
	
	AES_ctx k1;
	AES_set_key(&k1, keys.AES, 128);
	
	AES_cbc_decrypt(&k1, inbuff+sizeof(KIRK_CMD1_HEADER)+header->data_offset, outbuff, header->data_size);	
	
	return KIRK_OPERATION_SUCCESS;
}

int kirk_CMD4(void* outbuff, void* inbuff, int size)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
	
	KIRK_AES128CBC_HEADER *header = (KIRK_AES128CBC_HEADER*)inbuff;
	if(header->mode != KIRK_MODE_ENCRYPT_CBC) return KIRK_INVALID_MODE;
	if(header->data_size == 0) return KIRK_DATA_SIZE_ZERO;
	
	u8* key = kirk_4_7_get_key(header->keyseed);
	if(key == (u8*)KIRK_INVALID_SIZE) return KIRK_INVALID_SIZE;
	
	//Set the key
	AES_ctx aesKey;
	AES_set_key(&aesKey, key, 128);
 	AES_cbc_encrypt(&aesKey, inbuff+sizeof(KIRK_AES128CBC_HEADER), outbuff, size);
	
	return KIRK_OPERATION_SUCCESS;
}

int kirk_CMD7(void* outbuff, void* inbuff, int size)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
	
	KIRK_AES128CBC_HEADER *header = (KIRK_AES128CBC_HEADER*)inbuff;
	if(header->mode != KIRK_MODE_DECRYPT_CBC) return KIRK_INVALID_MODE;
	if(header->data_size == 0) return KIRK_DATA_SIZE_ZERO;
	
	u8* key = kirk_4_7_get_key(header->keyseed);
	if(key == (u8*)KIRK_INVALID_SIZE) return KIRK_INVALID_SIZE;
	
	//Set the key
	AES_ctx aesKey;
	AES_set_key(&aesKey, key, 128);
	
 	AES_cbc_decrypt(&aesKey, inbuff+sizeof(KIRK_AES128CBC_HEADER), outbuff, size);
	
	return KIRK_OPERATION_SUCCESS;
}

int kirk_CMD10(void* inbuff, int insize)
{
	if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
	
    KIRK_CMD1_HEADER* header = (KIRK_CMD1_HEADER*)inbuff;
    
	if(!(header->mode == KIRK_MODE_CMD1 || header->mode == KIRK_MODE_CMD2 || header->mode == KIRK_MODE_CMD3)) return KIRK_INVALID_MODE;
	if(header->data_size == 0) return KIRK_DATA_SIZE_ZERO;
	
	if(header->mode == KIRK_MODE_CMD1)
	{
        header_keys keys; //0-15 AES key, 16-31 CMAC key
        
        AES_cbc_decrypt(&aes_kirk1, inbuff, (u8*)&keys, 32); //decrypt AES & CMAC key to temp buffer
	    
	    AES_ctx cmac_key;
	    AES_set_key(&cmac_key, keys.CMAC, 128);
	    
		u8 cmac_header_hash[16];
		u8 cmac_data_hash[16];
		
		AES_CMAC(&cmac_key, inbuff+0x60, 0x30, cmac_header_hash);
	
		//Make sure data is 16 aligned
		int chk_size = header->data_size;
		if(chk_size % 16) chk_size += 16 - (chk_size % 16);
		AES_CMAC(&cmac_key, inbuff+0x60, 0x30 + chk_size + header->data_offset, cmac_data_hash);
	
		if(memcmp(cmac_header_hash, header->CMAC_header_hash, 16) != 0)
        {
            printf("header hash invalid\n");
            return KIRK_HEADER_HASH_INVALID;
        }
		if(memcmp(cmac_data_hash, header->CMAC_data_hash, 16) != 0)
        {
            printf("data hash invalid\n");
            return KIRK_DATA_HASH_INVALID;
        }
	
		return KIRK_OPERATION_SUCCESS;
	}
	return KIRK_SIG_CHECK_INVALID; //Checks for cmd 2 & 3 not included right now
}

int kirk_CMD11(void* outbuff, void* inbuff, int size)
{
    if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
	KIRK_SHA1_HEADER *header = (KIRK_SHA1_HEADER *)inbuff;
	if(header->data_size == 0 || size == 0) return KIRK_DATA_SIZE_ZERO;
	
    SHA1Context sha;
    SHA1Reset(&sha);
    size <<= 4;
    size >>= 4;
	size = size < header->data_size ? size : header->data_size;
    SHA1Input(&sha, inbuff+sizeof(KIRK_SHA1_HEADER), size);
    memcpy(outbuff, sha.Message_Digest, 16);
    return KIRK_OPERATION_SUCCESS;
}

int kirk_CMD14(void* outbuff, int size)
{
    if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
    int i;
    u8* buf = (u8*)outbuff;
    for(i = 0; i < size; i++)
    {
          buf[i] = rand()%255;
    }
    return KIRK_OPERATION_SUCCESS;
}

int kirk_init()
{
    AES_set_key(&aes_kirk1, kirk1_key, 128);
	is_kirk_initialized = 1;
	srand(time(0));
    return KIRK_OPERATION_SUCCESS;
}

u8* kirk_4_7_get_key(int key_type)
{
    switch(key_type)
	{
		case(0x03): return kirk7_key03; break;
		case(0x04): return kirk7_key04; break;
		case(0x05): return kirk7_key05; break;
		case(0x0C): return kirk7_key0C; break;
		case(0x0D): return kirk7_key0D; break;
		case(0x0E): return kirk7_key0E; break;
		case(0x0F): return kirk7_key0F; break;
		case(0x10): return kirk7_key10; break;
		case(0x11): return kirk7_key11; break;
		case(0x12): return kirk7_key12; break;
		case(0x38): return kirk7_key38; break;
		case(0x39): return kirk7_key39; break;
		case(0x3A): return kirk7_key3A; break;
		case(0x4B): return kirk7_key4B; break;
		case(0x53): return kirk7_key53; break;
		case(0x57): return kirk7_key57; break;
		case(0x5D): return kirk7_key5D; break;
		case(0x63): return kirk7_key63; break;
		case(0x64): return kirk7_key64; break;
		default: return (u8*)KIRK_INVALID_SIZE; break; //need to get the real error code for that, placeholder now :)
	}
}

int kirk_CMD1_ex(void* outbuff, void* inbuff, int size, KIRK_CMD1_HEADER* header)
{
    u8* buffer = (u8*)malloc(size);
    memcpy(buffer, header, sizeof(KIRK_CMD1_HEADER));
    memcpy(buffer+sizeof(KIRK_CMD1_HEADER), inbuff, header->data_size);
    int ret = kirk_CMD1(outbuff, buffer, size, 1);
    free(buffer);
    return ret;
}

int sceUtilsSetFuseID(void*fuse)
{
	memcpy(fuseID, fuse, 16);
	return 0;
}

int sceUtilsBufferCopyWithRange(void* outbuff, int outsize, void* inbuff, int insize, int cmd)
{
    switch(cmd)
    {
		case KIRK_CMD_DECRYPT_PRIVATE: 
             if(insize % 16) return SUBCWR_NOT_16_ALGINED;
             int ret = kirk_CMD1(outbuff, inbuff, insize, 1); 
             if(ret == KIRK_HEADER_HASH_INVALID) return SUBCWR_HEADER_HASH_INVALID;
             return ret;
             break;
		case KIRK_CMD_ENCRYPT_IV_0: return kirk_CMD4(outbuff, inbuff, insize); break;
		case KIRK_CMD_DECRYPT_IV_0: return kirk_CMD7(outbuff, inbuff, insize); break;
		case KIRK_CMD_PRIV_SIG_CHECK: return kirk_CMD10(inbuff, insize); break;
		case KIRK_CMD_SHA1_HASH: return kirk_CMD11(outbuff, inbuff, insize); break;
	}
	return -1;
}


int kirk_decrypt_keys(u8 *keys, void *inbuff)
{
	AES_cbc_decrypt(&aes_kirk1, inbuff, (u8*)keys, 16*2); //decrypt AES & CMAC key to temp buffer
	return 0;
}

int kirk_forge(u8* inbuff, int insize)
{
   KIRK_CMD1_HEADER* header = (KIRK_CMD1_HEADER*)inbuff;
   AES_ctx cmac_key;
   u8 cmac_header_hash[16];
   u8 cmac_data_hash[16];
   int chk_size;

   if(is_kirk_initialized == 0) return KIRK_NOT_INITIALIZED;
   if(!(header->mode == KIRK_MODE_CMD1 || header->mode == KIRK_MODE_CMD2 || header->mode == KIRK_MODE_CMD3)) return KIRK_INVALID_MODE;
   if(header->data_size == 0) return KIRK_DATA_SIZE_ZERO;

   if(header->mode == KIRK_MODE_CMD1){
      header_keys keys; //0-15 AES key, 16-31 CMAC key

      AES_cbc_decrypt(&aes_kirk1, inbuff, (u8*)&keys, 32); //decrypt AES & CMAC key to temp buffer
      AES_set_key(&cmac_key, keys.CMAC, 128);
      AES_CMAC(&cmac_key, inbuff+0x60, 0x30, cmac_header_hash);
      if(memcmp(cmac_header_hash, header->CMAC_header_hash, 16) != 0) return KIRK_HEADER_HASH_INVALID;

      //Make sure data is 16 aligned
      chk_size = header->data_size;
      if(chk_size % 16) chk_size += 16 - (chk_size % 16);
      AES_CMAC(&cmac_key, inbuff+0x60, 0x30 + chk_size + header->data_offset, cmac_data_hash);

      if(memcmp(cmac_data_hash, header->CMAC_data_hash, 16) != 0) {
      //printf("data hash invalid, correcting...\n");
    } else {
         printf("data hash is already valid!\n");
         return 100;
      }
      // Forge collision for data hash
    memcpy(cmac_data_hash,header->CMAC_data_hash,0x10);
    AES_CMAC_forge(&cmac_key, inbuff+0x60, 0x30+ chk_size + header->data_offset, cmac_data_hash);
      //printf("Last row in bad file should be :\n"); for(i=0;i<0x10;i++) printf("%02x", cmac_data_hash[i]);
      //printf("\n\n");

      return KIRK_OPERATION_SUCCESS;
   }
   return KIRK_SIG_CHECK_INVALID; //Checks for cmd 2 & 3 not included right now
}
