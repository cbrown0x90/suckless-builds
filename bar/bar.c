#include <time.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "sound.h"

//XSetRoot
static Window root;
static int screen;

//Disk Space
unsigned long remaining;
char unit;

//Internet
struct sockaddr_in* eth0;
struct sockaddr_in* wlan0;
struct ifaddrs* ip;
struct ifaddrs* tmp;
char* IPString;

//Sleep
Display* dpy;
int timeout, interval, prefer_blank, allow_exp;

//Battery
char level[4];
char status[12];
FILE* status_file;
FILE* capacity_file;

//Time
struct tm* date;

//Sound
sound s;

//
char bar[100];
struct timespec sleepval;

void init() {
    IPString = malloc(sizeof(char) * 20);

    soundInit();

    sleepval.tv_sec = 0;
    sleepval.tv_nsec = 500000000;

    dpy = XOpenDisplay(NULL);
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

}

void destroy() {
    soundDestroy();

    XCloseDisplay(dpy);
}

void getSizeUnit() {
    if (remaining < 1024) {
        unit = 'B';
    } else if (remaining < (1024 * 1024)) {
        remaining /= 1024;
        unit = 'K';
    } else if (remaining < (1024 * 1024 * 1024)) {
        remaining /= (1024 * 1204);
        unit = 'M';
    } else {
        remaining /= (1024 * 1024 * 1024);
        unit = 'G';
    }
}

void getDisk() {
    struct statvfs root;
    statvfs("/", &root);
    remaining = root.f_bfree * root.f_bsize;
    getSizeUnit();
}

void getIP() {
    eth0 = NULL;
    wlan0 = NULL;

    getifaddrs(&ip);
    for (tmp = ip; tmp != NULL; tmp = tmp->ifa_next) {

        if (strcmp(tmp->ifa_name, "eth0") == 0 &&
                tmp->ifa_addr->sa_family == AF_INET) {
            eth0 = (struct sockaddr_in*) tmp->ifa_addr;

        } else if (strcmp(tmp->ifa_name, "wlan0") == 0 &&
                tmp->ifa_addr->sa_family == AF_INET) {
            wlan0 = (struct sockaddr_in*) tmp->ifa_addr;
        }
    }
    if (eth0) {
        sprintf(IPString, "  %s ", inet_ntoa(eth0->sin_addr));
    } else if (wlan0) {
        sprintf(IPString, "  %s ", inet_ntoa(wlan0->sin_addr));
    } else {
        sprintf(IPString, " No IP");
    }
    freeifaddrs(ip);
}

void getSleep() {
    XGetScreenSaver(dpy, &timeout, &interval, &prefer_blank, &allow_exp);
}

void getBattery() {
    if (access("/sys/class/power_supply/BAT0", F_OK)) {
        memcpy(&status, "unknown", sizeof(char) * 7);
        memcpy(&level, "?", sizeof(char));
    } else {
        status_file = fopen("/sys/class/power_supply/BAT0/status", "r");
        capacity_file = fopen("/sys/class/power_supply/BAT0/capacity", "r");
        fscanf(status_file, "%s", status);
        fscanf(capacity_file, "%s", level);
        fclose(status_file);
        fclose(capacity_file);
    }
}

//Get time
void getTime() {
    time_t t = time(NULL);
    date = localtime(&t);
}

char* batteryIcon() {
    int tmp_level = atoi(level);
    if (strcmp(status, "Charging") == 0 ||
            strcmp(status, "Full") == 0) {
        return " ";
    } else if (tmp_level < 5) {
        return "";
    } else if (tmp_level < 25) {
        return "";
    } else if (tmp_level < 60) {
        return " ";
    } else if (tmp_level < 85) {
        return " ";
    } else {
        return " ";
    }
}

int main() {
    init();
    while (1) {
        getTime();
        getSleep();
        getBattery();
        getIP();
        getDisk();
        s = getMasterStatus();

        //calloc(100, sizeof(char));
        sprintf(bar, "  %ld%c |%s|%s %s%%\x01|%s| %s%s | %d-%02d-%02d %02d:%02d:%02d",
                remaining, unit,
                IPString,
                batteryIcon(), level,
                timeout ? "\x01 true\x01" : "\x03 false\x01",
                volIcon(), !s.status ? "Muted" : s.percent,
                1900 + date->tm_year, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);

        //XStoreName(dpy, root, 0);
        XStoreName(dpy, root, bar);
        nanosleep(&sleepval, NULL);
    }
    destroy();
}
