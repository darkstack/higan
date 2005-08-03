void bCPU::mmio_reset() {
//$2181-$2183
  status.wram_addr = 0x000000;

//$4016
  status.joypad1_strobe_value = 0x00;
  status.joypad1_read_pos     = 0;

//$4200
  status.nmi_enabled      = false;
  status.hirq_enabled     = false;
  status.virq_enabled     = false;
  status.auto_joypad_poll = false;

//$4201
  status.pio = 0xff;

//$4202-$4203
  status.mul_a = 0x00;
  status.mul_b = 0x00;

//$4204-$4206
  status.div_a = 0x0000;
  status.div_b = 0x00;

//$4207-$420a
  status.hirq_pos = 0;
  status.virq_pos = 0;

//$4214-$4217
  status.r4214 = 0x0000;
  status.r4216 = 0x0000;
}

//WMDATA
uint8 bCPU::mmio_r2180() {
uint8 r;
  r = mem_bus->read(0x7e0000 | status.wram_addr);
  status.wram_addr++;
  status.wram_addr &= 0x01ffff;
  return r;
}

//???
uint8 bCPU::mmio_r21c2() {
  return 0x20;
}

//???
uint8 bCPU::mmio_r21c3() {
  return 0x00;
}

/*
  The joypad contains a small bit shifter that has 16 bits.
  Reading from 4016 reads one bit from this buffer, then moves
  the buffer left one, and adds a '1' to the rightmost bit.
  Writing a one to $4016 will fill the buffer with the current
  joypad button states, and lock the bit shifter at position
  zero. All reads will be the first buffer state, or 'B'.
  A zero must be written back to $4016 to unlock the buffer,
  so that reads will increment the bit shifting position.
*/
//JOYSER0
uint8 bCPU::mmio_r4016() {
uint8 r = 0x00;
  if(status.joypad1_strobe_value == 1) {
    r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_B);
  } else {
    switch(status.joypad1_read_pos) {
    case  0:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_B);     break;
    case  1:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_Y);     break;
    case  2:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_SELECT);break;
    case  3:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_START); break;
    case  4:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_UP);    break;
    case  5:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_DOWN);  break;
    case  6:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_LEFT);  break;
    case  7:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_RIGHT); break;
    case  8:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_A);     break;
    case  9:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_X);     break;
    case 10:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_L);     break;
    case 11:r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_R);     break;
    case 16:r |= 1;break; //joypad connected bit
    default:r |= 1;break; //after 16th read, all subsequent reads return 1
    }
    if(++status.joypad1_read_pos > 17)status.joypad1_read_pos = 17;
  }
  return r;
}

//RDNMI
uint8 bCPU::mmio_r4210() {
uint8 r = 0x00;
uint16 v, h, hc, vs;
  v  = vcounter();
  h  = hcounter();
  hc = hcycles();
  vs = (overscan()?239:224);

  if(status.r4210_read == false) {
    if((v == (vs + 1) && hc >= 2) || v > (vs + 1)) {
      r |= 0x80;
      status.r4210_read = true;
      status.nmi_pin = 1;
    }
  }

  r |= 0x40;
  r |= 0x02; //cpu version number
  return r;
}

//TIMEUP
uint8 bCPU::mmio_r4211() {
uint8 r = 0x00;
  r |= 0x40;
  if(status.irq_triggered == true)r |= 0x80;
  status.irq_triggered = false;
  return r;
}

//HVBJOY
uint8 bCPU::mmio_r4212() {
uint8  r;
uint16 v, h, hc, vs;
  r = 0x00;

  v  = vcounter();
  h  = hcounter();
  hc = hcycles();
  vs = (overscan()?239:224);

//auto joypad polling
  if(v >= (vs + 1) && v <= (vs + 3))r |= 0x01;

//hblank
  if(hc <= 2 || hc >= 1096)r |= 0x40;

//vblank
  if(v >= (vs + 1))r |= 0x80;
  return r;
}

//RDIO
uint8 bCPU::mmio_r4213() {
  return status.pio;
}

//RDDIVL
uint8 bCPU::mmio_r4214() {
  return status.r4214;
}

//RDDIVH
uint8 bCPU::mmio_r4215() {
  return status.r4214 >> 8;
}

//RDMPYL
uint8 bCPU::mmio_r4216() {
  return status.r4216;
}

//RDMPYH
uint8 bCPU::mmio_r4217() {
  return status.r4216 >> 8;
}

//JOY1L
uint8 bCPU::mmio_r4218() {
uint8 r = 0x00;
uint16 v = vcounter();
  if(status.auto_joypad_poll == false)return 0x00; //can't read joypad if auto polling not enabled
//if(v >= 225 && v <= 227)return 0x00; //can't read joypad while SNES is polling input
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_A) << 7;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_X) << 6;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_L) << 5;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_R) << 4;
  return r;
}

//JOY1H
uint8 bCPU::mmio_r4219() {
uint8 r = 0x00;
uint16 v = vcounter();
  if(status.auto_joypad_poll == false)return 0x00; //can't read joypad if auto polling not enabled
//if(v >= 225 && v <= 227)return 0x00; //can't read joypad while SNES is polling input
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_B)      << 7;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_Y)      << 6;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_SELECT) << 5;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_START)  << 4;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_UP)     << 3;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_DOWN)   << 2;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_LEFT)   << 1;
  r |= (uint8)snes->get_input_status(SNES::DEV_JOYPAD1, SNES::JOYPAD_RIGHT);
  return r;
}

//DMAPx
uint8 bCPU::mmio_r43x0(uint8 i) {
  return channel[i].dmap;
}

//BBADx
uint8 bCPU::mmio_r43x1(uint8 i) {
  return channel[i].destaddr;
}

//A1TxL
uint8 bCPU::mmio_r43x2(uint8 i) {
  return channel[i].srcaddr;
}

//A1TxH
uint8 bCPU::mmio_r43x3(uint8 i) {
  return channel[i].srcaddr >> 8;
}

//A1Bx
uint8 bCPU::mmio_r43x4(uint8 i) {
  return channel[i].srcaddr >> 16;
}

//DASxL
uint8 bCPU::mmio_r43x5(uint8 i) {
  return channel[i].xfersize;
}

//DASxH
uint8 bCPU::mmio_r43x6(uint8 i) {
  return channel[i].xfersize >> 8;
}

//DASBx
uint8 bCPU::mmio_r43x7(uint8 i) {
  return channel[i].hdma_indirect_bank;
}

//A2AxL
uint8 bCPU::mmio_r43x8(uint8 i) {
  return channel[i].hdma_taddr;
}

//A2AxH
uint8 bCPU::mmio_r43x9(uint8 i) {
  return channel[i].hdma_taddr >> 8;
}

//NTRLx
uint8 bCPU::mmio_r43xa(uint8 i) {
  return channel[i].hdma_line_counter;
}

//???
uint8 bCPU::mmio_r43xb(uint8 i) {
  return channel[i].hdma_unknown;
}

uint8 bCPUMMIO::read(uint32 addr) {
uint8 i;
//cpu->sync();

//APU
  if(addr >= 0x2140 && addr <= 0x217f) {
    return apu->port_read(addr & 3);
  }

//HDMA
  if(addr >= 0x4300 && addr <= 0x437f) {
    i = (addr >> 4) & 7;
    switch(addr & 0xf) {
    case 0x0:return cpu->mmio_r43x0(i);
    case 0x1:return cpu->mmio_r43x1(i);
    case 0x2:return cpu->mmio_r43x2(i);
    case 0x3:return cpu->mmio_r43x3(i);
    case 0x4:return cpu->mmio_r43x4(i);
    case 0x5:return cpu->mmio_r43x5(i);
    case 0x6:return cpu->mmio_r43x6(i);
    case 0x7:return cpu->mmio_r43x7(i);
    case 0x8:return cpu->mmio_r43x8(i);
    case 0x9:return cpu->mmio_r43x9(i);
    case 0xa:return cpu->mmio_r43xa(i);
    case 0xb:return cpu->mmio_r43xb(i);
    case 0xc:return 0x43; //unmapped
    case 0xd:return 0x43; //unmapped
    case 0xe:return 0x43; //unmapped
    case 0xf:return cpu->mmio_r43xb(i); //mirror of 43xb
    }
  }

  switch(addr) {
  case 0x2180:return cpu->mmio_r2180(); //WMDATA
  case 0x21c2:return cpu->mmio_r21c2(); //???
  case 0x21c3:return cpu->mmio_r21c3(); //???
  case 0x2800:return cpu->srtc_read();
  case 0x4016:return cpu->mmio_r4016(); //JOYSER0
  case 0x4210:return cpu->mmio_r4210(); //RDNMI
  case 0x4211:return cpu->mmio_r4211(); //TIMEUP
  case 0x4212:return cpu->mmio_r4212(); //HVBJOY
  case 0x4213:return cpu->mmio_r4213(); //RDIO
  case 0x4214:return cpu->mmio_r4214(); //RDDIVL
  case 0x4215:return cpu->mmio_r4215(); //RDDIVH
  case 0x4216:return cpu->mmio_r4216(); //RDMPYL
  case 0x4217:return cpu->mmio_r4217(); //RDMPYH
  case 0x4218:return cpu->mmio_r4218(); //JOY1L
  case 0x4219:return cpu->mmio_r4219(); //JOY1H
  case 0x421a:case 0x421b:case 0x421c:case 0x421d:case 0x421e:case 0x421f:return 0x00;
  }
  return 0x00;
}

//WMDATA
void bCPU::mmio_w2180(uint8 value) {
  mem_bus->write(0x7e0000 | status.wram_addr, value);
  status.wram_addr++;
  status.wram_addr &= 0x01ffff;
}

//WMADDL
void bCPU::mmio_w2181(uint8 value) {
  status.wram_addr = (status.wram_addr & 0xffff00) | value;
  status.wram_addr &= 0x01ffff;
}

//WMADDM
void bCPU::mmio_w2182(uint8 value) {
  status.wram_addr = (status.wram_addr & 0xff00ff) | (value << 8);
  status.wram_addr &= 0x01ffff;
}

//WMADDH
void bCPU::mmio_w2183(uint8 value) {
  status.wram_addr = (status.wram_addr & 0x00ffff) | (value << 16);
  status.wram_addr &= 0x01ffff;
}

//JOYSER0
void bCPU::mmio_w4016(uint8 value) {
  status.joypad1_strobe_value = (value & 1);
  if(value == 1) {
    snes->poll_input();
    status.joypad1_read_pos = 0;
  }
}

//NMITIMEN
void bCPU::mmio_w4200(uint8 value) {
  status.nmi_enabled      = !!(value & 0x80);
  status.virq_enabled     = !!(value & 0x20);
  status.hirq_enabled     = !!(value & 0x10);
  status.auto_joypad_poll = !!(value & 0x01);

  if(status.nmi_enabled == false) {
    status.nmi_pin = 0;
  }

  if(status.virq_enabled == false && status.hirq_enabled == false) {
    status.irq_pin = 0;
  }
}

//WRIO
void bCPU::mmio_w4201(uint8 value) {
  if((status.pio & 0x80) && !(value & 0x80)) {
    ppu->latch_counters();
  }
  status.pio = value;
}

//WRMPYA
void bCPU::mmio_w4202(uint8 value) {
  status.mul_a = value;
}

void bCPU::mmio_w4203(uint8 value) {
  status.mul_b = value;
  status.r4216 = status.mul_a * status.mul_b;
}

//WRDIVL
void bCPU::mmio_w4204(uint8 value) {
  status.div_a = (status.div_a & 0xff00) | value;
}

//WRDIVH
void bCPU::mmio_w4205(uint8 value) {
  status.div_a = (status.div_a & 0x00ff) | (value << 8);
}

//WRDIVB
void bCPU::mmio_w4206(uint8 value) {
  status.div_b = value;
  status.r4214 = (status.div_b)?status.div_a / status.div_b : 0xffff;
  status.r4216 = (status.div_b)?status.div_a % status.div_b : status.div_a;
}

//HTIMEL
void bCPU::mmio_w4207(uint8 value) {
  status.hirq_pos = (status.hirq_pos & 0xff00) | value;
  if(status.irq_triggered == false)status.irq_pin = 1;
}

//HTIMEH
void bCPU::mmio_w4208(uint8 value) {
  status.hirq_pos = (status.hirq_pos & 0x00ff) | (value << 8);
  if(status.irq_triggered == false)status.irq_pin = 1;
}

//VTIMEL
void bCPU::mmio_w4209(uint8 value) {
  status.virq_pos = (status.virq_pos & 0xff00) | value;
  if(status.irq_triggered == false)status.irq_pin = 1;
}

//VTIMEH
void bCPU::mmio_w420a(uint8 value) {
  status.virq_pos = (status.virq_pos & 0x00ff) | (value << 8);
  if(status.irq_triggered == false)status.irq_pin = 1;
}

//DMAEN
void bCPU::mmio_w420b(uint8 value) {
  if(value != 0x00) {
    status.dma_state = DMASTATE_INIT;
  }

  for(int i=0;i<8;i++) {
    if(value & (1 << i)) {
      channel[i].active = true;
      channel[i].hdma_active = false;
    }
  }
}

//HDMAEN
void bCPU::mmio_w420c(uint8 value) {
  for(int i=0;i<8;i++) {
    channel[i].hdma_active = !!(value & (1 << i));
  }
}

//MEMSEL
void bCPU::mmio_w420d(uint8 value) {
  mem_bus->fastROM = value & 1;
}

//DMAPx
void bCPU::mmio_w43x0(uint8 value, uint8 i) {
  channel[i].dmap          = value;
  channel[i].direction     = !!(value & 0x80);
  channel[i].hdma_indirect = !!(value & 0x40);
  channel[i].incmode       = (value & 0x10)?-1:1;
  channel[i].fixedxfer     = !!(value & 0x08);
  channel[i].xfermode      = value & 7;
}

//BBADx
void bCPU::mmio_w43x1(uint8 value, uint8 i) {
  channel[i].destaddr = value;
}

//A1TxL
void bCPU::mmio_w43x2(uint8 value, uint8 i) {
  channel[i].srcaddr = (channel[i].srcaddr & 0xffff00) | value;
}

//A1TxH
void bCPU::mmio_w43x3(uint8 value, uint8 i) {
  channel[i].srcaddr = (channel[i].srcaddr & 0xff00ff) | (value << 8);
}

//A1Bx
void bCPU::mmio_w43x4(uint8 value, uint8 i) {
  channel[i].srcaddr = (channel[i].srcaddr & 0x00ffff) | (value << 16);
}

//DASxL
void bCPU::mmio_w43x5(uint8 value, uint8 i) {
  channel[i].xfersize = (channel[i].xfersize & 0xff00) | value;
}

//DASxH
void bCPU::mmio_w43x6(uint8 value, uint8 i) {
  channel[i].xfersize = (channel[i].xfersize & 0x00ff) | (value << 8);
}

//DASBx
void bCPU::mmio_w43x7(uint8 value, uint8 i) {
  channel[i].hdma_indirect_bank = value;
}

//A2AxL
void bCPU::mmio_w43x8(uint8 value, uint8 i) {
  channel[i].hdma_taddr = (channel[i].hdma_taddr & 0xffff00) | value;
}

//A2AxH
void bCPU::mmio_w43x9(uint8 value, uint8 i) {
  channel[i].hdma_taddr = (channel[i].hdma_taddr & 0xff00ff) | (value << 8);
}

//NTRLx
void bCPU::mmio_w43xa(uint8 value, uint8 i) {
  channel[i].hdma_line_counter = value;
}

//???
void bCPU::mmio_w43xb(uint8 value, uint8 i) {
  channel[i].hdma_unknown = value;
}

void bCPUMMIO::write(uint32 addr, uint8 value) {
uint8 i;
//cpu->sync();

//APU
  if(addr >= 0x2140 && addr <= 0x217f) {
    cpu->port_write(addr & 3, value);
    return;
  }

//HDMA
  if(addr >= 0x4300 && addr <= 0x437f) {
    i = (addr >> 4) & 7;
    switch(addr & 0xf) {
    case 0x0:cpu->mmio_w43x0(value, i);return;
    case 0x1:cpu->mmio_w43x1(value, i);return;
    case 0x2:cpu->mmio_w43x2(value, i);return;
    case 0x3:cpu->mmio_w43x3(value, i);return;
    case 0x4:cpu->mmio_w43x4(value, i);return;
    case 0x5:cpu->mmio_w43x5(value, i);return;
    case 0x6:cpu->mmio_w43x6(value, i);return;
    case 0x7:cpu->mmio_w43x7(value, i);return;
    case 0x8:cpu->mmio_w43x8(value, i);return;
    case 0x9:cpu->mmio_w43x9(value, i);return;
    case 0xa:cpu->mmio_w43xa(value, i);return;
    case 0xb:cpu->mmio_w43xb(value, i);return;
    case 0xc:return; //unmapped
    case 0xd:return; //unmapped
    case 0xe:return; //unmapped
    case 0xf:cpu->mmio_w43xb(value, i);return; //mirror of 43xb
    }
  }

  switch(addr) {
  case 0x2180:cpu->mmio_w2180(value);return; //WMDATA
  case 0x2181:cpu->mmio_w2181(value);return; //WMADDL
  case 0x2182:cpu->mmio_w2182(value);return; //WMADDM
  case 0x2183:cpu->mmio_w2183(value);return; //WMADDH
  case 0x2801:cpu->srtc_write(value);return;
  case 0x4016:cpu->mmio_w4016(value);return; //JOYSER0
  case 0x4200:cpu->mmio_w4200(value);return; //NMITIMEN
  case 0x4201:cpu->mmio_w4201(value);return; //WRIO
  case 0x4202:cpu->mmio_w4202(value);return; //WRMPYA
  case 0x4203:cpu->mmio_w4203(value);return; //WRMPYB
  case 0x4204:cpu->mmio_w4204(value);return; //WRDIVL
  case 0x4205:cpu->mmio_w4205(value);return; //WRDIVH
  case 0x4206:cpu->mmio_w4206(value);return; //WRDIVB
  case 0x4207:cpu->mmio_w4207(value);return; //HTIMEL
  case 0x4208:cpu->mmio_w4208(value);return; //HTIMEH
  case 0x4209:cpu->mmio_w4209(value);return; //VTIMEL
  case 0x420a:cpu->mmio_w420a(value);return; //VTIMEH
  case 0x420b:cpu->mmio_w420b(value);return; //DMAEN
  case 0x420c:cpu->mmio_w420c(value);return; //HDMAEN
  case 0x420d:cpu->mmio_w420d(value);return; //MEMSEL
  }
}

bCPUMMIO::bCPUMMIO(bCPU *_cpu) {
  cpu = _cpu;
}
