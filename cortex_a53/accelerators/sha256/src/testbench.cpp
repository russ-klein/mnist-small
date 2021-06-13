/*********************************************************************
 * Filename:   sha256.c
 * Author:     Brad Conte (brad AT bradconte.com)
 * Copyright:
 * Disclaimer: This code is presented "as is" without any guarantees.
 * Details:    Performs known-answer tests on the corresponding SHA1
 implementation. These tests do not encompass the full
 range of available test vectors, however, if the tests
 pass it is very, very likely that the code is correct
 and was compiled properly. This code also serves as
 example usage of the functions.
 *********************************************************************/

/*************************** HEADER FILES ***************************/
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "ac_channel.h"
#include "sha256_defines.h"
#include "sha256_classdef.h"

/*********************** FUNCTION DEFINITIONS ***********************/

int strlen(const BYTE *s)
{
   int count = 0;
   while (*s++) count++;

   return count;
}


int sha256_test()
{
    BYTE text1[] = {"abc"};
    BYTE text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
    BYTE text3[] = {"aaaaaaaaaa"};
    BYTE hash1[SHA256_BLOCK_SIZE] = {0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
        0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad};
    BYTE hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
        0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};
    //BYTE hash3[SHA256_BLOCK_SIZE] = {0xcd,0xc7,0x6e,0x5c,0x99,0x14,0xfb,0x92,0x81,0xa1,0xc7,0xe2,0x84,0xd7,0x3e,0x67,
    //    0xf1,0x80,0x9a,0x48,0xa4,0x97,0x20,0x0e,0x04,0x6d,0x39,0xcc,0xc7,0x11,0x2c,0xd0};
    BYTE hash3[SHA256_BLOCK_SIZE] = {0x41,0xed,0xec,0xe4,0x2d,0x63,0xe8,0xd9,0xbf,0x51,0x5a,0x9b,0xa6,0x93,0x2e,0x1c,
        0x20,0xcb,0xc9,0xf5,0xa5,0xd1,0x34,0x64,0x5a,0xdb,0x5d,0xb1,0xb9,0x73,0x7e,0xa3};
    BYTE buf[SHA256_BLOCK_SIZE];
    int idx;
    int pass = 1;
    int i;

    sha256 sha256_inst;

    ac_channel<BYTE> data_in;
    ac_channel<BYTE> data_out;

#ifdef HANDSHAKE

    ac_channel<bool> go;
    ac_channel<bool> done;
    
    go.write(true);
    sha256_inst.run(go, done, data_in, 0, 0, data_out);
    done.read();
    for (i=0; i<strlen(text1); i++) data_in.write(text1[i]);
    go.write(true);
    sha256_inst.run(go, done, data_in, strlen(text1), 1, data_out);
    done.read();
    go.write(true);
    sha256_inst.run(go, done, data_in, 0, 2, data_out);
    done.read();
    for (i=0; i<SHA256_BLOCK_SIZE; i++) buf[i] = data_out.read();
    pass = pass && !memcmp(hash1, buf, SHA256_BLOCK_SIZE);
    
    go.write(true);
    sha256_inst.run(go, done, data_in, 0, 0, data_out);
    done.read();
    for (i=0; i<strlen(text2); i++) data_in.write(text2[i]);
    go.write(true);
    sha256_inst.run(go, done, data_in, strlen(text2), 1, data_out);
    done.read();
    go.write(true);
    sha256_inst.run(go, done, data_in, 0, 2, data_out);
    done.read();
    for (i=0; i<SHA256_BLOCK_SIZE; i++) buf[i] = data_out.read();
    pass = pass && !memcmp(hash2, buf, SHA256_BLOCK_SIZE);
    
    go.write(true);
    sha256_inst.run(go, done, data_in, 0, 0, data_out);
    done.read();
    for (idx = 0; idx < 100; ++idx) {
        for (i=0; i<strlen(text3); i++) data_in.write(text3[i]);
        go.write(true);
        sha256_inst.run(go, done, data_in, strlen(text3), 1, data_out);
        done.read();
    }
    go.write(true);
    sha256_inst.run(go, done, data_in, 0, 2, data_out);
    done.read();
    for (i=0; i<SHA256_BLOCK_SIZE; i++) buf[i] = data_out.read();
    pass = pass && !memcmp(hash3, buf, SHA256_BLOCK_SIZE);

#else
    
    sha256_inst.run(data_in, 0, 0, data_out);
    for (i=0; i<strlen(text1); i++) data_in.write(text1[i]);
    sha256_inst.run(data_in, strlen(text1), 1, data_out);
    sha256_inst.run(data_in, 0, 2, data_out);
    for (i=0; i<SHA256_BLOCK_SIZE; i++) buf[i] = data_out.read();
    pass = pass && !memcmp(hash1, buf, SHA256_BLOCK_SIZE);
    
    sha256_inst.run(data_in, 0, 0, data_out);
    for (i=0; i<strlen(text2); i++) data_in.write(text2[i]);
    sha256_inst.run(data_in, strlen(text2), 1, data_out);
    sha256_inst.run(data_in, 0, 2, data_out);
    for (i=0; i<SHA256_BLOCK_SIZE; i++) buf[i] = data_out.read();
    pass = pass && !memcmp(hash2, buf, SHA256_BLOCK_SIZE);
    
    sha256_inst.run(data_in, 0, 0, data_out);
    for (idx = 0; idx < 100; ++idx) {
        for (i=0; i<strlen(text3); i++) data_in.write(text3[i]);
        sha256_inst.run(data_in, strlen(text3), 1, data_out);
    }
    sha256_inst.run(data_in, 0, 2, data_out);
    for (i=0; i<SHA256_BLOCK_SIZE; i++) buf[i] = data_out.read();
    pass = pass && !memcmp(hash3, buf, SHA256_BLOCK_SIZE);

#endif 

    return(pass);
}

int main()
{
    printf("SHA-256 tests: %s\n", sha256_test() ? "SUCCEEDED" : "FAILED");
    
    return(0);
}
