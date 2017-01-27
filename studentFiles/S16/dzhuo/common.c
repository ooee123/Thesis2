#include "networks.h"

// Common functions
int send_data(int socket, char* buffer, int len) {
    int sent_len = 0;
    int total = 0;

    while (total < len) {
        sent_len = send(socket, buffer, len, 0);

        if (sent_len < 0) {
            perror("send call");
            exit(-1);
        }

        total += sent_len;
    }

    return total;
}

int recv_data(int socket, char* buffer, int len) {
    int recv_len = 0;
    int total = 0;

    while (total < len) {
        recv_len = recv(socket, buffer + recv_len, len, 0);

        if (recv_len < 0) {
            perror("recv call");
            exit(-1);
        }
        else if (recv_len == 0) {
            return 0;
        }

        total += recv_len;
    }

    return total;
}

char* prep_base_packet(int len, int* packet_offset, char flag) {
    // Create packet
    char* pkt = calloc(MAX(len, 3), 1);
    int pkt_off = 2;
    int pkt_len = htons(len);

    // Add length and flag info
    memcpy(pkt, &pkt_len, pkt_off);
    pkt[pkt_off++] = flag;

    // Optional arg
    if (packet_offset)
        *packet_offset = pkt_off;

    return pkt;
}


void server_print_packets(char *buf, uint16_t pkt_len) {
    int char_count = 0;
    printf("Message received, length: %d\n", pkt_len);
    printf("Data: ");
    while (char_count < pkt_len) {
        putchar('_');
        if (isprint(buf[char_count]))
            putchar(buf[char_count]);
        else
            printf("d%d", buf[char_count]);
        char_count++;
    }
    printf("\nText: ");
    char_count = 0;
    while (char_count < pkt_len) {
        putchar(buf[char_count++]);
    }
    printf("\n\n");
}

// Helper functions for server directory
int get_empty_entry(server_data* sd) {
    int i, j;

    for (i = 0; i < sd->capacity / 8; i++) {
        for (j = 0; j < 8; j++) {
            if (((sd->bit_array[i] >> j) & 1) == 0) {
                return 8*i + j;
            }
        }
    }

    return -1;
}

void mark_entry(int index, server_data *sd) {
    sd->bit_array[index/8] |= (1 << (index % 8));
}

void unmark_entry(int index, server_data *sd) {
    sd->bit_array[index/8] &= ~(1 << (index % 8));
}

int next_entry(int index, server_data *sd) {
    index++;
    int i = index / 8;
    int j = index % 8;

    while (i < sd->capacity / 8) {
        while (j < 8) {
            if (((sd->bit_array[i] >> j) & 1) == 1) {
                return 8*i + j;
            }
            j++;
        }
        j = 0;
        i++;
    }

    // No next entry
    return -1;
}

// Increases bit array & directory, returns new index
int expand_directory(server_data *sd) {
    sd->capacity += 8;
    sd->dir = realloc(sd->dir, sizeof(entry) * sd->capacity);
    sd->bit_array = realloc(sd->bit_array, sd->capacity/8);
    sd->bit_array[sd->capacity/8 - 1] = 0;
    return sd->capacity - 8;
}
