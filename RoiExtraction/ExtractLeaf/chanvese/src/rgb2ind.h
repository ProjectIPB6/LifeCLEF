/**
 * @file rgb2ind.h
 * @brief Convert a truecolor RGB image to an indexed image
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

#ifndef _RGB2IND_H_
#define _RGB2IND_H_

int Rgb2Ind(unsigned char *Dest, unsigned char *Palette, int NumColors,
    const unsigned char *RgbImage, long NumPixels);

#endif /* _RGB2IND_H_ */
