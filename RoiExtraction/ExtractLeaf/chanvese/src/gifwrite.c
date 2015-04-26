/**
 * @file gifwrite.c
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

#include <stdio.h>
#include <stdlib.h>

/** @brief Maximum number of bits allowed by GIF for encoded symbols */
#define MAXBITS         12
/** @brief Maximum code, equals 2^MAXBITS - 1 */
#define MAXCODE         4095
/** @brief Size of the hash table */
#define TABLESIZE       5003
/** @brief Shift value used for hashing */
#define HASHSHIFT       4
/** @brief Hash value for an unused table entry */
#define UNUSED          -1


/** @brief Entry in the compression hash table representing a string */
typedef struct
{
    long Hash;              /**< Hash value to identify the string       */
    unsigned short Code;    /**< Compression code assigned to the string */
} tableentry;

/** @brief Management for writing codes of variable bitlength to a file */
typedef struct
{
    FILE *File;                 /**< the stdio file stream                  */
    unsigned int BitsPerCode;   /**< number of bits per code                */
    unsigned int BitAccum;      /**< accumulator to combine bits into bytes */
    int NumBits;                /**< number of bits in the accumulator      */
    int BlockSize;              /**< number of bytes in the current block   */
    unsigned char Block[255];   /**< block buffer                           */
} bitstream;

static void WriteWordLE(FILE *File, unsigned short Value);
static void WriteImageData(FILE *File, tableentry *Table,
    unsigned char *Data, int FrameLeft, int FrameTop,
    int FrameWidth, int FrameHeight, int ImageWidth);
static void CropFrame(int *FrameLeft, int *FrameTop,
    int *FrameWidth, int *FrameHeight, unsigned char *Data,
    int ImageWidth, int ImageHeight, int TransparentColor);


/**
 * @brief Write an animated GIF image
 * @param Image array holding the image data for each frame
 * @param ImageWidth, ImageHeight dimensions of the image
 * @param NumFrames number of frames
 * @param Palette (global) color palette used by all the frames
 * @param NumColors number of colors in Palette
 * @param TransparentColor index of which color is transparent
 * @param Delays the delay for each frame in centiseconds
 * @param OutputFile filename of the output GIF file
 * @return 1 on success, 0 on failure
 *
 * This routine writes a sequence of image frames as an animated GIF file.
 * Image should be an array of pointers where Image[k] points to image
 * data in row-major order for the kth frame,
 *
 *     Image[k][x + ImageWidth*y] = pixel at (x,y) in the kth frame.
 *
 * The value of Image[k][x + ImageWidth*y] is the palette index of the
 * pixel's color.  Palette should be ordered such that
 *
 *     Palette[3*i + 0] = red intensity of the ith color,
 *     Palette[3*i + 1] = green intensity of the ith color,
 *     Palette[3*i + 2] = blue intensity of the ith color,
 *
 * where i = 0, 1, ... NumColors - 1.  TransparentColor specifies an index to
 * denote transparent pixels.
 *
 * This routine always uses the overwrite "frame disposal" method, which means
 * that the current frame is drawn on top of the previous ones.  This approach
 * allows animated frames to be transparent everywhere except for the pixels
 * that differ from the preceding frame.  Call the FrameDifference() function
 * before GifWrite() to perform this optimization.
 *
 * Delays is an array specifying how long each frame of the animation is shown
 * in units of centiseconds (1/100th of a second).  Specifying Delays as NULL
 * sets a delay of 0.1 seconds per frame.
 *
 * Only a limited portion of the GIF specification is implemented.
 * Particularly,
 * - transparency is always used (GIF does not require this)
 * - local color tables are not supported
 * - the frame disposal method is hardcoded to overwrite
 * - the background color index is hardcoded to 0
 *
 * See [1] for details on the GIF format.
 *
 * For LZW compression, we follow the hash table algorithm used in GIFCSRC [2]
 * and the UNIX "compress" program [3].  The hash table uses open addressing
 * double hashing (no chaining) on the prefix code / next character
 * combination and a variant of Knuth's algorithm D (vol. 3, sec. 6.4) with
 * G. Knott's relatively-prime secondary probe.
 *
 * References:
 * [1] http://www.w3.org/Graphics/GIF/spec-gif89a.txt
 * [2] http://www.programmersheaven.com/download/15257/download.aspx
 * [3] http://www.freebsd.org/cgi/cvsweb.cgi/src/usr.bin/compress/
 */
int GifWrite(unsigned char **Image,
    int ImageWidth, int ImageHeight, int NumFrames,
    const unsigned char *Palette, int NumColors, int TransparentColor,
    const int *Delays, const char *OutputFile)
{
    FILE *File = NULL;
    tableentry *Table = NULL;
    const int NumPixels = ImageWidth*ImageHeight;
    int i, Frame, TableSizePow;
    int FrameLeft, FrameTop, FrameWidth, FrameHeight, Success = 0;
    
    /* Input checking */
    if(!Image || !Palette || !OutputFile || ImageWidth <= 0
        || ImageHeight <= 0 || NumFrames <= 0 || NumColors <= 2
        || TransparentColor < 0 || TransparentColor >= NumColors)
        return 0;
    
    for(Frame = 0; Frame < NumFrames; Frame++)
    {
        if(!Image[Frame])
            return 0;
        
        for(i = 0; i < NumPixels; i++)
            if(Image[Frame][i] >= NumColors)
            {
                fprintf(stderr, "Pixel values exceed palette.\n");
                return 0;
            }
    }
    
    if(!(Table = (tableentry *)malloc(sizeof(tableentry)*TABLESIZE)))
        return 0;
    
    if(!(File = fopen(OutputFile, "wb")))
    {
        fprintf(stderr, "Unable to open \"%s\" for writing.\n", OutputFile);
        goto Catch;
    }
    
    for(TableSizePow = 1; TableSizePow < NumColors && TableSizePow < 8;)
        TableSizePow++;
    
    /* GIF Header */
    fwrite("GIF89a", 1, 6, File);
    WriteWordLE(File, (unsigned short)ImageWidth);
    WriteWordLE(File, (unsigned short)ImageHeight);
    putc(0xF0 | (TableSizePow - 1), File);
    WriteWordLE(File, 0x0000);
    fwrite(Palette, 1, 3*NumColors, File);

    /* Pad unused palette entries with 0 */
    for(i = 3*((1 << TableSizePow) - NumColors); i > 0; i--)
        putc(0x00, File);
    
    /* Netscape animation extension */
    if(NumFrames > 1)
        fwrite("\x21\xFF\x0BNETSCAPE2.0\x03\x01\xFF\xFF", 1, 19, File);
        
    for(Frame = 0; Frame < NumFrames; Frame++)
    {
        CropFrame(&FrameLeft, &FrameTop, &FrameWidth, &FrameHeight,
            Image[Frame], ImageWidth, ImageHeight, TransparentColor);
        
        /* Write Graphic control extension and frame descriptor */
        WriteWordLE(File, 0xF921);          /* Graphic control label     */
        WriteWordLE(File, 0x0504);          /* Size and packed fields    */
        WriteWordLE(File, (Delays) ? Delays[Frame] : 10);
        putc(TransparentColor, File);
        WriteWordLE(File, 0x2C00);          /* Begin frame descriptor    */
        WriteWordLE(File, (unsigned short)FrameLeft);
        WriteWordLE(File, (unsigned short)FrameTop);
        WriteWordLE(File, (unsigned short)FrameWidth);
        WriteWordLE(File, (unsigned short)FrameHeight);
        putc(0x00, File);                   /* No local color table      */
        
        /* Write the current frame */
        WriteImageData(File, Table, Image[Frame], FrameLeft, FrameTop,
            FrameWidth, FrameHeight, ImageWidth);
    }
    
    putc(0x3B, File);                       /* File terminator           */
    
    if(ferror(File))
    {
        fprintf(stderr, "Error while writing to \"%s\".\n", OutputFile);
        goto Catch;
    }
    
    Success = 1;
Catch:
    if(File)
        fclose(File);
    if(Table)
        free(Table);
    return Success;
}


/** @brief Write a 16-bit word in big Endian format */
static void WriteWordLE(FILE *File, unsigned short Value)
{
    putc(Value & 0xFF, File);
    putc((Value & 0xFF00) >> 8, File);
}


/** @brief Flush the block buffer to GIF file */
static void FlushBlock(bitstream *Stream)
{
    putc(Stream->BlockSize, Stream->File);  /* Write the size of the block */
    fwrite(Stream->Block, 1, Stream->BlockSize, Stream->File);
    Stream->BlockSize = 0;
}


/** @brief Flush the bit accumulator to the block buffer */
static void FlushBits(bitstream *Stream, int MaxRemaining)
{
    for(; Stream->NumBits > MaxRemaining;
        Stream->BitAccum >>= 8, Stream->NumBits -= 8)
    {
        Stream->Block[Stream->BlockSize] =
            (unsigned char)(Stream->BitAccum & 0xFF);
        
        if((++Stream->BlockSize) == 255)
            FlushBlock(Stream);
    }
}


/** @brief Write a code having Stream->BitsPerCode bits */
static void WriteBits(bitstream *Stream, unsigned short Code)
{
    Stream->BitAccum |= (Stream->NumBits == 0) ?
        Code : (Code << Stream->NumBits);
    Stream->NumBits += Stream->BitsPerCode;
    FlushBits(Stream, 7);
}


/** @brief Write compressed image data for one frame of a GIF animation */
static void WriteImageData(FILE *File, tableentry *Table,
    unsigned char *Data, int FrameLeft, int FrameTop,
    int FrameWidth, int FrameHeight, int ImageWidth)
{
    bitstream Stream;
    long Hash;
    unsigned short ClearCode, FreeCode, Prefix, NextRaise;
    unsigned int AppendChar, InitBitsPerCode = 9;
    int x, y, i, Step;
    
    Stream.File = File;
    Stream.BitsPerCode = InitBitsPerCode;
    Stream.BitAccum = Stream.NumBits = Stream.BlockSize = 0;
    ClearCode = (unsigned short)(1 << (InitBitsPerCode - 1));
    NextRaise = (unsigned short)(1 << InitBitsPerCode);
    FreeCode = ClearCode + 2;
    
    for(i = 0; i < TABLESIZE; i++)
        Table[i].Hash = UNUSED;
    
    putc(InitBitsPerCode - 1, File);
    
    /* Get the first character (top left corner of the frame) */
    Prefix = Data[FrameLeft + ImageWidth*FrameTop];
    
    /* Loop over the pixels of the frame in row-major order.  The following is
       equivalent to the nested loop
          for(y = 0; y < FrameHeight; y++)
             for(x = 0; x < FrameWidth; x++)
       but starting from x=1, y=0 instead of x=0, y=0.*/
    for(x = 1, y = 0; y < FrameHeight; (++x >= FrameWidth) ? (x = 0, y++) : 0)
    {
        /* Get the next character */
        AppendChar = Data[(x + FrameLeft) + ImageWidth*(y + FrameTop)];
        
        /* Search for Prefix+AppendChar in the Table */
        Hash = (long)Prefix + (AppendChar << MAXBITS);
        i = (AppendChar << HASHSHIFT) ^ Prefix;
        Step = (i == 0) ? 1 : (TABLESIZE - i);
        
        while(Table[i].Hash != Hash && Table[i].Hash != UNUSED)
            if((i -= Step) < 0)
                i += TABLESIZE;
        
        if(Table[i].Hash != UNUSED)
        {
            /* Set Prefix <- Prefix+AppendChar */
            Prefix = Table[i].Code;
            continue;
        }
        
        /* If we get here, Prefix+AppendChar is not in the Table. */
        WriteBits(&Stream, Prefix);
        
        if(FreeCode < MAXCODE)
        {
            /* Increase BitsPerCode if necessary */
            if(FreeCode == NextRaise)
            {
                Stream.BitsPerCode++;
                NextRaise *= 2;
            }

            /* Add Prefix+AppendChar to the Table. */
            Table[i].Hash = Hash;
            Table[i].Code = FreeCode++;
        }
        else
        {   /* There are no free codes left, clear the Table. */
            WriteBits(&Stream, ClearCode);
            Stream.BitsPerCode = InitBitsPerCode;
            NextRaise = (unsigned short)(1 << InitBitsPerCode);
            FreeCode = ClearCode + 2;
            
            for(i = 0; i < TABLESIZE; i++)
                Table[i].Hash = UNUSED;
        }
        
        Prefix = AppendChar;
    }

    /* Flush buffers and write ending codes */
    WriteBits(&Stream, Prefix);
    WriteBits(&Stream, ClearCode + 1);
    FlushBits(&Stream, 0);
    
    if(Stream.BlockSize > 0)
        FlushBlock(&Stream);
    
    putc(0, File);
}


/** @brief Crop the extent of a frame according to its transparency */
static void CropFrame(int *FrameLeft, int *FrameTop,
    int *FrameWidth, int *FrameHeight, unsigned char *Data,
    int ImageWidth, int ImageHeight, int TransparentColor)
{
    int x, y, Left = ImageWidth, Right = 0, Top = ImageHeight, Bottom = 0;
    
    for(y = 0; y < ImageHeight; y++, Data += ImageWidth)
        for(x = 0; x < ImageWidth; x++)
            if(Data[x] != TransparentColor)
            {
                if(x < Left)
                    Left = x;
                if(x > Right)
                    Right = x;
                if(y < Top)
                    Top = y;
                if(y > Bottom)
                    Bottom = y;
            }
    
    if(Left == ImageWidth)
    {
        *FrameLeft = *FrameTop = 0;
        *FrameWidth = *FrameHeight = 1;
    }
    else
    {
        *FrameLeft = Left;
        *FrameTop = Top;
        *FrameWidth = Right - Left + 1;
        *FrameHeight = Bottom - Top + 1;
    }
}


/**
 * @brief Optimize animation frames by setting unchanged pixels to transparent
 * @param Image array holding the image data for each frame
 * @param ImageWidth, ImageHeight dimensions of the image
 * @param NumFrames number of frames
 * @param Palette (global) color palette used by all the frames
 * @param NumColors number of colors in Palette
 * @param TransparentColor index of which color is transparent
 * @param Delays the delay for each frame in centiseconds
 * @param OutputFile filename of the output GIF file
 */
void FrameDifference(unsigned char **Image,
    int ImageWidth, int ImageHeight, int NumFrames, int TransparentColor)
{
    const int NumPixels = ImageWidth*ImageHeight;
    int i, Frame, PrevFrame;
    
    /* Input checking */
    if(!Image || ImageWidth <= 0 || ImageHeight <= 0 || NumFrames <= 0)
        return;
    
    for(Frame = 0; Frame < NumFrames; Frame++)
        if(!Image[Frame])
            return;
    
    for(Frame = NumFrames - 1; Frame > 0; Frame--)
        for(i = 0; i < NumPixels; i++)
        {
            if(Image[Frame][i] == TransparentColor)
                continue;
            
            /* Find most recent frame where ith pixel is nontransparent */
            for(PrevFrame = Frame - 1; PrevFrame >= 0; PrevFrame--)
                if(Image[PrevFrame][i] != TransparentColor)
                    break;

            /* Set pixel to transparent if previous pixel is the same color */
            if(PrevFrame >= 0 && Image[PrevFrame][i] == Image[Frame][i])
                Image[Frame][i] = TransparentColor;
        }
}
