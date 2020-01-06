#pragma once

#include <fcntl.h>
#include <inttypes.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include <boost/optional/optional.hpp>

#include "exceptions.h"

class SocketCan
{
    // TODO: correct naming

    int soc;
    int read_can_port;

    bool is_interface_online(std::string port);
    int rx_data_n = 0;
    int tx_data_n = 0;

    char
        ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
    msghdr smsg;
    struct iovec iov;
    struct can_frame frame;

  public:
    int can_close();
    int can_send(std::pair<uint16_t, std::vector<uint8_t>> data);

    int GetReceivedDataSize();
    int GetSentDataSize();

    boost::optional<std::tuple<uint16_t, std::vector<uint8_t>, timeval>>
    can_receive(uint32_t timeout_ms);

    SocketCan(std::string port);
};
