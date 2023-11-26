#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "sys/node-id.h"
#include "sys/log.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uiplib.h"
#include "net/ipv6/uip-udp-packet.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#define DISCOVERY_PORT 12345
#define NEIGHBOR_PORT 12346
#define CHANNEL_FREE_THRESHOLD -80  // RSSI threshold to consider the channel as free

static struct simple_udp_connection discovery_conn;
static int channel_free = 1;

// Callback function for ctimer
void channel_free_callback() {
	channel_free = 1;
}

// Handler for incoming discovery messages
void discovery_handler(struct simple_udp_connection *c,
                       const uip_ipaddr_t *sender_addr,
                       uint16_t sender_port,
                       const uip_ipaddr_t *receiver_addr,
                       uint16_t receiver_port,
                       const uint8_t *data,
                       uint16_t datalen)
{
  if(strcmp((char *)data, "Discovery") == 0) {
    LOG_INFO("Received discovery message from ");
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");

    char response[30];
	snprintf(response, sizeof(response), "Hello from Mote %u", node_id);
	simple_udp_sendto(&discovery_conn, response, strlen(response) + 1, sender_addr);

  }
  //If message is 0xff packet is from jammer
  else if(data[0] == 0xff && channel_free) {
	LOG_INFO("Received jamming message from ");
	LOG_INFO_6ADDR(sender_addr);
	LOG_INFO_("\n");
	channel_free = 0;
	//Create ctimer to set the channel free again
	static struct ctimer ct;
	ctimer_set(&ct, CLOCK_SECOND*5, (void *)channel_free_callback, NULL);
  }
}

PROCESS(unicast_process, "Unicast with Discovery");
AUTOSTART_PROCESSES(&unicast_process);

PROCESS_THREAD(unicast_process, ev, data)
{
 	static struct etimer discovery_timer;

  	PROCESS_BEGIN();

  	// Register UDP connection for discovery messages
  	simple_udp_register(&discovery_conn, DISCOVERY_PORT, NULL, DISCOVERY_PORT, discovery_handler);
  	etimer_set(&discovery_timer, CLOCK_SECOND*5);

  	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&discovery_timer));
		if(channel_free){
			uip_ipaddr_t addr;
			uip_create_linklocal_allnodes_mcast(&addr); // Create multicast address
			LOG_INFO("Sending discovery message\n");
			simple_udp_sendto(&discovery_conn, "Discovery", 10, &addr);
		}
		etimer_reset(&discovery_timer);
    }
    PROCESS_END();
}