/**
 * @file rgb2ind.c
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

#include <stdio.h>
#include <string.h>
#include "rgb2ind.h"

/** @brief Bounding box struct for median cut color quantization */
typedef struct
{
    int Min[3];         /**< Minimum in red, green, and blue dimensions */
    int Max[3];         /**< Maximums                                   */
    double Average[3];    /**< Average color                              */
    long NumPixels;     /**< The number of pixels in the box            */
    long Volume;        /**< The volume of the box                      */
} bbox;


static long BoxVolume(bbox Box);
static void MedianSplit(bbox *NewBox, bbox *SplitBox,
    const unsigned char *RgbImage, long NumPixels);


/**
 * @brief Convert a truecolor RGB image to an indexed image
 * @param Dest where to store the indexed image
 * @param Palette where to store the palette
 * @param NumColors maximum number of colors to use
 * @param RgbImage the input RGB image
 * @param NumPixels number of pixels in RgbImage
 * @return 1 on success, 0 on failure
 *
 * This routine quantizes the colors of the input RgbImage to an indexed image
 * Dest using at most NumColors distinct colors.  RgbImage should be a
 * contiguous array of RGB triples,
 *
 *     ImageRgb[3*i + 0] = Red component of the ith pixel,
 *     ImageRgb[3*i + 1] = Green component of the ith pixel,
 *     ImageRgb[3*i + 2] = Blue component of the ith pixel.
 *
 * Dest should be an array with space for at least NumPixels elements, and
 * Palette should have space for at least 3*NumColors elements.  Dest and
 * Palette are filled such that the quantized image is
 *
 *     Palette[3*Dest[i] + 0] = Red component of the ith pixel,
 *     Palette[3*Dest[i] + 1] = Green component of the ith pixel,
 *     Palette[3*Dest[i] + 2] = Blue component of the ith pixel.
 *
 * The quantized image should approximate RgbImage.
 *
 * The quantization is performed using the median cut algorithm.  No dithering
 * is performed.
 */
int Rgb2Ind(unsigned char *Dest, unsigned char *Palette, int NumColors,
    const unsigned char *RgbImage, long NumPixels)
{
    float Merit, MaxMerit;
    const long NumEl = 3*NumPixels;
    long i, Dist, MinDist, Diff;
    int k, BestBox = 0, Channel, NumBoxes = 1;
    bbox Box[256];
    
    if(!Dest || !Palette || NumColors > 256 || !RgbImage || NumPixels <= 0)
        return 0;
    
    /* Determine the smallest box containing all pixels */
    Box[0].Min[0] = Box[0].Min[1] = Box[0].Min[2] = 255;
    Box[0].Max[0] = Box[0].Max[1] = Box[0].Max[2] = 0;
    
    for(i = 0; i < NumEl; i += 3)
        for(Channel = 0; Channel < 3; Channel++)
        {
            if(Box[0].Min[Channel] > RgbImage[i + Channel])
                Box[0].Min[Channel] = RgbImage[i + Channel];
            if(Box[0].Max[Channel] < RgbImage[i + Channel])
                Box[0].Max[Channel] = RgbImage[i + Channel];
        }
    
    Box[0].NumPixels = NumPixels;
    Box[0].Volume = BoxVolume(Box[0]);
    
    while(NumBoxes < NumColors)
    {
        MaxMerit = 0;
        
        /* Select a box to split */
        if(NumBoxes % 4 > 0)        /* Split according to NumPixels */
        {
            for(k = 0; k < NumBoxes; k++)
                if(Box[k].Volume > 2
                    && (Merit = (float)Box[k].NumPixels) > MaxMerit)
                {
                    MaxMerit = Merit;
                    BestBox = k;
                }
        }
        else                        /* Split according to NumPixels*Volume */
            for(k = 0; k < NumBoxes; k++)
                if(Box[k].Volume > 2
                    && (Merit = ((float)Box[k].NumPixels)
                    * ((float)Box[k].Volume)) > MaxMerit)
                {
                    MaxMerit = Merit;
                    BestBox = k;
                }
        
        /* Split the box */
        MedianSplit(&Box[NumBoxes], &Box[BestBox], RgbImage, NumPixels);
        NumBoxes++;
    }
    
    for(k = 0; k < NumBoxes; k++)
    {
        Box[k].Average[0] = Box[k].Average[1] = Box[k].Average[2] = 0;
        Box[k].NumPixels = 0;
    }
    
    /* Compute box averages */
    for(i = 0; i < NumEl; i += 3)
    {
        for(k = 0; k < NumBoxes; k++)
            if(Box[k].Min[0] <= RgbImage[i + 0]
                && RgbImage[i + 0] <= Box[k].Max[0]
                && Box[k].Min[1] <= RgbImage[i + 1]
                && RgbImage[i + 1] <= Box[k].Max[1]
                && Box[k].Min[2] <= RgbImage[i + 2]
                && RgbImage[i + 2] <= Box[k].Max[2])
                break;

        if(k == NumBoxes)
        {
            fprintf(stderr, "Color (%d,%d,%d) unassigned\n",
                    RgbImage[i + 0], RgbImage[i + 1], RgbImage[i + 2]);
            k = 0;
        }
        else
        {
            /* Accumate the average color for each box */
            Box[k].Average[0] += RgbImage[i + 0];
            Box[k].Average[1] += RgbImage[i + 1];
            Box[k].Average[2] += RgbImage[i + 2];
        }
        
        Box[k].NumPixels++;
    }
    
    /* Fill Palette with the box averages */
    for(k = 0; k < NumBoxes; k++)
        if(Box[k].NumPixels > 0)
            for(Channel = 0; Channel < 3; Channel++)
            {
                Box[k].Average[Channel] /= Box[k].NumPixels;
                
                if(Box[k].Average[Channel] < 0.5)
                    Palette[3*k + Channel] = 0;
                else if(Box[k].Average[Channel] >= 254.5)
                    Palette[3*k + Channel] = 255;
                else
                    Palette[3*k + Channel] =
                        (unsigned char)(Box[k].Average[Channel] + 0.5);
            }
        else
            for(Channel = 0; Channel < 3; Channel++)
                Palette[3*k + Channel] = 0;
    
    /* Assign palette indices to quantized pixels */
    for(i = 0; i < NumEl; i += 3)
    {
        /* Find the closest palette color */
        for(k = 0, MinDist = 1000000; k < NumBoxes; k++)
        {
            Diff = ((long)RgbImage[i + 0]) - Palette[3*k + 0];
            Dist = Diff * Diff;
            Diff = ((long)RgbImage[i + 1]) - Palette[3*k + 1];
            Dist += Diff * Diff;
            Diff = ((long)RgbImage[i + 2]) - Palette[3*k + 2];
            Dist += Diff * Diff;
            
            if(MinDist > Dist)
            {
                MinDist = Dist;
                BestBox = k;
            }
        }
        
        *Dest = BestBox;
        Dest++;
    }
    
    return 1;
}


/** @brief Compute the volume of a bbox */
static long BoxVolume(bbox Box)
{
    return (Box.Max[0] - Box.Min[0] + 1)
        * (Box.Max[1] - Box.Min[1] + 1)
        * (Box.Max[2] - Box.Min[2] + 1);
}


/** @brief Split a bbox along its longest dimension at the median */
static void MedianSplit(bbox *NewBox, bbox *SplitBox,
    const unsigned char *RgbImage, long NumPixels)
{
    bbox Box = *SplitBox;
    const long NumEl = 3*NumPixels;
    long i, Accum, Hist[256];
    int Length, MaxLength, MaxDim;
    
    /* Determine the longest box dimension */
    MaxLength = MaxDim = 0;
    
    for(i = 0; i < 3; i++)
        if((Length = Box.Max[i] - Box.Min[i] + 1) > MaxLength)
        {
            MaxLength = Length;
            MaxDim = i;
        }
    
    /* Build a histogram over MaxDim for pixels within Box */
    memset(Hist, 0, sizeof(long)*256);
    
    for(i = 0; i < NumEl; i += 3)
        if(Box.Min[0] <= RgbImage[i + 0] && RgbImage[i + 0] <= Box.Max[0]
            && Box.Min[1] <= RgbImage[i + 1] && RgbImage[i + 1] <= Box.Max[1]
            && Box.Min[2] <= RgbImage[i + 2] && RgbImage[i + 2] <= Box.Max[2])
            Hist[RgbImage[i + MaxDim]]++;
    
    Accum = Hist[i = Box.Min[MaxDim]];

    /* Set i equal to the median */
    while(2*Accum < Box.NumPixels && i < 254)
        Accum += Hist[++i];
    
    /* Adjust i so that the median is included with the larger partition */
    if(i > Box.Min[MaxDim]
        && ((i - Box.Min[MaxDim]) < (Box.Max[MaxDim] - i - 1)))
        Accum -= Hist[i--];
    
    /* Adjust i to ensure that boxes are not empty */
    for(; i >= Box.Max[MaxDim]; i--)
        Accum -= Hist[i];
    
    /* Split the boxes */
    *NewBox = Box;
    NewBox->Max[MaxDim] = i;
    NewBox->NumPixels = Accum;
    NewBox->Volume = BoxVolume(*NewBox);
    
    SplitBox->Min[MaxDim] = i + 1;
    SplitBox->NumPixels = Box.NumPixels - Accum;
    SplitBox->Volume = BoxVolume(*SplitBox);
}
