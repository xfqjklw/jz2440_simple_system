#define OPENJTAG_DEBUG 1

.extern     main
.text 
.global _start 
_start:

    b   Reset

ldr pc, _undefined_instruction
ldr pc, _software_interrupt
ldr pc, _prefetch_abort
ldr pc, _data_abort
ldr pc, _not_used
ldr pc, _irq
ldr pc, _fiq
	
Reset:                  
    ldr sp, =4096                       
    bl  disable_watch_dog
    bl  clock_init
    bl  memsetup

	#ifdef OPENJTAG_DEBUG
  	bl copy_isr_talbe_to_sram
	#else
	/*if you want to debug on sdram,you should not copy code*/
    //bl  nand_init
    //bl  copy_nand_to_sdram
	bl copy_steppingstone_to_sdram
	#endif
	
    msr cpsr_c, #0xd2        	@enter irq mode 
	ldr sp, =0x33F00000         @set irq sp
	
	msr cpsr_c, #0x13           @enter supervisor mode and enable irq fiq interrupt
	
    ldr pc, =on_sdram
    
on_sdram:     
    ldr sp, =0x34000000         @set supervisor mode sp on sdram
    ldr lr, =halt_loop        
    ldr pc, =main               

halt_loop:
    b   halt_loop

_undefined_instruction: .word undefined_instruction
_software_interrupt: .word software_interrupt
_prefetch_abort: .word prefetch_abort
_data_abort:  .word data_abort
_not_used:  .word not_used
_irq:   .word irq
_fiq:   .word fiq

irq:
    sub lr, lr, #4                  
    stmdb   sp!,    { r0-r12,lr }   
                                            
    ldr lr, =int_return             
    ldr pc, =EINT_Handle            
	
int_return:
    ldmia   sp!,    { r0-r12,pc }^ 

undefined_instruction:

software_interrupt:

prefetch_abort:

data_abort:

not_used:

fiq:

