/*
 * Copyright (c) 2018 INGCHIPS MESH
 */

#include <stdio.h>
#include "light_model.h"
#include "state_binding.h"
#include "device_composition.h"
#include "transition.h"
#include "eflash.h"
#include "mesh_api.h"
#include "kv_storage.h"
#include "profile.h"

#include "chip_peripherals.h"

#ifndef TRUE
#define TRUE 1
#define FALSE (!TRUE)
#endif
/*
const uint16_t MAX_TEMP =  0x4E20;
const uint16_t MIN_TEMP =  0x0320;
const uint16_t DIS_TEMP =  MAX_TEMP - MIN_TEMP;
const uint16_t MID_TEMP =  (MAX_TEMP+MIN_TEMP)/2;
*/

#define MAX_TEMP 0x4E20
#define MIN_TEMP 0x0320
#define DIS_TEMP (MAX_TEMP - MIN_TEMP)
#define MID_TEMP ((MAX_TEMP + MIN_TEMP) / 2)




bool light_status_set = false;
bool last_light_status = false;
u8_t gen_onoff_state;
static s16_t gen_level_state;
static float R,G,B;
uint8_t Rv, Gv,Bv;
uint32_t GWW,GCW;
static u16_t cur_ctl_light = 0;
static u16_t due_light =0;
//static s16_t cur_ctl_deluv = 0;
static s16_t due_deluv = 0;
static u16_t cur_ctl_temp = MID_TEMP;
static u16_t due_temp =0;
struct k_delayed_work breath_mode;
struct k_delayed_work breath_work;
struct k_delayed_work flash_work;
struct cached_data_t cust_mem;
#define SPI_LIGHT         GIO_L32_GPIO_3

#define HIGH_LEVEL_WORK      (0)
#define LIGHT_TEMP_CIRCUIT   (1)

#define UNUSED  (0xffffffff)
uint32_t val;



#define LED_R  PWM_R
#define LED_G  PWM_G
#define LED_B  PWM_B


uint16_t curr_hue = 0;
uint16_t curr_sa = 0;
uint16_t curr_lightness =0;
uint8_t remain;

void light2save(struct ble_npl_event *ev)
{
    printf("flash write light param\n");
    kv_put(CUST_CONF_FLASH_INDEX,(uint8_t*)&cust_mem,sizeof(cust_mem));
}

/**
 * @brief the mesh stack would call this API to modify the light status ,lightness and etc.
 */
#define PIN_SDI   GIO_GPIO_0
extern void set_led_color(uint8_t r, uint8_t g, uint8_t b);
void update_light_state(void)
{
	u32_t power, color;
    uint32_t cach_value =0;
	power = lightness;
    uint8_t ratio = 0;
    uint8_t set_lightness = 0;
	color = 100 * ((float) (temperature + 32768) / 65535);
    GWW = color;//100-color;
    GCW = 100-color;
    if(power>0)
        gen_onoff_state = 1;
    else
        gen_onoff_state = 0;
	printf("Light state: onoff=%d lightness=0x%04x CW= 0x%x\n", gen_onoff_state, (u16_t)power,GCW);
	if(1 == gen_onoff_state)
	{
        ratio = (uint32_t)(lightness *100)>>16;
        set_lightness =  ((uint32_t)(ratio<<8))/100;
        Rv= set_lightness;
        Gv= set_lightness;
        Bv= set_lightness;
        GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
        GIO_WriteValue(PIN_SDI, 0);
        set_led_color(set_lightness,set_lightness,set_lightness);
        if(last_lightness == lightness)
            goto light_save;
        return;
	}
	else
	{
        GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
        GIO_WriteValue(PIN_SDI, 0);
        set_led_color(0,0,0);
        goto light_save;
	}

light_save:
#if (FLASH_ENABLE)
        cach_value = last_lightness << 16 | GCW<<8 | GWW;
        printf("write light 0x%x\n",cach_value);
        cust_mem.lightness = last_lightness;
        cust_mem.temperature =  (GCW<<8 | GWW);
        cust_mem.R = Rv;
        cust_mem.G = Gv;
        cust_mem.B = Bv;
        cust_mem.status = gen_onoff_state;
        //cached to flash
        k_delayed_work_submit(&flash_work,500);
        return;
#endif
}

/**
 * @brief callback for the gen_onoff models status check.
 *
 */
int light_model_gen_onoff_get(struct bt_mesh_model *model, u8_t *state)
{
	*state = gen_onoff_state;
	return 0;
}

/**
 * @brief callback for the gen_onoff models set operation.
 *
 */
int light_model_gen_onoff_set(struct bt_mesh_model *model, u8_t state)
{
	gen_onoff_state = state;
	update_light_state();
	return 0;
}

int light_model_gen_level_get(struct bt_mesh_model *model, s16_t *level)
{
	*level = gen_level_state;
    printf("the curren level %d %d\n",gen_level_state,*level);
	return 0;
}

int light_model_gen_level_set(struct bt_mesh_model *model, s16_t level)
{
    uint8_t lightratio;
    uint16_t light = level + 32768;
    gen_level_state = level;
    uint32_t loc_r,loc_g,loc_b;
    uint32_t CW,WW;
    CW = GCW;
    WW = GWW;
    printf("the level is %d\n",level);
	if ((u16_t)light > 0x0000) {
#if (LIGHT_TEMP_CIRCUIT==0)
        CW = (GCW==0)? 10:GCW;
        WW = (GWW==0)? 10:GWW;
#endif
		gen_onoff_state = 1;
        if(light == 65535)
            lightratio =100;
        else
        {
            lightratio = light*100/65535;
            uint16_t temp = (light*100)&0xffff;
            if (temp > 32767)
                lightratio +=1;
        }
        printf("lightratio is 0x%x\n",lightratio);
        //configure the PWM input
        loc_r =  255 * lightratio/100;
        loc_g =  255 * lightratio/100;
        loc_b =  255 * lightratio/100;
#if (LIGHT_TEMP_CIRCUIT==0)
        CW = (CW * loc_r*100/Rv)/100;
        te = (CW * loc_r*100/Rv) % 100;
        if (te >=50)
            CW+=1;
        WW = (WW * loc_r*100/Rv)/100;
        te = (WW * loc_r*100/Rv)%100;
        if (te >50)
            WW +=1;
        printf("CW %d  WW %d \n",CW,WW);
        if(CW>100)
            CW =100;
        if(WW>100)
            WW =100;
#endif
        PWM_CONFIG(loc_r,LED_R);
        PWM_CONFIG(loc_g,LED_G);
        PWM_CONFIG(loc_b,LED_B);
#if (LIGHT_TEMP_CIRCUIT)
        light_ctl_adjust(WW,CW,light);
#else
        PWM_CONFIG(WW,PWM_WARM);
        PWM_CONFIG(CW,PWM_COLD);
#endif
        Rv = loc_r;
        Gv = loc_g;
        Bv = loc_b;
        GWW = WW;
        GCW = CW;

	}
	else if ((u16_t)gen_level_state == 0x0000) {
		gen_onoff_state = 0;
        Rv=Gv=Bv=0;
	    update_light_state();
	}

	return 0;
}

int light_model_HSL_hue_set(struct bt_mesh_model *model, uint16_t level)
{
	gen_level_state = level;
	if ((u16_t)gen_level_state > 0x0000) {
		gen_onoff_state = 1;
	}
	if ((u16_t)gen_level_state == 0x0000) {
		gen_onoff_state = 0;
	}
    //call PWM to adjust the lightness
	update_light_state();
	return 0;
}

int light_model_light_lightness_get(struct bt_mesh_model *model, u16_t *p_lightness,u16_t *lightness,u8_t *remain)
{
    s16_t temp=0;
    temp = gen_level_state +32768;
    *lightness = htole16(temp);
    *p_lightness = *lightness;
    *remain =0;
    return 0;
}

int light_model_light_lightness_set(struct bt_mesh_model *model, u16_t light)
{
    s16_t light1 = light - 32768;
	return light_model_gen_level_set(model, light1);
}


int light_model_light_HSL_get(struct bt_mesh_model *model, uint16_t *hue, uint16_t *sa, uint16_t *lightness,uint8_t *trans)
{
    *hue = curr_hue;
    *sa = curr_sa;
    *lightness = curr_lightness;
    *trans =  remain;
    return 0;
}

int light_model_light_HSL_set(struct bt_mesh_model *model,hsl_val_t* val)
{
    //set the value of HSL to notify the driven
    printk("start\n");
    HSL_PWM_LED(val->hue,val->sa, val->lightness);
    return 0;
}

int light_rgb_set(uint8_t rv ,uint8_t gv,uint8_t bv )
{
    uint16_t rl,gl,bl;
//    rl = (rv>=255)?100:(rv*100)>>8;
//    gl = (gv>=255)?100:(gv*100)>>8;
//    bl = (bv>=255)?100:(bv*100)>>8;
    printk("uint8_t %d %d %d\n",rl,gl,bl);
    PWM_CONFIG(rv,LED_R);
    PWM_CONFIG(gv,LED_G);
    PWM_CONFIG(bv,LED_B);
    return 0;
}

int lingtness_set(uint16_t light)
{
    uint16_t light1 =0;
    light1 = light*100>>16;
    PWM_CONFIG(light1,LED_R);
    PWM_CONFIG(light1,LED_G);
    PWM_CONFIG(light1,LED_B);
    return 0;
}
float Hue_2_RGB( float v1, float v2, float vH ) //Function Hue_2_RGB
{
    printk("Hue_2_RGB: v1 %f  v2 %f  vH %f\n",v1,v2,vH);
    if ( vH < 0 )
        vH += 1;
    if ( vH > 1 )
        vH -= 1;
    if (( 6.0 * vH ) < 1 )
        return ( v1 + ( v2 - v1 ) * 6.0 * vH );
    if (( 2.0 * vH ) < 1 )
        return ( v2 );
    if (( 3.0 * vH ) < 2 )
        return ( v1 + ( v2 - v1 ) * ( ( 2.0/3.0 ) - vH ) * 6.0 );
    return ( v1 );
}
typedef struct RGB_VAL
{
    uint8_t R_VAL;
    uint8_t G_VAL;
    uint8_t B_VAL;
}rgb_val_t;

void HSL_2_RGB(uint16_t hue,uint16_t sa, uint16_t light )
{
    float H,S,L,var_2,var_1;
    H = hue / 65535.0;
    S = sa / 65535.0;
    L = light / 65535.0;
    printk("input H %f S %f L %f\n",H,S,L);
    if ( S == 0 )
    {
        R = L;
        G = L;
        B = L;
    }
    else
    {
        if ( L < 0.5 )
            var_2 = L * ( 1 + S );
        else
            var_2 = ( L + S ) - ( S * L );
        var_1 = 2 * L - var_2;
        printk("var_2 %f var_1 %f \n",var_2,var_1);
        R = Hue_2_RGB( var_1, var_2, H + ( 1.0/3.0 ));
        G = Hue_2_RGB( var_1, var_2, H );
        B = Hue_2_RGB( var_1, var_2, H - ( 1.0/3.0 ));
    }
    printk("HSL : R: %f G:%f B:%f",R,G,B);
}

void HSL_PWM_LED(uint16_t hue,uint16_t sa, uint16_t light)
{
    HSL_2_RGB(hue,sa,light);
    Rv = (R*255.0+0.5);
    printk("RV %d\n",Rv);
    Gv = G*255.0+0.5;
    Bv = B*255.0+0.5;
    light_rgb_set(Rv,Gv,Bv);
}
#define CLOCK_FREQ 24000000u
#define FREQ_EYE 1000
#define PERA_THRESHOLD 0x100
uint32_t pwm_value_update(uint8_t VAL,uint8_t PIN )
{
    uint32_t VAL2;
    uint32_t period = PERA_THRESHOLD;

#if (HIGH_LEVEL_WORK)
     if(0==(PIN%2))
       VAL2 =period - VAL*period/100;
     else
       VAL2 =  VAL*period/100;
#else
     if(0==(PIN%2))
       VAL2 = VAL*period/100;
     else
       VAL2 = period - VAL*period/100L;
#endif
     if(VAL2==256)
         return 255;
     else
        return VAL2;
}
uint32_t PWM_CONFIG(uint8_t VAL,uint8_t PIN)
{
    uint32_t VAL2;
    uint32_t period = PERA_THRESHOLD;
    //one PWM is mapped to 2 opsition way's signal.
    if((PIN == PWM_WARM ) || (PIN == PWM_COLD))   //this 2 PIN ,represent 0-100
    {
#if (HIGH_LEVEL_WORK)
        if (0==PIN %2)
            VAL2 = period - VAL*period/100;  //low is lighten, high is off,
        else
            VAL2 = VAL*period/100;
        //printf("wm pin %d\n val VALE SET 0x%x  period 0x%x",PIN,VAL2,period);
#else
        if (0==PIN %2)
            VAL2 = VAL*period/100;  //low is lighten, high is off,
        else
            VAL2 = period - VAL*period/100;
     //   printf("wm pin %d\n VALE SET 0x%x  period 0x%x",PIN,VAL2,period);
#endif
    PWM_SetHighThreshold(PIN   >> 1, 0,VAL2);
    }
    else   //represent RGB : 0---256)
    {
#if (HIGH_LEVEL_WORK)
     if(0==(PIN%2))
       VAL2 =255 - VAL;
     else
       VAL2 = VAL;
#else
     if(0==(PIN%2))
       VAL2 = VAL;
     else
       VAL2 = 255 - VAL;
#endif
	// 	printf("pin %d\n VALE SET 0x%x  period 0x%x",PIN,VAL2,period);
  //  PWM_PIN_CONFIG(PIN,period,VAL2);
     if (VAL2==255)
         PWM_SetHighThreshold(PIN   >> 1, 0, PERA_THRESHOLD);
     else
         PWM_SetHighThreshold(PIN   >> 1, 0, VAL2);
    }
    return 0;
}

/*light ctrl temp call back*/
//static uint16_t local_temp;   //0x0320 --0x4E20
//static s16_t local_deluv;     //0x8000--0x7FFF
int light_model_ctl_temp_get(struct bt_mesh_model *model,uint16_t *temp , s16_t* deluv, uint16_t* tar_temp, s16_t* tar_deluv, uint8_t* time)
{

	return 0;
}

int light_model_ctl_temp_set(struct bt_mesh_model *model,uint16_t temp ,  s16_t deluv, uint8_t TID, uint8_t transit, uint8_t delay)
{
//	gen_onoff_state = state;
//	update_light_state();
	return 0;
}


//light ctl setup for PWM
void light_ctl_adjust(uint8_t WW, uint8_t CW,uint16_t light)
{
    // initial conditional 50% CW, 50% WW
    uint32_t mulrat =0;
    if(WW!=0 || CW !=0)
    {
        GWW = WW;
        GCW = CW;
    }
    uint32_t re = light *100;
    if(light ==65535)
    mulrat =100;
    else
    {
        mulrat = re>>16;
        if((re&0xffff)>32768)
        mulrat+=1;
    }
#if (LIGHT_TEMP_CIRCUIT)
    uint32_t temp_rat =0;
    uint32_t me = CW*100;
    if(CW==0 && WW==0 && light==0)  //light all off;
    {
        temp_rat =0;
        mulrat =0;
    }
    else
    {
        temp_rat = me/(CW+WW);
        if( me%100>50 )
            temp_rat+=1;
    }
    printf("mulrat 0x%x,temp_rat 0x%x\n",mulrat,temp_rat);
    PWM_CONFIG(mulrat,PWM_COLD);   //CW for LIGHTNESS
    PWM_CONFIG(temp_rat,PWM_WARM);             //WW for temperatue;
#else
    PWM_CONFIG(temp1=WW*mulrat/100,PWM_WARM);
    PWM_CONFIG(temp2=CW*mulrat/100,PWM_COLD);
    printf("final state WW %d, CW %d\n",temp1,temp2);
#endif
}
void update_light_ctl(uint16_t light , uint16_t temp, s16_t delua)
{
	printf("Light ctl state: lightness =%d temp =%d deluv=%d\n", light, temp,delua);
    uint8_t CW=0;
    uint8_t WW=0;
    uint32_t te =0;
    //temperature scope : 0x0320--0x4E20
    {
        CW = (temp-MIN_TEMP) * 100 / DIS_TEMP;
        te = ((temp-MIN_TEMP) * 100) % DIS_TEMP;
        if (te > (DIS_TEMP/2))
            CW +=1;
        WW = (DIS_TEMP-temp+MIN_TEMP)*100/DIS_TEMP;
        te = (DIS_TEMP-temp+MIN_TEMP)*100 % DIS_TEMP;
        if (te > (DIS_TEMP/2))
            WW +=1;

    }
    printf("updated CW %d WW %d\n",CW,WW);
    cur_ctl_temp = temp;
    cur_ctl_light = light;
    gen_level_state = (s16_t)(light - 32768);
//    cur_ctl_deluv = delua;
    PWM_CONFIG(Rv,LED_R);
    PWM_CONFIG(Gv,LED_G);
    PWM_CONFIG(Bv,LED_B);
    //to calculate the period  CW and WW
    light_ctl_adjust(WW,CW,light);
}

int light_model_ctl_get(struct bt_mesh_model *model,uint16_t *lightness,uint16_t *temp , uint16_t* tar_lightness, uint16_t* tar_temp, uint8_t* time)
{
    *lightness = cur_ctl_light;
    *temp = cur_ctl_temp;
    *tar_temp = cur_ctl_temp;
    *tar_lightness = cur_ctl_light;
    *time =0;
	return 0;
}

int light_model_ctl_set(struct bt_mesh_model *model,uint16_t light , uint16_t temp, s16_t deluv,uint8_t TID, uint8_t transit, uint8_t delay)
{
//	gen_onoff_state = state;
    due_light = light;
    due_temp = temp;
    due_deluv = deluv;
	update_light_ctl(due_light,due_temp,due_deluv);
	return 0;
}
void light_default_var_init(void);
int light_power_on()
{
    uint32_t lightstatus=0;
    gen_onoff_state =1;
    uint32_t count_m;
    uint32_t count_n ;
    //uint16_t light =0;
    uint8_t  cw=0;
#if (FLASH_ENABLE)
    struct cached_data_t  ctword={0};
    int16_t len;
    uint8_t* db = NULL;
    db = kv_get(CUST_CONF_FLASH_INDEX,&len);
    if(db && (len >0))
        memcpy(&ctword,(uint8_t*)db,sizeof(cust_mem));
    memcpy(&lightstatus,&ctword,4);   //read the lightness and temperature
    //restore the lightness and temp to light model
    if(lightstatus ==0 || lightstatus == 0xffffffff)
    {
        GCW =50;
        GWW =50;
        gen_level_state = 32767;
     //   light_reset(65535,50,50);
        //to make  the light change evently
        uint16_t pack_t = (65535- lightstatus >>16)/2000;
        if (pack_t ==0)
            pack_t =1;
        uint16_t pack_cw = 5;
        for(count_n=0;count_n<2000;count_n++)
        {
            cw = pack_cw * count_n/200;
 //           printf("2 %d %d",cw,ww);
            light_reset(65535,cw);
            for(count_m=0;count_m<7062;count_m++)
            {
            }
        }
        //revert the lightness and temperature from flash.
        last_lightness =  0xffff;
        temperature = 0;
    }
    else
    {
        gen_level_state = (s16_t)((lightstatus&0xffff)-32768);
        printf("last light status 0x%x \n",ctword.status);
        if(ctword.status)
            set_led_color(50,50,50);
        else
            set_led_color(0,0,0);
        light_status_set = true;
        last_light_status = ctword.status;
    }
//    printf("light from flash level light 0x%x %d GCW %d GWW %d\n",lightstatus&0xffff,gen_level_state,GCW,GWW);
#endif
    return 0;
}

int light_reset(uint16_t light, uint8_t cw)
{
    uint32_t re = light *100;
    uint32_t mulrat =0;
    if(light ==0)
    {
        mulrat =0;
    }
    else
    {
        if(light ==65535)
            mulrat =100;
        else
        {
            mulrat = re>>16;
            if((re&0xffff)>32768)
            mulrat+=1;
        }
    }
    uint8_t val = 0;
    val = pwm_value_update(mulrat,PIN_SDI);
    set_led_color(val,val,val);
    return 0;
}


void light_default_var_init(void)
{
	gen_def_trans_time_srv_user_data.tt = 0x00;

	gen_power_onoff_srv_user_data.onpowerup = STATE_RESTORE;

	light_lightness_srv_user_data.light_range_min = LIGHTNESS_MIN;
	light_lightness_srv_user_data.light_range_max = LIGHTNESS_MAX;
	light_lightness_srv_user_data.last = LIGHTNESS_MAX;
	light_lightness_srv_user_data.def = LIGHTNESS_MAX;

	/* Following 2 values are as per specification */
	light_ctl_srv_user_data.temp_range_min = TEMP_MIN;
	light_ctl_srv_user_data.temp_range_max = TEMP_MAX;

	light_ctl_srv_user_data.temp_def = TEMP_MIN;
	light_ctl_srv_user_data.lightness_temp_last =(uint32_t)LIGHTNESS_MAX <<16 | TEMP_MIN;
}

void light_default_status_init(void)
{
	//u16_t lightness;
    //read from the flash,the last value for lightness and light temperature
    uint32_t t1 = ((gen_level_state+32768));
    uint32_t t2 = (MIN_TEMP +  DIS_TEMP *(100-GCW)/100);
    last_lightness = t1;
    light_ctl_srv_user_data.lightness_temp_def = light_ctl_srv_user_data.lightness_temp_last =  (t1<<16) | t2;
    printf("t1 0x%x t2 0x%x 3 0x%x\n",t1,t2,light_ctl_srv_user_data.lightness_temp_last);
		lightness = (u16_t) (light_ctl_srv_user_data.lightness_temp_last >> 16);
extern bool last_light_status;
	if (last_light_status) {
		gen_onoff_srv_root_user_data.onoff = STATE_ON;
        gen_onoff_srv_root_user_data.target_onoff = STATE_ON;  //added by dengyiyn, for first status check
	} else {
		gen_onoff_srv_root_user_data.onoff = STATE_OFF;
        gen_onoff_srv_root_user_data.target_onoff = STATE_OFF; //added by dengyiyun, for first status check
	}

	/* Retrieve Default Lightness & Temperature Values */

	if (light_ctl_srv_user_data.lightness_temp_def) {
		light_ctl_srv_user_data.lightness_def = (u16_t)
			(light_ctl_srv_user_data.lightness_temp_def >> 16);

		light_ctl_srv_user_data.temp_def = (u16_t)
			(light_ctl_srv_user_data.lightness_temp_def);
	}

	light_lightness_srv_user_data.def =
		light_ctl_srv_user_data.lightness_def;

	light_ctl_srv_user_data.temp = light_ctl_srv_user_data.temp_def;

	/* Retrieve Range of Lightness & Temperature */

	if (light_lightness_srv_user_data.lightness_range) {
		light_lightness_srv_user_data.light_range_max = (u16_t)
			(light_lightness_srv_user_data.lightness_range >> 16);

		light_lightness_srv_user_data.light_range_min = (u16_t)
			(light_lightness_srv_user_data.lightness_range);
	}

	if (light_ctl_srv_user_data.temperature_range) {
		light_ctl_srv_user_data.temp_range_max = (u16_t)
			(light_ctl_srv_user_data.temperature_range >> 16);

		light_ctl_srv_user_data.temp_range_min = (u16_t)
			(light_ctl_srv_user_data.temperature_range);
	}

	switch (gen_power_onoff_srv_user_data.onpowerup) {
		case STATE_OFF:
			gen_onoff_srv_root_user_data.onoff = STATE_OFF;
			state_binding(ONOFF, ONOFF_TEMP);
			break;
		case STATE_DEFAULT:
			gen_onoff_srv_root_user_data.onoff = STATE_ON;
			state_binding(ONOFF, ONOFF_TEMP);
			break;
		case STATE_RESTORE:
			light_lightness_srv_user_data.last = (u16_t)
				(light_ctl_srv_user_data.lightness_temp_last >> 16);
            gen_level_srv_root_user_data.level = light_lightness_srv_user_data.last - 32768;
			light_ctl_srv_user_data.temp =
				(u16_t) (light_ctl_srv_user_data.lightness_temp_last);
            //add by dengyiyun ,to setup the actual of lightness
            light_lightness_srv_user_data.actual =  light_lightness_srv_user_data.last;
			state_binding(ONPOWERUP, ONOFF_TEMP);
			break;
	}

	default_tt = gen_def_trans_time_srv_user_data.tt;
}


int model_conf_init()
{
    light_default_var_init();
    transition_timers_init();
    light_default_status_init();
    //initialize the flash save work for light work.
    k_delayed_work_init(&flash_work,light2save);
    return 0;
}


static volatile bool breath_mode_is_run = FALSE;
#define BREATH_MODE_DURATION  60000
int model_breath_mode()
{
    uint8_t  cw=0;
    GCW =50;
    GWW =50;
    gen_level_state = 32767;
    uint16_t light = 0;
    uint32_t count_m,count_n=0;
    //to make  the light change evently
    uint16_t pack_t = (65535)/2000;
    uint16_t pack_cw = 5;
    //for lightness to darkness
    if (65535%2000>2000)
    pack_t+=1;
    for(count_n=0;count_n<2000;count_n++)
    {
        if(count_n==1999)
        {
            light = 0;
            cw =0;
        }
        else
        {
            uint32_t m= pack_t * count_n;
            uint32_t n= pack_cw * (count_n/200);
            light = (65535> m)? 65535- m:0;
            cw = (50 > n)?50 - n:0;
            cw = (light==0)? 0:50-n;
            if(cw>GCW)
            cw=GCW;
        }
				// printf(" 3:%d %d %d",light,cw,ww);
        light_reset(light,cw);
        for(count_m=0;count_m<1500;count_m++)
        {
        }
    }
    //from darkness to lightness
    for(count_n=0;count_n<2000;count_n++)
    {
        if(count_n==1999)
        {
            light = 65535;
            cw =50;
        }
        else
        {
            uint32_t m = pack_t * count_n;
            uint32_t n = pack_cw * (count_n/200);
            light =  (m > 65535)? 65535:m;
            cw = (n>50)?50:n;
            if(cw>GCW)
                cw=GCW;
        }
//    	printf(" 4:%d %d %d",light,cw,ww);
        light_reset(light,cw);
        for(count_m=0;count_m<1500;count_m++)
        {
        }
    }
// 	printf("breath cou %d\n",count_x++);
    return 0;
}
void breath_mode_expire(struct ble_npl_event *work)
{
    breath_mode_is_run = FALSE;
    printf("stop breath mode\n");
    return;
}
bool IS_BREATH_RUN()
{
    return breath_mode_is_run;
}


void breath_mode_work(struct ble_npl_event *work)
{
    if(IS_BREATH_RUN())
    {
        k_delayed_work_submit(&breath_work,2000);
        model_breath_mode();
    }
    else
    {
        printf("unbind mode finish\n");
    }
}

int unbind_light_mode_run(uint32_t duration)
{
    k_delayed_work_init(&breath_mode,breath_mode_expire);
    k_delayed_work_init(&breath_work,breath_mode_work);
    k_delayed_work_submit(&breath_mode,duration);
    k_delayed_work_submit(&breath_work,2000);
    breath_mode_is_run = TRUE;
    printf("unbind mode start\n");
    return 1;

}

int light_provsioned_complete()
{
    if(IS_BREATH_RUN())
    {
        breath_mode_is_run =  FALSE;
        k_delayed_work_cancel(&breath_mode);
        printf("breath mode stopped\n");
    }
    return 0;
}
