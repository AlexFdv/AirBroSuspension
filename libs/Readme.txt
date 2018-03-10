Problem with FEE and FreeRTOS tasks.
Thread: https://e2e.ti.com/support/microcontrollers/hercules/f/312/t/632209

"I am developing an application using FreeRTOS. I have about 7 tasks now with the same priority and I planned that one of them could read and save some data to EEPROM using FEE API (Flash EEPROM Emulation).
I enabled FEE in HalcoGen for this purpose.
BUT when I call some function (e.g. TI_Fee_Init()) within a FreeRTOS task, then a function call just hangs (nothing is returned, or task just crashes but nothing happens in this case).
If I call the same functions outside the FreeRTOS task (e.g. in the main function before vTaskStartScheduler() ) then everything works.
Could you please advice how to solve this issue?"

"FEE needs to be executed in supervisor mode. Can you check if your tasks are running in user mode or supervisory mode?"

"I tried to google, but couldn't find out how to check in which mode my tasks are executed (it looks like tasks are in user mode and scheduler is in privilege mode).
I found that I can switch to user mode by calling portSWITCH_TO_USER_MODE, actually it calls asm( " CPS #0x10"). But how to switch a processor to the supervisory mode?
Is it possible to make this switch only for one task?
I have found modes here: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0363e/DDI0363E_cortexr4_r1p3_trm.pdf#page=60&zoom=auto,-39,729
on page 2 14

b10000 User
b10001 FIQ
b10010 IRQ
b10011 Supervisor
b10111 Abort
b11011 Undefined
b11111 System

It says that "In User mode M[4:0] can be read. Writes to M[4:0] are ignored", so I can't set asm( " CPS #31") in the task itself."

"User can do a SVC call to switch to user or privileged mode. Mode = 0x10 for user and 0x1F for system mode
 In a header file, user needs to define function like below:
#pragma SWI_ALIAS(swiSwitchToMode, 1)
extern void swiSwitchToMode ( uint32 mode );
 svc.asm"