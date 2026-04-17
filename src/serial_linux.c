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
        snprintf(err_msg, sizeof(err_msg), "tcgetattr failed on %s: %s", device_path, strerror(errno));
        log_event(FATAL, err_msg);
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        snprintf(err_msg, sizeof(err_msg), "tcsetattr failed on %s: %s", device_path, strerror(errno));
        log_event(FATAL, err_msg);
        close(fd);
        return -1;
    }

    snprintf(err_msg, sizeof(err_msg), "Serial device %s initialized successfully on fd %d", device_path, fd);
    log_event(ACCESS, err_msg);
    return fd;
}

int serial_send(int fd, RobotPacket pkt)
{
    if (fd < 0)
    {
        log_event(WARNING, "serial_send invoked with invalid file descriptor");
        return -1;
    }

    ssize_t n = write(fd, &pkt, sizeof(RobotPacket));
    if (n != (ssize_t)sizeof(RobotPacket))
    {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "Incomplete write. Expected %zu, wrote %zd: %s", sizeof(RobotPacket), n, strerror(errno));
        log_event(WARNING, err_msg);
        return -1;
    }
    return 0;
}

void serial_close(int fd)
{
    if (fd >= 0)
    {
        if (close(fd) < 0)
        {
            char err_msg[128];
            snprintf(err_msg, sizeof(err_msg), "Failed to close fd %d: %s", fd, strerror(errno));
            log_event(WARNING, err_msg);
        }
        else
        {
            char log_msg[64];
            snprintf(log_msg, sizeof(log_msg), "Serial connection closed on fd %d", fd);
            log_event(ACCESS, log_msg);
        }
    }
}