-- WARNING: Do NOT edit the input and output ports in this file in a text
-- editor if you plan to continue editing the block that represents it in
-- the Block Editor! File corruption is VERY likely to occur.

-- Copyright (C) 1991-2005 Altera Corporation
-- Your use of Altera Corporation's design tools, logic functions 
-- and other software and tools, and its AMPP partner logic 
-- functions, and any output files any of the foregoing 
-- (including device programming or simulation files), and any 
-- associated documentation or information are expressly subject 
-- to the terms and conditions of the Altera Program License 
-- Subscription Agreement, Altera MegaCore Function License 
-- Agreement, or other applicable license agreement, including, 
-- without limitation, that your use is for the sole purpose of 
-- programming logic devices manufactured by Altera and sold by 
-- Altera or its authorized distributors.  Please refer to the 
-- applicable agreement for further details.


-- Generated by Quartus II Version 5.1 (Build Build 176 10/26/2005)
-- Created on Thu Mar 09 15:19:47 2006

LIBRARY ieee;
USE ieee.std_logic_1164.all;


--  Entity Declaration

ENTITY TestPnts2 IS
	-- {{ALTERA_IO_BEGIN}} DO NOT REMOVE THIS LINE!
	PORT
	(
		Reset 		: IN STD_LOGIC;
		Enable 		: IN STD_LOGIC;
		isAddr_Zero : IN STD_LOGIC;
		DV_FreeRun 	: IN STD_LOGIC;
		FR_Enable 	: IN STD_LOGIC;
		DV_Delayed 	: IN STD_LOGIC;
		DV_Error 	: IN STD_LOGIC;
		--
		DV_Error_o : OUT STD_LOGIC;
		FR_Mode	: OUT STD_LOGIC;	-- <= FR_Enable
		LED2 	: OUT STD_LOGIC;
		LED3 	: OUT STD_LOGIC;
		XX1 	: OUT STD_LOGIC;
		XX2 	: OUT STD_LOGIC;
		XX3 	: OUT STD_LOGIC;
		XX4 	: OUT STD_LOGIC;
		--
		TP_DV_FreeRun 	: OUT STD_LOGIC;
		TP_DV_Delayed 	: OUT STD_LOGIC;
		TP_isAddr_Zero 	: OUT STD_LOGIC
	);
	-- {{ALTERA_IO_END}} DO NOT REMOVE THIS LINE!
	
END TestPnts2;

-- Testit2 handles output to the dedicated test-points & etc..
--  Architecture Body

ARCHITECTURE T2 OF TestPnts2 IS
	
BEGIN

	TP_DV_FreeRun 	<= DV_FreeRun;
	TP_DV_Delayed 	<= DV_Delayed;
	TP_isAddr_Zero 	<= isAddr_Zero;
--
	DV_Error_o <= DV_Error;
	FR_Mode <= FR_Enable;
	LED2 	<= '0';
	LED3 	<= '0';
	XX1 	<= '0';
	XX2 	<= '0';
	XX3 	<= '0';
	XX4 	<= '0';
		
END T2;