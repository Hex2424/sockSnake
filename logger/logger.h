/**
 * @file logger.h
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

#ifndef UTILITY_LOGGER_LOGGER_H_
#define UTILITY_LOGGER_LOGGER_H_

#include <stdio.h>
#include <stdarg.h>


void Log_d(const char* TAG, const char* expression, ...);
void Log_e(const char* TAG, const char* expression, ...);
void Log_i(const char* TAG, const char* expression, ...);
void Log_w(const char* TAG, const char* expression, ...);

#endif // UTILITY_LOGGER_LOGGER_H_