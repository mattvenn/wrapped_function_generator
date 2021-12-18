# Function generator

Example project that uses the shared RAM as data for an arbitrary function generator.
The output is an 8bit port to a DAC.

In the function_generator directory:

* Run make to run the unit tests.
* Run make show_generator to see the waveform.

## Caravel Bus interface

Responds to reads and writes at the given BASE_ADDRESS (defaults to 0x3000_000)

    DATA PARTITION  NAME            DESCRIPTION
    15:0            period          clock cycles between putting next data on the output
    23:16           ram_end_addr    where to start reading the data in the shared RAM
    24              run             if high, set the design running

## RAMBus interface

* At init, read the first word from RAM.
* Starts reading a 32 bit word at address 0 from the shared RAM.
* Every period cycles, update the DAC output.
* After the last 8 bit word is put on the DAC output, start a new wishbone request for the next 32 bits of data.

# Shared RAM

For more information about the shared RAM, [see this doc](https://docs.google.com/document/d/1wLjU6hkAoYvSWNBAyTj8HmIV70eJWU3apa9_OEpsd3Y/edit#)

# Dependencies

* cocotb
* cocotb bus extension for wishbone
