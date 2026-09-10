/*
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#define LOG_TAG "android.hardware.health-service.qti"

#include <android-base/logging.h>
#include <android/binder_interface_utils.h>
#include <health/utils.h>
#include <health-impl/ChargerUtils.h>
#include <health-impl/Health.h>
#include <cutils/klog.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using aidl::android::hardware::health::HalHealthLoop;
using aidl::android::hardware::health::Health;
using aidl::android::hardware::health::HealthInfo;

#if !CHARGER_FORCE_NO_UI
using aidl::android::hardware::health::charger::ChargerCallback;
using aidl::android::hardware::health::charger::ChargerModeMain;
namespace aidl::android::hardware::health {
class ChargerCallbackImpl : public ChargerCallback {
  public:
    ChargerCallbackImpl(const std::shared_ptr<Health>& service) : ChargerCallback(service) {}
    bool ChargerEnableSuspend() override { return true; }
};
} //namespace aidl::android::hardware::health
#endif

static constexpr const char* gInstanceName = "default";
static constexpr std::string_view gChargerArg{"--charger"};

constexpr char *ucsiPSYName[]{
	(char *const)"ucsi-source-psy-soc:qcom,pmic_glink:qcom,ucsi1",
	(char *const)"ucsi-source-psy-soc:qcom,pmic_glink:qcom,ucsi2"
};

#define RETRY_COUNT    100

#define HEALTHD_TAG                 "healthd_msm"

#define BMS_READY_PATH              "/sys/class/power_supply/bms/soc_reporting_ready"
#define BMS_NOTIFY_READY_PATH       "/sys/class/power_supply/bms/soc_notify_ready"
#define BMS_BATT_INFO_PATH          "/sys/class/power_supply/bms/battery_info"
#define BMS_BATT_INFO_ID_PATH       "/sys/class/power_supply/bms/battery_info_id"
#define BMS_BATT_RES_ID_PATH        "/sys/class/power_supply/bms/resistance_id"
#define PERSIST_BATT_INFO_PATH      "/mnt/vendor/persist/bms/batt_info.txt"

#define WAIT_BMS_READY_TIMES_MAX        200
#define WAIT_BMS_READY_INTERVAL_USEC    200000

enum batt_info_params {
    BATT_INFO_NOTIFY = 0,
    BATT_INFO_SOC,
    BATT_INFO_RES_ID,
    BATT_INFO_VOLTAGE,
    BATT_INFO_TEMP,
    BATT_INFO_FCC,
    BATT_INFO_MAX,
};

static int batt_info_cached[BATT_INFO_MAX];
static bool healthd_msm_err_log_once;

static int write_file_int(char const* path, int value)
{
    int fd;
    char buffer[20];
    int rc = -1, bytes;

    fd = open(path, O_WRONLY);
    if (fd >= 0) {
        bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
        rc = write(fd, buffer, bytes);
        close(fd);
    }

    return rc > 0 ? 0 : -1;
}

/*
 * Restore the battery parameters saved before the last shutdown and hand them
 * back to the QG fuel gauge, then raise soc_notify_ready. qpnp-qg reports an
 * invalid SoC for its first MAX_WAIT_FOR_HEALTHD_COUNT polls until that flag is
 * set, so skipping this leaves the gauge without a starting point.
 */
static void healthd_batt_info_notify()
{
    int rc, fd, id = 0;
    int bms_ready = 0;
    int wait_count = 0;
    char buff[100] = "";
    int batt_info[BATT_INFO_MAX];
    char *ptr, *tmp, *temp_str;
    char path_str[50] = "";
    bool notify_bms = false;

    fd = open(PERSIST_BATT_INFO_PATH, O_RDONLY);
    if (fd < 0) {
        KLOG_WARNING(HEALTHD_TAG, "Error in opening batt_info.txt, fd=%d\n", fd);
        fd = creat(PERSIST_BATT_INFO_PATH, S_IRWXU);
        if (fd < 0) {
            KLOG_ERROR(HEALTHD_TAG, "Couldn't create file, fd=%d errno=%s\n", fd,
                 strerror(errno));
            goto out;
        }
        KLOG_DEBUG(HEALTHD_TAG, "Created file %s\n", PERSIST_BATT_INFO_PATH);
        close(fd);
        goto out;
    } else {
        KLOG_DEBUG(HEALTHD_TAG, "opened %s\n", PERSIST_BATT_INFO_PATH);
    }

    rc = read(fd, buff, (sizeof(buff) - 1));
    if (rc < 0) {
        KLOG_ERROR(HEALTHD_TAG, "Error in reading fd %d, rc=%d\n", fd, rc);
        close(fd);
        goto out;
    }
    close(fd);
    buff[rc] = '\0';
    temp_str = strtok_r(buff, ":", &ptr);
    id = 1;
    while (temp_str != NULL && id < BATT_INFO_MAX) {
        batt_info[id++] = (int)strtol(temp_str, &tmp, 10);
        temp_str = strtok_r(NULL, ":", &ptr);
    }

    if (id < BATT_INFO_MAX) {
        KLOG_ERROR(HEALTHD_TAG, "Read %d batt_info parameters\n", id);
        goto out;
    }

    /* Send batt_info parameters to FG driver */
    for (id = 1; id < BATT_INFO_MAX; id++) {
        snprintf(path_str, sizeof(path_str), "%s", BMS_BATT_INFO_ID_PATH);
        rc = write_file_int(path_str, id);
        if (rc < 0) {
            KLOG_ERROR(HEALTHD_TAG, "Error in writing batt_info_id %d, rc=%d\n", id,
                rc);
            goto out;
        }

        snprintf(path_str, sizeof(path_str), "%s", BMS_BATT_INFO_PATH);
        rc = write_file_int(path_str, batt_info[id]);
        if (rc < 0) {
            KLOG_ERROR(HEALTHD_TAG, "Error in writing batt_info %d, rc=%d\n",
                batt_info[id], rc);
            goto out;
        }
    }

    notify_bms = true;

out:
    fd = open(BMS_READY_PATH, O_RDONLY);
    if (fd < 0) {
        KLOG_ERROR(HEALTHD_TAG, "Couldn't open %s\n", BMS_READY_PATH);
        return;
    }

    /* Wait for soc_reporting_ready */
    wait_count = 0;
    memset(buff, 0, sizeof(buff));
    while (1) {
        rc = read(fd, buff, 1);
        if (rc > 0) {
            sscanf(buff, "%d\n", &bms_ready);
        } else {
            KLOG_ERROR(HEALTHD_TAG, "read soc-ready failed, rc=%d\n", rc);
            break;
        }

        if ((bms_ready > 0) || (wait_count++ > WAIT_BMS_READY_TIMES_MAX))
            break;

        usleep(WAIT_BMS_READY_INTERVAL_USEC);
        lseek(fd, 0, SEEK_SET);
    }
    close(fd);

    if (!bms_ready)
        notify_bms = false;

    if (!notify_bms) {
        KLOG_ERROR(HEALTHD_TAG, "Not notifying BMS\n");
        goto notify_ready;
    }

    /* Notify FG driver */
    snprintf(path_str, sizeof(path_str), "%s", BMS_BATT_INFO_ID_PATH);
    rc = write_file_int(path_str, BATT_INFO_NOTIFY);
    if (rc < 0) {
        KLOG_ERROR(HEALTHD_TAG, "Error in writing batt_info_id, rc=%d\n", rc);
        goto notify_ready;
    }

    snprintf(path_str, sizeof(path_str), "%s", BMS_BATT_INFO_PATH);
    rc = write_file_int(path_str, INT_MAX - 1);
    if (rc < 0)
        KLOG_ERROR(HEALTHD_TAG, "Error in writing batt_info, rc=%d\n", rc);

notify_ready:
    /*
     * Tell qpnp-qg that health has finished restoring, which lets it report a
     * real SoC and kicks its periodic update work.
     */
    memset(path_str, 0, sizeof(path_str));
    snprintf(path_str, sizeof(path_str), "%s", BMS_NOTIFY_READY_PATH);
    rc = write_file_int(path_str, 1);
    if (rc < 0)
        KLOG_ERROR(HEALTHD_TAG,
             "[OPPO_CHG] Error in writing Bms notify ready flag rc=%d\n", rc);
    else
        KLOG_ERROR(HEALTHD_TAG,
             "[OPPO_CHG] Success writing Bms notify ready flag rc=%d\n", rc);
}

static void healthd_store_batt_props(const HealthInfo& props)
{
    char buff[100];
    int fd, rc, len, batteryId = 0;

    if (!props.batteryPresent) {
        return;
    }

    if (props.batteryLevel == 0 || props.batteryVoltageMillivolts == 0) {
        return;
    }

    memset(buff, 0, sizeof(buff));
    fd = open(BMS_BATT_RES_ID_PATH, O_RDONLY);
    if (fd < 0) {
        if (!healthd_msm_err_log_once) {
            KLOG_ERROR(HEALTHD_TAG, "Couldn't open %s\n", BMS_BATT_RES_ID_PATH);
            healthd_msm_err_log_once = true;
        }
    } else {
        rc = read(fd, buff, 6);
        if (rc > 0) {
            sscanf(buff, "%d\n", &batteryId);
            batteryId /= 1000;
        } else if (!healthd_msm_err_log_once) {
            KLOG_ERROR(HEALTHD_TAG, "reading batt_res_id failed, rc=%d\n", rc);
            healthd_msm_err_log_once = true;
        }
        close(fd);
    }

    if (props.batteryLevel == batt_info_cached[BATT_INFO_SOC] &&
        props.batteryVoltageMillivolts == batt_info_cached[BATT_INFO_VOLTAGE] &&
        props.batteryTemperatureTenthsCelsius == batt_info_cached[BATT_INFO_TEMP] &&
        props.batteryFullChargeUah == batt_info_cached[BATT_INFO_FCC] &&
        batteryId == batt_info_cached[BATT_INFO_RES_ID])
        return;

    fd = open(PERSIST_BATT_INFO_PATH, O_RDWR | O_TRUNC);
    if (fd < 0) {
        /*
         * Print the error just only once as this function can be called as
         * long as the system is running and logs should not flood the console.
         */
        if (!healthd_msm_err_log_once) {
            KLOG_ERROR(HEALTHD_TAG, "Error in opening batt_info.txt, fd=%d\n", fd);
            healthd_msm_err_log_once = true;
        }
        return;
    }

    len = snprintf(buff, sizeof(buff), "%d:%d:%d:%d:%d", props.batteryLevel,
                   batteryId, props.batteryVoltageMillivolts,
                   props.batteryTemperatureTenthsCelsius,
                   props.batteryFullChargeUah);
    if (len < 0) {
        if (!healthd_msm_err_log_once) {
            KLOG_ERROR(HEALTHD_TAG, "Error in printing to buff, len=%d\n", len);
            healthd_msm_err_log_once = true;
        }
        close(fd);
        return;
    }

    buff[len] = '\0';
    rc = write(fd, buff, sizeof(buff));
    if (rc < 0) {
        if (!healthd_msm_err_log_once) {
            KLOG_ERROR(HEALTHD_TAG, "Error in writing to batt_info.txt, rc=%d\n", rc);
            healthd_msm_err_log_once = true;
        }
        close(fd);
        return;
    }

    batt_info_cached[BATT_INFO_SOC] = props.batteryLevel;
    batt_info_cached[BATT_INFO_RES_ID] = batteryId;
    batt_info_cached[BATT_INFO_VOLTAGE] = props.batteryVoltageMillivolts;
    batt_info_cached[BATT_INFO_TEMP] = props.batteryTemperatureTenthsCelsius;
    batt_info_cached[BATT_INFO_FCC] = props.batteryFullChargeUah;

    close(fd);
}

namespace aidl::android::hardware::health {
class HealthImpl : public Health {
  public:
    using Health::Health;

  protected:
    void UpdateHealthInfo(HealthInfo* health_info) override {
        healthd_store_batt_props(*health_info);
    }
};
} //namespace aidl::android::hardware::health

void qti_healthd_board_init(struct healthd_config *hc)
{
    int fd;
    unsigned char retries = RETRY_COUNT;
    int ret = 0;
    unsigned char buf;

    hc->ignorePowerSupplyNames.push_back(android::String8(ucsiPSYName[0]));
    hc->ignorePowerSupplyNames.push_back(android::String8(ucsiPSYName[1]));

    healthd_batt_info_notify();
retry:
    if (!retries) {
        KLOG_ERROR(LOG_TAG, "Cannot open battery/capacity, fd=%d\n", fd);
        return;
    }

    fd = open("/sys/class/power_supply/battery/capacity", 0440);
    if (fd >= 0) {
        KLOG_INFO(LOG_TAG, "opened battery/capacity after %d retries\n", RETRY_COUNT - retries);
        while (retries) {
            ret = read(fd, &buf, 1);
            if(ret >= 0) {
                KLOG_INFO(LOG_TAG, "Read Batt Capacity after %d retries ret : %d\n", RETRY_COUNT - retries, ret);
                close(fd);
                return;
            }

            retries--;
            usleep(100000);
        }

        KLOG_ERROR(LOG_TAG, "Failed to read Battery Capacity ret=%d\n", ret);
        close(fd);
        return;
    }

    retries--;
    usleep(100000);
    goto retry;
}

int main(int argc, char** argv) {
#ifdef __ANDROID_RECOVERY__
    android::base::InitLogging(argv, android::base::KernelLogger);
#endif
    auto config = std::make_unique<healthd_config>();
    ::android::hardware::health::InitHealthdConfig(config.get());
    qti_healthd_board_init(config.get());
    auto binder = ndk::SharedRefBase::make<aidl::android::hardware::health::HealthImpl>(
            gInstanceName, std::move(config));

    if (argc >= 2 && argv[1] == gChargerArg) {
#if !CHARGER_FORCE_NO_UI
        KLOG_INFO(LOG_TAG, "Starting charger mode with UI.");
        auto charger_callback = std::make_shared<aidl::android::hardware::health::ChargerCallbackImpl>(binder);
        return ChargerModeMain(binder, charger_callback);
#endif
        KLOG_INFO(LOG_TAG, "Starting charger mode without UI.");
    } else {
        KLOG_INFO(LOG_TAG, "Starting health HAL.");
    }

    auto hal_health_loop = std::make_shared<HalHealthLoop>(binder, binder);
    return hal_health_loop->StartLoop();
}
