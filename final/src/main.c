#include "stm32l476xx.h"
#include <stdio.h>
#include <stdlib.h>

#define SET_REG(REG, SELECT, VAL) {((REG)=((REG)&(~(SELECT))) | (VAL));};
#define TIM_COUNTERMODE_UP 0
#define presc 200

extern void delay_us(int r0);
extern void max7219_init();
extern void max7219_send(int r0, int r1);

// for 1, 5, 10
int count[3] = {-1, 0, 0};
int sev_seg_val[10] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B};
int keypad_value[4][4] = {{1, 2, 3, 10}, {4, 5, 6, 11}, {7, 8, 9, 12}, {14, 0, 15, 13}};
int mode = -1, sub_mode = 1;
int first = 0;
int output = 0, input = 0;
int real_num = 0;
int motor_state[3] = {0, 0, 0};

void GPIO_init()	{
	RCC->AHB2ENR = 0x7;

/* 7-seg */
	// PA5 for 7-seg DATA pin
	SET_REG(GPIOA->MODER, GPIO_MODER_MODE5, GPIO_MODER_MODE5_0);		// output mode
	SET_REG(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED5, GPIO_OSPEEDR_OSPEED5_0);

	// PA6 for 7-seg LOAD pin
	SET_REG(GPIOA->MODER, GPIO_MODER_MODE6, GPIO_MODER_MODE6_0);		// output mode
	SET_REG(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED6, GPIO_OSPEEDR_OSPEED6_0);

	// PA7 for 7-seg CLOCK pin
	SET_REG(GPIOA->MODER, GPIO_MODER_MODE7, GPIO_MODER_MODE7_0);		// output mode
	SET_REG(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED7, GPIO_OSPEEDR_OSPEED7_0);


/* Motor */
	// PB3 for motor_1
	SET_REG(GPIOB->MODER, GPIO_MODER_MODE3, GPIO_MODER_MODE3_1);		// AF¡@mode
	SET_REG(GPIOB->AFR[0], GPIO_AFRL_AFRL3, GPIO_AFRL_AFSEL3_0);

	// PB10 for motor_2
	SET_REG(GPIOB->MODER, GPIO_MODER_MODE10, GPIO_MODER_MODE10_1);		// AF¡@mode
	SET_REG(GPIOB->AFR[1], GPIO_AFRH_AFRH2, GPIO_AFRH_AFSEL10_0);

	// PB11 for motor_3
	SET_REG(GPIOB->MODER, GPIO_MODER_MODE11, GPIO_MODER_MODE11_1);		// AF¡@mode
	SET_REG(GPIOB->AFR[1], GPIO_AFRH_AFRH3, GPIO_AFRH_AFSEL11_0);

/* IR Module */
	// PB4 for IR_transmitter_1
	SET_REG(GPIOB->MODER, GPIO_MODER_MODE4, GPIO_MODER_MODE4_0);		// output mode
	SET_REG(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED4, GPIO_OSPEEDR_OSPEED4_0);

	// PB5 for IR_transmitter_2
	SET_REG(GPIOB->MODER, GPIO_MODER_MODE5, GPIO_MODER_MODE5_0);		// output mode
	SET_REG(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED5, GPIO_OSPEEDR_OSPEED5_0);

	// PB6 for IR_transmitter_3
	SET_REG(GPIOB->MODER, GPIO_MODER_MODE6, GPIO_MODER_MODE6_0);		// output mode
	SET_REG(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED6, GPIO_OSPEEDR_OSPEED6_0);

	// PC10 for IR_receiver_1
	SET_REG(GPIOC->MODER, GPIO_MODER_MODE10, 0);							// input mode
	SET_REG(GPIOC->PUPDR, GPIO_PUPDR_PUPD10, GPIO_PUPDR_PUPD10_1);		// pull down

	// PC11 for IR_receiver_2
	SET_REG(GPIOC->MODER, GPIO_MODER_MODE11, 0);							// input mode
	SET_REG(GPIOC->PUPDR, GPIO_PUPDR_PUPD11, GPIO_PUPDR_PUPD11_1);		// pull down

	// PC12 for IR_receiver_3
	SET_REG(GPIOC->MODER, GPIO_MODER_MODE12, 0);							// input mode
	SET_REG(GPIOC->PUPDR, GPIO_PUPDR_PUPD12, GPIO_PUPDR_PUPD12_1);		// pull down

/* keypad */
	SET_REG(GPIOC->MODER, GPIO_MODER_MODE1, GPIO_MODER_MODE1_0);
	SET_REG(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED1, GPIO_OSPEEDR_OSPEED1_0);

	SET_REG(GPIOC->MODER, GPIO_MODER_MODE2, GPIO_MODER_MODE2_0);
	SET_REG(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED2, GPIO_OSPEEDR_OSPEED2_0);

	SET_REG(GPIOC->MODER, GPIO_MODER_MODE3, GPIO_MODER_MODE3_0);
	SET_REG(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED3, GPIO_OSPEEDR_OSPEED3_0);

	SET_REG(GPIOC->MODER, GPIO_MODER_MODE4, GPIO_MODER_MODE4_0);
	SET_REG(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED1, GPIO_OSPEEDR_OSPEED4_0);

	SET_REG(GPIOC->MODER, GPIO_MODER_MODE5, 0);
	SET_REG(GPIOC->PUPDR, GPIO_PUPDR_PUPD5, GPIO_PUPDR_PUPD5_1);

	SET_REG(GPIOC->MODER, GPIO_MODER_MODE6, 0);
	SET_REG(GPIOC->PUPDR, GPIO_PUPDR_PUPD6, GPIO_PUPDR_PUPD6_1);

	SET_REG(GPIOC->MODER, GPIO_MODER_MODE7, 0);
	SET_REG(GPIOC->PUPDR, GPIO_PUPDR_PUPD7, GPIO_PUPDR_PUPD7_1);

	SET_REG(GPIOC->MODER, GPIO_MODER_MODE8, 0);
	SET_REG(GPIOC->PUPDR, GPIO_PUPDR_PUPD8, GPIO_PUPDR_PUPD8_1);

	//Set PC5,6,7,8 as input  mode	//1111 1111 1111 1100 0000 0011 1111 1111
	//Set PC1,2,3,4 as output mode	//1111 1111 1111 1111 1111 1101 0101 0111
	//								//1111 1111 1111 1100 0000 0001 0101 0111
	//GPIOC->MODER = GPIOC->MODER&0xFFFC0157;
	//set PC5,6,7,8 is Pull-down	//0000 0000 0000 0010 1010 1000 0000 0000
	//set PC1,2,3,4 is Pull-up 		//0000 0000 0000 0000 0000 0001 0101 0100
	//								//0000 0000 0000 0010 1010 1001 0101 0100
	//GPIOC->PUPDR = GPIOC->PUPDR | 0x2A954;
	//Set PC5,6,7,8 as medium speed //0000 0000 0000 0001 0101 0100 0000 0000
	//Set PC1,2,3,4 as medium speed //0000 0000 0000 0000 0000 0001 0101 0100
	//								//0000 0000 0000 0001 0101 0101 0101 0100
	//GPIOC->OSPEEDR = GPIOC->OSPEEDR | 0x15554;
}
/* motor setting start */
void timer_init()	{

	// set up for TIM2
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	SET_REG(TIM2->CR1, TIM_CR1_DIR | TIM_CR1_CMS, TIM_COUNTERMODE_UP);
	TIM2->ARR = (uint32_t)400;				// after 400 * 1/20 ms reload
	TIM2->PSC = (uint32_t)presc;			// each CNT is 1/20 ms
}

void PWM_channel_init()	{

	// motor_1 PWM set up
	TIM2->CCR2 = 16;						// from 12 to 51 to control posedge between 0.5 ~ 2.5 ms
	SET_REG(TIM2->CCER, TIM_CCER_CC2E, TIM_CCER_CC2E);
	SET_REG(TIM2->CCMR1, TIM_CCMR1_CC2S, 0);
	SET_REG(TIM2->CCMR1, TIM_CCMR1_OC2M, TIM_CCMR1_OC2M_1 + TIM_CCMR1_OC2M_2);	// PWM posedge first

	// motor_2 PWM set up
	TIM2->CCR3 = 16;						// from 12 to 51 to control posedge between 0.5 ~ 2.5 ms
	SET_REG(TIM2->CCER, TIM_CCER_CC3E, TIM_CCER_CC3E);
	SET_REG(TIM2->CCMR2, TIM_CCMR2_CC3S, 0);
	SET_REG(TIM2->CCMR2, TIM_CCMR2_OC3M, TIM_CCMR2_OC3M_1 + TIM_CCMR2_OC3M_2);	// PWM posedge first

	// motor_3 PWM set up
	TIM2->CCR4 = 16;						// from 12 to 51 to control posedge between 0.5 ~ 2.5 ms
	SET_REG(TIM2->CCER, TIM_CCER_CC4E, TIM_CCER_CC4E);
	SET_REG(TIM2->CCMR2, TIM_CCMR2_CC4S, 0);
	SET_REG(TIM2->CCMR2, TIM_CCMR2_OC4M, TIM_CCMR2_OC4M_1 + TIM_CCMR2_OC4M_2);	// PWM posedge first

	TIM2->EGR = TIM_EGR_UG;
	TIM2->CR1 |= TIM_CR1_CEN;
}
/* motor setting end */


void IR_init()	{
// IR_Module
	SET_REG(GPIOB->ODR, GPIO_ODR_OD4, GPIO_ODR_OD4);		//start transmitter_1
	SET_REG(GPIOB->ODR, GPIO_ODR_OD5, GPIO_ODR_OD5);		//start transmitter_2
	SET_REG(GPIOB->ODR, GPIO_ODR_OD6, GPIO_ODR_OD6);		//start transmitter_3
}

/* 7-seg display */
void display(int dis_val)	{
	int temp = 100000000, cur, state = 0, bit = 8;
	if(dis_val > temp || dis_val == -1)	{							// dis_val more than 8 digit print "-1"
		for(int i=0; i<8; i++)	{
			if(i == 6)	{
				max7219_send(8-i, 0x01);
			}
			else if(i == 7)	{
				max7219_send(8-i, sev_seg_val[1]);
			}
			else	{
				max7219_send(8-i, 0);
			}
		}
	}
	else if(dis_val == -2)	{							// dis_val more than 8 digit print "-1"
		for(int i=0; i<8; i++)	{
			max7219_send(8-i, 0);
		}
	}
	else	{										// print each digits
		while(bit != 0)	{
			temp /= 10;
			cur = dis_val / temp;
			dis_val %= temp;
			if(state == 0 && cur == 0)	{
				max7219_send(bit, 0);
			}
			else	{
				state = 1;
				max7219_send(bit, sev_seg_val[cur]);
			}
			bit--;
		}
	}
}

void EXTI_config()	{
	SET_REG(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN, RCC_APB2ENR_SYSCFGEN);
	SET_REG(SYSCFG->EXTICR[3], SYSCFG_EXTICR4_EXTI13, SYSCFG_EXTICR4_EXTI13_PC);
	SET_REG(EXTI->RTSR1, EXTI_RTSR1_RT13, EXTI_RTSR1_RT13);
	SET_REG(EXTI->IMR1, EXTI_IMR1_IM13, EXTI_IMR1_IM13);

	// set GPIO PC5,6,7,8 as interrupt input
	SET_REG(SYSCFG->EXTICR[2-1], SYSCFG_EXTICR2_EXTI5, SYSCFG_EXTICR2_EXTI5_PC);
	SET_REG(SYSCFG->EXTICR[2-1], SYSCFG_EXTICR2_EXTI6, SYSCFG_EXTICR2_EXTI6_PC);
	SET_REG(SYSCFG->EXTICR[2-1], SYSCFG_EXTICR2_EXTI7, SYSCFG_EXTICR2_EXTI7_PC);
	SET_REG(SYSCFG->EXTICR[3-1], SYSCFG_EXTICR3_EXTI8, SYSCFG_EXTICR3_EXTI8_PC);
	SET_REG(SYSCFG->EXTICR[3-1], SYSCFG_EXTICR3_EXTI10, SYSCFG_EXTICR3_EXTI10_PC);
	SET_REG(SYSCFG->EXTICR[3-1], SYSCFG_EXTICR3_EXTI11, SYSCFG_EXTICR3_EXTI11_PC);
	SET_REG(SYSCFG->EXTICR[4-1], SYSCFG_EXTICR4_EXTI12, SYSCFG_EXTICR4_EXTI12_PC);

	SET_REG(EXTI->RTSR1, EXTI_RTSR1_RT5, EXTI_RTSR1_RT5);
	SET_REG(EXTI->RTSR1, EXTI_RTSR1_RT6, EXTI_RTSR1_RT6);
	SET_REG(EXTI->RTSR1, EXTI_RTSR1_RT7, EXTI_RTSR1_RT7);
	SET_REG(EXTI->RTSR1, EXTI_RTSR1_RT8, EXTI_RTSR1_RT8);
	SET_REG(EXTI->RTSR1, EXTI_RTSR1_RT10, EXTI_RTSR1_RT10);
	SET_REG(EXTI->RTSR1, EXTI_RTSR1_RT11, EXTI_RTSR1_RT11);
	SET_REG(EXTI->RTSR1, EXTI_RTSR1_RT12, EXTI_RTSR1_RT12);

	SET_REG(EXTI->IMR1, EXTI_IMR1_IM5, EXTI_IMR1_IM5);
	SET_REG(EXTI->IMR1, EXTI_IMR1_IM6, EXTI_IMR1_IM6);
	SET_REG(EXTI->IMR1, EXTI_IMR1_IM7, EXTI_IMR1_IM7);
	SET_REG(EXTI->IMR1, EXTI_IMR1_IM8, EXTI_IMR1_IM8);
	SET_REG(EXTI->IMR1, EXTI_IMR1_IM10, EXTI_IMR1_IM10);
	SET_REG(EXTI->IMR1, EXTI_IMR1_IM11, EXTI_IMR1_IM11);
	SET_REG(EXTI->IMR1, EXTI_IMR1_IM12, EXTI_IMR1_IM12);

	//SET_REG(EXTI->PR1, EXTI_PR1_PIF5|EXTI_PR1_PIF6|EXTI_PR1_PIF7|EXTI_PR1_PIF8|EXTI_PR1_PIF10|EXTI_PR1_PIF11|EXTI_PR1_PIF12, 0);
}

void get_money(int time, int index){
	if(index == 10)	{		// 10
		for(int i = 0; i < time; i++)		{
			if(motor_state[2] == 0)	{
				TIM2->CCR4 = 44;
				motor_state[2] = 1;
			}
			else	{
				TIM2->CCR4 = 16;
				motor_state[2] = 0;
			}
			delay_us(700000);
		}
	}
	else if(index == 5)	{	// 5
		for(int i = 0; i < time; i++)		{
			if(motor_state[1] == 0)	{
				TIM2->CCR3 = 44;
				motor_state[1] = 1;
			}
			else	{
				TIM2->CCR3 = 16;
				motor_state[1] = 0;
			}
			delay_us(700000);
		}
	}
	else	{				// 1
		for(int i = 0; i < time; i++)		{
			if(motor_state[0] == 0)	{
				TIM2->CCR2 = 44;
				motor_state[0] = 1;
			}
			else	{
				TIM2->CCR2 = 16;
				motor_state[0] = 0;
			}
			delay_us(700000);
		}
	}
}

void NVIC_config(){
	// enable IRQ
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	NVIC_SetPriority(EXTI15_10_IRQn, 47);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_SetPriority(EXTI9_5_IRQn, 47);
}

void SysTick_Handler()	{
	// after a day input = output = 0


}

void EXTI15_10_IRQHandler()	{
	int pr = ((EXTI->PR1 >> 10) & 0x7);
	int which;
	switch(pr) {
		case 1:	// 1
			which = 0;
			input += 1;
			break;
		case 2:	// 5
			which = 1;
			input += 5;
			break;
		case 4:	// 10
			which = 2;
			input += 10;
			break;
		default:
			break;
	}
	if(which >= 0 && which <= 2)	{
		count[which] += 1;
	}
	// interrupt turn off
	SET_REG(EXTI->PR1, EXTI_PR1_PIF10, EXTI_PR1_PIF10);
	SET_REG(EXTI->PR1, EXTI_PR1_PIF11, EXTI_PR1_PIF11);
	SET_REG(EXTI->PR1, EXTI_PR1_PIF12, EXTI_PR1_PIF12);
}

void clear_input(void) {
	real_num = 0;
	mode = -1;
	sub_mode = 1;
}

void EXTI9_5_IRQHandler(void){
//	SysTick->CTRL = SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk);
	int c_idr = (EXTI->PR1 >> 5) & 0xF;
	int c_odr = (GPIOC->ODR >> 1) & 0xF;

	int column = 0, row = 0;
	if (c_idr == 1) column = 1;
	if (c_idr == 2) column = 2;
	if (c_idr == 4) column = 3;
	if (c_idr == 8) column = 4;
	if (c_odr == 1) row = 1;
	if (c_odr == 2) row = 2;
	if (c_odr == 4) row = 3;
	if (c_odr == 8) row = 4;
	int num;
	if (row <= 0 || column <= 0 || row >= 5 || column >= 5) {
		num = 14;
	} else {
		num = keypad_value[row-1][column-1];
	}
	delay_us(200000);
	if (num == 14) {
		display(-2);
		clear_input();
	} else {
		if (num == 10) {
			display(10001);
			first = 1;
			mode = 1;
		}
		if (num == 11) {
			display(10002);
			first = 1;
			mode = 2;
		}
		if (num == 12) {
			display(10003);
			first = 1;
			mode = 3;
		}
		if (num == 13) {
			display(10004);
			first = 1;
			mode = 4;
		}
		if (mode == -1) {
			display(-1);
			clear_input();
		} else {
			if (num == 15) {
				int a,b,c;
				switch(mode) {
					case 1:
						if (real_num > 1*count[0] + 5*count[1] + 10*count[2]) {
							display(-1);
							clear_input();
						} else {
							output += real_num;
							if (real_num / 10 > count[2]) {
								get_money(count[2], 10);
								count[2] = 0;
								real_num -= count[2] * 10;
							} else {
								get_money(real_num / 10, 10);
								count[2] -= real_num / 10;
								real_num -= (real_num / 10) * 10;
							}
							if (real_num / 5 > count[1]) {
								get_money(count[1], 5);
								count[1] = 0;
								real_num -= count[1] * 5;
							} else {
								get_money(real_num / 5, 5);
								count[1] -= real_num / 5;
								real_num -= (real_num / 5) * 5;
							}
							get_money(real_num, 1);
							delay_us(1000000);
							display(-2);
							count[0] -= real_num;
						}
						mode = -1;
						clear_input();
						break;
					case 2:
						switch(sub_mode) {
							case 1:
								if (real_num <= count[0]) {
									get_money(real_num, 1);
									sub_mode += 1;
									real_num = 0;
								} else {
									display(-1);
									clear_input();
								}
								break;
							case 2:
								if (real_num <= count[1]) {
									get_money(real_num, 5);
									sub_mode += 1;
									real_num = 0;
								} else {
									display(-1);
									clear_input();
								}
								break;
							case 3:
								if (real_num <= count[2]) {
									get_money(real_num, 10);
									sub_mode += 1;
								} else {
									display(-1);
								}
								clear_input();
								break;
						}
						delay_us(1000000);
						display(-2);
						break;
					case 3:
						a = count[0];
						b = count[1];
						c = count[2];
						display(count[0]+10000);
						delay_us(1000000);
						display(-2);
						delay_us(300000);
						display(count[1]+20000);
						delay_us(1000000);
						display(-2);
						delay_us(300000);
						display(count[2]+30000);
						delay_us(1000000);
						display(1*count[0] + 5*count[1] + 10*count[2]);
						delay_us(1000000);
						display(-2);
						mode = -1;
						clear_input();
						break;
					case 4:
						count[0] = 0;
						count[1] = 0;
						count[2] = 0;
						display(999999);
						delay_us(300000);
						display(-2);
						mode = -1;
						clear_input();
						break;
				}
			} else if (num == 10 || num == 11 || num == 12 || num == 13) {
				if (first == 1) {
					first = 0;
				} else {
					display(-1);
					clear_input();
				}
			} else {
				real_num *= 10;
				real_num += num;
				delay_us(100000);
				display(real_num);
			}
		}
	}
	// interrupt turn off
	SET_REG(EXTI->PR1, EXTI_PR1_PIF5, EXTI_PR1_PIF5);
	SET_REG(EXTI->PR1, EXTI_PR1_PIF6, EXTI_PR1_PIF6);
	SET_REG(EXTI->PR1, EXTI_PR1_PIF7, EXTI_PR1_PIF7);
	SET_REG(EXTI->PR1, EXTI_PR1_PIF8, EXTI_PR1_PIF8);

}

int main()	{
	GPIO_init();
	max7219_init();
	display(-2);
	timer_init();
	PWM_channel_init();
	IR_init();
	NVIC_config();
	EXTI_config();
	while(1)	{
		// give keypad value for each col
		for(int apos = 1; apos < 5; apos++) {
			GPIOC->ODR = 0x1 << apos;
		}
	}
}

//	SysTick_Config(1300000UL);


