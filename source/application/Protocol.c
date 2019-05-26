/*
 * Protocol.c
 *
 *  Created on: May 26, 2019
 *      Author: oleg
 */

#include "application/Protocol.h"
#include "application/SerialController.h"
#include "RtosWrapper/Rtos.h"
#include "RtosWrapper/RtosSemaphore.h"
#include "RtosWrapper/RtosQueue.h"

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


void printError(int code, const char* text)
{
    takeSemaphore(&uartMutexSemaphore);

    printText("#ERROR:");
    printNumber(code);
    printText(":");
    printText(text);
    printText("\n");

    giveSemaphore(&uartMutexSemaphore);
}

void printSuccess()
{
    takeSemaphore(&uartMutexSemaphore);
    printText("#OK\n");
    giveSemaphore(&uartMutexSemaphore);
}

void printSuccessString(const char* text)
{
    takeSemaphore(&uartMutexSemaphore);

    printText("#OK:");
    printText(text);
    printText("\n");

    giveSemaphore(&uartMutexSemaphore);
}

void printSuccessNumber(long number)
{
    takeSemaphore(&uartMutexSemaphore);

    printText("#OK:");
    printNumber(number);
    printText("\n");

    giveSemaphore(&uartMutexSemaphore);
}

void printSuccessLevels(const LevelValues* const levels)
{
    takeSemaphore(&uartMutexSemaphore);

    printText("#OK:");
    printLevels(levels);
    printText("\n");

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
