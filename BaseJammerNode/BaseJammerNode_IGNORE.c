#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "net/nullnet/nullnet.h"

PROCESS(jammer_process, "Constant Jammer Process");
AUTOSTART_PROCESSES(&jammer_process);

PROCESS_THREAD(jammer_process, ev, data) {
	static struct etimer mer;

	PROCESS_BEGIN();

	while(1) { 
		etimer_set(&mer,CLOCK_SECOND);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&mer));
		nullnet_buf[0] = 'J'; // An idenfier for jamming packet 
		nullnet_len = 1;
		NETSTACK_NETWORK.output(NULL);
	}

	PROCESS_END();
}
