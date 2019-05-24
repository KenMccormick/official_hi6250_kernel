/*
 * Copyright (C) huawei company
 *
 * This	program	is free	software; you can redistribute it and/or modify
 * it under	the	terms of the GNU General Public	License	version	2 as
 * published by	the	Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/delay.h>
#include "inputhub_route.h"
#include "inputhub_bridge.h"
#include "rdr_sensorhub.h"
#include "protocol.h"
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/fs.h>
#include "sensor_info.h"
#include "sensor_sys_info.h"
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/rtc.h>
#include <linux/jiffies.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/of_device.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <huawei_platform/inputhub/sensorhub.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_HW_TOUCH_KEY
#include <huawei_platform/sensor/huawei_key.h>
#endif
#include <linux/gpio.h>
#include <huawei_platform/power/charger/charger_ap/bq25892/bq25892_charger.h>

/*
* CONFIG_USE_CAMERA3_ARCH : the camera module build config
* du to the sensor_1V8 by camera power chip
*/
#ifdef CONFIG_USE_CAMERA3_ARCH
#define SENSOR_1V8_POWER
#endif

#ifdef SENSOR_1V8_POWER
#include <media/huawei/hw_extern_pmic.h>
#endif

#ifdef SENSOR_1V8_POWER
extern int hw_extern_pmic_query_state(int index, int *state);
#else
int hw_extern_pmic_query_state(int index, int *state)
{
	hwlog_err("the camera power cfg donot define\n");
	return 1;
}
#endif

extern struct sensor_detect_manager s_detect_manager[SENSOR_MAX];
int hisi_nve_direct_access(struct hisi_nve_info_user *user_info);

#ifdef CONFIG_HUAWEI_DSM
#define SENSOR_DSM_CONFIG
#endif
#define SENSOR_CONFIG_DATA_OK 0
#define MAX_MCU_DATA_LENGTH  30
#define ACC_OFFSET_NV_NUM	307
#define ACC_OFFSET_NV_SIZE	(60)
#define MAG_CALIBRATE_DATA_NV_NUM 233
#define MAG_CALIBRATE_DATA_NV_SIZE (MAX_MAG_CALIBRATE_DATA_LENGTH)
#define MAG_AKM_CALIBRATE_DATA_NV_SIZE (MAX_MAG_AKM_CALIBRATE_DATA_LENGTH)
#define OFFSET_BIG               127
#define OFFSET_SMALL             -128
#define Z_SCALE_BIG              255
#define Z_SCALE_SMALL            175
#define MAX_STR_SIZE	1024
#define DMP_DRIVER_FILE_ID 5
#define DMP_IMG_FILE_ID 6
#define ICM20628_ADDR   0x68
#define REG_BANK_SEL    0x7f
#define INT_PIN_CFG     0x0f
#define PWR_MGMT_1      0x06
#define SENSOR_VBUS "sensor-io"
#define DEBUG_DATA_LENGTH 10

#define NO_LCDTYPE_FOUND 	-6

#define TP_COLOR_BUF_SIZE		20
/*read max number from dts adapt sensor list*/
//lint -save -e732 -e713
enum PHONE_TYPE {
	NEXT = 11,
	KNIGHT,
	EVA,
	FARADAY,
	VIENNA,
	VNS,
	EDISON,
	BTV,
        NATASHA,
	CHICAGO,
	CMB,
	LON,/*22*/
	VICTORIA,
	VICKY,
	BLN,
	PRA,/*26*/
	DUKE,
	WAS,/*28*/
	PIC,
	BAC,/*30*/
	BOND,/*31*/
	HAYDN,/*32*/
	RNE,/*33*/
	FIGO,/*34*/
	LELAND,/*35*/
	ANE, /*36*/
	FLORIDA, /*37*/
	BACH2,/*38*/
	HANDEL,/*39*/
	LELANDPLUS = 40,/*40*/
};
enum PHONE_VERSION {
	V3 = 10,		/*decimal base*/
	V3_A = 101,
	V4 = 11,
	VN1,
	VN2,
	V1,
};

enum ALS_SENSNAME{
	APDS9922 = 1,
	LTR578 = 2,
};

#define DTS_COMP_LG_ER69006A "hisilicon,mipi_lg_eR69006A"
#define DTS_COMP_JDI_NT35695_CUT3_1 "hisilicon,mipi_jdi_NT35695_cut3_1"
#define DTS_COMP_JDI_NT35695_CUT2_5 "hisilicon,mipi_jdi_NT35695_cut2_5"

#define DTS_COMP_LG_ER69007  "hisilicon,mipi_lg_eR69007"
#define DTS_COMP_SHARP_NT35597  "hisilicon,mipi_sharp_knt_NT35597"
#define DTS_COMP_LG_ER69006_FHD      "hisilicon,mipi_lg_eR69006_FHD"
#define DTS_COMP_SHARP_NT35695  "hisilicon,mipi_sharp_NT35695_5p7"
#define DTS_COMP_MIPI_BOE_ER69006  "hisilicon,mipi_boe_ER69006_5P7"

#define DTS_COMP_BOE_OTM1906C  "hisilicon,boe_otm1906c_5p2_1080p_cmd"
#define DTS_COMP_EBBG_OTM1906C  "hisilicon,ebbg_otm1906c_5p2_1080p_cmd"
#define DTS_COMP_INX_OTM1906C  "hisilicon,inx_otm1906c_5p2_1080p_cmd"
#define DTS_COMP_JDI_NT35695  "hisilicon,jdi_nt35695_5p2_1080p_cmd"
#define DTS_COMP_LG_R69006  "hisilicon,lg_r69006_5p2_1080p_cmd"
#define DTS_COMP_SAMSUNG_S6E3HA3X02 "hisilicon,mipi_samsung_S6E3HA3X02"

#define DTS_COMP_LG_R69006_5P2  "hisilicon,mipi_lg_R69006_5P2"
#define DTS_COMP_SHARP_NT35695_5P2  "hisilicon,mipi_sharp_NT35695_5P2"
#define DTS_COMP_JDI_R63452_5P2  "hisilicon,mipi_jdi_R63452_5P2"

#define DTS_COMP_SAM_WQ_5P5  "hisilicon,mipi_samsung_S6E3HA3X02_5P5_AMOLED"
#define DTS_COMP_SAM_FHD_5P5  "hisilicon,mipi_samsung_D53G6EA8064T_5P5_AMOLED"

#define DTS_COMP_JDI_R63450_5P7 "hisilicon,mipi_jdi_duke_R63450_5P7"
#define DTS_COMP_SHARP_DUKE_NT35597 "hisilicon,mipi_sharp_duke_NT35597"

#define DTS_COMP_AUO_OTM1901A_5P2 "auo_otm1901a_5p2_1080p_video"
#define DTS_COMP_AUO_TD4310_5P2 "auo_td4310_5p2_1080p_video"
#define DTS_COMP_TM_FT8716_5P2 "tm_ft8716_5p2_1080p_video"
#define DTS_COMP_EBBG_NT35596S_5P2 "ebbg_nt35596s_5p2_1080p_video"
#define DTS_COMP_JDI_ILI7807E_5P2 "jdi_ili7807e_5p2_1080p_video"

//PIC
#define	DTS_COMP_BOE_TD4300_5P0	"boe_td4300_5p0_1080P_video"
#define	DTS_COMP_CTC_TD4310_5P0	"ctc_td4310_5p0_1080P_video"
#define	DTS_COMP_LG_TD4300_5P0	"lg_td4300_5p0_1080P_video"
#define	DTS_COMP_TIANMA_TD4300_5P0	"tianma_td4300_5p0_1080P_video"

//BAC
#define DTS_COMP_AUO_NT36672_5P5	"auo_nt36672_5p5_1080p_video"
#define DTS_COMP_BOE_TD4310_5P5		"boe_td4310_5p5_1080p_video"
#define DTS_COMP_CTC_FT8716_5P5		"ctc_ft8716_5p5_1080p_video"
#define DTS_COMP_TIANMA_TD4310_5P5		"tianma_td4310_5p5_1080p_video"


#define RGB_SENSOR_CAL_FILE_PATH "/data/light"
#define RGB_SENSOR_CAL_RESULT_MAX_LEN  (96)
#define KNIGHT_BIEL_TPLCD (3)
#define KNIGHT_LENS_TPLCD (4)
#define BOE_TPLCD (5)
#define EBBG_TPLCD (6)
#define INX_TPLCD (7)
#define SAMSUNG_TPLCD (8)
#define SHARP_TPLCD (9)
#define BIEL_TPLCD (10)
#define VITAL_TPLCD (11)
#define TM_TPLCD (12)
#define AUO_TPLCD (13)
#define CTC_TPLCD (14)

#define SENSOR_MAX_RESET_TIME_MS		(400)
#define SENSOR_DETECT_AFTER_POWERON_TIME_MS		(50)

#define MAG_CURRENT_FAC_RAIO  (10000)
#define MAX_STORGE_TIME  (21)
#define MAG_CURRENT_STORAGE_MAX_SIZE   (50)
#define CURRENT_MAX_VALUE    (3000)
#define CURRENT_MIN_VALUE     (0)

static unsigned long sensor_jiffies;
static struct regulator *sensorhub_vddio;
uint32_t need_reset_io_power = 0;

#define FINGERSENSE_TRANS_TOUT  msecs_to_jiffies(100)   /* Wait 100ms for data transmitting */
#define FINGERSENSE_FRESH_TIME  msecs_to_jiffies(10)    /* We think the data is fresh if it is collected in 10ms */

spinlock_t	fsdata_lock;
bool fingersense_enabled;
bool rpc_motion_request;
bool fingersense_data_ready;
bool fingersense_data_intrans;        /* the data is on the way */
unsigned long fingersense_data_ts;    /* timestamp for the data */
s16 fingersense_data[FINGERSENSE_DATA_NSAMPLES] = { 0 };

extern volatile int vibrator_shake;
volatile int hall_value;

static int acc_close_after_calibrate = true;
static int als_close_after_calibrate = true;
static int gyro_close_after_calibrate = true;
char sensor_chip_info[SENSOR_MAX][MAX_CHIP_INFO_LEN];
static int gsensor_offset[15];	/*g-sensor calibrate data*/
static int gyro_sensor_offset[15];
static int ps_sensor_offset[3];
static int ps_switch_mode = 0;
struct ps_external_ir_param ps_external_ir_param = {
       .external_ir = 0,
       .internal_ir_min_proximity_value = 750,
       .external_ir_min_proximity_value = 850,
       .internal_ir_pwindows_value = 75,
       .external_ir_pwindows_value = 300,
       .internal_ir_pwave_value = 10,
       .external_ir_pwave_value = 55,
       .internal_ir_threshold_value = 35,
       .external_ir_threshold_value = 60,
       .external_ir_calibrate_noise = 30,
       .external_ir_enable_gpio = 67,
};
struct ps_extend_platform_data ps_extend_platform_data = {
	.external_ir_mode_flag = 0,
	.external_ir_avg_algo = 0,
	.external_ir_calibrate_noise_max = 100,
	.external_ir_calibrate_noise_min = 3,
	.external_ir_calibrate_far_threshold_max = 800,
	.external_ir_calibrate_far_threshold_min = 10,
	.external_ir_calibrate_near_threshold_max = 1500,
	.external_ir_calibrate_near_threshold_min = 20,
	.external_ir_calibrate_pwindows_max = 800,
	.external_ir_calibrate_pwindows_min = 3,
	.external_ir_calibrate_pwave_max = 1500,
	.external_ir_calibrate_pwave_min = 5,
	.min_proximity_value = 850,
	.pwindows_value = 5,
	.pwave_value = 20,
	.threshold_value = 20,
	.calibrate_noise = 30,
};
static uint16_t als_offset[6];
static uint8_t hp_offset[24];
static uint8_t cap_prox_calibrate_data[CAP_PROX_CALIDATA_NV_SIZE]={0};
static int cap_prox_calibrate_len = 0;
union sar_calibrate_data sar_calibrate_datas;
char *sar_calibrate_order;
static RET_TYPE return_calibration = EXEC_FAIL;	/*acc calibrate result*/
static RET_TYPE ps_calibration_res = EXEC_FAIL;	/*ps calibrate result*/
static RET_TYPE als_calibration_res = EXEC_FAIL;	/*als calibrate result*/
static RET_TYPE gyro_calibration_res = EXEC_FAIL;	/*gyro  calibrate result*/
static RET_TYPE handpress_calibration_res = EXEC_FAIL;/* handpress calibrate result*/
static RET_TYPE return_cap_prox_calibration = EXEC_FAIL;
struct sensor_status sensor_status;
int key_state;
int hifi_supported = 0;
static struct sensor_status sensor_status_backup;
extern int first_start_flag;
extern int ps_first_start_flag;
extern int txc_ps_flag;
extern int ams_tmd2620_ps_flag;
extern int avago_apds9110_ps_flag;
extern int ltr578_ps_external_ir_calibrate_flag;
extern int als_first_start_flag;
extern int gyro_first_start_flag;
extern int handpress_first_start_flag;
extern int rohm_rgb_flag;
extern int avago_rgb_flag;
static int ltr578_flag = 0;
static int apds9922_flag = 0;
static int tp_color_from_nv_flag = 0;
static int rohm_rpr531_flag = 0;
int tmd2745_flag = 0;
extern int is_cali_supported;
static char buf[MAX_PKT_LENGTH] = { 0 };
extern struct compass_platform_data mag_data;
extern struct airpress_platform_data airpress_data;
extern struct als_platform_data als_data;
extern struct cap_prox_platform_data cap_prox_data;
extern struct sar_platform_data sar_pdata;

pkt_sys_dynload_req_t *dyn_req = (pkt_sys_dynload_req_t *) buf;
uint16_t sensorlist[SENSOR_LIST_NUM] = { 0 };

static char str_charger[] = "charger_plug_in_out";
static char str_charger_current_in[] = "charger_plug_current_in";
static char str_charger_current_out[] = "charger_plug_current_out";
static int current_mag_x_pre = 0;
static int current_mag_y_pre = 0;
static int current_mag_z_pre = 0;
static int mag_current_send_time = 0;
static int mag_current_storage_time = 0;
static int mag_current_storage[MAG_CURRENT_STORAGE_MAX_SIZE] = { 0 };
extern int mag_opend;
extern int akm_current_x_fac;
extern int akm_current_y_fac;
extern int akm_current_z_fac;
#ifdef SENSOR_DSM_CONFIG
extern struct dsm_client *shb_dclient;
#endif
static uint8_t debug_read_data_buf[DEBUG_DATA_LENGTH] = { 0 };

#ifdef CONFIG_HUAWEI_CHARGER_SENSORHUB
extern struct notifier_block usb_nb;
extern void fsa9685_get_gpio_int(void);
extern int charge_usb_notifier_call_sh(struct notifier_block *usb_nb,
				    unsigned long event, void *data);
#endif
extern int get_airpress_data;
extern int get_temperature_data;
extern int iom3_timeout;
extern int g_iom3_state;
extern struct completion iom3_reboot;
extern int iom3_power_state;
u8 phone_color;
u8 tplcd_manufacture;
u8 tp_manufacture = TS_PANEL_UNKNOWN;
struct charge_current_mag  charge_current_data;

static unsigned char tp_color_buf[TP_COLOR_BUF_SIZE] = { 0 };

void read_tp_module_notify(struct notifier_block *nb,
			unsigned long action, void *data);

static struct notifier_block readtp_notify = {
	.notifier_call = read_tp_module_notify,
};

static struct notifier_block charger_notify = {
	.notifier_call = NULL,
};
extern void read_sensorlist_info(struct device_node *dn, int sensor);
extern int akm_cal_algo;
static int32_t ps_calib_data[3] = { 0 };
static int ps_calib_state;
static int32_t gyro_calib_data[15] = {0};
int als_para_table = 0;
struct sleeve_detect_pare sleeve_detect_paremeter[MAX_PHONE_COLOR_NUM] = {0};

/* Although the GRAY and Black TP's RGB ink is same ,but some product may has both the GRAY
and Black TP,so must set the als para for  GRAY and Black TP

Although the CAFE_2 and BROWN TP's RGB ink is same ,but some product may has both the CAFE_2
and BROWN TP,so must set the als para for  CAFE_2 and BROWN TP
*/
BH1745_ALS_PARA_TABLE als_para_diff_tp_color_table[] = {
	{NEXT, V3, LG_TPLCD, BLACK,
	 {159, 717, 466, 2587, 2708, 159, 449, 1548, 100, -2478, 7481, -3216,
	  5782, 973, 1544, 2491, 1899, 1241, 1712, 843, 171, 2659, 8122, 5000,
	  200} },
	{NEXT, V3, LG_TPLCD, WHITE,
	 {197, 154, 73, 538, 541, 197, 434, 1511, 100, -2479, 7196, -2699, 5569,
	  950, 1531, 1660, 1914, 4726, 6095, 2263, 630, 2003, 4953, 3200, 300} },
	{NEXT, V3, LG_TPLCD, GOLD,
	 {192, 544, 412, 1962, 1677, 192, 424, 1510, 100, -2510, 7022, -2715,
	  5348, 978, 1516, 1718, 1951, 1414, 1782, 646, 182, 2132, 4634, 3200,
	  300} },
	{NEXT, V3, JDI_TPLCD, BLACK,
	 {504, 70, 11, 515, 489, 504, 274, 1144, 100, -2161, 3152, -2780, 3429,
	  964, 1522, 2481, 2015, 15675, 10524, 14809, 1877, 3258, 4744, 3200,
	  300} },
	{NEXT, V3, JDI_TPLCD, WHITE,
	 {208, 227, 139, 802, 737, 208, 476, 1636, 100, -2531, 7886, -2834,
	  6160, 991, 1528, 1856, 1897, 3081, 3805, 1363, 399, 1298, 4713, 3200,
	  300} },
	{NEXT, V3, JDI_TPLCD, GOLD,
	 {469, 274, 102, 943, 463, 469, 355, 1428, 100, -2449, 4030, -2501,
	  4310, 818, 1359, 2023, 2038, 1529, 1934, 700, 195, 2313, 4360, 3200,
	  300} },
	{NEXT, V3_A, LG_TPLCD, GOLD,
	 {221, 5137, 2855, 25042, 24306, 221, 657, 2423, 100, -2491, 9912,
	  -3234, 8383, 1207, 1662, 3065, 1955, 89, 92, 23, 11, 1380, 4819, 3200,
	  300} },
	{NEXT, V3, LG_TPLCD, GRAY,
	 {345, 999, 741, 5871, 4998, 346, 718, 2890, 100, -2516, 10599, -3501,
	  9174, 1373, 1724, 4096, 1964, 1079, 868, 216, 104, 3086, 4670, 3200,
	  300} },

	{NEXT, V4, LG_TPLCD, BLACK,
	 {346, 998, 740, 5871, 4998, 346, 718, 2890, 100, -2516, 10599, -3501,
	  9174, 1373, 1724, 4096, 1964, 1079, 868, 216, 104, 3086, 4670, 3200,
	  300} },
	{NEXT, V4, LG_TPLCD, GRAY,
	 {346, 998, 740, 5871, 4998, 346, 718, 2890, 100, -2516, 10599, -3501,
	  9174, 1373, 1724, 4096, 1964, 1079, 868, 216, 104, 3086, 4670, 3200,
	  300} },
	{NEXT, V4, LG_TPLCD, WHITE,
	 {191, 214, 147, 884, 780, 191, 459, 1633, 100, -2499, 7280, -2905,
	  5962, 1060, 1556, 2104, 1920, 5144, 6115, 2364, 630, 3253, 5223, 3200,
	  300} },
	{NEXT, V4, LG_TPLCD, GOLD,
	 {174, 806, 488, 3605, 3250, 174, 471, 1811, 100, -2374, 7027, -2835,
	  5795, 1038, 1624, 2113, 1982, 1235, 1401, 496, 144, 3023, 4732, 3200,
	  300} },
	{NEXT, V4, JDI_TPLCD, BLACK,
	 {334, 873, 660, 4844, 4071, 334, 719, 2815, 100, -2463, 10281, -3452,
	  9226, 1283, 1720, 3997, 1949, 1233, 1111, 268, 135, 3329, 4665, 3200,
	  300} },
	{NEXT, V4, JDI_TPLCD, GRAY,
	 {334, 873, 660, 4844, 4071, 334, 719, 2815, 100, -2463, 10281, -3452,
	  9226, 1283, 1720, 3997, 1949, 1233, 1111, 268, 135, 3329, 4665, 3200,
	  300} },
	{NEXT, V4, JDI_TPLCD, WHITE,
	 {199, 216, 128, 921, 871, 199, 482, 1724, 100, -2459, 7487, -2964,
	  6288, 1062, 1587, 2237, 1918, 4659, 5417, 2005, 564, 2997, 5221, 3200,
	  300} },
	{NEXT, V4, JDI_TPLCD, GOLD,
	 {195, 540, 318, 1966, 1954, 195, 282, 1569, 100, -2425, 6844, -1342,
	  3208, 944, 1551, 649, 2162, 1529, 1934, 700, 195, 2313, 4360, 3200,
	  300} },

	{NEXT, VN1, LG_TPLCD, BLACK,
	 {346, 998, 740, 5871, 4998, 346, 718, 2890, 100, -2516, 10599, -3501,
	  9174, 1373, 1724, 4096, 1964, 1079, 868, 216, 104, 3086, 4670, 3200,
	  300} },
	{NEXT, VN1, LG_TPLCD, GRAY,
	 {346, 998, 740, 5871, 4998, 346, 718, 2890, 100, -2516, 10599, -3501,
	  9174, 1373, 1724, 4096, 1964, 1079, 868, 216, 104, 3086, 4670, 3200,
	  300} },
	{NEXT, VN1, LG_TPLCD, WHITE,
	 {190, 197, 142, 785, 709, 190, 440, 1459, 100, -2491, 7087, -2939,
	  5807, 1029, 1545, 1994, 1886, 5042, 6508, 2514, 673, 2700, 4500, 3200,
	  300} },
	{NEXT, VN1, LG_TPLCD, GOLD,
	 {176, 554, 478, 2317, 1704, 176, 454, 1618, 100, -2455, 7108, -2944,
	  5779, 1041, 1573, 2131, 1959, 1986, 2450, 863, 252, 3388, 4782, 3200,
	  300} },
	{NEXT, VN1, LG_TPLCD, BROWN,
	 {196, 818, 662, 4091, 3008, 196, 452, 1770, 100, -2464, 7189, -2793,
	  5710, 1161, 1617, 2070, 1987, 1611, 1570, 591, 179, 2977, 4715, 3200,
	  300} },
	{NEXT, VN1, LG_TPLCD, CAFE_2,
	 {196, 629, 509, 3147, 2314, 196, 452, 1770, 100, -2464, 7189, -2793,
	  5710, 1161, 1617, 2070, 1987, 1611, 1570, 591, 179, 2977, 4715, 3200,
	  300} },
	{NEXT, VN1, LG_TPLCD, PINK,
	 {197, 636, 481, 3154, 2528, 197, 453, 1685, 100, -2396, 6669, -3030,
	  5773, 1104, 1628, 2418, 1958, 1722, 1792, 695, 198, 2700, 4500, 3200,
	  300} },
	{NEXT, VN1, JDI_TPLCD, BLACK,
	 {334, 873, 660, 4844, 4071, 334, 719, 2815, 100, -2463, 10281, -3452,
	  9226, 1283, 1720, 3997, 1949, 1233, 1111, 268, 135, 3329, 4665, 3200,
	  300} },
	{NEXT, VN1, JDI_TPLCD, GRAY,
	 {334, 873, 660, 4844, 4071, 334, 719, 2815, 100, -2463, 10281, -3452,
	  9226, 1283, 1720, 3997, 1949, 1233, 1111, 268, 135, 3329, 4665, 3200,
	  300} },
	{NEXT, VN1, JDI_TPLCD, WHITE,
	 {190, 197, 142, 785, 709, 190, 440, 1459, 100, -2491, 7087, -2939,
	  5807, 1029, 1545, 1994, 1886, 5042, 6508, 2514, 673, 2700, 4500, 3200,
	  300} },
	{NEXT, VN1, JDI_TPLCD, GOLD,
	 {176, 554, 478, 2317, 1704, 176, 454, 1618, 100, -2455, 7108, -2944,
	  5779, 1041, 1573, 2131, 1959, 1986, 2450, 863, 252, 3388, 4782, 3200,
	  300} },
	{NEXT, VN1, JDI_TPLCD, BROWN,
	 {196, 818, 662, 4091, 3008, 196, 452, 1770, 100, -2464, 7189, -2793,
	  5710, 1161, 1617, 2070, 1987, 1611, 1570, 591, 179, 2977, 4715, 3200,
	  300} },
	{NEXT, VN1, JDI_TPLCD, CAFE_2,
	 {196, 629, 509, 3147, 2314, 196, 452, 1770, 100, -2464, 7189, -2793,
	  5710, 1161, 1617, 2070, 1987, 1611, 1570, 591, 179, 2977, 4715, 3200,
	  300} },
	{NEXT, VN1, JDI_TPLCD, PINK,
	 {197, 636, 481, 3154, 2528, 197, 453, 1685, 100, -2396, 6669, -3030,
	  5773, 1104, 1628, 2418, 1958, 1722, 1792, 695, 198, 2700, 4500, 3200,
	  300} },

	{KNIGHT, V3, KNIGHT_BIEL_TPLCD, BLACK,
	 {537, 1346, 453, 9524, 8788, 537, 1861, 5575, 100, -2596, 14985, -4342,
	  16974, 1643, 1810, 10712, 2054, 775, 534, 87, 72, 3064, 4605, 5000,
	  200} },
	{KNIGHT, V3, KNIGHT_BIEL_TPLCD, GRAY,
	 {346, 998, 740, 5871, 4998, 346, 718, 2890, 100, -2516, 10599, -3501,
	  9174, 1373, 1724, 4096, 1964, 1079, 868, 216, 104, 3086, 4670, 30000,
	  0} },
	{KNIGHT, V3, KNIGHT_BIEL_TPLCD, WHITE,
	 {239, 232, 114, 1018, 951, 239, 732, 1909, 100, -2352, 7428, -2784,
	  7079, 1024, 1641, 2113, 1954, 3795, 4551, 1597, 484, 2756, 5410, 5000,
	  200} },
	{KNIGHT, V3, KNIGHT_BIEL_TPLCD, GOLD,
	 {255, 598, 251, 3511, 3655, 255, 1033, 3087, 100, -2355, 9517, -3342,
	  9610, 1235, 1771, 3732, 2030, 1792, 1681, 440, 202, 3488, 5161, 5000,
	  200} },
	{KNIGHT, V3, KNIGHT_LENS_TPLCD, BLACK,
	 {768, 791, 103, 4815, 4313, 768, 755, 2575, 100, -2300, 5291, -3106,
	  6564, 1124, 1667, 3882, 2136, 1422, 1138, 677, 162, 3302, 5041, 5000,
	  200} },
	{KNIGHT, V3, KNIGHT_LENS_TPLCD, GRAY,
	 {334, 873, 660, 4844, 4071, 334, 719, 2815, 100, -2463, 10281, -3452,
	  9226, 1283, 1720, 3997, 1949, 1233, 1111, 268, 135, 3329, 4665, 30000,
	  0} },
	{KNIGHT, V3, KNIGHT_LENS_TPLCD, WHITE,
	 {251, 235, 94, 1031, 1029, 251, 739, 1922, 100, -2368, 7310, -2678,
	  7103, 1031, 1631, 2026, 1964, 4484, 5348, 1782, 566, 3283, 5006, 5000,
	  200} },
	{KNIGHT, V3, KNIGHT_LENS_TPLCD, GOLD,
	 {215, 647, 182, 3087, 3532, 215, 819, 2292, 100, -2337, 8176, -2935,
	  7767, 1076, 1688, 2429, 1995, 1602, 1795, 496, 196, 3288, 4783, 5000,
	  200} },

	{KNIGHT, V3, DEFAULT_TPLCD, BLACK,
	 {768, 791, 103, 4815, 4313, 768, 755, 2575, 100, -2300, 5291, -3106,
	  6564, 1124, 1667, 3882, 2136, 1422, 1138, 677, 162, 3302, 5041, 5000,
	  200} },
	{KNIGHT, V3, DEFAULT_TPLCD, GRAY,
	 {346, 998, 740, 5871, 4998, 346, 718, 2890, 100, -2516, 10599, -3501,
	  9174, 1373, 1724, 4096, 1964, 1079, 868, 216, 104, 3086, 4670, 30000,
	  0} },
	{KNIGHT, V3, DEFAULT_TPLCD, WHITE,
	 {251, 235, 94, 1031, 1029, 251, 739, 1922, 100, -2368, 7310, -2678,
	  7103, 1031, 1631, 2026, 1964, 4484, 5348, 1782, 566, 3283, 5006, 5000,
	  200} },
	{KNIGHT, V3, DEFAULT_TPLCD, GOLD,
	 {215, 647, 182, 3087, 3532, 215, 819, 2292, 100, -2337, 8176, -2935,
	  7767, 1076, 1688, 2429, 1995, 1602, 1795, 496, 196, 3288, 4783, 5000,
	  200} },

	{EVA, V4, DEFAULT_TPLCD, BLACK,
	 {218, 471, 375, 2463, 1437, 218, 702, 2658, 100, -2542, 10285, -3356,
	  8880, 1298, 1668, 3618, 1989, 1556, 1489, 488, 171, 2200, 6500, 5000,
	  200} },
	{EVA, V4, DEFAULT_TPLCD, GRAY,
	 {218, 471, 375, 2463, 1437, 218, 702, 2658, 100, -2542, 10285, -3356,
	  8880, 1298, 1668, 3618, 1989, 1556, 1489, 488, 171, 2200, 6500, 5000,
	  200} },
	{EVA, V4, DEFAULT_TPLCD, WHITE,
	 {188, 186, 110, 759, 685, 188, 487, 1720, 100, -2438, 7438, -2954,
	  6294, 1025, 1586, 2165, 1929, 4631, 5879, 2687, 609, 2661, 6800, 5000,
	  200} },
	{EVA, V4, DEFAULT_TPLCD, GOLD,
	 {169, 635, 426, 3053, 2321, 169, 512, 2101, 100, -2470, 7841, -3120,
	  6352, 1153, 1623, 2737, 2013, 1107, 1180, 440, 128, 2150, 5500, 5000,
	  200} },
	{EVA, V4, DEFAULT_TPLCD, PINK,
	 {271, 183, 77, 1103, 1187, 271, 1090, 4239, 100, -2613, 15929, -3946,
	  13621, 1524, 1749, 6155, 1989, 4990, 4000, 790, 512, 2660, 6050, 5000,
	  200} },

	{FARADAY, V3, JDI_TPLCD, BLACK,
	 {328, 816, 134, 4004, 4681, 328, 820, 2310, 100, -2305, 7758, -3074,
	 7675, 1068, 1701, 2879, 2013, 1186, 1212, 500,141, 2908, 6486, 5000,
	 200  } },
	{FARADAY, V3, JDI_TPLCD, WHITE,
	 {199, 209, 112, 836, 780, 199, 418, 1533, 100, -2366, 6408, -2769,
	  5968, 958, 1581, 1817, 1918, 6294, 8165, 4199, 894, 4071, 6545, 5000,
	  200} },
	{FARADAY, V3, JDI_TPLCD, GOLD,
	 {182, 1000, 470,  4204, 3912, 182, 571, 1526, 100, -2251, 5937, -2432,
	 5392, 924, 1630, 1474, 1970, 994, 1245, 584,129, 3113, 5568, 5000,
	 200 } },
	{FARADAY, V3, JDI_TPLCD, BLUE,
	 {300, 1142, 530, 3003, 2105, 300, 202, 448, 100, -2728, 2824, -1851,
	  2219, 624, 983, 754, 1739, 874, 2109, 4626, 278, 3665, 5472, 5000,
	  200  }},
	{FARADAY, V3, JDI_TPLCD, PINK,
	 {384, 147, 40, 1308, 1386, 384, 1395, 3745, 100, -2626, 12339, -4178 ,
	 13500, 1874, 1872, 8708, 1958, 8474, 4626, 1335, 757, 3647, 5860,5000,
	 200 } },

	{FARADAY, V3, LG_TPLCD, BLACK,
	 {232, 1548, 458, 7252, 7348, 232, 747, 2147, 100, -2289, 6993, -2835,
	  6991, 1020, 1675, 2315, 2021, 596, 657, 297, 75, 2839, 6421, 5000,
	  200} },
	{FARADAY, V3, LG_TPLCD, WHITE,
	 {210, 213, 101, 851, 824, 210, 583, 1475, 100, -2275, 6155, -2597,
	  5800, 914, 1616, 1603, 1914, 6221, 8029, 4363, 875, 4077, 6841, 5000,
	  200} },
	{FARADAY, V3, LG_TPLCD, GOLD,
	 {190, 810, 332,  3333, 3385, 190, 680, 1793, 100, -2307, 6971, -2692,
	 6577, 959, 1634, 1810, 1957, 1392, 1731, 700,176, 3450, 5853, 5000,
	 200 } },
	{FARADAY, V3, LG_TPLCD, BLUE,
	 {274, 3027, 1506, 3164, 2207, 274, 175, 292, 100, -5065, 3679, 22,
	  2907, 5974, 595, 3114, 1010, 319, 2143, 4138, 227, 3871, 5977, 5000,
	  200} },
	{FARADAY, V3, LG_TPLCD, PINK,
	 {412, 122, 10, 1113, 1378, 412, 2571, 7214, 100, -2760, 21788, -5175,
	  24630, 2151, 1900, 21478, 1985, 10475, 5337, 869, 931, 3608, 5920,
	  5000, 200} },

	{FARADAY, V3, SHARP_TPLCD, BLACK,
	 {232, 1548, 458, 7252, 7348, 232, 747, 2147, 100, -2289, 6993, -2835,
	  6991, 1020, 1675, 2315, 2021, 596, 657, 297, 75, 2839, 6421, 5000,
	  200} },
	{FARADAY, V3, SHARP_TPLCD, WHITE,
	 {210, 213, 101, 851, 824, 210, 583, 1475, 100, -2275, 6155, -2597,
	  5800, 914, 1616, 1603, 1914, 6221, 8029, 4363, 875, 4077, 6841, 5000,
	  200} },
	{FARADAY, V3, SHARP_TPLCD, GOLD,
	 {190, 810, 332,  3333, 3385, 190, 680, 1793, 100, -2307, 6971, -2692,
	 6577, 959, 1634, 1810, 1957, 1392, 1731, 700,176, 3450, 5853, 5000,
	 200 } },
	{FARADAY, V3, SHARP_TPLCD, BLUE,
	 {274, 3027, 1506, 3164, 2207, 274, 175, 292, 100, -5065, 3679, 22,
	  2907, 5974, 595, 3114, 1010, 319, 2143, 4138, 227, 3871, 5977, 5000,
	  200} },
	{FARADAY, V3, SHARP_TPLCD, PINK,
	 {412, 122, 10, 1113, 1378, 412, 2571, 7214, 100, -2760, 21788, -5175,
	  24630, 2151, 1900, 21478, 1985, 10475, 5337, 869, 931, 3608, 5920,
	  5000, 200} },

	{FARADAY, V3, DEFAULT_TPLCD, BLACK,
	 {328, 816, 134, 4004, 4681, 328, 820, 2310, 100, -2305, 7758, -3074,
	 7675, 1068, 1701, 2879, 2013, 1186, 1212, 500,141, 2908, 6486, 30000,
	 0  } },
	{FARADAY, V3, DEFAULT_TPLCD, WHITE,
	 {199, 209, 112, 836, 780, 199, 418, 1533, 100, -2366, 6408, -2769,
	  5968, 958, 1581, 1817, 1918, 6294, 8165, 4199,894, 4071, 6545, 30000,
	  0} },
	{FARADAY, V3, DEFAULT_TPLCD, GOLD,
	 {182, 1000, 470,  4204, 3912, 182, 571, 1526, 100, -2251, 5937, -2432,
	 5392, 924, 1630, 1474, 1970, 994, 1245, 584,129, 3113, 5568, 30000,
	 0 } },
	{FARADAY, V3, DEFAULT_TPLCD, BLUE,
	 {300, 1142, 530, 3003, 2105, 300, 202, 448, 100, -2728, 2824, -1851,
	  2219, 624, 983, 754, 1739, 874, 2109, 4626, 278, 3665, 5472, 30000,
	  0} },
	{FARADAY, V3, DEFAULT_TPLCD, PINK,
	 {384, 147, 40, 1308, 1386, 384, 1395, 3745, 100, -2626, 12339, -4178 ,
	 13500, 1874, 1872, 8708, 1958, 8474, 4626, 1335, 757, 3647, 5860,30000,
	 0 } },

	{VIENNA, V4, DEFAULT_TPLCD, BLACK,
	 {268, 436, 230, 2010, 2214, 268, 619, 2429, 100, -2392, 8332, -3290,
	  7805, 1089, 1658, 3210, 1960, 1188, 1250, 461, 141, 1515, 6202, 5000,
	  200} },
	{VIENNA, V4, DEFAULT_TPLCD, GRAY,
	 {268, 436, 230, 2010, 2214, 268, 619, 2429, 100, -2392, 8332, -3290,
	  7805, 1089, 1658, 3210, 1960, 1188, 1250, 461, 141, 1515, 6202, 5000,
	  200} },
	{VIENNA, V4, DEFAULT_TPLCD, WHITE,
	 {210, 196, 107, 827, 792, 210, 543, 1930, 100, -2408, 8025, -3020,
	  6940, 1041, 1623, 2296, 1921, 2687, 3296, 1344, 350, 2150, 7060, 5000,
	  200} },
	{VIENNA, V4, DEFAULT_TPLCD, GOLD,
	 {186, 607, 406, 2561, 2884, 186, 487, 1837, 100, -2377, 6912, -2906,
	  6097, 1004, 1605, 2286, 1981, 986, 1200, 478, 128, 1833,5360,5000,
	  200} },

	{VIENNA, VN1, DEFAULT_TPLCD, BLACK,
	 {428, 639, 465, 3403, 6150, 428, 774, 2808, 100, -2433, 8823, -3284,
	  9514, 1215, 1694, 3726, 2015, 568, 564, 206, 69, 1140, 6500, 5000,
	  200} },
	{VIENNA, VN1, DEFAULT_TPLCD, GRAY,
	 {428, 639, 465, 3403, 6150, 428, 774, 2808, 100, -2433, 8823, -3284,
	  9514, 1215, 1694, 3726, 2015, 568, 564, 206, 69, 1140, 6500, 5000,
	  200} },
	{VIENNA, VN1, DEFAULT_TPLCD, WHITE,
	 {216, 271, 118, 1130, 1120, 216, 588, 2199, 100, -2431, 8630, -3066,
	  7406, 1055, 1623, 2616, 1970, 1955, 2336, 951, 262, 1580, 6500, 5000,
	  200} },
	{VIENNA, VN1, DEFAULT_TPLCD, GOLD,
	 {176, 858, 548, 3365, 3087, 176, 430, 1605, 100, -2422, 6700, -2816,
	  5395, 980, 1561, 1906, 1972, 477, 643, 309, 69, 1032, 6000, 5000,
	  200} },

	{VIENNA, VN2, DEFAULT_TPLCD, BLACK,
	 {331, 534, 370, 3065, 2869, 331, 741, 2898, 100, -2440, 10176, -3457,
	  9122, 1290, 1739, 4088, 2007, 880, 780, 257, 104, 1420, 6800, 5000,
	  200}},
	{VIENNA, VN2, DEFAULT_TPLCD, GRAY,
	 {331, 534, 370, 3065, 2869, 331, 741, 2898, 100, -2440, 10176, -3457,
	  9122, 1290, 1739, 4088, 2007, 880, 780, 257, 104, 1420, 6800, 5000,
	  200}},
	{VIENNA, VN2, DEFAULT_TPLCD, WHITE,
	 {216, 271, 118, 1130, 1120, 216, 588, 2199, 100, -2431, 8630, -3066,
	  7406, 1055, 1623, 2616, 1970, 1955, 2336, 951, 262, 1580, 6500, 5000,
	  200}},
	{VIENNA, VN2, DEFAULT_TPLCD, GOLD,
	 {176, 858, 548, 3365, 3087, 176, 430, 1605, 100, -2422, 6700, -2816,
	  5395, 980, 1561, 1906, 1972, 477, 643, 309, 69, 1032,6000,5000,
	  200}},

	{VNS,V4,BOE_TPLCD,GOLD,
	 {198, 551, 442, 2570, 2210, 271, 1090, 4239, 100, -2613, 15929, -3946,
	  13621, 1524, 1749, 6155, 1989, 4990, 4000, 790, 512, 2660, 6050, 5000,
	  200} },
	{VNS,V4,BOE_TPLCD,WHITE,
	 {224, 225, 139, 1029, 1049, 271, 1090, 4239, 100, -2613, 15929, -3946,
	  13621, 1524, 1749, 6155, 1989, 4990, 4000, 790, 512, 2660, 6050, 5000,
	  200} },
	{VNS,V4,BOE_TPLCD,BLACK,
	 {567, 792, 227, 3205, 2031, 249, 451, 1650, 100, -2512, 7591, -2862,
	  5730, 1022, 1542, 1866, 1934, 5507, 3392, 4859, 613, 1154, 4831, 3200,
	  300} },
	{VNS,V4,JDI_TPLCD,GOLD,
	 {198, 551, 442, 2570, 2210, 271, 1090, 4239, 100, -2613, 15929, -3946,
	  13621, 1524, 1749, 6155, 1989, 4990, 4000, 790, 512, 2660, 6050, 5000,
	  200} },
	{VNS,V4,JDI_TPLCD,WHITE,
	 {224, 225, 139, 1029, 1049, 271, 1090, 4239, 100, -2613, 15929, -3946,
	  13621, 1524, 1749, 6155, 1989, 4990, 4000, 790, 512, 2660, 6050, 5000,
	  200} },
	{VNS,V4,JDI_TPLCD,BLACK,
	 {567, 792, 227, 3205, 2031, 249, 451, 1650, 100, -2512, 7591, -2862,
	  5730, 1022, 1542, 1866, 1934, 5507, 3392, 4859, 613, 1154, 4831, 3200,
	  300} },
	{VNS,V4,LG_TPLCD,GOLD,
	 {198, 551, 442, 2570, 2210, 271, 1090, 4239, 100, -2613, 15929, -3946,
	  13621, 1524, 1749, 6155, 1989, 4990, 4000, 790, 512, 2660, 6050, 5000,
	  200} },
	{VNS,V4,LG_TPLCD,WHITE,
	 {224, 225, 139, 1029, 1049, 271, 1090, 4239, 100, -2613, 15929, -3946,
	  13621, 1524, 1749, 6155, 1989, 4990, 4000, 790, 512, 2660, 6050, 5000,
	  200} },
	{VNS,V4,LG_TPLCD,BLACK,
	 {567, 792, 227, 3205, 2031, 249, 451, 1650, 100, -2512, 7591, -2862,
	  5730, 1022, 1542, 1866, 1934, 5507, 3392, 4859, 613, 1154, 4831, 3200,
	  300} },
	{VNS, V4, INX_TPLCD, GOLD,
	{196, 700, 440, 3342, 3138, 271, 1090, 4239, 100, -2613, 15929, -3946,
	 13621, 1524, 1749, 6155, 1989, 4990, 4000, 790, 512, 2660, 6050, 5000,
	 200} },

	{VNS,V4,INX_TPLCD,WHITE,
	 {236, 293, 161, 1401, 1410, 271, 1090, 4239, 100, -2613, 15929, -3946,
	  13621, 1524, 1749, 6155, 1989, 4990, 4000, 790,512, 2660, 6050, 5000,
	  200} },

	{VNS, V4, INX_TPLCD, BLACK,
	 {671, 610, 229, 3121, 1403, 249, 451, 1650, 100, -2512, 7591, -2862,
	  5730, 1022, 1542, 1866, 1934, 5507, 3392, 4859, 613, 1154, 4831, 3200,
	  300} },
	{VNS, V4, EBBG_TPLCD, GOLD,
	 {196, 700, 440, 3342, 3138, 271, 1090, 4239, 100, -2613, 15929, -3946,
	  13621, 1524, 1749, 6155, 1989, 4990, 4000, 790, 512, 2660, 6050, 5000,
	  200} },
	{VNS,V4,EBBG_TPLCD,WHITE,
	 {236, 293, 161, 1401, 1410, 271, 1090, 4239, 100, -2613, 15929, -3946,
	 13621, 1524, 1749, 6155, 1989, 4990, 4000, 790,512, 2660, 6050, 5000,
	 200}},
	{VNS, V4, EBBG_TPLCD, BLACK,
	 {671, 610, 229, 3121, 1403, 249, 451, 1650, 100, -2512, 7591, -2862,
	  5730, 1022, 1542, 1866, 1934, 5507, 3392, 4859, 613, 1154, 4831, 3200,
	  300} },

	{EDISON, V3, SAMSUNG_TPLCD, BLACK,
	 {250, 604, 439, 2748, 2166, 250, 579, 2266, 100, -2608, 9256, -3304,
	  7620, 1222, 1581, 3312, 1999, 1517, 1550, 610, 182, 2587, 7218, 3200,
	  300} },
	{EDISON, V3, SAMSUNG_TPLCD, WHITE,
	 {187, 242, 140, 950, 870, 187, 524, 1795, 100, -2614, 8477, -3321,
	  7214, 1110, 1526, 2745, 1891, 5536, 7046, 3356, 750, 4102, 9094, 3200,
	  300} },
	{EDISON, V3, SAMSUNG_TPLCD, GOLD,
	 {163, 686, 425, 2569, 2236, 163, 484, 1738, 100, -2618, 7947, -3108,
	  6514, 1068, 1501, 2338, 1944, 1586, 2137, 892, 219, 3288, 6804, 3200,
	  300} },

	{BTV, V3, DEFAULT_TPLCD, BLACK,
	 {180, 281, 169, 1220, 1143, 180, 586, 2134, 100, -2612, 9271, -3261,
	  7879, 1191, 1568, 2883, 1944, 3943, 4403, 1798, 513, 3239, 8109, 5000,
	  200} },

	{BTV, V3, DEFAULT_TPLCD, WHITE,
         {180, 281, 169, 1220, 1143, 180, 586, 2134, 100, -2612, 9271, -3261,
          7879, 1191, 1568, 2883, 1944, 3943, 4403, 1798, 513, 3239, 8109, 5000,
          200} },

	{HAYDN, V3, DEFAULT_TPLCD, WHITE,
	 {171, 223, 134, 803, 788, 171, 285, 589, 100, -4269, 5377, -3052,
	  5306, 2466, 1227, 2116, 1229, 4741, 6598, 3360, 683, 4103, 7330, 3200,
	  300} },
    {HANDEL, V3, DEFAULT_TPLCD, WHITE,
        {207, 481, 212, 1786, 1780, 207, 336, 673, 100, -3310, 6364, -3364,
         6256, 1818, 1232, 2833, 1239, 3507, 3663, 1390, 441, 4407, 6192,6000,
         200} },
	{NATASHA, V3, DEFAULT_TPLCD, BLACK,
	 {194, 354, 136, 1060, 1140, 194, 412, 1370, 100, -2713, 7576, -2787,
	  5320, 963, 1418, 1598, 1896, 11495, 8100, 1968, 1143, 3163, 2480, 4500,
	  200} },
	{NATASHA, V3, DEFAULT_TPLCD, GRAY,
	 {194, 354, 136, 1060, 1140, 194, 412, 1370, 100, -2713, 7576, -2787,
	  5320, 963, 1418, 1598, 1896, 11495, 8100, 1968, 1143, 3163, 2480, 4500,
	  200} },
	{NATASHA, VN1, DEFAULT_TPLCD, BLACK,
	 {262, 815, 629, 4136, 3710, 262, 498, 1505, 100, -2549, 7155, -3497,
	  6012, 1223, 1582, 3356, 1987, 1111, 1488, 715, 136, 3529, 7406, 4500,
	  200} },
	{NATASHA, VN1, DEFAULT_TPLCD, GRAY,
	 {262, 815, 629, 4136, 3710, 262, 498, 1505, 100, -2549, 7155, -3497,
	  6012, 1223, 1582, 3356, 1987, 1111, 1488, 715, 136, 3529, 7406, 4500,
	  200} },
	{CHICAGO, V1, DEFAULT_TPLCD, GOLD,
	 {159, 717, 466, 2587, 2708, 159, 449, 1548, 100, -2478, 7481, -3216,
	  5782, 973, 1544, 2491, 1899, 1241, 1712, 843, 171, 2659, 8122, 5000,
	  200} },
	{CHICAGO, V3, DEFAULT_TPLCD, BLACK,
	 {341, 894, 749, 4847, 3689, 341, 745, 2953, 100, -2528, 11134, -3420,
	  9328, 1325, 1698, 3619, 1964, 673, 695, 219, 81, 1985, 7892, 5000,
	  200} },
	{CHICAGO, V3, DEFAULT_TPLCD, GRAY,
	 {341, 894, 749, 4847, 3689, 341, 745, 2953, 100, -2528, 11134, -3420,
	  9328, 1325, 1698, 3619, 1964, 673, 695, 219, 81, 1985, 7892, 5000,
	  200} },
	{CHICAGO, V3, DEFAULT_TPLCD, WHITE,
	 {183, 248, 162, 977, 875, 183, 474, 1645, 100, -2470, 7568, -2887,
	  6064, 1021, 1567, 1964, 1915, 3093, 4541, 2156, 456, 2601, 8291, 5000,
	  200} },
	{CHICAGO, V3, DEFAULT_TPLCD, GOLD,
	 {169, 756, 585, 3117, 2452, 169, 428, 1549, 100, -2443, 6839, -2719,
	  5383, 1021, 1567, 1853, 1953, 867, 1181, 572, 116, 2168, 6812, 5000,
	  200} },
	{CHICAGO, V3, DEFAULT_TPLCD, CAFE_2,
	 {250, 693, 494, 3528, 2992, 250, 529, 2062, 100, -2448, 7776, -3229,
	  6547, 1177, 1648, 3055, 1988, 835, 909, 424, 101, 1892, 7536, 5000,
	  200} },
	{CHICAGO, V3, DEFAULT_TPLCD, BROWN,
	 {250, 693, 494, 3528, 2992, 250, 529, 2062, 100, -2448, 7776, -3229,
	  6547, 1177, 1648, 3055, 1988, 835, 909, 424, 101, 1892, 7536, 5000,
	  200} },
	{CHICAGO, V4, DEFAULT_TPLCD, BLACK,
	 {1687, 1508, 258, 13889, 11664, 1687, 771, 2595, 100, -2626, 6028, -5137,
	  8841, 1726, 1709, 24659, 2053, 715, 404, 343, 84, 3344, 7029, 5000,
	  200} },
	{CHICAGO, V4, DEFAULT_TPLCD, GRAY,
	 {1687, 1508, 258, 13889, 11664, 1687, 771, 2595, 100, -2626, 6028, -5137,
	  8841, 1726, 1709, 24659, 2053, 715, 404, 343, 84, 3344, 7029, 5000,
	  200} },
	{CHICAGO, V4, DEFAULT_TPLCD, WHITE,
	 {206, 140, 181, 772, 644, 206, 516, 1193, 100, -2399, 7452, -4204,
	  6217, 1193, 1686, 4293, 1959, 3879, 5235, 2572, 513, 3344, 6500, 5000,
	  200} },
	{CHICAGO, V4, DEFAULT_TPLCD, GOLD,
	 {199, 592, 451, 3573, 3675, 199, 597, 1933, 100, -2546, 9049, -3568,
	  6972, 1395, 1693, 3597, 2009, 1029, 1246, 533, 124, 3344, 6500, 5000,
	  150} },
	{CHICAGO, V4, DEFAULT_TPLCD, CAFE_2,
	 {265, 729, 649, 5359, 4587, 265, 544, 1807, 100, -2272, 6810, -3520,
	  6325, 1269, 1821, 3773, 2034, 921, 974, 501, 108, 3344, 6500, 5000,
	  150} },
	{CHICAGO, V4, DEFAULT_TPLCD, BROWN,
	 {265, 729, 649, 5359, 4587, 265, 544, 1807, 100, -2272, 6810, -3520,
	  6325, 1269, 1821, 3773, 2034, 921, 974, 501, 108, 3344, 6500, 5000,
	  150} },

	{DUKE, V3, SHARP_TPLCD, BLACK,
	 {504, 1826, 236, 11410, 10313, 504, 1390, 4538, 100, -2646, 9370, -3522,
	  11918, 1516, 1672, 5723, 2135, 400, 334, 104, 40, 2200, 5800, 30000,
	  0} },
	{DUKE, V3, SHARP_TPLCD, BLUE,
	 {438, 4079, 764, 4506, 3207, 438, 183, 404, 100, -4988, 3572, -1371,
	  1865, 588, 566, 558, 1832, 172, 1214, 2102, 117, 2500, 5800, 5000,
	  200} },
	{DUKE, V3, SHARP_TPLCD, WHITE,
	 {317, 320, 183, 1427, 1289, 217, 663, 1724, 100, -2273, 6050, -2606,
	  6240, 966, 1636, 1865, 1969, 2462, 3229, 1549, 313, 2700, 5970, 5000,
	  200} },

	{DUKE, V3, JDI_TPLCD, BLACK,
	 {438, 1903, 497, 8756, 7857, 438, 911, 2893, 100, -2593, 8247, -2593,
	  7866, 1205, 1568, 2287, 2130, 353, 410, 152, 43, 2200, 5800, 30000,
	  0} },
	{DUKE, V3, JDI_TPLCD, BLUE,
	 {474, 1586, 212, 2890, 2339, 474, 199, 444, 100, -3699, 3511, -1664,
	  2236, 669, 746, 634, 1685, 563, 1906, 2988, 183, 2200, 5800, 5000,
	  200} },
	{DUKE, V3, JDI_TPLCD, WHITE,
	 {203, 253, 128, 1089, 994, 203, 681, 1757, 100, -2319, 6504, -2567,
	  6431, 962, 1616, 1785, 1968, 3255, 4467, 1968, 412, 2850, 6050, 5000,
	  200} },

	{DUKE, V3, DEFAULT_TPLCD, BLACK,
	 {492, 1521, 231, 9943, 8735, 492, 1340, 4338, 100, -2361, 8939, -3437,
	  11520, 1303, 1796, 5333, 2122, 432, 368, 121, 44, 2160, 5800, 30000,
	  0} },
	{DUKE, V3, DEFAULT_TPLCD, BLUE,
	 {384, 3044, 1190, 3362, 2917, 384, 178, 315, 100, -4943, 3587, -2395,
	  2526, 591, 582, 577, 1281, 203, 1484, 2524, 136, 3000, 4000, 30000,
	  0} },
	{DUKE, V3, DEFAULT_TPLCD, WHITE,
	 {317, 320, 183, 1427, 1289, 217, 663, 1724, 100, -2273, 6050, -2606,
	  6240, 966, 1636, 1865, 1969, 2462, 3229, 1549, 313, 2700, 5970, 30000,
	  0} },

	{CMB, V3, BIEL_TPLCD, BLACK,
	 {341, 894, 749, 4847, 3689, 341, 745, 2953, 100, -2528, 11134, -3420,
	  9328, 1325, 1698, 3619, 1964, 730, 661, 231, 78, 1928, 8290, 30000,
	  0} },
	{CMB, V3, BIEL_TPLCD, WHITE,
	 {183, 248, 162, 977, 875, 183, 474, 1645, 100, -2470, 7568, -2887,
	  6064, 1021, 1567, 1964, 1915, 2484, 3136, 1649, 319, 1839, 8717, 30000,
	  0} },

	{CMB, V3, VITAL_TPLCD, BLACK,
	 {341, 894, 749, 4847, 3689, 341, 745, 2953, 100, -2528, 11134, -3420,
	  9328, 1325, 1698, 3619, 1964, 730, 661, 231, 78, 1928, 8290, 30000,
	  0} },
	{CMB, V3, VITAL_TPLCD, WHITE,
	 {183, 248, 162, 977, 875, 183, 474, 1645, 100, -2470, 7568, -2887,
	  6064, 1021, 1567, 1964, 1915, 2484, 3136, 1649, 319, 1839, 8717, 30000,
	  0} },
	{CMB, V3, DEFAULT_TPLCD, SILVER,
	 {183, 248, 162, 977, 875, 183, 474, 1645, 100, -2470, 7568, -2887,
	  6064, 1021, 1567, 1964, 1915, 2484, 3136, 1649, 319, 1839, 8717, 30000,
	  0} },
	{CMB, V3, DEFAULT_TPLCD, BLACK,
	 {341, 894, 749, 4847, 3689, 341, 745, 2953, 100, -2528, 11134, -3420,
	  9328, 1325, 1698, 3619, 1964, 730, 661, 231, 78, 1928, 8290, 30000,
	  0} },
	{CMB, V3, DEFAULT_TPLCD, WHITE,
	 {183, 248, 162, 977, 875, 183, 474, 1645, 100, -2470, 7568, -2887,
	  6064, 1021, 1567, 1964, 1915, 2484, 3136, 1649, 319, 1839, 8717, 30000,
	  0} },
	{CMB, V3, DEFAULT_TPLCD, GOLD,
	 {169, 756, 585, 3117, 2452, 169, 428, 1549, 100, -2443, 6839, -2719,
	  5383, 1021, 1567, 1853, 1953, 865, 1066, 553, 109, 1988, 7046, 30000,
	  0} },
	{CMB, V3, DEFAULT_TPLCD, CAFE_2,
	 {250, 693, 494, 3528, 2992, 250, 529, 2062, 100, -2448, 7776, -3229,
	  6547, 1177, 1648, 3055, 1988, 873, 860, 432, 99, 1819, 7772, 30000,
	  0} },
	{CMB, V3, DEFAULT_TPLCD, BROWN,
	 {250, 693, 494, 3528, 2992, 250, 529, 2062, 100, -2448, 7776, -3229,
	  6547, 1177, 1648, 3055, 1988, 873, 860, 432, 99, 1819, 7772, 30000,
	  0} },
	{CMB, V3, DEFAULT_TPLCD, BLUE,
	 {300, 1142, 530, 3003, 2105, 300, 202, 448, 100, -2728, 2824, -1851,
	  2219, 624, 983, 754, 1739, 874, 2109, 4626, 278, 2000, 7000, 30000,
	  0} },

	{LON, V3, DEFAULT_TPLCD, BLACK,
	 {550, 6573, 3916, 34236, 26201, 550, 442, 1502, 100, -2378, 5055, -3137,
	  5205, 1064, 1563, 3154, 2076, 186, 177, 144, 25, 3641, 6568, 5000,
	  200} },
	{LON, V3, DEFAULT_TPLCD, BLACK2,
	 {550, 6573, 3916, 34236, 26201, 550, 442, 1502, 100, -2378, 5055, -3137,
	  5205, 1064, 1563, 3154, 2076, 186, 177, 144, 25, 3641, 6568, 5000,
	  200} },
	{LON, V3, DEFAULT_TPLCD, GRAY,
	 {550, 6573, 3916, 34236, 26201, 550, 442, 1502, 100, -2378, 5055, -3137,
	  5205, 1064, 1563, 3154, 2076, 186, 177, 144, 25, 3641, 6568, 5000,
	  200} },
	{LON, V3, DEFAULT_TPLCD, WHITE,
	 {242, 655, 491, 3482, 3265, 242, 494, 1563, 100, -2903, 8252, -3412,
	  6059, 1552, 1479, 3229, 2003, 1466, 1473, 808, 174, 3044, 8765, 5000,
	  200} },
	{LON, V3, DEFAULT_TPLCD, GOLD,
	 {261, 1605, 1135, 11004, 11054, 261, 940, 3080, 100, -2696, 14018, -4183,
	  11108, 1716, 1754, 6652, 2059, 571, 495, 110, 59, 3181, 6282, 5000,
	  200} },
	{LON, V4, DEFAULT_TPLCD, BLACK,
	 {580, 7698, 5645, 34479, 27328, 580, 441, 1286, 100, -2361, 5537, -3511,
	  5490, 993, 1568, 3472, 1955, 128, 147, 101, 19, 3026, 6938, 5000,
	  200} },
	{LON, V4, DEFAULT_TPLCD, BLACK2,
	 {276, 3230, 2539, 18902, 18304, 276, 581, 1766, 100, -2638, 9007, -3930,
	  7111, 1445, 1643, 4450, 1998, 284, 289, 119, 32, 3189, 7728, 5000,
	  200} },
	{LON, V4, DEFAULT_TPLCD, GRAY,
	{580, 7698, 5645, 34479, 27328, 580, 441, 1286, 100, -2361, 5537, -3511,
	 5490, 993, 1568, 3472, 1955, 128, 147, 101, 19, 3026, 6938, 5000,
	 200} },
	{LON, V4, DEFAULT_TPLCD, WHITE,
	 {220, 477, 485, 2515, 2188, 220, 510, 1481, 100, -2583, 8133, -3559,
	  6424, 1299, 1609, 3090, 1941, 1414, 1612, 755, 168, 2364, 8163, 5000,
	  200} },
	{LON, V4, DEFAULT_TPLCD, GOLD,
	 {257, 1750, 1072, 10256, 10957, 257, 962, 2996, 100, -2851, 16029, -4489,
	  11661, 1735, 1655, 7165, 2017, 389, 382, 80, 42, 2299, 7204, 5000,
	  200} },

	{LON, VN1, DEFAULT_TPLCD, BLACK,
	 {363, 3658, 3016, 17238, 13071, 363, 474, 1471, 100, -2567, 6973, -3368,
	  5753, 1175, 1546, 2982, 2007, 249, 370, 189, 36, 3374, 7898, 5000,
	  200} },
	{LON, VN1, DEFAULT_TPLCD, BLACK2,
	 {338, 3575, 2411, 21986, 21940, 338, 709, 2115, 100, -2488, 10499, -4231,
	  8846, 1383, 1749, 5937, 1959, 264, 314, 95, 32, 3412, 7698, 5000,
	  200} },
	{LON, VN1, DEFAULT_TPLCD, GRAY,
	{363, 3658, 3016, 17238, 13071, 363, 474, 1471, 100, -2567, 6973, -3368,
	 5753, 1175, 1546, 2982, 2007, 249, 370, 189, 36, 3374, 7898, 5000,
	 200} },
	{LON, VN1, DEFAULT_TPLCD, WHITE,
	 {220, 477, 485, 2515, 2188, 220, 510, 1481, 100, -2583, 8133, -3559,
	  6424, 1299, 1609, 3090, 1941, 1414, 1612, 755, 168, 2364, 8163, 5000,
	  200} },
	{LON, VN1, DEFAULT_TPLCD, GOLD,
	 {236, 1058, 804, 6614, 6782, 236, 1158, 3604, 100, -2732, 18826, -4503,
	  14090, 1692, 1737, 7550, 2006, 763, 891, 139, 85, 3350, 7252, 5000,
	  150} },

	{VICTORIA, V3, DEFAULT_TPLCD, WHITE,
	 {229, 124, 72, 685, 715, 229, 603, 1788, 100, -2729, 9704, -3850,
	  7481, 1481, 1601, 4365, 1969, 9288, 10212, 4418, 1179, 4073, 9612, 5000,
	  200} },
	{VICTORIA, V3, DEFAULT_TPLCD, BLACK,
	 {368, 766, 568, 4888, 4591, 368, 886, 2439, 100, -2778, 12391, -5575,
	  11362, 1713, 1679, 18255, 1891, 1721, 1480, 471, 195, 4276, 8228, 5000,
	  200} },
	{VICTORIA, V3, DEFAULT_TPLCD, GRAY,
	 {368, 766, 568, 4888, 4591, 368, 886, 2439, 100, -2778, 12391, -5575,
	  11362, 1713, 1679, 18255, 1891, 1721, 1480, 471, 195, 4276, 8228, 5000,
	  200} },
	{VICKY, V3, DEFAULT_TPLCD, WHITE,
	 {229, 124, 72, 685, 715, 229, 603, 1788, 100, -2729, 9704, -3850,
	  7481, 1481, 1601, 4365, 1969, 9288, 10212, 4418, 1179, 4073, 9612, 5000,
	  200} },
	{VICKY, V3, DEFAULT_TPLCD, BLACK,
	 {368, 766, 568, 4888, 4591, 368, 886, 2439, 100, -2778, 12391, -5575,
	  11362, 1713, 1679, 18255, 1891, 1721, 1480, 471, 195, 4276, 8228, 5000,
	  200} },
	{VICKY, V3, DEFAULT_TPLCD, GRAY,
	 {368, 766, 568, 4888, 4591, 368, 886, 2439, 100, -2778, 12391, -5575,
	  11362, 1713, 1679, 18255, 1891, 1721, 1480, 471, 195, 4276, 8228, 5000,
	  200} },

	{VICTORIA, V4, DEFAULT_TPLCD, WHITE,
	 {229, 124, 72, 685, 715, 229, 603, 1788, 100, -2729, 9704, -3850,
	  7481, 1481, 1601, 4365, 1969, 9288, 10212, 4418, 1179, 4073, 9612, 5000,
	  200} },
	{VICTORIA, V4, DEFAULT_TPLCD, BLACK,
	 {492, 530, 485, 2959, 2204, 492, 517, 1667, 100, -2540, 6486, -4010,
	  6170, 1264, 1587, 5536, 2053, 3098, 2669, 1742, 361, 4769, 7145, 6000,
	  150} },
	{VICTORIA, V4, DEFAULT_TPLCD, GRAY,
	{492, 530, 485, 2959, 2204, 492, 517, 1667, 100, -2540, 6486, -4010,
	 6170, 1264, 1587, 5536, 2053, 3098, 2669, 1742, 361, 4769, 7145, 6000,
	 150} },
	{VICKY, V4, DEFAULT_TPLCD, WHITE,
	 {229, 124, 72, 685, 715, 229, 603, 1788, 100, -2729, 9704, -3850,
	  7481, 1481, 1601, 4365, 1969, 9288, 10212, 4418, 1179, 4073, 9612, 5000,
	  200} },
	{VICKY, V4, DEFAULT_TPLCD, BLACK,
	{492, 530, 485, 2959, 2204, 492, 517, 1667, 100, -2540, 6486, -4010,
	 6170, 1264, 1587, 5536, 2053, 3098, 2669, 1742, 361, 4769, 7145, 6000,
	 150} },
	{VICKY, V4, DEFAULT_TPLCD, GRAY,
	{492, 530, 485, 2959, 2204, 492, 517, 1667, 100, -2540, 6486, -4010,
	 6170, 1264, 1587, 5536, 2053, 3098, 2669, 1742, 361, 4769, 7145, 6000,
	 150} },
	{VICTORIA, VN1, DEFAULT_TPLCD, WHITE,
	 {249, 176, 165, 1036, 965, 249, 778, 2225, 100, -2532, 11984, -4279,
	  9814, 1403,1736, 5559, 1931, 5864, 6001, 1851, 705, 3624,8748, 5000,
	  200} },
	{VICTORIA, VN1, DEFAULT_TPLCD, BLACK,
	  {1243,948,355,4506,2891,1243,474,1604,100,-2369,4147,-3605,
	 5485,953,1460,5438,2129,2395,1997,2300,407,5634,6658,6000,
	 150}},
	{VICTORIA, VN1, DEFAULT_TPLCD, GRAY,
	 {1243,948,355,4506,2891,1243,474,1604,100,-2369,4147,-3605,
	 5485,953,1460,5438,2129,2395,1997,2300,407,5634,6658,6000,
	 150}},
	{VICKY, VN1, DEFAULT_TPLCD, WHITE,
	 {230, 166, 148, 923, 857, 230, 700, 2068, 100, -2466, 10740, -3960,
	  8704, 1293, 1726, 4284, 1965, 4534, 6143, 1847, 617, 3211, 8412, 5000,
	  200} },
	{VICKY, VN1, DEFAULT_TPLCD, BLACK,
	{500,1188,712,6805,5267,500,496,1563,100,-2218,3966,-4079,
	 6010,971,1573,7560,2023,877,904,940,139,3996,6190,6000,
	 150}},
	{VICKY, VN1, DEFAULT_TPLCD, GRAY,
	{500,1188,712,6805,5267,500,496,1563,100,-2218,3966,-4079,
	 6010,971,1573,7560,2023,877,904,940,139,3996,6190,6000,
	 150}},
	{BACH2, V3, DEFAULT_TPLCD, WHITE,
	{164,273,263,941,718,164,321,652,100,-3996,4892,-2930,
	 5284,2105,1568,1837,1369,6054,8168,3406,796,4669,6076,6000,
	 100}},
	{BACH2, V3, DEFAULT_TPLCD, BLACK,
	{241,598,288,2500,2187,241,385,838,100,-3680,5576,-3202,
	 5958,2308,1668,2547,1488,2124,2252,680,245,3450,5462,6000,
	 100}},
};

/* Although the GRAY and Black TP's RGB ink is same ,but some product may has both the GRAY
and Black TP,so must set the als para for  GRAY and Black TP

Although the CAFE_2 and BROWN TP's RGB ink is same ,but some product may has both the CAFE_2
and BROWN TP,so must set the als para for  CAFE_2 and BROWN TP
*/
APDS9251_ALS_PARA_TABLE apds_als_para_diff_tp_color_table[] = {
	{NEXT, V4, LG_TPLCD, BLACK,
	 {3989, 2459, 2836, 2915, 2788, 875, 1223, 778, 213, 136, 56,
	  1145, 1076, 744, 47, 2687, 5050, 5000, 200, 0, 100} },
	{NEXT, V4, LG_TPLCD, WHITE,
	 {5194, 2464, 942, 897, 812, 845, 1329, 714, 284, 190, 84, 3688, 5435,
	  1698, 138, 2844, 4855, 3200, 300, 0, 100} },
	{NEXT, V4, LG_TPLCD, GOLD,
	 {5160, 2469,  3263, 2996, 2912, 851, 1306, 728, 193, 133, 61, 1142,
	  1661, 497, 40, 3011, 4719, 3200, 300, 0, 100} },
	{NEXT, V4, LG_TPLCD, GRAY,
	 {5917, 2671, 4969, 3860, 3352, 763, 1425, 636, 395, 280, 133, 886,
	  1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{NEXT, V4, JDI_TPLCD, BLACK,
	 {5917, 2671, 4969, 3860, 3352, 763, 1425, 636, 395, 280, 133, 886,
	  1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{NEXT, V4, JDI_TPLCD, WHITE,
	 {5194, 2464, 942, 897, 812, 845, 1329, 714, 284, 190, 84, 3688, 5435,
	  1698, 138, 2844, 4855, 3200, 300, 0, 100} },
	{NEXT, V4, JDI_TPLCD, GOLD,
	 {5160, 2469,  3263, 2996, 2912, 851, 1306, 728, 193, 133, 61, 1142,
	  1661, 497, 40, 3011, 4719, 3200, 300, 0, 100} },
	{NEXT, V4, JDI_TPLCD, GRAY,
	 {5917, 2671, 4969, 3860, 3352, 763, 1425, 636, 395, 280, 133, 886,
	  1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },

	{NEXT, VN1, LG_TPLCD, BLACK,
	 {10219, 1910, 5387, 4286, 4017, 878, 1241, 688, 368, 261, 137, 876,
	  1004, 219, 30, 3004, 4464, 3200, 300, 0, 100} },
	{NEXT, VN1, LG_TPLCD, GRAY,
	 {10219, 1910, 5387, 4286, 4017, 878, 1241, 688, 368, 261, 137, 876,
	  1004, 219, 30, 3004, 4464, 3200, 300, 0, 100} },
	{NEXT, VN1, LG_TPLCD, WHITE,
	 {5424, 2050, 765, 733, 667, 924, 1254, 808, 267, 179, 79, 4818, 7259,
	  2303, 180, 2700, 4500, 3200, 300, 0, 100} },
	{NEXT, VN1, LG_TPLCD, GOLD,
	 {5405, 2046, 2528, 2162, 2280, 922, 1228, 814, 192, 137, 68, 1333,
	  1955, 611, 47, 2745, 4523, 3200, 300, 0, 100} },
	{NEXT, VN1, LG_TPLCD, BROWN,
	 {5819, 2156, 4047, 3490, 3080, 916, 1273, 783, 264, 190, 81, 1357,
	  1736, 523, 50, 3002, 4397, 3200, 300, 0, 100} },
	{NEXT, VN1, LG_TPLCD, CAFE_2,
	 {5819, 2156, 3113, 2685, 2369, 916, 1273, 783, 264, 190, 81, 1357,
	  1736, 523, 50, 3002, 4397, 3200, 300, 0, 100} },
	{NEXT, VN1, LG_TPLCD, PINK,
	 {5371, 2214, 2921, 2752, 2377, 904, 1263, 760, 255, 179, 82, 940,
	  1229, 380, 35, 2700, 4500, 3200, 300, 0, 100} },
	{NEXT, VN1, JDI_TPLCD, BLACK,
	 {10219, 1910, 5387, 4286, 4017, 878, 1241, 688, 368, 261, 137, 876,
	  1004, 219, 30, 3004, 4464, 3200, 300, 0, 100} },
	{NEXT, VN1, JDI_TPLCD, GRAY,
	 {10219, 1910, 5387, 4286, 4017, 878, 1241, 688, 368, 261, 137, 876,
	  1004, 219, 30, 3004, 4464, 3200, 300, 0, 100} },
	{NEXT, VN1, JDI_TPLCD, WHITE,
	 {5424, 2050, 765, 733, 667, 924, 1254, 808, 267, 179, 79, 4818, 7259,
	  2303, 180, 2700, 4500, 3200, 300, 0, 100} },
	{NEXT, VN1, JDI_TPLCD, GOLD,
	 {5405, 2046, 2528, 2162, 2280, 922, 1228, 814, 192, 137, 68, 1333,
	  1955, 611, 47, 2745, 4523, 3200, 300, 0, 100} },
	{NEXT, VN1, JDI_TPLCD, BROWN,
	 {5819, 2156, 4047, 3490, 3080, 916, 1273, 783, 264, 190, 81, 1357,
	  1736, 523, 50, 3002, 4397, 3200, 300, 0, 100} },
	{NEXT, VN1, JDI_TPLCD, CAFE_2,
	 {5819, 2156, 3113, 2685, 2369, 916, 1273, 783, 264, 190, 81, 1357,
	  1736, 523, 50, 3002, 4397, 3200, 300, 0, 100} },
	{NEXT, VN1, JDI_TPLCD, PINK,
	 {5371, 2214, 2921, 2752, 2377, 904, 1263, 760, 255, 179, 82, 940, 1229,
	  380, 35, 2700, 4500, 3200, 300, 0, 100} },

	{KNIGHT, V3, KNIGHT_BIEL_TPLCD, BLACK,
	 {16548, 1420, 10654, 8424, 6442, 815, 1095, 622, 578, 345, 166, 512,
	  531, 90, 20, 3142, 4328, 5000, 200, 1, 15} },
	{KNIGHT, V3, KNIGHT_BIEL_TPLCD, WHITE,
	 {6889, 1700, 1562, 1444, 1342, 1000, 1142, 875, 334, 210, 87, 2285,
	  3342, 1043, 85, 2900, 4844, 5000, 200, 0, 100} },
	{KNIGHT, V3, KNIGHT_BIEL_TPLCD, GOLD,
	 {11350, 1652,  3858, 3782, 3263, 953, 1108, 820, 231, 142, 57, 1096,
	  1531, 374, 33, 3069, 4574, 5000, 200, 0, 100} },
	{KNIGHT, V3, KNIGHT_BIEL_TPLCD, GRAY,
	 {5917, 2671, 4969, 3860, 3352, 763, 1425, 636, 395, 280, 133, 886,
	  1124, 287, 34, 3102, 4587, 20000, 0, 0, 100} },
	{KNIGHT, V3, KNIGHT_LENS_TPLCD, BLACK,
	 {4903, 1793, 6407, 4110, 2487, 1052, 1394, 869, 736, 517, 231, 828,
	  957, 467, 68, 3406, 4558, 5000, 200, 1, 28} },
	{KNIGHT, V3, KNIGHT_LENS_TPLCD, WHITE,
	 {7186, 1688, 1263, 1194, 1094, 1001, 1140, 881, 297, 185, 76, 3049,
	  4452, 1327, 108, 3123, 4815, 5000, 200, 0, 100} },
	{KNIGHT, V3, KNIGHT_LENS_TPLCD, GOLD,
	 {7816, 1737,  5104, 5072, 4090, 977, 1121, 873, 242, 148, 59, 817,
	  1124, 275, 25, 3187, 4367, 5000, 200, 0, 100} },
	{KNIGHT, V3, KNIGHT_LENS_TPLCD, GRAY,
	 {5917, 2671, 4969, 3860, 3352, 763, 1425, 636, 395, 280, 133, 886,
	  1124, 287, 34, 3102, 4587, 20000, 0, 0, 100} },

	{KNIGHT, V3, DEFAULT_TPLCD, BLACK,
	 {4903, 1793, 6407, 4110, 2487, 1052, 1394, 869, 736, 517, 231, 828,
	  957, 467, 68, 3406, 4558, 5000, 200, 1, 28} },
	{KNIGHT, V3, DEFAULT_TPLCD, WHITE,
	 {7186, 1688, 1263, 1194, 1094, 1001, 1140, 881, 297, 185, 76, 3049,
	  4452, 1327, 108, 3123, 4815, 5000, 200, 0, 100} },
	{KNIGHT, V3, DEFAULT_TPLCD, GOLD,
	 {7816, 1737,  5104, 5072, 4090, 977, 1121, 873, 242, 148, 59, 817,
	  1124, 275, 25, 3187, 4367, 5000, 200, 0, 100} },
	{KNIGHT, V3, DEFAULT_TPLCD, GRAY,
	 {5917, 2671, 4969, 3860, 3352, 763, 1425, 636, 395, 280, 133, 886,
	  1124, 287, 34, 3102, 4587, 20000, 0, 0, 100} },

	{EVA, V4, DEFAULT_TPLCD, BLACK,
	 {8746, 1927, 2190, 2079, 2022, 861, 1147, 710, 373, 246, 105, 1289,
	  1715, 570, 48, 2086, 5794, 5000, 200, 0, 100} },
	{EVA, V4, DEFAULT_TPLCD, GRAY,
	 {8746, 1927, 2190, 2079, 2022, 861, 1147, 710, 373, 246, 105, 1289,
	  1715, 570, 48, 2086, 5794, 5000, 200, 0, 100} },
	{EVA, V4, DEFAULT_TPLCD, WHITE,
	 {5581, 2007, 811, 767, 727, 929, 1196, 823, 280, 182, 78, 3824, 5717,
	  2243, 149, 2575, 5280, 5000, 200, 0, 100} },
	{EVA, V4, DEFAULT_TPLCD, GOLD,
	 {6116, 2118, 2859, 2555, 2530, 910, 1183, 802, 189, 123, 54, 801, 1002,
	  340, 27, 2571, 4723, 5000, 200, 0, 100} },
	{EVA, V4, DEFAULT_TPLCD, PINK,
	 {13543, 2092, 978, 1009, 846, 803, 1193, 670, 322, 211, 91, 3890, 4310,
	  745, 114, 2340, 4685, 5000, 200 , 0, 100} },

	{FARADAY, V3, JDI_TPLCD, BLACK,
	 {8996, 1643, 3930, 3543, 2844, 950, 1139, 797, 448, 283, 119, 1024, 1234,
	 424, 42, 2694, 5371, 5000, 200,0,100} },
	{FARADAY, V3, JDI_TPLCD, WHITE,
	 {6634, 1699, 831, 791, 721, 1010, 1139, 891, 319, 198, 82, 5874, 8059,
	  3499, 239, 3719, 5654, 5000, 200, 0, 100} },
	{FARADAY, V3, JDI_TPLCD, GOLD,
	 {6709, 1670,  3991, 3697, 3327, 1017, 1096, 914, 220, 137, 57, 1019, 1375,
	 545, 38, 3050, 5258, 5000, 200,0,100} },
	{FARADAY, V3, JDI_TPLCD, BLUE,
	 {1024, 1693, 4071, 2242, 2242, 1263, 1460, 1096, 1718, 1250, 572,
	  788, 1495, 3312, 165, 3381, 5996, 5000, 200, 0, 100} },
	{FARADAY, V3, JDI_TPLCD, PINK,
	 {19889, 1389, 976, 910, 718, 845, 1037, 693, 302, 190, 80,
	 7536, 5924, 1385, 232, 3312, 5050,5000,200,0,100} },

	{FARADAY, V3, LG_TPLCD, BLACK,
	 {7028, 1880, 7468, 6669, 5427, 958, 1170, 819, 407, 255, 108, 507,
	  634, 242, 21, 2630, 5234, 5000, 200, 0, 100} },
	{FARADAY, V3, LG_TPLCD, WHITE,
	 {6325, 1716, 843, 802, 732, 1014, 1140, 895, 317, 197, 82, 5635,
	  7753, 3519, 237, 3630, 5665, 5000, 200, 0, 100} },
	{FARADAY, V3, LG_TPLCD, GOLD,
	 {7429, 1713,  3471, 3397, 3164, 992, 1106, 880, 225, 142, 59,
	 1270, 1733, 601, 44, 3343, 5231, 5000, 200,0,100} },
	{FARADAY, V3, LG_TPLCD, BLUE,
	 {590, 1400, 4757, 2900, 2505, 1439, 1552, 1289, 2845, 2174,
	  1008, 354, 1379, 2957, 135, 3652, 6328, 5000, 200, 0, 100} },
	{FARADAY, V3, LG_TPLCD, PINK,
	 {31023, 1457, 831, 824, 650, 730, 1017, 562, 282, 176, 72,
	  9022, 6993, 985, 257, 3228, 4844, 5000, 200, 0, 100} },

	{FARADAY, V3, SHARP_TPLCD, BLACK,
	 {7028, 1880, 7468, 6669, 5427, 958, 1170, 819, 407, 255, 108,
	  507, 634, 242, 21, 2630, 5234, 5000, 200, 0, 100} },
	{FARADAY, V3, SHARP_TPLCD, WHITE,
	 {6325, 1716, 843, 802, 732, 1014, 1140, 895, 317, 197, 82,
	  5635, 7753, 3519, 237, 3630, 5665, 5000, 200, 0, 100} },
	{FARADAY, V3, SHARP_TPLCD, GOLD,
	 {7429, 1713,  3471, 3397, 3164, 992, 1106, 880, 225, 142, 59,
	 1270, 1733, 601, 44, 3343, 5231, 5000, 200,0,100} },
	{FARADAY, V3, SHARP_TPLCD, BLUE,
	 {590, 1400, 4757, 2900, 2505, 1439, 1552, 1289, 2845, 2174, 1008,
	  354, 1379, 2957, 135, 3652, 6328, 5000, 200, 0, 100} },
	{FARADAY, V3, SHARP_TPLCD, PINK,
	 {31023, 1457, 831, 824, 650, 730, 1017, 562, 282, 176, 72, 9022,
	  6993, 985, 257, 3228, 4844, 5000, 200, 0, 100} },

	{FARADAY, V3, DEFAULT_TPLCD, BLACK,
	 {8996, 1643, 3930, 3543, 2844, 950, 1139, 797, 448, 283, 119, 1024, 1234,
	 424, 42, 2694, 5371, 20000, 0,0,100} },
	{FARADAY, V3, DEFAULT_TPLCD, WHITE,
	 {6634, 1699, 831, 791, 721, 1010, 1139, 891, 319, 198, 82, 5874,
	 8059, 3499, 239, 3719, 5654, 20000, 0, 0, 100} },
	{FARADAY, V3, DEFAULT_TPLCD, GOLD,
	 {6709, 1670,  3991, 3697, 3327, 1017, 1096, 914, 220, 137, 57, 1019,
	 1375, 545, 38, 3050, 5258, 20000, 0,0,100} },
	{FARADAY, V3, DEFAULT_TPLCD, BLUE,
	 {1024, 1693, 4071, 2242, 2242, 1263, 1460, 1096, 1718, 1250,
	  572, 788, 1495, 3312, 165, 3381, 5996, 20000, 0, 0, 100} },
	{FARADAY, V3, DEFAULT_TPLCD, PINK,
	 {19889, 1389, 976, 910, 718, 845, 1037, 693, 302, 190, 80,
	 7536, 5924, 1385, 232, 3312, 5050,20000,0,0,100} },

	{VIENNA, V4, DEFAULT_TPLCD, BLACK,
	 {7505, 2074, 2934, 2900, 2519, 876, 1299, 743, 437, 291, 129,
	  1178, 1482, 443, 47, 1858, 4896, 5000, 200, 0, 100} },
	{VIENNA, V4, DEFAULT_TPLCD, WHITE,
	 {7059, 2001, 400, 399, 351, 906, 1231, 788, 360, 240, 105, 3616,
	  4989, 1615, 141, 2427, 5153, 5000, 200, 0, 100} },
	{VIENNA, V4, DEFAULT_TPLCD, GOLD,
	 {5281, 2341, 2615, 2661, 2450, 883, 1258, 770, 236, 156, 70, 1035,
	  1380, 460, 36, 2010, 4680, 5000, 200, 0, 100} },
	{VIENNA, V4, DEFAULT_TPLCD, GRAY,
	 {7505, 2074, 2934, 2900, 2519, 876, 1299, 743, 437, 291, 129, 1178,
	  1482, 443, 47, 1858, 4896, 5000, 200, 0, 100} },

	{VIENNA, VN1, DEFAULT_TPLCD, BLACK,
	 {11757, 1220, 3987, 5223, 4154, 1004, 1326, 843, 489, 346, 160,
	  482, 578, 180, 20, 1280, 5600, 5000, 200, 0, 100} },
	{VIENNA, VN1, DEFAULT_TPLCD, WHITE,
	 {7356, 2025, 1014, 1018, 900, 894, 1211, 772, 399, 266, 117, 1776,
	  2360, 818, 72, 1330, 5400, 5000, 200, 0, 100} },
	{VIENNA, VN1, DEFAULT_TPLCD, GOLD,
	 {4876, 2347, 3621, 3596, 3366, 886, 1233, 781, 252, 167, 72,
	  437, 577, 222, 18, 928, 4823, 5000, 200, 0, 100} },
	{VIENNA, VN1, DEFAULT_TPLCD, GRAY,
	 {11757, 1220, 3987, 5223, 4154, 1004, 1326, 843, 489, 346,
	  160, 482, 578, 180, 20, 1280, 5600, 5000, 200, 0, 100} },

	{VIENNA, VN2, DEFAULT_TPLCD, BLACK,
	 {8013, 2327, 3095, 2783, 2497, 812, 1283, 674, 501, 337, 149,
	  720, 810, 225, 30, 1400, 4800, 5000, 200,0,100}},
	{VIENNA, VN2, DEFAULT_TPLCD, WHITE,
	 {7356, 2025, 1014, 1018, 900, 894, 1211, 772, 399, 266, 117,
	  1776, 2360, 818, 72, 1330, 5400, 5000, 200,0,100}},
	{VIENNA, VN2, DEFAULT_TPLCD, GOLD,
	 {4876, 2347, 3621, 3596, 3366, 886, 1233, 781, 252, 167, 72,
	  437, 577, 222, 18, 928, 4823, 5000, 200 ,0,100}},
	{VIENNA, VN2, DEFAULT_TPLCD, GRAY,
	 {8013, 2327, 3095, 2783, 2497, 812, 1283, 674, 501, 337, 149,
	  720, 810, 225, 30, 1400, 4800, 5000, 200,0,100}},

	{VNS,V4,BOE_TPLCD,GOLD,
	 {8471, 2307, 3315, 3100, 2760, 1006, 715, 800, 221, 147, 66,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{VNS,V4,BOE_TPLCD,WHITE,
	 {16434, 1333, 1441, 1437, 1239, 1036,751, 866, 309, 200, 93,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{VNS,V4,BOE_TPLCD,BLACK,
	 {3922, 1925, 5985, 2080, 1749, 965, 1301, 853, 1051, 590, 400,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 1, 430} },
	{VNS,V4,JDI_TPLCD,GOLD,
	 {8471, 2307, 3315, 3100, 2760, 1006, 715, 800, 221, 147, 66,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{VNS,V4,JDI_TPLCD,WHITE,
	 {16434, 1333, 1441, 1437, 1239, 1036,751, 866, 309, 200, 93,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{VNS,V4,JDI_TPLCD,BLACK,
	 {3922, 1925, 5985, 2080, 1749, 965, 1301, 853, 1051, 590, 400,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 1, 430} },
	{VNS,V4,LG_TPLCD,GOLD,
	 {8471, 2307, 3315, 3100, 2760, 1006, 715, 800, 221, 147, 66,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{VNS,V4,LG_TPLCD,WHITE,
	 {16434, 1333, 1441, 1437, 1239, 1036,751, 866, 309, 200, 93,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{VNS,V4,LG_TPLCD,BLACK,
	 {3922, 1925, 5985, 2080, 1749, 965, 1301, 853, 1051, 590, 400,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 1, 430} },
	{VNS, V4, INX_TPLCD, GOLD,
	 {10079, 1804, 3986, 3917, 3431, 930, 1160, 806, 182, 149, 66,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{VNS,V4,INX_TPLCD,WHITE,
	 {6240, 1954, 1950, 1803, 1532, 940, 1218, 844, 294, 217, 92,
	  886, 1124, 287, 34, 3102, 4587,3200, 300, 0, 100} },
	{VNS, V4, INX_TPLCD, BLACK,
	 {3711, 798, 4004, 1293, 975, 1639, 1778, 1392, 1013, 810, 560,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 1, 38} },

	{VNS, V4, EBBG_TPLCD, GOLD,
	 {10079, 1804, 3986, 3917, 3431, 930, 1160, 806, 182, 149, 66,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{VNS,V4,EBBG_TPLCD,WHITE,
	 {6240, 1954, 1950, 1803, 1532, 940, 1218, 844, 294, 217, 92,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 0, 100} },
	{VNS, V4, EBBG_TPLCD, BLACK,
	 {3711, 798, 4004, 1293, 975, 1639, 1778, 1392, 1013, 810, 560,
	  886, 1124, 287, 34, 3102, 4587, 3200, 300, 1, 38} },
	{BTV, V3, DEFAULT_TPLCD, BLACK,
	 {649, 2715, 1431, 1379, 1287, 1002, 2004, 824, 300, 190, 100,
	  3240, 4074, 1571, 117, 3239, 3209, 5000, 200, 0, 100} },
	{BTV, V3, DEFAULT_TPLCD, WHITE,
         {649, 2715, 1431, 1379, 1287, 1002, 2004, 824, 300, 190, 100,
          3240, 4074, 1571, 117, 3239, 3209, 5000, 200, 0, 100} },
	{HAYDN, V3, DEFAULT_TPLCD, WHITE,
         {12572, -350, 1023, 1011, 911, 1625, 1001, 1742, 181, 130, 43,
          3354, 4873, 2131, 115, 2770, 7637, 5000, 200, 1, 38} },
    {HANDEL, V3, DEFAULT_TPLCD, WHITE,
         {6924, 1068, 1880, 1774, 1879, 1022, 1038, 960, 568, 266, 93,
          2168, 3474, 1336, 116, 3628, 5334, 6000, 200, 1, 38} },
	{NATASHA, V3, DEFAULT_TPLCD, BLACK,
	 {4016, 2116, 533, 512, 445, 930, 1237, 827, 479, 317, 139,
	  7592, 7305, 1724, 324, 2706, 3028, 4500, 200, 0, 100} },
	{CHICAGO, V1, DEFAULT_TPLCD, GOLD,
	 {3989, 2459, 2836, 2915, 2788, 875, 1223, 778, 213, 136, 56,
	  1145, 1076, 744, 47, 2687, 5050, 5000, 200, 0, 100} },
	{CHICAGO, V3, DEFAULT_TPLCD, BLACK,
	 {10241, 1896, 4544, 3631, 3330, 822, 1196, 675, 559, 380, 170,
	  640, 731, 226, 30, 1845, 5512, 5000, 200, 0, 100} },
	{CHICAGO, V3, DEFAULT_TPLCD, WHITE,
	 {6382, 1837, 1043, 978, 892, 956, 1175, 851, 341, 227, 99,
	  3084, 4403, 2032, 144, 2551, 6042, 5000, 200, 0, 100} },
	{CHICAGO, V3, DEFAULT_TPLCD, GOLD,
	 {5026, 2150, 3245, 2813, 2943, 928, 1255, 828, 235, 156, 68,
	  607, 824, 373, 25, 1485, 5238, 5000, 200, 0, 100} },
	{CHICAGO, V3, DEFAULT_TPLCD, CAFE_2,
	 {6934, 2068, 3507, 3131, 2848, 900, 1221, 780, 352, 235, 104,
	  770, 913, 385, 33, 1778, 5535, 5000, 200, 0, 100} },
	{CHICAGO, V3, DEFAULT_TPLCD, BROWN,
	 {6934, 2068, 3507, 3131, 2848, 900, 1221, 780, 352, 235, 104,
	  770, 913, 385, 33, 1778, 5535, 5000, 200, 0, 100} },

	{DUKE, V3, SHARP_TPLCD, BLACK,
	 {11088, 1414, 7795, 5700, 3610, 875, 1114, 670, 516, 361, 167,
	 421, 476, 146, 18, 1930, 5250, 30000, 0, 0, 100} },
	{DUKE, V3, SHARP_TPLCD, BLUE,
	 {520, 1172, 4968, 2636, 2248, 1697, 1834, 1496, 2270, 1803, 854,
	  225, 1084, 2065, 67, 2990, 5944, 5000, 200, 0, 100} },
	{DUKE, V3, SHARP_TPLCD, WHITE,
	 {6163, 1659, 1134, 1011, 921, 1022, 1178, 880, 294, 189, 79,
	  2752, 4056, 1746, 100, 2555, 5565, 5000, 200, 0, 100} },

	{DUKE, V3, JDI_TPLCD, BLACK,
	 {7754, 1487, 8061, 5274, 4144, 914, 1151, 697, 681, 477, 215,
	  280, 384, 137, 13, 2000, 5250, 30000, 0, 0, 100} },
	{DUKE, V3, JDI_TPLCD, BLUE,
	 {843, 1328, 4013, 2127, 1641, 1558, 1771, 1338, 1740, 1363, 710,
	  458, 1344, 2368, 92, 2000, 5500, 5000, 200, 0, 100} },
	{DUKE, V3, JDI_TPLCD, WHITE,
	 {6267, 1681, 1172, 1059, 977, 1020, 1166, 882, 276, 176, 75,
	  2938, 4365, 1759, 104, 2850, 5450, 5000, 200, 0, 100} },

	{DUKE, V3, DEFAULT_TPLCD, BLACK,
	 {11088, 1414, 7795, 5700, 3610, 875, 1114, 670, 516, 361, 167,
	  421, 476, 146, 18, 1930, 5250, 30000, 0, 0, 100} },
	{DUKE, V3, DEFAULT_TPLCD, BLUE,
	 {520, 1172, 4968, 2636, 2248, 1697, 1834, 1496, 2270, 1803, 854,
	  225, 1084, 2065, 67, 2990, 5944, 30000, 0, 0, 100} },
	{DUKE, V3, DEFAULT_TPLCD, WHITE,
	 {6163, 1659, 1134, 1011, 921, 1022, 1178, 880, 294, 189, 79,
	  2752, 4056, 1746, 100, 2555, 5565, 30000, 0, 0, 100} },

	{CMB, V3, BIEL_TPLCD, BLACK,
	 {10241, 1896, 4544, 3631, 3330, 822, 1196, 675, 559, 380, 170,
	  640, 731, 226, 30, 1845, 5512, 20000, 0, 0, 100} },
	{CMB, V3, BIEL_TPLCD, WHITE,
	 {6382, 1837, 1043, 978, 892, 956, 1175, 851, 341, 227, 99,
	  3084, 4403, 2032, 144, 2551, 6042, 20000, 0, 0, 100} },

	{CMB, V3, VITAL_TPLCD, BLACK,
	 {10241, 1896, 4544, 3631, 3330, 822, 1196, 675, 559, 380, 170,
	  640, 731, 226, 30, 1845, 5512, 20000, 0, 0, 100} },
	{CMB, V3, VITAL_TPLCD, WHITE,
	 {6382, 1837, 1043, 978, 892, 956, 1175, 851, 341, 227, 99,
	  3084, 4403, 2032, 144, 2551, 6042, 20000, 0, 0, 100} },

	{CMB, V3, DEFAULT_TPLCD, BLACK,
	 {10241, 1896, 4544, 3631, 3330, 822, 1196, 675, 559, 380, 170,
	  640, 731, 226, 30, 1845, 5512, 20000, 0, 0, 100} },
	{CMB, V3, DEFAULT_TPLCD, WHITE,
	 {6382, 1837, 1043, 978, 892, 956, 1175, 851, 341, 227, 99,
	  3084, 4403, 2032, 144, 2551, 6042, 20000, 0, 0, 100} },
	{CMB, V3, DEFAULT_TPLCD, GOLD,
	 {5026, 2150, 3245, 2813, 2943, 928, 1255, 828, 235, 156, 68,
	  607, 824, 373, 25, 1485, 5238, 20000, 0, 0, 100} },
	{CMB, V3, DEFAULT_TPLCD, CAFE_2,
	 {6934, 2068, 3507, 3131, 2848, 900, 1221, 780, 352, 235, 104,
	  770, 913, 385, 33, 1778, 5535, 20000, 0, 0, 100} },
	{CMB, V3, DEFAULT_TPLCD, BROWN,
	 {6934, 2068, 3507, 3131, 2848, 900, 1221, 780, 352, 235, 104,
	  770, 913, 385, 33, 1778, 5535, 20000, 0, 0, 100} },
	{CMB, V3, DEFAULT_TPLCD, SILVER,
	 {6382, 1837, 1043, 978, 892, 956, 1175, 851, 341, 227, 99,
	  3084, 4403, 2032, 144, 2551, 6042, 20000, 0, 0, 100} },
	{CMB, V3, DEFAULT_TPLCD, BLUE,
	 {1024, 1693, 4071, 2242, 2242, 1263, 1460, 1096, 1718, 1250,
	  572, 788, 1495, 3312, 165, 2000, 6000, 20000, 0, 0, 100} },
	{LON, VN1, DEFAULT_TPLCD, WHITE,
	 {1236, 698, 3639, 3108, 2617, 891, 990, 891, 511, 244, 209,
	  4486, 6072, 2433, 169, 3069, 9265, 5000,200,1,33} },
	{LON, VN1, DEFAULT_TPLCD, BLACK,
	 {12455, -296, 23765, 16984, 12680, 1012, 1046, 1012, 841, 437, 370,
	  712, 996, 480, 38, 3287, 8100, 5000,200,1,38} },
	{LON, VN1, DEFAULT_TPLCD, GRAY,
	 {12455, -296, 23765, 16984, 12680, 1012, 1046, 1012, 841, 437, 370,
	  712, 996, 480, 38, 3287, 8100, 5000,200,1,38} },
	{LON, VN1, DEFAULT_TPLCD,BLACK2,
	 {15106, 1185, 25038, 19759, 14902, 664, 955, 664, 739,371, 320,
	  763, 935, 271, 29, 3251, 6550, 5000,200,1,25} },
	{LON, VN1, DEFAULT_TPLCD, GOLD,
	 {11687, 2391, 8154, 7469, 5936, 740, 1250,740, 435, 120, 80,
	  2251, 2779, 498, 64, 3147, 4976, 5000,150,1,18} },
	{BACH2, V3, DEFAULT_TPLCD, WHITE,
	 {12598, -228, 915, 867, 804, 1413, 1000, 1442, 179, 115, 43,
	  6328, 8329, 3296, 286, 4233, 6333, 30000,50,1,38} },
	{BACH2, V3, DEFAULT_TPLCD, BLACK,
	 {13836, -503, 2276, 2048, 1772, 1457, 1000, 1388, 260, 169, 62,
	  2039, 2540, 1028, 106, 3211, 6472, 30000,50,1,38} },
};
/*in the array of pinhole_als_para_diff_tp_color_table, these figures represent the parameter
for the als sensor */
PINHOLE_ALS_PARA_TABLE pinhole_als_para_diff_tp_color_table[] = {
// there is a bug in codes which causes that the XXX_UNKNOWN arg can not be chosen. so you  must separately config TP manufacture arg.
	{ANE, V4, APDS9922, TS_PANEL_UNKNOWN, {733, 230, 196, 203, 228, 0, 2900, 3000, 5000, 200}},
	{ANE, V4, APDS9922, TS_PANEL_OFILIM, {734, 230, 196, 203, 228, 0, 2900, 3000, 5000, 200}},
	{ANE, V4, APDS9922, TS_PANEL_TRULY, {735, 230, 196, 203, 228, 0, 2900, 3000, 5000, 200}},
	{ANE, V4, APDS9922, TS_PANEL_MUTTO, {736, 230, 196, 203, 228, 0, 2900, 3000, 5000, 200}},

	{ANE, V4, LTR578,   TS_PANEL_UNKNOWN, {555, 186, 331, 340, 370, 0, 1970, 1710, 5000, 200}},
	{ANE, V4, LTR578,   TS_PANEL_OFILIM, {556, 186, 331, 340, 370, 0, 1970, 1710, 5000, 200}},
	{ANE, V4, LTR578,   TS_PANEL_TRULY, {557, 186, 331, 340, 370, 0, 1970, 1710, 5000, 200}},
	{ANE, V4, LTR578,   TS_PANEL_MUTTO, {558, 186, 331, 340, 370, 0, 1970, 1710, 5000, 200}},

	{RNE, V4, APDS9922, TS_PANEL_UNKNOWN, {1287, 305, 349, 361, 551,0}},
	{RNE, V4, APDS9922, TS_PANEL_OFILIM, {1287, 305, 349, 361, 551,0}},
	{RNE, V4, APDS9922, TS_PANEL_JUNDA, {1287, 305, 349, 361, 551,0}},

	{RNE, V4, LTR578, TS_PANEL_UNKNOWN, {1422, 439, 662, 661, 943,0}},
	{RNE, V4, LTR578, TS_PANEL_OFILIM, {1422, 439, 662, 661, 943,0}},
	{RNE, V4, LTR578, TS_PANEL_JUNDA, {1422, 439, 662, 661, 943,0}},

	{BLN, V4, APDS9922, TS_PANEL_UNKNOWN, {1200, 400, 268, 327, 400,1}},
	{BLN, V4, APDS9922, TS_PANEL_OFILIM, {1200, 400, 314, 380, 536,1}},
	{BLN, V4, APDS9922, TS_PANEL_LENS, {1200, 400, 268, 327, 400,1}},
	{BLN, V4, APDS9922, TS_PANEL_EELY, {1300, 400, 303, 348, 560,1}},

	{BLN, V4, LTR578, TS_PANEL_UNKNOWN, {1200, 300, 570, 570, 930,1}},
	{BLN, V4, LTR578, TS_PANEL_OFILIM, {1200, 300, 600, 600, 800,1}},
	{BLN, V4, LTR578, TS_PANEL_LENS, {1200, 300, 450, 530, 620,1}},
	{BLN, V4, LTR578, TS_PANEL_EELY, {1200, 300, 570, 570, 930,1}},

	{PRA, V4, APDS9922, TS_PANEL_UNKNOWN, {980, 350, 306, 366, 558,1}},
	{PRA, V4, APDS9922, TS_PANEL_OFILIM, {980, 350, 330, 366, 620,1}},
	{PRA, V4, APDS9922, TS_PANEL_TRULY, {980, 350, 306, 366, 558,1}},
	{PRA, V4, APDS9922, TS_PANEL_EELY, {980, 350, 310, 390, 656,1}},

	{PRA, V4, LTR578, TS_PANEL_UNKNOWN, {1150, 350, 600, 600, 724,1}},
	{PRA, V4, LTR578, TS_PANEL_OFILIM, {1150, 350, 600, 600, 800,1}},
	{PRA, V4, LTR578, TS_PANEL_TRULY, {1150, 350, 600, 600, 724,1}},
	{PRA, V4, LTR578, TS_PANEL_EELY, {1150, 350, 480, 560, 900,1}},

	{WAS, V4, APDS9922, DEFAULT_TPLCD, {1100, 320, 250, 220, 500,0}},
	{WAS, V4, APDS9922, TM_TPLCD, {1100, 300, 246, 220, 438,0}},
	{WAS, V4, APDS9922, AUO_TPLCD, {1100, 300, 262, 204, 430,0}},
	{WAS, V4, APDS9922, EBBG_TPLCD, {1200, 300, 215, 214, 460,0}},
	{WAS, V4, APDS9922, JDI_TPLCD, {1100, 300, 240, 225, 460,0}},

	{WAS, V4, LTR578, DEFAULT_TPLCD, {1300, 320, 500, 500, 800,0}},
	{WAS, V4, LTR578, TM_TPLCD, {1350, 320, 550, 525, 900,0}},
	{WAS, V4, LTR578, AUO_TPLCD, {1200, 320, 553, 500, 861,0}},
	{WAS, V4, LTR578, EBBG_TPLCD, {1350, 320, 530, 510, 820,0}},
	{WAS, V4, LTR578, JDI_TPLCD, {1200, 320, 510, 490, 850,0}},

	{PIC, V4, APDS9922, BOE_TPLCD, {1131, 423, 260, 277, 362, 0}},
	{PIC, V4, APDS9922, CTC_TPLCD, {1131, 423, 338, 360, 470, 0}},
	{PIC, V4, APDS9922, LG_TPLCD,  {1131, 423, 260, 277, 362, 0}},
	{PIC, V4, APDS9922, TM_TPLCD,  {1131, 423, 260, 277, 362, 0}},
	{PIC, V4, APDS9922, DEFAULT_TPLCD, {1131, 423, 260, 277, 362, 0}},

	{PIC, V4, LTR578, BOE_TPLCD, {1312, 431, 496, 580, 707, 0}},
	{PIC, V4, LTR578, CTC_TPLCD, {1312, 431, 496, 580, 707, 0}},
	{PIC, V4, LTR578, LG_TPLCD,  {1312, 431, 496, 580, 707, 0}},
	{PIC, V4, LTR578, TM_TPLCD,  {1312, 431, 496, 580, 707, 0}},
	{PIC, V4, LTR578, DEFAULT_TPLCD, {1312, 431, 496, 580, 707, 0}},

	{BAC, V4, APDS9922, AUO_TPLCD, {942, 352, 270, 291, 332, 0}},
	{BAC, V4, APDS9922, BOE_TPLCD, {942, 352, 270, 291, 332, 0}},
	{BAC, V4, APDS9922, CTC_TPLCD, {942, 352, 270, 291, 332, 0}},
	{BAC, V4, APDS9922, TM_TPLCD,  {942, 352, 270, 291, 332, 0}},
	{BAC, V4, APDS9922, DEFAULT_TPLCD, {942, 352, 270, 291, 332, 0}},

	{BAC, V4, LTR578, AUO_TPLCD, {992, 354, 509, 513, 624, 0}},
	{BAC, V4, LTR578, BOE_TPLCD, {992, 354, 509, 513, 624, 0}},
	{BAC, V4, LTR578, CTC_TPLCD, {992, 354, 509, 513, 624, 0}},
	{BAC, V4, LTR578, TM_TPLCD,  {992, 354, 509, 513, 624, 0}},
	{BAC, V4, LTR578, DEFAULT_TPLCD, {992, 354, 509, 513, 624, 0}},

	{BOND, V4, LTR578, TS_PANEL_UNKNOWN, {1300, 300, 650, 610, 810,0}},
	{BOND, V4, LTR578, TS_PANEL_OFILIM, {1300, 300, 650, 610, 810,0}},
	{BOND, V4, LTR578, TS_PANEL_TRULY, {1200, 285, 787, 650, 910,0}},
	{BOND, V4, LTR578, TS_PANEL_EELY, {1527, 380, 675, 772, 1050,0}},

	{BOND, V4, APDS9922, TS_PANEL_UNKNOWN, {1370, 388, 336, 380, 480,0}},
	{BOND, V4, APDS9922, TS_PANEL_OFILIM, {1370, 388, 336, 380, 480,0}},
	{BOND, V4, APDS9922, TS_PANEL_TRULY, {1233, 350, 396, 396, 533,0}},
	{BOND, V4, APDS9922, TS_PANEL_EELY, {1657, 458, 360, 380, 600,0}},

	{LELAND, V4, LTR578, TS_PANEL_UNKNOWN, {1350, 350, 655, 642, 840,0}},
	{LELAND, V4, LTR578, TS_PANEL_EELY, {1350, 350, 655, 642, 840,0}},
	{LELAND, V4, LTR578, TS_PANEL_JUNDA, {1313, 342, 740, 675, 910,0}},
	{LELAND, V4, LTR578, TS_PANEL_OFILIM, {1400, 400, 763, 671, 931,0}},

	{LELAND, V4, APDS9922, TS_PANEL_UNKNOWN, {1300, 300, 359, 353, 530,0}},
	{LELAND, V4, APDS9922, TS_PANEL_EELY, {1300, 300, 359, 353, 530,0}},
	{LELAND, V4, APDS9922, TS_PANEL_JUNDA, {1250, 300, 385, 345, 500,0}},
	{LELAND, V4, APDS9922, TS_PANEL_OFILIM, {1500, 400, 440, 400, 550,0}},

	{FIGO, V4, LTR578, TS_PANEL_UNKNOWN, {1208, 355, 689, 585, 864,0}},
	{FIGO, V4, LTR578, TS_PANEL_OFILIM, {1208, 355, 689, 585, 864,0}},
	{FIGO, V4, LTR578, TS_PANEL_TRULY, {1219, 328, 741, 586, 858,0}},
	{FIGO, V4, LTR578, TS_PANEL_EELY, {1371, 387, 699, 631, 872,0}},

	{FIGO, V4, APDS9922, TS_PANEL_UNKNOWN, {1369, 399, 425, 382, 517,0}},
	{FIGO, V4, APDS9922, TS_PANEL_OFILIM, {1369, 399, 425, 382, 517,0}},
	{FIGO, V4, APDS9922, TS_PANEL_TRULY, {1339, 361, 472, 385, 535,0}},
	{FIGO, V4, APDS9922, TS_PANEL_EELY, {1570, 444, 430, 414, 555,0}},

	{FLORIDA, V4, LTR578, TS_PANEL_UNKNOWN, {1223, 350, 710, 620, 840,0}},
	{FLORIDA, V4, LTR578, TS_PANEL_OFILIM, {1223, 350, 710, 620, 840,0}},
	{FLORIDA, V4, LTR578, TS_PANEL_MUTTO, {1166, 347, 700, 637, 870,0}},

	{FLORIDA, V4, APDS9922, TS_PANEL_UNKNOWN, {1450, 361, 358, 311, 465,0}},
	{FLORIDA, V4, APDS9922, TS_PANEL_OFILIM, {1450, 361, 358, 311, 465,0}},
	{FLORIDA, V4, APDS9922, TS_PANEL_MUTTO, {1397, 395, 360, 330, 481,0}},

	{LELANDPLUS, V4, LTR578, TS_PANEL_UNKNOWN, {1450, 400, 750, 710, 960,0}},
	{LELANDPLUS, V4, LTR578, TS_PANEL_EELY, {1450, 400, 750, 710, 960,0}},
	{LELANDPLUS, V4, LTR578, TS_PANEL_OFILIM, {1450, 400, 730, 650, 930,0}},
};

TMD2745_ALS_PARA_TABLE tmd2745_als_para_diff_tp_color_table[] = {
	/* tp_color reserved for future use */
	/* AMS TMD2745: Extend-Data Format -- {D_Factor, B_Coef, C_Coef, D_Coef, is_min_algo, is_auto_gain} */
	{ANE, V4, TS_PANEL_UNKNOWN, 0, {510, 142, 803, 60, 0, 0, 8393, 5182, 5000, 200}},
	{ANE, V4, TS_PANEL_OFILIM, 0, {511, 142, 803, 60, 0, 0, 8393, 5182, 5000, 200}},
	{ANE, V4, TS_PANEL_TRULY, 0, {512, 142, 803, 60, 0, 0, 8393, 5182, 5000, 200}},
	{ANE, V4, TS_PANEL_MUTTO, 0, {513, 142, 803, 60, 0, 0, 8393, 5182, 5000, 200}},

	{RNE, V4, TS_PANEL_UNKNOWN, 0, {1127, 94, 349, 0, 0, 0}},
	{RNE, V4, TS_PANEL_OFILIM, 0, {1127, 94, 349, 0, 0, 0}},
	{RNE, V4, TS_PANEL_JUNDA, 0, {1127, 94, 349, 0, 0, 0}},

	{PIC, V4, BOE_TPLCD, 0, {1059, 133, 319, 0, 0, 0}},
	{PIC, V4, CTC_TPLCD, 0, {975, 150, 648, 53, 0, 0}},
	{PIC, V4, LG_TPLCD, 0, {1059, 133, 319, 0, 0, 0}},
	{PIC, V4, TM_TPLCD, 0, {920, 112, 364, 0, 0, 0}},
	{PIC, V4, DEFAULT_TPLCD, 0, {538, 95, 1729, 190, 1, 0}},

	{BAC, V4, AUO_TPLCD, 0, {956, 104, 608, 40, 0, 0}},
	{BAC, V4, BOE_TPLCD, 0, {839, 120, 413, 2, 0, 0}},
	{BAC, V4, CTC_TPLCD, 0, {635, 129, 389, 0, 0, 0}},
	{BAC, V4, TM_TPLCD, 0, {1147, 108, 343, 0, 0, 0}},
	{BAC, V4, DEFAULT_TPLCD, 0, {538, 95, 1729, 190, 1, 0}},

	{BOND, V4, TS_PANEL_UNKNOWN, 0, {1081, 153, 637, 51, 0, 0}},
	{BOND, V4, TS_PANEL_OFILIM, 0, {1081, 153, 637, 51, 0, 0}},
	{BOND, V4, TS_PANEL_TRULY, 0, {1222, 161, 565,34, 0, 0}},
	{BOND, V4, TS_PANEL_EELY, 0, {1111, 164, 476, 27, 0, 0}},

	{LELAND, V4, TS_PANEL_UNKNOWN, 0, {1128, 206, 516, 35, 0, 0}},
	{LELAND, V4, TS_PANEL_EELY, 0, {1128, 206, 516, 35, 0, 0}},
	{LELAND, V4, TS_PANEL_JUNDA, 0, {1115, 176, 569, 42, 0, 0}},
	{LELAND, V4, TS_PANEL_OFILIM, 0, {1288, 165, 521, 37, 0, 0}},

	{FIGO, V4, TS_PANEL_UNKNOWN, 0, {1210, 173, 509, 26, 0, 0}},
	{FIGO, V4, TS_PANEL_OFILIM, 0, {1120, 161, 513, 28, 0, 0}},
	{FIGO, V4, TS_PANEL_TRULY, 0, {1210, 173, 509, 26, 0, 0}},
	{FIGO, V4, TS_PANEL_EELY, 0, {1176, 154, 559, 37, 0, 0}},
};

RPR531_ALS_PARA_TABLE rpr531_als_para_diff_tp_color_table[] ={
	{ANE, V4, TS_PANEL_UNKNOWN,
//		 C_d0  D_d0  A_d0  H_d0  C_d1  D_d1  A_d1  H_d1  C     D     A     H
		{6325, 2531, 1682, 1682, 3723, 1221, 559, 559, 800, 1450, 1890, 2318,
		 1017, 159, 5000, 200}},
	{ANE, V4, TS_PANEL_OFILIM,
		{6325, 2531, 1682, 1682, 3723, 1221, 559, 559, 800, 1450, 1890, 2318,
		 1017, 159, 5000, 200}},
	{ANE, V4, TS_PANEL_TRULY,
		{6325, 2531, 1682, 1682, 3723, 1221, 559, 559, 800, 1450, 1890, 2318,
		 1017, 159, 5000, 200}},
	{ANE, V4, TS_PANEL_MUTTO,
		{6325, 2531, 1682, 1682, 3723, 1221, 559, 559, 800, 1450, 1890, 2318,
		 1017, 159, 5000, 200}},

	{RNE, V4, TS_PANEL_UNKNOWN,
		{15626, 6167, 12261, 779, 9675, 2779, 6115, 135, 1371, 1900, 2300, 5759}},
	{RNE, V4, TS_PANEL_OFILIM,
		{15626, 6167, 12261, 779, 9675, 2779, 6115, 135, 1371, 1900, 2300, 5759}},
	{RNE, V4, TS_PANEL_JUNDA,
		{15626, 6167, 12261, 779, 9675, 2779, 6115, 135, 1371, 1900, 2300, 5759}},

	{BOND, V4, TS_PANEL_UNKNOWN,
		{12816,7653,2881,2727,7395,4179,1117,1087,1000,1580,1840,2506}},
	{BOND, V4, TS_PANEL_OFILIM,
		{12816,7653,2881,2727,7395,4179,1117,1087,1000,1580,1840,2506}},
	{BOND, V4, TS_PANEL_TRULY,
		{12870,6438,3669,1033,6744,2736,1311,177,1000,1832,2123,5814}},
	{BOND, V4, TS_PANEL_EELY,
		{14871,9598,3540,1365,8785,5490,1539,468,1000,1520,1780,2800}},

	{LELAND, V4, TS_PANEL_UNKNOWN,
		{13471,7353,2955,1763,8207,4029,1262,649,1000,1560,1950,2700}},
	{LELAND, V4, TS_PANEL_EELY,
		{13471,7353,2955,1763,8207,4029,1262,649,1000,1560,1950,2700}},
	{LELAND, V4, TS_PANEL_JUNDA,
		{12444,7645,2742,1822,7518,4141,1142,676,1000,1530,1975,2692}},
	{LELAND, V4, TS_PANEL_OFILIM,
		{14784,7899,6557,1345,8476,4099,3252,459,1000,1704,2000,2925}},

	{PIC, V4, BOE_TPLCD,
		{12428, 7032, 7398, 1520, 7410, 3534, 3644, 538, 800, 1550, 1859, 2818}},
	{PIC, V4, CTC_TPLCD,
		{12484, 7422, 3965, 13926, 6550, 3437, 1568, 6178, 800, 1768, 1980, 2253}},
	{PIC, V4, LG_TPLCD,
		{12428, 7032, 7398, 1520, 7410, 3534, 3644, 538, 800, 1550, 1859, 2818}},
	{PIC, V4, TM_TPLCD,
		{10508, 7904, 1280, 1520, 6475, 4737, 410, 538, 800, 1480, 1859, 2818}},
	{PIC, V4, DEFAULT_TPLCD,
		{10843,9754,3779,1810,6417,5571,1629,580,1000,1520,2300,3100}},

	{BAC, V4, AUO_TPLCD,
		{9419, 3843, 2435, 1520, 6472, 1814, 1050, 538, 800, 1400, 1925, 2818}},
	{BAC, V4, BOE_TPLCD,
		{8686, 4436, 2637, 1520, 6249, 2028, 1188, 538, 800, 1360, 1859, 2818}},
	{BAC, V4, CTC_TPLCD,
		{5756, 2807, 1987, 1520, 4610, 1150, 876, 538, 800, 1360, 1700, 2818}},
	{BAC, V4, TM_TPLCD,
		{12419, 5420, 3083, 1520, 8746, 2277, 1317, 538, 800, 1360, 1859, 2818}},
	{BAC, V4, DEFAULT_TPLCD,
		{10843,9754,3779,1810,6417,5571,1629,580,1000,1520,2300,3100}},

	{FIGO, V4, TS_PANEL_UNKNOWN,
		{13300,8163,3045,1547,7423,4225,1210,505,1000,1650,2125,3059}},
	{FIGO, V4, TS_PANEL_OFILIM,
		{13300,8163,3045,1547,7423,4225,1210,505,1000,1650,2125,3059}},
	{FIGO, V4, TS_PANEL_TRULY,
		{12296,6949,4185,1268,6764,3197,1702,350,1000,1700,2157,3618}},
	{FIGO, V4, TS_PANEL_EELY,
		{14313,5378,2665,1268,7873,2474,1084,350,1000,1582,2157,3618}},

	{FLORIDA, V4, TS_PANEL_UNKNOWN,
		{11982,6090,3263,1960,7220,3142,1352,739,1000,1550,1989,2650}},
	{FLORIDA, V4, TS_PANEL_OFILIM,
		{11982,6090,3263,1960,7220,3142,1352,739,1000,1550,1989,2650}},
	{FLORIDA, V4, TS_PANEL_MUTTO,
		{13570,8036,3272,1291,8360,4534,1408,434,1000,1510,1907,2966}},

	{LELANDPLUS, V4, TS_PANEL_UNKNOWN,
		{14089, 8074, 4613, 1810, 8314, 4268, 2277, 580, 1000, 1580, 2300, 3100}},
	{LELANDPLUS, V4, TS_PANEL_EELY,
		{14089, 8074, 4613, 1810, 8314, 4268, 2277, 580, 1000, 1580, 1900, 3100}},
	{LELANDPLUS, V4, TS_PANEL_OFILIM,
		{13094, 8239, 5609, 1810, 7637, 4355, 2795, 580, 1000, 1680, 1900, 3100}},
};
void key_fb_notifier_action(int enable);

enum SENSOR_POWER_CHECK {
	SENSOR_POWER_STATE_OK = 0,
	SENSOR_POWER_STATE_INIT_NOT_READY,
	SENSOR_POWER_STATE_CHECK_ACTION_FAILED,
	SENSOR_POWER_STATE_CHECK_RESULT_FAILED,
	SENSOR_POWER_STATE_NOT_PMIC,
};

static int sensor_power_pmic_flag;
static int sensor_power_init_finish;
int sensor_pmic_power_check(void)
{
	int ret = 0;
	int state = 0;
	int result = SENSOR_POWER_STATE_OK;

	if (!sensor_power_init_finish) {
		result = SENSOR_POWER_STATE_INIT_NOT_READY;
		goto out;
	}
	if (!sensor_power_pmic_flag) {
		result =  SENSOR_POWER_STATE_NOT_PMIC;
		goto out;
	}

	ret = hw_extern_pmic_query_state(1, &state);
	if (ret) {
		result = SENSOR_POWER_STATE_CHECK_ACTION_FAILED;
		goto out;
	}
	if (!state)
		result = SENSOR_POWER_STATE_CHECK_RESULT_FAILED;
out:
	hwlog_info("sensor check result:%d\n", result);
	return result;
}

static bool should_be_processed_when_sr(int sensor_tag)
{
	bool ret = true;	/*can be closed default*/
	switch (sensor_tag) {
	case TAG_PS:
	case TAG_STEP_COUNTER:
	case TAG_SIGNIFICANT_MOTION:
	case TAG_PHONECALL:
	case TAG_GPS_4774_I2C:
	case TAG_CHARGER:
	case TAG_SWITCH:
	case TAG_FP:
	case TAG_MAGN_BRACKET:
		ret = false;
		break;

	default:
		break;
	}

	return ret;
}

static DEFINE_MUTEX(mutex_update);
/*To keep same with mcu, to activate sensor need open first and then setdelay*/
void update_sensor_info(const pkt_header_t *pkt)
{
	if (TAG_SENSOR_BEGIN <= pkt->tag && pkt->tag < TAG_SENSOR_END) {
		mutex_lock(&mutex_update);
		if (CMD_CMN_OPEN_REQ == pkt->cmd) {
			sensor_status.opened[pkt->tag] = 1;
		} else if (CMD_CMN_CLOSE_REQ == pkt->cmd) {
			sensor_status.opened[pkt->tag] = 0;
			sensor_status.status[pkt->tag] = 0;
			sensor_status.delay[pkt->tag] = 0;
		} else if (CMD_CMN_INTERVAL_REQ == pkt->cmd) {
			sensor_status.delay[pkt->tag] =
			    ((const pkt_cmn_interval_req_t *)pkt)->param.period;
			sensor_status.status[pkt->tag] = 1;
			sensor_status.batch_cnt[pkt->tag] =
			    ((const pkt_cmn_interval_req_t *)pkt)->param.
			    batch_count;
		}
		mutex_unlock(&mutex_update);
	}
}

void disable_sensors_when_reboot(void)
{
	int tag = 0;
	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		if ((tag == TAG_CHARGER) || (tag == TAG_SWITCH))
			continue;
		if (sensor_status.status[tag]) {
			inputhub_sensor_enable(tag, false);
			msleep(50);
			hwlog_info("disable sensor - %d before reboot\n", tag);
		}
	}
}

void disable_sensors_when_suspend(void)
{
	int tag = 0;

	memset(&sensor_status_backup, 0, sizeof(sensor_status_backup));
	memcpy(&sensor_status_backup, &sensor_status,
	       sizeof(sensor_status_backup));
	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		if (sensor_status_backup.status[tag] && !(hifi_supported == 1 && (sensor_status.batch_cnt[tag] > 1))) {
			if (should_be_processed_when_sr(tag)) {
				inputhub_sensor_enable(tag, false);
			}
		}
	}
}

void enable_sensors_when_resume(void)
{
	int tag = 0;

	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		if (sensor_status_backup.status[tag]
		    && (0 == sensor_status.status[tag])
		    && !(hifi_supported == 1 && (sensor_status.batch_cnt[tag] > 1))) {
			if (should_be_processed_when_sr(tag)) {
				inputhub_sensor_enable(tag, true);
				inputhub_sensor_setdelay(tag,
							 sensor_status_backup.
							 delay[tag]);
			}
		}
	}
}

void __dmd_log_report(int dmd_mark, const char *err_func,
			const char *err_msg)
{
#ifdef SENSOR_DSM_CONFIG
	if (!dsm_client_ocuppy(shb_dclient)) {
		dsm_client_record(shb_dclient, "[%s]%s", err_func, err_msg);
		dsm_client_notify(shb_dclient, dmd_mark);
	}
#endif
}

static uint8_t gsensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
static uint8_t msensor_calibrate_data[MAX_MAG_CALIBRATE_DATA_LENGTH];
static uint8_t msensor_akm_calibrate_data[MAX_MAG_AKM_CALIBRATE_DATA_LENGTH];
static uint8_t ps_sensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
static uint8_t als_sensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
static uint8_t gyro_sensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
static uint8_t handpress_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
/*******************************************************************************
Function:	read_gsensor_offset_from_nv
Description:   读取NV项中的gsensor 校准数据，并发送给mcu 侧
Data Accessed:  无
Data Updated:   无
Input:         无
Output:         无
Return:         成功或者失败信息: 0->成功, -1->失败
*******************************************************************************/
int read_gsensor_offset_from_nv(void)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	memset(&user_info, 0, sizeof(user_info));
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = ACC_OFFSET_NV_NUM;
	user_info.valid_size = ACC_OFFSET_NV_SIZE;
	strncpy(user_info.nv_name, "gsensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return -1;
	}
	first_start_flag=1;
	//copy to gsensor_offset by pass
	memcpy(gsensor_offset, user_info.nv_data, sizeof(gsensor_offset));
	hwlog_info
	    ("nve_direct_access read gsensor_offset_sen (%d %d %d %d %d %d)\n",
	     gsensor_offset[0], gsensor_offset[1], gsensor_offset[2],
	     gsensor_offset[3], gsensor_offset[4], gsensor_offset[5]);
	hwlog_info
	("nve_direct_access read gsensor_xis_angle (%d %d %d %d %d %d %d %d %d)\n",
		gsensor_offset[6],gsensor_offset[7],gsensor_offset[8],gsensor_offset[9],gsensor_offset[10],
		gsensor_offset[11],gsensor_offset[12],gsensor_offset[13],gsensor_offset[14]);
/*
	gsensor_data.offset_x = gsensor_offset[0];
	gsensor_data.offset_y = gsensor_offset[1];
	gsensor_data.offset_z = gsensor_offset[2];
	gsensor_data.sensitivity_x = gsensor_offset[3];
	gsensor_data.sensitivity_y = gsensor_offset[4];
	gsensor_data.sensitivity_z = gsensor_offset[5];
*/
	pkg_ap.tag = TAG_ACCEL;
	pkg_ap.cmd = CMD_ACCEL_OFFSET_REQ;
	pkg_ap.wr_buf = gsensor_offset;
	/*pkg_ap.wr_buf=gsensor_offset;*/
	pkg_ap.wr_len = sizeof(int) * 15;
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	memcpy(&gsensor_calibrate_data, pkg_ap.wr_buf,
	       (sizeof(gsensor_calibrate_data) <
		pkg_ap.wr_len) ? sizeof(gsensor_calibrate_data) : pkg_ap.
	       wr_len);
	if (ret) {
		hwlog_err
			("send gsensor offset data to mcu fail,ret=%d\n", ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("set gsensor offset fail,err=%d\n", pkg_mcu.errno);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "set gsensor offset fail\n");
	} else {
		hwlog_info("send gsensor offset data to mcu success\n");
	}
	return 0;
}

/*******************************************************************************
Function:	write_gsensor_offset_to_nv
Description:  将temp数据写入NV 项中
Data Accessed:  无
Data Updated:   无
Input:        g-sensor 校准值
Output:         无
Return:         成功或者失败信息: 0->成功, -1->失败
*******************************************************************************/
int write_gsensor_offset_to_nv(char *temp, int length)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	const int32_t *poffset_data = (const int32_t *)user_info.nv_data;

	if (temp == NULL || length < 0) {
		hwlog_err("write_gsensor_offset_to_nv fail, invalid para!\n");
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			"write_gsensor_offset_to_nv fail, invalid para!\n");
		return -1;
	}
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = ACC_OFFSET_NV_NUM;
	user_info.valid_size = ACC_OFFSET_NV_SIZE;
	strncpy(user_info.nv_name, "gsensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';

	/*copy to nv by pass*/
	memcpy(user_info.nv_data, temp,
		sizeof(user_info.nv_data) < length ? sizeof(user_info.nv_data):length);
	memcpy(&gsensor_calibrate_data, temp,
		MAX_SENSOR_CALIBRATE_DATA_LENGTH < length ? MAX_SENSOR_CALIBRATE_DATA_LENGTH:length);
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access write error(%d)\n", ret);
		return -1;
	}
	hwlog_info("nve_direct_access write temp (%d %d %d  %d %d %d %d %d %d  %d %d %d %d %d %d)\n",
		   *poffset_data, *(poffset_data + 1), *(poffset_data + 2),*(poffset_data + 3), *(poffset_data + 4),
		   *(poffset_data + 5),*(poffset_data + 6), *(poffset_data + 7),*(poffset_data + 8),*(poffset_data + 9),
		   *(poffset_data + 10),*(poffset_data + 11),*(poffset_data + 12), *(poffset_data + 13),*(poffset_data + 14));
	return ret;
}

int read_gyro_sensor_offset_from_nv(void)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;

	memset(&user_info, 0, sizeof(user_info));
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = GYRO_CALIDATA_NV_NUM;
	user_info.valid_size = GYRO_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "GYRO", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return -1;
	}
	gyro_first_start_flag = 1;
	/*copy to gsensor_offset by pass*/
	memcpy(gyro_sensor_offset, user_info.nv_data,
	       sizeof(gyro_sensor_offset));
	hwlog_info( "nve_direct_access read gyro_sensor offset: %d %d %d  sensitity:%d %d %d \n",gyro_sensor_offset[0],gyro_sensor_offset[1],gyro_sensor_offset[2],
            gyro_sensor_offset[3],gyro_sensor_offset[4],gyro_sensor_offset[5]);
	hwlog_info( "nve_direct_access read gyro_sensor xis_angle: %d %d %d  %d %d %d %d %d %d \n",gyro_sensor_offset[6],gyro_sensor_offset[7],gyro_sensor_offset[8],
            gyro_sensor_offset[9],gyro_sensor_offset[10],gyro_sensor_offset[11],gyro_sensor_offset[12],gyro_sensor_offset[13],gyro_sensor_offset[14]);

	pkg_ap.tag = TAG_GYRO;
	pkg_ap.cmd = CMD_GYRO_OFFSET_REQ;
	pkg_ap.wr_buf = gyro_sensor_offset;
	pkg_ap.wr_len = sizeof(int) * 15;
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	memcpy(&gyro_sensor_calibrate_data, pkg_ap.wr_buf,
	       (sizeof(gyro_sensor_calibrate_data) <
		pkg_ap.wr_len) ? sizeof(gyro_sensor_calibrate_data) :
		pkg_ap.wr_len);
	if (ret) {
		hwlog_err("send gyro_sensor offset data to mcu fail,ret=%d\n",
			  ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("set gyro_sensor offset fail,err=%d\n",
			  pkg_mcu.errno);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "set gyro_sensor offset fail\n");
	} else {
		hwlog_info("send gyro_sensor offset data to mcu success\n");
	}
	return 0;
}

int write_gyro_sensor_offset_to_nv(char *temp, int length)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	const int32_t *poffset_data = (const int32_t *)user_info.nv_data;

	if (temp == NULL) {
		hwlog_err
		    ("write_gyro_sensor_offset_to_nv fail, invalid para!\n");
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			"write_gyro_sensor_offset_to_nv fail, invalid para!\n");
		return -1;
	}
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = GYRO_CALIDATA_NV_NUM;
	user_info.valid_size = GYRO_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "GYRO", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';

	/*copy to nv by pass*/
	memcpy(user_info.nv_data, temp,
	       sizeof(user_info.nv_data) <
	       length ? sizeof(user_info.nv_data) : length);
	memcpy(&gyro_sensor_calibrate_data, temp,
	       sizeof(user_info.nv_data) <
	       length ? sizeof(user_info.nv_data) : length);
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access write error(%d)\n", ret);
		return -1;
	}
	hwlog_info( "gyro calibrate data (%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d )\n", *poffset_data, *(poffset_data + 1), *(poffset_data + 2), *(poffset_data + 3),
            *(poffset_data + 4),*(poffset_data + 5), *(poffset_data + 6), *(poffset_data + 7), *(poffset_data + 8),*(poffset_data + 9),*(poffset_data + 10),
            *(poffset_data + 11), *(poffset_data + 12), *(poffset_data + 13),*(poffset_data + 14));
	return ret;
}

int read_mag_calibrate_data_from_nv(void)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	memset(&user_info, 0, sizeof(user_info));
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	/*read from nv*/
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = MAG_CALIBRATE_DATA_NV_NUM;
	if (1 == akm_cal_algo) {
		user_info.valid_size = MAG_AKM_CALIBRATE_DATA_NV_SIZE;
	} else {
		user_info.valid_size = MAG_CALIBRATE_DATA_NV_SIZE;
	}
	strncpy(user_info.nv_name, "msensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return -1;
	}
	/*send to mcu*/
	pkg_ap.tag = TAG_MAG;
	pkg_ap.cmd = CMD_MAG_SET_CALIBRATE_TO_MCU_REQ;
	pkg_ap.wr_buf = &user_info.nv_data;
	if (1 == akm_cal_algo) {
		pkg_ap.wr_len = MAG_AKM_CALIBRATE_DATA_NV_SIZE;
		memcpy(&msensor_akm_calibrate_data, pkg_ap.wr_buf,
		       sizeof(msensor_akm_calibrate_data));
	} else {
		pkg_ap.wr_len = MAG_CALIBRATE_DATA_NV_SIZE;
		memcpy(&msensor_calibrate_data, pkg_ap.wr_buf,
		       sizeof(msensor_calibrate_data));
	}
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	if (ret != 0) {
		hwlog_err("set mag_sensor data failed, ret = %d!\n", ret);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "set mag_sensor data failed\n");
		return -1;
	}

	if (pkg_mcu.errno != 0) {
		hwlog_err("set mag offset fail,err=%d\n", pkg_mcu.errno);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "set mag offset fail\n");
	} else {
		hwlog_info("send mag_sensor data %d, %d, %d to mcu success\n",
			   *((uint32_t *) pkg_ap.wr_buf),
			   *((uint32_t *) pkg_ap.wr_buf + 1),
			   *((uint32_t *) pkg_ap.wr_buf + 2));
	}

	return 0;
}

int write_magsensor_calibrate_data_to_nv(const char *src)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;

	if (NULL == src) {
		hwlog_err("%s fail, invalid para!\n", __func__);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "invalid para\n");
		return -1;
	}

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = MAG_CALIBRATE_DATA_NV_NUM;
	if (1 == akm_cal_algo) {
		user_info.valid_size = MAG_AKM_CALIBRATE_DATA_NV_SIZE;
		memcpy(&msensor_akm_calibrate_data, src, sizeof(msensor_akm_calibrate_data));
	} else {
		user_info.valid_size = MAG_CALIBRATE_DATA_NV_SIZE;
		memcpy(&msensor_calibrate_data, src, sizeof(msensor_calibrate_data));
	}
	strncpy(user_info.nv_name, "msensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	memcpy(user_info.nv_data, src,
	       sizeof(user_info.nv_data) <
	       user_info.valid_size ? sizeof(user_info.nv_data) : user_info.
	       valid_size);
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access write error(%d)\n", ret);
		return -1;
	}

	return ret;
}

static int median_in_current_storage(int *current_storage, unsigned int current_storage_size)
{
	int ret = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	int temp =0;
	int median = (current_storage_size+1)/2-1;     //for the median in this array

	if((current_storage == NULL) ||(median < 0))
	{
		return ret;
	}

	for(; i < current_storage_size-1; ++i)
	{
		for(j =i+1; j < current_storage_size; ++j)
		{
			if(current_storage[i] > current_storage[j])
			{
				temp = current_storage[j];
				current_storage[j] = current_storage[i];
				current_storage[i] = temp;
			}
		}
	}
	ret = current_storage[median];
	return ret;
}

static void send_mag_charger_to_mcu(void)
{
	int ret = 0;
	write_info_t pkg_ap;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	/*send to mcu*/
	pkg_ap.tag = TAG_MAG;
	pkg_ap.cmd = CMD_MAG_SET_CALIBRATE_TO_MCU_REQ;
	pkg_ap.wr_buf =&charge_current_data ;
	pkg_ap.wr_len = sizeof(charge_current_data);

	ret = write_customize_cmd(&pkg_ap, NULL);
	if (ret != 0) {
		hwlog_err("notify mag environment change failed, ret = %d!\n",ret);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			"notify mag environment change failed\n");
	} else {
			hwlog_info ("magnetic %s event ! current_offset = %d, %d, %d\n",charge_current_data.str_charge,
				charge_current_data.current_offset_x,charge_current_data.current_offset_y,charge_current_data.current_offset_z);
		}
}

static int send_current_to_mcu_mag(int current_value_now)
{
	current_value_now = -current_value_now;
	if(current_value_now<CURRENT_MIN_VALUE || current_value_now>CURRENT_MAX_VALUE){
		return SEND_ERROR;
	}

	if(mag_current_send_time >= MAX_STORGE_TIME){
		if(mag_current_storage_time < MAX_STORGE_TIME){
			mag_current_storage[mag_current_storage_time] = current_value_now;
			mag_current_storage_time++;
			return SEND_SUC;
		}else{
			mag_current_storage_time = 0;
			current_value_now = median_in_current_storage(mag_current_storage,MAX_STORGE_TIME);
		}
	}else{
		mag_current_send_time++;
	}

	charge_current_data.current_offset_x = current_value_now * akm_current_x_fac / MAG_CURRENT_FAC_RAIO;
	charge_current_data.current_offset_y = current_value_now * akm_current_y_fac / MAG_CURRENT_FAC_RAIO;
	charge_current_data.current_offset_z = current_value_now * akm_current_z_fac / MAG_CURRENT_FAC_RAIO;

	if(((charge_current_data.current_offset_x != current_mag_x_pre)
		|| (charge_current_data.current_offset_y !=current_mag_y_pre)
		|| (charge_current_data.current_offset_z !=current_mag_z_pre))
		&& iom3_sr_status == ST_SCREENON
		&& mag_opend)
	{
		current_mag_x_pre = charge_current_data.current_offset_x;
		current_mag_y_pre = charge_current_data.current_offset_y;
		current_mag_z_pre = charge_current_data.current_offset_z;
		send_mag_charger_to_mcu();
	}
	return SEND_SUC;
}

int mag_enviroment_change_notify(struct notifier_block *nb,
			unsigned long action, void *data)
{
	int ret = 0;
	write_info_t pkg_ap;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	if(CAL_BY_CHARGER==mag_data.charger_trigger){
		if(action >= CHARGER_TYPE_NONE){
			close_send_current();
			mag_current_send_time = 0;
			mag_current_storage_time = 0;
			memset(&charge_current_data,0,sizeof(charge_current_data));
			memcpy(charge_current_data.str_charge,str_charger_current_out,sizeof(str_charger_current_out));
			send_mag_charger_to_mcu();
		}else{
			memset(&charge_current_data,0,sizeof(charge_current_data));
			memcpy(charge_current_data.str_charge,str_charger_current_in,sizeof(str_charger_current_in));
			open_send_current(send_current_to_mcu_mag);
		}
	}else if(NO_CAL_BY_CHARGER == mag_data.charger_trigger){
		/*send to mcu*/
		pkg_ap.tag = TAG_MAG;
		pkg_ap.cmd = CMD_MAG_SET_CALIBRATE_TO_MCU_REQ;
		pkg_ap.wr_buf = str_charger;
		pkg_ap.wr_len = sizeof(str_charger);

		ret = write_customize_cmd(&pkg_ap, NULL);
		if (ret != 0) {
			hwlog_err("notify mag environment change failed, ret = %d!\n",ret);
			__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__, "notify mag environment change failed\n");
			return -1;
		} else {
			hwlog_err("magnetic environmnet changed, notify successfully\n");
		}
	}
	return 0;
}

int read_ps_offset_from_nv(void)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	memset(&user_info, 0, sizeof(user_info));
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = PS_CALIDATA_NV_NUM;
	user_info.valid_size = PS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "PSENSOR", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return -1;
	}
	ps_first_start_flag = 1;

	memcpy(ps_sensor_offset, user_info.nv_data,
			sizeof(ps_sensor_offset));
	hwlog_info( "nve_direct_access read ps_offset (%d,%d,%d)\n",
			ps_sensor_offset[0], ps_sensor_offset[1],
			ps_sensor_offset[2]);
	if(ltr578_ps_external_ir_calibrate_flag == 1)
	{
		ps_external_ir_param.external_ir_pwindows_value = ps_sensor_offset[2] - ps_sensor_offset[1];
		ps_external_ir_param.external_ir_pwave_value = ps_sensor_offset[1] - ps_sensor_offset[0];
		ps_external_ir_param.external_ir_calibrate_noise = ps_sensor_offset[0];
		hwlog_info("%s:set ltr578 offset ps_data[0]:%d,ps_data[1]:%d,ps_data[2]:%d,pwindows:%d,pwave:%d\n",
				__func__,ps_sensor_offset[0],ps_sensor_offset[1],ps_sensor_offset[2],\
				ps_external_ir_param.external_ir_pwindows_value,ps_external_ir_param.external_ir_pwave_value);
	}

	pkg_ap.tag=TAG_PS;
	pkg_ap.cmd=CMD_PS_OFFSET_REQ;
	pkg_ap.wr_buf=ps_sensor_offset;
	pkg_ap.wr_len=sizeof(ps_sensor_offset);
	ret=write_customize_cmd(&pkg_ap,  &pkg_mcu);
	memcpy(&ps_sensor_calibrate_data, pkg_ap.wr_buf,
	       (sizeof(ps_sensor_calibrate_data) <
		pkg_ap.wr_len) ? sizeof(ps_sensor_calibrate_data) :
		pkg_ap.wr_len);
	if (ret) {
		hwlog_err("send psensor offset data to mcu fail,ret=%d\n",
			ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("set psensor offset fail,err=%d\n", pkg_mcu.errno);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "set psensor offset fail\n");
	} else {
		hwlog_info("send psensor offset data to mcu success\n");
	}
	return 0;
}

int write_ps_offset_to_nv(int *temp)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	const int *poffset_data = (const int *)user_info.nv_data;

	if (temp == NULL) {
		hwlog_err("write_ps_offset_to_nv fail, invalid para!\n");
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "write_ps_offset_to_nv fail, invalid para!\n");
		return -1;
	}
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = PS_CALIDATA_NV_NUM;
	user_info.valid_size = PS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "PSENSOR", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';

	hwlog_info("write_ps_offset_to_nv temp: %d,%d,%d\n",
				temp[0],temp[1],temp[2]);

	memcpy(user_info.nv_data, temp, sizeof(temp[0]) * 3);
	memcpy(&ps_sensor_calibrate_data, temp, sizeof(temp[0]) * 3);
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access write error(%d)\n", ret);
		return -1;
	}

	hwlog_info( "nve_direct_access write temp (%d,%d,%d)\n",
				*poffset_data, *(poffset_data + 1),
				*(poffset_data + 2));

	msleep(10);
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = PS_CALIDATA_NV_NUM;
	user_info.valid_size = PS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "PSENSOR", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve direct access read error(%d)\n", ret);
		return -1;
	}

	if((*poffset_data != temp[0]) || (*(poffset_data+1) != temp[1]) ||
		(*(poffset_data+2) != temp[2])) {
		hwlog_err("nv write fail, (%d,%d,%d)\n", *poffset_data,
					*(poffset_data+1),*(poffset_data+2));
		return -1;
	}
	return ret;
}

int read_als_offset_from_nv(void)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;

	memset(&user_info, 0, sizeof(user_info));
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = ALS_CALIDATA_NV_NUM;
	user_info.valid_size = ALS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "LSENSOR", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return -1;
	}

	als_first_start_flag = 1;
	memcpy(als_offset, user_info.nv_data, sizeof(als_offset));
	hwlog_info
	    ("nve_direct_access read lsensor_offset (%d %d %d %d %d %d)\n",
	     als_offset[0], als_offset[1], als_offset[2], als_offset[3],
	     als_offset[4], als_offset[5]);

	pkg_ap.tag = TAG_ALS;
	pkg_ap.cmd = CMD_ALS_OFFSET_REQ;
	pkg_ap.wr_buf = als_offset;
	pkg_ap.wr_len = ALS_CALIDATA_NV_SIZE;
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	memcpy(&als_sensor_calibrate_data, pkg_ap.wr_buf,
	       (sizeof(als_sensor_calibrate_data) <
		pkg_ap.wr_len) ? sizeof(als_sensor_calibrate_data) :
		pkg_ap.wr_len);
	if (ret) {
		hwlog_err("send als offset data to mcu fail,ret=%d\n", ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("set als offset fail,err=%d\n", pkg_mcu.errno);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "set als offset fail\n");
	} else {
		hwlog_info("send als offset data to mcu success\n");
	}
	return 0;
}

int write_als_offset_to_nv(char *temp)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	const uint16_t *poffset_data = (const uint16_t *)user_info.nv_data;

	if (temp == NULL) {
		hwlog_err("write_als_offset_to_nv fail, invalid para!\n");
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "write_als_offset_to_nv fail, invalid para!\n");
		return -1;
	}
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = ALS_CALIDATA_NV_NUM;
	user_info.valid_size = ALS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "LSENSOR", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';

	/*copy to nv by pass*/
	memcpy(user_info.nv_data, temp, sizeof(user_info.nv_data));
	memcpy(&als_sensor_calibrate_data, temp, sizeof(user_info.nv_data));
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access write error(%d)\n", ret);
		return -1;
	}
	hwlog_info("nve_direct_access als write temp (%d %d %d %d %d %d  )\n",
		   *poffset_data, *(poffset_data + 1), *(poffset_data + 2),
		   *(poffset_data + 3), *(poffset_data + 4),
		   *(poffset_data + 5));
	return ret;
}

int write_handpress_offset_to_sensorhub(void* offset)
{
	int ret = 0;
	write_info_t	pkg_ap;
	read_info_t	pkg_mcu;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_HANDPRESS;
	pkg_ap.cmd = CMD_HANDPRESS_SET_CALIDATA_REQ;
	pkg_ap.wr_buf = offset;
	pkg_ap.wr_len = HANDPRESS_CALIDATA_NV_SIZE;
	ret = write_customize_cmd(&pkg_ap,  &pkg_mcu);
	if(ret) {
		hwlog_err("send hp offset data to mcu fail,ret=%d\n", ret);
		ret =  -1;
	}
	if(pkg_mcu.errno != 0) {
		hwlog_err("set hp offset fail,err=%d\n", pkg_mcu.errno);
		ret = -1;
	} else
		hwlog_info("send hp offset data to mcu success\n");

	return ret;
}

/*
* handpress nv read write
*/
int read_handpress_offset_from_nv(void)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;

	hwlog_info("[%s] ++\n", __func__);

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = HANDPRESS_CALIDATA_NV_NUM;
	user_info.valid_size = HANDPRESS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "HPDATA", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	if ((ret = hisi_nve_direct_access(&user_info))!=0)
	{
		hwlog_err("nve handpress read error(%d)\n", ret);
		return -1;
	}
	handpress_first_start_flag = 1;
	memcpy(hp_offset, user_info.nv_data, HANDPRESS_CALIDATA_NV_SIZE);
	memcpy(&handpress_calibrate_data, hp_offset,
	       (sizeof(handpress_calibrate_data) <
		sizeof(hp_offset)) ? sizeof(handpress_calibrate_data) :
		sizeof(hp_offset));
	ret = write_handpress_offset_to_sensorhub(hp_offset);
	if (ret)
		return -1;

	hwlog_err("[%s] +--\n", __func__);

	return 0;
}

int write_handpress_offset_to_nv(void* offset)
{
	int ret = 0;
	int8_t offset2[24] = {0};
	struct hisi_nve_info_user user_info;

	if(NULL == offset) {
		hwlog_err("write_handpress_offset_to_nv fail, invalid para!\n");
		return -1;
	}
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = HANDPRESS_CALIDATA_NV_NUM;
	user_info.valid_size = HANDPRESS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "HPDATA", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';

	memcpy(offset2, offset, HANDPRESS_CALIDATA_NV_SIZE);
	memcpy(user_info.nv_data, offset2, HANDPRESS_CALIDATA_NV_SIZE);
	if ((ret = hisi_nve_direct_access(&user_info))!=0)
	{
		hwlog_err("nve_direct_access write error(%d)\n", ret);
		return -1;
	}
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = HANDPRESS_CALIDATA_NV_NUM;
	user_info.valid_size = HANDPRESS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "HPDATA", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	if ((ret = hisi_nve_direct_access(&user_info))!=0)
	{
		hwlog_err("nve handpress read error(%d)\n", ret);
		return -1;
	}
	memset(offset2, 0, sizeof(offset2));
	memcpy(offset2, user_info.nv_data, sizeof(offset2));
	hwlog_err("offsets: %d%d%d%d%d%d%d%d\n",
		offset2[0], offset2[1], offset2[2], offset2[3],
		offset2[4], offset2[5], offset2[6], offset2[7]);

	return ret;
}

/*----------------------------------------------------------------------------*/
void enable_sensors_when_recovery_iom3(void)
{
	int tag;

	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		if ((tag == TAG_CHARGER) || (tag == TAG_SWITCH))
			continue;
		if (sensor_status.status[tag]) {
			inputhub_sensor_enable_nolock(tag, true);
			inputhub_sensor_setdelay_nolock(tag,
							sensor_status.
							delay[tag],
							sensor_status.
							batch_cnt[tag]);
		}
	}
}

void enable_key_when_recovery_iom3(void)
{
	int ret = 0;
	pkt_header_t pkt;
	pkt_cmn_close_req_t close_pkt;
	pkt_cmn_interval_req_t interval_pkt;
	open_param_t open_param;
	close_param_t close_param;

	memset(&open_param, 0, sizeof(open_param));
	memset(&close_param, 0, sizeof(close_param));
	if (strlen(sensor_chip_info[KEY]) == 0) {
		hwlog_err("no key\n");
		return;
	}

	hwlog_info("enable_key_when_recovery_iom3 ++.\n");
	if (key_state) {//	open
		memset(&pkt, 0, sizeof(pkt));
		pkt.tag = TAG_KEY;
		pkt.cmd = CMD_CMN_OPEN_REQ;
		pkt.resp = NO_RESP,
		pkt.length = 0;
		ret = inputhub_mcu_write_cmd_nolock(&pkt, sizeof(pkt));
		if (ret < 0) {
			hwlog_err("write open cmd err.\n");
			return;
		}
		memset(&interval_pkt, 0, sizeof(interval_pkt));
		interval_pkt.hd.tag = TAG_KEY;
		interval_pkt.hd.cmd = CMD_CMN_INTERVAL_REQ;
		interval_pkt.hd.resp = NO_RESP;
		interval_pkt.hd.length = sizeof(open_param_t);
		open_param.period = 20;
		open_param.mode = AUTO_MODE;
		open_param.batch_count = 1;
		memcpy(&interval_pkt.param, &open_param, sizeof(interval_pkt.param));
		ret = inputhub_mcu_write_cmd_nolock(&interval_pkt, sizeof(interval_pkt));
		if (ret < 0)
			hwlog_err("write interval cmd err.\n");
	} else {	//close
		memset(&close_pkt, 0, sizeof(close_pkt));
		close_pkt.hd.tag = TAG_KEY;
		close_pkt.hd.cmd = CMD_CMN_CLOSE_REQ;
		close_pkt.hd.resp = NO_RESP;
		close_pkt.hd.length = sizeof(close_param_t);
		memcpy(&close_pkt.close_param, &close_param,
			       sizeof(close_pkt.close_param));
		ret = inputhub_mcu_write_cmd_nolock(&close_pkt, sizeof(close_pkt));
		if (ret < 0)
			hwlog_err("write close cmd err.\n");
	}
	hwlog_info("enable_key_when_recovery_iom3 --.\n");
}

void disable_key_when_reboot(void)
{
	int ret = 0;
	pkt_cmn_close_req_t close_pkt;
	close_param_t close_param;

	memset(&close_param, 0, sizeof(close_param));
	if (strlen(sensor_chip_info[KEY]) == 0) {
		hwlog_err("no key\n");
		return;
	}

	memset(&close_pkt, 0, sizeof(close_pkt));
	close_pkt.hd.tag = TAG_KEY;
	close_pkt.hd.cmd = CMD_CMN_CLOSE_REQ;
	close_pkt.hd.resp = NO_RESP;
	close_pkt.hd.length = sizeof(close_param_t);
	memcpy(&close_pkt.close_param, &close_param,
		       sizeof(close_pkt.close_param));
	ret = inputhub_mcu_write_cmd_nolock(&close_pkt, sizeof(close_pkt));
	if (ret < 0)
		hwlog_err("write close cmd err.\n");

	hwlog_info("close key when reboot\n");
}

int write_customize_cmd_noresp(int tag, int cmd, const void *data,
			int length)
{
	uint8_t buf[MAX_PKT_LENGTH] = { 0 };

	if (length + sizeof(pkt_header_t) > MAX_PKT_LENGTH) {
		hwlog_err("-----------> length = %d is too large in %s\n",
			  length, __func__);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CMD, __func__,
			       "-------> write length is too large\n");
		return -EINVAL;
	}

	((pkt_header_t *) buf)->tag = tag;
	((pkt_header_t *) buf)->cmd = cmd;
	((pkt_header_t *) buf)->resp = NO_RESP;
	((pkt_header_t *) buf)->length = length;
	memcpy(buf + sizeof(pkt_header_t), data,
	       ((pkt_header_t *) buf)->length);
	return inputhub_mcu_write_cmd_nolock(&buf,
					     sizeof(pkt_header_t) +
					     ((pkt_header_t *) buf)->length);
}

void update_fingersense_zaxis_data(s16 *buffer, int nsamples)
{
	unsigned long flags = 0;

	if (nsamples < 0) {
		hwlog_err
		    ("The second parameter of %s is wrong(negative number)\n",
		     __func__);
		return;
	}

	spin_lock_irqsave(&fsdata_lock, flags);

	memcpy(fingersense_data, buffer,
	       min(nsamples, FINGERSENSE_DATA_NSAMPLES) * sizeof(*buffer));
	fingersense_data_ready = true;
	fingersense_data_intrans = false;

	spin_unlock_irqrestore(&fsdata_lock, flags);
}

void reset_calibrate_when_recovery_iom3(void)
{
	write_customize_cmd_noresp(TAG_ACCEL, CMD_ACCEL_OFFSET_REQ,
				   &gsensor_calibrate_data, sizeof(int) * 15);
	if (1 == akm_cal_algo) {
		write_customize_cmd_noresp(TAG_MAG,
					   CMD_MAG_SET_CALIBRATE_TO_MCU_REQ,
					   &msensor_akm_calibrate_data,
					   MAG_AKM_CALIBRATE_DATA_NV_SIZE);
	} else {
		write_customize_cmd_noresp(TAG_MAG,
					   CMD_MAG_SET_CALIBRATE_TO_MCU_REQ,
					   &msensor_calibrate_data,
					   MAG_CALIBRATE_DATA_NV_SIZE);
	}
	if (txc_ps_flag == 1 || ams_tmd2620_ps_flag == 1 || avago_apds9110_ps_flag == 1) {
		write_customize_cmd_noresp(TAG_PS, CMD_PS_OFFSET_REQ,
					   &ps_sensor_calibrate_data,
					   PS_CALIDATA_NV_SIZE);
	}
	if (rohm_rgb_flag == 1 || avago_rgb_flag == 1 || is_cali_supported == 1) {
		write_customize_cmd_noresp(TAG_ALS, CMD_ALS_OFFSET_REQ,
					   &als_sensor_calibrate_data,
					   sizeof(int) * 3);
	}
	if (strlen(sensor_chip_info[GYRO])) {
		write_customize_cmd_noresp(TAG_GYRO, CMD_GYRO_OFFSET_REQ,
					   &gyro_sensor_calibrate_data,
					   sizeof(int) * 15);
	}
	write_customize_cmd_noresp(TAG_PRESSURE,
				CMD_AIRPRESS_SET_CALIDATA_REQ,
				&airpress_data, sizeof(airpress_data));
	if (strlen(sensor_chip_info[HANDPRESS])) {
		write_customize_cmd_noresp(TAG_HANDPRESS,
				CMD_HANDPRESS_SET_CALIDATA_REQ,
				&handpress_calibrate_data,
				HANDPRESS_CALIDATA_NV_SIZE);
	}
	if (strlen(sensor_chip_info[CAP_PROX])) {
		write_customize_cmd_noresp(TAG_CAP_PROX,
			CMD_CAP_PROX_SET_CALIDATA_REQ,
			&sar_calibrate_datas,
			sizeof(sar_calibrate_datas));
	}
}

int detect_i2c_device(struct device_node *dn, char *device_name)
{
	int i = 0, ret = 0, i2c_address = 0, i2c_bus_num = 0;
	int register_add = 0, len = 0;
	u32 wia[10] = { 0 };
	uint8_t detected_device_id;
	struct property *prop = NULL;

	if (of_property_read_u32(dn, "bus_number", &i2c_bus_num)
	    || of_property_read_u32(dn, "reg", &i2c_address)
	    || of_property_read_u32(dn, "chip_id_register", &register_add)) {
		hwlog_err
		    ("%s:read i2c bus_number (%d)or bus address(%x) or chip_id_register(%x) from dts fail\n",
		     device_name, i2c_bus_num, i2c_address, register_add);
		__dmd_log_report(DSM_SHB_ERR_IOM7_DYNLOAD, __func__,
			 "read i2c bus_number or bus address or chip_id_register from dts fail\n");
		return -1;
	}

	prop = of_find_property(dn, "chip_id_value", NULL);
	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;
	len = prop->length / 4;

	if (of_property_read_u32_array(dn, "chip_id_value", wia, len)) {
		hwlog_err
		    ("%s:read chip_id_value (id0=%x) from dts fail len=%d\n",
		     device_name, wia[0], len);
		__dmd_log_report(DSM_SHB_ERR_IOM7_DYNLOAD, __func__,
			       "read chip_id_value from dts fail\n");
		return -1;
	}

	hwlog_info
	    ("%s:read i2c bus_number (%d) slave address(0x%x) chip_id_register(0x%x) chipid value 0x%x 0x%x 0x%x 0x%x\n",
	     device_name, i2c_bus_num, i2c_address, register_add, wia[0],
	     wia[1], wia[2], wia[3]);
	ret = mcu_i2c_rw((uint8_t)i2c_bus_num, (uint8_t)i2c_address, (uint8_t*)&register_add, 1, &detected_device_id, 1);
	if (ret) {
		hwlog_err
		    ("%s:detect_i2c_device:send i2c read cmd to mcu fail,ret=%d\n",
		     device_name, ret);
		return -1;
	}
	for (i = 0; i < len; i++) {
		if ((detected_device_id == (char)wia[i]) || ((detected_device_id & CHIP_VERSION_MASK) == (char)wia[i])) {
			hwlog_info
			    ("%s:i2c detect  suc!chip_value:0x%x\n",
			     device_name, detected_device_id);
			return 0;
		}
	}
	hwlog_info("%s:i2c detect fail,chip_value:0x%x,len:%d!\n",
		device_name, detected_device_id, len);
	return -1;
}

int send_fileid_to_mcu(void)
{
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	int ret = 0;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	dyn_req->end = 1;
	pkg_ap.tag = TAG_SYS;
	pkg_ap.cmd = CMD_SYS_DYNLOAD_REQ;
	pkg_ap.wr_buf = &(dyn_req->end);
	pkg_ap.wr_len = dyn_req->file_count * sizeof(dyn_req->file_list[0])
	    + sizeof(dyn_req->end) + sizeof(dyn_req->file_count);

	if (g_iom3_state == IOM3_ST_RECOVERY ||
		iom3_power_state == ST_SLEEP) {
		ret =
		    write_customize_cmd_noresp(TAG_SYS, CMD_SYS_DYNLOAD_REQ,
					       pkg_ap.wr_buf, pkg_ap.wr_len);
	} else {
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	}

	if (ret) {
		hwlog_err("send file id to mcu fail,ret=%d\n", ret);
		__dmd_log_report(DSM_SHB_ERR_IOM7_DYNLOAD, __func__,
			       "send file id to mcu fail\n");
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("file id set fail\n");
		__dmd_log_report(DSM_SHB_ERR_IOM7_DYNLOAD, __func__,
			       "file id set fail\n");
		return -1;
	}

	return 0;
}

#if 0
static bool check_sensorhub_isensor_version(void)
{
	int len = 0;
	struct device_node *sensorhub_node = NULL;
	const char *is_isensor = NULL;
	sensorhub_node =
	    of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "can't find node sensorhub\n");
		return false;
	}
	is_isensor = of_get_property(sensorhub_node, "isensor_version", &len);
	if (!is_isensor) {
		hwlog_err("%s, can't find property isensor_version\n",
			  __func__);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "can't find property isensor_version\n");
		return false;
	}
	if (strstr(is_isensor, "yes")) {
		hwlog_info("%s, sensorhub is isensor interface version\n",
			   __func__);
		return true;
	} else {
		hwlog_info("%s, sensorhub is not isensor interface version\n",
			   __func__);
		return false;
	}
}
#endif
static int check_sensor_1V8_from_pmic(void)
{
	int len = 0, ret = 0, i = 0;
	int sensor_1v8_flag = 0, state = 0;
	int sensor_1v8_ldo = 0;
	struct device_node *sensorhub_node = NULL;
	const char *sensor_1v8_from_pmic = NULL;
	sensorhub_node =
	    of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "can't find node sensorhub\n");
		return 0;
	}
	sensor_1v8_from_pmic = of_get_property(sensorhub_node, "sensor_1v8_from_pmic", &len);
	if (!sensor_1v8_from_pmic) {
		hwlog_info("%s, can't find property sensor_1v8_from_pmic\n",
			  __func__);
		return 1;
	}
	sensor_power_pmic_flag = 1;
	if (strstr(sensor_1v8_from_pmic, "yes")) {
		hwlog_info("%s, sensor_1v8 from pmic\n",__func__);
		if (of_property_read_u32(sensorhub_node, "sensor_1v8_ldo", &sensor_1v8_ldo)){
			hwlog_err("%s,read sensor_1v8_ldo fail\n",__func__);
			return 0;
		} else {
			hwlog_info("%s,read sensor_1v8_ldo(%d) succ\n",__func__,sensor_1v8_ldo);
			sensor_1v8_flag = 1;
		}
	} else {
		hwlog_info("%s, sensor_1v8 not from pmic\n",
			   __func__);
		return 1;
	}

	if(sensor_1v8_flag){
		for(i = 0; i < 10; i++){
			ret = hw_extern_pmic_query_state(sensor_1v8_ldo, &state);
			if(state){
				hwlog_info("sensor_1V8 set high succ!\n");
				break;
			}
			msleep(200);
		}
		if(i==10 && state == 0){
			hwlog_err("sensor_1V8 set high fail,ret:%d!\n",ret);
			return 0;
		}
	}
	sensor_power_init_finish = 1;
	return 1;
}

int mcu_i2c_rw(uint8_t bus_num, uint8_t i2c_add,
	uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len)
{
	int ret;
	struct sensor_combo_cfg cfg;
	cfg.bus_type = TAG_I2C;
	cfg.bus_num = bus_num;
	cfg.i2c_address = i2c_add;

	ret = combo_bus_trans(&cfg, tx, tx_len, rx_out, rx_len);
	return ret < 0 ? -1 : 0;
}

int mcu_spi_rw(uint8_t bus_num, union SPI_CTRL ctrl,
	uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len)
{
	int ret;
	struct sensor_combo_cfg cfg;
	cfg.bus_type = TAG_SPI;
	cfg.bus_num = bus_num;
	cfg.ctrl = ctrl;

	ret = combo_bus_trans(&cfg, tx, tx_len, rx_out, rx_len);
	return ret < 0 ? -1 : 0;
}

int combo_bus_trans(struct sensor_combo_cfg *p_cfg, uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len)
{
	int ret;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_combo_bus_trans_req_t *pkt_combo_trans;
	uint32_t cmd_wd_len;

	if (!p_cfg) {
		hwlog_err("%s: p_cfg null\n", __func__);
		return -1;
	}

	memset((void *)&pkg_ap, 0, sizeof(pkg_ap));
	memset((void *)&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = p_cfg->bus_type;

	/* check and get bus type */
	if (pkg_ap.tag == TAG_I2C)
		pkg_ap.cmd            = CMD_I2C_TRANS_REQ;
	else if (pkg_ap.tag == TAG_SPI)
		pkg_ap.cmd            = CMD_SPI_TRANS_REQ;
	else {
		hwlog_err("%s: bus_type %d unknown\n", __func__, p_cfg->bus_type);
		return -1;
	}

	cmd_wd_len = tx_len + sizeof(*pkt_combo_trans);
	pkt_combo_trans = kzalloc((size_t)cmd_wd_len, GFP_KERNEL);
	if (!pkt_combo_trans) {
		hwlog_err("alloc failed in %s\n", __func__);
		return -1;
	}

	pkt_combo_trans->busid   = p_cfg->bus_num;
	pkt_combo_trans->ctrl    = p_cfg->ctrl;
	pkt_combo_trans->rx_len  = (uint16_t)rx_len;
	pkt_combo_trans->tx_len  = (uint16_t)tx_len;
	if (tx_len && tx) {
		memcpy((void *)pkt_combo_trans->tx, (void *)tx, (unsigned long)tx_len);
	}
	pkg_ap.wr_buf            = ((pkt_header_t*)pkt_combo_trans + 1);
	pkg_ap.wr_len            = (int)(cmd_wd_len - sizeof(pkt_combo_trans->hd));

	hwlog_info("%s: tag %d cmd %d data=%d, tx_len=%d,rx_len=%d\n",
		__func__, pkg_ap.tag, pkg_ap.cmd, p_cfg->data, tx_len, rx_len);

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	if (ret) {
		hwlog_err("send cmd to mcu fail, data=%d, tx_len=%d,rx_len=%d\n",
			p_cfg->data, tx_len, rx_len);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("mcu_rw fail, data=%d, tx_len=%d,rx_len=%d\n",
				p_cfg->data, tx_len, rx_len);
			ret = -1;
		} else {
			if (rx_out && rx_len) {
				memcpy((void *)rx_out, (void *)pkg_mcu.data, (unsigned long)rx_len);
			}
			ret = pkg_mcu.data_length;
		}
	}
	kfree(pkt_combo_trans);
	return ret;
}

int fill_extend_data_in_dts(struct device_node *dn,
			const char *name, unsigned char *dest,
			size_t max_size, int flag)
{
	int ret = 0;
	int buf[SENSOR_PLATFORM_EXTEND_DATA_SIZE] = { 0 };
	struct property *prop = NULL;
	unsigned int len = 0;
	unsigned int i = 0;
	int *pbuf = buf;
	unsigned char *pdest = dest;
	if (!dn || !name || !dest)
		return -1;

	if (0 == max_size)
		return -1;

	prop = of_find_property(dn, name, NULL);
	if (!prop)
		return -EINVAL;

	if (!prop->value)
		return -ENODATA;

	len = prop->length / 4;	/*len: word*/
	if (0 == len || len > max_size) {
		hwlog_err("In %s: len err! len = %d\n", __func__, len);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "len err\n");
		return -1;
	}
	ret = of_property_read_u32_array(dn, name, buf, len);
	if (ret) {
		hwlog_err("In %s: read %s failed!\n", __func__, name);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "read failed\n");
		return -1;
	}
	if (EXTEND_DATA_TYPE_IN_DTS_BYTE == flag) {
		for (i = 0; i < len; i++) {
			*(dest + i) = (uint8_t) buf[i];
		}
	}
	if (EXTEND_DATA_TYPE_IN_DTS_HALF_WORD == flag) {
		for (i = 0; i < len; i++) {
			memcpy(pdest, pbuf, 2);
			pdest += 2;
			pbuf++;
		}
	}
	if (EXTEND_DATA_TYPE_IN_DTS_WORD == flag)
		memcpy(dest, buf, len * sizeof(int));

	return 0;
}

void read_chip_info(struct device_node *dn, SENSOR_DETECT_LIST sname)
{
	char *chip_info = NULL;
	int ret = 0;

	ret = of_property_read_string(dn, "compatible",
				    (const char **)&chip_info);
	if (ret)
		hwlog_err("%s:read name_id:%d info fail\n", __func__, sname);
	else
		strncpy(sensor_chip_info[sname], chip_info,
			MAX_CHIP_INFO_LEN - 1);

	hwlog_info("get chip info from dts success. sensor name=%s\n",
		   sensor_chip_info[sname]);
}

int send_para_flag = 0;
int set_pinhole_para_flag = 0;
int set_rpr531_para_flag = 0;
int set_tmd2745_para_flag = 0;
uint8_t sens_name = APDS9922;
static DEFINE_MUTEX(mutex_set_para);
static BLOCKING_NOTIFIER_HEAD(tp_notifier_list);
int tpmodule_register_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&tp_notifier_list, nb);
}
EXPORT_SYMBOL(tpmodule_register_client);

int tpmodule_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&tp_notifier_list, nb);
}
EXPORT_SYMBOL(tpmodule_unregister_client);

int tpmodule_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&tp_notifier_list, val, v);
}
EXPORT_SYMBOL(tpmodule_notifier_call_chain);

void set_rpr531_als_extend_prameters(void)
{
	int rpr531_als_para_table = 0;
	unsigned int i = 0;
	for(i=0; i<ARRAY_SIZE(rpr531_als_para_diff_tp_color_table);i++)
	{
		if((rpr531_als_para_diff_tp_color_table[i].phone_type == als_data.als_phone_type)
			&& (rpr531_als_para_diff_tp_color_table[i].phone_version == als_data.als_phone_version)
			&&( rpr531_als_para_diff_tp_color_table[i].tp_manufacture == tp_manufacture))
		{
			rpr531_als_para_table = i;
			break;
		}
	}
	memcpy(als_data.als_extend_data,rpr531_als_para_diff_tp_color_table[rpr531_als_para_table].rpr531_para,
		sizeof(rpr531_als_para_diff_tp_color_table[rpr531_als_para_table].rpr531_para) >
		SENSOR_PLATFORM_EXTEND_DATA_SIZE?
		SENSOR_PLATFORM_EXTEND_DATA_SIZE:
		sizeof(rpr531_als_para_diff_tp_color_table[rpr531_als_para_table].rpr531_para));

	hwlog_info ("rpr531_als_para_tabel= %d\n",rpr531_als_para_table);

	for( i=0; i<RPR531_PARA_SIZE; i++){
		hwlog_info( "%d = %d", i, rpr531_als_para_diff_tp_color_table[rpr531_als_para_table].rpr531_para[i] );
	}
	hwlog_info("\n");
}
void set_pinhole_als_extend_parameters(void)
{
	int als_para_table = 0;
	int i =0;
	for (i = 0;
	     i < ARRAY_SIZE(pinhole_als_para_diff_tp_color_table);
	     i++) {
		if ((pinhole_als_para_diff_tp_color_table[i].
		     phone_type == als_data.als_phone_type)
		    && (pinhole_als_para_diff_tp_color_table[i].
			phone_version ==
			als_data.als_phone_version)
		    && (pinhole_als_para_diff_tp_color_table[i].
			sens_name == sens_name)
		    && (pinhole_als_para_diff_tp_color_table[i].
			tp_manufacture == tp_manufacture)) {
			als_para_table = i;
			break;
		}
	}

	memcpy(als_data.als_extend_data,
	       pinhole_als_para_diff_tp_color_table
	       [als_para_table].pinhole_para,
	       sizeof(pinhole_als_para_diff_tp_color_table
		      [als_para_table].pinhole_para) >
	       SENSOR_PLATFORM_EXTEND_DATA_SIZE ?
	       SENSOR_PLATFORM_EXTEND_DATA_SIZE :
	       sizeof(pinhole_als_para_diff_tp_color_table
		      [als_para_table].pinhole_para));
	hwlog_info
	    ("als_para_tabel=%d phone_type=%d, sens_name=%d, phone_version=%d\n",
	     als_para_table,
	     als_data.als_phone_type,
	     sens_name,
	     als_data.als_phone_version);
	for( i=0; i<pinhole_para_size; i++){
		hwlog_info( "%d = %d", i, pinhole_als_para_diff_tp_color_table[als_para_table].pinhole_para[i] );
	}
	hwlog_info("\n");
}

void set_tmd2745_als_extend_parameters(void)
{
	unsigned int i;
	for (i = 0;
		i < ARRAY_SIZE(tmd2745_als_para_diff_tp_color_table);
		i++) {
		if ((tmd2745_als_para_diff_tp_color_table[i].
			phone_type == als_data.als_phone_type)
			&& (tmd2745_als_para_diff_tp_color_table[i].
			phone_version ==
			als_data.als_phone_version)
			&& (tmd2745_als_para_diff_tp_color_table[i].
			tp_manufacture == tp_manufacture)) {
			als_para_table = i;
			break;
		}
	}

	memcpy(als_data.als_extend_data,
		tmd2745_als_para_diff_tp_color_table[als_para_table].als_para,
		sizeof(tmd2745_als_para_diff_tp_color_table[als_para_table].als_para) >
		SENSOR_PLATFORM_EXTEND_DATA_SIZE ?
		SENSOR_PLATFORM_EXTEND_DATA_SIZE :
		sizeof(tmd2745_als_para_diff_tp_color_table[als_para_table].als_para));
	hwlog_err
		("als_para_table=%d, phone_type=%d, phone_version=%d\n",
		als_para_table,
		als_data.als_phone_type,
		als_data.als_phone_version);
	for( i=0; i<TMD2745_PARA_SIZE; i++){
		hwlog_info( "%d = %d", i, tmd2745_als_para_diff_tp_color_table[als_para_table].als_para[i] );
	}
	hwlog_info("\n");
}

extern void resend_als_parameters_to_mcu(void);

void read_tp_module_notify(struct notifier_block *nb,
			unsigned long action, void *data)
{
	hwlog_info("%s, start!\n", __func__);
	if(action == TS_PANEL_UNKNOWN){
		return;
	}
	mutex_lock(&mutex_set_para);
	tp_manufacture = action;
	if(1 == set_pinhole_para_flag){
		set_pinhole_als_extend_parameters();
	}else if(true == set_rpr531_para_flag){
		set_rpr531_als_extend_prameters();
	}else if(true == set_tmd2745_para_flag){
		set_tmd2745_als_extend_parameters();
	}else{
		hwlog_info("%s, not get sensor yet!\n", __func__);
		mutex_unlock(&mutex_set_para);
		return;
	}
	mutex_unlock(&mutex_set_para);
	hwlog_info("%s, get tp module type = %d\n", __func__, tp_manufacture);
	if(1 == send_para_flag){
		resend_als_parameters_to_mcu();
	}
}

void read_is_cali_supported(struct device_node *dn)
{
	int temp = 0;

	if (of_property_read_u32(dn, "is_cali_supported", &temp))
		hwlog_err("%s:read als is_cali_supported fail\n", __func__);
	else
		is_cali_supported = (int)temp;
}

void read_als_data_from_dts(struct device_node *dn)
{
	int temp = 0;
	int als_phone_type = 0;
	int als_phone_version = 0;
	int i = 0, ret = 0;
	char *chip_info = NULL;
	read_chip_info(dn, ALS);
	temp = of_property_read_string(dn, "compatible",
				    (const char **)&chip_info);
	if (temp < 0)
		hwlog_err("%s:read als poll_interval fail\n", __func__);
	else
		strncpy(sensor_chip_info[ALS], chip_info,
			MAX_CHIP_INFO_LEN - 1);
	if (!strncmp
	    (chip_info, "huawei,rohm_bh1745", sizeof("huawei,rohm_bh1745"))) {
		rohm_rgb_flag = 1;
		hwlog_err("%s:rohm_bh1745 i2c_address suc,%d \n", __func__,
			  temp);
	}

	if (!strncmp(chip_info, "huawei,avago_apds9251",
	     sizeof("huawei,avago_apds9251"))) {
		avago_rgb_flag = 1;
		hwlog_err("%s:avago_apds9251 i2c_address suc,%d \n", __func__,
			  temp);
	}

	if (!strncmp(chip_info, "huawei,ltr578_als",
	     sizeof("huawei,ltr578_als"))) {
		sens_name = LTR578;
		ltr578_flag = 1;
		hwlog_err("%s:ltr578_als i2c_address suc,%d \n", __func__,
			  temp);
	}

	if (!strncmp(chip_info, "huawei,apds9922_als",
	     sizeof("huawei,apds9922_als"))) {
		sens_name = APDS9922;
		apds9922_flag = 1;
		hwlog_err("%s:apds9922_als i2c_address suc,%d \n", __func__,
			  temp);
	}

	if (!strncmp(chip_info, "huawei,tmd2745_als",
	     sizeof("huawei,tmd2745_als"))) {
		tmd2745_flag = 1;
		hwlog_err("%s:tmd2745_als i2c_address suc,%d \n", __func__,
			  temp);
	}

	if (!strncmp(chip_info, "huawei,rohm_rpr531_als",
	     sizeof("huawei,rohm_rpr531_als"))) {
		rohm_rpr531_flag= true;
		hwlog_info("%s:rpr531_als i2c_address suc,%d \n", __func__,
			  temp);
	}

	temp = of_get_named_gpio(dn, "gpio_int1", 0);
	if (temp < 0)
		hwlog_err("%s:read gpio_int1 fail\n", __func__);
	else
		als_data.gpio_int1 = (uint8_t) temp;

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read als poll_interval fail\n", __func__);
	else
		als_data.poll_interval = (uint16_t) temp;

	if (of_property_read_u32(dn, "init_time", &temp))
		hwlog_err("%s:read als init time fail\n", __func__);
	else
		als_data.init_time = (uint16_t) temp;

	if (of_property_read_u32(dn, "GA1", &temp))
		hwlog_err("%s:read als ga1 fail\n", __func__);
	else
		als_data.GA1 = temp;

	if (of_property_read_u32(dn, "GA2", &temp))
		hwlog_err("%s:read als ga2 fail\n", __func__);
	else
		als_data.GA2 = temp;

	if (of_property_read_u32(dn, "GA3", &temp))
		hwlog_err("%s:read als ga3 fail\n", __func__);
	else
		als_data.GA3 = temp;

	if (of_property_read_u32(dn, "als_phone_type", &als_phone_type))
		hwlog_err("%s:read als_phone_type fail\n", __func__);
	else
		als_data.als_phone_type = (uint8_t) als_phone_type;

	if (of_property_read_u32(dn, "als_phone_version", &als_phone_version))
		hwlog_err("%s:read als_phone_version fail\n", __func__);
	else
		als_data.als_phone_version = (uint8_t) als_phone_version;
	als_data.als_phone_tp_colour = phone_color;

	if (of_property_read_u32(dn, "atime", &temp))
		hwlog_err("%s:read als atime fail\n", __func__);
	else
		als_data.atime = (uint8_t) temp;

	if (of_property_read_u32(dn, "again", &temp))
		hwlog_err("%s:read als again fail\n", __func__);
	else
		als_data.again = (uint8_t) temp;

	if (of_property_read_u32(dn, "tp_color_from_nv", &temp))
		hwlog_info("%s:read als tp_color_from_nv fail\n", __func__);
	else
		tp_color_from_nv_flag = (uint8_t)temp;

	read_is_cali_supported( dn );

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read als file_id fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
	dyn_req->file_count++;
	read_sensorlist_info(dn, ALS);
	ret = of_property_read_u32(dn, "phone_color_num", &temp);
	if (0 == ret){
		if(temp < MAX_PHONE_COLOR_NUM){
			ret = of_property_read_u32_array(dn, "sleeve_detect_threshhold", sleeve_detect_paremeter, temp * 2);
			if (ret) {
				hwlog_err("%s: read sleeve_detect_threshhold failed!\n", __func__);
			}
		}
	}else{
		hwlog_err("%s:read phone_color_num fail(%d)\n", __func__, ret);
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read als sensor_list_info_id fail\n", __func__);
	else
		sensorlist[++sensorlist[0]] = (uint16_t) temp;

	als_para_table = 0;

	if (rohm_rgb_flag == 1) {
			for (i = 0;
			     i < ARRAY_SIZE(als_para_diff_tp_color_table);
			     i++) {
				if ((als_para_diff_tp_color_table[i].
				     phone_type == als_data.als_phone_type)
				    && (als_para_diff_tp_color_table[i].
					phone_version ==
					als_data.als_phone_version)
				    && (als_para_diff_tp_color_table[i].
					tp_lcd_manufacture == tplcd_manufacture
					|| als_para_diff_tp_color_table[i].
					tp_lcd_manufacture == DEFAULT_TPLCD)
				    && (als_para_diff_tp_color_table[i].
					tp_color == phone_color)) {
					als_para_table = i;
					break;
				}
			}
			memcpy(als_data.als_extend_data,
			       als_para_diff_tp_color_table[als_para_table].
			       bh745_para,
			       sizeof(als_para_diff_tp_color_table
				      [als_para_table].bh745_para) >
			       SENSOR_PLATFORM_EXTEND_DATA_SIZE ?
			       SENSOR_PLATFORM_EXTEND_DATA_SIZE :
			       sizeof(als_para_diff_tp_color_table
				      [als_para_table].bh745_para));

			hwlog_err
			    ("als_para_tabel=%d ,bh1745 phone_color=0x%x  tplcd_manufacture=%d, phone_type=%d,phone_version=%d\n",
			     als_para_table, phone_color, tplcd_manufacture,
			     als_data.als_phone_type,
			     als_data.als_phone_version);
	} else if (avago_rgb_flag == 1) {
			for (i = 0;
			     i < ARRAY_SIZE(apds_als_para_diff_tp_color_table);
			     i++) {
				if ((apds_als_para_diff_tp_color_table[i].
				     phone_type == als_data.als_phone_type)
				    && (apds_als_para_diff_tp_color_table[i].
					phone_version ==
					als_data.als_phone_version)
				    && (apds_als_para_diff_tp_color_table[i].
					tp_lcd_manufacture == tplcd_manufacture
					|| apds_als_para_diff_tp_color_table[i].
					tp_lcd_manufacture == DEFAULT_TPLCD)
				    && (apds_als_para_diff_tp_color_table[i].
					tp_color == phone_color)) {
					als_para_table = i;
					break;
				}
			}
			memcpy(als_data.als_extend_data,
			       apds_als_para_diff_tp_color_table
			       [als_para_table].apds251_para,
			       sizeof(apds_als_para_diff_tp_color_table
				      [als_para_table].apds251_para) >
			       SENSOR_PLATFORM_EXTEND_DATA_SIZE ?
			       SENSOR_PLATFORM_EXTEND_DATA_SIZE :
			       sizeof(apds_als_para_diff_tp_color_table
				      [als_para_table].apds251_para));
			hwlog_err
			    ("als_para_tabel=%d apds9251 phone_color=0x%x phone_type=%d,phone_version=%d\n",
			     als_para_table, phone_color,
			     als_data.als_phone_type,
			     als_data.als_phone_version);
	} else if((apds9922_flag == 1)||(ltr578_flag == 1)){
		if(WAS==als_data.als_phone_type || PIC==als_data.als_phone_type || BAC==als_data.als_phone_type){
			tp_manufacture = tplcd_manufacture;
		}/*when the product is Was,the tp_manufacture is the same as tplcd_manufacture*/
		mutex_lock(&mutex_set_para);
		set_pinhole_als_extend_parameters();
		set_pinhole_para_flag = 1;
		mutex_unlock(&mutex_set_para);
	}else if(true == rohm_rpr531_flag){
		if(PIC == als_data.als_phone_type || BAC == als_data.als_phone_type){
			tp_manufacture = tplcd_manufacture;
		}
		mutex_lock(&mutex_set_para);
		set_rpr531_als_extend_prameters();
		set_rpr531_para_flag = true;
		mutex_unlock(&mutex_set_para);
	} else if (tmd2745_flag == 1) {
		if(PIC == als_data.als_phone_type || BAC == als_data.als_phone_type){
			tp_manufacture = tplcd_manufacture;
		}
		mutex_lock(&mutex_set_para);
		set_tmd2745_als_extend_parameters();
		set_tmd2745_para_flag = true;
		mutex_unlock(&mutex_set_para);
	}else {
		ret = fill_extend_data_in_dts(dn, "als_extend_data",
					    als_data.als_extend_data, 12,
					    EXTEND_DATA_TYPE_IN_DTS_HALF_WORD);
		if (ret) {
			hwlog_err
			    ("als_extend_data:fill_extend_data_in_dts failed!\n");
		}
	}
}

int read_tpcolor_from_nv(void)
{
	int ret = 0;
	int i = 0;
	char nv_tp_color[16] = "";
	struct hisi_nve_info_user user_info;
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = TP_COLOR_NV_NUM;
	user_info.valid_size = TP_COLOR_NV_SIZE;
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return -1;
	}
	memcpy(nv_tp_color, user_info.nv_data, sizeof(nv_tp_color)-1);

	if(strstr(nv_tp_color,"white")){
		phone_color = WHITE;
	}else if(strstr(nv_tp_color,"black")){
		phone_color = BLACK;
	}else if(strstr(nv_tp_color,"gold")){
		phone_color = GOLD;
	}else{
		hwlog_info("other colors\n");
	}

	hwlog_info("phone_color = 0x%x, nv_tp_color = %s\n",phone_color,nv_tp_color);
	als_data.als_phone_tp_colour = phone_color;

	als_para_table = 0;
	if (rohm_rgb_flag == 1) {
			for (i = 0;
			     i < ARRAY_SIZE(als_para_diff_tp_color_table);
			     i++) {
				if ((als_para_diff_tp_color_table[i].
				     phone_type == als_data.als_phone_type)
				    && (als_para_diff_tp_color_table[i].
					phone_version ==
					als_data.als_phone_version)
				    && (als_para_diff_tp_color_table[i].
					tp_lcd_manufacture == tplcd_manufacture
					|| als_para_diff_tp_color_table[i].
					tp_lcd_manufacture == DEFAULT_TPLCD)
				    && (als_para_diff_tp_color_table[i].
					tp_color == phone_color)) {
					als_para_table = i;
					break;
				}
			}
			memcpy(als_data.als_extend_data,
			       als_para_diff_tp_color_table[als_para_table].
			       bh745_para,
			       sizeof(als_para_diff_tp_color_table
				      [als_para_table].bh745_para) >
			       SENSOR_PLATFORM_EXTEND_DATA_SIZE ?
			       SENSOR_PLATFORM_EXTEND_DATA_SIZE :
			       sizeof(als_para_diff_tp_color_table
				      [als_para_table].bh745_para));
			hwlog_info
			    (" als_para_tabel=%d ,bh1745 phone_color=0x%x  tplcd_manufacture=%d, phone_type=%d,phone_version=%d\n",
			     als_para_table, phone_color, tplcd_manufacture,
			     als_data.als_phone_type,
			     als_data.als_phone_version);
	} else if (avago_rgb_flag == 1) {
			for (i = 0;
			     i < ARRAY_SIZE(apds_als_para_diff_tp_color_table);
			     i++) {
				if ((apds_als_para_diff_tp_color_table[i].
				     phone_type == als_data.als_phone_type)
				    && (apds_als_para_diff_tp_color_table[i].
					phone_version ==
					als_data.als_phone_version)
				    && (apds_als_para_diff_tp_color_table[i].
					tp_lcd_manufacture == tplcd_manufacture
					|| apds_als_para_diff_tp_color_table[i].
					tp_lcd_manufacture == DEFAULT_TPLCD)
				    && (apds_als_para_diff_tp_color_table[i].
					tp_color == phone_color)) {
					als_para_table = i;
					break;
				}
			}
			memcpy(als_data.als_extend_data,
			       apds_als_para_diff_tp_color_table
			       [als_para_table].apds251_para,
			       sizeof(apds_als_para_diff_tp_color_table
				      [als_para_table].apds251_para) >
			       SENSOR_PLATFORM_EXTEND_DATA_SIZE ?
			       SENSOR_PLATFORM_EXTEND_DATA_SIZE :
			       sizeof(apds_als_para_diff_tp_color_table
				      [als_para_table].apds251_para));
			hwlog_info
			    ("als_para_tabel=%d apds9251 phone_color=0x%x phone_type=%d,phone_version=%d\n",
			     als_para_table, phone_color,
			     als_data.als_phone_type,
			     als_data.als_phone_version);
	}

	return 0;
}

int get_tpcolor_from_nv(void)
{
	int ret = 0;

	if(tp_color_from_nv_flag)
	{
		hwlog_info(" %s: tp_color_from_nv_flag = %d\n", __func__,tp_color_from_nv_flag);
		ret = read_tpcolor_from_nv();
		if(ret != 0)
		{
			hwlog_err("%s: read tp_color from NV fail\n", __func__);
			return -1;
		}
		resend_als_parameters_to_mcu();
	}

	return 0;
}

const char *get_sensor_info_by_tag(int tag)
{
	SENSOR_DETECT_LIST sname = SENSOR_MAX;

	sname = get_id_by_sensor_tag(tag);

	return (sname != SENSOR_MAX) ? sensor_chip_info[sname] : "";
}

/*******************************************************************************
Function:	sensor_set_cfg_data
Description: 其他配置信息，例如是否需要根据电池电流大小校准指南针
Data Accessed:  无
Data Updated:   无
Input:        无
Output:         无
Return:         成功或者失败信息: 0->成功, -1->失败
*******************************************************************************/
int sensor_set_current_info(void)
{
	int ret = 0;
	u8 need_current = 0;
	write_info_t pkg_ap;

	memset(&pkg_ap, 0, sizeof(pkg_ap));

	need_current = check_if_need_current();
	if (need_current == 0)
		return 0;

	pkg_ap.tag = TAG_CURRENT;
	pkg_ap.cmd = CMD_CURRENT_CFG_REQ;
	pkg_ap.wr_buf = &need_current;
	pkg_ap.wr_len = sizeof(need_current);

	if (g_iom3_state == IOM3_ST_RECOVERY || iom3_power_state == ST_SLEEP)
		ret = write_customize_cmd_noresp(TAG_CURRENT,
				CMD_CURRENT_CFG_REQ, pkg_ap.wr_buf, pkg_ap.wr_len);
	else
		ret = write_customize_cmd(&pkg_ap, NULL);

	if (ret) {
		hwlog_err("send current cfg data to mcu fail,ret=%d\n", ret);
		return -1;
	} else {
		hwlog_info("set current cfg data to mcu success\n");
	}

	return 0;
}

static ssize_t show_sensor_list_info(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int i;
	hwlog_info( "sensor list: ");
	for(i = 0; i <= sensorlist[0]; i++)
		hwlog_info( " %d  ", sensorlist[i]);
	hwlog_info("\n");
	memcpy(buf, sensorlist, ((sensorlist[0]+1)*sizeof(uint16_t)));
	return (sensorlist[0] + 1) * sizeof(uint16_t);
}
static DEVICE_ATTR(sensor_list_info, S_IRUGO,
				   show_sensor_list_info, NULL);

#define SENSOR_SHOE_INFO(TAG)\
static ssize_t sensor_show_##TAG##_info(struct device *dev,\
				struct device_attribute *attr, char *buf)\
{\
	return snprintf(buf, MAX_STR_SIZE, "%s\n", sensor_chip_info[TAG]);\
}

SENSOR_SHOE_INFO(PS);
static DEVICE_ATTR(ps_info, S_IRUGO, sensor_show_PS_info, NULL);

SENSOR_SHOE_INFO(ALS);
static DEVICE_ATTR(als_info, S_IRUGO, sensor_show_ALS_info, NULL);

SENSOR_SHOE_INFO(GYRO);
static DEVICE_ATTR(gyro_info, S_IRUGO, sensor_show_GYRO_info, NULL);

SENSOR_SHOE_INFO(MAG);
static DEVICE_ATTR(mag_info, S_IRUGO, sensor_show_MAG_info, NULL);

SENSOR_SHOE_INFO(ACC);
static DEVICE_ATTR(acc_info, S_IRUGO, sensor_show_ACC_info, NULL);

SENSOR_SHOE_INFO(AIRPRESS);
static DEVICE_ATTR(airpress_info, S_IRUGO, sensor_show_AIRPRESS_info, NULL);

SENSOR_SHOE_INFO(HANDPRESS);
static DEVICE_ATTR(handpress_info, S_IRUGO, sensor_show_HANDPRESS_info, NULL);

//SENSOR_SHOE_INFO(CAP_PROX);
//static DEVICE_ATTR(cap_prox_info, S_IRUGO, sensor_show_CAP_PROX_info, NULL);

SENSOR_SHOE_INFO(CHARGER);
static DEVICE_ATTR(charger_info, S_IRUGO, sensor_show_CHARGER_info, NULL);

SENSOR_SHOE_INFO(SWITCH);
static DEVICE_ATTR(switch_info, S_IRUGO, sensor_show_SWITCH_info, NULL);

unsigned int sensor_read_number[TAG_END] = {0,0,0,0,0};

#define SENSOR_SHOW_VALUE(TAG) \
static ssize_t sensor_show_##TAG##_read_data(struct device *dev, \
				struct device_attribute *attr, char *buf) \
{\
	hwlog_info("[sensorhub_test], %s return %d\n", __func__, sensor_read_number[TAG]);\
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_read_number[TAG]);\
}

SENSOR_SHOW_VALUE(TAG_ACCEL);
static DEVICE_ATTR(acc_read_data, 0664, sensor_show_TAG_ACCEL_read_data, NULL);

SENSOR_SHOW_VALUE(TAG_MAG);
static DEVICE_ATTR(mag_read_data, 0664, sensor_show_TAG_MAG_read_data, NULL);

SENSOR_SHOW_VALUE(TAG_GYRO);
static DEVICE_ATTR(gyro_read_data, 0664, sensor_show_TAG_GYRO_read_data, NULL);

SENSOR_SHOW_VALUE(TAG_ALS);
static DEVICE_ATTR(als_read_data, 0664, sensor_show_TAG_ALS_read_data, NULL);

SENSOR_SHOW_VALUE(TAG_PS);
static DEVICE_ATTR(ps_read_data, 0664, sensor_show_TAG_PS_read_data, NULL);

SENSOR_SHOW_VALUE(TAG_PRESSURE);
static DEVICE_ATTR(airpress_read_data, 0664, sensor_show_TAG_PRESSURE_read_data,
		   NULL);

SENSOR_SHOW_VALUE(TAG_HANDPRESS);
static DEVICE_ATTR(handpress_read_data, 0664, sensor_show_TAG_HANDPRESS_read_data, NULL);

/*SENSOR_SHOW_VALUE(TAG_CAP_PROX);*/
/*static DEVICE_ATTR(cap_prox_read_data, 0664, sensor_show_TAG_CAP_PROX_read_data, NULL);*/

#define SHOW_SELFTEST_RESULT(TAG) \
static ssize_t show_##TAG##_selfTest_result(struct device *dev,\
				struct device_attribute *attr, char *buf)\
{\
	return snprintf(buf, MAX_STR_SIZE, "%s\n", sensor_status.TAG##_selfTest_result);\
}

SHOW_SELFTEST_RESULT(gyro);
SHOW_SELFTEST_RESULT(mag);
SHOW_SELFTEST_RESULT(accel);
SHOW_SELFTEST_RESULT(gps_4774_i2c);

static ssize_t show_handpress_selfTest_result(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int result = 0;

	if (strncmp(sensor_status.handpress_selfTest_result, "1", strlen("1")))
		result = 0;
	else
		result = 1;

	return snprintf(buf, MAX_STR_SIZE, "%d\n", result);
}

#ifdef SENSOR_DSM_CONFIG
#define SET_SENSOR_SELFTEST(TAGUP, TAGLOW) \
static ssize_t attr_set_##TAGLOW##_selftest(struct device *dev, struct device_attribute *attr,\
				const char *buf, size_t size)\
{\
	unsigned long val = 0;\
	int err = -1;\
	write_info_t	pkg_ap;\
	read_info_t	pkg_mcu;\
	memset(&pkg_ap, 0, sizeof(pkg_ap));\
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));\
	if (strict_strtoul(buf, 10, &val))\
		return -EINVAL;\
	if (1 == val) {\
		pkg_ap.tag = TAG_##TAGUP;\
		pkg_ap.cmd = CMD_##TAGUP##_SELFTEST_REQ;\
		pkg_ap.wr_buf = NULL;\
		pkg_ap.wr_len = 0;\
		err = write_customize_cmd(&pkg_ap,  &pkg_mcu);\
		if (err) {\
			hwlog_err("send %s selftest cmd to mcu fail,ret=%d\n", #TAGUP, err);\
			memcpy(sensor_status.TAGLOW##_selfTest_result, "0", 2);\
			__dmd_log_report(DSM_SHB_ERR_IOM7_OTHER, __func__, "-->selftest fail\n");\
			return size;\
		} \
		if (pkg_mcu.errno != 0) {\
			hwlog_err("%s selftest fail\n", #TAGUP);\
			memcpy(sensor_status.TAGLOW##_selfTest_result, "0", 2);\
		} else {\
			hwlog_info("%s selftest  success, data len=%d\n", #TAGUP, pkg_mcu.data_length);\
			memcpy(sensor_status.TAGLOW##_selfTest_result, "1", 2);\
		} \
	} \
	return size;\
}
#else
#define SET_SENSOR_SELFTEST(TAGUP, TAGLOW) \
static ssize_t attr_set_##TAGLOW##_selftest(struct device *dev, struct device_attribute *attr,\
				const char *buf, size_t size)\
{\
	unsigned long val = 0;\
	int err = -1;\
	write_info_t	pkg_ap;\
	read_info_t	pkg_mcu;\
	memset(&pkg_ap, 0, sizeof(pkg_ap));\
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));\
	if (strict_strtoul(buf, 10, &val))\
		return -EINVAL;\
	if (1 == val) {\
		pkg_ap.tag = TAG_##TAGUP;\
		pkg_ap.cmd = CMD_##TAGUP##_SELFTEST_REQ;\
		pkg_ap.wr_buf = NULL;\
		pkg_ap.wr_len = 0;\
		err = write_customize_cmd(&pkg_ap,  &pkg_mcu);\
		if (err) {\
			hwlog_err("send %s selftest cmd to mcu fail,ret=%d\n", #TAGUP, err);\
			memcpy(sensor_status.TAGLOW##_selfTest_result, "0", 2);\
			return size;\
		} \
		if (pkg_mcu.errno != 0) {\
			hwlog_err("%s selftest fail\n", #TAGUP);\
			memcpy(sensor_status.TAGLOW##_selfTest_result, "0", 2);\
		} else {\
			hwlog_info("%s selftest  success, data len=%d\n", #TAGUP, pkg_mcu.data_length);\
			memcpy(sensor_status.TAGLOW##_selfTest_result, "1", 2);\
		} \
	} \
	return size;\
}
#endif

SET_SENSOR_SELFTEST(GYRO, gyro);
static DEVICE_ATTR(gyro_selfTest, 0664, show_gyro_selfTest_result,
		   attr_set_gyro_selftest);

SET_SENSOR_SELFTEST(MAG, mag);
static DEVICE_ATTR(mag_selfTest, 0664, show_mag_selfTest_result,
		   attr_set_mag_selftest);

SET_SENSOR_SELFTEST(ACCEL, accel);
static DEVICE_ATTR(acc_selfTest, 0664, show_accel_selfTest_result,
		   attr_set_accel_selftest);

SET_SENSOR_SELFTEST(GPS_4774_I2C, gps_4774_i2c);
static DEVICE_ATTR(gps_4774_i2c_selfTest, 0664,
		   show_gps_4774_i2c_selfTest_result,
		   attr_set_gps_4774_i2c_selftest);

SET_SENSOR_SELFTEST(HANDPRESS, handpress);
static DEVICE_ATTR(handpress_selfTest, 0664,
		   show_handpress_selfTest_result,
		   attr_set_handpress_selftest);

static ssize_t i2c_rw_pi(struct device *dev,
			 struct device_attribute *attr, const char *buf,
			 size_t count)
{
	uint64_t val = 0;
	int ret = 0;
	uint8_t bus_num = 0, i2c_address = 0, reg_add = 0, len = 0;
	uint8_t rw = 0, buf_temp[DEBUG_DATA_LENGTH] = { 0 };

	if (strict_strtoull(buf, 16, &val))
		return -EINVAL;
	/*##(bus_num)##(i2c_addr)##(reg_addr)##(len)*/
	bus_num = (val >> 40) & 0xff;
	i2c_address = (val >> 32) & 0xff;
	reg_add = (val >> 24) & 0xff;
	len = (val >> 16) & 0xff;
	if (len > DEBUG_DATA_LENGTH - 1) {
		hwlog_err("len exceed %d\n", len);
		len = DEBUG_DATA_LENGTH - 1;
	}
	rw = (val >> 8) & 0xff;
	buf_temp[0] = reg_add;
	buf_temp[1] = (uint8_t) (val & 0xff);

	hwlog_info
	    ("In %s! bus_num = %d, i2c_address = %d, reg_add = %d, len = %d, rw = %d, buf_temp[1] = %d\n",
	     __func__, bus_num, i2c_address, reg_add, len, rw, buf_temp[1]);
/*static int mcu_i2c_rw(uint8_t bus_num, uint8_t i2c_add, uint8_t register_add, uint8_t rw, int len, uint8_t *buf)*/
	if (rw) {
		ret = mcu_i2c_rw(bus_num, i2c_address, &buf_temp[0], 1, &buf_temp[1], len);
	} else {
		ret = mcu_i2c_rw(bus_num, i2c_address, buf_temp, 2, NULL, 0);
	}
	if (ret < 0)
		hwlog_err("oper %d(1/32:r 0/31:w) i2c reg fail!\n", rw);
	if (rw) {
		hwlog_err("i2c reg %x value %x %x %x %x\n", reg_add,
			  buf_temp[1], buf_temp[2], buf_temp[3], buf_temp[4]);
		memcpy(debug_read_data_buf, &buf_temp[1], len);
	}
	return count;
}

static ssize_t i2c_rw_pi_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	unsigned int i = 0;
	unsigned int len = 0;
	char *p = buf;
	if (!buf)
		return -1;

	for (i = 0; i < DEBUG_DATA_LENGTH; i++) {
		snprintf(p, 6, "0x%x,", debug_read_data_buf[i]);
		if (debug_read_data_buf[i] > 0xf) {
			p += 5;
			len += 5;
		} else {
			p += 4;
			len += 4;
		}
	}

	p = buf;
	*(p + len - 1) = 0;

	p = NULL;
	return len;
}

static DEVICE_ATTR(i2c_rw, 0664, i2c_rw_pi_show, i2c_rw_pi);

static uint8_t i2c_rw16_data_buf[2] = { 0 };

static ssize_t i2c_rw16_pi(struct device *dev,
			   struct device_attribute *attr, const char *buf,
			   size_t count)
{
	uint64_t val = 0;
	int ret = 0;
	uint8_t bus_num = 0, i2c_address = 0, reg_add = 0, len = 0;
	uint8_t rw = 0, buf_temp[3] = { 0 };

	if (strict_strtoull(buf, 16, &val))
		return -EINVAL;
	/*##(bus_num)##(i2c_addr)##(reg_addr)##(len)*/
	bus_num = (val >> 48) & 0xff;
	i2c_address = (val >> 40) & 0xff;
	reg_add = (val >> 32) & 0xff;
	len = (val >> 24) & 0xff;
	if (len > 2) {
		hwlog_err("len exceed %d\n", len);
		len = 2;
	}
	rw = (val >> 16) & 0xff;
	buf_temp[0] = reg_add;
	buf_temp[1] = (uint8_t) (val >> 8);
	buf_temp[2] = (uint8_t) (val & 0xff);

	hwlog_info
	    ("In %s! bus_num=%d, i2c_address=%d, reg_add=%d, len=%d, rw=%d, buf_temp[1]=0x%02x, buf_temp[2]=0x%02x\n",
	     __func__, bus_num, i2c_address, reg_add, len, rw, buf_temp[1],
	     buf_temp[2]);
	if (rw) {
		ret = mcu_i2c_rw(bus_num, i2c_address, buf_temp, 1, &buf_temp[1], (uint32_t)len);
	} else {
		ret = mcu_i2c_rw(bus_num, i2c_address, buf_temp, 1 + len, NULL, 0);
	}
	if (ret < 0)
		hwlog_err("oper %d(1:r 0:w) i2c reg fail!\n", rw);
	if (rw) {
		hwlog_err("i2c reg %x value %x %x\n", reg_add, buf_temp[1],
			  buf_temp[2]);
		memcpy(i2c_rw16_data_buf, &buf_temp[1], 2);
	}
	return count;
}

static ssize_t i2c_rw16_pi_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	char *p = buf;

	if (!buf)
		return -1;

	snprintf(p, 8, "0x%02x%02x\n", i2c_rw16_data_buf[0],
		 i2c_rw16_data_buf[1]);
	*(p + 7) = 0;

	p = NULL;
	return 8;
}
static DEVICE_ATTR(i2c_rw16, 0664, i2c_rw16_pi_show, i2c_rw16_pi);

#define CLI_TIME_STR_LEN (20)
#define CLI_CONTENT_LEN_MAX (256)
/*pass mark as NA*/
static char *cali_error_code_str[] = {"NULL", "NA", "EXEC_FAIL", "NV_FAIL", "COMMU_FAIL", "RET_TYPE_MAX"};
#define DATA_CLLCT	"/data/hwzd_logs/dataCollection.log"
#define HAND_DATA_CLLCT	"/data/hwzd_logs/handSensorCalibData.log"

#define ACC_CALI_X_OFFSET	"testName:ACC_CALI_X_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_Y_OFFSET	"testName:ACC_CALI_Y_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_Z_OFFSET	"testName:ACC_CALI_Z_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_X_SEN		"testName:ACC_CALI_X_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_Y_SEN		"testName:ACC_CALI_Y_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_Z_SEN		"testName:ACC_CALI_Z_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_00		"testName:ACC_CALI_XIS_00*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_01		"testName:ACC_CALI_XIS_01*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_02		"testName:ACC_CALI_XIS_02*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_10		"testName:ACC_CALI_XIS_10*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_11		"testName:ACC_CALI_XIS_11*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_12		"testName:ACC_CALI_XIS_12*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_20		"testName:ACC_CALI_XIS_20*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_21		"testName:ACC_CALI_XIS_21*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_22		"testName:ACC_CALI_XIS_22*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define PS_CALI_RAW_DATA	"testName:PS_CALI_RAW_DATA*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_R		"testName:ALS_CALI_R*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_G		"testName:ALS_CALI_G*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_B		"testName:ALS_CALI_B*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_C		"testName:ALS_CALI_C*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_ALS	"testName:ALS_CALI_ALS*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_IR		"testName:ALS_CALI_IR*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_VISIBLE	"testName:ALS_CALI_VISIBLE*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_REF		"testName:ALS_CALI_REF*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_DATA0		"testName:ALS_CALI_DATA0*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_DATA1		"testName:ALS_CALI_DATA1*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_LUX		"testName:ALS_CALI_LUX*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_CCT		"testName:ALS_CALI_CCT*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_X_OFFSET	"testName:GYRO_CALI_X_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_Y_OFFSET	"testName:GYRO_CALI_Y_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_Z_OFFSET	"testName:GYRO_CALI_Z_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_X_SEN	"testName:GYRO_CALI_X_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_Y_SEN	"testName:GYRO_CALI_Y_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_Z_SEN	"testName:GYRO_CALI_Z_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_00	"testName:GYRO_CALI_XIS_00*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_01	"testName:GYRO_CALI_XIS_01*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_02	"testName:GYRO_CALI_XIS_02*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_10	"testName:GYRO_CALI_XIS_10*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_11	"testName:GYRO_CALI_XIS_11*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_12	"testName:GYRO_CALI_XIS_12*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_20	"testName:GYRO_CALI_XIS_20*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_21	"testName:GYRO_CALI_XIS_21*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_22	"testName:GYRO_CALI_XIS_22*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define PRESS_CALI_OFFSET	"testName:PRESS_CALI_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"

static void save_to_file(const char *file_path, const char *content)
{
	static mm_segment_t oldfs;
	struct file *fp = NULL;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	if(ps_external_ir_param.external_ir == 1) {
		fp = filp_open(file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
	} else {
		fp = filp_open(file_path, O_WRONLY | O_APPEND, 0644);
	}
	if (IS_ERR(fp)) {
		hwlog_err("oper %s fail\n", file_path);
		set_fs(oldfs);
		return;
	}

	vfs_write(fp, content, strlen(content), &(fp->f_pos));

	filp_close(fp, NULL);

	set_fs(oldfs);
	return;
}

static char *get_cali_error_code(int error_code)
{
	if ((error_code > 0) && (error_code < RET_TYPE_MAX)) {
		return cali_error_code_str[error_code];
	}
	return NULL;
}

static void get_test_time(char *date_str, size_t size)
{
	struct timeval time;
	unsigned long local_time;
	struct tm tm;

	/*if (size < CLI_TIME_STR_LEN){ //This is dead code, leave it for forture use.
	//hwlog_err("%s calibrate date size(%d) less than need\n", __func__, size);
	//date_str = NULL;
	//return;
	//}*/
	do_gettimeofday(&time);
	local_time = (u32) (time.tv_sec - (sys_tz.tz_minuteswest * 60));
	rtc_time_to_tm(local_time, &tm);

	snprintf(date_str, CLI_TIME_STR_LEN, "%04ld-%02d-%02d %02d:%02d:%02d",
		 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
		 tm.tm_min, tm.tm_sec);
}

static ssize_t attr_acc_calibrate_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int val = return_calibration;
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static int acc_calibrate_save(const void *buf, int length)
{
	const int32_t *poffset_data = (const int32_t *)buf;
	int ret = 0;
	if (buf == NULL || length <= 0) {
		hwlog_err("%s invalid argument.", __func__);
		return_calibration = EXEC_FAIL;
		return -1;
	}
	hwlog_info("%s:gsensor calibrate ok, %d  %d  %d  %d  %d  %d %d  %d  %d %d  %d  %d %d  %d  %d \n",
		__func__, *poffset_data, *(poffset_data + 1),*(poffset_data + 2), *(poffset_data + 3),*(poffset_data + 4),
		*(poffset_data + 5),*(poffset_data + 6), *(poffset_data + 7),*(poffset_data + 8), *(poffset_data + 9),
		*(poffset_data + 10),*(poffset_data + 11), *(poffset_data + 12),*(poffset_data + 13), *(poffset_data + 14));
	ret = write_gsensor_offset_to_nv((char *)buf, length);
	if (ret) {
		hwlog_err("nv write fail.\n");
		return_calibration = NV_FAIL;
		return -1;
	}
	return_calibration = SUC;
	return 0;
}

static ssize_t attr_acc_calibrate_write(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long val = 0;
	unsigned long delay;
	int ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	char content[CLI_CONTENT_LEN_MAX] = { 0 };
	const int32_t *acc_cali_data = NULL;
	char date_str[CLI_TIME_STR_LEN] = { 0 };

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	/*if (1 != val) return count;*/
	if ((1 != val) && (2 != val) && (3 != val) && (4 != val) && (5 != val)
	    && (6 != val)) {
		return count;
	}
	if (sensor_status.opened[TAG_ACCEL] == 0) { /*if acc is not opened, open first*/
		acc_close_after_calibrate = true;
		hwlog_info("send acc open cmd(during calibrate) to mcu.\n");
		pkg_ap.tag = TAG_ACCEL;
		pkg_ap.cmd = CMD_CMN_OPEN_REQ;
		pkg_ap.wr_buf = NULL;
		pkg_ap.wr_len = 0;
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			return_calibration = COMMU_FAIL;
			hwlog_err
			    ("send acc open cmd(during calibrate) to mcu fail,ret=%d\n",
			     ret);
			return count;
		}
	} else {
		acc_close_after_calibrate = false;
	}
	/*period must <= 100 ms*/
	if ((sensor_status.delay[TAG_ACCEL] == 0)
	    || (sensor_status.delay[TAG_ACCEL] > 20)) {
		delay = 20;
		hwlog_info("send acc setdelay cmd(during calibrate) to mcu.\n");
		pkg_ap.tag = TAG_ACCEL;
		pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
		pkg_ap.wr_buf = &delay;
		pkg_ap.wr_len = sizeof(delay);
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			return_calibration = COMMU_FAIL;
			hwlog_err
			    ("send acc set delay cmd(during calibrate) to mcu fail,ret=%d\n",
			     ret);
			return count;
		}
	}
	msleep(300);
	/*send calibrate command, need set delay first*/
	pkg_ap.tag = TAG_ACCEL;
	pkg_ap.cmd = CMD_ACCEL_SELFCALI_REQ;
	/*pkg_ap.wr_buf=NULL;*/
	/*pkg_ap.wr_len=0;*/
	pkg_ap.wr_buf = &val;
	pkg_ap.wr_len = sizeof(val);
	hwlog_err("acc calibrator val is %d  len is %lu.\n", val, sizeof(val));
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	if (ret) {
		return_calibration = COMMU_FAIL;
		hwlog_err("send acc calibrate cmd to mcu fail,ret=%d\n", ret);
		return count;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("acc calibrate fail, %d\n", pkg_mcu.errno);
		return_calibration = EXEC_FAIL;
	} else {
		hwlog_info("acc calibrate  success, data len=%d\n",
			   pkg_mcu.data_length);
		acc_calibrate_save(pkg_mcu.data, pkg_mcu.data_length);
	}

	get_test_time(date_str, sizeof(date_str));
	acc_cali_data = (const int32_t *)pkg_mcu.data;

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_X_OFFSET,
		 *acc_cali_data,
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_Y_OFFSET,
		 *(acc_cali_data + 1),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_Z_OFFSET,
		 *(acc_cali_data + 2),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_X_SEN,
		 *(acc_cali_data + 3),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_Y_SEN,
		 *(acc_cali_data + 4),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_Z_SEN,
		 *(acc_cali_data + 5),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_XIS_00,
		 *(acc_cali_data + 6),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_XIS_01,
		 *(acc_cali_data + 7),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_XIS_02,
		 *(acc_cali_data + 8),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_XIS_10,
		 *(acc_cali_data + 9),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_XIS_11,
		 *(acc_cali_data + 10),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_XIS_12,
		 *(acc_cali_data + 11),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_XIS_20,
		 *(acc_cali_data + 12),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_XIS_21,
		 *(acc_cali_data + 13),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, ACC_CALI_XIS_22,
		 *(acc_cali_data + 14),
		 ((return_calibration == SUC) ? "pass" : "fail"), (int)val,
		 get_cali_error_code(return_calibration), date_str);
	save_to_file(DATA_CLLCT, content);

	if (acc_close_after_calibrate == true) {
		acc_close_after_calibrate = false;
		hwlog_info("send acc close cmd(during calibrate) to mcu.\n");
		pkg_ap.tag = TAG_ACCEL;
		pkg_ap.cmd = CMD_CMN_CLOSE_REQ;
		pkg_ap.wr_buf = NULL;
		pkg_ap.wr_len = 0;
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			return_calibration = COMMU_FAIL;
			hwlog_err
			    ("send acc close cmd(during calibrate) to mcu fail,ret=%d\n",
			     ret);
			return count;
		}
	}
	return count;
}

static DEVICE_ATTR(acc_calibrate, 0664, attr_acc_calibrate_show,
		   attr_acc_calibrate_write);

static ssize_t attr_gyro_calibrate_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	int val = gyro_calibration_res;
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static int gyro_calibrate_save(const void *buf, int length)
{
	const int32_t *poffset_data = (const int32_t *)buf;
	int ret = 0;
	if (buf == NULL || length <= 0) {
		hwlog_err("%s invalid argument.", __func__);
		gyro_calibration_res = EXEC_FAIL;
		return -1;
	}
	hwlog_info( "%s:gyro_sensor calibrate ok, %d  %d  %d %d  %d  %d %d  %d  %d %d  %d  %d  %d  %d  %d\n", __func__, *poffset_data, *(poffset_data + 1), *(poffset_data + 2),
            *(poffset_data + 3), *(poffset_data + 4),*(poffset_data + 5),*(poffset_data + 6), *(poffset_data + 7),*(poffset_data + 8), *(poffset_data + 9),*(poffset_data + 10),
            *(poffset_data + 11), *(poffset_data + 12),*(poffset_data + 13), *(poffset_data + 14));
	ret = write_gyro_sensor_offset_to_nv((char *)buf, length);
	if (ret) {
		hwlog_err("nv write fail.\n");
		gyro_calibration_res = NV_FAIL;
		return -1;
	}
	gyro_calibration_res = SUC;
	return 0;
}

static ssize_t attr_gyro_calibrate_write(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	unsigned long val = 0;
	unsigned long delay;
	int ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	char content[CLI_CONTENT_LEN_MAX] = { 0 };
	const int32_t *gyro_cali_data = NULL;
	char date_str[CLI_TIME_STR_LEN] = { 0 };

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	if((val < 1) || (val > 7)) {
		hwlog_err("set gyro calibrate val invalid,val=%lu\n", val);
		return count;
	}

	if (sensor_status.opened[TAG_GYRO] == 0) { /*if gyro is not opened, open first*/
		gyro_close_after_calibrate = true;
		hwlog_info("send gyro open cmd(during calibrate) to mcu.\n");
		pkg_ap.tag = TAG_GYRO;
		pkg_ap.cmd = CMD_CMN_OPEN_REQ;
		pkg_ap.wr_buf = NULL;
		pkg_ap.wr_len = 0;
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			gyro_calibration_res = COMMU_FAIL;
			hwlog_err
			    ("send gyro open cmd(during calibrate) to mcu fail,ret=%d\n",
			     ret);
			return count;
		}
	} else {
		gyro_close_after_calibrate = false;
	}

	if ((sensor_status.delay[TAG_GYRO] == 0)
		|| (sensor_status.delay[TAG_GYRO] > 10)) {
		delay = 10;
		hwlog_info
			("send gyro setdelay cmd(during calibrate) to mcu.\n");
		pkg_ap.tag = TAG_GYRO;
		pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
		pkg_ap.wr_buf = &delay;
		pkg_ap.wr_len = sizeof(delay);
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			gyro_calibration_res = COMMU_FAIL;
			hwlog_err
			    ("send gyro set delay cmd(during calibrate) to mcu fail,ret=%d\n",
			     ret);
			return count;
		}
	}
	msleep(300);
	/*send calibrate command, need set delay first*/
	pkg_ap.tag = TAG_GYRO;
	pkg_ap.cmd = CMD_GYRO_SELFCALI_REQ;
	pkg_ap.wr_buf = &val;
	pkg_ap.wr_len = sizeof(val);
	hwlog_err("gyro calibrator val is %lu  len is %lu.\n", val, sizeof(val));

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	if (ret) {
		gyro_calibration_res = COMMU_FAIL;
		hwlog_err("send gyro calibrate cmd to mcu fail,ret=%d\n", ret);
		return count;
	}
	gyro_cali_data = (const int32_t *)pkg_mcu.data;
	if (pkg_mcu.errno != 0) {
		hwlog_err("gyro calibrate fail, %d\n", pkg_mcu.errno);
		gyro_calibration_res = EXEC_FAIL;
	} else {
		if(val == 1){
			gyro_calib_data[0] = *gyro_cali_data;
			gyro_calib_data[1] = *(gyro_cali_data+1);
			gyro_calib_data[2] = *(gyro_cali_data+2);
			gyro_calibrate_save(gyro_calib_data,
						pkg_mcu.data_length);
		} else if(val == 7) {
		    gyro_calib_data[3] = *(gyro_cali_data+3);
		    gyro_calib_data[4] = *(gyro_cali_data+4);
		    gyro_calib_data[5] = *(gyro_cali_data+5);
		    gyro_calib_data[6] = *(gyro_cali_data+6);
		    gyro_calib_data[7] = *(gyro_cali_data+7);
		    gyro_calib_data[8] = *(gyro_cali_data+8);
		    gyro_calib_data[9] = *(gyro_cali_data+9);
		    gyro_calib_data[10] = *(gyro_cali_data+10);
		    gyro_calib_data[11] = *(gyro_cali_data+11);
		    gyro_calib_data[12] = *(gyro_cali_data+12);
		    gyro_calib_data[13] = *(gyro_cali_data+13);
		    gyro_calib_data[14] = *(gyro_cali_data+14);
			gyro_calibrate_save(gyro_calib_data,
						pkg_mcu.data_length);
		} else
		    gyro_calibration_res = SUC;

		hwlog_info("gyro calibrate success, val=%lu data=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d, len=%d\n", val,gyro_calib_data[0],gyro_calib_data[1],gyro_calib_data[2],
                    gyro_calib_data[3],gyro_calib_data[4],gyro_calib_data[5],gyro_calib_data[6],gyro_calib_data[7],gyro_calib_data[8],
                    gyro_calib_data[9],gyro_calib_data[10],gyro_calib_data[11],gyro_calib_data[12],gyro_calib_data[13],
                    gyro_calib_data[14],pkg_mcu.data_length);

	}

	if(val == 1 || val == 7) {
	    get_test_time(date_str, sizeof(date_str));
	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_X_OFFSET,
	    	gyro_calib_data[0], ((gyro_calibration_res == SUC) ?
	    	"pass" : "fail"), (int)val,
	    	get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_Y_OFFSET,
	    	gyro_calib_data[1], ((gyro_calibration_res == SUC) ?
	    	"pass":"fail"),(int)val,
	    	get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_Z_OFFSET,
	    	gyro_calib_data[2], ((gyro_calibration_res == SUC) ?
	    	"pass":"fail"),(int)val,
	    	get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_X_SEN,
	    	gyro_calib_data[3], ((gyro_calibration_res == SUC) ?
	    	"pass":"fail"),(int)val,
	    	get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_Y_SEN,
	    	gyro_calib_data[4], ((gyro_calibration_res == SUC) ?
	    	"pass":"fail"),(int)val,
	    	get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_Z_SEN,
	    	gyro_calib_data[5], ((gyro_calibration_res == SUC) ?
	    	"pass":"fail"),(int)val,
	    	get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_XIS_00,
			gyro_calib_data[6], ((gyro_calibration_res == SUC)?
			"pass":"fail"),(int)val,
			get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_XIS_01,
			gyro_calib_data[7], ((gyro_calibration_res == SUC)?
			"pass":"fail"),(int)val,
			get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_XIS_02,
			gyro_calib_data[8], ((gyro_calibration_res == SUC)?
			"pass":"fail"),(int)val,
			get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_XIS_10,
			gyro_calib_data[9], ((gyro_calibration_res == SUC)?
			"pass":"fail"),(int)val,
			get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_XIS_11,
			gyro_calib_data[10], ((gyro_calibration_res == SUC)?
			"pass":"fail"),(int)val,
			get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_XIS_12,
			gyro_calib_data[11], ((gyro_calibration_res == SUC)?
			"pass":"fail"),(int)val,
			get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_XIS_20,
			gyro_calib_data[12], ((gyro_calibration_res == SUC)?
			"pass":"fail"),(int)val,
			get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_XIS_21,
			gyro_calib_data[13], ((gyro_calibration_res == SUC)?
			"pass":"fail"),(int)val,
			get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);

	    memset(&content, 0, sizeof(content));
	    snprintf(content, CLI_CONTENT_LEN_MAX, GYRO_CALI_XIS_22,
			gyro_calib_data[14], ((gyro_calibration_res == SUC)?
			"pass":"fail"),(int)val,
			get_cali_error_code(gyro_calibration_res), date_str);
	    save_to_file(DATA_CLLCT, content);
	}

	if (gyro_close_after_calibrate == true) {
		gyro_close_after_calibrate = false;
		hwlog_info("send gyro close cmd(during calibrate) to mcu.\n");
		pkg_ap.tag = TAG_GYRO;
		pkg_ap.cmd = CMD_CMN_CLOSE_REQ;
		pkg_ap.wr_buf = NULL;
		pkg_ap.wr_len = 0;
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			gyro_calibration_res = COMMU_FAIL;
			hwlog_err
			    ("send gyro close cmd(during calibrate) to mcu fail,ret=%d\n",
			     ret);
			return count;
		}
	}
	return count;
}

static DEVICE_ATTR(gyro_calibrate, 0664, attr_gyro_calibrate_show,
		   attr_gyro_calibrate_write);

static ssize_t attr_ps_calibrate_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	int val = ps_calibration_res;
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static int ps_calibrate_save(const void *buf, int length)
{
	int temp_buf[3] = {0};
	int ret=0;
	if(buf == NULL||length <= 0 || length > 12)
	{
		hwlog_err("%s invalid argument.", __func__);
		ps_calibration_res=EXEC_FAIL;
		return -1;
	}
	memcpy(temp_buf, buf, length);
	hwlog_info( "%s:psensor calibrate ok, %d,%d,%d \n", __func__,
				 temp_buf[0], temp_buf[1], temp_buf[2]);
	if(ltr578_ps_external_ir_calibrate_flag == 1)
	{
	    ps_external_ir_param.external_ir_pwindows_value = temp_buf[2] - temp_buf[1];
	    ps_external_ir_param.external_ir_pwave_value = temp_buf[1] - temp_buf[0];
	    ps_external_ir_param.external_ir_calibrate_noise = temp_buf[0];
	    hwlog_info("%s:set nv ltr578 offset ps_data[0]:%d,ps_data[1]:%d,ps_data[2]:%d,pwindows:%d,pwave:%d\n",
	                __func__,temp_buf[0],temp_buf[1],temp_buf[2],\
	                ps_external_ir_param.external_ir_pwindows_value,ps_external_ir_param.external_ir_pwave_value);
	}
	ret = write_ps_offset_to_nv(temp_buf);
	if(ret)
	{
		hwlog_err("nv write fail.\n");
		ps_calibration_res=NV_FAIL;
		return -1;
	}
	ps_calibration_res=SUC;
	return 0;
}

static ssize_t attr_ps_calibrate_write(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = 0;
	write_info_t	pkg_ap;
	read_info_t	pkg_mcu;
	char content[CLI_CONTENT_LEN_MAX] = {0};
	char date_str[CLI_TIME_STR_LEN] = {0};

	if((txc_ps_flag != 1) && (ams_tmd2620_ps_flag != 1) &&
		(avago_apds9110_ps_flag != 1) && (ltr578_ps_external_ir_calibrate_flag != 1)) {
		hwlog_info("ps sensor is not txc_ps_224 or ams_tmd2620 or avago_apds9110,no need calibrate\n");
		return count;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;
	if((val < 1)||(val > 3)){
		hwlog_err("set ps calibrate val invalid,val=%lu\n", val);
		return count;
	}

	if(val == 1){
		ps_calib_state=0;
	}
	pkg_ap.tag=TAG_PS;
	pkg_ap.cmd=CMD_PS_SELFCALI_REQ;
	pkg_ap.wr_buf=&val;
	pkg_ap.wr_len=sizeof(val);
	hwlog_err("ps calibrator val is %lu  len is %lu.\n", val, sizeof(val));
	ret=write_customize_cmd(&pkg_ap,  &pkg_mcu);
	if(ret)
	{
		ps_calibration_res=COMMU_FAIL;
		hwlog_err("send ps calibrate cmd to mcu fail,ret=%d\n", ret);
		goto save_log;
	}
	if(pkg_mcu.errno!=0)
	{
		hwlog_err("ps calibrate fail, %d\n", pkg_mcu.errno);
		ps_calibration_res=EXEC_FAIL;
		goto save_log;
	}
	else
	{
		//hwlog_info("ps calibrate  success, data len=%d\n", pkg_mcu.data_length);
		ps_calib_data[ps_calib_state] = *((int32_t *)pkg_mcu.data);
		hwlog_info("ps calibrate success, data=%d, len=%d\n",
			ps_calib_data[ps_calib_state],pkg_mcu.data_length);
		ps_calib_state++;
	}

	if(val == 2){
		ps_calibration_res=SUC;
	}else{
		ps_calibrate_save(ps_calib_data, 3*pkg_mcu.data_length);
	}

save_log:
	get_test_time(date_str, sizeof(date_str));
	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, PS_CALI_RAW_DATA,
		ps_calib_data[val-1],((ps_calibration_res == SUC) ?
		"pass":"fail"), (int)val,
		get_cali_error_code(ps_calibration_res), date_str);
	save_to_file(DATA_CLLCT, content);

	return count;
}

static DEVICE_ATTR(ps_calibrate, 0664, attr_ps_calibrate_show,
		   attr_ps_calibrate_write);
static ssize_t attr_ps_switch_mode_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	int val = ps_switch_mode;
	return snprintf(buf, PAGE_SIZE, "%d",val);
}

static ssize_t attr_ps_switch_mode_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = 0;
	write_info_t	pkg_ap;
	read_info_t	pkg_mcu;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	if(ps_external_ir_param.external_ir == 1) {
		if (strict_strtoul(buf, 10, &val))
			return -EINVAL;
		if((val < 0)||(val > 1)){
			hwlog_err("set ps switch mode val invalid,val=%lu\n", val);
			return count;
		}
		if(val == 1){
			ps_extend_platform_data.external_ir_mode_flag  = 1;
			ps_extend_platform_data.min_proximity_value = ps_external_ir_param.external_ir_min_proximity_value;
			ps_extend_platform_data.pwindows_value = ps_external_ir_param.external_ir_pwindows_value;
			ps_extend_platform_data.pwave_value = ps_external_ir_param.external_ir_pwave_value;
			ps_extend_platform_data.threshold_value  = ps_external_ir_param.external_ir_threshold_value;
			ps_extend_platform_data.calibrate_noise  = ps_external_ir_param.external_ir_calibrate_noise;
		}
		else{
			ps_extend_platform_data.external_ir_mode_flag  = 0;
			ps_extend_platform_data.min_proximity_value = ps_external_ir_param.internal_ir_min_proximity_value;
			ps_extend_platform_data.pwindows_value = ps_external_ir_param.internal_ir_pwindows_value;
			ps_extend_platform_data.pwave_value = ps_external_ir_param.internal_ir_pwave_value;
			ps_extend_platform_data.threshold_value  = ps_external_ir_param.internal_ir_threshold_value;
		}

		hwlog_info("external_ir:%d,external_ir_enable_gpio:%d, min_proximity_value:%d, pwindows_value:%d, pwave_value:%d, threshold_value:%d\n",
							ps_extend_platform_data.external_ir_mode_flag,
							ps_external_ir_param.external_ir_enable_gpio,
							ps_extend_platform_data.min_proximity_value,
							ps_extend_platform_data.pwindows_value,
							ps_extend_platform_data.pwave_value,
							ps_extend_platform_data.threshold_value);

		pkg_ap.tag=TAG_PS;
		pkg_ap.cmd=CMD_PS_RESET_PARA_REQ;
		pkg_ap.wr_buf=(void *)(&ps_extend_platform_data);
		pkg_ap.wr_len=sizeof(ps_extend_platform_data);
		hwlog_err("ps switch mode val is %lu  len is %lu.\n", val, sizeof(val));
		ret=write_customize_cmd(&pkg_ap,  &pkg_mcu);
		if(ret)
		{
			hwlog_err("send ps switch mode cmd to mcu fail,ret=%d\n", ret);
			return count;
		}
		if(pkg_mcu.errno!=0)
		{
			hwlog_err("ps switch mode fail, %d\n", pkg_mcu.errno);
		}
		else
		{
			ps_switch_mode=val;
			if(val == 1){
				gpio_direction_output(ps_external_ir_param.external_ir_enable_gpio, 1);
			}
			else{
				gpio_direction_output(ps_external_ir_param.external_ir_enable_gpio, 0);
			}
			hwlog_info("ps switch mode  success, data len=%d\n", pkg_mcu.data_length);
		}
	}else {
		hwlog_err(" external_ir not enable in this product. operation fail!");
	}
	return count;
}
static DEVICE_ATTR(ps_switch_mode, 0664, attr_ps_switch_mode_show,
		   attr_ps_switch_mode_store);

static ssize_t attr_als_calibrate_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int val = als_calibration_res;
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static int rgb_cal_result_write_file(char *filename, char *param)
{
	struct file *fop;
	mm_segment_t old_fs;

	if (NULL == filename) {
		hwlog_err("filename is empty\n");
		return -1;
	}

	if (NULL == param) {
		hwlog_err("param is empty\n");
		return -1;
	}
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fop = filp_open(filename, O_CREAT | O_RDWR, 0644);
	if (IS_ERR_OR_NULL(fop)) {
		set_fs(old_fs);
		hwlog_err
		    ("Create file error!! Path = %s IS_ERR_OR_NULL(fop) = %d fop = %Kp\n",
		     filename, IS_ERR_OR_NULL(fop), fop);
		return -1;
	}

	vfs_write(fop, (char *)param, strlen(param), &fop->f_pos);
	filp_close(fop, NULL);
	set_fs(old_fs);

	hwlog_info("%s: write rgb calibration data to file ok\n", __func__);

	return 0;
}

static int als_calibrate_save(const void *buf, int length)
{
	const uint16_t *poffset_data = (const uint16_t *)buf;
	int ret = 0;
	uint16_t *bh1745_als_cal_result = (uint16_t *) buf;

	if (buf == NULL || length <= 0) {
		hwlog_err("%s invalid argument.", __func__);
		als_calibration_res = EXEC_FAIL;
		return -1;
	}

	hwlog_info("%s:als calibrate ok, %d  %d  %d %d  %d  %d\n", __func__,
		   *poffset_data, *(poffset_data + 1), *(poffset_data + 2),
		   *(poffset_data + 3), *(poffset_data + 4),
		   *(poffset_data + 5));
	ret = write_als_offset_to_nv((char *)buf);
	if (ret) {
		hwlog_err("nv write fail.\n");
		als_calibration_res = NV_FAIL;
		return -1;
	}
	als_calibration_res = SUC;
	return 0;
}

static ssize_t attr_als_calibrate_write(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = 0;
	unsigned long delay;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	char content[CLI_CONTENT_LEN_MAX] = { 0 };
	const int32_t *als_cali_data = NULL;
	char date_str[CLI_TIME_STR_LEN] = { 0 };

	if ( 1!=rohm_rgb_flag && 1!=avago_rgb_flag && 1!=is_cali_supported ) {
		hwlog_warn( "als sensor is not rohm_bh1745, avago_apds9251. is_cali_supported = %d.\
			no need to calibrate\n", is_cali_supported );
		return count;
	}

        /*read tp color from NV before als calibrate*/
        ret = get_tpcolor_from_nv();
        if(ret)
        {
                hwlog_err( "get_tpcolor_from_nv read from nv fail, ret=%d",ret);
        }

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	if (1 != val)
		return count;

	if (sensor_status.opened[TAG_ALS] == 0) { /*if ALS is not opened, open first*/
		als_close_after_calibrate = true;
		hwlog_info("send als open cmd(during calibrate) to mcu.\n");
		pkg_ap.tag = TAG_ALS;
		pkg_ap.cmd = CMD_CMN_OPEN_REQ;
		pkg_ap.wr_buf = NULL;
		pkg_ap.wr_len = 0;
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			als_calibration_res = COMMU_FAIL;
			hwlog_err
			    ("send ALS open cmd(during calibrate) to mcu fail,ret=%d\n",
			     ret);
			return count;
		}
	} else {
		als_close_after_calibrate = false;
	}
	/*period must <= 100 ms*/
	if ((sensor_status.delay[TAG_ALS] == 0)
	    || (sensor_status.delay[TAG_ALS] > 100)) {
		delay = 100;
		hwlog_info("send als setdelay cmd(during calibrate) to mcu.\n");
		pkg_ap.tag = TAG_ALS;
		pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
		pkg_ap.wr_buf = &delay;
		pkg_ap.wr_len = sizeof(delay);
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			als_calibration_res = COMMU_FAIL;
			hwlog_err
			    ("send ALS set delay cmd(during calibrate) to mcu fail,ret=%d\n",
			     ret);
			return count;
		}
	}
	msleep(350);
	/*send calibrate command, need set delay first*/

	pkg_ap.tag = TAG_ALS;
	pkg_ap.cmd = CMD_ALS_SELFCALI_REQ;
	pkg_ap.wr_buf = NULL;
	pkg_ap.wr_len = 0;
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	if (ret) {
		als_calibration_res = COMMU_FAIL;
		hwlog_err("send als calibrate cmd to mcu fail,ret=%d\n", ret);
		return count;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("als calibrate fail, %d\n", pkg_mcu.errno);
		als_calibration_res = EXEC_FAIL;
	} else {
		hwlog_info("als calibrate  success, data len=%d\n",
			   pkg_mcu.data_length);
		als_calibrate_save(pkg_mcu.data, pkg_mcu.data_length);
	}

	get_test_time(date_str, sizeof(date_str));
	als_cali_data = (const int32_t *)pkg_mcu.data;

	if ( 1==rohm_rgb_flag || 1==avago_rgb_flag ){

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_R, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_G, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_B, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_C, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_LUX, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_CCT, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

	}else if( 1==ltr578_flag || 1==apds9922_flag){
		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_ALS, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_IR, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_LUX, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

	}else if( 1==rohm_rpr531_flag){
		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_DATA0, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_DATA1, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_LUX, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

	}else if( 1==tmd2745_flag){
		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_VISIBLE, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_REF, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);

		memset(&content, 0, sizeof(content));
		snprintf(content, CLI_CONTENT_LEN_MAX, ALS_CALI_LUX, *als_cali_data,
				((als_calibration_res == SUC) ? "pass" : "fail"),
		get_cali_error_code(als_calibration_res), date_str);
		save_to_file(DATA_CLLCT, content);
	}

    if(als_close_after_calibrate == true) {
        als_close_after_calibrate = false;
        hwlog_info("send als close cmd(during calibrate) to mcu.\n");
		pkg_ap.tag=TAG_ALS;
		pkg_ap.cmd=CMD_CMN_CLOSE_REQ;
		pkg_ap.wr_buf=NULL;
		pkg_ap.wr_len=0;
		ret=write_customize_cmd(&pkg_ap,  &pkg_mcu);
		if(ret)
		{
			als_calibration_res=COMMU_FAIL;
			hwlog_err("send ALS close cmd(during calibrate) to mcu fail,ret=%d\n", ret);
			return count;
		}
    }
	return count;
}

static DEVICE_ATTR(als_calibrate, 0664, attr_als_calibrate_show,
		   attr_als_calibrate_write);

static ssize_t attr_cap_prox_calibrate_show(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	int ret = 0;
	u32 *pcaldata = NULL;
	struct hisi_nve_info_user user_info;

	memset(&user_info, 0, sizeof(user_info));

	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = CAP_PROX_CALIDATA_NV_NUM;
	user_info.valid_size = CAP_PROX_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "Csensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return snprintf(buf, PAGE_SIZE, "%d\n",
				return_cap_prox_calibration);
	}

	pcaldata = (u32 *) (user_info.nv_data + 4);
	return snprintf(buf, PAGE_SIZE, "%d:%08x %08x %08x\n",
			return_cap_prox_calibration, pcaldata[0], pcaldata[1],
			pcaldata[2]);
}

static int cap_prox_calibrate_save(const void *buf, int length)
{
	int ret=0;
	u32 *pcaldata = NULL;
	struct hisi_nve_info_user user_info;

	if (buf == NULL ) {
		hwlog_err("%s invalid argument.", __func__);
		return_cap_prox_calibration = EXEC_FAIL;
		return -1;
	}
	pcaldata = (u32 *)buf;
	hwlog_err("%s:cap_prox calibrate ok, %08x  %08x  %08x \n", __func__,
				pcaldata[0],pcaldata[1],pcaldata[2]);

	memset(&user_info, 0, sizeof(user_info));
	//read from nv
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = CAP_PROX_CALIDATA_NV_NUM;
	user_info.valid_size = CAP_PROX_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "Csensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	if ((ret = hisi_nve_direct_access(&user_info))!=0) {
		return_cap_prox_calibration = EXEC_FAIL;
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return -2;
	}

	memcpy(user_info.nv_data, buf, length);
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = CAP_PROX_CALIDATA_NV_NUM;
	user_info.valid_size = CAP_PROX_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "Csensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	if ((ret = hisi_nve_direct_access(&user_info)) != 0) {
		return_cap_prox_calibration = EXEC_FAIL;
		hwlog_err("nve_direct_access write error(%d)\n", ret);
		return -3;
	}

	return_cap_prox_calibration = SUC;
	return 0;
}

static struct work_struct cap_prox_calibrate_work;
static void cap_prox_calibrate_work_func(struct work_struct *work)
{
	int ret = 0;

	hwlog_info("cap_prox calibrate work enter ++\n");
	ret = cap_prox_calibrate_save(cap_prox_calibrate_data,cap_prox_calibrate_len);
	if (ret < 0)  {
		hwlog_err("nv write faild.\n");
	}
	hwlog_info("cap_prox calibrate work enter --\n");
}
static ssize_t attr_cap_prox_calibrate_write(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = 0;
	int calibrate_index = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;

	hwlog_info("attr_cap_prox_calibrate_write\n");
				  ;
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	if (strlen(sensor_chip_info[CAP_PROX]) == 0) {
		hwlog_err("no sar sensor\n");
		return -EINVAL;
	}
	/* init the result failed */
	return_cap_prox_calibration = EXEC_FAIL;
	calibrate_index = (int)val;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	hwlog_info("cap_prox_calibrate : %lu\n", val);
	if (val >= 0) {
		pkg_ap.tag = TAG_CAP_PROX;
		pkg_ap.cmd = CMD_CAP_PROX_CALIBRATE_REQ;
		pkg_ap.wr_buf = &val;
		pkg_ap.wr_len = sizeof(val);
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			return_cap_prox_calibration = COMMU_FAIL;
			hwlog_err
			    ("send cap_prox calibrate cmd to mcu fail,ret=%d\n",
			     ret);
			return count;
		}
		if (pkg_mcu.errno != 0) {
			hwlog_err("cap_prox calibrate fail\n");
			return_cap_prox_calibration = EXEC_FAIL;
		} else {
			hwlog_info("cap_prox calibrate  success, data len=%d\n",
				   pkg_mcu.data_length);

			if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,cypress_sar_psoc4000", strlen("huawei,cypress_sar_psoc4000"))) {
				uint16_t cypress_data[2] = {0};
				memcpy(cypress_data, pkg_mcu.data, sizeof(cypress_data));
				switch(calibrate_index) {
				case 0:	/* free data */
					sar_calibrate_datas.cypres_cali_data.sar_idac = cypress_data[0];
					sar_calibrate_datas.cypres_cali_data.raw_data = cypress_data[1];
					break;
				case 1: /* near data */
					sar_calibrate_datas.cypres_cali_data.near_signaldata = cypress_data[0];
					break;
				case 2:/* far data */
					sar_calibrate_datas.cypres_cali_data.far_signaldata = cypress_data[0];
#if 0
					snprintf(content, CLI_CONTENT_LEN_MAX, "idac:%u, rawdata:%u, near:%u, far:%u\n",
						sar_calibrate_datas.cypres_cali_data.sar_idac,
						sar_calibrate_datas.cypres_cali_data.raw_data,
						sar_calibrate_datas.cypres_cali_data.near_signaldata,
						sar_calibrate_datas.cypres_cali_data.far_signaldata);
					save_to_file(SAR_DATA_CLLCT, content);
#endif
					break;
				default:
					hwlog_err("sar calibrate err\n");
					break;
				}
				hwlog_info("cypress_data %u %u\n", cypress_data[0], cypress_data[1]);
				hwlog_info("idac:%d,rawdata:%d,near:%d,far:%d\n",
					sar_calibrate_datas.cypres_cali_data.sar_idac,
					sar_calibrate_datas.cypres_cali_data.raw_data,
					sar_calibrate_datas.cypres_cali_data.near_signaldata,
					sar_calibrate_datas.cypres_cali_data.far_signaldata);
				cap_prox_calibrate_len = sizeof(cap_prox_calibrate_data);
				memset(cap_prox_calibrate_data, 0, cap_prox_calibrate_len);
				memcpy(cap_prox_calibrate_data, &sar_calibrate_datas, cap_prox_calibrate_len);
		    }
                  else if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,semtech-sx9323", strlen("huawei,semtech-sx9323")))
                  {
			uint16_t semtech = 0;
			memcpy(&semtech, pkg_mcu.data, sizeof(semtech));
			switch(calibrate_index) {
				case 1: /* near data */
					sar_calibrate_datas.semtech_cali_data.diff= semtech;
					break;
				case 2:/* far data */
					sar_calibrate_datas.semtech_cali_data.offset = semtech;
					break;
				default:
					hwlog_err(" semtech sar calibrate err\n");
					break;
			}
			hwlog_info("semtech_data %u\n", semtech);
			hwlog_info("semtech_data offset:%d,diff:%d\n",
					sar_calibrate_datas.semtech_cali_data.offset,
					sar_calibrate_datas.semtech_cali_data.diff);
			cap_prox_calibrate_len = pkg_mcu.data_length;
			memset(cap_prox_calibrate_data, 0, cap_prox_calibrate_len);
			memcpy(cap_prox_calibrate_data, &sar_calibrate_datas, cap_prox_calibrate_len);
		    } else {
				cap_prox_calibrate_len = pkg_mcu.data_length;
				memcpy(cap_prox_calibrate_data, pkg_mcu.data, sizeof(cap_prox_calibrate_data));
		    }
			INIT_WORK(&cap_prox_calibrate_work, cap_prox_calibrate_work_func);
			queue_work(system_power_efficient_wq, &cap_prox_calibrate_work);

		}
	}
	return count;
}


static DEVICE_ATTR(cap_prox_calibrate, 0664, attr_cap_prox_calibrate_show,
			attr_cap_prox_calibrate_write);

static ssize_t attr_cap_prox_freespace_show(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;
	u16 *pfreespace = NULL;

	memset(&user_info, 0, sizeof(user_info));

	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = CAP_PROX_CALIDATA_NV_NUM;
	user_info.valid_size = CAP_PROX_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "Csensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return -1;
	}

	pfreespace = (u16 *) user_info.nv_data;
	return snprintf(buf, MAX_STR_SIZE, "%04x%04x\n", pfreespace[0],
			pfreespace[1]);
}

static ssize_t attr_cap_prox_freespace_write(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	uint32_t val = 0;
	char *pt = NULL;
	int ret = 0;
	struct hisi_nve_info_user user_info;
	u16 *pfreespace = NULL;

	val = simple_strtoul(buf, &pt, 0);

	memset(&user_info, 0, sizeof(user_info));

	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = CAP_PROX_CALIDATA_NV_NUM;
	user_info.valid_size = CAP_PROX_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "Csensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error(%d)\n", ret);
		return -1;
	}

	pfreespace = (u16 *) user_info.nv_data;
	pfreespace[0] = (u16) ((val >> 16) & 0xffff);
	pfreespace[1] = (u16) (val & 0xffff);

	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = CAP_PROX_CALIDATA_NV_NUM;
	user_info.valid_size = CAP_PROX_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "Csensor", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access write error(%d)\n", ret);
		return -2;
	}

	return count;
}


static DEVICE_ATTR(cap_prox_freespace, 0664, attr_cap_prox_freespace_show,
			attr_cap_prox_freespace_write);


/*if val is odd, then last status is sleep, if is even number, then last status is wakeup */
static ssize_t attr_iom3_sr_test_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	unsigned long val = 0;
	unsigned long times = 0;

	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	times = val;

	if (val > 0) {
		for (; val > 0; val--) {
			disable_sensors_when_suspend();
			tell_ap_status_to_mcu(ST_SLEEP);
			msleep(2);
			tell_ap_status_to_mcu(ST_WAKEUP);
			enable_sensors_when_resume();
		}

		if (times % 2) {
			tell_ap_status_to_mcu(ST_SLEEP);
			enable_sensors_when_resume();
		}
	}
	return count;
}

static DEVICE_ATTR(iom3_sr_test, 0660, NULL, attr_iom3_sr_test_store);

int fingersense_commu(unsigned int cmd, unsigned int pare,
		      unsigned int responsed)
{
	int ret = -1;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = TAG_FINGERSENSE;
	pkg_ap.cmd = cmd;
	pkg_ap.wr_buf = &pare;
	pkg_ap.wr_len = sizeof(pare);

	if (responsed == NO_RESP)
		ret = write_customize_cmd(&pkg_ap, NULL);	/*enable/disable fingersense is no response */
	else
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);	/*request fingersense data need response */
	if (ret) {
		hwlog_err("send finger sensor cmd(%d) to mcu fail,ret=%d\n",
			  cmd, ret);
		return ret;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("send finger sensor cmd(%d) to mcu fail(%d)\n",
				cmd, pkg_mcu.errno);
	}
	return ret;
}

int fingersense_enable(unsigned int enable)
{
	unsigned int cmd = 0;
	unsigned int delay = 10000;
	unsigned int ret = -1;

	if (1 == enable) {
		cmd = CMD_CMN_OPEN_REQ;
		ret = fingersense_commu(cmd, enable, NO_RESP);
		if (ret) {
			hwlog_err("%s: finger sense enable fail\n",
				  __FUNCTION__);
			return ret;
		}

		cmd = CMD_CMN_INTERVAL_REQ;

		ret = fingersense_commu(cmd, delay, NO_RESP);
		if (ret) {
			hwlog_err("%s: set delay fail\n", __FUNCTION__);
			return ret;
		}
		hwlog_info("%s: finger sense enable succsess\n", __FUNCTION__);
	} else {
		cmd = CMD_CMN_CLOSE_REQ;
		ret = fingersense_commu(cmd, enable, NO_RESP);
		if (ret) {
			hwlog_info("%s: finger sense close fail\n",
				   __FUNCTION__);
			return ret;
		}
		hwlog_info("%s: finger sense close succsess\n", __FUNCTION__);
	}

	return 0;
}

static ssize_t attr_set_fingersense_enable(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret = -1;

	if (strict_strtoul(buf, 10, &val)) {
		hwlog_err("%s: finger sense enable val(%lu) invalid",
			  __FUNCTION__, val);
		return -EINVAL;
	}

	hwlog_info("%s: finger sense enable val (%ld)\n", __FUNCTION__, val);
	if ((val != 0) && (val != 1)) {
		hwlog_err("%s:finger sense set enable fail, invalid val\n",
			  __FUNCTION__);
		return size;
	}

	if (fingersense_enabled == val) {
		hwlog_info
		    ("%s:finger sense already at seted state,fingersense_enabled:%d\n",
		     __FUNCTION__, fingersense_enabled);
		return size;
	}
	ret = fingersense_enable(val);
	if (ret) {
		hwlog_err("%s: finger sense enable fail: %d \n", __FUNCTION__,
			  ret);
		return size;
	}
	fingersense_enabled = val;

	return size;
}

static ssize_t attr_get_fingersense_enable(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", fingersense_enabled);
}

static DEVICE_ATTR(set_fingersense_enable, 0660, attr_get_fingersense_enable,
		   attr_set_fingersense_enable);

static ssize_t attr_fingersense_data_ready(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", fingersense_data_ready);
}

static DEVICE_ATTR(fingersense_data_ready, 0440, attr_fingersense_data_ready,
		   NULL);

static ssize_t attr_fingersense_latch_data(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	size = min(size, sizeof(fingersense_data));

	if ((!fingersense_data_ready)
	    || (!fingersense_enabled)) {
		hwlog_err
		    ("%s:fingersense zaxix not ready(%d) or not enable(%d)\n",
		     __FUNCTION__, fingersense_data_ready, fingersense_enabled);
		return size;
	}

	memcpy(buf, (char *)fingersense_data, size);

	return size;
}

static DEVICE_ATTR(fingersense_latch_data, 0440, attr_fingersense_latch_data,
		   NULL);

/*
* Calculate whether a is in the range of [b, c].
*/
int is_time_inrange(unsigned long a, unsigned long b, unsigned long c)
{
	return ((long)(a - b) >= 0) && ((long)(a - c) <= 0);
}

static ssize_t attr_fingersense_req_data(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t size)
{
	int ret = -1;
	unsigned int cmd = CMD_CMN_CONFIG_REQ;
	unsigned int sub_cmd = CMD_ACCEL_FINGERSENSE_REQ_DATA_REQ;
	unsigned int skip = 0;
	unsigned long local_jiffies = jiffies;
	unsigned long flags = 0;
	
#if defined(CONFIG_HISI_VIBRATOR)
	if ((vibrator_shake == 1) || (HALL_COVERD & hall_value)) {
		hwlog_err
		    ("coverd, vibrator shaking, not send fingersense req data cmd to mcu\n");
		return -1;
	}
#endif
	if (!fingersense_enabled) {
		hwlog_err("%s: finger sense not enable,  dont req data\n",
			  __FUNCTION__);
		return size;
	}

	spin_lock_irqsave(&fsdata_lock, flags);
	
	/* We started transmitting the data in recent time. It's just on the way. Wait for it. */
	if (fingersense_data_intrans
		&& is_time_inrange(fingersense_data_ts, local_jiffies - FINGERSENSE_TRANS_TOUT, local_jiffies))  {
		skip = 1;
	}

	/* The data was collected a short while ago. Just use it. */
	if (fingersense_data_ready
		&& (is_time_inrange(fingersense_data_ts, local_jiffies - FINGERSENSE_FRESH_TIME, local_jiffies))) {
		skip = 1;
	}

	if (skip) {
		spin_unlock_irqrestore(&fsdata_lock, flags);
		return size;
	}

	fingersense_data_ready = false;

	spin_unlock_irqrestore(&fsdata_lock, flags);

	ret = fingersense_commu(cmd, sub_cmd, NO_RESP);
	if (ret) {
		hwlog_err("%s: finger sense send requst data failed\n",
			  __FUNCTION__);
		return size;
	}

	spin_lock_irqsave(&fsdata_lock, flags);
	
	fingersense_data_intrans = true;   /* the data is on the way */
	fingersense_data_ts = jiffies;     /* record timestamp for the data */

	spin_unlock_irqrestore(&fsdata_lock, flags);
	return size;
}

void preread_fingersense_data(void)
{
#if defined(CONFIG_HISI_VIBRATOR)

	if ((vibrator_shake == 1) || (HALL_COVERD & hall_value)) {
		return;
	}
#endif

	if (!fingersense_enabled) {
		return;
	}

	attr_fingersense_req_data(NULL, NULL, NULL, (unsigned long)0);
}

EXPORT_SYMBOL(preread_fingersense_data);

static DEVICE_ATTR(fingersense_req_data, 0220, NULL, attr_fingersense_req_data);


int rpc_commu(unsigned int cmd, unsigned int pare,
		      unsigned int responsed)
{
	int ret = -1;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	rpc_ioctl_t pkg_ioctl;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = TAG_RPC;
	pkg_ap.cmd = cmd;
	pkg_ioctl.sub_cmd = pare;
	pkg_ap.wr_buf = &pkg_ioctl;
	pkg_ap.wr_len = sizeof(pkg_ioctl);

	if (responsed == NO_RESP){
		ret = write_customize_cmd(&pkg_ap, NULL);
        }
	if (ret) {
		hwlog_err("send rpc cmd(%d) to mcu fail,ret=%d\n",
			  cmd, ret);
		return ret;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("send rpc cmd(%d) to mcu fail(%d)\n",
				cmd, pkg_mcu.errno);
	}
	return ret;
}

int rpc_motion(unsigned int motion)
{
	unsigned int cmd = 0;
	unsigned int sub_cmd = 0;
	unsigned int ret = -1;
	if (1 == motion) {
		cmd = CMD_CMN_CONFIG_REQ;
		sub_cmd = CMD_RPC_START_REQ;
		ret = rpc_commu(cmd, sub_cmd , NO_RESP);
		if (ret) {
			hwlog_err("%s: rpc motion enable fail\n",
				  __FUNCTION__);
			return ret;
		}
		hwlog_info("%s: rpc motion start succsess\n", __FUNCTION__);
	} else {
		cmd = CMD_CMN_CONFIG_REQ;
		sub_cmd = CMD_RPC_STOP_REQ;
		ret = rpc_commu(cmd,sub_cmd, NO_RESP);
		if (ret) {
			hwlog_info("%s: rpc motion close fail\n",
				   __FUNCTION__);
			return ret;
		}
		hwlog_info("%s: rpc motion stop succsess\n", __FUNCTION__);
	}

	return ret;
}

/*acc enable node*/
#define SHOW_ENABLE_FUNC(NAME, TAG)\
static ssize_t show_##NAME##_enable_result(struct device *dev,\
				struct device_attribute *attr, char *buf)\
{\
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_status.status[TAG]);\
}

#define STORE_ENABLE_FUNC(NAME, TAG, CMD1, CMD2)\
static ssize_t attr_set_##NAME##_enable(struct device *dev, struct device_attribute *attr,\
				const char *buf, size_t size)\
{\
	unsigned long val = 0;\
	int ret = -1;\
	write_info_t	pkg_ap;\
	read_info_t pkg_mcu;\
	memset(&pkg_ap, 0, sizeof(pkg_ap));\
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));\
	if (strict_strtoul(buf, 10, &val))\
		return -EINVAL;\
	if (1 == val) {\
		pkg_ap.tag = TAG;\
		pkg_ap.cmd = CMD1;\
		pkg_ap.wr_buf = NULL;\
		pkg_ap.wr_len = 0;\
		ret = write_customize_cmd(&pkg_ap,  &pkg_mcu);\
		if (ret) {\
			hwlog_err("send %s enable cmd to mcu fail,ret=%d\n", #NAME, ret);\
			return size;\
		} \
		if (pkg_mcu.errno != 0) \
			hwlog_err("set %s enable fail\n", #NAME);\
		else \
			hwlog_info("%s enable success\n", #NAME);\
		if ((TAG == TAG_ACCEL) && (acc_close_after_calibrate == true)) {\
			acc_close_after_calibrate = false;\
			hwlog_info("%s received open command during calibrate, will not close after calibrate!\n", #NAME);\
		} \
	} else {\
		pkg_ap.tag = TAG;\
		pkg_ap.cmd = CMD2;\
		pkg_ap.wr_buf = NULL;\
		pkg_ap.wr_len = 0;\
		ret = write_customize_cmd(&pkg_ap,  &pkg_mcu);\
		if (ret) {\
			hwlog_err("send %s disable cmd to mcu fail,ret=%d\n", #NAME, ret);\
			return size;\
		} \
		if (pkg_mcu.errno != 0)\
			hwlog_err("set %s disable fail\n", #NAME);\
		else\
			hwlog_info("%s disable success\n", #NAME);\
	} \
	return size;\
}

#define SHOW_DELAY_FUNC(NAME, TAG) \
static ssize_t show_##NAME##_delay_result(struct device *dev,\
				struct device_attribute *attr, char *buf)\
{\
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_status.delay[TAG]);\
}

#define STORE_DELAY_FUNC(NAME, TAG, CMD)  \
static ssize_t attr_set_##NAME##_delay(struct device *dev, struct device_attribute *attr,\
				const char *buf, size_t size)\
{\
	unsigned long val = 0;\
	int ret = -1;\
	write_info_t	pkg_ap;\
	read_info_t pkg_mcu;\
\
	memset(&pkg_ap, 0, sizeof(pkg_ap));\
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));\
	if (sensor_status.opened[TAG] == 0) {\
		hwlog_err("send tag %d delay must be opend first! \n", TAG);\
		return -EINVAL;\
	} \
	if (strict_strtoul(buf, 10, &val))\
		return -EINVAL;\
	if (val >= 10 && val < 1000) {\
		pkg_ap.tag = TAG;\
		pkg_ap.cmd = CMD;\
		pkg_ap.wr_buf = &val;\
		pkg_ap.wr_len = sizeof(val);\
		ret = write_customize_cmd(&pkg_ap,  &pkg_mcu);\
		if (ret) {\
			hwlog_err("send %s delay cmd to mcu fail,ret=%d\n", #NAME, ret);\
			return size;\
		} \
		if (pkg_mcu.errno != 0)\
			hwlog_err("set %s delay fail\n", #NAME);\
		else {\
			hwlog_info("set %s delay (%ld)success\n", #NAME, val);\
		} \
	} \
	return size;\
}

static int is_gsensor_gather_enable;

static ssize_t attr_set_gsensor_gather_enable(struct device *dev,
					      struct device_attribute *attr,
					      const char *buf, size_t size)
{
	int ret = -1;
	unsigned long enable = 0;
	unsigned int delay = 50;

	if (strict_strtoul(buf, 10, &enable))
		return -EINVAL;

	if ((enable != 0) && (enable != 1))
		return -EINVAL;

	if (is_gsensor_gather_enable == enable) {
		hwlog_info
		    ("gsensor gather already seted to state, is_gsensor_gather_enable(%d)\n",
		     is_gsensor_gather_enable);
		return size;
	}

	ret = inputhub_sensor_enable(TAG_GPS_4774_I2C, enable);
	if (ret) {
		hwlog_err("send GSENSOR GATHER enable cmd to mcu fail,ret=%d\n",
			  ret);
		return -EINVAL;
	}

	if (1 == enable) {
		ret = inputhub_sensor_setdelay(TAG_GPS_4774_I2C, delay);
		if (ret) {
			hwlog_err
			    ("send GSENSOR GATHER set delay cmd to mcu fail,ret=%d\n",
			     ret);
			return -EINVAL;
		}
	}

	is_gsensor_gather_enable = enable;
	hwlog_info("GSENSOR GATHER set to state(%lu) success\n", enable);

	return size;
}

SHOW_ENABLE_FUNC(acc, TAG_ACCEL)
STORE_ENABLE_FUNC(acc, TAG_ACCEL, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(acc_enable, 0664, show_acc_enable_result,
		   attr_set_acc_enable);
SHOW_DELAY_FUNC(acc, TAG_ACCEL)
STORE_DELAY_FUNC(acc, TAG_ACCEL, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(acc_setdelay, 0664, show_acc_delay_result,
		   attr_set_acc_delay);

static DEVICE_ATTR(gsensor_gather_enable, 0664, NULL,
		   attr_set_gsensor_gather_enable);

SHOW_ENABLE_FUNC(gyro, TAG_GYRO)
STORE_ENABLE_FUNC(gyro, TAG_GYRO, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(gyro_enable, 0664, show_gyro_enable_result,
		   attr_set_gyro_enable);
SHOW_DELAY_FUNC(gyro, TAG_GYRO)
STORE_DELAY_FUNC(gyro, TAG_GYRO, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(gyro_setdelay, 0664, show_gyro_delay_result,
		   attr_set_gyro_delay);

SHOW_ENABLE_FUNC(mag, TAG_MAG)
STORE_ENABLE_FUNC(mag, TAG_MAG, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(mag_enable, 0664, show_mag_enable_result,
		   attr_set_mag_enable);
SHOW_DELAY_FUNC(mag, TAG_MAG)
STORE_DELAY_FUNC(mag, TAG_MAG, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(mag_setdelay, 0664, show_mag_delay_result,
		   attr_set_mag_delay);

SHOW_ENABLE_FUNC(als, TAG_ALS)
STORE_ENABLE_FUNC(als, TAG_ALS, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(als_enable, 0664, show_als_enable_result,
		   attr_set_als_enable);
SHOW_DELAY_FUNC(als, TAG_ALS)
STORE_DELAY_FUNC(als, TAG_ALS, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(als_setdelay, 0664, show_als_delay_result,
		   attr_set_als_delay);

SHOW_ENABLE_FUNC(sbl, TAG_LABC)
STORE_ENABLE_FUNC(sbl, TAG_LABC, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(sbl_enable, 0664, show_sbl_enable_result,
		   attr_set_sbl_enable);
SHOW_DELAY_FUNC(sbl, TAG_LABC)
STORE_DELAY_FUNC(sbl, TAG_LABC, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(sbl_setdelay, 0664, show_sbl_delay_result,
		   attr_set_sbl_delay);

#define BL_SETTING_LEN 16
static char buffer[BL_SETTING_LEN] = { 0 };

static ssize_t show_sbl_backlight_result(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "[LABC] sbl_backlight '%s'\n",
			buffer);
}

static ssize_t attr_set_sbl_backlight(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t size)
{
	int ret = -1;
	/*char buffer[BL_SETTING_LEN]={0};*/
	write_info_t pkg_ap;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	strncpy(buffer, buf, BL_SETTING_LEN - 1);
	if (buf[size - 1] == '\n' && size < BL_SETTING_LEN)
		buffer[size - 1] = 0;
	hwlog_debug("[LABC] set backlight '%s'\n", buffer);
	pkg_ap.tag = TAG_LABC;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = buffer;
	pkg_ap.wr_len = BL_SETTING_LEN;
	ret = write_customize_cmd(&pkg_ap, NULL);

	if (ret)
		hwlog_err("[LABC] send backlight cmd to mcu fail,ret=%d\n",
			  ret);

	return size;
}

static DEVICE_ATTR(sbl_setbacklight, 0664, show_sbl_backlight_result,
		   attr_set_sbl_backlight);

SHOW_ENABLE_FUNC(ps, TAG_PS)
STORE_ENABLE_FUNC(ps, TAG_PS, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(ps_enable, 0664, show_ps_enable_result, attr_set_ps_enable);
SHOW_DELAY_FUNC(ps, TAG_PS)
STORE_DELAY_FUNC(ps, TAG_PS, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(ps_setdelay, 0664, show_ps_delay_result, attr_set_ps_delay);

SHOW_ENABLE_FUNC(os, TAG_ORIENTATION)
STORE_ENABLE_FUNC(os, TAG_ORIENTATION, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(orientation_enable, 0664, show_os_enable_result,
		   attr_set_os_enable);
SHOW_DELAY_FUNC(os, TAG_ORIENTATION)
STORE_DELAY_FUNC(os, TAG_ORIENTATION, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(orientation_setdelay, 0664, show_os_delay_result,
		   attr_set_os_delay);

SHOW_ENABLE_FUNC(lines, TAG_LINEAR_ACCEL)
STORE_ENABLE_FUNC(lines, TAG_LINEAR_ACCEL, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(lines_enable, 0664, show_lines_enable_result,
		   attr_set_lines_enable);
SHOW_DELAY_FUNC(lines, TAG_LINEAR_ACCEL)
STORE_DELAY_FUNC(lines, TAG_LINEAR_ACCEL, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(lines_setdelay, 0664, show_lines_delay_result,
		   attr_set_lines_delay);

SHOW_ENABLE_FUNC(gras, TAG_GRAVITY)
STORE_ENABLE_FUNC(gras, TAG_GRAVITY, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(gras_enable, 0664, show_gras_enable_result,
		   attr_set_gras_enable);
SHOW_DELAY_FUNC(gras, TAG_GRAVITY)
STORE_DELAY_FUNC(gras, TAG_GRAVITY, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(gras_setdelay, 0664, show_gras_delay_result,
		   attr_set_gras_delay);

SHOW_ENABLE_FUNC(rvs, TAG_ROTATION_VECTORS)
STORE_ENABLE_FUNC(rvs, TAG_ROTATION_VECTORS, CMD_CMN_OPEN_REQ,
		  CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(rvs_enable, 0664, show_rvs_enable_result,
		   attr_set_rvs_enable);
SHOW_DELAY_FUNC(rvs, TAG_ROTATION_VECTORS)
STORE_DELAY_FUNC(rvs, TAG_ROTATION_VECTORS, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(rvs_setdelay, 0664, show_rvs_delay_result,
		   attr_set_rvs_delay);

SHOW_ENABLE_FUNC(airpress, TAG_PRESSURE)
STORE_ENABLE_FUNC(airpress, TAG_PRESSURE, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(airpress_enable, 0664, show_airpress_enable_result,
		   attr_set_airpress_enable);
SHOW_DELAY_FUNC(airpress, TAG_PRESSURE)
STORE_DELAY_FUNC(airpress, TAG_PRESSURE, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(airpress_setdelay, 0664, show_airpress_delay_result,
		   attr_set_airpress_delay);

static ssize_t attr_set_pdr_delay(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret = -1;
	int start_update_flag;
	int precise;
	int interval;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pdr_ioctl_t pkg_ioctl;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;
	/*val define: xyyzzz x:0 start 1 update yy:precise zzz:interval*/
	if (val == 0)
		val = 1010;
	start_update_flag = (val / 100000);
	precise = (val / 1000) % 100;
	interval = val % 1000;

	hwlog_info
	    ("val = %lu start_update_flag = %d precise = %d interval= %d\n", val,
	     start_update_flag, precise, interval);
	if (precise == 0)
		precise = 1;
	if (interval == 0)
		interval = 240;

	pkg_ap.tag = TAG_PDR;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;

	if (val >= 1000) {
		pkg_ioctl.sub_cmd =
		    (start_update_flag ==
		     0 ? CMD_FLP_PDR_START_REQ : CMD_FLP_PDR_UPDATE_REQ);
		pkg_ioctl.start_param.report_interval = interval * 1000;
		pkg_ioctl.start_param.report_precise = precise * 1000;
		pkg_ioctl.start_param.report_count = interval / precise;
		pkg_ioctl.start_param.report_times = 0;
		pkg_ap.wr_buf = &pkg_ioctl;
		pkg_ap.wr_len = sizeof(pkg_ioctl);
	} else if (val == 2) { 	/*2: stop command*/
		pkg_ioctl.sub_cmd = CMD_FLP_PDR_STOP_REQ;
		pkg_ioctl.stop_param = 30000;
		pkg_ap.wr_buf = &pkg_ioctl;
		pkg_ap.wr_len = sizeof(pkg_ioctl);
	}
	hwlog_info(" pkg_ioctl.sub_cmd = %d\n ", pkg_ioctl.sub_cmd);
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	if (ret) {
		hwlog_err("send pdr delay cmd to mcu fail,ret=%d\n", ret);
		return size;
	}
	if (pkg_mcu.errno != 0)
		hwlog_err("set pdr delay fail\n");
	else
		hwlog_info("set pdr delay (%ld)success\n", val);

	return size;
}

/*SHOW_ENABLE_FUNC(pdr, TAG_PDR)*/
STORE_ENABLE_FUNC(pdr, TAG_PDR, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(pdr_enable, 0220, NULL, attr_set_pdr_enable);
/*SHOW_DELAY_FUNC(pdr, TAG_PDR)*/
static DEVICE_ATTR(pdr_setdelay, 0220, NULL, attr_set_pdr_delay);


SHOW_ENABLE_FUNC(handpress, TAG_HANDPRESS)
STORE_ENABLE_FUNC(handpress, TAG_HANDPRESS, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(handpress_enable, 0664, show_handpress_enable_result,
			attr_set_handpress_enable);
SHOW_DELAY_FUNC(handpress, TAG_HANDPRESS)
STORE_DELAY_FUNC(handpress, TAG_HANDPRESS, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(handpress_setdelay, 0664, show_handpress_delay_result,
			attr_set_handpress_delay);

SHOW_ENABLE_FUNC(cap_prox, TAG_CAP_PROX)
STORE_ENABLE_FUNC(cap_prox, TAG_CAP_PROX, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(cap_prox_enable, 0664, show_cap_prox_enable_result,
			attr_set_cap_prox_enable);
SHOW_DELAY_FUNC(cap_prox, TAG_CAP_PROX)
STORE_DELAY_FUNC(cap_prox, TAG_CAP_PROX, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(cap_prox_setdelay, 0664, show_cap_prox_delay_result,
			attr_set_cap_prox_delay);

SHOW_ENABLE_FUNC(magn_bracket, TAG_MAGN_BRACKET)
STORE_ENABLE_FUNC(magn_bracket, TAG_MAGN_BRACKET, CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ)
static DEVICE_ATTR(magn_bracket_enable, 0664, show_magn_bracket_enable_result,
			attr_set_magn_bracket_enable);
SHOW_DELAY_FUNC(magn_bracket, TAG_MAGN_BRACKET)
STORE_DELAY_FUNC(magn_bracket, TAG_MAGN_BRACKET, CMD_CMN_INTERVAL_REQ)
static DEVICE_ATTR(magn_bracket_setdelay, 0664, show_magn_bracket_delay_result,
			attr_set_magn_bracket_delay);

#ifdef CONFIG_IOM3_RECOVERY
static ssize_t start_iom3_recovery(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	hwlog_info("%s +\n", __func__);
	iom3_need_recovery(SENSORHUB_USER_MODID, SH_FAULT_USER_DUMP);
	hwlog_info("%s -\n", __func__);
	return size;
}

static DEVICE_ATTR(iom3_recovery, 0664, NULL, start_iom3_recovery);
#endif
/*fordden sensor cmd from HAL*/
int flag_for_sensor_test;
static ssize_t attr_set_sensor_test_mode(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t size)
{
	unsigned long val = 0;
	hwlog_info("%s +\n", __func__);
	if (strict_strtoul(buf, 10, &val)) {
		hwlog_err("In %s! val = %lu\n", __func__, val);
		return -EINVAL;
	}
	if (1 == val)
		flag_for_sensor_test = 1;
	else
		flag_for_sensor_test = 0;
	return size;
}

static DEVICE_ATTR(sensor_test, 0660, NULL, attr_set_sensor_test_mode);

/*buf: motion value, 2byte,
   motion type, 0-11
   second status 0-4
*/

#define MOTION_DT_STUP_LENGTH (5)
#define RADIX_16 (16)
static ssize_t attr_set_dt_motion_stup(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t size)
{
	char dt_motion_value[MOTION_DT_STUP_LENGTH] = { };
	int i = 0;
	unsigned long source = 0;

	source = simple_strtoul(buf, NULL, RADIX_16);
	hwlog_err("%s buf %s, source %lu, size %lu\n", __func__, buf, source, size);

	for (; i < MOTION_DT_STUP_LENGTH - 1; i++) {
		dt_motion_value[i] = source % ((i + 1) * RADIX_16);
		source = source / RADIX_16;
	}

	dt_motion_value[MOTION_DT_STUP_LENGTH - 1] = '\0';
	hwlog_err("%s motion %x %x %x %x\n", __func__, dt_motion_value[0],
		  dt_motion_value[1], dt_motion_value[2], dt_motion_value[3]);
	inputhub_route_write(ROUTE_MOTION_PORT, dt_motion_value, MOTION_DT_STUP_LENGTH - 1);

	return size;
}

static DEVICE_ATTR(dt_motion_stup, 0664, NULL, attr_set_dt_motion_stup);

int stop_auto_accel;
static ssize_t attr_set_stop_auto_data(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t size)
{
	stop_auto_accel = simple_strtoul(buf, NULL, 16);
	hwlog_err("%s stop_auto_accel %d\n", __func__, stop_auto_accel);
	return size;
}

static DEVICE_ATTR(dt_stop_auto_data, 0664, NULL, attr_set_stop_auto_data);

extern void disable_motions_when_sysreboot(void);
extern void enable_motions_when_recovery_iom3(void);
int stop_auto_motion = 0;
extern int inputhub_mcu_write_cmd_adapter(const void *buf, unsigned int length,
				   struct read_info *rd);
int tell_cts_test_to_mcu(int status)
{
	read_info_t pkg_mcu;
	int ret = 0;
	if (status == 1) {
		pkt_header_t pkth;
		pkth.tag = TAG_AR;
		pkth.cmd = CMD_CMN_CLOSE_REQ;
		pkth.resp = 0x10;
		pkth.length = 0;
		ret = inputhub_mcu_write_cmd_adapter(&pkth, sizeof(pkth), NULL);
		hwlog_info("close ar return %d in %s\n", ret,  __func__);
	}
	if ((0 == status) || (status == 1)) {
		pkt_sys_statuschange_req_t pkt;
		pkt.hd.tag = TAG_SYS;
		pkt.hd.cmd = CMD_SYS_CTS_RESTRICT_MODE_REQ;
		pkt.hd.resp = RESP;
		pkt.hd.length = sizeof(pkt) - sizeof(pkt.hd);
		pkt.status = status;

		ret = inputhub_mcu_write_cmd_adapter(&pkt, sizeof(pkt), &pkg_mcu);
	} else {
		hwlog_err("error status %d in %s\n", status, __func__);
		return -EINVAL;
	}
	return ret;
}
ssize_t attr_stop_auto_motion_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", stop_auto_motion);
}
static ssize_t attr_set_stop_auto_motion(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	unsigned long val = 0;
	val = simple_strtoul(buf, NULL, 16);
	if(1 == val) {//cts test,disable motion
		disable_motions_when_sysreboot();
		stop_auto_motion = 1;
		hwlog_err("%s stop_auto_motion =%d, val = %lu\n",
					__func__, stop_auto_motion,val);
		tell_cts_test_to_mcu(1);
	}

	if(0 == val) {
		stop_auto_motion = 0;
		enable_motions_when_recovery_iom3();
		hwlog_err("%s stop_auto_motion =%d, val = %lu\n",
					__func__, stop_auto_motion,val);
		tell_cts_test_to_mcu(0);
	}

	return size;
}

static DEVICE_ATTR(dt_stop_auto_motion, 0660, attr_stop_auto_motion_show,
					attr_set_stop_auto_motion);

int stop_auto_als;
static ssize_t attr_set_stop_als_auto_data(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	stop_auto_als = simple_strtoul(buf, NULL, 16);
	hwlog_err("%s stop_auto_als %d\n", __func__, stop_auto_als);
	return size;
}

static DEVICE_ATTR(dt_stop_als_auto_data, 0664, NULL,
		   attr_set_stop_als_auto_data);

int stop_auto_ps;
static ssize_t attr_set_stop_ps_auto_data(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t size)
{
	stop_auto_ps = simple_strtoul(buf, NULL, 16);
	hwlog_err("%s stop_auto_ps %d\n", __func__, stop_auto_ps);
	return size;
}

static DEVICE_ATTR(dt_stop_ps_auto_data, 0664, NULL,
		   attr_set_stop_ps_auto_data);

static ssize_t attr_set_sensor_motion_stup(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	int i = 0;
	unsigned long source = 0;
	struct sensor_data event;
	source = simple_strtoul(buf, NULL, 16);

	if (source) {		/*1: landscape */
		hwlog_err("%s landscape\n", __func__);
		event.type = TAG_ACCEL;
		event.length = 12;
		event.value[0] = 1000;
		event.value[1] = 0;
		event.value[2] = 0;
	} else {		/*0: portial */
		hwlog_err("%s portial\n", __func__);
		event.type = TAG_ACCEL;
		event.length = 12;
		event.value[0] = 0;
		event.value[1] = 1000;
		event.value[2] = 0;
	}

	for (i = 0; i < 20; i++) {
		msleep(100);
		report_sensor_event(TAG_ACCEL, event.value, event.length);
	}
	return size;
}

static DEVICE_ATTR(dt_sensor_stup, 0664, NULL, attr_set_sensor_motion_stup);

static ssize_t attr_set_sensor_stepcounter_stup(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	unsigned long source = 0;
	struct sensor_data event;
	source = simple_strtoul(buf, NULL, 10);

	event.type = TAG_STEP_COUNTER;
	event.length = 12;
	event.value[0] = source;
	event.value[1] = 0;
	event.value[2] = 0;

	report_sensor_event(TAG_STEP_COUNTER, event.value, event.length);
	return size;
}

static DEVICE_ATTR(dt_stepcounter_stup, 0664, NULL,
		   attr_set_sensor_stepcounter_stup);

static ssize_t attr_set_dt_hall_sensor_stup(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t size)
{
	struct sensor_data event;
	unsigned long source = 0;
	source = simple_strtoul(buf, NULL, 16);
	hwlog_err("%s buf %s, source %lu\n", __func__, buf, source);

	event.type = TAG_HALL;
	event.length = 4;
	event.value[0] = (int)source;

	inputhub_route_write(ROUTE_SHB_PORT, (char *)&event, 8);
	return size;
}

static DEVICE_ATTR(dt_hall_sensor_stup, 0664, NULL,
		   attr_set_dt_hall_sensor_stup);

static ssize_t attr_set_dt_als_sensor_stup(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	struct sensor_data event;
	unsigned long source = 0;
	source = simple_strtoul(buf, NULL, 16);
	hwlog_err("%s buf %s, source %lu\n", __func__, buf, source);

	event.type = TAG_ALS;
	event.length = 4;
	event.value[0] = (int)source;

	inputhub_route_write(ROUTE_SHB_PORT, (char *)&event, 8);
	return size;
}

static DEVICE_ATTR(dt_als_sensor_stup, 0664, NULL, attr_set_dt_als_sensor_stup);

static ssize_t attr_set_dt_ps_sensor_stup(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t size)
{
	struct sensor_data event;
	unsigned long source = 0;
	source = simple_strtoul(buf, NULL, 16);
	hwlog_err("%s buf %s, source %lu\n", __func__, buf, source);

	event.type = TAG_PS;
	event.length = 4;
	event.value[0] = (int)source;

	inputhub_route_write(ROUTE_SHB_PORT, (char *)&event, 8);
	return size;
}

static DEVICE_ATTR(dt_ps_sensor_stup, 0664, NULL, attr_set_dt_ps_sensor_stup);

static ssize_t show_iom3_sr_status(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%s\n",
			(iom3_sr_status ==
			 ST_SLEEP) ? "ST_SLEEP" : "ST_WAKEUP");
}

static DEVICE_ATTR(iom3_sr_status, 0664, show_iom3_sr_status, NULL);

ssize_t show_mag_calibrate_method(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", mag_data.calibrate_method);
}
ssize_t show_cap_prox_calibrate_method(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sar_pdata.calibrate_type);
}
ssize_t show_cap_prox_calibrate_orders(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%s\n", sar_calibrate_order);
}
static int airpress_cali_flag;
ssize_t show_sensor_read_airpress_common(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	airpress_cali_flag = 1;
	return snprintf(buf, MAX_STR_SIZE, "%d\n", get_airpress_data);
}

static ssize_t show_sensor_read_airpress(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	return show_sensor_read_airpress_common(dev, attr, buf);
}

static DEVICE_ATTR(read_airpress, 0664, show_sensor_read_airpress, NULL);

static ssize_t show_sensor_read_temperature(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", get_temperature_data);
}

static DEVICE_ATTR(read_temperature, 0664, show_sensor_read_temperature, NULL);

static ssize_t show_dump_sensor_status(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	int tag = 0;
	hwlog_info("-------------------------------------\n");
	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		if (unlikely((tag == TAG_PS) || (tag == TAG_STEP_COUNTER) || (tag == TAG_MAGN_BRACKET))) {	/*ps and step counter need always on, just skip */
			continue;
		}
		hwlog_info(" %s\t %s\t %d\n", obj_tag_str[tag],
			   sensor_status.opened[tag] ? "open" : "close",
			   sensor_status.delay[tag]);
	}
	hwlog_info("-------------------------------------\n");
	return snprintf(buf, MAX_STR_SIZE, "please check log %d\n",
			get_temperature_data);
}

static DEVICE_ATTR(dump_sensor_status, 0664, show_dump_sensor_status, NULL);

static ssize_t show_airpress_set_calidata(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	if (strlen(sensor_chip_info[AIRPRESS]) != 0)
		return snprintf(buf, MAX_STR_SIZE, "%d\n",
				airpress_data.offset);
	else
		return -1;
}

static ssize_t store_airpress_set_calidata(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	long source = 0;
	int ret = 0;
	int i;
	struct hisi_nve_info_user user_info;
	int temp;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	char content[CLI_CONTENT_LEN_MAX] = { 0 };
	char date_str[CLI_TIME_STR_LEN] = { 0 };

	if (strlen(sensor_chip_info[AIRPRESS]) == 0) {
		hwlog_err("AIRPRESS not exits !!\n");
		return -1;
	}
	memset(&user_info, 0, sizeof(user_info));
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	if (!airpress_cali_flag) {
		hwlog_warn("Takes effect only when the calibration data\n");
		return -1;
	}
	source = simple_strtol(buf, NULL, 10);
	airpress_data.offset += (int)source;

	for (i = 0; i < 48; i++)
		airpress_data.airpress_extend_data[i] = i;
	/*send to mcu*/
	pkg_ap.tag = TAG_PRESSURE;
	pkg_ap.cmd = CMD_AIRPRESS_SET_CALIDATA_REQ;
	pkg_ap.wr_buf = (const void *)&airpress_data;
	pkg_ap.wr_len = sizeof(airpress_data);
	hwlog_info("***%s***\n", __func__);
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	if (ret != 0) {
		hwlog_err("set airpress_sensor data failed, ret = %d!\n", ret);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "set airpress_sensor data failed\n");
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("set airpress_sensor sysfs offset fail,err=%d\n",
			  pkg_mcu.errno);
		__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
			       "set airpress_sensor offset fail\n");
	} else {
		hwlog_info
		    ("send airpress_sensor sysfs data :%d to mcu success\n",
		     airpress_data.offset);
	}

	get_test_time(date_str, sizeof(date_str));

	memset(&content, 0, sizeof(content));
	snprintf(content, CLI_CONTENT_LEN_MAX, PRESS_CALI_OFFSET,
		 airpress_data.offset, "pass", "SUC", date_str);
	save_to_file(DATA_CLLCT, content);

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = AIRPRESS_CALIDATA_NV_NUM;
	user_info.valid_size = AIRPRESS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "AIRDATA", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	memcpy(user_info.nv_data, &airpress_data.offset,
	       sizeof(airpress_data.offset));
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access write error(%d)\n", ret);
		return -1;
	}
	msleep(10);
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = AIRPRESS_CALIDATA_NV_NUM;
	user_info.valid_size = AIRPRESS_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "AIRDATA", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve direct access read error(%d)\n", ret);
		return -1;
	}
	memcpy(&temp, user_info.nv_data, sizeof(temp));
	if (temp != airpress_data.offset) {
		hwlog_err("nv write fail, (%d %d)\n", temp,
			  airpress_data.offset);
		return -1;
	}
	airpress_cali_flag = 0;
	return size;
}

static DEVICE_ATTR(airpress_set_calidata, 0664, show_airpress_set_calidata,
		   store_airpress_set_calidata);

static ssize_t attr_handpress_calibrate_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int result = (handpress_calibration_res == SUC) ? 0 : handpress_calibration_res;

	return snprintf(buf, PAGE_SIZE, "%d\n", result);
}

static struct work_struct handpress_calibrate_work;
static void handpress_calibrate_work_func(struct work_struct *work)
{
	int ret = 0;

	hwlog_err("handpress calibrate work enter ++\n");
	ret = write_handpress_offset_to_nv(hp_offset);
	if (ret < 0)  {
		hwlog_err("nv write faild.\n");
		handpress_calibration_res = NV_FAIL;
	}
	hwlog_err("handpress calibrate work enter --\n");
}
static ssize_t attr_handpress_calibrate_write(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	int data_len = 0;
	char content[CLI_CONTENT_LEN_MAX] = { 0 };
	char date_str[CLI_TIME_STR_LEN] = { 0 };
	uint16_t hand_offset[8] = { 0 };
	uint16_t hand_metal[8] = { 0 };
	uint8_t hand_idac1[8] = { 0 };
	uint8_t hand_calidata[24] = { 0 };

	handpress_calibration_res = EXEC_FAIL;

	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	if (strlen(sensor_chip_info[HANDPRESS]) == 0) {
		hwlog_err("no handpress\n");
		return count;
	}

	memset(&content, 0, sizeof(content));

	if (val == 1) {
		if ((hall_value & 0x01) != 0x01) {
			hwlog_err("hall value:%d\n", hall_value);
			handpress_calibration_res = POSITION_FAIL;
			return count;
		}
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_HANDPRESS;
	pkg_ap.cmd = CMD_HANDPRESS_SELFCALI_REQ;
	pkg_ap.wr_buf = &val;
	pkg_ap.wr_len = sizeof(val);
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	if(ret) {
		handpress_calibration_res = COMMU_FAIL;
		hwlog_err("send handpress calibrate cmd to mcu fail,ret=%d\n", ret);
		goto hp_cali_out;
	}

	if(pkg_mcu.errno != 0) {
		hwlog_err("handpress calibrate fail, %d\n", pkg_mcu.errno);
		handpress_calibration_res = EXEC_FAIL;
		goto hp_cali_out;
	}

	hwlog_err("data_len:%d\n", pkg_mcu.data_length);
	data_len = sizeof(hand_calidata);
	data_len = pkg_mcu.data_length>data_len ? data_len : pkg_mcu.data_length;
	memcpy(hand_calidata, pkg_mcu.data, data_len);
	memcpy(hp_offset, hand_calidata, sizeof(hp_offset));
	handpress_calibration_res = SUC;
	if (val == 1) {
		INIT_WORK(&handpress_calibrate_work, handpress_calibrate_work_func);
		queue_work(system_power_efficient_wq, &handpress_calibrate_work);
		msleep(50);
		memcpy(hand_offset, &hp_offset[8], sizeof(hand_offset));
		memcpy(hand_idac1, hp_offset, sizeof(hand_idac1));
		get_test_time(date_str, sizeof(date_str));
		snprintf(content, CLI_CONTENT_LEN_MAX,
			"info 1:\ntime:%s\noffset:%d %d %d %d %d %d %d %d\nidac2:%d %d %d %d %d %d %d %d\n",
			date_str,
			hand_offset[0], hand_offset[1], hand_offset[2], hand_offset[3],
			hand_offset[4], hand_offset[5], hand_offset[6], hand_offset[7],
			hand_idac1[0], hand_idac1[1], hand_idac1[2], hand_idac1[3],
			hand_idac1[4], hand_idac1[5], hand_idac1[6], hand_idac1[7]);
			hwlog_info("time:%s\noffset:%d %d %d %d %d %d %d %d\nidac2:%d %d %d %d %d %d %d %d\n",
			date_str,
			hand_offset[0], hand_offset[1], hand_offset[2], hand_offset[3],
			hand_offset[4], hand_offset[5], hand_offset[6], hand_offset[7],
			hand_idac1[0], hand_idac1[1], hand_idac1[2], hand_idac1[3],
			hand_idac1[4], hand_idac1[5], hand_idac1[6], hand_idac1[7]);
	} else if (val == 2) {
		data_len = sizeof(hand_metal);
		memcpy(hand_metal, hand_calidata, data_len);
		snprintf(content, CLI_CONTENT_LEN_MAX, "info 2:\nmetal:%d %d %d %d %d %d %d %d\n",
			hand_metal[0], hand_metal[1], hand_metal[2], hand_metal[3],
			hand_metal[4], hand_metal[5], hand_metal[6], hand_metal[7]);
		hwlog_info("metal:%d %d %d %d %d %d %d %d\n",
			hand_metal[0], hand_metal[1], hand_metal[2], hand_metal[3],
			hand_metal[4], hand_metal[5], hand_metal[6], hand_metal[7]);
	} else {
		hwlog_err("invalid input:%d\n", (int)val);
		snprintf(content, CLI_CONTENT_LEN_MAX, "invalid input:%d\n", (int)val);
	}

	save_to_file(HAND_DATA_CLLCT, content);

hp_cali_out:
	return count;
}
static DEVICE_ATTR(handpress_calibrate, 0664,
		   attr_handpress_calibrate_show,
		   attr_handpress_calibrate_write);

ssize_t sensors_calibrate_show(int tag, struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	switch (tag) {
	case TAG_ACCEL:
		return snprintf(buf, PAGE_SIZE, "%d\n", return_calibration != SUC);	/*flyhorse k: SUC-->"0", OTHERS-->"1"*/

	case TAG_PS:
		return snprintf(buf, PAGE_SIZE, "%d\n", ps_calibration_res != SUC);	/*flyhorse k: SUC-->"0", OTHERS-->"1"*/

	case TAG_ALS:
		return snprintf(buf, PAGE_SIZE, "%d\n", als_calibration_res != SUC); /*flyhorse k: SUC-->"0", OTHERS-->"1"*/

	case TAG_GYRO:
		return snprintf(buf, PAGE_SIZE, "%d\n", gyro_calibration_res != SUC); /*flyhorse k: SUC-->"0", OTHERS-->"1"*/

	case TAG_PRESSURE:
		return show_airpress_set_calidata(dev, attr, buf);

	case TAG_HANDPRESS:
		return snprintf(buf, PAGE_SIZE, "%d\n", handpress_calibration_res != SUC);

    case TAG_CAP_PROX:
		return snprintf(buf, PAGE_SIZE, "%d\n", return_cap_prox_calibration != SUC);
	default:
		hwlog_err("tag %d calibrate not implement in %s\n", tag,
			  __func__);
		break;
	}

	return 0;
}

ssize_t sensors_calibrate_store(int tag, struct device *dev,
				struct device_attribute *attr, const char *buf,
				size_t count)
{
	switch (tag) {
	case TAG_ACCEL:
		return attr_acc_calibrate_write(dev, attr, buf, count);

	case TAG_PS:
		return attr_ps_calibrate_write(dev, attr, buf, count);

	case TAG_ALS:
		return attr_als_calibrate_write(dev, attr, buf, count);

	case TAG_GYRO:
		return attr_gyro_calibrate_write(dev, attr, buf, count);

	case TAG_PRESSURE:
		return store_airpress_set_calidata(dev, attr, buf, count);

	case TAG_HANDPRESS:
		return attr_handpress_calibrate_write(dev, attr, buf, count);
     case TAG_CAP_PROX:
              return attr_cap_prox_calibrate_write(dev, attr, buf, count);
	default:
		hwlog_err("tag %d calibrate not implement in %s\n", tag,
			  __func__);
		break;
	}

	return count;
}

int ois_commu(int tag, unsigned int cmd, unsigned int pare,
		      unsigned int responsed)
{
	int ret = -1;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = tag;
	pkg_ap.cmd = cmd;
	pkg_ap.wr_buf = &pare;
	pkg_ap.wr_len = sizeof(pare);

	if (responsed == NO_RESP){
		ret = write_customize_cmd(&pkg_ap, NULL);
		if (ret) {
			hwlog_err("send ois cmd(%d) to mcu fail,ret=%d\n",
				cmd, ret);
			return ret;
		}
	} else {
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
		if (ret) {
			hwlog_err("send ois gyro cfg cmd failed, ret = %d!\n", ret);
			__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
				"send ois gyro cfg cmd failed\n");
			return ret;
		}
		if (pkg_mcu.errno != 0) {
			hwlog_err("set ois gyro cfg cmd fail,err=%d\n",
				pkg_mcu.errno);
			__dmd_log_report(DSM_SHB_ERR_IOM7_CFG_DATA, __func__,
				"set ois gyro cfg cmd fail\n");
		} else {
			hwlog_info("set ois gyro cfg cmd %d success\n", pare);
			sensor_status.gyro_ois_status = pare;
		}
	}

	return ret;
}

static ssize_t show_ois_ctrl(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n",
				sensor_status.gyro_ois_status);
}

static ssize_t store_ois_ctrl(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	int source = 0, ret = 0;
	unsigned int cmd = 0;
	unsigned int delay = 10;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	source = simple_strtol(buf, NULL, 10);

	if (1 == source) {
		cmd = CMD_CMN_OPEN_REQ;
		ret = ois_commu(TAG_OIS, cmd, source, NO_RESP);
		if (ret) {
			hwlog_err("%s: ois open gyro fail\n",
				  __FUNCTION__);
			return size;
		}

		cmd = CMD_CMN_INTERVAL_REQ;
		ret = ois_commu(TAG_OIS, cmd, delay, NO_RESP);
		if (ret) {
			hwlog_err("%s: set delay fail\n", __FUNCTION__);
			return size;
		}

		cmd = CMD_GYRO_OIS_REQ;
		ret = ois_commu(TAG_GYRO, cmd, source, RESP);
		if (ret) {
			hwlog_err("%s: ois enable fail\n", __FUNCTION__);
			return size;
		}
		hwlog_info("%s:ois enable succsess\n", __FUNCTION__);
	}
        else if(0 == source){
		cmd = CMD_GYRO_OIS_REQ;
		ret = ois_commu(TAG_GYRO, cmd, source, RESP);
		if (ret) {
			hwlog_err("%s:ois close fail\n", __FUNCTION__);
			return size;
		}

		cmd = CMD_CMN_CLOSE_REQ;
		ret = ois_commu(TAG_OIS, cmd, source, NO_RESP);
		if (ret) {
			hwlog_err("%s: ois close gyro fail\n",
				  __FUNCTION__);
			return size;
		}
		hwlog_info("%s:ois close succsess\n", __FUNCTION__);
	}
	else if(2 == source){
		cmd = CMD_GYRO_OIS_REQ;
		ret = ois_commu(TAG_GYRO, cmd, source, RESP);
		if (ret) {
			hwlog_err("%s: ois enable fail\n", __FUNCTION__);
			return size;
		}
		hwlog_info("%s:ois reset succsess\n", __FUNCTION__);
	}
	else if (3 == source) {
		source = 1;
		cmd = CMD_CMN_OPEN_REQ;
		ret = ois_commu(TAG_OIS, cmd, source, NO_RESP);
		if (ret) {
			hwlog_err("%s: ois open gyro fail\n",
				  __FUNCTION__);
			return size;
		}

		cmd = CMD_CMN_INTERVAL_REQ;
		ret = ois_commu(TAG_OIS, cmd, delay, NO_RESP);
		if (ret) {
			hwlog_err("%s: set delay fail\n", __FUNCTION__);
			return size;
		}
		cmd = CMD_GYRO_OIS_REQ;
		ret = ois_commu(TAG_GYRO, cmd, source, NO_RESP);
		if (ret) {
			hwlog_err("%s: ois enable no_resp fail\n", __FUNCTION__);
			return size;
		}
		hwlog_info("%s:ois enable succsess\n", __FUNCTION__);
	}
	else{
		hwlog_info("%s:ois commend is not right\n", __FUNCTION__);
	}

	return size;
}

static DEVICE_ATTR(ois_ctrl, 0664, show_ois_ctrl,
		   store_ois_ctrl);

void key_fb_notifier_action(int enable)
{
	int ret = 0;
	int interval = 20;
	write_info_t pkg_ap;

	hwlog_info("key_fb_notifier_action fb :%d\n", enable);
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	if (strlen(sensor_chip_info[KEY]) == 0) {
		hwlog_info("no key.\n");
		return;
	}

	if (enable) {
		pkg_ap.cmd = CMD_CMN_OPEN_REQ;
		pkg_ap.tag = TAG_KEY;
		pkg_ap.wr_buf = NULL;
		pkg_ap.wr_len = 0;
		ret = write_customize_cmd(&pkg_ap, NULL);
		if (ret < 0) {
			hwlog_err("write open err.\n");
			return;
		}
		//
		pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
		pkg_ap.tag = TAG_KEY;
		pkg_ap.wr_buf = &interval;
		pkg_ap.wr_len = sizeof(interval);
		ret = write_customize_cmd(&pkg_ap, NULL);
		if (ret)
			hwlog_err("write interval err.\n");
		else
			key_state = 1;
	} else {
		pkg_ap.cmd = CMD_CMN_CLOSE_REQ;
		pkg_ap.tag = TAG_KEY;
		pkg_ap.wr_buf = NULL;
		pkg_ap.wr_len = 0;
		ret = write_customize_cmd(&pkg_ap, NULL);
		if (ret)
			hwlog_err("write close err.\n");
		else
			key_state = 0;
	}
}

static ssize_t show_key_debug(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n",
				sensor_status.gyro_ois_status);
}

static ssize_t store_key_debug(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	int source = 0, ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	int value;
	int offset_value;
	int in_time = 500;

	if (strlen(sensor_chip_info[KEY]) == 0) {
		hwlog_err("no key\n");
		return size;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	source = simple_strtol(buf, NULL, 10);
	value = ((source>>8) & 0xff);
	offset_value = (source & 0xff);
	hwlog_info("value:%d  offset:%d\n", value, offset_value);
	switch(value) {
		case 0:
			pkg_ap.cmd = CMD_CMN_CLOSE_REQ;
			pkg_ap.tag = TAG_KEY;
			pkg_ap.wr_buf = NULL;
			pkg_ap.wr_len = 0;
			break;
		case 1:
			pkg_ap.cmd = CMD_CMN_OPEN_REQ;
			pkg_ap.tag = TAG_KEY;
			pkg_ap.wr_buf = NULL;
			pkg_ap.wr_len = 0;
			break;
		case 2:		// set offset
			pkg_ap.cmd = CMD_KEY_SET_CALIDATA_REQ;
			pkg_ap.tag = TAG_KEY;
			pkg_ap.wr_buf = &offset_value;
			pkg_ap.wr_len = sizeof(offset_value);
			break;
		case 3:
			pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
			pkg_ap.tag = TAG_KEY;
			pkg_ap.wr_buf = &in_time;
			pkg_ap.wr_len = sizeof(in_time);
			break;
		case 4:
			pkg_ap.cmd = CMD_KEY_BACKLIGHT_REQ;
			pkg_ap.tag = TAG_KEY;
			pkg_ap.wr_buf = &offset_value;
			pkg_ap.wr_len = sizeof(offset_value);
			break;
		case 5:
			pkg_ap.cmd = CMD_KEY_SET_CALIDATA_REQ;
			pkg_ap.tag = TAG_KEY;
			pkg_ap.wr_buf = &offset_value;
			pkg_ap.wr_len = sizeof(offset_value);
			break;
#if 1
		case 6:	// sar open
			pkg_ap.cmd = CMD_CMN_OPEN_REQ;
			pkg_ap.tag = TAG_CAP_PROX;
			pkg_ap.wr_buf = NULL;
			pkg_ap.wr_len = 0;
			break;
		case 7:// sar setdelay
			pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
			pkg_ap.tag = TAG_CAP_PROX;
			pkg_ap.wr_buf = &in_time;
			pkg_ap.wr_len = sizeof(in_time);
			break;
		case 8:// sar close
			pkg_ap.cmd = CMD_CMN_CLOSE_REQ;
			pkg_ap.tag = TAG_CAP_PROX;
			pkg_ap.wr_buf = NULL;
			pkg_ap.wr_len = 0;
			break;
#endif
		default:
			return -1;
	}

	if (value >= 6 && value <= 8) {
		ret = write_customize_cmd(&pkg_ap, NULL);
		if (ret < 0) {
			hwlog_err("err. write cmd\n");
			return -1;
		}
		return size;
	}
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu);
	if (ret < 0) {
		hwlog_err("err. write cmd\n");
		return -1;
	}

	if (0 != pkg_mcu.errno) {
		hwlog_info("mcu err \n");
		return -1;
	}

	return size;
}
static DEVICE_ATTR(key_debug,0664, show_key_debug, store_key_debug);

#ifdef CONFIG_HW_TOUCH_KEY
int huawei_set_key_backlight(void *param_t)
{
	int ret = 0;
	int key_brightness = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	struct key_param_t *param = (struct key_param_t *)param_t;

	if (NULL == param) {
		hwlog_err("param null\n");
		return 0;
	}

	if (strlen(sensor_chip_info[KEY]) == 0) {
		hwlog_err("no key\n");
		return 0;
	}
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	key_brightness = ((param->test_mode << 16) | param->brightness1 | (param->brightness2<<8));

	pkg_ap.cmd = CMD_KEY_BACKLIGHT_REQ;
	pkg_ap.tag = TAG_KEY;
	pkg_ap.wr_buf = &key_brightness;
	pkg_ap.wr_len = sizeof(key_brightness);
	if (g_iom3_state == IOM3_ST_RECOVERY || iom3_power_state == ST_SLEEP) {
		ret = write_customize_cmd_noresp(TAG_KEY,
                                         CMD_KEY_BACKLIGHT_REQ,
                                         pkg_ap.wr_buf,
                                         pkg_ap.wr_len);
	} else
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu);

	if (ret < 0) {
		hwlog_err("err. write cmd\n");
		return -1;
	}

	if (0 != pkg_mcu.errno) {
		hwlog_info("mcu err \n");
		return -1;
	}

	return 0;

}
#else
int huawei_set_key_backlight(void *param_t)
{
	return 0;
}
#endif

static ssize_t show_sar_data(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,cypress_sar_psoc4000", strlen("huawei,cypress_sar_psoc4000"))) {
		return snprintf(buf, MAX_STR_SIZE, "idac:%d rawdata:%d near:%d far:%d\n",
			sar_calibrate_datas.cypres_cali_data.sar_idac,
			sar_calibrate_datas.cypres_cali_data.raw_data,
			sar_calibrate_datas.cypres_cali_data.near_signaldata,
			sar_calibrate_datas.cypres_cali_data.far_signaldata);
	}
	if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,semtech-sx9323", strlen("huawei,semtech-sx9323"))) {
		return snprintf(buf, MAX_STR_SIZE, "offset:%d diff:%d \n",
			sar_calibrate_datas.semtech_cali_data.offset,
			sar_calibrate_datas.semtech_cali_data.diff);
	}

	return -1;
}
static DEVICE_ATTR(sar_data,0444, show_sar_data, NULL);

static ssize_t show_hifi_supported(struct device *dev,
					  struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", hifi_supported);
}
static DEVICE_ATTR(hifi_supported, 0664, show_hifi_supported, NULL);


static enum detect_state sensor_detect_flag = DET_FAIL;
static ssize_t show_sensor_detect(struct device *dev,
					  struct device_attribute *attr, char *buf)
{
	int i = 0;
	int detect_result = 0;
	char result[(MAX_SENSOR_NAME_LENGTH + 1)*SENSOR_MAX];
	memset(&result, ' ', (MAX_SENSOR_NAME_LENGTH + 1)*SENSOR_MAX);

	for(i = 0; i < SENSOR_MAX; i++)
	{
		detect_result = s_detect_manager[i].detect_result;
		if(detect_result == sensor_detect_flag){
			snprintf(&buf[i*MAX_SENSOR_NAME_LENGTH], MAX_SENSOR_NAME_LENGTH, "%s ", s_detect_manager[i].sensor_name_str);
		}
	}

	return MAX_STR_SIZE;
}

static ssize_t store_sensor_detect(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t size)
{
	int flag = 0;
	flag = simple_strtol(buf, NULL, 10);
	sensor_detect_flag = flag?DET_SUCC:DET_FAIL;
	hwlog_info("sensor detect value %d\n", (int)sensor_detect_flag);

	return size;
}

static DEVICE_ATTR(sensor_detect, 0660, show_sensor_detect, store_sensor_detect);


static struct attribute *sensor_attributes[] = {
	&dev_attr_acc_info.attr,
	&dev_attr_mag_info.attr,
	&dev_attr_gyro_info.attr,
	&dev_attr_ps_info.attr,
	&dev_attr_als_info.attr,
	&dev_attr_acc_read_data.attr,
	&dev_attr_mag_read_data.attr,
	&dev_attr_gyro_read_data.attr,
	&dev_attr_ps_read_data.attr,
	&dev_attr_als_read_data.attr,
	&dev_attr_gyro_selfTest.attr,
	&dev_attr_mag_selfTest.attr,
	&dev_attr_acc_selfTest.attr,
	&dev_attr_gps_4774_i2c_selfTest.attr,
	&dev_attr_i2c_rw.attr,
	&dev_attr_i2c_rw16.attr,
	&dev_attr_acc_calibrate.attr,
	&dev_attr_acc_enable.attr,
	&dev_attr_acc_setdelay.attr,
	&dev_attr_set_fingersense_enable.attr,
	&dev_attr_fingersense_req_data.attr,
	&dev_attr_fingersense_data_ready.attr,
	&dev_attr_fingersense_latch_data.attr,
	&dev_attr_gsensor_gather_enable.attr,
	&dev_attr_gyro_calibrate.attr,
	&dev_attr_gyro_enable.attr,
	&dev_attr_gyro_setdelay.attr,
	&dev_attr_mag_enable.attr,
	&dev_attr_mag_setdelay.attr,
	&dev_attr_als_calibrate.attr,
	&dev_attr_als_enable.attr,
	&dev_attr_als_setdelay.attr,
	&dev_attr_sbl_enable.attr,
	&dev_attr_sbl_setdelay.attr,
	&dev_attr_sbl_setbacklight.attr,
	&dev_attr_ps_calibrate.attr,
	&dev_attr_ps_enable.attr,
	&dev_attr_ps_setdelay.attr,
	&dev_attr_ps_switch_mode.attr,
	&dev_attr_pdr_enable.attr,
	&dev_attr_pdr_setdelay.attr,
	&dev_attr_orientation_enable.attr,
	&dev_attr_orientation_setdelay.attr,
	&dev_attr_lines_enable.attr,
	&dev_attr_lines_setdelay.attr,
	&dev_attr_gras_enable.attr,
	&dev_attr_gras_setdelay.attr,
	&dev_attr_rvs_enable.attr,
	&dev_attr_rvs_setdelay.attr,
	&dev_attr_sensor_list_info.attr,
#ifdef CONFIG_IOM3_RECOVERY
	&dev_attr_iom3_recovery.attr,
#endif
	&dev_attr_sensor_test.attr,
	&dev_attr_iom3_sr_test.attr,
	&dev_attr_dt_motion_stup.attr,
	&dev_attr_dt_sensor_stup.attr,
	&dev_attr_dt_stop_auto_data.attr,
	&dev_attr_dt_hall_sensor_stup.attr,
	&dev_attr_dt_stop_als_auto_data.attr,
	&dev_attr_dt_als_sensor_stup.attr,
	&dev_attr_dt_stop_ps_auto_data.attr,
	&dev_attr_dt_ps_sensor_stup.attr,
	&dev_attr_dt_stop_auto_motion.attr,
	&dev_attr_airpress_info.attr,
	&dev_attr_airpress_enable.attr,
	&dev_attr_airpress_setdelay.attr,
	&dev_attr_airpress_read_data.attr,
	&dev_attr_airpress_set_calidata.attr,
	&dev_attr_read_airpress.attr,
	&dev_attr_read_temperature.attr,
	&dev_attr_dt_stepcounter_stup.attr,
	&dev_attr_handpress_calibrate.attr,
	&dev_attr_handpress_selfTest.attr,
	&dev_attr_handpress_info.attr,
	&dev_attr_handpress_enable.attr,
	&dev_attr_handpress_setdelay.attr,
	&dev_attr_handpress_read_data.attr,
	&dev_attr_ois_ctrl.attr,
	&dev_attr_iom3_sr_status.attr,
	&dev_attr_dump_sensor_status.attr,
	&dev_attr_cap_prox_calibrate.attr,
	&dev_attr_cap_prox_freespace.attr,
	&dev_attr_cap_prox_enable.attr,
	&dev_attr_cap_prox_setdelay.attr,
	&dev_attr_key_debug.attr,
	&dev_attr_sar_data.attr,
	&dev_attr_charger_info.attr,
	&dev_attr_switch_info.attr,
	&dev_attr_magn_bracket_enable.attr,
	&dev_attr_magn_bracket_setdelay.attr,
	&dev_attr_hifi_supported.attr,
	&dev_attr_sensor_detect.attr,
	NULL
};

static const struct attribute_group sensor_node = {
	.attrs = sensor_attributes,
};

static struct platform_device sensor_input_info = {
	.name = "huawei_sensor",
	.id = -1,
};

int get_lcd_module_by_xml( const char *lcd_model );
int get_lcd_module(void)
{
	int ret = NO_LCDTYPE_FOUND;
	struct device_node *np = NULL;
	const char *lcd_model = NULL;

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_JDI_NT35695_CUT3_1);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_JDI_NT35695_CUT3_1);
		return JDI_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LG_ER69006A);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_LG_ER69006A);
		return LG_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_JDI_NT35695_CUT2_5);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_JDI_NT35695_CUT2_5);
		return JDI_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LG_ER69007);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_LG_ER69007);
		return KNIGHT_BIEL_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SHARP_NT35597);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_SHARP_NT35597);
		return KNIGHT_BIEL_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SHARP_NT35695);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_SHARP_NT35695);
		return KNIGHT_BIEL_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LG_ER69006_FHD);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_LG_ER69006_FHD);
		return KNIGHT_BIEL_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_MIPI_BOE_ER69006);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_MIPI_BOE_ER69006);
		return KNIGHT_LENS_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_BOE_OTM1906C);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_BOE_OTM1906C);
		return BOE_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_INX_OTM1906C);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_INX_OTM1906C);
		return INX_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_EBBG_OTM1906C);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_EBBG_OTM1906C);
		return EBBG_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_JDI_NT35695);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_JDI_NT35695);
		return JDI_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LG_R69006);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_LG_R69006);
		return LG_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SAMSUNG_S6E3HA3X02);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_SAMSUNG_S6E3HA3X02);
		return SAMSUNG_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LG_R69006_5P2);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_LG_R69006_5P2);
		return LG_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SHARP_NT35695_5P2);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_SHARP_NT35695_5P2);
		return SHARP_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_JDI_R63452_5P2);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_JDI_R63452_5P2);
		return JDI_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SAM_WQ_5P5);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_SAM_WQ_5P5);
		return BIEL_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SAM_FHD_5P5);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_SAM_FHD_5P5);
		return VITAL_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_JDI_R63450_5P7);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_JDI_R63450_5P7);
		return JDI_TPLCD;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SHARP_DUKE_NT35597);
	ret = of_device_is_available(np);
	if (np && ret) {
		hwlog_info("%s is present\n", DTS_COMP_SHARP_DUKE_NT35597);
		return SHARP_TPLCD;
	}

	np = of_find_compatible_node(NULL,NULL,"huawei,lcd_panel_type");
	if(!np){
		hwlog_err("not find lcd_panel_type node\n");
		return ret;
	}
	if(of_property_read_string(np,"lcd_panel_type",&lcd_model)){
		hwlog_err("not find lcd_model in dts\n");
		return ret;
	}

	hwlog_info("find lcd_panel_type suc in dts!!lcd_panel_type = %s\n",lcd_model);

	ret = NO_LCDTYPE_FOUND;
	ret = get_lcd_module_by_xml( lcd_model );
	if( NO_LCDTYPE_FOUND != ret ){
		return ret;
	}

	hwlog_err("sensor kernel failed to get lcd module\n");
	return ret;
}

int get_lcd_module_by_xml( const char *lcd_model )// to solve smartIDE NSIQ problem, grab all ifs of judging by xml file here.
{
	if(!strncmp(lcd_model, DTS_COMP_AUO_OTM1901A_5P2, strlen(DTS_COMP_AUO_OTM1901A_5P2))){
		hwlog_info("%s is present\n", DTS_COMP_AUO_OTM1901A_5P2);
		return AUO_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_AUO_TD4310_5P2, strlen(DTS_COMP_AUO_TD4310_5P2))){
		hwlog_info("%s is present\n", DTS_COMP_AUO_TD4310_5P2);
		return AUO_TPLCD ;
	}

	if(!strncmp(lcd_model, DTS_COMP_TM_FT8716_5P2, strlen(DTS_COMP_TM_FT8716_5P2))){
		hwlog_info("%s is present\n", DTS_COMP_TM_FT8716_5P2);
		return TM_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_EBBG_NT35596S_5P2, strlen(DTS_COMP_EBBG_NT35596S_5P2))){
		hwlog_info("%s is present\n", DTS_COMP_EBBG_NT35596S_5P2);
		return EBBG_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_JDI_ILI7807E_5P2, strlen(DTS_COMP_JDI_ILI7807E_5P2))){
		hwlog_info("%s is present\n", DTS_COMP_JDI_ILI7807E_5P2);
		return JDI_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_BOE_TD4300_5P0, strlen(DTS_COMP_BOE_TD4300_5P0))){
		hwlog_info("%s is present\n", DTS_COMP_BOE_TD4300_5P0);
		return BOE_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_CTC_TD4310_5P0, strlen(DTS_COMP_CTC_TD4310_5P0))){
		hwlog_info("%s is present\n", DTS_COMP_CTC_TD4310_5P0);
		return CTC_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_LG_TD4300_5P0, strlen(DTS_COMP_LG_TD4300_5P0))){
		hwlog_info("%s is present\n", DTS_COMP_LG_TD4300_5P0);
		return LG_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_TIANMA_TD4300_5P0, strlen(DTS_COMP_TIANMA_TD4300_5P0))){
		hwlog_info("%s is present\n", DTS_COMP_TIANMA_TD4300_5P0);
		return TM_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_AUO_NT36672_5P5, strlen(DTS_COMP_AUO_NT36672_5P5))){
		hwlog_info("%s is present\n", DTS_COMP_AUO_NT36672_5P5);
		return AUO_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_BOE_TD4310_5P5, strlen(DTS_COMP_BOE_TD4310_5P5))){
		hwlog_info("%s is present\n", DTS_COMP_BOE_TD4310_5P5);
		return BOE_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_CTC_FT8716_5P5, strlen(DTS_COMP_CTC_FT8716_5P5))){
		hwlog_info("%s is present\n", DTS_COMP_CTC_FT8716_5P5);
		return CTC_TPLCD;
	}

	if(!strncmp(lcd_model, DTS_COMP_TIANMA_TD4310_5P5, strlen(DTS_COMP_TIANMA_TD4310_5P5))){
		hwlog_info("%s is present\n", DTS_COMP_TIANMA_TD4310_5P5);
		return TM_TPLCD;
	}

	return NO_LCDTYPE_FOUND;
}

extern atomic_t iom3_rec_state;
int mcu_sys_ready_callback(const pkt_header_t *head)
{
	int ret = 0, result = 0;
	unsigned int time_of_vddio_power_reset = 0;
	unsigned long new_sensor_jiffies = 0;

#ifdef CONFIG_HUAWEI_CHARGER_SENSORHUB
	enum hisi_charger_type type = hisi_get_charger_type();
#endif

	if (ST_MINSYSREADY == ((pkt_sys_statuschange_req_t *) head)->status) {
		hwlog_info("sys ready mini!\n");
		tplcd_manufacture = get_lcd_module();
		hwlog_err("sensor kernel get_lcd_module tplcd_manufacture=%d\n",
			  tplcd_manufacture);
		result = check_sensor_1V8_from_pmic();
		if(!result)
			hwlog_err("check sensor_1V8 from pmic fail\n");
		if(need_reset_io_power) {
			 new_sensor_jiffies = jiffies- sensor_jiffies;
			 time_of_vddio_power_reset = jiffies_to_msecs(new_sensor_jiffies);
			if (time_of_vddio_power_reset < SENSOR_MAX_RESET_TIME_MS)
				msleep(SENSOR_MAX_RESET_TIME_MS - time_of_vddio_power_reset);

			hwlog_info("time_of_vddio_power_reset %u\n", time_of_vddio_power_reset);
			ret = regulator_enable(sensorhub_vddio);
			if (ret < 0)
				hwlog_err("sensor vddio enable 2.85V\n");

			msleep(SENSOR_DETECT_AFTER_POWERON_TIME_MS);
		}
		ret = init_sensors_cfg_data_from_dts(BOOT_DETECT);
		if (ret)
			hwlog_err
			    ("get sensors cfg data from dts fail,ret=%d, use default config data!\n",
			     ret);
		else
			hwlog_info("get sensors cfg data from dts success!\n");
	} else if (ST_MCUREADY == ((pkt_sys_statuschange_req_t *) head)->status) {
		hwlog_info("mcu all ready!\n");
		ret = sensor_set_cfg_data();
		if (ret < 0)
			hwlog_err("sensor_chip_detect ret=%d\n", ret);
		ret = sensor_set_fw_load();
		if (ret < 0) {
			hwlog_err("sensor fw dload err ret=%d\n", ret);
		}
		unregister_mcu_event_notifier(TAG_SYS, CMD_SYS_STATUSCHANGE_REQ,
					      mcu_sys_ready_callback);
#ifdef CONFIG_IOM3_RECOVERY
		atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
#endif
#ifdef CONFIG_HUAWEI_CHARGER_SENSORHUB
		if (strlen(sensor_chip_info[SWITCH]) != 0) {
			fsa9685_get_gpio_int();
		}
		if (strlen(sensor_chip_info[CHARGER]) != 0) {
			//if (!charger_type_ever_notify)
		    	charge_usb_notifier_call_sh(&usb_nb, type, NULL);
		}
#endif
	} else {
		hwlog_info("other status\n");
	}
	return 0;
}

int mcu_reboot_callback(const pkt_header_t *head)
{
	hwlog_err("%s\n", __func__);
	complete(&iom3_reboot);
	return 0;
}

static struct of_device_id sensorhub_io_supply_ids[] = {
	{.compatible = "huawei,sensorhub_io"},
	{}
};

MODULE_DEVICE_TABLE(of, sensorhub_io_supply_ids);

void reset_sensor_power(void)
{
    int ret = 0;
    if(!need_reset_io_power)
    {
        hwlog_warn("%s: no need to reset sensor power\n", __func__);
        return;
    }

	if(IS_ERR(sensorhub_vddio)){
           hwlog_err("%s: regulator_get fail!\n", __func__);
	    return;
	}
	ret = regulator_disable(sensorhub_vddio);
	if (ret< 0) {
		hwlog_err("failed to disable regulator sensorhub_vddio\n");
		return;
       }
        msleep(10);
        ret = regulator_enable(sensorhub_vddio);
        msleep(5);
	if (ret< 0) {
		hwlog_err("failed to enable regulator sensorhub_vddio\n");
		return;
       }
       hwlog_info("%s done\n", __FUNCTION__);
}

int g_sensorhub_wdt_irq = -1;
static int sensorhub_io_driver_probe(struct platform_device *pdev)
{
	int ret = 0;
    uint32_t val = 0;
    struct device_node *power_node = NULL;
	if (!of_match_device(sensorhub_io_supply_ids, &pdev->dev)) {
		hwlog_err("[%s,%d]: sensorhub_io_driver_probe match fail !\n",
			  __FUNCTION__, __LINE__);
		return -ENODEV;
	}
        power_node = of_find_node_by_name(NULL, "sensorhub_io_power");
        if(!power_node)
        {
            hwlog_err("%s failed to find dts node sensorhub_io_power\n", __func__);
        }else if(of_property_read_u32(power_node, "need-reset", &val))
        {
            hwlog_err("%s failed to find property need-reset\n", __func__);
        }else
        {
            need_reset_io_power = val;
            hwlog_info("%s property need-reset is %d\n", __func__, val);
        }
	sensorhub_vddio = regulator_get(&pdev->dev, SENSOR_VBUS);
	if (IS_ERR(sensorhub_vddio)) {
		hwlog_err("%s: regulator_get fail!\n", __func__);
		return -EINVAL;
	}

	ret = regulator_enable(sensorhub_vddio);
	if (ret < 0)
		hwlog_err("failed to enable regulator sensorhub_vddio\n");

	if(need_reset_io_power) {
		ret = regulator_disable(sensorhub_vddio);
		if (ret < 0)
			hwlog_err("failed to disable regulator sensorhub_vddio\n");

		sensor_jiffies = jiffies;
	}
	g_sensorhub_wdt_irq = platform_get_irq(pdev, 0);
	if (g_sensorhub_wdt_irq < 0) {
		pr_err("[%s] platform_get_irq err\n", __func__);
		return -ENXIO;
	}
	sensor_power_pmic_flag = 0;
	sensor_power_init_finish = 0;
	hwlog_err("%s: sensorhub_io_driver_probe success!\n", __func__);
	return 0;
}

extern int sensorhub_pm_suspend(struct device *dev);
extern int sensorhub_pm_resume(struct device *dev);

struct dev_pm_ops sensorhub_io_pm_ops = {
	.suspend = sensorhub_pm_suspend,
	.resume = sensorhub_pm_resume,
};

struct platform_driver sensorhub_io_driver = {
	.probe = sensorhub_io_driver_probe,
	.driver = {
		   .name = "Sensorhub_io_driver",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(sensorhub_io_supply_ids),
		   .pm = &sensorhub_io_pm_ops,
		   },
};

static int __init sensor_input_info_init(void)
{
	int ret = 0;
	hwlog_info("[%s] ++ \n", __func__);

	spin_lock_init(&fsdata_lock);

	ret = platform_device_register(&sensor_input_info);
	if (ret) {
		hwlog_err("%s: platform_device_register failed, ret:%d.\n",
			  __func__, ret);
		goto REGISTER_ERR;
	}

	ret = sysfs_create_group(&sensor_input_info.dev.kobj, &sensor_node);
	if (ret) {
		hwlog_err
		    ("sensor_input_info_init sysfs_create_group error ret =%d. \n",
		     ret);
		goto SYSFS_CREATE_CGOUP_ERR;
	}
	hwlog_info("[%s] --\n", __func__);

	return 0;
SYSFS_CREATE_CGOUP_ERR:
	platform_device_unregister(&sensor_input_info);
REGISTER_ERR:
	return ret;

}

static int sensorhub_io_driver_init(void)
{
	int ret = 0;
	hwlog_info("[%s] ++", __func__);

	ret = platform_driver_register(&sensorhub_io_driver);
	if (ret) {
		hwlog_err
		    ("%s: platform_device_register(sensorhub_io_driver) failed, ret:%d.\n",
		     __func__, ret);
		return ret;
	}
	hwlog_info("[%s] --", __func__);

	return 0;
}

static int judge_tp_color_correct(u8 color)
{
	/*if the tp color is correct, after the 4~7 bit inversion shoud be same with 0~3 bit;*/
	return ((color & 0x0f) == ((~(color >> 4)) & 0x0f));
}

static int __init light_sensor_read_tp_color_cmdline(char *arg)
{
	int len = 0;
	int tp_color = 0;
	memset(tp_color_buf, 0, sizeof(tp_color_buf));
	tpmodule_register_client(&readtp_notify);
	if (arg) {
		len = strlen(arg);

		if (len > sizeof(tp_color_buf)) {
			len = sizeof(tp_color_buf);
		}
		memcpy(tp_color_buf, arg, len);
	} else {
		hwlog_info("%s : arg is NULL\n", __func__);
		phone_color = WHITE;	/*WHITE is the default tp color*/
		return 0;
	}

	tp_color = (int)simple_strtol(tp_color_buf, NULL, 10);

	if (judge_tp_color_correct(tp_color)) {
		phone_color = tp_color;
	} else {
		hwlog_err("light sensor LCD/TP ID error!\n");
		phone_color = WHITE;	/*WHITE is the default tp color*/
	}
	hwlog_err("light sensor read tp color is %s, 0x%x\n", tp_color_buf,
		  phone_color);
	return 0;
}

early_param("TP_COLOR", light_sensor_read_tp_color_cmdline);

static int mag_current_notify(void)
{
	int ret = 0;
	if(mag_data.charger_trigger) {
		charger_notify.notifier_call = mag_enviroment_change_notify;
		ret = hisi_charger_type_notifier_register(&charger_notify);
		if (ret < 0)
			hwlog_err("mag_charger_type_notifier_register failed\n");
	}
	return ret;
}

fs_initcall(sensorhub_io_driver_init);
late_initcall_sync(mag_current_notify);
device_initcall(sensor_input_info_init);
MODULE_DESCRIPTION("sensor input info");
MODULE_AUTHOR("huawei driver group of K3V3");
MODULE_LICENSE("GPL");
//lint -restore
