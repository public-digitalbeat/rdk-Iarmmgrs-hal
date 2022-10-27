#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>

#include "plat_power.h"

#define FILE_NAME_SIZE   64
#define SIZE 10


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




static IARM_Bus_PWRMgr_PowerState_t power_state;

/**
 * @brief Initialize the underlying Power Management module.
 *
 * This function must initialize all aspects of the CPE's Power Management module.
 *
 * @param None.
 * @return    Return Code.
 * @retval    0 if successful.
 */
int PLAT_INIT(void)
{
    printf("Inside Power Hal %s :%d\n",__FUNCTION__,__LINE__);
    power_state = IARM_BUS_PWRMGR_POWERSTATE_ON;
    printf("Power Hal PLAT_INIT power_state %d\n",power_state);
    return 0;
}

/**
 * @brief Set the CPE Power State.
 *
 * This function sets the CPE's current power state to the specified state.
 *
 * @param [in]  newState    The power state to which to set the CPE.
 * @return    Return Code.
 * @retval    0 if successful.
 */
int PLAT_API_SetPowerState(IARM_Bus_PWRMgr_PowerState_t newState)
{
	printf("Inside Power Hal %s :%d\n",__FUNCTION__,__LINE__);
	/* TODO: Add standby mode */
	char sysfs[FILE_NAME_SIZE];
	const char SYS_POWER_WAKE_LOCK[] = "/sys/power/wake_lock";
	const char SYS_POWER_WAKE_UNLOCK[] = "/sys/power/wake_unlock";
	const char wake_lock_name[] = "tdk-test";

	printf("newState:::::::::::PLAT_API_SetPowerState in hal:%d\n",newState);
	sprintf(sysfs,"%s","/sys/power/state");
	switch(newState){
		case IARM_BUS_PWRMGR_POWERSTATE_OFF:
			printf("Inside case poweroff\n");
			system("systemctl stop wifi.service");
			amsysfs_set_sysfs_str(sysfs,"mem");
			power_state = IARM_BUS_PWRMGR_POWERSTATE_OFF;
			break;
		case IARM_BUS_PWRMGR_POWERSTATE_STANDBY:
			printf("Inside case standby\n");
			power_state = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
			break;
                case IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP:
                        printf("Inside case lightsleep\n");
                        power_state = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP;
                        break;
		case IARM_BUS_PWRMGR_POWERSTATE_ON:
			printf("Inside case poweron\n");
			amsysfs_set_sysfs_str(sysfs,"wake");
			/* relase wake lock */
			amsysfs_set_sysfs_str(SYS_POWER_WAKE_UNLOCK, wake_lock_name);
			system("systemctl start wifi.service");
			power_state = IARM_BUS_PWRMGR_POWERSTATE_ON;
			break;
		default:
			break;
	}

	return 0;
}

/**
 * @brief Get the CPE Power State.
 *
 * This function returns the current power state of the CPE.
 *
 * @param [in]  curState    The address of a location to hold the current power state of
 *                          the CPE on return.
 * @return    Return Code.
 * @retval    0 if successful.
 */
int PLAT_API_GetPowerState(IARM_Bus_PWRMgr_PowerState_t *curState)
{
	printf("Inside Power Hal %s :%d\n",__FUNCTION__,__LINE__);

	if(curState != NULL)
	{
		printf("Power Hal Get  power_state %d\n",power_state);
		*curState = power_state;
		printf("curstate in Get power hal:::%d\n",*curState);

	}
	return 0;
}

/**
 * @brief This API resets the power state of the device.
 *
 * @param[in] newState The state to be set.
 * The input paramter is not in use.
 */
void PLAT_Reset(IARM_Bus_PWRMgr_PowerState_t newState)
{
	// TODO: Should we check the newState?
	system("sleep 5; /lib/rdk/rebootNow.sh -s PowerMgr_Plat -o "
			"'Rebooting the box from PLAT_Reset()...'");
}

/**
 * @brief Close the IR device module.
 *
 * This function must terminate the CPE Power Management module. It must reset any data
 * structures used within Power Management module and release any Power Management
 * specific handles and resources.
 *
 * @param None.
 * @return None.
 */
void PLAT_TERM(void)
{
    return;
}
