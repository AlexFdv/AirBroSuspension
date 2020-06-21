/*
 * Protocol.c
 *
 *  Created on: May 26, 2019
 *      Author: oleg
 */

#include <Util.h>
#include "application/Protocol.h"
#include "application/SerialController.h"
#include "RtosWrapper/Rtos.h"
#include "RtosWrapper/RtosSemaphore.h"
#include "RtosWrapper/RtosQueue.h"

#include "Utils/Util.h"

#define OK_HEADER "#OK"
#define ERROR_HEADER "#ERROR"
#define NEW_LINE_CHAR "\n"
#define CARRIAGE_RETURN_CHAR "\r"

#define LINE_ENDING_DEFAULT         NEW_LINE_CHAR    // for unit tests and production
#define LINE_ENDING_FOR_TERMINAL    CARRIAGE_RETURN_CHAR NEW_LINE_CHAR

#define LINE_ENDING LINE_ENDING_DEFAULT



/*========================== Static members ==========================*/

static Semaphore uartMutexSemaphore;


/*========================= Static functions =========================*/
inline void printLevels(const LevelValues* const levels);


/*========================= Global functions =========================*/

bool protocol_init(void)
{
    uartMutexSemaphore = createMutexSemaphore();

    return (uartMutexSemaphore.handle != NULL);
}

void printErrorStr(int code, const char* text)
{
    takeSemaphore(&uartMutexSemaphore);

    printText(ERROR_HEADER ":");
    printNumber(code);
    printText(":");
    printText(text);
    printText(LINE_ENDING);

    giveSemaphore(&uartMutexSemaphore);
}

void printError(int code)
{
    printErrorStr(code, err2str(code));
}

void printSuccess()
{
    takeSemaphore(&uartMutexSemaphore);
    printText(OK_HEADER LINE_ENDING);
    giveSemaphore(&uartMutexSemaphore);
}

void printSuccessString(const char* text)
{
    takeSemaphore(&uartMutexSemaphore);

    printText(OK_HEADER ":");
    printText(text);
    printText(LINE_ENDING);

    giveSemaphore(&uartMutexSemaphore);
}

void printSuccessNumber(long number)
{
    takeSemaphore(&uartMutexSemaphore);

    printText(OK_HEADER ":");
    printNumber(number);
    printText(LINE_ENDING);

    giveSemaphore(&uartMutexSemaphore);
}

void printSuccessLevels(const LevelValues* const levels)
{
    takeSemaphore(&uartMutexSemaphore);

    printText(OK_HEADER ":");
    printLevels(levels);
    printText(LINE_ENDING);

    giveSemaphore(&uartMutexSemaphore);
}


void sendDiagnosticData(const void *data, size_t len)
{
    sciSendDataLin((uint8*)data, len);
}

/*================= Static functions implementations =================*/

inline void printLevels(const LevelValues* const levels)
{
    portSHORT i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        printNumber(levels->wheels[i]);
        printText(":");
    }
}
