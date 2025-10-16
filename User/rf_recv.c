#include "rf_recv.h"

// 标志位，是否处于rf对码模式
// 0--未处于
// 1--正在对码，等待按键长按
// 2--已经对完码，等待按键松手
volatile u8 flag_is_in_rf_learning = 0;

// rf对码期间，用于临时存放遥控器地址的变量
// 在 rf_key_get_key_id() 函数中取得，如果对码成功，则直接应用该地址，写入flash
volatile u32 tmp_rf_addr = 0xFFFFFFFF;
volatile rf_remote_info_t rf_remote_info = {0};

volatile bit flag_is_rf_enable = 0;      // 标志位，是否使能rf遥控器的功能
volatile bit flag_is_recved_rf_data = 0; // 是否接收到了rf信号
volatile u32 rf_data = 0;                // 存放接收到的rf数据

// // 将按键键值与
// static const u8 rf_key_map_table[][2] =
// {

// }

// 定义触摸按键的按键事件
enum RF_KEY_EVENT
{
    RF_KEY_EVENT_NONE = 0,
    RF_KEY_EVENT_ID_1_CLICK,
    RF_KEY_EVENT_ID_1_LONG,
    RF_KEY_EVENT_ID_1_HOLD,
    RF_KEY_EVENT_ID_1_LOOSE,

    RF_KEY_EVENT_ID_2_CLICK,
    RF_KEY_EVENT_ID_2_LONG,
    RF_KEY_EVENT_ID_2_HOLD,
    RF_KEY_EVENT_ID_2_LOOSE,

    RF_KEY_EVENT_ID_3_CLICK,
    RF_KEY_EVENT_ID_3_LONG,
    RF_KEY_EVENT_ID_3_HOLD,
    RF_KEY_EVENT_ID_3_LOOSE,

    RF_KEY_EVENT_ID_4_CLICK,
    RF_KEY_EVENT_ID_4_LONG,
    RF_KEY_EVENT_ID_4_HOLD,
    RF_KEY_EVENT_ID_4_LOOSE,

    RF_KEY_EVENT_ID_5_CLICK,
    RF_KEY_EVENT_ID_5_LONG,
    RF_KEY_EVENT_ID_5_HOLD,
    RF_KEY_EVENT_ID_5_LOOSE,

    RF_KEY_EVENT_ID_6_CLICK,
    RF_KEY_EVENT_ID_6_LONG,
    RF_KEY_EVENT_ID_6_HOLD,
    RF_KEY_EVENT_ID_6_LOOSE,

    RF_KEY_EVENT_ID_7_CLICK,
    RF_KEY_EVENT_ID_7_LONG,
    RF_KEY_EVENT_ID_7_HOLD,
    RF_KEY_EVENT_ID_7_LOOSE,

    RF_KEY_EVENT_ID_8_CLICK,
    RF_KEY_EVENT_ID_8_LONG,
    RF_KEY_EVENT_ID_8_HOLD,
    RF_KEY_EVENT_ID_8_LOOSE,

    RF_KEY_EVENT_ID_9_CLICK,
    RF_KEY_EVENT_ID_9_LONG,
    RF_KEY_EVENT_ID_9_HOLD,
    RF_KEY_EVENT_ID_9_LOOSE,

    RF_KEY_EVENT_ID_10_CLICK,
    RF_KEY_EVENT_ID_10_LONG,
    RF_KEY_EVENT_ID_10_HOLD,
    RF_KEY_EVENT_ID_10_LOOSE,

    RF_KEY_EVENT_ID_11_CLICK,
    RF_KEY_EVENT_ID_11_LONG,
    RF_KEY_EVENT_ID_11_HOLD,
    RF_KEY_EVENT_ID_11_LOOSE,

    RF_KEY_EVENT_ID_12_CLICK,
    RF_KEY_EVENT_ID_12_LONG,
    RF_KEY_EVENT_ID_12_HOLD,
    RF_KEY_EVENT_ID_12_LOOSE,

#if USE_MY_TEST_433_REMOTE

    // 测试时使用到的按键事件，实际不使用
    RF_KEY_EVENT_ID_TEST_1_CLICK,
    RF_KEY_EVENT_ID_TEST_1_LONG,
    RF_KEY_EVENT_ID_TEST_1_HOLD,
    RF_KEY_EVENT_ID_TEST_1_LOOSE,

    RF_KEY_EVENT_ID_TEST_2_CLICK,
    RF_KEY_EVENT_ID_TEST_2_LONG,
    RF_KEY_EVENT_ID_TEST_2_HOLD,
    RF_KEY_EVENT_ID_TEST_2_LOOSE,

    RF_KEY_EVENT_ID_TEST_3_CLICK,
    RF_KEY_EVENT_ID_TEST_3_LONG,
    RF_KEY_EVENT_ID_TEST_3_HOLD,
    RF_KEY_EVENT_ID_TEST_3_LOOSE,

    RF_KEY_EVENT_ID_TEST_4_CLICK,
    RF_KEY_EVENT_ID_TEST_4_LONG,
    RF_KEY_EVENT_ID_TEST_4_HOLD,
    RF_KEY_EVENT_ID_TEST_4_LOOSE,

#endif // #if USE_MY_TEST_433_REMOTE
};

#define RF_KEY_EFFECT_EVENT_NUMS (4) // 单个触摸按键的有效按键事件个数 (单击、长按、持续、松开)
// 将按键id和按键事件绑定起来，在 xx 函数中，通过查表的方式得到按键事件
// static const u8 rf_key_event_table[][RF_KEY_EFFECT_EVENT_NUMS + 1] = {
//     // [0]--按键对应的id号，用于查表，[1]、[2]、[3]...--用于与 key_driver.h 中定义的按键事件KEY_EVENT绑定关系(一定要一一对应)
//     {RF_KEY_ID_1, RF_KEY_EVENT_ID_1_CLICK, RF_KEY_EVENT_ID_1_LONG, RF_KEY_EVENT_ID_1_HOLD, RF_KEY_EVENT_ID_1_LOOSE},      //
//     {RF_KEY_ID_2, RF_KEY_EVENT_ID_2_CLICK, RF_KEY_EVENT_ID_2_LONG, RF_KEY_EVENT_ID_2_HOLD, RF_KEY_EVENT_ID_2_LOOSE},      //
//     {RF_KEY_ID_3, RF_KEY_EVENT_ID_3_CLICK, RF_KEY_EVENT_ID_3_LONG, RF_KEY_EVENT_ID_3_HOLD, RF_KEY_EVENT_ID_3_LOOSE},      //
//     {RF_KEY_ID_4, RF_KEY_EVENT_ID_4_CLICK, RF_KEY_EVENT_ID_4_LONG, RF_KEY_EVENT_ID_4_HOLD, RF_KEY_EVENT_ID_4_LOOSE},      //
//     {RF_KEY_ID_5, RF_KEY_EVENT_ID_5_CLICK, RF_KEY_EVENT_ID_5_LONG, RF_KEY_EVENT_ID_5_HOLD, RF_KEY_EVENT_ID_5_LOOSE},      //
//     {RF_KEY_ID_6, RF_KEY_EVENT_ID_6_CLICK, RF_KEY_EVENT_ID_6_LONG, RF_KEY_EVENT_ID_6_HOLD, RF_KEY_EVENT_ID_6_LOOSE},      //
//     {RF_KEY_ID_7, RF_KEY_EVENT_ID_7_CLICK, RF_KEY_EVENT_ID_7_LONG, RF_KEY_EVENT_ID_7_HOLD, RF_KEY_EVENT_ID_7_LOOSE},      //
//     {RF_KEY_ID_8, RF_KEY_EVENT_ID_8_CLICK, RF_KEY_EVENT_ID_8_LONG, RF_KEY_EVENT_ID_8_HOLD, RF_KEY_EVENT_ID_8_LOOSE},      //
//     {RF_KEY_ID_9, RF_KEY_EVENT_ID_9_CLICK, RF_KEY_EVENT_ID_9_LONG, RF_KEY_EVENT_ID_9_HOLD, RF_KEY_EVENT_ID_9_LOOSE},      //
//     {RF_KEY_ID_10, RF_KEY_EVENT_ID_10_CLICK, RF_KEY_EVENT_ID_10_LONG, RF_KEY_EVENT_ID_10_HOLD, RF_KEY_EVENT_ID_10_LOOSE}, //
//     {RF_KEY_ID_11, RF_KEY_EVENT_ID_11_CLICK, RF_KEY_EVENT_ID_11_LONG, RF_KEY_EVENT_ID_11_HOLD, RF_KEY_EVENT_ID_11_LOOSE}, //
//     {RF_KEY_ID_12, RF_KEY_EVENT_ID_12_CLICK, RF_KEY_EVENT_ID_12_LONG, RF_KEY_EVENT_ID_12_HOLD, RF_KEY_EVENT_ID_12_LOOSE}, //

// #if USE_MY_TEST_433_REMOTE

//     {RF_KEY_ID_TEST_1, RF_KEY_EVENT_ID_TEST_1_CLICK, RF_KEY_EVENT_ID_TEST_1_LONG, RF_KEY_EVENT_ID_TEST_1_HOLD, RF_KEY_EVENT_ID_TEST_1_LOOSE}, //
//     {RF_KEY_ID_TEST_2, RF_KEY_EVENT_ID_TEST_2_CLICK, RF_KEY_EVENT_ID_TEST_2_LONG, RF_KEY_EVENT_ID_TEST_2_HOLD, RF_KEY_EVENT_ID_TEST_2_LOOSE}, //
//     {RF_KEY_ID_TEST_3, RF_KEY_EVENT_ID_TEST_3_CLICK, RF_KEY_EVENT_ID_TEST_3_LONG, RF_KEY_EVENT_ID_TEST_3_HOLD, RF_KEY_EVENT_ID_TEST_3_LOOSE}, //
//     {RF_KEY_ID_TEST_4, RF_KEY_EVENT_ID_TEST_4_CLICK, RF_KEY_EVENT_ID_TEST_4_LONG, RF_KEY_EVENT_ID_TEST_4_HOLD, RF_KEY_EVENT_ID_TEST_4_LOOSE}, //

// #endif // #if USE_MY_TEST_433_REMOTE
// };

// 控制一路PWM遥控器对应的表格
static const u8 rf_key_1_event_table[][RF_KEY_EFFECT_EVENT_NUMS + 1] = {
    {RF_KEY_ID_9, RF_KEY_EVENT_ID_9_CLICK, RF_KEY_EVENT_ID_9_LONG, RF_KEY_EVENT_ID_9_HOLD, RF_KEY_EVENT_ID_9_LOOSE},      //
    {RF_KEY_ID_10, RF_KEY_EVENT_ID_10_CLICK, RF_KEY_EVENT_ID_10_LONG, RF_KEY_EVENT_ID_10_HOLD, RF_KEY_EVENT_ID_10_LOOSE}, //
    {RF_KEY_ID_11, RF_KEY_EVENT_ID_11_CLICK, RF_KEY_EVENT_ID_11_LONG, RF_KEY_EVENT_ID_11_HOLD, RF_KEY_EVENT_ID_11_LOOSE}, //
    {RF_KEY_ID_12, RF_KEY_EVENT_ID_12_CLICK, RF_KEY_EVENT_ID_12_LONG, RF_KEY_EVENT_ID_12_HOLD, RF_KEY_EVENT_ID_12_LOOSE}, //
};

// 控制两路PWM遥控器对应的表格
static const u8 rf_key_1_2_event_table[][RF_KEY_EFFECT_EVENT_NUMS + 1] = {
    {RF_KEY_ID_1, RF_KEY_EVENT_ID_1_CLICK, RF_KEY_EVENT_ID_1_LONG, RF_KEY_EVENT_ID_1_HOLD, RF_KEY_EVENT_ID_1_LOOSE}, //
    {RF_KEY_ID_2, RF_KEY_EVENT_ID_2_CLICK, RF_KEY_EVENT_ID_2_LONG, RF_KEY_EVENT_ID_2_HOLD, RF_KEY_EVENT_ID_2_LOOSE}, //
    {RF_KEY_ID_3, RF_KEY_EVENT_ID_3_CLICK, RF_KEY_EVENT_ID_3_LONG, RF_KEY_EVENT_ID_3_HOLD, RF_KEY_EVENT_ID_3_LOOSE}, //
    {RF_KEY_ID_4, RF_KEY_EVENT_ID_4_CLICK, RF_KEY_EVENT_ID_4_LONG, RF_KEY_EVENT_ID_4_HOLD, RF_KEY_EVENT_ID_4_LOOSE}, //
    {RF_KEY_ID_5, RF_KEY_EVENT_ID_5_CLICK, RF_KEY_EVENT_ID_5_LONG, RF_KEY_EVENT_ID_5_HOLD, RF_KEY_EVENT_ID_5_LOOSE}, //
    {RF_KEY_ID_6, RF_KEY_EVENT_ID_6_CLICK, RF_KEY_EVENT_ID_6_LONG, RF_KEY_EVENT_ID_6_HOLD, RF_KEY_EVENT_ID_6_LOOSE}, //
    {RF_KEY_ID_7, RF_KEY_EVENT_ID_7_CLICK, RF_KEY_EVENT_ID_7_LONG, RF_KEY_EVENT_ID_7_HOLD, RF_KEY_EVENT_ID_7_LOOSE}, //
    {RF_KEY_ID_8, RF_KEY_EVENT_ID_8_CLICK, RF_KEY_EVENT_ID_8_LONG, RF_KEY_EVENT_ID_8_HOLD, RF_KEY_EVENT_ID_8_LOOSE}, //
};

extern u8 rf_key_get_key_id(void);
volatile struct key_driver_para rf_key_para = {
    // 编译器不支持指定成员赋值的写法，会报错：
    // .scan_times = 10,   // 扫描频率，单位：ms
    // .last_key = NO_KEY, // 上一次得到的按键键值，初始化为无效的键值
    // // .filter_value = NO_KEY, // 按键消抖期间得到的键值(在key_driver_scan()函数中使用)，初始化为 NO_KEY
    // // .filter_cnt = 0, // 按键消抖期间的累加值(在key_driver_scan()函数中使用)，初始化为0
    // .filter_time = 3,       // 按键消抖次数，与扫描频率有关
    // .long_time = 50,        // 判定按键是长按对应的数量，与扫描频率有关
    // .hold_time = (75 + 15), // 判定按键是HOLD对应的数量，与扫描频率有关
    // // .press_cnt = 0, // 与long_time和hold_time对比, 判断长按事件和HOLD事件
    // // .click_cnt = 0,
    // .click_delay_time = 20, // 按键抬起后，等待连击的数量，与扫描频率有关
    // // .notify_value = 0,
    // .key_type = KEY_TYPE_AD, // 按键类型为ad按键
    // .get_value = ad_key_get_key_id,

    // .latest_key_val = AD_KEY_ID_NONE,
    // .latest_key_event = KEY_EVENT_NONE,

    RF_KEY_SCAN_CIRCLE_TIMES, // .scan_times 扫描频率，单位：ms
    0,                        // .cur_scan_times 按键扫描频率, 单位ms，由1ms的定时器中断内累加，在key_driver_scan()中清零
    // NO_KEY,
    0, // .last_key

    0,                   // .filter_value
    0,                   // .filter_cnt
    RF_KEY_FILTER_TIMES, // .filter_time 按键消抖次数，与扫描频率有关（rf按键不消抖）

    RF_LONG_PRESS_TIME_THRESHOLD_MS / RF_KEY_SCAN_CIRCLE_TIMES,                                     // .long_time
    (RF_LONG_PRESS_TIME_THRESHOLD_MS + RF_HOLD_PRESS_TIME_THRESHOLD_MS) / RF_KEY_SCAN_CIRCLE_TIMES, // .hold_time
    0,                                                                                              // .press_cnt

    0,                              // .click_cnt
    0,                              // .click_delay_cnt
    200 / RF_KEY_SCAN_CIRCLE_TIMES, // .click_delay_time
    // NO_KEY,
    0,                 // .notify_value
    KEY_TYPE_RF,       // .key_type
    rf_key_get_key_id, // .get_value

    NO_KEY,         // .latest_key_val
    KEY_EVENT_NONE, // .latest_key_event
}; // volatile struct key_driver_para rf_key_para

/**
 * @brief 获取 rf 遥控器按键键值，供 key_driver 调用
 *          这里不能区分遥控器类型，只能获取遥控器按键
 *
 * @return u8
 */
static u8 rf_key_get_key_id(void)
{
    if (flag_is_recved_rf_data)
    {
        flag_is_recved_rf_data = 0;
        if (rf_data)
        {
            // u8 ret = (u8)rf_data;
            // u8 ret = ((u8)rf_data) & 0x0F; // 获取低4位作为键值
            u8 ret = ((u8)rf_data) & 0xFF; // 获取低8位作为键值

            // printf("rf_data 0x %lx\n", rf_data);

            if (flag_is_in_rf_learning)
            {
                // 如果在rf对码期间，直接获取地址
                // tmp_rf_addr = ((u32)rf_data >> 8);

                if (RF_KEY_ID_12 == (ret & 0x0F))
                {
                    tmp_rf_addr = ((u32)rf_data) >> 4; // 数据高20位是地址，低4位是键值
                    ret &= 0x0F;
                }
                else if (RF_KEY_ID_1 == ret)
                {
                    // 如果是控制两路PWM的遥控器在对码
                    tmp_rf_addr = ((u32)rf_data) >> 8; // 数据高16位是地址，低8位是键值
                }

                // printf("tmp_rf_addr 0x %lx\n", tmp_rf_addr);
            }
            else
            {
                // 如果不在rf对码期间，并且遥控器的地址不一致
                // if ((rf_data >> 8) != rf_remote_info.rf_addr)
                // if ((rf_data >> 4) != rf_remote_info.rf_addr) // 数据高20位是地址，低4位是键值

                if (rf_remote_info.is_addr_valid == 0xC5)
                {
                    // 如果之前存放了有效的遥控器的地址
                    // 判断遥控器类型：
                    if (1 == rf_remote_info.rf_remote_type &&
                        (rf_data >> 4) == rf_remote_info.rf_addr)
                    {
                        // 如果是控制一路PWM的遥控器，接收到的数据低4位是键值
                        ret &= 0x0F;
                    }
                    else if (2 == rf_remote_info.rf_remote_type &&
                             (rf_data >> 8) == rf_remote_info.rf_addr)
                    {
                        // 如果是控制两路PWM的遥控器，接收到的数据低8位是键值
                        // ret &= 0xFF; // 可以不写这一句
                    }
                    else
                    {
                        // 如果遥控器的地址不一样
                        ret = NO_KEY;
                    }
                }
                else
                {
                    // 如果之前没有存放有效的遥控器的地址
                    ret = NO_KEY;
                }
            }

            // printf("key id %bu\n", ret);
            rf_data = 0;    // 接收完成后，清除接收到的数据
            return (u8)ret; // 直接获取键值
        }
        else
        {
            return NO_KEY;
        }
    }
    else
    {
        return NO_KEY;
    }
}

/**
 * @brief __rf_key_get_event的子函数，将按键值和 key_driver_scan 得到的按键事件转换成触摸按键的事件
 *
 * @param key_val 触摸按键键值
 * @param key_event 在key_driver_scan得到的按键事件 KEY_EVENT
 * @param table_index 在哪个表格中进行查表，
 *                      1--在控制一路PWM遥控器对应的表格
 *                      2--控制两路PWM遥控器对应的表格
 * @return u8
 */
static u8 __sub_fun_rf_key_get_event(const u8 key_val, const u8 key_event, const u8 table_index)
{
    u8 ret_key_event = RF_KEY_EVENT_NONE;
    u8 i = 0;
    if (1 == table_index)
    {
        for (i = 0; i < ARRAY_SIZE(rf_key_1_event_table); i++)
        {
            // 如果往 KEY_EVENT 枚举中添加了新的按键事件，这里查表的方法就会失效，需要手动修改
            if (key_val == rf_key_1_event_table[i][0])
            {
                ret_key_event = rf_key_1_event_table[i][key_event];
                break;
            }
        }
    }
    else if (2 == table_index)
    {
        for (i = 0; i < ARRAY_SIZE(rf_key_1_2_event_table); i++)
        {
            // 如果往 KEY_EVENT 枚举中添加了新的按键事件，这里查表的方法就会失效，需要手动修改
            if (key_val == rf_key_1_2_event_table[i][0])
            {
                ret_key_event = rf_key_1_2_event_table[i][key_event];
                break;
            }
        }
    }
    else
    {
        // 传参有误
    }

    return ret_key_event;
}

/**
 * @brief 将按键值和 key_driver_scan 得到的按键事件转换成触摸按键的事件
 *          函数内部会做遥控器类型区分
 *
 * @param key_val 触摸按键键值
 * @param key_event 在key_driver_scan得到的按键事件 KEY_EVENT
 * @return u8 在 rf_key_event_table 中找到的对应的按键事件，如果没有则返回 RF_KEY_EVENT_NONE
 */
static u8 __rf_key_get_event(const u8 key_val, const u8 key_event)
{
    volatile u8 ret_key_event = RF_KEY_EVENT_NONE;
    u8 i = 0;

    if (flag_is_in_rf_learning)
    {
        // 如果在对码阶段
        /*
            先判断之前是否存放了有效的遥控器
                如果有，判断现在读到的遥控器按键是否与存放的遥控器信息一致
                    如果一致，进行查表
                    如果不一致，判断键值是不是对码按键
                        如果是1+2双路PWM遥控器的对码按键
                        如果是单路PWM遥控器的对码按键
                如果没有，判断键值是不是对码按键
                    如果是1+2双路PWM遥控器的对码按键
                    如果是单路PWM遥控器的对码按键
        */
        if (tmp_rf_addr == rf_remote_info.rf_addr && rf_remote_info.is_addr_valid == 0xC5)
        {
            // 之前存放了有效的遥控器，并且现在读到的遥控器的地址一样
            if (1 == rf_remote_info.rf_remote_type)
            {
                // 单路PWM遥控器
                ret_key_event = __sub_fun_rf_key_get_event(key_val, key_event, 1);
            }
            else if (2 == rf_remote_info.rf_remote_type)
            {
                // 1+2两路PWM遥控器
                ret_key_event = __sub_fun_rf_key_get_event(key_val, key_event, 2);
            }
        }
        else
        {
            // 现在读到的遥控器按键是否与存放的遥控器信息不一致，或者之前没有存放有效的遥控器
            // 开始判断是不是对码按键
            if (RF_KEY_ID_1 == key_val)
            {
                // 如果是1+2双路PWM遥控器的对码按键
                ret_key_event = __sub_fun_rf_key_get_event(key_val, key_event, 2);

                // printf("rf type 1+2\n");
            }
            else if (RF_KEY_ID_12 == key_val)
            {
                // 如果是单路PWM遥控器的对码按键
                ret_key_event = __sub_fun_rf_key_get_event(key_val, key_event, 1);
                // printf("rf type 1\n");
            }
            else
            {
                // ret_key_event = RF_KEY_EVENT_NONE;
            }
        }
    } // if (flag_is_in_rf_learning) // 如果在对码阶段
    else
    {
        // 如果不在对码阶段
        if (tmp_rf_addr == rf_remote_info.rf_addr && rf_remote_info.is_addr_valid == 0xC5)
        {
            // 之前存放了有效的遥控器，并且现在读到的遥控器的地址一样
            if (1 == rf_remote_info.rf_remote_type)
            {
                // 单路PWM遥控器
                ret_key_event = __sub_fun_rf_key_get_event(key_val, key_event, 1);
            }
            else if (2 == rf_remote_info.rf_remote_type)
            {
                // 1+2两路PWM遥控器
                ret_key_event = __sub_fun_rf_key_get_event(key_val, key_event, 2);
            }
        }
        else
        {
            // ret_key_event = RF_KEY_EVENT_NONE;
        }
    }

    return ret_key_event;
}

//  函数内部会做遥控器类型区分
void rf_key_handle(void)
{
    u8 rf_key_event = RF_KEY_EVENT_NONE;

    // 如果是无效的按键信息，函数直接返回
    if (rf_key_para.latest_key_val == NO_KEY)
    {
        return;
    }

    rf_key_event = __rf_key_get_event(rf_key_para.latest_key_val, rf_key_para.latest_key_event);
    rf_key_para.latest_key_val = NO_KEY;
    rf_key_para.latest_key_event = KEY_EVENT_NONE;

    { // 用于进行对码的代码块
        static u8 last_key_event = KEY_EVENT_NONE;

        if (1 == flag_is_in_rf_learning) // 处于rf对码期间，才进入
        {
            // 如果上一次检测到的按键【键值和事件】 与最新检测到的【键值和事件】相等，说明是同一个按键长按
            if (last_key_event == rf_key_event)
            {
                // 只有按下 开机/关机 按键，才进行对码
                if (RF_KEY_EVENT_ID_1_HOLD == rf_key_event ||
                    RF_KEY_EVENT_ID_12_HOLD == rf_key_event)
                {
                    // 如果一定要长按一段时间在对码，可以加上下面的语句：
                    // static u16 hold_cnt = 0; // 长按计数值，如果长按了一段时间，进行对码
                    // hold_cnt++;

                    // if (hold_cnt >= 1)
                    {
                        // hold_cnt = 0;
                        // 存放对码的地址
                        rf_remote_info.is_addr_valid = 0xC5; // 表示记录了一个有效的按键地址
                        rf_remote_info.rf_addr = tmp_rf_addr;
                        // 记录遥控器的类型：
                        if (RF_KEY_EVENT_ID_1_HOLD == rf_key_event)
                        {
                            // 2--1+2两路PWM遥控器
                            rf_remote_info.rf_remote_type = 2;
                        }
                        else // RF_KEY_EVENT_ID_12_HOLD == rf_key_event
                        {
                            // 1--单路PWM遥控器
                            rf_remote_info.rf_remote_type = 1;
                        }

                        flash_erase_sector(FLASH_START_ADDR);
                        flash_program(FLASH_START_ADDR, (u8 *)&rf_remote_info, sizeof(rf_remote_info));

#if USE_MY_DEBUG
                        // printf("rf learn\n");
                        // printf("rf_remote_type %bu\n", rf_remote_info.rf_remote_type);
                        // printf("learn addr: 0x %lx\n", rf_remote_info.rf_addr);
#endif
                        flag_is_in_rf_learning = 2; // 对码完成后，等待按键松手
                    }
                }
            }
            else
            {
                last_key_event = rf_key_event;
            }
        }
        else if (2 == flag_is_in_rf_learning) // 等待对码按键松手
        {
            if (RF_KEY_EVENT_ID_1_LOOSE == rf_key_event || /* 如果检测到控制两路PWM的遥控器的开关按键松手 */
                RF_KEY_EVENT_ID_12_LOOSE == rf_key_event)  /* 如果检测到控制一路PWM的遥控器的开关按键松手 */
            {
                flag_is_in_rf_learning = 0; // 退出对码/学习
                return;
            }
        }

    } // 用于进行对码的代码块

    // printf("rf event %bu\n", rf_key_event);

    // 如果按键地址无效，未进行对码，函数直接返回，不进行键值处理
    // 正在进行对码，不进行键值处理
    // if (0xC5 != rf_remote_info.is_addr_valid || flag_is_in_rf_learning)
    if (0xC5 != rf_remote_info.is_addr_valid) /* 如果之前未进行对码，不进行键值处理 */
    {
        return;
    }

    // 正在学习，但是接收到的按键地址与记录的地址一样:
    if (flag_is_in_rf_learning && rf_remote_info.rf_addr == tmp_rf_addr)
    {
        // 如果还在对码/学习过程中，退出对码/学习
        flag_is_in_rf_learning = 0;
    }
    // else
    // {
    //     // 如果按键无效(不处于对码期间，或是按键的地址与记录的地址不一样)
    //     return;
    // }

    switch (rf_key_event)
    {
    case RF_KEY_EVENT_ID_1_CLICK: // ON/OFF
    case RF_KEY_EVENT_ID_1_LOOSE: // 长按后松手也是 ON/OFF

        // printf("key 1 click\n");

        // 只要有一路开启，便认为灯光已经打开：
        if (get_pwm_channel_0_status() || get_pwm_channel_1_status())
        {
            expect_adjust_pwm_channel_0_duty = 0;
            expect_adjust_pwm_channel_1_duty = 0;
            // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(0);
            // adjust_pwm_channel_1_duty = get_pwm_channel_x_adjust_duty(0);
            // cur_pwm_channel_0_duty = adjust_pwm_channel_0_duty; // 更新当前的占空比对应的数值
            // cur_pwm_channel_1_duty = adjust_pwm_channel_1_duty; // 更新当前的占空比对应的数值
            // set_pwm_channel_0_duty(adjust_pwm_channel_0_duty);
            // set_pwm_channel_1_duty(adjust_pwm_channel_1_duty);

            // pwm_channel_0_disable();
            // pwm_channel_1_disable();

            // printf("all pwm channels are disable\n");
        }
        else
        {
            expect_adjust_pwm_channel_0_duty = MAX_PWM_DUTY;
            expect_adjust_pwm_channel_1_duty = MAX_PWM_DUTY;
            // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(MAX_PWM_DUTY);
            // adjust_pwm_channel_1_duty = get_pwm_channel_x_adjust_duty(MAX_PWM_DUTY);

            // cur_pwm_channel_0_duty = adjust_pwm_channel_0_duty; // 更新当前的占空比对应的数值
            // set_pwm_channel_0_duty(adjust_pwm_channel_0_duty);

            // cur_pwm_channel_1_duty = adjust_pwm_channel_1_duty; // 更新当前的占空比对应的数值
            // set_pwm_channel_1_duty(adjust_pwm_channel_1_duty);

            // pwm_channel_0_enable();
            // pwm_channel_1_enable();

            // printf("all pwm channels are enable\n");
        }

        break;

    case RF_KEY_EVENT_ID_2_CLICK: // 1+2按键，亮度设置为100%
    case RF_KEY_EVENT_ID_2_LOOSE: // 长按后松手也是 将 1+2 亮度设置为100%

        // printf("key 2 click\n");

        expect_adjust_pwm_channel_0_duty = MAX_PWM_DUTY;
        expect_adjust_pwm_channel_1_duty = MAX_PWM_DUTY;
        // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(PWM_DUTY_100_PERCENT);
        // adjust_pwm_channel_1_duty = get_pwm_channel_x_adjust_duty(PWM_DUTY_100_PERCENT);

        // cur_pwm_channel_0_duty = adjust_pwm_channel_0_duty; // 更新当前的占空比对应的数值
        // cur_pwm_channel_1_duty = adjust_pwm_channel_1_duty; // 更新当前的占空比对应的数值

        // set_pwm_channel_0_duty(adjust_pwm_channel_0_duty);
        // set_pwm_channel_1_duty(adjust_pwm_channel_1_duty);

        break;

    case RF_KEY_EVENT_ID_3_CLICK: // 增加 pwm_channel_0 duty

        expect_adjust_pwm_channel_0_duty += (PWM_DUTY_100_PERCENT * 5 / 100); // 每次调节5%
        if (expect_adjust_pwm_channel_0_duty > PWM_DUTY_100_PERCENT)
        {
            // 防止溢出
            expect_adjust_pwm_channel_0_duty = PWM_DUTY_100_PERCENT;
        }

        // 主函数会频繁调用该语句，可以优化掉：
        // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(expect_adjust_pwm_channel_0_duty);

        break;

    case RF_KEY_EVENT_ID_3_HOLD: // 增加 pwm_channel_0 duty

        expect_adjust_pwm_channel_0_duty += PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS);
        if (expect_adjust_pwm_channel_0_duty > PWM_DUTY_100_PERCENT)
        {
            // 防止溢出
            expect_adjust_pwm_channel_0_duty = PWM_DUTY_100_PERCENT;
        }

        break;

    case RF_KEY_EVENT_ID_4_CLICK: // 增加 pwm_channel_1 duty

        // printf("key 4 click\n");
        expect_adjust_pwm_channel_1_duty += (PWM_DUTY_100_PERCENT * 5 / 100); // 每次调节5%
        if (expect_adjust_pwm_channel_1_duty > PWM_DUTY_100_PERCENT)
        {
            // 防止溢出
            expect_adjust_pwm_channel_1_duty = PWM_DUTY_100_PERCENT;
        }

        // 主函数会频繁调用该语句，可以优化掉：
        // adjust_pwm_channel_1_duty = get_pwm_channel_x_adjust_duty(expect_adjust_pwm_channel_1_duty);

        break;

    case RF_KEY_EVENT_ID_4_HOLD: // 增加 pwm_channel_1 duty

        expect_adjust_pwm_channel_1_duty += PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS);
        if (expect_adjust_pwm_channel_1_duty > PWM_DUTY_100_PERCENT)
        {
            // 防止溢出
            expect_adjust_pwm_channel_1_duty = PWM_DUTY_100_PERCENT;
        }

        break;

    case RF_KEY_EVENT_ID_5_CLICK: // set pwm_channel_0 50%
    case RF_KEY_EVENT_ID_5_LOOSE: // 长按后松手也是将 pwm_channel_0 设置为50%

        // printf("key 5 click\n");

        expect_adjust_pwm_channel_0_duty = PWM_DUTY_50_PERCENT;
        // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(PWM_DUTY_50_PERCENT);
        // cur_pwm_channel_0_duty = adjust_pwm_channel_0_duty; // 更新当前的占空比对应的数值
        // set_pwm_channel_0_duty(adjust_pwm_channel_0_duty);

        break;

    case RF_KEY_EVENT_ID_6_CLICK: // set pwm_channel_1 50%
    case RF_KEY_EVENT_ID_6_LOOSE: // 长按后松手也是将 pwm_channel_1 设置为50%

        // printf("key 6 click\n");

        expect_adjust_pwm_channel_1_duty = PWM_DUTY_50_PERCENT;
        // adjust_pwm_channel_1_duty = get_pwm_channel_x_adjust_duty(PWM_DUTY_50_PERCENT);
        // cur_pwm_channel_1_duty = adjust_pwm_channel_1_duty; // 更新当前的占空比对应的数值
        // set_pwm_channel_1_duty(adjust_pwm_channel_1_duty);

        break;

    case RF_KEY_EVENT_ID_7_CLICK: // 减小 pwm_channel_0 duty

        // printf("key 7 click\n");

        if (expect_adjust_pwm_channel_0_duty >= (PWM_DUTY_100_PERCENT * 5 / 100)) // 如果当前pwm占空比大于最大占空比的5%
        {
            // expect_adjust_pwm_channel_0_duty -= (limited_max_pwm_duty * 5 / 100); // 每次调节5%（以旋钮限制的占空比为100%）
            expect_adjust_pwm_channel_0_duty -= (PWM_DUTY_100_PERCENT * 5 / 100); // 每次调节5%
            if (expect_adjust_pwm_channel_0_duty < (PWM_DUTY_100_PERCENT * 5 / 100))
            {
                expect_adjust_pwm_channel_0_duty = 0;
            }
        }
        else
        {
            // 如果  expect_adjust_pwm_channel_0_duty 已经小于 最大占空比的5% (PWM_DUTY_100_PERCENT * 5 / 100)
            expect_adjust_pwm_channel_0_duty = 0;
        }

        // 主函数会频繁调用该语句，可以优化掉：
        // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(expect_adjust_pwm_channel_0_duty);

        break;

    case RF_KEY_EVENT_ID_7_HOLD: // 减小 pwm_channel_0 duty

        if (expect_adjust_pwm_channel_0_duty >= (PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS)))
        {
            expect_adjust_pwm_channel_0_duty -= PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS);

            if (expect_adjust_pwm_channel_0_duty < (PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS)))
            {
                expect_adjust_pwm_channel_0_duty = 0;
            }
        }
        else
        {
            expect_adjust_pwm_channel_0_duty = 0;
        }

        break;

    case RF_KEY_EVENT_ID_8_CLICK: // 减小 pwm_channel_1 duty

        // printf("key 8 click\n");

        if (expect_adjust_pwm_channel_1_duty >= (PWM_DUTY_100_PERCENT * 5 / 100)) // 如果当前pwm占空比大于最大占空比的5%
        {
            expect_adjust_pwm_channel_1_duty -= (PWM_DUTY_100_PERCENT * 5 / 100); // 每次调节5%
            if (expect_adjust_pwm_channel_1_duty < (PWM_DUTY_100_PERCENT * 5 / 100))
            {
                expect_adjust_pwm_channel_1_duty = 0;
            }
        }
        else
        {
            // 如果  expect_adjust_pwm_channel_1_duty 已经小于 最大占空比的5% (PWM_DUTY_100_PERCENT * 5 / 100)
            expect_adjust_pwm_channel_1_duty = 0;
        }

        // 主函数会频繁调用该语句，可以优化掉：
        // adjust_pwm_channel_1_duty = get_pwm_channel_x_adjust_duty(expect_adjust_pwm_channel_1_duty);

        break;

    case RF_KEY_EVENT_ID_8_HOLD: // 减小 pwm_channel_1 duty

        if (expect_adjust_pwm_channel_1_duty >= (PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS)))
        {
            expect_adjust_pwm_channel_1_duty -= PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS);

            if (expect_adjust_pwm_channel_1_duty < (PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS)))
            {
                expect_adjust_pwm_channel_1_duty = 0;
            }
        }
        else
        {
            expect_adjust_pwm_channel_1_duty = 0;
        }

        break;

    // ===========================================================================
    // 只控制一路PWM的遥控器，在软件上是两路一起控制
    case RF_KEY_EVENT_ID_9_CLICK: // 加大 pwm_channel_0 占空比； 加大 pwm_channel_1 占空比

        // printf("key 9 click\n");

        expect_adjust_pwm_channel_0_duty += (PWM_DUTY_100_PERCENT * 5 / 100); // 每次调节5%
        if (expect_adjust_pwm_channel_0_duty > PWM_DUTY_100_PERCENT)
        {
            // 防止溢出
            expect_adjust_pwm_channel_0_duty = PWM_DUTY_100_PERCENT;
        }

        expect_adjust_pwm_channel_1_duty += (PWM_DUTY_100_PERCENT * 5 / 100); // 每次调节5%
        if (expect_adjust_pwm_channel_1_duty > PWM_DUTY_100_PERCENT)
        {
            // 防止溢出
            expect_adjust_pwm_channel_1_duty = PWM_DUTY_100_PERCENT;
        }

        // 主函数会频繁调用该语句，可以优化掉：
        // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(expect_adjust_pwm_channel_0_duty);

        break;

    case RF_KEY_EVENT_ID_9_HOLD: // 加大 pwm_channel_0 占空比； 加大 pwm_channel_1 占空比

        // printf("key id 9 hold\n");

        /*
            示例
            长按要实现无极调节，每次检测到HOLD的时间间隔为150ms，灯光亮度范围0~6000，调节时间3s
            每次检测到HOLD的时间间隔为 50ms ，灯光亮度范围 0~6000，调节时间3s，那么每次HOLD调节 1.6%
        */

        // expect_adjust_pwm_channel_0_duty += (PWM_DUTY_100_PERCENT * 5 / 100); // 每次 HOLD 调节5%
        expect_adjust_pwm_channel_0_duty += PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS);
        if (expect_adjust_pwm_channel_0_duty > PWM_DUTY_100_PERCENT)
        {
            // 防止溢出
            expect_adjust_pwm_channel_0_duty = PWM_DUTY_100_PERCENT;
        }

        expect_adjust_pwm_channel_1_duty += PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS);
        if (expect_adjust_pwm_channel_1_duty > PWM_DUTY_100_PERCENT)
        {
            // 防止溢出
            expect_adjust_pwm_channel_1_duty = PWM_DUTY_100_PERCENT;
        }

        break;

    case RF_KEY_EVENT_ID_10_CLICK: // 设置 pwm_channel_0 占空比为50%； 设置 pwm_channel_1 占空比为50%
    case RF_KEY_EVENT_ID_10_LOOSE: // 长按后松手，也是 设置 pwm_channel_0 占空比为50%；设置 pwm_channel_1 占空比为50%

        expect_adjust_pwm_channel_0_duty = PWM_DUTY_50_PERCENT;
        expect_adjust_pwm_channel_1_duty = PWM_DUTY_50_PERCENT;
        // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(PWM_DUTY_50_PERCENT);
        // cur_pwm_channel_0_duty = adjust_pwm_channel_0_duty; // 更新当前的占空比对应的数值
        // set_pwm_channel_0_duty(adjust_pwm_channel_0_duty);

        break;

    case RF_KEY_EVENT_ID_11_CLICK: // 减小 pwm_channel_0 占空比；减小 pwm_channel_1 占空比

        // printf("key 11 click\n");

        if (expect_adjust_pwm_channel_0_duty >= (PWM_DUTY_100_PERCENT * 5 / 100)) // 如果当前pwm占空比大于最大占空比的5%
        {
            // expect_adjust_pwm_channel_0_duty -= (limited_max_pwm_duty * 5 / 100); // 每次调节5%（以旋钮限制的占空比为100%）
            expect_adjust_pwm_channel_0_duty -= (PWM_DUTY_100_PERCENT * 5 / 100); // 每次调节5%
            if (expect_adjust_pwm_channel_0_duty < (PWM_DUTY_100_PERCENT * 5 / 100))
            {
                expect_adjust_pwm_channel_0_duty = 0;
            }
        }
        else
        {
            // 如果  expect_adjust_pwm_channel_0_duty 已经小于 最大占空比的5% (PWM_DUTY_100_PERCENT * 5 / 100)
            expect_adjust_pwm_channel_0_duty = 0;
        }

        if (expect_adjust_pwm_channel_1_duty >= (PWM_DUTY_100_PERCENT * 5 / 100)) // 如果当前pwm占空比大于最大占空比的5%
        {
            // expect_adjust_pwm_channel_1_duty -= (limited_max_pwm_duty * 5 / 100); // 每次调节5%（以旋钮限制的占空比为100%）
            expect_adjust_pwm_channel_1_duty -= (PWM_DUTY_100_PERCENT * 5 / 100); // 每次调节5%
            if (expect_adjust_pwm_channel_1_duty < (PWM_DUTY_100_PERCENT * 5 / 100))
            {
                expect_adjust_pwm_channel_1_duty = 0;
            }
        }
        else
        {
            // 如果  expect_adjust_pwm_channel_1_duty 已经小于 最大占空比的5% (PWM_DUTY_100_PERCENT * 5 / 100)
            expect_adjust_pwm_channel_1_duty = 0;
        }

        // 主函数会频繁调用该语句，可以优化掉：
        // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(expect_adjust_pwm_channel_0_duty);

        break;

    case RF_KEY_EVENT_ID_11_HOLD: // 减小 pwm_channel_0 占空比；减小 pwm_channel_0 占空比

        // printf("key id 11 hold\n");

        if (expect_adjust_pwm_channel_0_duty >= (PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS)))
        {
            expect_adjust_pwm_channel_0_duty -= PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS);

            if (expect_adjust_pwm_channel_0_duty < (PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS)))
            {
                expect_adjust_pwm_channel_0_duty = 0;
            }
        }
        else
        {
            expect_adjust_pwm_channel_0_duty = 0;
        }

        if (expect_adjust_pwm_channel_1_duty >= (PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS)))
        {
            expect_adjust_pwm_channel_1_duty -= PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS);

            if (expect_adjust_pwm_channel_1_duty < (PWM_DUTY_100_PERCENT / (RF_ADJUST_TOTAL_TIMES_FOR_HOLD / RF_HOLD_PRESS_TIME_THRESHOLD_MS)))
            {
                expect_adjust_pwm_channel_1_duty = 0;
            }
        }
        else
        {
            expect_adjust_pwm_channel_1_duty = 0;
        }

        break;

    case RF_KEY_EVENT_ID_12_CLICK: // 控制 pwm_channel_0 开关的按键
    case RF_KEY_EVENT_ID_12_LOOSE: // 长按后松手，也是 控制 pwm_channel_0 开关

        // printf("key 12 click\n");
        if (get_pwm_channel_0_status() || get_pwm_channel_1_status()) // 如果PWM已经使能
        {
            expect_adjust_pwm_channel_0_duty = 0;
            expect_adjust_pwm_channel_1_duty = 0;
            // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(0);
            // cur_pwm_channel_0_duty = adjust_pwm_channel_0_duty; // 更新当前的占空比对应的数值
            // set_pwm_channel_0_duty(adjust_pwm_channel_0_duty);
            // pwm_channel_0_disable();
            // printf("pwm channel 0 is disable\n");
        }
        else // 如果PWM未使能
        {
            expect_adjust_pwm_channel_0_duty = MAX_PWM_DUTY;
            expect_adjust_pwm_channel_1_duty = MAX_PWM_DUTY;
            // adjust_pwm_channel_0_duty = get_pwm_channel_x_adjust_duty(MAX_PWM_DUTY);
            // cur_pwm_channel_0_duty = adjust_pwm_channel_0_duty; // 更新当前的占空比对应的数值
            // set_pwm_channel_0_duty(adjust_pwm_channel_0_duty);
            // pwm_channel_0_enable();
            // printf("pwm channel 0 is enable\n");
        }

        break;

#if USE_MY_TEST_433_REMOTE // 测试时使用的遥控器按键和功能，实际不使用

    case RF_KEY_EVENT_ID_TEST_1_CLICK:

        // printf("test 1 click\n");
        if (limited_max_pwm_duty <= (MAX_PWM_DUTY - 500))
        {
            limited_max_pwm_duty += 500;
        }

        break;

    case RF_KEY_EVENT_ID_TEST_2_CLICK:

        // printf("test 2 click\n");

        if (limited_max_pwm_duty >= (0 + 500))
        {
            limited_max_pwm_duty -= 500;
        }

        break;

    case RF_KEY_EVENT_ID_TEST_3_CLICK:

        // printf("test 3 click\n");

        if (limited_pwm_duty_due_to_temp <= (MAX_PWM_DUTY - 500))
        {
            limited_pwm_duty_due_to_temp += 500;
        }

        break;

    case RF_KEY_EVENT_ID_TEST_4_CLICK:

        // printf("test 4 click\n");

        if (limited_pwm_duty_due_to_temp >= (0 + 500))
        {
            limited_pwm_duty_due_to_temp -= 500;
        }

        break;

#endif // #if USE_MY_TEST_433_REMOTE

    default:
        break;
    }
}

void rf_recv_init(void)
{
// MY_DEBUG:
#if USE_MY_TEST_PIN // 测试时使用，在开发板上使用 P01 脚，用于检测rf信号

    P0_PU |= GPIO_P01_PULL_UP(0x01);      // 上拉
    P0_MD0 &= ~(GPIO_P01_MODE_SEL(0x03)); // 输入模式

#else // 实际用到的、非测试时使用的rf信号检测引脚：

    P0_PU |= GPIO_P03_PULL_UP(0x01);      // 上拉
    P0_MD0 &= ~(GPIO_P03_MODE_SEL(0x03)); // 输入模式

#endif // #if USE_MY_TEST_PIN

    // 检测有无433遥控器功能的引脚：
    P1_PU |= GPIO_P11_PULL_UP(0x01);      // 上拉
    P1_MD0 &= ~(GPIO_P11_MODE_SEL(0x03)); // 输入模式

    if (0 == RF_ENABLE_PIN) // 检测脚接了0R电阻到GND，说明有433遥控器的功能
    {
        flag_is_rf_enable = 1;
    }
    else // 检测脚未接0R电阻，说明没有433遥控器的功能
    {
        flag_is_rf_enable = 0;
    }

    // MY_DEBUG:
    // flag_is_rf_enable = 1; // 测试时使用（使能433遥控的功能）
    // flag_is_rf_enable = 0; // 测试时使用

    if (flag_is_rf_enable)
    {
        flash_read(FLASH_START_ADDR, (u8 *)&rf_remote_info, sizeof(rf_remote_info));

#if USE_MY_DEBUG
        // if (0xC5 == rf_remote_info.is_addr_valid)
        // {
        //     printf("rf addr valid\n");
        // }
        // else
        // {
        //     printf("rf addr unvalid\n");
        // }

        // printf("rf addr: 0x %lx\n", rf_remote_info.rf_addr);
#endif

        flag_is_in_rf_learning = 1; // 上电后，使能对码功能
    }
}
