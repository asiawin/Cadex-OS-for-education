#include "kernel.h"
#include "lib.h"
#include "keychar.h"

//***********************************************//
//   Cadex Kernel Version 2.1 Build 1023B        //
//***********************************************//

uint32 vga_index;
uint16 cursor_pos = 0, cursor_next_line_index = 1;
static uint32 next_line_index = 1;
uint8 g_fore_color = WHITE, g_back_color = BLACK;
////////////////////// NOTES FOR DEVELOPERS ////////////////////////////
// If you are running this OS in VirtualBox, VMWare or on a real machine,
// change the VGA_SLEEP given following to any number greater than 10
// Or, if your running this OS in QEMU, set it to 1
// If you are using an IDE like VS code or Visual Studio, ignore all the
// syntax error it shows.

#define VGA_SLEEP 10

uint16
vga_entry(unsigned char ch, uint8 fore_color, uint8 back_color)
{
  uint16 ax = 0;
  uint8 ah = 0, al = 0;

  ah = back_color;
  ah <<= 4;
  ah |= fore_color;
  ax = ah;
  ax <<= 8;
  al = ch;
  ax |= al;

  return ax;
}

void clear_vga_buffer(uint16 **buffer, uint8 fore_color, uint8 back_color)
{
  uint32 i;
  for (i = 0; i < BUFSIZE; i++)
  {
    (*buffer)[i] = vga_entry(NULL, fore_color, back_color);
  }
  next_line_index = 1;
  vga_index = 0;
}

void clear_screen()
{
  clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
  cursor_pos = 0;
  cursor_next_line_index = 1;
}

void init_vga(uint8 fore_color, uint8 back_color)
{
  vga_buffer = (uint16 *)VGA_ADDRESS;
  clear_vga_buffer(&vga_buffer, fore_color, back_color);
  g_fore_color = fore_color;
  g_back_color = back_color;
}

uint8 inb(uint16 port)
{
  uint8 data;
  asm volatile("inb %1, %0"
               : "=a"(data)
               : "Nd"(port));
  return data;
}

void outb(uint16 port, uint8 data)
{
  asm volatile("outb %0, %1"
               :
               : "a"(data), "Nd"(port));
}

void move_cursor(uint16 pos)
{
  outb(0x3D4, 14);
  outb(0x3D5, ((pos >> 8) & 0x00FF));
  outb(0x3D4, 15);
  outb(0x3D5, pos & 0x00FF);
}

void move_cursor_next_line()
{
  cursor_pos = 80 * cursor_next_line_index;
  cursor_next_line_index++;
  move_cursor(cursor_pos);
}

void gotoxy(uint16 x, uint16 y)
{
  vga_index = 80 * y;
  vga_index += x;
  if (y > 0)
  {
    cursor_pos = 80 * cursor_next_line_index * y;
    cursor_next_line_index++;
    move_cursor(cursor_pos);
  }
}

char get_input_keycode()
{
  char ch = 0;
  while ((ch = inb(KEYBOARD_PORT)) != 0)
  {
    if (ch > 0)
      return ch;
  }
  return ch;
}

/*
keep the cpu busy for doing nothing(nop)
so that io port will not be processed by cpu
here timer can also be used, but lets do this in looping counter
*/
void wait_for_io(uint32 timer_count)
{
  while (1)
  {
    asm volatile("nop");
    timer_count--;
    if (timer_count <= 0)
      break;
  }
}
void sleep(uint32 timer_count)
{
  wait_for_io(timer_count * 0x02FFFFFF);
}
void shutdown()
{
  clear_screen();
  sleep(160);
}

void print_new_line()
{
  if (next_line_index >= 55)
  {
    next_line_index = 0;
    clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
  }
  vga_index = 80 * next_line_index;
  next_line_index++;
  move_cursor_next_line();
}

void print_char(char ch)
{
  vga_buffer[vga_index] = vga_entry(ch, g_fore_color, g_back_color);
  vga_index++;
  move_cursor(++cursor_pos);
}

void print_string(char *str)
{
  uint32 index = 0;
  while (str[index])
  {
    if (str[index] == '\n')
    {
      print_new_line();
      index++;
    }
    else
    {
      print_char(str[index]);
      index++;
    }
  }
}

void print_int(int num)
{
  char str_num[digit_count(num) + 1];
  itoa(num, str_num);
  print_string(str_num);
}
void print_color_string(char *str, uint8 fore_color, uint8 back_color)
{
  uint32 index = 0;
  uint8 fc, bc;
  fc = g_fore_color;
  bc = g_back_color;
  g_fore_color = fore_color;
  g_back_color = back_color;
  while (str[index])
  {
    if (str[index] == '\n')
    {
      print_new_line();
      index++;
    }
    else
    {
      print_char(str[index]);
      index++;
    }
  }
  g_fore_color = fc;
  g_back_color = bc;
}

int read_int()
{
  char ch = 0;
  char keycode = 0;
  char data[32];
  int index = 0;
  do
  {
    keycode = get_input_keycode();
    if (keycode == KEY_ENTER)
    {
      data[index] = '\0';
      print_new_line();
      break;
    }
    else if (keycode == KEY_BACKSPACE)
    {
      break;
    }
    else if (keycode == KEY_KEYPAD_PLUS)
    {
      break;
    }
    else
    {
      ch = get_ascii_char(keycode);
      print_char(ch);
      data[index] = ch;
      index++;
    }
    sleep(VGA_SLEEP);
  } while (ch > 0);

  return atoi(data);
}

uint16 get_box_draw_char(uint8 chn, uint8 fore_color, uint8 back_color)
{
  uint16 ax = 0;
  uint8 ah = 0;

  ah = back_color;
  ah <<= 4;
  ah |= fore_color;
  ax = ah;
  ax <<= 8;
  ax |= chn;

  return ax;
}
void draw_generic_box(uint16 x, uint16 y,
                      uint16 width, uint16 height,
                      uint8 fore_color, uint8 back_color,
                      uint8 topleft_ch,
                      uint8 topbottom_ch,
                      uint8 topright_ch,
                      uint8 leftrightside_ch,
                      uint8 bottomleft_ch,
                      uint8 bottomright_ch)
{
  uint32 i;

  //increase vga_index to x & y location
  vga_index = 80 * y;
  vga_index += x;

  //draw top-left box character
  vga_buffer[vga_index] = get_box_draw_char(topleft_ch, fore_color, back_color);

  vga_index++;
  //draw box top characters, -
  for (i = 0; i < width; i++)
  {
    vga_buffer[vga_index] = get_box_draw_char(topbottom_ch, fore_color, back_color);
    vga_index++;
  }

  //draw top-right box character
  vga_buffer[vga_index] = get_box_draw_char(topright_ch, fore_color, back_color);

  // increase y, for drawing next line
  y++;
  // goto next line
  vga_index = 80 * y;
  vga_index += x;

  //draw left and right sides of box
  for (i = 0; i < height; i++)
  {
    //draw left side character
    vga_buffer[vga_index] = get_box_draw_char(leftrightside_ch, fore_color, back_color);
    vga_index++;
    //increase vga_index to the width of box
    vga_index += width;
    //draw right side character
    vga_buffer[vga_index] = get_box_draw_char(leftrightside_ch, fore_color, back_color);
    //goto next line
    y++;
    vga_index = 80 * y;
    vga_index += x;
  }
  //draw bottom-left box character
  vga_buffer[vga_index] = get_box_draw_char(bottomleft_ch, fore_color, back_color);
  vga_index++;
  //draw box bottom characters, -
  for (i = 0; i < width; i++)
  {
    vga_buffer[vga_index] = get_box_draw_char(topbottom_ch, fore_color, back_color);
    vga_index++;
  }
  //draw bottom-right box character
  vga_buffer[vga_index] = get_box_draw_char(bottomright_ch, fore_color, back_color);

  vga_index = 0;
}
void draw_box(uint8 boxtype,
              uint16 x, uint16 y,
              uint16 width, uint16 height,
              uint8 fore_color, uint8 back_color)
{
  switch (boxtype)
  {
  case BOX_SINGLELINE:
    draw_generic_box(x, y, width, height,
                     fore_color, back_color,
                     218, 196, 191, 179, 192, 217);
    break;

  case BOX_DOUBLELINE:
    draw_generic_box(x, y, width, height,
                     fore_color, back_color,
                     201, 205, 187, 186, 200, 188);
    break;
  }
}
void fill_box(uint8 ch, uint16 x, uint16 y, uint16 width, uint16 height, uint8 color)
{
  uint32 i, j;

  for (i = 0; i < height; i++)
  {
    //increase vga_index to x & y location
    vga_index = 80 * y;
    vga_index += x;

    for (j = 0; j < width; j++)
    {
      vga_buffer[vga_index] = get_box_draw_char(ch, 0, color);
      vga_index++;
    }
    y++;
  }
}

char getchar()
{
  char keycode = 0;
  sleep(VGA_SLEEP);
  keycode = get_input_keycode();
  sleep(VGA_SLEEP);
  return get_ascii_char(keycode);
}

void display_menu()
{
  // Initialise the VGA with White foreground and Blue background
  init_vga(WHITE, BLUE);
  gotoxy(29, 0);
  // Print OS name
  print_string(OS_NAME);
  gotoxy(27, 1);
  // Print OS version
  print_string(OS_VERSION);
  print_string("\n\nType 11 for help\n");
}

void makeErrorDialog(char *strdisp)
{
  gotoxy(5, 10);
  print_color_string(strdisp, BRIGHT_RED, WHITE);
  fill_box(KEY_0, 1, 1, BOX_MAX_WIDTH, BOX_MAX_HEIGHT, WHITE);
  draw_box(BOX_DOUBLELINE, 0, 0, BOX_MAX_WIDTH, BOX_MAX_HEIGHT, BLUE, GREEN);
  for (uint32 i = 0; i < 30; i++)
  {
    /* This is because we don't want to show the cursor in the dialog */
    move_cursor_next_line();
  }
}
void print_binary(uint32 num)
{
  // This is to print the binary numbers
  char bin_arr[32];
  uint32 index = 31;
  uint32 i;
  while (num > 0)
  {
    if (num & 1)
    {
      bin_arr[index] = '1';
    }
    else
    {
      bin_arr[index] = '0';
    }
    index--;
    num >>= 1;
  }

  for (i = 0; i < 32; ++i)
  {
    if (i <= index)
      print_char('0');
    else
      print_char(bin_arr[i]);
  }
}

void cpuid(uint32 value, uint32 *eax, uint32 *ebx, unsigned int *ecx, uint32 *edx)
{
  // This is to print the cpu id
  uint32 eaxres, ebxres, ecxres, edxres;
  asm("xorl\t%eax, %eax");
  asm("xorl\t%ebx, %ebx");
  asm("xorl\t%ecx, %ecx");
  asm("xorl\t%edx, %edx");
  asm("movl\t%0, %%eax"
      : "=m"(value));
  asm("cpuid");
  asm("movl\t%%eax, %0"
      : "=m"(eaxres));
  asm("movl\t%%ebx, %0"
      : "=m"(ebxres));
  asm("movl\t%%ecx, %0"
      : "=m"(ecxres));
  asm("movl\t%%edx, %0"
      : "=m"(edxres));
  *eax = eaxres;
  *ebx = ebxres;
  *ecx = ecxres;
  *edx = edxres;
}

void print_eax(uint32 eax)
{
  // This is to print the data in the eax register
  uint32 step_id, model, family_id, proc_type, ext_mod_id, ext_fam_id;
  step_id = model = family_id = proc_type = ext_mod_id = ext_fam_id = eax;

  step_id &= (2 << 3) - 1; //bits 0-3
  model >>= 4;             //bits 4-7
  model &= (2 << 3) - 1;
  family_id >>= 8; //bits 8-11
  family_id &= (2 << 3) - 1;
  proc_type >>= 12; //bits 12-13
  proc_type &= (2 << 1) - 1;
  ext_mod_id >>= 16; //bits 16-19
  ext_mod_id &= (2 << 3) - 1;
  ext_fam_id >>= 20; //bits 20-27
  ext_fam_id &= (2 << 7) - 1;

  print_string("\nEAX :-");
  print_string("\n  Stepping ID: ");
  print_int(step_id);
  print_string("\n  Model: ");
  print_int(model);
  print_string("\n  Family ID: ");
  print_int(family_id);
  print_string("\n  Processor Type: ");
  print_int(proc_type);
  print_string("\n  Extended Model ID: ");
  print_int(ext_mod_id);
  print_string("\n  Extended Family ID: ");
  print_int(ext_fam_id);
}

void print_ebx(uint32 ebx)
{
  // This is to print the data in the EBX register
  uint32 brand_index, cache_line_size, max_addr_id, init_apic_id;
  brand_index = cache_line_size = max_addr_id = init_apic_id = 0;
  char *bytes = (char *)&ebx;

  brand_index = bytes[0];     //bits 0-7
  cache_line_size = bytes[1]; //bits 8-15
  max_addr_id = bytes[2];     //bits 16-23
  init_apic_id = bytes[3];    //bits 24-31

  print_string("\nEBX :-");
  print_string("\n  Brand Index: ");
  print_int(brand_index);
  print_string("\n  Cache Line Size: ");
  print_int(cache_line_size);
  print_string("\n  Max Addressable ID for Logical Processors: ");
  print_int(max_addr_id);
  print_string("\n  Initial APIC ID: ");
  print_int(init_apic_id);
}

void print_edx(uint32 edx)
{
  //  this is to print the data in the EDX register.
  print_string("\nEDX :-");
  print_string("\n  bit-31 [ ");
  print_binary(edx);
  print_string(" ] bit-0");
  print_string("\n  Bit 0 : FPU-x87 FPU on Chip");
  print_string("\n  Bit 1 : VME-Virtual-8086 Mode Enhancement");
  print_string("\n  Bit 2 : DE-Debugging Extensions");
  print_string("\n  Bit 3 : PSE-Page Size Extensions");
  print_string("\n  Bit 4 : TSC-Time Stamp Counter");
  print_string("\n  Bit 5 : MSR-RDMSR and WRMSR Support");
  print_string("\n  Bit 6 : PAE-Physical Address Extensions");
}

void commandline()
{
  // This is the main commandline function
  char ch = 0;
  char keycode = 0;
  char choice;
  uint32 eax, ebx, ecx, edx;
  while (1)
  {
    display_menu();
    print_string("\n>>> ");
    choice = read_int();
    switch (choice)
    {
    case 1:
      gotoxy(35, 0);
      print_string("List of available commands:-");
      print_string("\n\n  [11]: Displays help ");
      print_string("\n\n  [22]: Shows time");
      print_string("\n\n  [33]: Opens calculator");
      print_string("\n\n  [44]: Prints ASCII character patterns on the screen.");
      break;
    case 22:
      makeErrorDialog("err");
      break;
    case 33:
      print_char(30); //Up
      print_char(17); //Right
      print_char(16); //Left
      print_char(31); //Down
      print_string("\n\n NOTE: This is shown because printing ASCII patterns is not yet implemented.");
      break;
    case 4:
      shutdown();
      break;
    case 55:
      print_string("\n\nSorry! This is not yet implemented. This will be available in the next version of Cadex.");

      break;
    case 6:
      clear_screen();
      init_vga(WHITE, BLACK);
      gotoxy((VGA_MAX_WIDTH / 2) - strlen("Cadex"), 1);
      print_color_string("Cadex", WHITE, BLACK);
      draw_box(BOX_DOUBLELINE, 0, 0, BOX_MAX_WIDTH, BOX_MAX_HEIGHT, BRIGHT_GREEN, BLACK);
      gotoxy(35, 35);
      print_color_string("Cadex", BRIGHT_RED, BLACK);
      fill_box(KEY_0, 29, 3, 10, 10, BLUE);
      draw_box(BOX_DOUBLELINE, 50, 50, 28, 10, BLUE, GREEN);
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      move_cursor_next_line();
      getchar();
      break;
    case 7:
      clear_screen();
      init_vga(WHITE, BLACK);
      cpuid(0x01, &eax, &ebx, &ecx, &edx);

      print_eax(eax);
      print_ebx(ebx);
      print_edx(edx);
      break;
    case 8:
      print_string("\n\nList of available commands:-");
      print_string("\n\n  [11]: Displays help ");
      print_string("\n\n  [22]: Shows time");
      print_string("\n\n  [33]: Opens calculator");
      print_string("\n\n  [44]: Prints ASCII character patterns on the screen.");
      break;
    case 9:
      print_string("\n\nList of available commands:-");
      print_string("\n\n  [11]: Displays help ");
      print_string("\n\n  [22]: Shows time");
      print_string("\n\n  [33]: Opens calculator");
      print_string("\n\n  [44]: Prints ASCII character patterns on the screen.");
      break;
    case 10:
      print_string("\n\nList of available commands:-");
      print_string("\n\n  [11]: Displays help ");
      print_string("\n\n  [22]: Shows time");
      print_string("\n\n  [33]: Opens calculator");
      print_string("\n\n  [44]: Prints ASCII character patterns on the screen.");
      break;
    case 11:
      // We Need to clear the screen before showing help
      clear_screen();
      init_vga(WHITE, BLUE);
      print_color_string("Cadex Haltic Fox Help", BRIGHT_GREEN, BRIGHT_CYAN);
      print_string("\n\nList of available commands:-");
      print_string("\n\n  [11]: Displays help ");
      print_string("\n  [22]: Prints ASCII character patterns on the screen.");
      print_string("\n  [33] Shows cpu information on screen. A reboot is required after entering this page.");
      print_string("\n  [44]: Pauses the system ");
      print_color_string("(Risky!)", RED, BLUE);
      print_string("\n  [55] Opens system settings (Coming sooon)");
      print_color_string("\n\n NOTE: THIS OS IS ONLY MADE FOR TUTORIAL/ EDUCATION PURPOSES. YOU CAN'T RUN THIS OS ON A REAL PC.", BRIGHT_RED, BLUE);

      break;
    default:
      do
      {
        keycode = get_input_keycode();
        if (keycode == KEY_BACKSPACE)
        {
          // You can add a clear_screen() and init_vga() here
          break;
        }
        else if (keycode == KEY_KEYPAD_PLUS)
        {
          // NOTE: This part of code does not  work.
          gotoxy((VGA_MAX_WIDTH / 2) - strlen("Error: Command not found!"), 35);
          print_string("Hey! No Plus Here.");
          break;
        }
        else
        {
          clear_screen();
          init_vga(WHITE, BLUE);
          draw_box(BOX_DOUBLELINE, 22, 5, 31, 4, RED, BLUE);
          gotoxy(26, 7);
          print_color_string("Error: Command not found!", WHITE, BLUE);
          for (uint32 i = 0; i < 20; i++)
          {
            move_cursor_next_line();
          }
        }
      } while (ch > 0);
      break;
      clear_screen();
    }
    getchar();
    init_vga(WHITE, BLUE);
  }
}
void progressBar()
{
  // For fun!
  for (uint16 i = 0; i < 60; i++)
  {
    move_cursor_next_line();
  }
  gotoxy(1, 1);
  print_string("|");
  sleep(90);
  print_string("||");
  sleep(14);
  print_string("||||");
  sleep(122);
  print_string("||||||");
  sleep(103);
  print_string("||||||||");
  sleep(93);
  print_string("||||||||||");
  sleep(104);
  print_string("||||||||||");
  sleep(10);
  print_string("||||||||||");
  sleep(122);
  print_string("");
}
void kernel_entry()
{
  // Some dummy stuff
  init_vga(WHITE, BLACK);
  gotoxy(0, 0);
  print_string("Checking cbc.bin...");
  sleep(60);
  clear_screen();
  print_string("Loading drivers...");
  sleep(120);
  clear_screen();
  print_string("Loading kernel..");
  sleep(60);
  clear_screen();
  print_string("Starting CadeOS..");
  sleep(160);
  clear_screen();
  progressBar();
  // End dummy stuff
  commandline();
}
