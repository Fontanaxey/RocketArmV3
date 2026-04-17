#ifndef SERIAL_H
#define SERIAL_H
int serial_init(const char *device_path);
// int serial_send(int fd, RobotPacket pkt);
void serial_close(int fd);
#endif