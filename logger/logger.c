/**
 * @file logger.c
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


#include "logger.h"
#include "colors.h"

////////////////////////////////
// DEFINES

#define COLOR_PRINT_MACRO(LETTER, COLOR)                \
    va_list args;                               \
    va_start(args, expression);                 \
    logData_(TAG, LETTER, COLOR, expression, args);     \
    va_end(args);

////////////////////////////////
// PRIVATE CONSTANTS

static const char* TAG = "LOGGER";

////////////////////////////////
// PRIVATE TYPES


////////////////////////////////
// PRIVATE METHODS

static inline void logData_(const char* TAG, const char letter, const char* colorString, const char* expression, va_list args);


////////////////////////////////
// IMPLEMENTATION


/**
 * @brief Public method for logging DEBUG messages
 * 
 * @param[in] TAG               File tag 
 * @param[in] expression[in]    Expression, works same as printf
 * @param ...                   Variadic arguments (numbers, strings, chars...) same as in printf
 */
void inline Log_d(const char* TAG, const char* expression, ...)
{
    #if VERBOSE_LEVEL > 2
        COLOR_PRINT_MACRO('D', LIGHT_WHITE);
    #endif
}


/**
 * @brief Public method for logging ERROR messages
 * 
 * @param[in] TAG               File tag 
 * @param[in] expression[in]    Expression, works same as printf
 * @param ...                   Variadic arguments (numbers, strings, chars...) same as in printf
 */
void inline Log_e(const char* TAG, const char* expression, ...)
{
    #if VERBOSE_LEVEL > 0
        COLOR_PRINT_MACRO('E', LIGHT_RED);
    #endif
}


/**
 * @brief Public method for logging INFORMATIONAL messages
 * 
 * @param[in] TAG               File tag 
 * @param[in] expression[in]    Expression, works same as printf
 * @param ...                   Variadic arguments (numbers, strings, chars...) same as in printf
 */
void inline Log_i(const char* TAG, const char* expression, ...)
{
    #if VERBOSE_LEVEL > 1
        COLOR_PRINT_MACRO('I', LIGHT_BLUE);
    #endif
}

/**
 * @brief Public method for logging WARNING messages
 * 
 * @param[in] TAG               File tag 
 * @param[in] expression[in]    Expression, works same as printf
 * @param ...                   Variadic arguments (numbers, strings, chars...) same as in printf
 */
void inline Log_w(const char* TAG, const char* expression, ...)
{
    #if VERBOSE_LEVEL > 1
        COLOR_PRINT_MACRO('W', YELLOW);
    #endif
}

/**
 * @brief Public method for logging any type of messages
 * 
 * @param[in] TAG           File tag 
 * @param colorString       String 
 * @param expression 
 * @param args 
 */
static void logData_(const char* TAG, const char letter, const char* colorString, const char* expression, va_list args)
{
    printf("%s(%c)(%s)-> ", colorString, letter, TAG);
    vprintf(expression, args);
    printf("%s\n", END);
}