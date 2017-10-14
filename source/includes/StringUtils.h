/*
 * StringUtils.h
 *
 *  Created on: 14 זמגע. 2017 נ.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_STRINGUTILS_H_
#define SOURCE_INCLUDES_STRINGUTILS_H_

#include <ctype.h>

bool isDigits(const char* str)
{
    if (str == NULL)
        return false;

    int len = strlen(str);

    if (len == 0)
        return false;

    int i = 0;
    while (isdigit(str[i]) && (++i < len));

    return (i == len);
}

#endif /* SOURCE_INCLUDES_STRINGUTILS_H_ */
