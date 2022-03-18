import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, FallingEdge, ClockCycles, with_timeout


async def until_signal_has_value(clk, sig, value):
    while True:
        await RisingEdge(clk)
        if sig.value == value:
            return

@cocotb.test()
async def test_start(dut):
    clock = Clock(dut.clk, 25, units="ns") # 40M
    cocotb.fork(clock.start())
    
    dut.RSTB <= 0
    dut.power1 <= 0;
    dut.power2 <= 0;
    dut.power3 <= 0;
    dut.power4 <= 0;

    await ClockCycles(dut.clk, 8)
    dut.power1 <= 1;
    await ClockCycles(dut.clk, 8)
    dut.power2 <= 1;
    await ClockCycles(dut.clk, 8)
    dut.power3 <= 1;
    await ClockCycles(dut.clk, 8)
    dut.power4 <= 1;

    await ClockCycles(dut.clk, 80)
    dut.RSTB <= 1

    # wait with a timeout for the project to become active
    await with_timeout(RisingEdge(dut.dbg_active), 2000, 'us')

    # sync with start of DAC
    await RisingEdge(dut.dbg_dac_start)  
    await FallingEdge(dut.dbg_dac_start)

    # firmware sets up these:
    period = 10
    max_addr = 4
    # wait for data to start
    await ClockCycles(dut.clk, period * 3)
    for i in range(period * max_addr * 2):

        # ensure value from DAC is correct
        assert dut.dac == (i % (max_addr * 4))

        # wait period
        await ClockCycles(dut.clk, period)


