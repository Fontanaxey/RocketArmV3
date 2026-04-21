#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define START_BYTE 0xAA
#define END_BYTE 0x55

#define ANGLE_MIN 0
#define ANGLE_MAX 180
#define DEFAULT_POS 90

typedef struct __attribute__((packed))
{
    uint8_t start;
    uint8_t id;
    uint8_t value;
    uint8_t checksum;
    uint8_t end;
} RobotPacket;

static inline uint8_t calculate_checksum(uint8_t id, uint8_t value)
{
    return (id ^ value);
}

static inline void protocol_create_packet(RobotPacket *pkt, uint8_t id, uint8_t value)
{
    if (!pkt)
        return;
    pkt->start = START_BYTE;
    pkt->id = id;
    pkt->value = value;
    pkt->checksum = calculate_checksum(id, value);
    pkt->end = END_BYTE;
}

#endif