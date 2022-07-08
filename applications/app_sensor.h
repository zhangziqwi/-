/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-02     hehung       the first version
 */
#ifndef APPLICATIONS_APP_SENSOR_H_
#define APPLICATIONS_APP_SENSOR_H_


/*STTS751 ÎÂ¶È´«¸ÐÆ÷*/
#define STTS751_REG_STATUS      (0x01U)
#define STTS751_REG_CONFIG      (0x03U)
#define STTS751_REG_CONRAT      (0x04U)

#define STTS751_REG_TEMPVH      (0x00U)
#define STTS751_REG_TEMPVL      (0x02U)

#define STTS751_REG_TEMPHH      (0x05U)
#define STTS751_REG_TEMPHL      (0x06U)
#define STTS751_REG_TEMPLH      (0x07U)
#define STTS751_REG_TEMPLL      (0x08U)
#define STTS751_REG_ONESHOT     (0x0FU)
#define STTS751_REG_MANUALID    (0xFEU)
#define STTS751_REG_PRODUCT     (0xFDU)

extern int Read_Temperature_H_STTS751(void);
extern int Read_Temperature_L_STTS751(void);
extern signed char Read_STTS751_Manualfacture_ID(void);
extern signed char Read_STTS751_Product_ID(void);


/*IIC slave address*/
#define STTS751_ADDR            0x94U


#define LPS22HH_REG_INT_CFG     (0x0BU)
#define LPS22HH_REG_THS_P_L     (0x0CU)
#define LPS22HH_REG_THS_P_H     (0x0DU)
#define LPS22HH_REG_IF_CTRL     (0x0EU)

#define LPS22HH_REG_WHO_AM_I    (0x0FU)
#define LPS22HH_REG_CTRL_1      (0x10U)
#define LPS22HH_REG_CTRL_2      (0x11U)
#define LPS22HH_REG_CTRL_3      (0x12U)

#define LPS22HH_REG_REF_P_L     (0x15U)
#define LPS22HH_REG_REF_P_H     (0x16U)
#define LPS22HH_REG_RPDS_L      (0x18U)
#define LPS22HH_REG_RPDS_H      (0x19U)
#define LPS22HH_REG_INT_SOURCE  (0x24U)

#define LPS22HH_REG_STATUS      (0x27U)
#define LPS22HH_REG_PRE_OUT_XL  (0x28U)
#define LPS22HH_REG_PRE_OUT_L   (0x29U)
#define LPS22HH_REG_PRE_OUT_H   (0x2AU)

#define LPS22HH_REG_TEMP_OUT_L  (0x2BU)
#define LPS22HH_REG_TEMP_OUT_H  (0x2CU)

/*IIC slave address write and read*/
/*SD0 connect to power supply*/
#define LPS22HH_ADDR_W_1            0xBAU
#define LPS22HH_ADDR_R_1            0xBBU
/*SD0 connect to ground*/
#define LPS22HH_ADDR_W_0            0xB8U
#define LPS22HH_ADDR_R_0            0xB9U

extern uint8_t Get_WHO_AM_I_LPS22HH(void);
extern int Get_Status_LPS22HH(void);
extern int Get_Pressure_XL_LPS22HH(void);
extern int Get_Pressure_L_LPS22HH(void);
extern int Get_Pressure_H_LPS22HH(void);
extern int Get_Temp_L_LPS22HH(void);
extern int Get_Temp_H_LPS22HH(void);
extern int Get_Pressure(void);
extern int Get_Temp(void);
extern void Init_LPS22HH(void);


#define HTS221_REG_WHO_AM_I     (0x0FU)

#define HTS221_AV_CONF          (0x10U)
#define HTS221_REG_CTRL_1       (0x20U)
#define HTS221_REG_CTRL_2       (0x21U)
#define HTS221_REG_CTRL_3       (0x22U)

#define HTS221_REG_STATUS       (0x27U)

#define HTS221_REG_HUMI_OUT_L   (0x28U)
#define HTS221_REG_HUMI_OUT_H   (0x29U)

#define HTS221_REG_TEMP_OUT_L   (0x2AU)
#define HTS221_REG_TEMP_OUT_H   (0x2BU)

#define HTS221_REG_H0_rH_x2     (0x30U)
#define HTS221_REG_H1_rH_x2     (0x31U)

#define HTS221_REG_T0_degC_x8   (0x32U)
#define HTS221_REG_T1_degC_x8   (0x33U)
#define HTS221_REG_T1_T0        (0x35U)

#define HTS221_REG_H0_T0_OUT1   (0x36U)
#define HTS221_REG_H0_T0_OUT2   (0x37U)

#define HTS221_REG_H1_T0_OUT1   (0x3AU)
#define HTS221_REG_H1_T0_OUT2   (0x3BU)

#define HTS221_REG_T0_OUT1      (0x3CU)
#define HTS221_REG_T0_OUT2      (0x3DU)

#define HTS221_REG_T1_OUT1      (0x3EU)
#define HTS221_REG_T1_OUT2      (0x3FU)

/*IIC slave address write and read*/
#define HTS221_ADDR_W_1         0xBEU
#define HTS221_ADDR_R_1         0xBFU

extern int Get_WHO_AM_I_HTS221(void);
extern int Get_Status_HTS221(void);
extern int Get_Humidity_L_HTS221(void);
extern int Get_Humidity_H_HTS221(void);
extern int Get_Temp_L_HTS221(void);
extern int Get_Temp_H_HTS221(void);
extern int Get_Humidity_HTS221(void);
extern int Get_Temp_HTS221(void);
extern void Init_HTS221(void);
extern int HTS221_GetCalHumi(void);
extern int HTS221_GetCalTemp(void);

/*register address*/
#define LIS2DW12_OUT_T_L            (0x0DU)
#define LIS2DW12_OUT_T_H            (0x0EU)

#define LIS2DW12_REG_WHO_AM_I       (0x0FU)

#define LIS2DW12_CTRL1              (0x20U)
#define LIS2DW12_CTRL2              (0x21U)
#define LIS2DW12_CTRL3              (0x22U)
#define LIS2DW12_CTRL4              (0x23U)
#define LIS2DW12_CTRL5              (0x24U)
#define LIS2DW12_CTRL6              (0x25U)

#define LIS2DW12_OUT_T              (0x26U)

#define LIS2DW12_CFG_STATUS         (0x27U)

#define LIS2DW12_OUT_X_L            (0x28U)
#define LIS2DW12_OUT_X_H            (0x29U)
#define LIS2DW12_OUT_Y_L            (0x2AU)
#define LIS2DW12_OUT_Y_H            (0x2BU)
#define LIS2DW12_OUT_Z_L            (0x2CU)
#define LIS2DW12_OUT_Z_H            (0x2DU)

#define LIS2DW12_FIFO_CTRL          (0x2EU)
#define LIS2DW12_FIFO_SAMPLES       (0x2FU)

#define LIS2DW12_TAP_THS_X          (0x30U)
#define LIS2DW12_TAP_THS_Y          (0x31U)
#define LIS2DW12_TAP_THS_Z          (0x32U)
#define LIS2DW12_INT_DUR            (0x33U)

#define LIS2DW12_WAKE_UP_THS        (0x34U)
#define LIS2DW12_WAKE_UP_DUR        (0x35U)

#define LIS2DW12_FREE_FALL          (0x36U)
#define LIS2DW12_STATUS_DUP         (0x37U)
#define LIS2DW12_WAKE_UP_SRC        (0x38U)
#define LIS2DW12_TAP_SRC            (0x39U)
#define LIS2DW12_SIXD_SRC           (0x3AU)

#define LIS2DW12_ALL_INT_SRC        (0x3BU)

#define LIS2DW12_X_OFS_USR          (0x3CU)
#define LIS2DW12_Y_OFS_USR          (0x3DU)
#define LIS2DW12_Z_OFS_USR          (0x3EU)

#define LIS2DW12_CTRL7              (0x3FU)

/*IIC slave address write and read*/
/*SD0 connect to power supply*/
#define LIS2DW12_ADDR_W_1           0x32U
#define LIS2DW12_ADDR_R_1           0x33U

extern int Set_Reg_LIS2DW12(uint8_t add, uint8_t reg, uint8_t dat);
extern int Get_Reg_LIS2DW12(uint8_t add, uint8_t reg);
extern int Get_WHO_AM_I_LIS2DW12(void);
extern int Get_Status_LIS2DW12(void);
extern int Get_Status_LIS2DW12(void);
extern float Get_Temp_1_LIS2DW12(void);
extern signed char Get_Temp_2_LIS2DW12(void);
extern short Get_X_LIS2DW12(void);
extern short Get_Y_LIS2DW12(void);
extern short Get_Z_LIS2DW12(void);
extern void Init_LIS2DW12(void);
extern void Send_Uart_LIS2DW12(void);

#define LSM6DSO_FUNC_CFG_ACCESS     (0x01U)
#define LSM6DSO_PIN_CTRLs           (0x02U)

#define LSM6DSO_COUNTER_BDR_REG1    (0x0BU)
#define LSM6DSO_COUNTER_BDR_REG2    (0x0CU)

#define LSM6DSO_WHO_AM_I            (0x0FU)

#define LSM6DSO_CTRL1_XL            (0x10U)
#define LSM6DSO_CTRL2_G             (0x11U)
#define LSM6DSO_CTRL3_C             (0x12U)
#define LSM6DSO_CTRL4_C             (0x13U)
#define LSM6DSO_CTRL5_C             (0x14U)
#define LSM6DSO_CTRL6_G             (0x15U)
#define LSM6DSO_CTRL7_G             (0x16U)
#define LSM6DSO_CTRL8_XL            (0x17U)
#define LSM6DSO_CTRL9_XL            (0x18U)
#define LSM6DSO_CTRL10_C            (0x19U)

#define LSM6DSO_CFG_STATUS          (0x1EU)

#define LSM6DSO_TEMP_L              (0x20U)
#define LSM6DSO_TEMP_H              (0x21U)

#define LSM6DSO_OUT_X_L_G           (0x22U)
#define LSM6DSO_OUT_X_H_G           (0x23U)
#define LSM6DSO_OUT_Y_L_G           (0x24U)
#define LSM6DSO_OUT_Y_H_G           (0x25U)
#define LSM6DSO_OUT_Z_L_G           (0x26U)
#define LSM6DSO_OUT_Z_H_G           (0x27U)

#define LSM6DSO_OUT_X_L_A           (0x28U)
#define LSM6DSO_OUT_X_H_A           (0x29U)
#define LSM6DSO_OUT_Y_L_A           (0x2AU)
#define LSM6DSO_OUT_Y_H_A           (0x2BU)
#define LSM6DSO_OUT_Z_L_A           (0x2CU)
#define LSM6DSO_OUT_Z_H_A           (0x2DU)

#define LSM6DSO_EMB_FUNC_EN_A       (0x04U)
#define LSM6DSO_EMB_FUNC_EN_B       (0x05U)

#define LSM6DSO_STEP_COUNTER_L      (0x62U)
#define LSM6DSO_STEP_COUNTER_H      (0x63U)
#define LSM6DSO_EMB_FUNC_SRC        (0x64U)


/*IIC slave address write and read*/
/*SD0 connect to power supply*/
#define LSM6DSO_ADDR_W_1            0xD6U
#define LSM6DSO_ADDR_R_1            0xD7U


/* Exported Functions --------------------------------------------------------*/
extern int Set_Reg_LSM6DSO(uint8_t add, uint8_t reg, uint8_t dat);
extern int Get_Reg_LSM6DSO(uint8_t add, uint8_t reg);
extern int Get_WHO_AM_I_LSM6DSO(void);
extern int Get_Status_LSM6DSO(void);
extern float Get_Temp_LSM6DSO(void);
extern short Get_X_G_LSM6DSO(void);
extern short Get_Y_G_LSM6DSO(void);
extern short Get_Z_G_LSM6DSO(void);
extern short Get_X_A_LSM6DSO(void);
extern short Get_Y_A_LSM6DSO(void);
extern short Get_Z_A_LSM6DSO(void);
extern void Init_LSM6DSO(void);




#define LIS2MDL_OFFSET_X_REG_L      (0x45U)
#define LIS2MDL_OFFSET_X_REG_H      (0x46U)
#define LIS2MDL_OFFSET_Y_REG_L      (0x47U)
#define LIS2MDL_OFFSET_Y_REG_H      (0x48U)
#define LIS2MDL_OFFSET_Z_REG_L      (0x49U)
#define LIS2MDL_OFFSET_Z_REG_H      (0x4AU)

#define LIS2MDL_REG_WHO_AM_I        (0x4FU)

#define LIS2MDL_CFG_A               (0x60U)
#define LIS2MDL_CFG_B               (0x61U)
#define LIS2MDL_CFG_C               (0x62U)

#define LIS2MDL_CFG_INT_CTRL        (0x63U)
#define LIS2MDL_CFG_SOURCE          (0x64U)
#define LIS2MDL_CFG_THS_L           (0x65U)
#define LIS2MDL_CFG_THS_H           (0x66U)

#define LIS2MDL_CFG_STATUS          (0x67U)

#define LIS2MDL_OUTX_L              (0x68U)
#define LIS2MDL_OUTX_H              (0x69U)
#define LIS2MDL_OUTY_L              (0x6AU)
#define LIS2MDL_OUTY_H              (0x6BU)
#define LIS2MDL_OUTZ_L              (0x6CU)
#define LIS2MDL_OUTZ_H              (0x6DU)

#define LIS2MDL_TEMP_L              (0x6EU)
#define LIS2MDL_TEMP_H              (0x6FU)


/*IIC slave address write and read*/
/*SD0 connect to power supply*/
#define LIS2MDL_ADDR_W_1            0x3CU
#define LIS2MDL_ADDR_R_1            0x3DU


/* Exported Functions --------------------------------------------------------*/
extern int Set_Reg_LIS2MDL(uint8_t add, uint8_t reg, uint8_t dat);
extern int Get_Reg_LIS2MDL(uint8_t add, uint8_t reg);
extern int Get_WHO_AM_I_LIS2MDL(void);
extern int Get_Status_LIS2MDL(void);
extern int Get_Temp_LIS2MDL(void);
extern short GET_X_LIS2MDL(void);
extern short GET_Y_LIS2MDL(void);
extern short GET_Z_LIS2MDL(void);
extern void Init_LIS2MDL(void);

extern void Send_Uart_LIS2MDL(void);
extern void OLED_Display_LIS2MDL(void);


int Get_Reg(uint8_t add, uint8_t reg);
int Set_Reg(uint8_t add, uint8_t reg, uint8_t dat);


#endif /* APPLICATIONS_APP_SENSOR_H_ */
