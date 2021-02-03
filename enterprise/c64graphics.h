#ifndef C64GRAPHICS_H
#define C64GRAPHICS_H


// Rendering a line of a character
// and moving to the next line with the output pointer
#define PUT_CHAR_ROW_AND_MOVE(output, r) \
    *(output) = (room_colors[ *(r)>>6   ]) | (room_colors[(*(r)>>4)&3]>>1);  ++(output);\
    *(output) = (room_colors[(*(r)>>2)&3]) | (room_colors[ *(r)    &3]>>1);  (output) += 2*VIDEO_X-1;

// Unrolled loop for rendering all 8 lines of a character
// and moving to the next line with the output pointer
#define PUT_CHAR_AND_MOVE(output, r) \
    PUT_CHAR_ROW_AND_MOVE(output, r)\
    ++(r);\
    PUT_CHAR_ROW_AND_MOVE(output, r)\
    ++(r);\
    PUT_CHAR_ROW_AND_MOVE(output, r)\
    ++(r);\
    PUT_CHAR_ROW_AND_MOVE(output, r)\
    ++(r);\
    PUT_CHAR_ROW_AND_MOVE(output, r)\
    ++(r);\
    PUT_CHAR_ROW_AND_MOVE(output, r)\
    ++(r);\
    PUT_CHAR_ROW_AND_MOVE(output, r)\
    ++(r);\
    PUT_CHAR_ROW_AND_MOVE(output, r)

#endif