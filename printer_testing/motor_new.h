void strobe_120(void);
void strobe_130(void);
void strobe_150(void);
void strobe_200(void);
void strobe_250(void);
void strobe_300(void);
void strobe_280(void);
void strobe_260(void);
void empty_strobe_280(void);

static void even_rotate(void)
{
		strobe_150();
		
		//-------------- 1 ----------------------------
        gpio_direction_output(133,1);//1A
        gpio_direction_output(132,0);//1B
        
        strobe_120();
        
        gpio_direction_output(131,0);//2A
        gpio_direction_output(130,0);//2B

        strobe_120();

        //-------------- 2 ----------------------------
        gpio_direction_output(133,1);
        gpio_direction_output(132,0);
        
        strobe_120();
        
        gpio_direction_output(131,0);
        gpio_direction_output(130,1);

        strobe_120();

        //-------------- 3 ----------------------------
        gpio_direction_output(133,0);
        gpio_direction_output(132,0);
        
        strobe_120();
        
        gpio_direction_output(131,0);
        gpio_direction_output(130,1);

        strobe_120();

        //-------------- 4 ----------------------------
        gpio_direction_output(133,0);
        gpio_direction_output(132,0);
        
        strobe_120();
        
        gpio_direction_output(131,1);
        gpio_direction_output(130,1);
        
}

static void odd_rotate(void)
{
	
		strobe_150();
		
         //-------------- 5 ----------------------------
        gpio_direction_output(133,0);
        gpio_direction_output(132,0);
        
        strobe_120();
        
        gpio_direction_output(131,1);
        gpio_direction_output(130,0);

        strobe_120();

        //-------------- 6 ----------------------------
        gpio_direction_output(133,0);
        gpio_direction_output(132,1);
        
        strobe_120();
        
        gpio_direction_output(131,1);
        gpio_direction_output(130,0);

        strobe_120();

        //-------------- 7 ----------------------------
        gpio_direction_output(133,0);
        gpio_direction_output(132,1);
        
        strobe_120();
        
        gpio_direction_output(131,0);
        gpio_direction_output(130,0);

        strobe_120();

        //-------------- 8 ----------------------------
        gpio_direction_output(133,1);
        gpio_direction_output(132,1);
        
        strobe_120();
        
        gpio_direction_output(131,0);
        gpio_direction_output(130,0);
        
}

void strobe_120(void)
{
	udelay(30);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(30);
	gpio_direction_output(134,1);
        gpio_direction_output(135,1);
        gpio_direction_output(136,1);
	udelay(30);
	gpio_direction_output(134,1);
        gpio_direction_output(135,1);
        gpio_direction_output(136,1);
	udelay(30);
	gpio_direction_output(134,0);
        gpio_direction_output(135,0);
        gpio_direction_output(136,0);
}
void strobe_130(void)
{
	udelay(30);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(20);
	gpio_direction_output(134,1);
        gpio_direction_output(135,1);
        gpio_direction_output(136,1);
	udelay(20);
	gpio_direction_output(134,1);
        gpio_direction_output(135,1);
        gpio_direction_output(136,1);
	udelay(20);
	gpio_direction_output(134,0);
        gpio_direction_output(135,0);
        gpio_direction_output(136,0);
}

void strobe_150(void)
{

	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,0);
	gpio_direction_output(135,0);
	gpio_direction_output(136,0);

}

void strobe_200(void)
{
	udelay(50);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,0);
	gpio_direction_output(135,0);
	gpio_direction_output(136,0);

}

void strobe_250(void)
{
	udelay(100);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,0);
	gpio_direction_output(135,0);
	gpio_direction_output(136,0);

}

void strobe_300(void)
{
	udelay(150);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(50);
	gpio_direction_output(134,0);
	gpio_direction_output(135,0);
	gpio_direction_output(136,0);

}
void strobe_280(void)
{

	udelay(70);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	
	udelay(70);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	
	udelay(70);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	
	udelay(70);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	
}
void empty_strobe_280(void)
{
	
udelay(70);
gpio_direction_output(134,0);
gpio_direction_output(135,0);
gpio_direction_output(136,0);
udelay(70);
gpio_direction_output(134,0);
gpio_direction_output(135,0);
gpio_direction_output(136,0);
udelay(70);
gpio_direction_output(134,0);
gpio_direction_output(135,0);
gpio_direction_output(136,0);
udelay(70);
gpio_direction_output(134,0);
gpio_direction_output(135,0);
gpio_direction_output(136,0);

}

static void empty_rotate(void)
{
	gpio_direction_output(137,1);// --------------Driver IC power -----------   
	
	//-------------- 8 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	empty_strobe_280();

	//-------------- 7 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	empty_strobe_280();
	//udelay(150);

	//-------------- 6 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	empty_strobe_280();
	//udelay(250);

	//-------------- 5 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(150);

	//-------------- 4 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(200);

	//-------------- 3 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(150);

	//-------------- 2 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(150);

	//-------------- 1 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(250);
	
	gpio_direction_output(137,0);// --------------Driver IC power -----------   
}

static void empty_rrotate(void)
{
	gpio_direction_output(137,1);// --------------Driver IC power -----------   
	
	//-------------- 1 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(250);
	
	//-------------- 2 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(150);
	
	//-------------- 3 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(150);
	
	//-------------- 4 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(200);
	
	//-------------- 5 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	empty_strobe_280();
	//udelay(150);
	
	//-------------- 6 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	empty_strobe_280();
	//udelay(250);
	
	//-------------- 7 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	empty_strobe_280();
	//udelay(150);
	
	//-------------- 8 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	empty_strobe_280();

	gpio_direction_output(137,0);// --------------Driver IC power -----------   
}

