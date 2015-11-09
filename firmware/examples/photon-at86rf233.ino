#include "at86rf233/at86rf2xx.h"

SYSTEM_THREAD(ENABLED);

int received = 0;

int interrupt_pin = D2;
int reset_pin = D6;
int sleep_pin = D5;
int cs_pin = D0;

void setup() {
  Serial.begin(115200);
  at86rf2xx_init(2000000, cs_pin, interrupt_pin, sleep_pin, reset_pin);
}

void loop() {
  at86rf2xx_eventHandler();
  return;
}

void at86rf2xx_eventHandler() {
  if (at86rf2xx_events > 0)
  {
    at86rf2xx_events--;
    uint8_t irq_mask;
    uint8_t state;

    /* If transceiver is sleeping register access is impossible and frames are
     * lost anyway, so return immediately.
     */
    state = at86rf2xx_get_status();
    if(state == AT86RF2XX_STATE_SLEEP)
      return;

    /* read (consume) device status */
    irq_mask = at86rf2xx_reg_read(AT86RF2XX_REG__IRQ_STATUS);

    /*  Incoming radio frame! */
    if (irq_mask & AT86RF2XX_IRQ_STATUS_MASK__RX_START) {
      Serial.println("[at86rf2xx] EVT - RX_START");
    }

    /*  Done receiving radio frame; what do you want
     *  to do with it?
     */
    if (irq_mask & AT86RF2XX_IRQ_STATUS_MASK__TRX_END)
    {
      if(state == AT86RF2XX_STATE_RX_AACK_ON || state == AT86RF2XX_STATE_BUSY_RX_AACK) {
        Serial.println("[at86rf2xx] EVT - RX_END");
        at86rf2xx_receive_data();
      }
    }
  }
}

void at86rf2xx_receive_data() {
  /*  print the length of the frame
   *  (including the header)
   */
  size_t pkt_len = at86rf2xx_rx_len();
  Serial.print("Frame length: ");
  Serial.print(pkt_len);
  Serial.println(" bytes");

  /*  Print the frame, byte for byte  */
  Serial.println("Frame dump (ASCII):");
  uint8_t data[pkt_len];
  at86rf2xx_rx_read(data, pkt_len, 0);
  for (int d=0; d<pkt_len; d++) {
    Serial.print((char)data[d]);
  }
  Serial.println();

  /* How many frames is this so far?  */
  Serial.print("[[Total frames received: ");
  Serial.print(++received);
  Serial.println("]]\n");
}
