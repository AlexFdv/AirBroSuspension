/*
 * StringUtils.h
 *
 *  Created on: 14 זמגע. 2017 נ.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_STRINGUTILS_H_
#define SOURCE_INCLUDES_STRINGUTILS_H_

#include <ctype.h>

bool isDigits(const char* str, char stopChar)
{
    if (str == NULL)
        return false;

    size_t len = 0;
    char* pch = strchr(str, stopChar);
    if (pch != NULL)
        len = pch - str;
    else
        len = strlen(str);

    if (len == 0)
        return false;

    size_t i = 0;
    while (isdigit(str[i]) && (++i < len));

    return (i == len);
}

#endif /* SOURCE_INCLUDES_STRINGUTILS_H_ */
