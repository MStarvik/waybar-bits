#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


unsigned long long get_bytes(const char *path, bool *success) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    *success = false;
    return 0;
  }

  unsigned long long bytes;
  fscanf(file, "%llu", &bytes);
  fclose(file);

  return bytes;
}

void format_bytes(unsigned long long bytes, char *buf) {
  static const char *units[] = {"B", "K", "M", "G", "T", "P", "E", "Z", "Y"};

  int i = 0;
  while (bytes >= 1000) {
    bytes /= 1000;
    i++;
  }

  sprintf(buf, "%llu%s", bytes, units[i]);
}


struct network_traffic {
  unsigned long long rx_bytes;
  unsigned long long tx_bytes;
  bool valid;
};

void network_traffic_init(struct network_traffic *traffic) {
  traffic->rx_bytes = 0;
  traffic->tx_bytes = 0;
  traffic->valid = true;
}

void network_traffic_add(struct network_traffic *traffic,
                         const struct network_traffic *other) {
  traffic->rx_bytes += other->rx_bytes;
  traffic->tx_bytes += other->tx_bytes;
}

void network_traffic_sub(struct network_traffic *traffic,
                                 const struct network_traffic *other) {
  traffic->rx_bytes -= other->rx_bytes;
  traffic->tx_bytes -= other->tx_bytes;
}


struct network_interface {
  char *rx_path;
  char *tx_path;
};

void network_interface_init(struct network_interface *iface, const char *name) {
  const int rx_path_len = strlen("/sys/class/net/") + strlen(name) +
                          strlen("/statistics/rx_bytes") + 1;
  const int tx_path_len = strlen("/sys/class/net/") + strlen(name) +
                          strlen("/statistics/tx_bytes") + 1;

  iface->rx_path = malloc(rx_path_len);
  iface->tx_path = malloc(tx_path_len);

  sprintf(iface->rx_path, "/sys/class/net/%s/statistics/rx_bytes", name);
  sprintf(iface->tx_path, "/sys/class/net/%s/statistics/tx_bytes", name);
}

void network_interface_get_traffic(const struct network_interface *iface,
                                   struct network_traffic *traffic) {
  traffic->valid = true;
  traffic->rx_bytes = get_bytes(iface->rx_path, &traffic->valid);
  traffic->tx_bytes = get_bytes(iface->tx_path, &traffic->valid);
}


int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <iface>...\n", argv[0]);
    return 1;
  }

  const int num_ifaces = argc - 1;

  struct network_interface ifaces[num_ifaces];
  for (int i = 0; i < num_ifaces; i++) {
    network_interface_init(&ifaces[i], argv[i + 1]);
  }

  struct network_traffic traffic_prev[num_ifaces];
  for (int i = 0; i < num_ifaces; i++) {
    network_interface_get_traffic(&ifaces[i], &traffic_prev[i]);
  }

  while (1) {
    sleep(1);

    struct network_traffic traffic_delta;
    network_traffic_init(&traffic_delta);

    for (int i = 0; i < num_ifaces; i++) {
      struct network_traffic traffic_iface;
      network_interface_get_traffic(&ifaces[i], &traffic_iface);

      if (traffic_iface.valid && traffic_prev[i].valid) {
        struct network_traffic traffic_iface_delta = traffic_iface;
        network_traffic_sub(&traffic_iface_delta, &traffic_prev[i]);
        
        network_traffic_add(&traffic_delta, &traffic_iface_delta);
      }

      traffic_prev[i] = traffic_iface;
    }

    char rx_buf[5];
    char tx_buf[5];

    format_bytes(traffic_delta.rx_bytes, rx_buf);
    format_bytes(traffic_delta.tx_bytes, tx_buf);

    printf("{\"text\": \"%4s⇣ %4s⇡\"}\n", rx_buf, tx_buf);
    fflush(stdout);
  }
}
