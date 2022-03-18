/*
 * SPDX-FileCopyrightText: 2020 Efabless Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * SPDX-License-Identifier: Apache-2.0
 */

//#include "verilog/dv/caravel/defs.h"

#include <defs.h>
#include <stub.c>


#define PROJECT_ID 0

#define REG_CONFIG          		(*(volatile uint32_t*)0x30000000)
#define SRAM_LATENCY_CSR_BASE_ADDR	0x300FF800
#define SRAM_BASE_ADDR      		0x300FFC00
#define OPENRAM(addr)       		(*(volatile uint32_t*)(SRAM_BASE_ADDR + (addr & 0x3fc)))
#define OPENRAM_LATENCY			(*(volatile uint32_t*)(SRAM_LATENCY_CSR_BASE_ADDR))
#define SRAM_WRITE_PORT 		31  // last bit of the 1st bank logic analyser. If set to 0, Caravel can write to shared RAM

void config_generator(uint16_t period, uint8_t end_addr, bool run)
{
    REG_CONFIG = (run << 24) + (end_addr << 16) + period;
}

void write_to_ram(uint8_t addr, uint32_t data)
{
    OPENRAM(addr << 2) = data;
}

uint32_t read_from_ram(uint8_t addr)
{
    return OPENRAM(addr << 2);
}

void set_ram_latencies(uint8_t mgmt_pre, uint8_t mgmt_read, uint8_t uprj_pre, uint8_t uprj_read)
{
    OPENRAM_LATENCY = 	(mgmt_pre << 24) 
	    		| (mgmt_read << 16) 
			| (uprj_pre << 8) 
			| uprj_read;   
}

uint32_t read_ram_latencies()
{
    return OPENRAM_LATENCY;
}

void main()
{
	/* 
	IO Control Registers
	| DM     | VTRIP | SLOW  | AN_POL | AN_SEL | AN_EN | MOD_SEL | INP_DIS | HOLDH | OEB_N | MGMT_EN |
	| 3-bits | 1-bit | 1-bit | 1-bit  | 1-bit  | 1-bit | 1-bit   | 1-bit   | 1-bit | 1-bit | 1-bit   |

	Output: 0000_0110_0000_1110  (0x1808) = GPIO_MODE_USER_STD_OUTPUT
	| DM     | VTRIP | SLOW  | AN_POL | AN_SEL | AN_EN | MOD_SEL | INP_DIS | HOLDH | OEB_N | MGMT_EN |
	| 110    | 0     | 0     | 0      | 0      | 0     | 0       | 1       | 0     | 0     | 0       |
	
	 
	Input: 0000_0001_0000_1111 (0x0402) = GPIO_MODE_USER_STD_INPUT_NOPULL
	| DM     | VTRIP | SLOW  | AN_POL | AN_SEL | AN_EN | MOD_SEL | INP_DIS | HOLDH | OEB_N | MGMT_EN |
	| 001    | 0     | 0     | 0      | 0      | 0     | 0       | 0       | 0     | 1     | 0       |

	*/

    // 8 outputs for DAC
	reg_mprj_io_8  =   GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_9  =   GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_10 =   GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_11 =   GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_12 =   GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_13 =   GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_14 =   GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_15 =   GPIO_MODE_USER_STD_OUTPUT;

    // debug
    reg_mprj_io_16 =   GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_17 =   GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_18 =   GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_19 =   GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_20 =   GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_21 =   GPIO_MODE_USER_STD_OUTPUT;

    /* Apply configuration */
    reg_mprj_xfer = 1;
    while (reg_mprj_xfer == 1);

    reg_la0_iena = 0xFFFFFFFF; // input enable off
    reg_la0_oenb = 0xFFFFFFFF; // output enable on

    // enable wishbone
    reg_wb_enable  = 1;

    // allow Caravel to write to the shared RAM
    reg_la0_data &= ~(1 << SRAM_WRITE_PORT); 

    // configure latencies to 3,3,3,3
    set_ram_latencies(3,3,3,3);

    // load the function data into sram
    uint8_t i = 0;
    for(i = 0; i < 4; i ++)
    {
        write_to_ram(i, 4*i + ((4*i+1) << 8) + ((4*i+2) << 16) + ((4*i+3) << 24));
    }

    // read 1 address back just to check
    read_from_ram(0);

    // activate the project by setting the 1st bit of 1st bank of LA - depends on the project ID
    reg_la0_data |= (1 << PROJECT_ID);

    // configure function generator: 10 clock cycles per sample, 64 * 4 samples, start run
    config_generator(10, 4, 1);
}
