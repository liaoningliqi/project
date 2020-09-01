
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

  .syntax unified
  .cpu cortex-m3
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler

/* start address for the initialization values of the .data section. defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */  
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss
/* stack used for SystemInit_ExtMemCtl; always internal RAM used */

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:  
  ldr   sp, =_estack     /* set stack pointer */

/* Copy the data segment initializers from flash to SRAM */  
  movs  r1, #0
  b  LoopCopyDataInit

CopyDataInit:
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4
    
LoopCopyDataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds  r2, r0, r1
  cmp  r2, r3
  bcc  CopyDataInit
  ldr  r2, =_sbss
  b  LoopFillZerobss
/* Zero fill the bss segment. */  
FillZerobss:
  movs  r3, #0
  str  r3, [r2], #4
    
LoopFillZerobss:
  ldr  r3, = _ebss
  cmp  r2, r3
  bcc  FillZerobss

/* Call the clock system intitialization function.*/
/*  bl  SystemInit   */
/* Call static constructors */
  bl __libc_init_array
/* Call the application entry point. */
  bl  main
  bx  lr    
  b  .
.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an 
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None     
 * @retval None       
*/
  .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler
/******************************************************************************
*
* The minimal vector table for a Cortex M3. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
* 
*******************************************************************************/
  .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors
    
    
g_pfnVectors:
  .word  _estack
  .word  Reset_Handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler
  
  /* External Interrupts */
  .word  n00_BB0_IRQHandler_ERR
  .word  n01_BB1_IRQHandler_LLE
  .word  n02_RTC_M0_IRQHandler
  .word  n03_RTC_M1_IRQHandler
  .word  n04_TMR0_IRQHandler
  .word  n05_TMR1_IRQHandler
  .word  n06_TMR2_IRQHandler
  .word  n07_EWK_IRQHandler
  .word  n08_EXINT_IRQHandler
  .word  n09_GPIO_IRQHandler
  .word  n10_BB0_IRQHandler
  .word  n11_BB1_IRQHandler
  .word  n12_DMA_IRQHandler
  .word  n13_TMR1_IRQHandler
  .word  n14_SPI0_IRQHandler
  .word  n15_SPI1_IRQHandler
  .word  n16_URT0_IRQHandler
  .word  n17_URT1_IRQHandler
  .word  n18_I2C_IRQHandler
  .word  n19_DMA_IRQHandler
  .word  n20_BB0_IRQHandler
  .word  n21_BB1_IRQHandler
  .word  n22_TMR2_IRQHandler
  .word  n23_SPI1_IRQHandler
  .word  n24_EXINT_IRQHandler
  .word  n25_GPIO_IRQHandler
  .word  n26_I2C_IRQHandler
  .word  n27_URT0_IRQHandler
  .word  n28_URT1_IRQHandler


  .weak      NMI_Handler
  .thumb_set NMI_Handler,Default_Handler
  
  .weak      HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler
  
  .weak      MemManage_Handler
  .thumb_set MemManage_Handler,Default_Handler
  
  .weak      BusFault_Handler
  .thumb_set BusFault_Handler,Default_Handler

  .weak      UsageFault_Handler
  .thumb_set UsageFault_Handler,Default_Handler

  .weak      SVC_Handler
  .thumb_set SVC_Handler,Default_Handler

  .weak      DebugMon_Handler
  .thumb_set DebugMon_Handler,Default_Handler

  .weak      PendSV_Handler
  .thumb_set PendSV_Handler,Default_Handler

  .weak      SysTick_Handler
  .thumb_set SysTick_Handler,Default_Handler              


   .weak      n00_BB0_IRQHandler_ERR                   
   .thumb_set n00_BB0_IRQHandler_ERR,Default_Handler      
   
   .weak      n01_BB1_IRQHandler_LLE                   
   .thumb_set n01_BB1_IRQHandler_LLE,Default_Handler      
   
   .weak      n02_RTC_M0_IRQHandler                   
   .thumb_set n02_RTC_M0_IRQHandler,Default_Handler      
   
   .weak      n03_RTC_M1_IRQHandler                   
   .thumb_set n03_RTC_M1_IRQHandler,Default_Handler      
   
   .weak      n04_TMR0_IRQHandler                   
   .thumb_set n04_TMR0_IRQHandler,Default_Handler      
   
   .weak      n05_TMR1_IRQHandler                   
   .thumb_set n05_TMR1_IRQHandler,Default_Handler      
   
   .weak      n06_TMR2_IRQHandler                   
   .thumb_set n06_TMR2_IRQHandler,Default_Handler      
   
   .weak      n07_EWK_IRQHandler                   
   .thumb_set n07_EWK_IRQHandler,Default_Handler      
   
   .weak      n08_EXINT_IRQHandler                   
   .thumb_set n08_EXINT_IRQHandler,Default_Handler      
   
   .weak      n09_GPIO_IRQHandler                   
   .thumb_set n09_GPIO_IRQHandler,Default_Handler      

   .weak      n10_BB0_IRQHandler                   
   .thumb_set n10_BB0_IRQHandler,Default_Handler      

   .weak      n11_BB1_IRQHandler                   
   .thumb_set n11_BB1_IRQHandler,Default_Handler      

   .weak      n12_DMA_IRQHandler                   
   .thumb_set n12_DMA_IRQHandler,Default_Handler      

   .weak      n13_TMR1_IRQHandler                   
   .thumb_set n13_TMR1_IRQHandler,Default_Handler      

   .weak      n14_SPI0_IRQHandler                   
   .thumb_set n14_SPI0_IRQHandler,Default_Handler      

   .weak      n15_SPI1_IRQHandler                   
   .thumb_set n15_SPI1_IRQHandler,Default_Handler      

   .weak      n16_URT0_IRQHandler                   
   .thumb_set n16_URT0_IRQHandler,Default_Handler      

   .weak      n17_URT1_IRQHandler                   
   .thumb_set n17_URT1_IRQHandler,Default_Handler      

   .weak      n18_I2C_IRQHandler                   
   .thumb_set n18_I2C_IRQHandler,Default_Handler      

   .weak      n19_DMA_IRQHandler      
   .thumb_set n19_DMA_IRQHandler,Default_Handler

   .weak      n20_BB0_IRQHandler      
   .thumb_set n20_BB0_IRQHandler,Default_Handler

   .weak      n21_BB1_IRQHandler      
   .thumb_set n21_BB1_IRQHandler,Default_Handler

   .weak      n22_TMR2_IRQHandler      
   .thumb_set n22_TMR2_IRQHandler,Default_Handler

   .weak      n23_SPI1_IRQHandler      
   .thumb_set n23_SPI1_IRQHandler,Default_Handler

   .weak      n24_EXINT_IRQHandler      
   .thumb_set n24_EXINT_IRQHandler,Default_Handler

   .weak      n25_GPIO_IRQHandler      
   .thumb_set n25_GPIO_IRQHandler,Default_Handler

   .weak      n26_I2C_IRQHandler      
   .thumb_set n26_I2C_IRQHandler,Default_Handler

   .weak      n27_URT0_IRQHandler      
   .thumb_set n27_URT0_IRQHandler,Default_Handler

   .weak      n28_URT1_IRQHandler      
   .thumb_set n28_URT1_IRQHandler,Default_Handler
