/*
 * common.h
 *
 *  Created on: Oct 9, 2013
 *      Author: libin.eb
 */

#ifndef COMMON_H_
#define COMMON_H_

/***********************************************************************************
 * TYPEDEFS
 */

typedef signed   char   int8;
typedef unsigned char   uint8;

typedef signed   short  int16;
typedef unsigned short  uint16;

typedef signed   int 	int32;
typedef unsigned int  	uint32;

typedef void (*ISR_FUNC_PTR)(void);
typedef void (*VFPTR)(void);

/********************************Macros********************************************/
#define SUCCESS									0
#define FAILURE									1

#define TRUE									1
#define FALSE									0


#endif /* COMMON_H_ */
