/*
 * yaffey: Utility for reading, editing and writing YAFFS2 images
 * Copyright (C) 2012 David Place <david.t.place@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef ANDROIDUIDS_H
#define ANDROIDUIDS_H

#include <QMap>

//from:
//http://source-android.frandroid.com/system/core/include/private/android_filesystem_config.h

#define AID_ROOT             0  /* traditional unix root user */

#define AID_SYSTEM        1000  /* system server */

#define AID_RADIO         1001  /* telephony subsystem, RIL */
#define AID_BLUETOOTH     1002  /* bluetooth subsystem */
#define AID_GRAPHICS      1003  /* graphics devices */
#define AID_INPUT         1004  /* input devices */
#define AID_AUDIO         1005  /* audio devices */
#define AID_CAMERA        1006  /* camera devices */
#define AID_LOG           1007  /* log devices */
#define AID_COMPASS       1008  /* compass device */
#define AID_MOUNT         1009  /* mountd socket */
#define AID_WIFI          1010  /* wifi subsystem */
#define AID_ADB           1011  /* android debug bridge (adbd) */
#define AID_INSTALL       1012  /* group for installing packages */
#define AID_MEDIA         1013  /* mediaserver process */
#define AID_DHCP          1014  /* dhcp client */
#define AID_SDCARD_RW     1015  /* external storage write access */
#define AID_VPN           1016  /* vpn system */
#define AID_KEYSTORE      1017  /* keystore subsystem */
#define AID_USB           1018  /* USB devices */
#define AID_DRM           1019  /* DRM server */
#define AID_AVAILABLE     1020  /* available for use */
#define AID_GPS           1021  /* GPS daemon */
#define AID_UNUSED1       1022  /* deprecated, DO NOT USE */
#define AID_MEDIA_RW      1023  /* internal media storage write access */
#define AID_MTP           1024  /* MTP USB driver access */
#define AID_NFC           1025  /* nfc subsystem */
#define AID_DRMRPC        1026  /* group for drm rpc */

#define AID_SHELL         2000  /* adb and debug shell user */
#define AID_CACHE         2001  /* cache access */
#define AID_DIAG          2002  /* access to diagnostic resources */

/* The 3000 series are intended for use as supplemental group id's only.
 * They indicate special Android capabilities that the kernel is aware of. */
#define AID_NET_BT_ADMIN  3001  /* bluetooth: create any socket */
#define AID_NET_BT        3002  /* bluetooth: create sco, rfcomm or l2cap sockets */
#define AID_INET          3003  /* can create AF_INET and AF_INET6 sockets */
#define AID_NET_RAW       3004  /* can create raw INET sockets */
#define AID_NET_ADMIN     3005  /* can configure interfaces and routing tables. */
#define AID_NET_BW_STATS  3006  /* read bandwidth statistics */
#define AID_NET_BW_ACCT   3007  /* change bandwidth statistics accounting */

#define AID_MISC          9998  /* access to misc storage */
#define AID_NOBODY        9999

#define AID_APP          10000  /* first app user */

static QMap<int, QString> getDefaultIDs() {
    QMap<int, QString> map;
    map.insert(AID_ROOT, "root");
    map.insert(AID_SYSTEM, "system");
    map.insert(AID_RADIO, "radio");
    map.insert(AID_BLUETOOTH, "bluetooth");
    map.insert(AID_GRAPHICS, "graphics");
    map.insert(AID_INPUT, "input");
    map.insert(AID_AUDIO, "audio");
    map.insert(AID_CAMERA, "camera");
    map.insert(AID_LOG, "log");
    map.insert(AID_COMPASS, "compass");
    map.insert(AID_MOUNT, "mount");
    map.insert(AID_WIFI, "wifi");
    map.insert(AID_ADB, "adb");
    map.insert(AID_INSTALL, "install");
    map.insert(AID_MEDIA, "media");
    map.insert(AID_DHCP, "dhcp");
    map.insert(AID_SDCARD_RW, "sdcard_rw");
    map.insert(AID_VPN, "vpn");
    map.insert(AID_KEYSTORE, "keystore");
    map.insert(AID_USB, "usb");
    map.insert(AID_DRM, "drm");
    map.insert(AID_AVAILABLE, "available");
    map.insert(AID_GPS, "gps");
    map.insert(AID_UNUSED1, "unused1");
    map.insert(AID_MEDIA_RW, "media_rw");
    map.insert(AID_MTP, "mtp");
    map.insert(AID_NFC, "nfc");
    map.insert(AID_DRMRPC, "drmrpc");
    map.insert(AID_SHELL, "shell");
    map.insert(AID_CACHE, "cache");
    map.insert(AID_DIAG, "diag");
    map.insert(AID_NET_BT_ADMIN, "net_bt_admin");
    map.insert(AID_NET_BT, "net_bt");
    map.insert(AID_INET, "inet");
    map.insert(AID_NET_RAW, "net_raw");
    map.insert(AID_NET_ADMIN, "net_admin");
    map.insert(AID_NET_BW_STATS, "net_bw_stats");
    map.insert(AID_NET_BW_ACCT, "net_bw_acct");
    map.insert(AID_MISC, "misc");
    map.insert(AID_NOBODY, "nobody");
    return map;
}
static const QMap<int, QString> ANDROID_IDS = getDefaultIDs();

#endif  //ANDROIDUIDS_H
