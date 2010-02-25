-- Project: Sync_box 
-- ClkDiv.vhd
-- Description: This block takes an 8-bit divisor passed from a 
-- command interface driven by on-board 8051 microcontroller. 
--
LIBRARY ieee ;
USE ieee.std_logic_1164.ALL ;
USE ieee.std_logic_arith.ALL ;
USE ieee.std_logic_unsigned.ALL ;

ENTITY ClkDiv IS
	-- {{ALTERA_IO_BEGIN}} DO NOT REMOVE THIS LINE!
  PORT ( 
		Reset 	: IN STD_LOGIC;
		Clk100M	: IN STD_LOGIC;
		CLK_EXT	: IN STD_LOGIC;	-- optional external clock input, not used.
		CmdData : IN STD_LOGIC_VECTOR(31 downto 0);
		clk_adj_div_load: IN STD_LOGIC; -- 50MHz clock divisor for clk_adj
		------
		Clk50M	: OUT STD_LOGIC;
		Clk25M	: OUT STD_LOGIC;
		clk_adj	: OUT STD_LOGIC	-- a divide-down version of clock
		);
	-- {{ALTERA_IO_END}} DO NOT REMOVE THIS LINE!
  END ClkDiv;

ARCHITECTURE rtl OF ClkDiv IS

SIGNAL clk_cntr1	:STD_LOGIC_VECTOR(1 DOWNTO 0):= "00";
-- clk_cntr2 and clk_100MHz_div are intentionally one bit more than the divisor to 
-- enforce a '0' in MSB and avoid sign confusion during comparisons.
SIGNAL clk_cntr2	:STD_LOGIC_VECTOR(8 DOWNTO 0):= "000000000";  
SIGNAL clk_100MHz_div : std_logic_vector(9 downto 0) := "0000010100"; -- note that 100MHz is divided down to generate other clocks
signal clk_adj_div    : std_logic_vector(8 downto 0);
signal clk25m_internal : std_logic; -- a copy of 25MHz clock for internal use

BEGIN 

--------------------------------------
-- generating 50MHz and 25MHz clocks
clk_div : PROCESS (Clk100M)
BEGIN     
-- 	IF(Reset = '0') then 
--		clk_cntr1 <= "01"; 
--    ELSIF (rising_edge(Clk100M)) THEN
    IF (rising_edge(Clk100M)) THEN
		clk_cntr1 <= clk_cntr1 + 1 ;
		Clk50M <= clk_cntr1(0);
		clk25m_internal <= clk_cntr1(1);
		Clk25M <= clk_cntr1(1);
	END IF ;
END PROCESS clk_div;

-----------------------------------------------------------
-- capturing adjustable clock divisor from PIO interface
clk_100MHz_div <= clk_adj_div & '0';


clk_div_reg: process (Reset, clk25m_internal)
begin
	if (Reset = '0' )then 
		clk_adj_div 	<= "000001010";
	elsif (rising_edge(clk25m_internal)) then
		if (clk_adj_div_load = '1') then 
			clk_adj_div (7 downto 0) <= CmdData(7 downto 0);
		else
			clk_adj_div <= clk_adj_div;
		end if;		
	end if;
end process clk_div_reg;

--------------------------------------
-- create frequency-adjustable clock 
divit :process(Clk100M) 
begin 
--	if(Reset = '0') then 
--		Clk5M <= '0'; 
--		clk_cntr2 <= (others => '0'); 
--	elsif(rising_edge(Clk100M)) then 
	if(rising_edge(Clk100M)) then 
		if(clk_cntr2 >= (clk_100MHz_div-1) ) then 
			clk_cntr2 <= (others => '0'); 
		else 
			clk_cntr2 <= clk_cntr2 + 1; 
		end if; 
		if(clk_cntr2 >= clk_adj_div) then
			clk_adj <= '1';
		else
			clk_adj <= '0';
		end if;		
	end if; 
end process divit; 



END rtl;
  

  
  
  
  
  







