library ieee ;
use ieee.std_logic_1164.all ;
use ieee.std_logic_arith.all ;
use ieee.std_logic_unsigned.all ;

entity FreeRun is
	GENERIC (	-- Initial FR Value on Reset, comment out all but 1 of the following
--			FR_CNT      : STD_LOGIC_VECTOR := X"000"	-- 00
			FR_CNT      : STD_LOGIC_VECTOR := X"02F"	-- = 47; default for SC2
--			FR_CNT      : STD_LOGIC_VECTOR := X"002"	-- = 3; for testing
			);

	-- {{ALTERA_IO_BEGIN}} DO NOT REMOVE THIS LINE!
	PORT
	(
		Clk25M 	: IN STD_LOGIC;
		Reset 	: IN STD_LOGIC;
		Enable 	: IN STD_LOGIC;
		CmdData : IN STD_LOGIC_VECTOR(31 downto 0);
		FR_Load : IN STD_LOGIC;     -- active high
		isAddr_Zero : IN STD_LOGIC; -- active low input
		DV_FreeRun 	: OUT STD_LOGIC	-- active low output, 40 ns.
	);
	-- {{ALTERA_IO_END}} DO NOT REMOVE THIS LINE!
end FreeRun;

architecture V4c of FreeRun is

	signal FRLoadReg	: STD_LOGIC_VECTOR(11 DOWNTO 0);	-- control input of counter.
	signal FRCntr		: STD_LOGIC_VECTOR(11 DOWNTO 0); 
	
BEGIN

dwncnt: process (Reset, Clk25M)
begin
	if (Reset = '0' )then 
		FRLoadReg 	<= FR_CNT;
		FRCntr 		<= X"000";
		DV_FreeRun 	<= '1';

	elsif (rising_edge(Clk25M)) then
--	elsif (falling_edge(Clk25M)) then
	 
		if (FR_Load = '1') then 
			FRLoadReg <= CmdData(11 downto 0);
		else
			FRLoadReg <= FRLoadReg;
		end if;
		
		if (Enable = '1' AND isAddr_Zero = '0') then
			if (FRCntr = X"000" ) then 
				FRCntr <= FRLoadReg; 
			else 
				FRCntr <= FRCntr - 1; 
			end if; 
		end if; 

		if (FRCntr = X"000" AND Enable = '1' AND isAddr_Zero = '0') then 
			DV_FreeRun <= '0';
		else  
			DV_FreeRun <= '1';
		end if ;
			
	end if;
end process dwncnt;

end V4c;
