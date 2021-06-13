/* MTI_DPI */

/*
 * Copyright 2002-2019 Mentor Graphics Corporation.
 *
 * Note:
 *   This file is automatically generated.
 *   Please do not edit this file - you will lose your edits.
 *
 * Settings when this file was generated:
 *   PLATFORM = 'linux_x86_64'
 */
#ifndef INCLUDED_DPI_FUNCTIONS
#define INCLUDED_DPI_FUNCTIONS

#ifdef __cplusplus
#define DPI_LINK_DECL  extern "C" 
#else
#define DPI_LINK_DECL 
#endif

#include "svdpi.h"



DPI_LINK_DECL DPI_DLLESPEC
char
get_key();

DPI_LINK_DECL DPI_DLLESPEC
char
key_ready();

DPI_LINK_DECL DPI_DLLESPEC
int
send_char_to_terminal(
    int outchar);

DPI_LINK_DECL DPI_DLLESPEC
int
start_external_terminal();

#endif 
