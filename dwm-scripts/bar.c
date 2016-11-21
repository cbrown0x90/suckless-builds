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
#include "sound.c"

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
char IPString[20];

//Sleep
Display* dpy;
int timeout, interval, prefer_blank, allow_exp;

//Battery
int level;
char status[12];
FILE* status_file;
FILE* capacity_file;

//Time
struct tm* date;

//
char bar[100];
struct timespec sleepval;

void init() {

    sleepval.tv_sec = 0;
    sleepval.tv_nsec = 500000000;

    dpy = XOpenDisplay(NULL);
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    mute_regex = pcre_compile(
            "\\[(o[fn]f?)\\]",
            0,
            &error,
            &erroffset,
            NULL);

    vol_regex = pcre_compile(
            "\\[(\\d?\\d?\\d%)\\]",
            0,
            &error,
            &erroffset,
            NULL);
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
}

void getSleep() {
    XGetScreenSaver(dpy, &timeout, &interval, &prefer_blank, &allow_exp);
}

void getBattery() {
    status_file = fopen("/sys/class/power_supply/BAT0/status", "r");
    capacity_file = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    fscanf(status_file, "%s", status);
    fscanf(capacity_file, "%d", &level);
    fclose(status_file);
    fclose(capacity_file);
}

//Get time
void getTime() {
    time_t t = time(NULL);
    date = localtime(&t);
}

char* batteryIcon() {
    if (strcmp(status, "Charging") == 0 ||
            strcmp(status, "Full") == 0) {
        return "";
    } else if (level < 5) {
        return "";
    } else if (level < 25) {
        return "";
    } else if (level < 60) {
        return "";
    } else if (level < 85) {
        return "";
    } else {
        return "";
    }
}

void getIPString() {
    if (eth0) {
        sprintf(IPString, " %s", inet_ntoa(eth0->sin_addr));
    } else {
        sprintf(IPString, " %s", wlan0 ? inet_ntoa(wlan0->sin_addr) : "No IP");
    }
}

int main() {
    init();
    while (1) {
        getTime();
        getSleep();
        getBattery();
        getIP();
        getIPString();
        getDisk();
        getSound();
        regex();

        sprintf(bar, " %ld%c | %s | %s %d%% | %s | %s%s | %d-%02d-%02d %02d:%02d:%02d",
                remaining, unit,
                IPString,
                batteryIcon(), level,
                timeout ? " true" : " false",
                volIcon(), ((strcmp(mute, "off")) == 0 ? "Muted" : vol),
                1900 + date->tm_year, date->tm_mon, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);

        XStoreName(dpy, root, bar);
        nanosleep(&sleepval, NULL);
    }
}
