
/*
 * MIT License                                    
 *                                                 
 * Copyright (c) 2024 Leon Cotten                 
 *                                                 
 * This language is provided under the MIT Licence. 
 * See LICENSE for more information.                
 */

#ifndef FREAKYFETCH_VERSION
  #define FREAKYFETCH_VERSION "unkown" // needs to be changed by the build script
#endif

#define _GNU_SOURCE // for strcasestr

#include "fetch.h"
#include <getopt.h>
#include <stdbool.h>

// COLORS
#define NORMAL "\x1b[0m"
#define BOLD "\x1b[1m"
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define SPRING_GREEN "\x1b[38;5;120m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[0;35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"
#define PINK "\x1b[38;5;201m"
#define LPINK "\x1b[38;5;213m"

#ifdef _WIN32
  #define BLOCK_CHAR "\xdb"     // block char for colors
char* MOVE_CURSOR = "\033[21C"; // moves the cursor after printing the image or the ascii logo
#else
  #define BLOCK_CHAR "\u2587"
char* MOVE_CURSOR = "\033[18C";
#endif // _WIN32

#ifdef __DEBUG__
static bool* verbose_enabled = NULL;
#endif

// all configuration flags available
struct configuration {
  struct flags show; // all true by default
  bool show_image,   // false by default
      show_colors;   // true by default
  bool show_gpu[256];
  bool show_gpus; // global gpu toggle
};

// user's config stored on the disk
struct user_config {
  char *config_directory, // configuration directory name
      *cache_content;     // cache file content
  int read_enabled, write_enabled;
};

// reads the config file
struct configuration parse_config(struct info* user_info, struct user_config* user_config_file) {
  LOG_I("parsing config");
  char buffer[256]; // buffer for the current line
  // enabling all flags by default
  struct configuration config_flags;
  memset(&config_flags, true, sizeof(config_flags));

  config_flags.show_image = false;

  FILE* config = NULL; // config file pointer

  if (user_config_file->config_directory == NULL) { // if config directory is not set, try to open the default
    if (getenv("HOME") != NULL) {
      char homedir[512];
      sprintf(homedir, "%s/.config/freakyfetch/config", getenv("HOME"));
      LOG_V(homedir);
      config = fopen(homedir, "r");
      if (!config) {
        if (getenv("PREFIX") != NULL) {
          char prefixed_etc[512];
          sprintf(prefixed_etc, "%s/etc/freakyfetch/config", getenv("PREFIX"));
          LOG_V(prefixed_etc);
          config = fopen(prefixed_etc, "r");
        } else
          config = fopen("/etc/freakyfetch/config", "r");
      }
    }
  } else
    config = fopen(user_config_file->config_directory, "r");
  if (config == NULL) return config_flags; // if config file does not exist, return the defaults

  int gpu_cfg_count = 0;

  // reading the config file
  while (fgets(buffer, sizeof(buffer), config)) {
    sscanf(buffer, "distro=%s", user_info->os_name);
    if (sscanf(buffer, "image=\"%[^\"]\"", user_info->image_name)) {
      if (user_info->image_name[0] == '~') {                                                          // replacing the ~ character with the home directory
        memmove(&user_info->image_name[0], &user_info->image_name[1], strlen(user_info->image_name)); // remove the first char
        char temp[128] = "/home/";
        strcat(temp, user_info->user);
        strcat(temp, user_info->image_name);
        sprintf(user_info->image_name, "%s", temp);
      }
      config_flags.show_image = 1; // enable the image flag
    }

    // reading other values
    if (sscanf(buffer, "user=%[truefalse]", buffer)) {
      config_flags.show.user = !strcmp(buffer, "true");
      LOG_V(config_flags.show.user);
    }
    if (sscanf(buffer, "os=%[truefalse]", buffer)) {
      config_flags.show.os = strcmp(buffer, "false");
      LOG_V(config_flags.show.os);
    }
    if (sscanf(buffer, "host=%[truefalse]", buffer)) {
      config_flags.show.model = strcmp(buffer, "false");
      LOG_V(config_flags.show.model);
    }
    if (sscanf(buffer, "kernel=%[truefalse]", buffer)) {
      config_flags.show.kernel = strcmp(buffer, "false");
      LOG_V(config_flags.show.kernel);
    }
    if (sscanf(buffer, "cpu=%[truefalse]", buffer)) {
      config_flags.show.cpu = strcmp(buffer, "false");
      LOG_V(config_flags.show.cpu);
    }
    if (sscanf(buffer, "gpu=%d", &gpu_cfg_count)) {
      if (gpu_cfg_count > 255) {
        LOG_E("gpu config index is too high, setting it to 255");
        gpu_cfg_count = 255;
      } else if (gpu_cfg_count < 0) {
        LOG_E("gpu config index is too low, setting it to 0");
        gpu_cfg_count = 0;
      }
      config_flags.show_gpu[gpu_cfg_count] = false;
      LOG_V(config_flags.show_gpu[gpu_cfg_count]);
    }
    if (sscanf(buffer, "gpus=%[truefalse]", buffer)) { // global gpu toggle
      if (strcmp(buffer, "false") == 0) {
        config_flags.show_gpus = false;
        config_flags.show.gpu  = false; // enable getting gpu info
      } else {
        config_flags.show_gpus = true;
        config_flags.show.gpu  = true;
      }
      LOG_V(config_flags.show_gpus);
      LOG_V(config_flags.show.gpu);
    }
    if (sscanf(buffer, "ram=%[truefalse]", buffer)) {
      config_flags.show.ram = strcmp(buffer, "false");
      LOG_V(config_flags.show.ram);
    }
    if (sscanf(buffer, "resolution=%[truefalse]", buffer)) {
      config_flags.show.resolution = strcmp(buffer, "false");
      LOG_V(config_flags.show.resolution);
    }
    if (sscanf(buffer, "shell=%[truefalse]", buffer)) {
      config_flags.show.shell = strcmp(buffer, "false");
      LOG_V(config_flags.show.shell);
    }
    if (sscanf(buffer, "pkgs=%[truefalse]", buffer)) {
      config_flags.show.pkgs = strcmp(buffer, "false");
      LOG_V(config_flags.show.pkgs);
    }
    if (sscanf(buffer, "uptime=%[truefalse]", buffer)) {
      config_flags.show.uptime = strcmp(buffer, "false");
      LOG_V(config_flags.show.uptime);
    }
    if (sscanf(buffer, "colors=%[truefalse]", buffer)) {
      config_flags.show_colors = strcmp(buffer, "false");
      LOG_V(config_flags.show_colors);
    }
  }
  LOG_V(user_info->os_name);
  LOG_V(user_info->image_name);
  fclose(config);
  return config_flags;
}

// prints logo (as an image) of the given system.
int print_image(struct info* user_info) {
  LOG_I("printing image");
#ifndef __IPHONE__
  char command[256];
  if (strlen(user_info->image_name) < 1) {
    char* repl_str = strcmp(user_info->os_name, "android") == 0 ? "/data/data/com.termux/files/usr/lib/freakyfetch/freaky.png"
                     : strcmp(user_info->os_name, "macos") == 0 ? "/usr/local/lib/freakyfetch/freaky.png"
                                                                : "/usr/lib/freakyfetch/freaky.png";
    sprintf(user_info->image_name, repl_str); // image command for android
    LOG_V(user_info->image_name);
  }
  sprintf(command, "viu -t -w 18 -h 9 %s 2> /dev/null", user_info->image_name); // creating the command to show the image
  LOG_V(command);
  if (system(command) != 0) // if viu is not installed or the image is missing
    printf("\033[0E\033[3C%s\n"
           "   There was an\n"
           "    error: viu\n"
           "  is not installed\n"
           " or the image file\n"
           "   was not found\n"
           "   see IMAGES.md\n"
           "   for more info.\n\n",
           RED);
#else
  // unfortunately, the iOS stdlib does not have system(); because it reports that it is not available under iOS during compilation
  printf("\033[0E\033[3C%s\n"
         "   There was an\n"
         "   error: images\n"
         "   are currently\n"
         "  disabled on iOS.\n\n",
         RED);
#endif
  return 9;
}

// Replaces all terms in a string with another term.
void replace(char* original, char* search, char* replacer) {
  char* ch;
  char buffer[1024];
  int offset = 0;
  while ((ch = strstr(original + offset, search))) {
    strncpy(buffer, original, ch - original);
    buffer[ch - original] = 0;
    sprintf(buffer + (ch - original), "%s%s", replacer, ch + strlen(search));
    original[0] = 0;
    strcpy(original, buffer);
    offset = ch - original + strlen(replacer);
  }
}

// Replaces all terms in a string with another term, case insensitive
void replace_ignorecase(char* original, char* search, char* replacer) {
  char* ch;
  char buffer[1024];
  int offset = 0;
#ifdef _WIN32
  #define strcasestr(o, s) strstr(o, s)
#endif
  while ((ch = strcasestr(original + offset, search))) {
    strncpy(buffer, original, ch - original);
    buffer[ch - original] = 0;
    sprintf(buffer + (ch - original), "%s%s", replacer, ch + strlen(search));
    original[0] = 0;
    strcpy(original, buffer);
    offset = ch - original + strlen(replacer);
  }
}

#ifdef _WIN32
// windows sucks and hasn't a strstep, so I copied one from https://stackoverflow.com/questions/8512958/is-there-a-windows-variant-of-strsep-function
char* strsep(char** stringp, const char* delim) {
  char* start = *stringp;
  char* p;
  p = (start != NULL) ? strpbrk(start, delim) : NULL;
  if (p == NULL)
    *stringp = NULL;
  else {
    *p       = '\0';
    *stringp = p + 1;
  }
  return start;
}
#endif

// freakifies distro name
void freak_name(struct info* user_info) {
#define STRING_TO_FREAK(original, freakified) \
  if (strcmp(user_info->os_name, original) == 0) sprintf(user_info->os_name, "%s", freakified)
  // linux
  STRING_TO_FREAK("alpine", "Freakpine");
  else STRING_TO_FREAK("amogos", "Freaky Imposter");
  else STRING_TO_FREAK("android", "Freakdroid");
  else STRING_TO_FREAK("arch", "Freaky Arch");
  else STRING_TO_FREAK("arcolinux", "Freaky Arco");
  else STRING_TO_FREAK("artix", "Freaky Artix");
  else STRING_TO_FREAK("debian", "Debfreakian");
  else STRING_TO_FREAK("devuan", "Freaky Devuan");
  else STRING_TO_FREAK("deepin", "Freaky Deepin");
  else STRING_TO_FREAK("endeavouros", "Freaky endeavouros");
  else STRING_TO_FREAK("EndeavourOS", "Freaky EndeavourOS");
  else STRING_TO_FREAK("fedora", "Freakdora");
  else STRING_TO_FREAK("femboyos", "Freaky Femboy");
  else STRING_TO_FREAK("gentoo", "Freaky Gentoo");
  else STRING_TO_FREAK("gnu", "Freaky GNU");
  else STRING_TO_FREAK("guix", "Freaky Guix");
  else STRING_TO_FREAK("linuxmint", "Linux FreakMint");
  else STRING_TO_FREAK("manjaro", "Freakjaro");
  else STRING_TO_FREAK("manjaro-arm", "Freakjaroo ARM");
  else STRING_TO_FREAK("neon", "Freakeon");
  else STRING_TO_FREAK("nixos", "Freaky NixOS");
  else STRING_TO_FREAK("opensuse-leap", "OpenFreakSuse");
  else STRING_TO_FREAK("opensuse-tumbleweed", "OpenFreakSuse Tumbleweed");
  else STRING_TO_FREAK("pop", "Freaky PopOS");
  else STRING_TO_FREAK("raspbian", "Raspfreakian");
  else STRING_TO_FREAK("rocky", "Freaky Rocky");
  else STRING_TO_FREAK("slackware", "Freaky Slackware");
  else STRING_TO_FREAK("solus", "Freaky Solus");
  else STRING_TO_FREAK("ubuntu", "Freakbuntu");
  else STRING_TO_FREAK("void", "Freaky void");
  else STRING_TO_FREAK("xerolinux", "Freaky Xero");

  // BSD
  else STRING_TO_FREAK("freebsd", "FreakBSD");
  else STRING_TO_FREAK("openbsd", "Freaky OpenBSD");

  // Apple family
  else STRING_TO_FREAK("macos", "Freaky macOS");
  else STRING_TO_FREAK("ios", "Freaky iOS");

  // Windows
  else STRING_TO_FREAK("windows", "Freakdows");

  else sprintf(user_info->os_name, "%s", "Ultra Freaky OS");
#undef STRING_TO_FREAK
}

// freakifies kernel name
void freak_kernel(char* kernel) {
#define KERNEL_TO_FREAK(str, original, freakified) \
  if (strcmp(str, original) == 0) sprintf(str, "%s", freakified)

  LOG_I("freakifying kernel");

  char* temp_kernel = kernel;
  char* token;
  char splitted[16][128] = {};

  int count = 0;
  while ((token = strsep(&temp_kernel, " "))) { // split kernel name
    strcpy(splitted[count], token);
    count++;
  }
  strcpy(kernel, "");
  for (int i = 0; i < 16; i++) {
    // replace kernel name with freakified version
    KERNEL_TO_FREAK(splitted[i], "Linux", "Freaky Linux");
    else KERNEL_TO_FREAK(splitted[i], "linux", "Freaky Linux");
    else KERNEL_TO_FREAK(splitted[i], "alpine", "Freakpine");
    else KERNEL_TO_FREAK(splitted[i], "amogos", "Freaky Imposter");
    else KERNEL_TO_FREAK(splitted[i], "android", "Freakdroid");
    else KERNEL_TO_FREAK(splitted[i], "arch", "Freaky Arch");
    else KERNEL_TO_FREAK(splitted[i], "artix", "Freaky Artix");
    else KERNEL_TO_FREAK(splitted[i], "debian", "Debfreakian");
    else KERNEL_TO_FREAK(splitted[i], "deepin", "Freaky Deepin");
    else KERNEL_TO_FREAK(splitted[i], "endeavouros", "Freaky EndeavourOS");
    else KERNEL_TO_FREAK(splitted[i], "EndeavourOS", "Freaky EndeavourOS");
    else KERNEL_TO_FREAK(splitted[i], "fedora", "Freakdora");
    else KERNEL_TO_FREAK(splitted[i], "femboyos", "Freaky Femboy");
    else KERNEL_TO_FREAK(splitted[i], "gentoo", "Freaky Gentoo");
    else KERNEL_TO_FREAK(splitted[i], "gnu", "Freaky GNU");
    else KERNEL_TO_FREAK(splitted[i], "guix", "Freaky GUIX");
    else KERNEL_TO_FREAK(splitted[i], "linuxmint", "Linx FreakMint");
    else KERNEL_TO_FREAK(splitted[i], "manjaro", "Freakjaro");
    else KERNEL_TO_FREAK(splitted[i], "manjaro-arm", "Freakjaro ARM");
    else KERNEL_TO_FREAK(splitted[i], "neon", "Freakeon");
    else KERNEL_TO_FREAK(splitted[i], "nixos", "Freaky NixOS");
    else KERNEL_TO_FREAK(splitted[i], "opensuse-leap", "OpenFreakSUSE Leap");
    else KERNEL_TO_FREAK(splitted[i], "opensuse-tumbleweed", "OpenFreakSUSE Tumbleweed");
    else KERNEL_TO_FREAK(splitted[i], "pop", "Freaky PopOS");
    else KERNEL_TO_FREAK(splitted[i], "raspbian", "Raspfreakian");
    else KERNEL_TO_FREAK(splitted[i], "rocky", "Freaky Rocky");
    else KERNEL_TO_FREAK(splitted[i], "slackware", "Freaky Slackware");
    else KERNEL_TO_FREAK(splitted[i], "solus", "Freaky solus");
    else KERNEL_TO_FREAK(splitted[i], "ubuntu", "Freakbuntu");
    else KERNEL_TO_FREAK(splitted[i], "void", "Freaky void");
    else KERNEL_TO_FREAK(splitted[i], "xerolinux", "Freaky Xero");

    // BSD
    else KERNEL_TO_FREAK(splitted[i], "freebsd", "FreakBSD");
    else KERNEL_TO_FREAK(splitted[i], "openbsd", "Freaky OpenBSD");

    // Apple family
    else KERNEL_TO_FREAK(splitted[i], "macos", "Freaky macOS");
    else KERNEL_TO_FREAK(splitted[i], "ios", "Freaky iOS");

    // Windows
    else KERNEL_TO_FREAK(splitted[i], "windows", "Freakdows");

    if (i != 0) strcat(kernel, " ");
    strcat(kernel, splitted[i]);
  }
#undef KERNEL_TO_FREAK
  LOG_V(kernel);
}

// freakifies hardware names
void freak_hw(char* hwname) {
  LOG_I("freakifing hardware")
#define HW_TO_FREAK(original, freakified) replace_ignorecase(hwname, original, freakified);
  HW_TO_FREAK("lenovo", "Freaky Lenovo")
  HW_TO_FREAK("cpu", "Freaky CPU");
  HW_TO_FREAK("core", "Freaky Core");
  HW_TO_FREAK("gpu", "Freaky GPU")
  HW_TO_FREAK("graphics", "Freaky Graphics")
  HW_TO_FREAK("corporation", "Freaky Corporation")
  HW_TO_FREAK("nvidia", "Freaky Nvidia")
  HW_TO_FREAK("mobile", "Freaky Mobile")
  HW_TO_FREAK("intel", "Freaky Intel")
  HW_TO_FREAK("celeron", "Freaky Celeron")
  HW_TO_FREAK("radeon", "Freaky Radeon")
  HW_TO_FREAK("geforce", "Freaky GeForce")
  HW_TO_FREAK("raspberry", "Freaky Raspberry")
  HW_TO_FREAK("broadcom", "Freaky Broadcom")
  HW_TO_FREAK("motorola", "Freaky Motorola")
  HW_TO_FREAK("proliant", "Freaky ProLiant")
  HW_TO_FREAK("poweredge", "Freaky PowerEdge")
  HW_TO_FREAK("apple", "Freaky Apple")
  HW_TO_FREAK("electronic", "Freaky Electronic")
  HW_TO_FREAK("processor", "Freaky Processor")
  HW_TO_FREAK("microsoft", "Freakysoft")
  HW_TO_FREAK("ryzen", "Freaky Ryzen")
  HW_TO_FREAK("advanced", "Freaky Advanced")
  HW_TO_FREAK("micro", "Freaky Micro")
  HW_TO_FREAK("devices", "Freaky Devices")
  HW_TO_FREAK("inc.", "Freaky inc.")
  HW_TO_FREAK("lucienne", "Freaky Lucienne")
  HW_TO_FREAK("tuxedo", "Freaky Tuxedo")
  HW_TO_FREAK("aura", "+100 aura")
#undef HW_TO_FREAK
}

// freakifies package manager names
void freak_pkgman(char* pkgman_name) {
  LOG_I("uwufing package managers")
#define PKGMAN_TO_FREAK(original, freakified) replace_ignorecase(pkgman_name, original, freakified);
  // these package managers do not have edits yet:
  // apk, apt, guix, nix, pkg, xbps
  PKGMAN_TO_FREAK("brew-cask", "Freaky brew-cask");
  PKGMAN_TO_FREAK("brew-cellar", "Freaky brew-cellar");
  PKGMAN_TO_FREAK("emerge", "Freaky emerge");
  PKGMAN_TO_FREAK("flatpak", "Freakpak");
  PKGMAN_TO_FREAK("pacman", "Freaky pacman");
  PKGMAN_TO_FREAK("port", "Freaky port");
  PKGMAN_TO_FREAK("snap", "Freaky snap");
#undef PKGMAN_TO_FREAK
}

// freakifies everything
void freakify_all(struct info* user_info) {
  LOG_I("freakifing everything");
  if (strcmp(user_info->os_name, "windows"))
    MOVE_CURSOR = "\033[21C"; // to print windows logo on not windows systems
  freak_kernel(user_info->kernel);
  for (int i = 0; user_info->gpu_model[i][0]; i++) freak_hw(user_info->gpu_model[i]);
  freak_hw(user_info->cpu_model);
  LOG_V(user_info->cpu_model);
  freak_hw(user_info->model);
  LOG_V(user_info->model);
  freak_pkgman(user_info->pkgman_name);
  LOG_V(user_info->pkgman_name);
}

// prints all the collected info and returns the number of printed lines
int print_info(struct configuration* config_flags, struct info* user_info) {
  int line_count = 0;
#ifdef _WIN32
  // prints without overflowing the terminal width
  #define responsively_printf(buf, format, ...)     \
    {                                               \
      sprintf(buf, format, __VA_ARGS__);            \
      printf("%.*s\n", user_info->ws_col - 4, buf); \
      line_count++;                                 \
    }
#else // _WIN32
  // prints without overflowing the terminal width
  #define responsively_printf(buf, format, ...)         \
    {                                                   \
      sprintf(buf, format, __VA_ARGS__);                \
      printf("%.*s\n", user_info->win.ws_col - 4, buf); \
      line_count++;                                     \
    }
#endif                  // _WIN32
  char print_buf[1024]; // for responsively print

  // print collected info - from host to cpu info
  if (config_flags->show.user)
    responsively_printf(print_buf, "%s%s%s%s@%s", MOVE_CURSOR, NORMAL, BOLD, user_info->user, user_info->host);
  freak_name(user_info);
  if (config_flags->show.os)
    responsively_printf(print_buf, "%s%s%sOS     %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->os_name);
  if (config_flags->show.model)
    responsively_printf(print_buf, "%s%s%sMODEL  %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->model);
  if (config_flags->show.kernel)
    responsively_printf(print_buf, "%s%s%sKERNEL   %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->kernel);
  if (config_flags->show.cpu)
    responsively_printf(print_buf, "%s%s%sCPU    %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->cpu_model);

  for (int i = 0; i < 256; i++) {
    if (config_flags->show_gpu[i])
      if (user_info->gpu_model[i][0])
        responsively_printf(print_buf, "%s%s%sGPU    %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->gpu_model[i]);
  }

  if (config_flags->show.ram) // print ram
    responsively_printf(print_buf, "%s%s%sMEMORY   %s%i MiB/%i MiB", MOVE_CURSOR, NORMAL, BOLD, NORMAL, (user_info->ram_used), user_info->ram_total);
  if (config_flags->show.resolution) // print resolution
    if (user_info->screen_width != 0 || user_info->screen_height != 0)
      responsively_printf(print_buf, "%s%s%sRESOLUTION%s  %dx%d", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->screen_width, user_info->screen_height);
  if (config_flags->show.shell) // print shell name
    responsively_printf(print_buf, "%s%s%sSHELL    %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->shell);
  if (config_flags->show.pkgs) // print pkgs
    responsively_printf(print_buf, "%s%s%sPKGS     %s%d: %s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->pkgs, user_info->pkgman_name);
  // #endif
  if (config_flags->show.uptime) {
    switch (user_info->uptime) { // formatting the uptime which is store in seconds
    case 0 ... 3599:
      responsively_printf(print_buf, "%s%s%sUPTIME %s%lim", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->uptime / 60 % 60);
      break;
    case 3600 ... 86399:
      responsively_printf(print_buf, "%s%s%sUPTIME %s%lih, %lim", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->uptime / 3600, user_info->uptime / 60 % 60);
      break;
    default:
      responsively_printf(print_buf, "%s%s%sUPTIME %s%lid, %lih, %lim", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->uptime / 86400, user_info->uptime / 3600 % 24, user_info->uptime / 60 % 60);
    }
  }
  // clang-format off
	if (config_flags->show_colors)
		printf("%s"	BOLD BLACK BLOCK_CHAR BLOCK_CHAR RED BLOCK_CHAR
								BLOCK_CHAR GREEN BLOCK_CHAR BLOCK_CHAR YELLOW
								BLOCK_CHAR BLOCK_CHAR BLUE BLOCK_CHAR BLOCK_CHAR
								MAGENTA BLOCK_CHAR BLOCK_CHAR CYAN BLOCK_CHAR
								BLOCK_CHAR WHITE BLOCK_CHAR BLOCK_CHAR NORMAL "\n", MOVE_CURSOR);
  // clang-format on
  return line_count;
}

// writes cache to cache file
void write_cache(struct info* user_info) {
  LOG_I("writing cache");
  char cache_file[512];
  sprintf(cache_file, "%s/.cache/freakyfetch.cache", getenv("HOME")); // default cache file location
  LOG_V(cache_file);
  FILE* cache_fp = fopen(cache_file, "w");
  if (cache_fp == NULL) {
    LOG_E("Failed to write to %s!", cache_file);
    return;
  }
  // writing all info to the cache file
  fprintf( // writing most of the values to config file
      cache_fp,
      "user=%s\nhost=%s\nversion_name=%s\nhost_model=%s\nkernel=%s\ncpu=%"
      "s\nscreen_width=%d\nscreen_height=%d\nshell=%s\npkgs=%d\npkgman_name=%"
      "s\n",
      user_info->user, user_info->host, user_info->os_name, user_info->model, user_info->kernel,
      user_info->cpu_model, user_info->screen_width, user_info->screen_height, user_info->shell,
      user_info->pkgs, user_info->pkgman_name);

  for (int i = 0; user_info->gpu_model[i][0]; i++) // writing gpu names to file
    fprintf(cache_fp, "gpu=%s\n", user_info->gpu_model[i]);

  fclose(cache_fp);
  return;
}

// reads cache file if it exists
int read_cache(struct info* user_info) {
  LOG_I("reading cache");
  char cache_file[512];
  sprintf(cache_file, "%s/.cache/freakyfetch.cache", getenv("HOME")); // cache file location
  LOG_V(cache_file);
  FILE* cache_fp = fopen(cache_file, "r");
  if (cache_fp == NULL) return 0;
  char buffer[256];                                 // line buffer
  int gpuc = 0;                                     // gpu counter
  while (fgets(buffer, sizeof(buffer), cache_fp)) { // reading the file
    sscanf(buffer, "user=%99[^\n]", user_info->user);
    sscanf(buffer, "host=%99[^\n]", user_info->host);
    sscanf(buffer, "version_name=%99[^\n]", user_info->os_name);
    sscanf(buffer, "host_model=%99[^\n]", user_info->model);
    sscanf(buffer, "kernel=%99[^\n]", user_info->kernel);
    sscanf(buffer, "cpu=%99[^\n]", user_info->cpu_model);
    if (sscanf(buffer, "gpu=%99[^\n]", user_info->gpu_model[gpuc]) != 0) gpuc++;
    sscanf(buffer, "screen_width=%i", &user_info->screen_width);
    sscanf(buffer, "screen_height=%i", &user_info->screen_height);
    sscanf(buffer, "shell=%99[^\n]", user_info->shell);
    sscanf(buffer, "pkgs=%i", &user_info->pkgs);
    sscanf(buffer, "pkgman_name=%99[^\n]", user_info->pkgman_name);
  }
  LOG_V(user_info->user);
  LOG_V(user_info->host);
  LOG_V(user_info->os_name);
  LOG_V(user_info->model);
  LOG_V(user_info->kernel);
  LOG_V(user_info->cpu_model);
  LOG_V(user_info->gpu_model[gpuc]);
  LOG_V(user_info->screen_width);
  LOG_V(user_info->screen_height);
  LOG_V(user_info->shell);
  LOG_V(user_info->pkgs);
  LOG_V(user_info->pkgman_name);
  fclose(cache_fp);
  return 1;
}

// prints logo (as ascii art) of the given system.
int print_ascii(struct info* user_info) {
  FILE* file;
  char ascii_file[1024];
  // First tries to get ascii art file from local directory. Useful for debugging
  sprintf(ascii_file, "./res/ascii/freaky.txt");
  LOG_V(ascii_file);
  file = fopen(ascii_file, "r");
  if (!file) { // if the file does not exist in the local directory, open it from the installation directory
    if (strcmp(user_info->os_name, "android") == 0)
      sprintf(ascii_file, "/data/data/com.termux/files/usr/lib/freakyfetch/ascii/freaky.txt");
    else if (strcmp(user_info->os_name, "macos") == 0)
      sprintf(ascii_file, "/usr/local/lib/freakyfetch/ascii/freaky.txt");
    else
      sprintf(ascii_file, "/usr/lib/freakyfetch/ascii/freaky.txt");
    LOG_V(ascii_file);

    file = fopen(ascii_file, "r");
    if (!file) {
      // Prevent infinite loops
      if (strcmp(user_info->os_name, "unknown") == 0) {
        LOG_E("No\nunknown\nascii\nfile\n\n\n\n");
        return 7;
      }
      sprintf(user_info->os_name, "unknown"); // current os is not supported
      LOG_V(user_info->os_name);
      return print_ascii(user_info);
    }
  }
  char buffer[256]; // line buffer
  int line_count = 1;
  printf("\n");
  while (fgets(buffer, 256, file)) { // replacing color placecholders
    replace(buffer, "{NORMAL}", NORMAL);
    replace(buffer, "{BOLD}", BOLD);
    replace(buffer, "{BLACK}", BLACK);
    replace(buffer, "{RED}", RED);
    replace(buffer, "{GREEN}", GREEN);
    replace(buffer, "{SPRING_GREEN}", SPRING_GREEN);
    replace(buffer, "{YELLOW}", YELLOW);
    replace(buffer, "{BLUE}", BLUE);
    replace(buffer, "{MAGENTA}", MAGENTA);
    replace(buffer, "{CYAN}", CYAN);
    replace(buffer, "{WHITE}", WHITE);
    replace(buffer, "{PINK}", PINK);
    replace(buffer, "{LPINK}", LPINK);
    replace(buffer, "{BLOCK}", BLOCK_CHAR);
    replace(buffer, "{BLOCK_VERTICAL}", BLOCK_CHAR);
    replace(buffer, "{BACKGROUND_GREEN}", "\e[0;42m");
    replace(buffer, "{BACKGROUND_RED}", "\e[0;41m");
    replace(buffer, "{BACKGROUND_WHITE}", "\e[0;47m");
    printf("%s", buffer); // print the line after setting the color
    line_count++;
  }
  // Always set color to NORMAL, so there's no need to do this in every ascii file.
  printf(NORMAL);
  fclose(file);
  return line_count;
}

/* prints distribution list
   distributions are listed by distribution branch
   to make the output easier to understand by the user.*/
void list(char* arg) {
  LOG_I("printing supported distro list");
  // clang-format off
	printf("%s -d <options>\n  Available distributions:\n    freaky\n"); // Other/spare distributions colors
  // clang-format on
}

// prints the usage
void usage(char* arg) {
  LOG_I("printing usage");
  printf("Usage: %s <args>\n"
         "    -c  --config        use custom config path\n"
         "    -h, --help          prints this help page\n"
#ifndef __IPHONE__
         "    -i, --image         prints logo as image and use a custom image "
         "if provided\n"
         "                        %sworks in most terminals\n"
#else
         "    -i, --image         prints logo as image and use a custom image "
         "if provided\n"
         "                        %sdisabled under iOS\n"
#endif
         "                        read README.md for more info%s\n"
         "    -l, --list          lists all supported distributions\n"
         "    -V, --version       prints the current uwufetch version\n"
#ifdef __DEBUG__
         "    -v, --verbose       logs everything\n"
#endif
         "    -w, --write-cache   writes to the cache file (~/.cache/uwufetch.cache)\n"
         "    -r, --read-cache    reads from the cache file (~/.cache/uwufetch.cache)\n",
         arg,
#ifndef __IPHONE__
         BLUE,
#else
         RED,
#endif
         NORMAL);
}

// the main function is on the bottom of the file to avoid double function declarations
int main(int argc, char* argv[]) {
#ifdef __DEBUG__
  verbose_enabled = get_verbose_handle();
#endif
  struct user_config user_config_file = {0};
  struct info user_info               = {0};
  struct configuration config_flags   = parse_config(&user_info, &user_config_file);
  char* custom_distro_name            = NULL;
  char* custom_image_name             = NULL;

#ifdef _WIN32
  // packages disabled by default because chocolatey is too slow
  config_flags.show.pkgs = 0;
#endif

  int opt                      = 0;
  struct option long_options[] = {
      {"config", required_argument, NULL, 'c'},
      {"distro", required_argument, NULL, 'd'},
      {"help", no_argument, NULL, 'h'},
      {"image", optional_argument, NULL, 'i'},
      {"list", no_argument, NULL, 'l'},
      {"read-cache", no_argument, NULL, 'r'},
      {"version", no_argument, NULL, 'V'},
#ifdef __DEBUG__
      {"verbose", no_argument, NULL, 'v'},
#endif
      {"write-cache", no_argument, NULL, 'w'},
      {0}};
#ifdef __DEBUG__
  #define OPT_STRING "c:d:hi::lrVvw"
#else
  #define OPT_STRING "c:d:hi::lrVw"
#endif

  // reading cmdline options
  while ((opt = getopt_long(argc, argv, OPT_STRING, long_options, NULL)) != -1) {
    switch (opt) {
    case 'c': // set the config directory
      user_config_file.config_directory = optarg;
      config_flags                      = parse_config(&user_info, &user_config_file);
      break;
    case 'd': // set the distribution name
      custom_distro_name = optarg;
      break;
    case 'h':
      usage(argv[0]);
      return 0;
    case 'i': // set ascii logo as output
      config_flags.show_image = true;
      if (argv[optind]) custom_image_name = argv[optind];
      break;
    case 'l':
      list(argv[0]);
      return 0;
    case 'r':
      user_config_file.read_enabled = true;
      break;
    case 'V':
      printf("Freakyfetch version %s\n", FREAKYFETCH_VERSION);
      return 0;
#ifdef __DEBUG__
    case 'v':
      *verbose_enabled = true;
      LOG_I("version %s", FREAKYFETCH_VERSION);
      break;
#endif
    case 'w':
      user_config_file.write_enabled = true;
      break;
    default:
      return 1;
    }
  }

  if (user_config_file.read_enabled) {
    // if no cache file found write to it
    if (!read_cache(&user_info)) {
      user_config_file.read_enabled  = false;
      user_config_file.write_enabled = true;
    } else {
      int buf_sz = 256;
      char buffer[buf_sz]; // line buffer
      struct thread_varg vargp = {
          buffer, &user_info, NULL, {true, true, true, true, true, true, true, true}};
      if (config_flags.show.ram) get_ram(&vargp);
      if (config_flags.show.uptime) {
        LOG_I("getting additional not-cached info");
        get_sys(&user_info);
        get_upt(&vargp);
      }
    }
  }
  if (!user_config_file.read_enabled)
    get_info(config_flags.show, &user_info);
  LOG_V(user_info.gpu_model[1]);

  if (user_config_file.write_enabled) {
    write_cache(&user_info);
  }
  if (custom_distro_name) sprintf(user_info.os_name, "%s", custom_distro_name);
  if (custom_image_name) sprintf(user_info.image_name, "%s", custom_image_name);

  freakify_all(&user_info);

  // print ascii or image and align cursor for print_info()
  printf("\033[%dA", config_flags.show_image ? print_image(&user_info) : print_ascii(&user_info));

  // print info and move cursor down if the number of printed lines is smaller that the default image height
  int to_move = 9 - print_info(&config_flags, &user_info);
  printf("\033[%d%c", to_move < 0 ? -to_move : to_move, to_move < 0 ? 'A' : 'B');
  LOG_I("Execution completed successfully!");
  return 0;
}
