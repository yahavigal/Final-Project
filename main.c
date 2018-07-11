/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include "MCC_Generated_Files/mcc.h"

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int main ( void )
{
    int j, i;
    float avg_adc = 0;
    uint8_t* sample = (uint8_t*)(&avg_adc), sample1, sample2, sample3, sample4, delimeter;
    uint8_t pdata_tia[2], pdata_ref[2], pdata_mode[2], pdata_lock[2], pdata_r1[1], pdata_r2[1], pdata_r3[1];
    uint8_t lock_address = 0x01, tiacn_address = 0x10, refcn_address = 0x11, modecn_address = 0x12;
    uint16_t slave_address = 0x48;
    I2C2_MESSAGE_STATUS pstatus = 0;
    static I2C2_TRANSACTION_REQUEST_BLOCK   trBlock[2];

    /* Call the System Initialize routine*/
    SYSTEM_Initialize();
    ADC1_ChannelSelect(ADC1_CHANNEL_AN10);
    ANSCbits.ANSC0 = 1 ;
    TRISCbits.TRISC0 = 1;
    
    /* configure the lmp91000 LOCK register */
    pdata_lock[0] = lock_address;
    pdata_lock[1] = 0x00;
    I2C2_MasterWrite(pdata_lock, 2, slave_address, &pstatus);
    while(!I2C2_MasterQueueIsEmpty());
    while(pstatus != I2C2_MESSAGE_COMPLETE);
    
    /* configure lmp91000 TIACN register */
    pdata_tia[0] = tiacn_address;
    pdata_tia[1] = 0x1C;
    I2C2_MasterWrite(pdata_tia, 2, slave_address, &pstatus);
    while(!I2C2_MasterQueueIsEmpty());
    while(pstatus != I2C2_MESSAGE_COMPLETE);
    
    /* configure lmp91000 REFCN register */
    pdata_ref[0] = refcn_address;
    pdata_ref[1] = 0x10;
    I2C2_MasterWrite(pdata_ref, 2, slave_address, &pstatus);
    while(!I2C2_MasterQueueIsEmpty());
    while(pstatus != I2C2_MESSAGE_COMPLETE);
    
    /* configure lmp91000 MODECN register */
    pdata_mode[0] = modecn_address;
    pdata_mode[1] = 0x03;
    I2C2_MasterWrite(pdata_mode, 2, slave_address, &pstatus);
    while(!I2C2_MasterQueueIsEmpty());
    while(pstatus != I2C2_MESSAGE_COMPLETE);
    
    /* delay the configuration to create a step input */
    for (j=0; j<10000; j++);
    
    /* configure lmp91000 REFCN register */
    pdata_ref[0] = refcn_address;
    pdata_ref[1] = 0x14;    // for 0.2V
//    pdata_ref[1] = 0x1c;    // for 0.7V
    I2C2_MasterWrite(pdata_ref, 2, slave_address, &pstatus);
    while(!I2C2_MasterQueueIsEmpty());
    while(pstatus != I2C2_MESSAGE_COMPLETE);
    
    /* read the registers */
    pdata_r1[0] = 0x00;
    pdata_r2[0] = 0x00;
    pdata_r3[0] = 0x00;
    
    /* read TIACN register content */
    I2C2_MasterWriteTRBBuild(&trBlock[0], pdata_tia, 1, slave_address);
    I2C2_MasterReadTRBBuild(&trBlock[1], pdata_r1, 1, slave_address);
    I2C2_MasterTRBInsert(2, &trBlock[0], &pstatus);
    while(pstatus != I2C2_MESSAGE_COMPLETE);
    
    /* read REFCN register content */
    I2C2_MasterWriteTRBBuild(&trBlock[0], pdata_ref, 1, slave_address);
    I2C2_MasterReadTRBBuild(&trBlock[1], pdata_r2, 1, slave_address);
    I2C2_MasterTRBInsert(2, &trBlock[0], &pstatus);
    while(pstatus != I2C2_MESSAGE_COMPLETE);
    
    /* read MODECN register content */
    I2C2_MasterWriteTRBBuild(&trBlock[0], pdata_mode, 1, slave_address);
    I2C2_MasterReadTRBBuild(&trBlock[1], pdata_r3, 1, slave_address);
    I2C2_MasterTRBInsert(2, &trBlock[0], &pstatus);
    while(pstatus != I2C2_MESSAGE_COMPLETE);
    
    /* write registers content to file through uart */
    UART2_Write(pdata_r1[0]);
    while(!(UART2_StatusGet() & UART2_TX_COMPLETE));  
    UART2_Write(pdata_r2[0]);
    while(!(UART2_StatusGet() & UART2_TX_COMPLETE));  
    UART2_Write(pdata_r3[0]);
    while(!(UART2_StatusGet() & UART2_TX_COMPLETE)); 
    
    /* sample A/D converter */
    AD1CHS = ADC1_CHANNEL_AN10;
    while(1)
    {
        AD1CON1bits.SAMP = 1;           //Start sampling
        for (i = 0 ; i < 1000 ; i++) ; //Sample delay, conversion start automatically

        AD1CON1bits.SAMP = 0;           //Start sampling
        for (i = 0 ; i < 1000 ; i++) ; //Sample delay, conversion start automatically

        while(!AD1CON1bits.DONE);       //Wait for conversion to complete
        avg_adc = ADC1BUF0;

        /* convert sample to float and parse it through 4 ints */
        avg_adc = (avg_adc * 3.3) / 1023;
        sample1 = *(sample);
        sample2 = *(sample+1);
        sample3 = *(sample+2);
        sample4 = *(sample+3);
        delimeter = 0x20;
        
        /* write A/D sample to file through uart (MSB first, then LSB */
        UART2_Write(sample4);
        while(!(UART2_StatusGet() & UART2_TX_COMPLETE)); 
        UART2_Write(sample3);
        while(!(UART2_StatusGet() & UART2_TX_COMPLETE)); 
        UART2_Write(sample2);
        while(!(UART2_StatusGet() & UART2_TX_COMPLETE)); 
        UART2_Write(sample1);
        while(!(UART2_StatusGet() & UART2_TX_COMPLETE));
        UART2_Write(delimeter);
        while(!(UART2_StatusGet() & UART2_TX_COMPLETE)); 
    }
}