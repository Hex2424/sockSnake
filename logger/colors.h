/**
 * @file colors.h
 *
 * MORE INFO ABOUT THE FILE'S CONTENTS
 *
 * @copyright This file is a part of the project particleSim and is distributed under MIT license which
 * should have been included with the project. If not see: https://choosealicense.com/licenses/mit/
 *
 * @author Markas Vielavičius (markas.vielavicius@bytewall.com)
 *
 * @date 2022-09-05
 */

#ifndef UTILITY_LOGGER_COLORS_H_
#define UTILITY_LOGGER_COLORS_H_

typedef const char* color_t;
#define color static color_t


color BLACK = "\033[0;30m";
color RED = "\033[0;31m";
color GREEN = "\033[0;32m";
color BROWN = "\033[0;33m";
color BLUE = "\033[0;34m";
color PURPLE = "\033[0;35m";
color CYAN = "\033[0;36m";
color LIGHT_GRAY = "\033[0;37m";
color DARK_GRAY = "\033[1;30m";
color LIGHT_RED = "\033[1;31m";
color LIGHT_GREEN = "\033[1;32m";
color YELLOW = "\033[1;33m";
color LIGHT_BLUE = "\033[1;34m";
color LIGHT_PURPLE = "\033[1;35m";
color LIGHT_CYAN = "\033[1;36m";
color LIGHT_WHITE = "\033[1;37m";
color BOLD = "\033[1m";
color FAINT = "\033[2m";
color ITALIC = "\033[3m";
color UNDERLINE = "\033[4m";
color BLINK = "\033[5m";
color NEGATIVE = "\033[7m";
color CROSSED = "\033[9m";
color END = "\033[0m";



#endif // UTILITY_LOGGER_COLORS_H_