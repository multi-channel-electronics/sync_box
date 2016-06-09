## Generated SDC file "Synco.out.sdc"

## Copyright (C) 1991-2015 Altera Corporation. All rights reserved.
## Your use of Altera Corporation's design tools, logic functions 
## and other software and tools, and its AMPP partner logic 
## functions, and any output files from any of the foregoing 
## (including device programming or simulation files), and any 
## associated documentation or information are expressly subject 
## to the terms and conditions of the Altera Program License 
## Subscription Agreement, the Altera Quartus Prime License Agreement,
## the Altera MegaCore Function License Agreement, or other 
## applicable license agreement, including, without limitation, 
## that your use is for the sole purpose of programming logic 
## devices manufactured by Altera and sold by Altera or its 
## authorized distributors.  Please refer to the applicable 
## agreement for further details.


## VENDOR  "Altera"
## PROGRAM "Quartus Prime"
## VERSION "Version 15.1.0 Build 185 10/21/2015 SJ Lite Edition"

## DATE    "Tue Jan 12 11:37:13 2016"

##
## DEVICE  "EPM570T144C3"
##


#**************************************************************
# Time Information
#**************************************************************

set_time_format -unit ns -decimal_places 3



#**************************************************************
# Create Clock
#**************************************************************

create_clock -name {Clk100M} -period 10.000 -waveform { 0.000 5.000 } [get_ports {Clk100M}]
create_clock -name {PIO_nWR} -period 10.000 -waveform { 0.000 5.000 } [get_ports {PIO_nWR}]

#create_clock -name {ClkDiv:clkd|Clk25M} -period 40.000 -waveform { 0.000 20.0 } [get_registers {ClkDiv:clkd|Clk25M}]
create_clock -name {ClkDiv:clkd|clk_adj} -period 40.000 -waveform { 0.000 20.0 } [get_registers {ClkDiv:clkd|clk_adj}]
create_clock -name {ClkDiv:clkd|clk25m_internal} -period 40.000 -waveform { 0.000 20.0 } [get_registers {ClkDiv:clkd|clk25m_internal}]

create_generated_clock \
        -name {ClkDiv:clkd|Clk25M} \
        -source [get_ports {Clk100M}] \
        -divide_by 4 \
        [get_registers ClkDiv:clkd|Clk25M]


#**************************************************************
# Create Generated Clock
#**************************************************************



#**************************************************************
# Set Clock Latency
#**************************************************************



#**************************************************************
# Set Clock Uncertainty
#**************************************************************



#**************************************************************
# Set Input Delay
#**************************************************************



#**************************************************************
# Set Output Delay
#**************************************************************



#**************************************************************
# Set Clock Groups
#**************************************************************



#**************************************************************
# Set False Path
#**************************************************************



#**************************************************************
# Set Multicycle Path
#**************************************************************



#**************************************************************
# Set Maximum Delay
#**************************************************************



#**************************************************************
# Set Minimum Delay
#**************************************************************



#**************************************************************
# Set Input Transition
#**************************************************************

