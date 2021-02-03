#ifndef PALETTE_H
#define PALETTE_H

// Various Enterprise palettes&biases approximating C64 colors
// and mappings of C64 color numbers to byte values for
// Enterprise 16-colour mode video memory

#if defined PALETTE1
      static const unsigned char palette[] = { 0, 1, 2, 4, 11, 88, 56, 63 };
      static const unsigned char bias=248; // Bright upper colours
      // c64 colors[sic!] to e128 colours[sic!]:
      //  0 (black)       -> 0    1 (white)       -> 15
      //  2 (red)         -> 1    3 (cyan)        -> 14
      //  4 (purple)      -> 13   5 (green)       -> 2
      //  6 (blue)        -> 3    7 (yellow)      -> 11
      //  8 (orange)      -> 4    9 (brown)       -> 5
      // 10 (light red)   -> 9   11 (dark grey)   -> 6
      // 12 (grey)        -> 8   13 (light green) -> 10
      // 14 (light blue)  -> 12  15 (light grey)  -> 7

      // Colour ( b3 b2 b1 b0 ) becomes
      //   bit pattern ( b0  0 b2  0  b1  0 b3  0 ) for left pixel
      //   bit pattern (  0 b0  0 b2  0  b1  0 b3 ) for right pixel
      static const unsigned char pixelvalue[] = {
            0b00000000 /*<- 0000 =  0*/, 0b10101010 /*<- 1111 = 15*/,
            0b10000000 /*<- 0001 =  1*/, 0b00101010 /*<- 1110 = 14*/,
            0b10100010 /*<- 1101 = 13*/, 0b00001000 /*<- 0010 =  2*/,
            0b10001000 /*<- 0011 =  3*/, 0b10001010 /*<- 1011 = 11*/,
            0b00100000 /*<- 0100 =  4*/, 0b10100000 /*<- 0101 =  5*/,
            0b10000010 /*<- 1001 =  9*/, 0b00101000 /*<- 0110 =  6*/,
            0b00000010 /*<- 1000 =  8*/, 0b00001010 /*<- 1010 = 10*/,
            0b00100010 /*<- 1100 = 12*/, 0b10101000 /*<- 0111 =  7*/
      };
#elif defined PALETTE2
      static const unsigned char palette[] = { 0, 255, 1, 2, 4, 11, 88, 7 };
      static const unsigned char bias=56;  // Medium-brightness upper colours
      // c64 colors[sic!] to e128 colours[sic!]:
      //  0 (black)       -> 0    1 (white)       -> 1
      //  2 (red)         -> 2    3 (cyan)        -> 14
      //  4 (purple)      -> 13   5 (green)       -> 3
      //  6 (blue)        -> 4    7 (yellow)      -> 11
      //  8 (orange)      -> 5    9 (brown)       -> 6
      // 10 (light red)   -> 9   11 (dark grey)   -> 8
      // 12 (grey)        -> 7   13 (light green) -> 10
      // 14 (light blue)  -> 12  15 (light grey)  -> 15

      // Colour ( b3 b2 b1 b0 ) becomes
      //   bit pattern ( b0  0 b2  0  b1  0 b3  0 ) for left pixel
      //   bit pattern (  0 b0  0 b2  0  b1  0 b3 ) for right pixel
      static const unsigned char pixelvalue[] = {
            0b00000000 /*<- 0000 =  0*/, 0b10000000 /*<- 0001 =  1*/,
            0b00001000 /*<- 0010 =  2*/, 0b00101010 /*<- 1110 = 14*/,
            0b10100010 /*<- 1101 = 13*/, 0b10001000 /*<- 0011 =  3*/,
            0b00100000 /*<- 0100 =  4*/, 0b10001010 /*<- 1011 = 11*/,
            0b10100000 /*<- 0101 =  5*/, 0b00101000 /*<- 0110 =  6*/,
            0b10000010 /*<- 1001 =  9*/, 0b00000010 /*<- 1000 =  8*/, 
            0b10101000 /*<- 0111 =  7*/, 0b00001010 /*<- 1010 = 10*/,
            0b00100010 /*<- 1100 = 12*/, 0b10101010 /*<- 1111 = 15*/
      };
#elif defined PALETTE3
      static const unsigned char palette[] = { 56, 255, 79, 58, 60, 11, 88, 63 };
      static const unsigned char bias=0;  // Dark upper colours
      // c64 colors[sic!] to e128 colours[sic!]:
      //  0 (black)       -> 8    1 (white)       -> 1
      //  2 (red)         -> 9    3 (cyan)        -> 14
      //  4 (purple)      -> 13   5 (green)       -> 10
      //  6 (blue)        -> 12   7 (yellow)      -> 11
      //  8 (orange)      -> 5    9 (brown)       -> 6
      // 10 (light red)   -> 2   11 (dark grey)   -> 0
      // 12 (grey)        -> 15  13 (light green) -> 3
      // 14 (light blue)  -> 4   15 (light grey)  -> 7

      // Colour ( b3 b2 b1 b0 ) becomes
      //   bit pattern ( b0  0 b2  0  b1  0 b3  0 ) for left pixel
      //   bit pattern (  0 b0  0 b2  0  b1  0 b3 ) for right pixel
      static const unsigned char pixelvalue[] = {
          0b00000010 /*<- 1000 =  8*/, 0b10000000 /*<- 0001 =  1*/,
          0b10000010 /*<- 1001 =  9*/, 0b00101010 /*<- 1110 = 14*/,
          0b10100010 /*<- 1101 = 13*/, 0b00001010 /*<- 1010 = 10*/,
          0b00100010 /*<- 1100 = 12*/, 0b10001010 /*<- 1011 = 11*/,
          0b10100000 /*<- 0101 =  5*/, 0b00101000 /*<- 0110 =  6*/,
          0b00001000 /*<- 0010 =  2*/, 0b00000000 /*<- 0000 =  0*/,
          0b10101010 /*<- 1111 = 15*/, 0b10001000 /*<- 0011 =  3*/,
          0b00100000 /*<- 0100 =  4*/, 0b10101000 /*<- 0111 =  7*/
      };
#else
    #error You must choose one from the palette variations! (#define PALETTEn)
#endif



#endif