#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "serial_linux.h"
#include "errhandler.h"
#include "protocol.h"

int serial_init(const char *device_path)
{
    char err_msg[256];

    int fd = open(device_path, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        snprintf(err_msg, sizeof(err_msg), "Failed to open device %s: %s", device_path, strerror(errno));
        log_event(FATAL, err_msg);
        return -1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
        snprintf(err_msg, sizeof(err_msg), "tcgetattr failed: %s", strerror(errno));
        log_event(FATAL, err_msg);
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;

    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;

    tty.c_cflag |= (CLOCAL | CREAD);

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        snprintf(err_msg, sizeof(err_msg), "tcsetattr failed: %s", strerror(errno));
        log_event(FATAL, err_msg);
        close(fd);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);

    snprintf(err_msg, sizeof(err_msg), "Serial initialized: %s (9600 8N1)", device_path);
    log_event(ACCESS, err_msg);
    return fd;
}

int serial_send(int fd, RobotPacket pkt)
{
    if (fd < 0)
        return -1;

    ssize_t n = write(fd, &pkt, sizeof(RobotPacket));

    if (n != (ssize_t)sizeof(RobotPacket))
    {
        char err_msg[128];
        snprintf(err_msg, sizeof(err_msg), "Write mismatch: sent %zd/%zu bytes", n, sizeof(RobotPacket));
        log_event(WARNING, err_msg);
        return -1;
    }

    // Optional: force to drain the output buffer to ensure the packet is sent immediately.
    // tcdrain(fd);

    return 0;
}

void serial_close(int fd)
{
    if (fd >= 0)
    {
        tcdrain(fd);
        if (close(fd) < 0)
            log_event(WARNING, "Error during serial_close");
        else
            log_event(ACCESS, "Serial port closed cleanly");
    }
}