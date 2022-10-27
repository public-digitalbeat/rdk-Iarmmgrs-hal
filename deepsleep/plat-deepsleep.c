#include <stdio.h>
#include <stdlib.h>
#include "plat_power.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "irMgrInternal.h"
#include "libIARM.h"
#include "deepSleepMgr.h"

#define FILE_NAME_SIZE   64
#define LOGE printf

/* Platform wake up reason   refer to aml_comp/kernel/am_4.4/include/linux/pm.h  wake up reason */
typedef enum _plat_wakeup_reason {
    UDEFINED_WAKEUP = 0,
    CHARGING_WAKEUP = 1,
    REMOTE_WAKEUP   = 2,
    RTC_WAKEUP      = 3,
    BT_WAKEUP       = 4,
    WIFI_WAKEUP     = 5,
    POWER_KEY_WAKEUP = 6,
    AUTO_WAKEUP      = 7,
    CEC_WAKEUP       = 8,
    REMOTE_CUS_WAKEUP  = 9,
    ETH_PMT_WAKUP      = 10,
    CECB_WAKEUP        = 11,
    ETH_PHY_GPIO       = 12,
    VAD_WAKEUP         = 13,
    MAX_WAKEUP
} plat_wakeup_reason;

/* RDK Wakeup Reason  */
typedef enum _DeepSleep_WakeupReason_t
{
  DEEPSLEEP_WAKEUPREASON_IR = 0,
  DEEPSLEEP_WAKEUPREASON_RCU_BT,
  DEEPSLEEP_WAKEUPREASON_RCU_RF4CE,
  DEEPSLEEP_WAKEUPREASON_GPIO,
  DEEPSLEEP_WAKEUPREASON_LAN,
  DEEPSLEEP_WAKEUPREASON_WLAN,
  DEEPSLEEP_WAKEUPREASON_TIMER,
  DEEPSLEEP_WAKEUPREASON_FRONT_PANEL,
  DEEPSLEEP_WAKEUPREASON_WATCHDOG,
  DEEPSLEEP_WAKEUPREASON_SOFTWARE_RESET,
  DEEPSLEEP_WAKEUPREASON_THERMAL_RESET,
  DEEPSLEEP_WAKEUPREASON_WARM_RESET,
  DEEPSLEEP_WAKEUPREASON_COLDBOOT,
  DEEPSLEEP_WAKEUPREASON_STR_AUTH_FAILURE,
  DEEPSLEEP_WAKEUPREASON_CEC,
  DEEPSLEEP_WAKEUPREASON_PRESENCE,
  DEEPSLEEP_WAKEUPREASON_VOICE,
  DEEPSLEEP_WAKEUPREASON_UNKNOWN
}DeepSleep_WakeupReason_t;

typedef struct _IARM_Bus_DeepSleepMgr_WakeupKeyCode_Param_t {
               unsigned int keyCode;
} IARM_Bus_DeepSleepMgr_WakeupKeyCode_Param_t;

int PLAT_DS_GetWakeupSources(void);
int PLAT_DS_SetNetworkStandby(uint32_t deep_sleep_timeout, bool *isGPIOWakeup, bool  isNetworkStandby);

bool g_IsNetworkStandby_modeset = false;

int PLAT_DS_INIT(void)
{
    printf("Inside DeepSleep  Hal %s :%d\n",__FUNCTION__,__LINE__);

    LOG("INIT DONE\r\n");
    return 0;
}

static int amsysfs_get_sysfs_int(const char *path)
{
    int fd;
    int val = 0;
    char  bcmd[16];
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, bcmd, sizeof(bcmd));
        val = strtol(bcmd, NULL, 10);
        close(fd);
    } else {
        LOGE("unable to open file %s,err: %s\n", path, strerror(errno));
    }
    return val;
}

static int amsysfs_set_sysfs_str(const char *path, const char *val)
{
    int fd;
    int bytes;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        bytes = write(fd, val, strlen(val));
        close(fd);
        return 0;
    } else {
        LOGE("unable to open file %s,err: %s\n", path, strerror(errno));
    }
    return -1;
}

static int amsysfs_set_sysfs_int(const char *path, int val)
{
    int fd;
    int bytes;
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", val);
        bytes = write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    } else {
        LOGE("unable to open file %s,err: %s\n", path, strerror(errno));
    }
    return -1;
}

/**
 * @brief Function to put the system to Deepsleep Mode.
 *
 * This function will internally call the Linux STR CAll and will be waiting in the Deepsleep State.
 * This function only return when the system actually resumes from STR state
 *
 * @param deep_sleep_timeout- Timeout value after which system should wake from deepsleep.
 * @return 0 IF it is Not user triggered wakeup.
 *         1 If the Resume is due to User triggered wakeup
 */
int PLAT_DS_SetDeepSleep(uint32_t deep_sleep_timeout, bool *isGPIOWakeup)
{
    printf("PLAT_DS_DeepSleepWakeup: putting to sleep...\n");
    printf("Inside Power Hal %s :%d deep_sleep_timeout is %d\n",__FUNCTION__,__LINE__,deep_sleep_timeout);
    return PLAT_DS_SetNetworkStandby(deep_sleep_timeout, isGPIOWakeup, false);
}

int PLAT_DS_SetNetworkStandby(uint32_t deep_sleep_timeout, bool *isGPIOWakeup, bool  isNetworkStandby)
{
    printf("PLAT_DS_DeepSleepWakeup: putting to sleep..\n");
    printf("Inside Power Hal %s :%d deep_sleep_timeout is %d\n",__FUNCTION__,__LINE__,deep_sleep_timeout);
    int wakeup_reason=0;
    *isGPIOWakeup = 0;
    const char SYS_POWER_EARLY_SUSPEND_TRIGGER[] = "/sys/class/meson_pm/early_suspend_trigger";
    const char SYS_POWER_TIMER[] = "/sys/class/meson_pm/time_out";
    const char sysfs[] = "/sys/power/state";
    const char SYS_POWER_WAKE_CAUSE[] = "/sys/class/meson_pm/suspend_reason";

    amsysfs_set_sysfs_str("/proc/sys/vm/drop_caches", "3");
    printf("cleared page cache. \n");

    if (deep_sleep_timeout != 0)
    {
        amsysfs_set_sysfs_int(SYS_POWER_TIMER, deep_sleep_timeout);
        if(amsysfs_get_sysfs_int(SYS_POWER_TIMER) != deep_sleep_timeout)
        {
            printf("strange value in %s: %d. \n", SYS_POWER_TIMER, amsysfs_get_sysfs_int(SYS_POWER_TIMER));
        }
        else
        {
            printf("timer set to %d\n", deep_sleep_timeout);
        }
    }

    //amsysfs_set_sysfs_str(SYS_POWER_EARLY_SUSPEND_TRIGGER, "1");
    amsysfs_set_sysfs_str(sysfs,"mem");

    printf("Waking up from STR mode\n");

    wakeup_reason = amsysfs_get_sysfs_int(SYS_POWER_WAKE_CAUSE);
    if(wakeup_reason <0)
    {
        printf("In %s at %d  Get wakeup reason failed \n",__FUNCTION__,__LINE__);
        return -1;
    }
    printf("In %s at %d Wakeup reason is %x\n",__FUNCTION__,__LINE__,wakeup_reason);

    if(wakeup_reason != RTC_WAKEUP)
    {
        printf("DeepSleep: wakeup due to user action!\n");
        *isGPIOWakeup = 1;
    }
    else {
        printf("DeepSleep: wakeup due to non-user event!\n");
    }
    return 0;

}

void PLAT_DS_DeepSleepWakeup(void)
{
    printf("PLAT_DS_DeepSleepWakeup: Waking up from Deepsleep mode \n");
    printf("Inside DeepSleep  Hal %s :%d\n",__FUNCTION__,__LINE__);
    char sysfs[FILE_NAME_SIZE];
    uint32_t deep_sleep_timeout = 0;
    const char SYS_POWER_TIMER[] = "/sys/class/meson_pm/time_out";
    if (g_IsNetworkStandby_modeset) g_IsNetworkStandby_modeset = false;
    //Clearing the Deepsleep timeout set after Resuming from DeepSleep
    amsysfs_set_sysfs_int(SYS_POWER_TIMER, deep_sleep_timeout);
   
    const char SYS_POWER_EARLY_SUSPEND_TRIGGER[] = "/sys/class/meson_pm/early_suspend_trigger";
    amsysfs_set_sysfs_str(SYS_POWER_EARLY_SUSPEND_TRIGGER, "0");

}

int PLAT_DS_GetLastWakeupKeyCode(IARM_Bus_DeepSleepMgr_WakeupKeyCode_Param_t *wakeupKeyCode)
{
    printf("%s : Enter\n",__FUNCTION__);
    return -1;
}

int PLAT_DS_GetLastWakeupReason(DeepSleep_WakeupReason_t *wakeupReason)
{

    printf("%s : Enter\n",__FUNCTION__);
    int wakeup_reason, result =0;
    const char SYS_POWER_WAKE_CAUSE[] = "/sys/class/meson_pm/suspend_reason";

    if (wakeupReason != NULL) {
        printf("Fetching wakeup reason from %s : \n", SYS_POWER_WAKE_CAUSE);
        wakeup_reason = amsysfs_get_sysfs_int(SYS_POWER_WAKE_CAUSE);
        if (!wakeup_reason)
        {
           printf("Platform Reset Reason not implemented");
        }
        else
        {
            printf("Platform Wakeup Reason is :%d \n",wakeup_reason);
            switch(wakeup_reason)
            {
                case REMOTE_WAKEUP:
                    *wakeupReason = DEEPSLEEP_WAKEUPREASON_IR;
                    break;

                //case GPIO_WAKEUP:
                 //   *wakeupReason = DEEPSLEEP_WAKEUPREASON_GPIO;
                  //  break;

                case RTC_WAKEUP:
                    *wakeupReason = DEEPSLEEP_WAKEUPREASON_TIMER;
                    break;

                case BT_WAKEUP:
                    *wakeupReason = DEEPSLEEP_WAKEUPREASON_RCU_BT;
                    break;

                case WIFI_WAKEUP:
                    *wakeupReason = DEEPSLEEP_WAKEUPREASON_WLAN;
                    break;

                //case LAN_WAKEUP:
                //    *wakeupReason = DEEPSLEEP_WAKEUPREASON_LAN;
                //    break;
                case ETH_PHY_GPIO:
                     *wakeupReason = DEEPSLEEP_WAKEUPREASON_LAN;
                     break;


                case POWER_KEY_WAKEUP:
                    *wakeupReason = DEEPSLEEP_WAKEUPREASON_FRONT_PANEL;
                    break;

                case CEC_WAKEUP:
                    *wakeupReason = DEEPSLEEP_WAKEUPREASON_CEC;
                    break;

                default:
                    *wakeupReason = DEEPSLEEP_WAKEUPREASON_UNKNOWN;
            }
        }
        printf("Wakeup reason value:-[%d]\n",*wakeupReason);
    } else {
        printf("Function argument NULL\n");
        result = -1;
    }

    return result;
}

void PLAT_DS_TERM(void)
{
//Stub

}

