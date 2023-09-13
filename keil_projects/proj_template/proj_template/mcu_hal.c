#include "mcu_hal.h"

// MCU_ADC_TAB* adcTable;

// void adc_init(void)
// {
// 	CLK_EnableModuleClock(ADC_MODULE);

// 	//P10设置成ADC channel1 analog input
// 	SYS->P1_MFP |= SYS_MFP_P10_ADC_CH1;
// 	//P12设置成ADC channel2 analog input
// 	SYS->P1_MFP |= SYS_MFP_P12_ADC_CH2;

// 	// Enable channel 2
// 	// ADC_Open(ADC, 0, 0, 0x01 << 1);	// P10脚2.5v只采一次,P12采100次求平均值
// 	ADC_Open(ADC, 0, 0, 0x01 << 2);

// 	//Select ADC input range.1 means 0.4V~2.4V ;0 means 0.4V~1.4V.
// 	//0.4V~2.4V & 0.4V~1.4V both are theoretical value,the real range is determined by bandgap voltage.
// 	ADC_SelInputRange(ADC_INPUTRANGE_HIGH);

// 	// Power on ADC
// 	ADC_POWER_ON(ADC);

// 	// Enable ADC convert complete interrupt
// 	ADC_EnableInt(ADC, ADC_ADIF_INT);
// 	NVIC_EnableIRQ(ADC_IRQn);
// }

// void adc_main(void)
// {
// 	while((ADC) -> STATUS |= ADC_STATUS_ADIF_Msk)	// 语句有问题，()里不能判断ADC_STATUS[0]是0是1
// 	{// 转换完一次序列后处理
// 		是否需要计算平均值
// 		算实际电压值
// 		开启下一次转换
// 	}
// }

// uint16 Get_Adc_Average(uint32 adcChannel,uint16 rollCount)
// {
// 	uint32 adcCodeSum = 0;
// 	for(int32_t t=0; t < rollCount; t++)
// 	{
// 		ADC_Open(ADC, 0, 0, adcChannel);	// 0x01 << 2
// 		ADC_START_CONV(ADC);	// 我不知道完成中断在那里用，还是说，转换完会自动进?
// 		ADC_EnableInt(ADC, ADC_ADIF_INT);
// 		(ADC) -> STATUS |= ADC_STATUS_ADIF_Msk;	//  ADIF: This bit can be cleared to 0 by software writing 1
// 												// 通过软件写1给ADIF位清0是否表示退出ADC完成中断
// 		adcCodeSum += (uint32_t)ADC_GET_CONVERSION_DATA(ADC, 0);
// 	}
// 	return adcCodeSum/rollCount;
// } 

// uint32 adc_getVoltage(uint8 idx)
// {
// 	return adcTable[idx].voltage;
// }

// void adc_isr(void)
// {
// 	;// 转换完成中断处理
// 	// 读取adcCode赋值给adcTablee[idx].adcCode
// 	adcTable[idx].adcCodeSum += adcTablee[idx].adcCode;
// 	// rollCount++;
// 	// 是否有剩余的idx未转换
// 	// 有的话 adcNowConvIdx++;ADC_Open(ADC, 0, 0, adcTablee[adcNowConvIdx].adcChannel);
// 	//ADC_START_CONV(ADC);
// 	// clear int flag?

// }





// 用于追踪已完成的ADC转换次数
volatile uint8_t adcCount = 0;

// 初始化ADC
void adc_init(void)
{
	CLK_EnableModuleClock(ADC_MODULE);

	//P10设置成ADC channel1 analog input
	SYS->P1_MFP |= SYS_MFP_P10_ADC_CH1;
	//P12设置成ADC channel2 analog input
	SYS->P1_MFP |= SYS_MFP_P12_ADC_CH2;

	// Enable channel 2
	// ADC_Open(ADC, 0, 0, 0x01 << 1);	// P10脚2.5v只采一次,P12采100次求平均值
	ADC_Open(ADC, 0, 0, 0x01 << 2);

	//Select ADC input range.1 means 0.4V~2.4V ;0 means 0.4V~1.4V.
	//0.4V~2.4V & 0.4V~1.4V both are theoretical value,the real range is determined by bandgap voltage.
	ADC_SelInputRange(ADC_INPUTRANGE_HIGH);

	// Power on ADC
	ADC_POWER_ON(ADC);

	// Enable ADC convert complete interrupt
	ADC_EnableInt(ADC, ADC_ADIF_INT);
	NVIC_EnableIRQ(ADC_IRQn);
}

// ADC完成中断服务程序
void adc_isr(MCU_ADC_TAB* p_adcVal)
{
	p_adcVal = NULL;	// 初始化p_adcVal
	// 读取ADC结果并进行处理
    *p_adcVal += ADC_GET_CONVERSION_DATA(ADC, 0);	// 是否可以用volatile  adcValue代替MCU_ADC_TAB* p_adcVal
    
    // 增加已完成的转换次数
    adcCount++;
    
    if (adcCount < 100) 
	{
        // 如果未完成100次转换,启动下一次ADC转换
        ADC_START_CONV(ADC);
    } 
	else 
	{
        // 如果已完成100次转换,禁用ADC模块
		ADC_POWER_DOWN(ADC);
    }
}// 指针的话, p_adcVal的值能保留,不用while,不阻塞

int adc_main() 
 {
    adc_init();
    
    // 启动第一次ADC转换
	ADC_START_CONV(ADC);
    
    // 主循环
    while (1) {
        // 在中断中处理ADC值，主循环可以执行其他任务
    }
    
    return 0;
}
