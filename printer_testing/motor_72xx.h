void strobe_150(void);
void strobe_200(void);
void strobe_250(void);
void strobe_300(void);
void strobe_280(void);
void strobe_260(void);

static void even_rotate(void)
{
//	gpio_direction_output(57,0); //177~=57 //Rajesh 
//	udelay(1000);
//	gpio_direction_output(57,1); //177~=57 //Rajesh 
//	udelay(500);

	
	//-------------- 1 ----------------------------
        gpio_direction_output(133,1);//1A
        gpio_direction_output(132,0);//1B
	gpio_direction_output(131,0);//2A
	gpio_direction_output(130,0);//2B
	
	strobe_280();

	//-------------- 2 ----------------------------
        gpio_direction_output(133,1);
        gpio_direction_output(132,0);
        gpio_direction_output(131,0);
        gpio_direction_output(130,1);

	strobe_280();

	//-------------- 3 ----------------------------
        gpio_direction_output(133,0);
        gpio_direction_output(132,0);
        gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	strobe_280();

	//-------------- 4 ----------------------------
        gpio_direction_output(133,0);
	gpio_direction_output(132,0);
        gpio_direction_output(131,1);
	gpio_direction_output(130,1);

	strobe_280();

//	gpio_direction_output(57,0);

}

static void odd_rotate(void)
{
//gpio_direction_output(57,1);
//	udelay(1000);

	//-------------- 5 ----------------------------
	strobe_280();
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	strobe_280();

	//-------------- 6 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	strobe_280();

	//-------------- 7 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);

	strobe_280();
	
	//-------------- 8 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);
	
	strobe_280();

//gpio_direction_output(57,0);

}

void strobe_150(void)
{

	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,0);
	gpio_direction_output(44,0);
	gpio_direction_output(68,0);

}

void strobe_200(void)
{
	udelay(50);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,0);
	gpio_direction_output(44,0);
	gpio_direction_output(68,0);

}

void strobe_250(void)
{

	udelay(100);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,0);
	gpio_direction_output(44,0);
	gpio_direction_output(68,0);

}

void strobe_300(void)
{
	udelay(150);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(50);
	gpio_direction_output(45,0);
	gpio_direction_output(44,0);
	gpio_direction_output(68,0);

}
void strobe_280(void)
{

	udelay(100);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(100);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(100);
	gpio_direction_output(134,1);
	gpio_direction_output(135,1);
	gpio_direction_output(136,1);
	udelay(100);
	gpio_direction_output(134,0);
	gpio_direction_output(135,0);
	gpio_direction_output(136,0);

}
void strobe_260(void)
{

	udelay(65);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(65);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(65);
	gpio_direction_output(45,1);
	gpio_direction_output(44,1);
	gpio_direction_output(68,1);
	udelay(65);
	gpio_direction_output(45,0);
	gpio_direction_output(44,0);
	gpio_direction_output(68,0);

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
//gpio_direction_output(57,1);

	//-------------- 1 ----------------------------
	gpio_direction_output(133,1);//1A
	gpio_direction_output(132,0);//1B
	gpio_direction_output(131,0);//2A
	gpio_direction_output(130,0);//2B
	
	udelay(400);

	//-------------- 2 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	udelay(400);

	//-------------- 3 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,0);
	gpio_direction_output(130,1);

	udelay(400);

	//-------------- 4 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,1);
	
	udelay(400);
	
	//-------------- 5 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,0);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	udelay(400);

	//-------------- 6 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,1);
	gpio_direction_output(130,0);

	udelay(400);

	//-------------- 7 ----------------------------
	gpio_direction_output(133,0);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);
	
	udelay(400);
	
	//-------------- 8 ----------------------------
	gpio_direction_output(133,1);
	gpio_direction_output(132,1);
	gpio_direction_output(131,0);
	gpio_direction_output(130,0);

	udelay(400);
//gpio_direction_output(57,0);

}



