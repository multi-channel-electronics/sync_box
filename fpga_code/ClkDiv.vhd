
LIBRARY ieee ;
USE ieee.std_logic_1164.ALL ;
USE ieee.std_logic_arith.ALL ;
USE ieee.std_logic_unsigned.ALL ;

ENTITY ClkDiv IS
  PORT ( 
--		Reset 	: IN STD_LOGIC;
		Clk100M	: IN STD_LOGIC;
		CLK_EXT	: IN STD_LOGIC;	-- optional external clock input, not used.
		------
		Clk50M	: OUT STD_LOGIC;
		Clk25M	: OUT STD_LOGIC;
		Clk5M	: OUT STD_LOGIC
		);
  END ClkDiv;

ARCHITECTURE rtl OF ClkDiv IS

SIGNAL clk_cntr1	:STD_LOGIC_VECTOR(1 DOWNTO 0):= "00";
SIGNAL clk_cntr2	:STD_LOGIC_VECTOR(4 DOWNTO 0):= "00000";

BEGIN 

clk_div : PROCESS (Clk100M)
BEGIN     
-- 	IF(Reset = '0') then 
--		clk_cntr1 <= "01"; 
--    ELSIF (rising_edge(Clk100M)) THEN
    IF (rising_edge(Clk100M)) THEN
		clk_cntr1 <= clk_cntr1 + 1 ;
		Clk50M <= clk_cntr1(0);
		Clk25M <= clk_cntr1(1);
	END IF ;
END PROCESS clk_div;


-- create 5MHz clock 
divit :process(Clk100M) 
begin 
--	if(Reset = '0') then 
--		Clk5M <= '0'; 
--		clk_cntr2 <= (others => '0'); 
--	elsif(rising_edge(Clk100M)) then 
	if(rising_edge(Clk100M)) then 
		if(clk_cntr2 >= 19) then 
			clk_cntr2 <= (others => '0'); 
		else 
			clk_cntr2 <= clk_cntr2 + 1; 
		end if; 
		if(clk_cntr2 >= 10) then
			Clk5M <= '1';
		else
			Clk5M <= '0';
		end if;		
	end if; 
end process divit; 

END rtl;
  

  
  
  
  
  







