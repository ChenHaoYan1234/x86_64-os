#pragma once

void divide_error();
void debug();
void nmi();
void int3();
void overflow();
void bounds();
void undefined_opcode();
void dev_not_available();
void double_fault();
void coprocessor_segment_overrun();
void invalid_tss();
void segment_not_present();
void stack_segment_fault();
void general_protection();
void page_fault();
// 15 Intel reserved. Do not use.
void x87_fpu_error();
void alignment_check();
void machine_check();
void simd_fpu_exception();
void virtualization_exception();

void do_divide_error(unsigned long rsp, unsigned long);
void do_debug(unsigned long rsp, unsigned long);
void do_nmi(unsigned long rsp, unsigned long);
void do_int3(unsigned long rsp, unsigned long);
void do_overflow(unsigned long rsp, unsigned long);
void do_bounds(unsigned long rsp, unsigned long);
void do_undefined_opcode(unsigned long rsp, unsigned long);
void do_dev_not_available(unsigned long rsp, unsigned long);
void do_double_fault(unsigned long rsp, unsigned long error_code);
void do_coprocessor_segment_overrun(unsigned long rsp, unsigned long);
void do_invalid_tss(unsigned long rsp, unsigned long error_code);
void do_segment_not_present(unsigned long rsp, unsigned long error_code);
void do_stack_segment_fault(unsigned long rsp, unsigned long error_code);
void do_general_protection(unsigned long rsp, unsigned long error_code);
void do_page_fault(unsigned long rsp, unsigned long error_code);
// 15 Intel reserved. Do not use.
void do_x87_fpu_error(unsigned long rsp, unsigned long);
void do_alignment_check(unsigned long rsp, unsigned long error_code);
void do_machine_check(unsigned long rsp, unsigned long);
void do_simd_fpu_exception(unsigned long rsp, unsigned long);
void do_virtualization_exception(unsigned long rsp, unsigned long);


void sys_vector_init();
