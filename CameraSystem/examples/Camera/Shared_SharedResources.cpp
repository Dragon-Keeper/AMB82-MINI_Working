/*
 * Shared_SharedResources.cpp - 共享资源实现
 * 提供跨模块共享的全局资源和状态
 */

#include "Shared_SharedResources.h"
#include <string.h>

TimeSyncTaskStatus g_timeSyncStatus;

void updateTimeSyncStatusFromTask(TimeSyncState state, const char* status, int progress, const char* server) {
    g_timeSyncStatus.state = state;
    if (status) {
        strncpy(g_timeSyncStatus.status, status, sizeof(g_timeSyncStatus.status) - 1);
        g_timeSyncStatus.status[sizeof(g_timeSyncStatus.status) - 1] = '\0';
    } else {
        g_timeSyncStatus.status[0] = '\0';
    }
    g_timeSyncStatus.progress = progress;
    if (server) {
        strncpy(g_timeSyncStatus.server, server, sizeof(g_timeSyncStatus.server) - 1);
        g_timeSyncStatus.server[sizeof(g_timeSyncStatus.server) - 1] = '\0';
    } else {
        g_timeSyncStatus.server[0] = '\0';
    }
    g_timeSyncStatus.updated = true;
}