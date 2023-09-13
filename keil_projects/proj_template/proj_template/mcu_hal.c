#include "mcu_hal.h"

// MCU_ADC_TAB* adcTable;

// void adc_init(void)
// {
// 	CLK_EnableModuleClock(ADC_MODULE);

// 	//P10���ó�ADC channel1 analog input
// 	SYS->P1_MFP |= SYS_MFP_P10_ADC_CH1;
// 	//P12���ó�ADC channel2 analog input
// 	SYS->P1_MFP |= SYS_MFP_P12_ADC_CH2;

// 	// Enable channel 2
// 	// ADC_Open(ADC, 0, 0, 0x01 << 1);	// P10��2.5vֻ��һ��,P12��100����ƽ��ֵ
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
// 	while((ADC) -> STATUS |= ADC_STATUS_ADIF_Msk)	// ��������⣬()�ﲻ���ж�ADC_STATUS[0]��0��1
// 	{// ת����һ�����к���
// 		�Ƿ���Ҫ����ƽ��ֵ
// 		��ʵ�ʵ�ѹֵ
// 		������һ��ת��
// 	}
// }

// uint16 Get_Adc_Average(uint32 adcChannel,uint16 rollCount)
// {
// 	uint32 adcCodeSum = 0;
// 	for(int32_t t=0; t < rollCount; t++)
// 	{
// 		ADC_Open(ADC, 0, 0, adcChannel);	// 0x01 << 2
// 		ADC_START_CONV(ADC);	// �Ҳ�֪������ж��������ã�����˵��ת������Զ���?
// 		ADC_EnableInt(ADC, ADC_ADIF_INT);
// 		(ADC) -> STATUS |= ADC_STATUS_ADIF_Msk;	//  ADIF: This bit can be cleared to 0 by software writing 1
// 												// ͨ�����д1��ADIFλ��0�Ƿ��ʾ�˳�ADC����ж�
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
// 	;// ת������жϴ���
// 	// ��ȡadcCode��ֵ��adcTablee[idx].adcCode
// 	adcTable[idx].adcCodeSum += adcTablee[idx].adcCode;
// 	// rollCount++;
// 	// �Ƿ���ʣ���idxδת��
// 	// �еĻ� adcNowConvIdx++;ADC_Open(ADC, 0, 0, adcTablee[adcNowConvIdx].adcChannel);
// 	//ADC_START_CONV(ADC);
// 	// clear int flag?

// }





// ����׷������ɵ�ADCת������
volatile uint8_t adcCount = 0;

// ��ʼ��ADC
void adc_init(void)
{
	CLK_EnableModuleClock(ADC_MODULE);

	//P10���ó�ADC channel1 analog input
	SYS->P1_MFP |= SYS_MFP_P10_ADC_CH1;
	//P12���ó�ADC channel2 analog input
	SYS->P1_MFP |= SYS_MFP_P12_ADC_CH2;

	// Enable channel 2
	// ADC_Open(ADC, 0, 0, 0x01 << 1);	// P10��2.5vֻ��һ��,P12��100����ƽ��ֵ
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

// ADC����жϷ������
void adc_isr(MCU_ADC_TAB* p_adcVal)
{
	p_adcVal = NULL;	// ��ʼ��p_adcVal
	// ��ȡADC��������д���
    *p_adcVal += ADC_GET_CONVERSION_DATA(ADC, 0);	// �Ƿ������volatile  adcValue����MCU_ADC_TAB* p_adcVal
    
    // ��������ɵ�ת������
    adcCount++;
    
    if (adcCount < 100) 
	{
        // ���δ���100��ת��,������һ��ADCת��
        ADC_START_CONV(ADC);
    } 
	else 
	{
        // ��������100��ת��,����ADCģ��
		ADC_POWER_DOWN(ADC);
    }
}// ָ��Ļ�, p_adcVal��ֵ�ܱ���,����while,������

int adc_main() 
 {
    adc_init();
    
    // ������һ��ADCת��
	ADC_START_CONV(ADC);
    
    // ��ѭ��
    while (1) {
        // ���ж��д���ADCֵ����ѭ������ִ����������
    }
    
    return 0;
}
