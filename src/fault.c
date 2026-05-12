#include "at32f403a_407_gpio.h"
#include "at32f403a_407_wdt.h"

#include "display.h"
#include "power.h"

static void fault_block(void)
{
    /* wait for power button to be released if it was pressed */
    while (!gpio_input_data_bit_read(GPIOA, GPIO_PINS_1))
    {
        wdt_counter_reload();
    }

    /* wait for power button press */
    while (gpio_input_data_bit_read(GPIOA, GPIO_PINS_1))
    {
        wdt_counter_reload();
    }

    power_set(false);
}

void hard_fault(uint32_t *stack_frame, uint32_t exc)
{
    uint32_t r0   = stack_frame[0];
    uint32_t r1   = stack_frame[1];
    uint32_t r2   = stack_frame[2];
    uint32_t r3   = stack_frame[3];
    uint32_t r12  = stack_frame[4];
    uint32_t lr   = stack_frame[5];
    uint32_t pc   = stack_frame[6];
    uint32_t psr  = stack_frame[7];
    uint32_t hfsr = SCB->HFSR;
    uint32_t cfsr = SCB->CFSR;
    uint32_t mmar = SCB->MMFAR;
    uint32_t bfar = SCB->BFAR;
    uint32_t afsr = SCB->AFSR;

    print_line("!!!Hard Fault detected!!!");

    print_line("Stack frame:");
    print_line("R0 : 0x%08" PRIX32 "  R1 : 0x%08" PRIX32, r0, r1);
    print_line("R2 : 0x%08" PRIX32 "  R3 : 0x%08" PRIX32, r2, r3);
    print_line("R12: 0x%08" PRIX32 "  LR : 0x%08" PRIX32, r12, lr);
    print_line("PC : 0x%08" PRIX32 "  PSR: 0x%08" PRIX32, pc, psr);

    print_line("Fault status:");
    print_line("HFSR: 0x%08" PRIX32 "  CFSR: 0x%08" PRIX32, hfsr, cfsr);
    /* TODO: why do these contain the address of the registers themselves? */
    print_line("MMAR: 0x%08" PRIX32 "  BFAR: 0x%08" PRIX32, mmar, bfar);
    print_line("AFSR: 0x%08" PRIX32, afsr);

    print_line("Other:");
    print_line("EXC_RETURN: 0x%08" PRIX32, exc);

    print_line("Hard fault status:");
    if (hfsr & SCB_HFSR_FORCED_Msk)
    {
        print_line("- Forced Hard fault");
    }
    if (hfsr & SCB_HFSR_VECTTBL_Msk)
    {
        print_line("- Bus fault on vector table read");
    }
    print_line("MemManage fault status:");
    if (cfsr & SCB_CFSR_MMARVALID_Msk)
    {
        print_line("- MMAR holds a valid address");
    }
    else
    {
        print_line("- MMAR holds an invalid address");
    }
    if (cfsr & SCB_CFSR_MLSPERR_Msk)
    {
        print_line("- Floating-point lazy state preservation");
    }
    if (cfsr & SCB_CFSR_MSTKERR_Msk)
    {
        print_line("- Stacking has caused an access violation");
    }
    if (cfsr & SCB_CFSR_MUNSTKERR_Msk)
    {
        print_line("- Unstacking has caused an access violation");
    }
    if (cfsr & SCB_CFSR_DACCVIOL_Msk)
    {
        print_line("- Load or store not permitted");
    }
    if (cfsr & SCB_CFSR_IACCVIOL_Msk)
    {
        print_line("- Instruction fetch not permitted");
    }
    print_line("Bus fault status:");
    if (cfsr & SCB_CFSR_BFARVALID_Msk)
    {
        print_line("- BFAR holds a valid address");
    }
    else
    {
        print_line("- BFAR holds an invalid address");
    }
    if (cfsr & SCB_CFSR_LSPERR_Msk)
    {
        print_line("- Floating-point lazy state preservation");
    }
    if (cfsr & SCB_CFSR_STKERR_Msk)
    {
        print_line("- Stacking has caused a Bus fault");
    }
    if (cfsr & SCB_CFSR_UNSTKERR_Msk)
    {
        print_line("- Unstacking has caused a Bus fault");
    }
    if (cfsr & SCB_CFSR_IMPRECISERR_Msk)
    {
        print_line("- Data bus error, return address unrelated");
    }
    if (cfsr & SCB_CFSR_PRECISERR_Msk)
    {
        print_line("- Data bus error, return address points to cause");
    }
    if (cfsr & SCB_CFSR_IBUSERR_Msk)
    {
        print_line("- Instruction bus error");
    }
    print_line("Usage fault status:");
    if (cfsr & SCB_CFSR_DIVBYZERO_Msk)
    {
        print_line("- SDIV or UDIV instruction with divisor of 0");
    }
    if (cfsr & SCB_CFSR_UNALIGNED_Msk)
    {
        print_line("- Unaligned memory access");
    }
    if (cfsr & SCB_CFSR_NOCP_Msk)
    {
        print_line("- Attempted to access a coprocessor");
    }
    if (cfsr & SCB_CFSR_INVPC_Msk)
    {
        print_line("- Illegal attempt to load EXC_RETURN to PC");
    }
    if (cfsr & SCB_CFSR_INVSTATE_Msk)
    {
        print_line("- Instruction makes illegal use of EPSR");
    }
    if (cfsr & SCB_CFSR_UNDEFINSTR_Msk)
    {
        print_line("- Undefined instruction");
    }

    fault_block();
}

void HardFault_Handler(void)
{
    __asm volatile
    (
        "tst   lr, #0b0100;"
        "ite   eq;         "
        "mrseq r0, msp;    "
        "mrsne r0, psp;    "
        "mov   r1, lr;     "
        "b     hard_fault; "
    );
}

void NMI_Handler(void)        { print_line(__FUNCTION__); fault_block(); }
void MemManage_Handler(void)  { print_line(__FUNCTION__); fault_block(); }
void BusFault_Handler(void)   { print_line(__FUNCTION__); fault_block(); }
void UsageFault_Handler(void) { print_line(__FUNCTION__); fault_block(); }
