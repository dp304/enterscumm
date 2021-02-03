
/* E n t e r S C U M M
 * Enterprise 128 viewer for Maniac Mansion background images
 * To build:
 * zcc +enterprise -compiler sdcc enterscumm.c -o entscumm -create-app 
 */

#include <stdio.h>
#include <string.h>
#include <enterprise.h>
#include <games.h>

#define PALETTE2
#include "palette.h"
#include "c64graphics.h"

#define C_UPARROW    0x8b
#define C_RIGHTARROW 0x8c
#define C_DOWNARROW  0x9b
#define C_LEFTARROW  0x9c
//#define C_LEFTARROW  0x9e

static const unsigned char text_row1[] = {2, C_UPARROW, C_UPARROW};
static const unsigned char text_row2[] = {4, 'N', 'E', 'X', 'T'};
static const unsigned char text_row3[] = {22,
      C_LEFTARROW, ' ', 'S', 'C', 'R', 'O', 'L', 'L',
      ' ', ' ', ' ', ' ', ' ', ' ',
      'S', 'C', 'R', 'O', 'L', 'L', ' ', C_RIGHTARROW};
static const unsigned char text_row4[] = {4, 'P', 'R', 'E', 'V'};
static const unsigned char text_row5a[] = {2, C_DOWNARROW, C_DOWNARROW};
static const unsigned char text_row5b[] = {13,
      'E', 'N', 'T', 'E', 'R', ' ', '-', ' ', 'i', 'n', 'p', 'u', 't'};

#define PRINT_AT(row, column, text) {\
      set_cursor_position((row), (column));\
      exos_write_block(102, (text)[0], (text)+1);\
}
#define CLEAR_TEXT exos_write_character(102, 0x1a)

unsigned char buf[64];

unsigned char keypress;

unsigned int video_addr;
unsigned int segment, offset;
unsigned char *video_mem;

unsigned char room_file_name[] = {6, '0','0','.','L','F','L'};
unsigned char room_no = 1;
unsigned char room_w, room_h, room_x;
unsigned char room_colors[4];

unsigned char char_map[2048];
unsigned char pic_map[3400];
unsigned char col_map[3400];

__sfr __at 0xb0 page0;
__sfr __at 0xb1 page1;
__sfr __at 0xb2 page2;
__sfr __at 0xb3 page3;

#define VIDEO_X 40
#define VIDEO_Y 16
#define VIDEO_MEM_SIZE (2*(VIDEO_X)*(VIDEO_Y)*9)
#define MM_SCREEN_SIZE (2*40*136)

#define CLEAR_WITH(i) memset(video_mem, (pixelvalue[(i)])|((pixelvalue[i])>>1), VIDEO_MEM_SIZE);

void set_ink(int channel, int ink) {
      esccmd_cmd='I';
      esccmd_x=ink;
      exos_write_block(channel, 3, esccmd);
}
void set_beam_on(int channel) {
      esccmd_cmd='S';
      exos_write_block(channel, 2, esccmd);
}
void set_beam_off(int channel) {
      esccmd_cmd='s';
      exos_write_block(channel, 2, esccmd);
}
void plot(int channel, int x, int y) {
      esccmd_cmd='A';
      esccmd_x = x;
      esccmd_y = y;
      exos_write_block(channel, 6, esccmd);
}
void plot_ellipse(int channel, int x, int y) {
      esccmd_cmd='E';
      esccmd_x = x;
      esccmd_y = y;
      exos_write_block(channel, 6, esccmd);
}
void plot_paint(int channel) {
      esccmd_cmd='F';
      exos_write_block(channel, 2, esccmd);
}

void set_cursor_position(unsigned char row, unsigned char column) {
      esccmd_cmd = '=';
      esccmd_p1 = row + 0x20;
      esccmd_p2 = column + 0x20;
      exos_write_block(102, 4, esccmd);
}

void set_cursor_on() {
      esccmd_cmd = 'O';
      exos_write_block(102, 2, esccmd);
}

void set_cursor_off() {
      esccmd_cmd = 'o';
      exos_write_block(102, 2, esccmd);
}

int exos_get_primary_video_address(int channel) __z88dk_fastcall __naked {
      channel;  // to avoid warning
      __asm
            ld a,l     ; channel
            ld b,3     ; special function code ADDR
            rst 30h    ; EXOS
            defb 11
            ld hl,bc   ; primary video address result
            ret
      __endasm;
}

int exos_seek(register int channel, unsigned long pos) __smallc __z88dk_callee __naked {
      channel; pos;  // to avoid warning
      __asm
            pop de               ; return address
            pop hl               ; low word of pos
            ld (_buf),hl
            pop hl               ; high word of pos
            ld (_buf+2),hl
            pop hl
            ld a,l               ; A: channel
            push de              ; put back return address
            ld de,_buf           ; DE: start of parameter block
            ld c,1               ; C: write flags (set new pointer value)
            rst 30h              ; EXOS
            defb 10              ; set/read channel status
            ld h,0
            ld l,a
            ret
      __endasm;
}

void exos_explain_error(int code) __z88dk_fastcall __naked {
      code;
      __asm
            ld a,l
            ld de,_buf
            rst 30h
            defb 28
            ret
      __endasm;
}

void show_error(unsigned char errcode) {
      set_cursor_position(7,1);
      buf[0]=buf[1]=buf[2]='*';
      buf[3]=' ';
      exos_write_block(102, 4, buf);
      exos_explain_error(errcode);
      exos_write_block(102, buf[0], buf+1);
      exos_write_character(102, '.');

      getk();
      while (getk() == 0) {}

      set_cursor_position(7,1);
      exos_write_character(102, 0x19); // shift+del
      set_cursor_position(8,1);
      exos_write_character(102, 0x19); // shift+del
}

unsigned char input_room_no() {
      static unsigned char digit;
      static unsigned char new_room_no;
      set_cursor_position(3, 20);
      set_cursor_on();
      do { digit = getk(); } while (digit<'0' || digit>'9');
      exos_write_character(102, digit);
      new_room_no = 10*(digit-'0');
      do { digit = getk(); } while (digit<'0' || digit>'9');
      exos_write_character(102, digit);
      new_room_no += digit-'0';
      set_cursor_off();

      if (new_room_no<1 || new_room_no>52 || new_room_no==room_no) {
            set_cursor_position(3, 20);
            printk("%02d", room_no);
            return 0;
      } else {
            room_no = new_room_no;
            return 1;
      }
}

void* get_sp() __z88dk_fastcall __naked {
      __asm
            ld hl,2
            add hl,sp
            ret
      __endasm;
}

// Decode RLE
void derle(unsigned char *dst, unsigned int len) {
      static unsigned char lookup[4];

      exos_read_block(106, 4, lookup);

      do {
            unsigned char flag = exos_read_character(106);
            unsigned char run_len, val;
            if ((flag&192) == 0) {
                  run_len = flag + 1;
                  if (run_len > len) run_len = len;
                  exos_read_block(106, run_len, dst);
            } else {
                  if (flag&128) {
                        run_len = (flag&31) + 1;
                        val = lookup[(flag>>5)&3];
                  } else {
                        run_len = (flag&63) + 1;
                        val = exos_read_character(106);
                  }
                  if (run_len > len) run_len = len;
                  memset(dst, val, run_len);
            }
            dst += run_len;
            len -= run_len;
      } while (len != 0);
}

void open_room() {
      static char errcode;
      static unsigned int char_map_offset;
      static unsigned int pic_map_offset;
      static unsigned int col_map_offset;

      room_x = 0;
      room_file_name[1] = '0' + room_no/10;
      room_file_name[2] = '0' + room_no%10;
      errcode = exos_open_channel(106, room_file_name);
      if (errcode!=0) goto open_room_handle_error;
      errcode = exos_seek(106, 0x04);
      if (errcode!=0) goto open_room_handle_error;
      room_w = exos_read_character(106);
      room_h = exos_read_character(106);
      exos_read_block(106, 4, room_colors);
      for (unsigned char i=0; i!=4; ++i) {
            room_colors[i] = pixelvalue[room_colors[i]&15];
      }
      exos_read_block(106, 2, &char_map_offset);
      exos_read_block(106, 2, &pic_map_offset);
      exos_read_block(106, 2, &col_map_offset);
      //printf("char %p  pic %p  col %p\n", char_map_offset, pic_map_offset, col_map_offset);
      exos_seek(106, char_map_offset);
      derle(char_map, 2048);
      exos_seek(106, pic_map_offset);
      derle(pic_map, room_w*room_h);
      exos_seek(106, col_map_offset);
      derle(col_map, room_w*room_h);
      for (unsigned int i=0; i!=room_w*room_h; ++i) {
            col_map[i] = pixelvalue[col_map[i]&7];
      }

      goto open_room_cleanup;

open_room_handle_error:
      show_error(errcode);
      room_w = room_h = 0;
open_room_cleanup:
      exos_close_channel(106);
}

void draw_room() {
      static unsigned char *output;
      const unsigned char xend = (room_w-room_x>VIDEO_X ? VIDEO_X : room_w-room_x);
      static unsigned int i;

      if (room_w==0) return;

      output = video_mem;
      
      i = (int)room_x * (int)room_h;

      for (unsigned char x=0; x!=xend; x++) {
            for (unsigned char y=0; y!=room_h; y++) {
                  room_colors[3] = col_map[i];
                  register unsigned char * r = char_map + ((unsigned int)(pic_map[i])<<3);

                  PUT_CHAR_AND_MOVE(output, r);

                  /*register unsigned char * const r_end = r + 8;
                  while (r!=r_end) {
                        PUT_CHAR_ROW_AND_MOVE(output,r)
                        ++r;
                  }*/
                  ++i;
            }
            output -= (MM_SCREEN_SIZE-2);
      }
}

void scroll_left() {
      static unsigned char *output;
      static unsigned int i;

      if (room_w==0 || room_x==0) return;
      // shift old screen
      memmove(video_mem+2, video_mem, MM_SCREEN_SIZE-2);
      --room_x;
      // draw new strip
      output = video_mem;
      i = room_x * room_h;
      for (unsigned char y=0; y<room_h; y++) {
            room_colors[3] = col_map[i];
            register unsigned char * r = char_map + ((unsigned int)(pic_map[i])<<3);

            PUT_CHAR_AND_MOVE(output, r);

            /*register unsigned char * const r_end = r + 8;
            while (r!=r_end) {
                  PUT_CHAR_ROW_AND_MOVE(output,r)
                  ++r;
            }*/
            ++i;
      }
}

void scroll_right() {
      static unsigned char *output;
      static unsigned int i;

      if (room_w==0 || room_w-room_x<=VIDEO_X) return;
      // shift old screen
      memmove(video_mem, video_mem+2, MM_SCREEN_SIZE-2);
      ++room_x;
      // draw new strip
      output = video_mem+(2*(VIDEO_X-1));
      i = (room_x + (VIDEO_X-1)) * room_h;
      for (unsigned char y=0; y<room_h; y++) {
            room_colors[3] = col_map[i];
            register unsigned char * r = char_map + ((unsigned int)(pic_map[i])<<3);

            PUT_CHAR_AND_MOVE(output, r);

            /*register unsigned char * const r_end = r + 8;
            while (r!=r_end) {
                  PUT_CHAR_ROW_AND_MOVE(output,r)
                  ++r;
            }*/

            ++i;
      }
}

void main()
{
      exos_close_channel(103);
      exos_open_channel(103, DEV_SOUND);

      exos_close_channel(102);
      set_exos_variable(EV_MODE_VID, VM_HW_TXT);
      set_exos_variable(EV_COLR_VID, CM_2);
      set_exos_variable(EV_X_SIZ_VID, 40);
      set_exos_variable(EV_Y_SIZ_VID, 8);
      exos_open_channel(102, DEV_VIDEO);
      exos_reset_font(102);
      set_cursor_off();
      exos_display_page(102, 1, 8, 18);

      set_exos_variable(EV_BORD_VID, 20);

      exos_close_channel(101);
      set_exos_variable(EV_MODE_VID, VM_HRG);
      set_exos_variable(EV_COLR_VID, CM_16);
      set_exos_variable(EV_X_SIZ_VID, VIDEO_X);
      set_exos_variable(EV_Y_SIZ_VID, VIDEO_Y);
      exos_open_channel(101, DEV_VIDEO);


      esccmd_cmd = 'C'; /* SET PALETTE */
      exos_write_block(101, 2, esccmd);
      exos_write_block(101, 8, palette);
      set_exos_variable(28, bias); /* SET BIAS */
 
      video_addr = exos_get_primary_video_address(101);
      segment = 0xfc + (video_addr >> 14);
      offset = video_addr & 0x3fff;

      page2 = segment;
      page3 = segment + 1;
      video_mem = (void *)(0x8000 + offset);

      CLEAR_WITH(0);
      exos_display_page(101, 1, VIDEO_Y, 1);

      CLEAR_TEXT;
      PRINT_AT(1, 20, text_row1);
      PRINT_AT(2, 19, text_row2);
      PRINT_AT(3, 10, text_row3);
      PRINT_AT(4, 19, text_row4);
      PRINT_AT(5, 20, text_row5a);
      PRINT_AT(5, 26, text_row5b);
      
      exos_write_character(103, 0x07);  // PING

      keyboard_click_off();

      room_no = 1;

      static unsigned char room_load_required;
      room_load_required = 1;
      do
      {
            if (room_load_required) {
                  CLEAR_WITH(0);
                  set_cursor_position(3,20);
                  printk("%02d", room_no);
                  set_cursor_position(1,2);
                  printk("         ");
                  open_room();
                  if (room_w>0) {
                        set_cursor_position(1,2);
                        printk("%3d x %d", room_w, room_h);
                        draw_room();
                  }

                  room_load_required = 0;
            }

            switch ((char)joystick(0))
            {
            case 1:
                  scroll_right();
                  break;
            case 2:
                  scroll_left();
                  break;
            case 4:
                  if (room_no > 1) {
                        --room_no;
                        room_load_required = 1;
                  }
                  break;
            case 8:
                  if (room_no < 52) {
                        ++room_no;
                        room_load_required = 1;
                  }
                  break;
            default:
                  break;
            }

            keypress = getk();
            if (keypress == 10) {
                  room_load_required = input_room_no();
            }
      } while (keypress != 27);

      exos_close_channel(103);
      set_exos_variable(EV_BORD_VID, 0);
      keyboard_click_on();
}
