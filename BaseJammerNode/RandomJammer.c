#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "sys/node-id.h"
#include "sys/log.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uiplib.h"
#include "net/ipv6/uip-udp-packet.h"
#include "random.h"

#define LOG_MODULE "Jammer"
#define LOG_LEVEL LOG_LEVEL_INFO
#define JAMMING_CHANNEL 26
#define JAMMER_PORT 12345

static struct simple_udp_connection jammer_conn;


PROCESS(jammer_process, "UDP Jammer");
AUTOSTART_PROCESSES(&jammer_process);

PROCESS_THREAD(jammer_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();
  int seed = node_id *2;
  random_init(seed);
  LOG_INFO("UDP Jammer started\n");

  // Set the radio channel
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, JAMMING_CHANNEL);

  // Register UDP connection
  simple_udp_register(&jammer_conn, JAMMER_PORT, NULL, JAMMER_PORT, NULL);

  // Set the etimer to expire after a short interval
  etimer_set(&et, CLOCK_SECOND / 20); // 50 ms interval

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    uip_ipaddr_t addr;
    uip_create_linklocal_allnodes_mcast(&addr); // Create multicast address

    // Transmit noise via UDP broadcast
    if(random_rand() > 0xafff){
        uint8_t jamming_packet[127]; // Max size of a packet
        memset(jamming_packet, 0xff, sizeof(jamming_packet)); // Fill packet with noise
        simple_udp_sendto(&jammer_conn, jamming_packet, sizeof(jamming_packet), &addr);
    }
    // Reset the etimer for the next transmission
    etimer_reset(&et);
  }

  PROCESS_END();
}

