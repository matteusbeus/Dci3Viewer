/**
 *  \file tools.h
 *  \brief Misc tools methods
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides some unpack functions for compression used by the SGDK
 */

#ifndef _TOOLS_H_
#define _TOOLS_H_

long aplib_unpack(unsigned short *src, unsigned short *dest);

long lz4w_unpack(const unsigned short *src, unsigned short *dest);

#endif // _TOOLS_H_
