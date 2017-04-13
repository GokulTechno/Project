void strobe_150(void);
void strobe_200(void);
void strobe_250(void);
void strobe_300(void);
void strobe_280(void);
void strobe_260(void);

static void even_rotate(void)
{
	gpio_direction_output(137,1); //177~=57 //Rajesh 
	
	strobe_260();
	//-------------- 1 ----------------------------
	gpio_direction_output(133,1);//1A
	gpio_direction_output(132,0);//1B
	gpio_direction_output(131,0);//2A
	gpio_direction_output(130,0);//2B
	
	strobe_260();

	//-------------- 2 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	strobe_260();

	//-------------- 3 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	strobe_260();

	//-------------- 4 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,1);

	strobe_260();

	gpio_direction_output(137,0);

}

static void odd_rotate(void)
{
gpio_direction_output(137,1);

	//-------------- 5 ----------------------------
	strobe_260();
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	strobe_260();

	//-------------- 6 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	strobe_260();

	//-------------- 7 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);

	strobe_260();
	
	//-------------- 8 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);
	
	strobe_260();

gpio_direction_output(137,0);

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
void strobe_260(void)
{

	udelay(65);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(65);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(65);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(65);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);

}
void empty_strobe_260(void)
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
gpio_direction_output(137,1);

	empty_strobe_260();

	//-------------- 1 ----------------------------
	gpio_direction_output(133,1);//1A
	gpio_direction_output(132,0);//1B
	gpio_direction_output(131,0);//2A
	gpio_direction_output(130,0);//2B
	
	empty_strobe_260();

	//-------------- 2 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	empty_strobe_260();

	//-------------- 3 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	empty_strobe_260();

	//-------------- 4 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,1);
	
	empty_strobe_260();
	
	//-------------- 5 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	empty_strobe_260();

	//-------------- 6 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	empty_strobe_260();

	//-------------- 7 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);
	
	empty_strobe_260();
	
	//-------------- 8 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);

gpio_direction_output(137,0);

}



