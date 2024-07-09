/*
 * MIT License                                    
 *                                                 
 * Copyright (c) 2024 Leon Cotten                 
 *                                                 
 * This language is provided under the MIT Licence. 
 * See LICENSE for more information.                
 */

#include <stdio.h>
#include <stdlib.h>

#define ANSI_PINK "\033[95m"
#define ANSI_BOLD "\033[1m"
#define ANSI_RESET "\033[0m"
#define RESET_TO_PINK ANSI_RESET ANSI_PINK

int main() {
  system("chafa /usr/local/bin/freaky.png");
  puts(ANSI_PINK"╭──────────👅" ANSI_BOLD "FREAKY" ANSI_RESET ANSI_PINK"👅──────────╮  ");
  puts(" " ANSI_BOLD "OS"       RESET_TO_PINK": 👅Freakbuntu👅         ");
  puts(" " ANSI_BOLD "Host"     RESET_TO_PINK": 👅FreakyServer👅         ");
  puts(" " ANSI_BOLD "Kernel"   RESET_TO_PINK": 👅Freaknix👅               ");
  puts(" " ANSI_BOLD "Shell"    RESET_TO_PINK": 👅Freaksh👅               ");
  puts(" " ANSI_BOLD "WM"       RESET_TO_PINK": 👅Freakyland👅         ");
  puts(" " ANSI_BOLD "Terminal" RESET_TO_PINK": 👅Alacfreaky👅               ");
  puts(" " ANSI_BOLD "Font"     RESET_TO_PINK": 👅Hack Freaky Font👅     ");
  puts("╰──────────────────────────────╯  ");

  return 0;
}
