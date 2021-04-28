/**********************************************************************
* Author:	jaron.ho
* Date:		2018-09-29
* Brief:	v4l2 capture
**********************************************************************/
#ifndef V4L2_CAPTURE_H
#define V4L2_CAPTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Brief:	open video device
 * Param:	devName - device name, e.g. "/dev/video0"
 *			width - must > 0, may be modified
 *			height - must > 0, may be modified
 * Return:	void
 */
extern void v4l2_open(const char* devName, unsigned int* width, unsigned int* height);

/*
 * Brief:	close video device
 * Param:	void
 * Return:	void
 */
extern void v4l2_close(void);

/*
 * Brief:	switch capture flow
 * Param:	enable - 0.stop,1.start
 * Return:	void
 */
extern void v4l2_enable(int enable);

/*
 * Brief:	capture current frame
 * Param:	length - frame length
 * Return:	void*
 */
extern void* v4l2_capture(size_t* length);

/*
 * Brief:	show device information
 * Param:	void
 * Return:	void
 */
extern void v4l2_info(void);

#ifdef __cplusplus
}
#endif

#endif  // V4L2_CAPTURE_H
