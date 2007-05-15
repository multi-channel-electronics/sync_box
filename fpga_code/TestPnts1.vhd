-- WARNING: Do NOT edit the input and output ports in this file in a text
-- editor if you plan to continue editing the block that represents it in
-- the Block Editor! File corruption is VERY likely to occur.

-- Copyright (C) 1991-2005 Altera Corporation
-- Your use of Altera Corporation's design tools, logic functions 

LIBRARY ieee;
USE ieee.std_logic_1164.all;


-- TestPnts1 will handle various test+diagnotic outputs to TESTHRD1.
--  Entity Declaration

ENTITY TestPnts1 IS
	-- {{ALTERA_IO_BEGIN}} DO NOT REMOVE THIS LINE!
	PORT
	(
		Clk25M 		: IN STD_LOGIC;
		isAddr_Zero : IN STD_LOGIC;
		DV_RTS : IN STD_LOGIC;
		DV_FreeRun : IN STD_LOGIC;
		DV_Error : IN STD_LOGIC;
		dv_buf_o    : IN std_logic;
		xxdv_o    : IN std_logic;
		TestOut1 : OUT STD_LOGIC_VECTOR(8 downto 1)
	);
	-- {{ALTERA_IO_END}} DO NOT REMOVE THIS LINE!
	
END TestPnts1;


--  Architecture Body

ARCHITECTURE T1a OF TestPnts1 IS

	
BEGIN

TestOut1(1) <= Clk25M;
TestOut1(2) <= xxdv_o;
TestOut1(3) <= dv_buf_o;
TestOut1(4) <= dv_buf_o;
TestOut1(5) <= DV_Error;
TestOut1(6) <= DV_FreeRun;
TestOut1(7) <= DV_RTS;
TestOut1(8) <= isAddr_Zero;

END T1a;
