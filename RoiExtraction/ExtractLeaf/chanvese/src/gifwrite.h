/**
 * @file gifwrite.h
 * @brief Write animated GIF files
 * @author Pascal Getreuer <getreuer@gmail.com>
 *
 * Copyright (c) 2011, Pascal Getreuer
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under, at your option, the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version, or the terms of the
 * simplified BSD license.
 *
 * You should have received a copy of these licenses along with this program.
 * If not, see <http://www.gnu.org/licenses/> and
 * <http://www.opensource.org/licenses/bsd-license.html>.
 */

#ifndef _GIFWRITE_H_
#define _GIFWRITE_H_

int GifWrite(unsigned char **Image,
    int ImageWidth, int ImageHeight, int NumFrames,
    const unsigned char *Palette, int NumColors, int TransparentColor,
    const int *Delays, const char *OutputFile);
void FrameDifference(unsigned char **Image,
    int ImageWidth, int ImageHeight, int NumFrames, int TransparentColor);

#endif
