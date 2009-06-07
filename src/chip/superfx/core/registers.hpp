

//accepts a callback binding so r14 writes can trigger ROM buffering transparently
struct reg16_t : noncopyable {
  uint16_t data;
  function<void (uint16_t)> on_modify;

  inline operator unsigned() const { return data; }
  inline uint16_t assign(uint16_t i) {
    if(on_modify) on_modify(i);
    else data = i;
    return data;
  }

  inline unsigned operator++() { return assign(data + 1); }
  inline unsigned operator--() { return assign(data - 1); }
  inline unsigned operator++(int) { unsigned r = data; assign(data + 1); return r; }
  inline unsigned operator--(int) { unsigned r = data; assign(data - 1); return r; }
  inline unsigned operator   = (unsigned i) { return assign(i); }
  inline unsigned operator  |= (unsigned i) { return assign(data | i); }
  inline unsigned operator  ^= (unsigned i) { return assign(data ^ i); }
  inline unsigned operator  &= (unsigned i) { return assign(data & i); }
  inline unsigned operator <<= (unsigned i) { return assign(data << i); }
  inline unsigned operator >>= (unsigned i) { return assign(data >> i); }
  inline unsigned operator  += (unsigned i) { return assign(data + i); }
  inline unsigned operator  -= (unsigned i) { return assign(data - i); }
  inline unsigned operator  *= (unsigned i) { return assign(data * i); }
  inline unsigned operator  /= (unsigned i) { return assign(data / i); }
  inline unsigned operator  %= (unsigned i) { return assign(data % i); }

  inline unsigned operator   = (const reg16_t& i) { return assign(i); }

  reg16_t() : data(0) {}
};

struct sfr_t {
  bool irq;   //interrupt flag
  bool b;     //WITH flag
  bool ih;    //immediate higher 8-bit flag
  bool il;    //immediate lower 8-bit flag
  bool alt2;  //ALT2 mode
  bool alt1;  //ALT2 instruction mode
  bool r;     //ROM r14 read flag
  bool g;     //GO flag
  bool ov;    //overflow flag
  bool s;     //sign flag
  bool cy;    //carry flag
  bool z;     //zero flag

  operator uint16_t() const {
    return (irq << 15) | (b << 12) | (ih << 11) | (il << 10) | (alt2 << 9) | (alt1 << 8)
         | (r << 6) | (g << 5) | (ov << 4) | (s << 3) | (cy << 2) | (z << 1);
  }

  sfr_t& operator=(uint16_t data) {
    irq  = data & 0x8000;
    b    = data & 0x1000;
    ih   = data & 0x0800;
    il   = data & 0x0400;
    alt2 = data & 0x0200;
    alt1 = data & 0x0100;
    r    = data & 0x0040;
    g    = data & 0x0020;
    ov   = data & 0x0010;
    s    = data & 0x0008;
    cy   = data & 0x0004;
    z    = data & 0x0002;
    return *this;
  }
};

struct scmr_t {
  unsigned ht;
  bool ron;
  bool ran;
  unsigned md;

  operator uint8_t() const {
    return ((ht >> 1) << 5) | (ron << 4) | (ran << 3) | ((ht & 1) << 2) | (md);
  }

  scmr_t& operator=(uint8_t data) {
    ht  = (bool)(data & 0x20) << 1;
    ht |= (bool)(data & 0x04) << 0;
    ron = data & 0x10;
    ran = data & 0x08;
    md  = data & 0x03;
    return *this;
  }
};

struct por_t {
  bool obj;
  bool freezehigh;
  bool highnibble;
  bool dither;
  bool transparent;

  operator uint8_t() const {
    return (obj << 4) | (freezehigh << 3) | (highnibble << 2) | (dither << 1) | (transparent);
  }

  por_t& operator=(uint8_t data) {
    obj         = data & 0x10;
    freezehigh  = data & 0x08;
    highnibble  = data & 0x04;
    dither      = data & 0x02;
    transparent = data & 0x01;
    return *this;
  }
};

struct cfgr_t {
  bool irq;
  bool ms0;

  operator uint8_t() const {
    return (irq << 7) | (ms0 << 5);
  }

  cfgr_t& operator=(uint8_t data) {
    irq = data & 0x80;
    ms0 = data & 0x20;
    return *this;
  }
};

struct regs_t {
  uint8_t pipeline;
  uint16_t ramaddr;

  reg16_t r[16];    //general purpose registers
  sfr_t sfr;        //status flag register
  uint8_t pbr;      //program bank register
  uint8_t rombr;    //game pack ROM bank register
  bool rambr;       //game pack RAM bank register
  uint16_t cbr;     //cache base register
  uint8_t scbr;     //screen base register
  scmr_t scmr;      //screen mode register
  uint8_t colr;     //color register
  por_t por;        //plot option register
  bool bramr;       //back-up RAM register
  uint8_t vcr;      //version code register
  cfgr_t cfgr;      //config register
  bool clsr;        //clock select register

  unsigned romcl;   //clock ticks until romdr is valid
  uint8_t romdr;    //ROM buffer data register

  unsigned ramcl;   //clock ticks until ramdr is valid
  uint16_t ramar;   //RAM buffer address register
  uint8_t ramdr;    //RAM buffer data register

  reg16_t *sreg, *dreg;
  reg16_t& sr() { return *sreg; }  //source register (from)
  reg16_t& dr() { return *dreg; }  //destination register (to)

  void reset() {
    sfr.b    = 0;
    sfr.alt1 = 0;
    sfr.alt2 = 0;

    sreg = &r[0];
    dreg = &r[0];
  }
} regs;

struct cache_t {
  uint8_t buffer[512];
  bool valid[32];
} cache;

struct pixelcache_t {
  uint16_t offset;
  uint8_t bitpend;
  uint8_t data[8];
} pixelcache[2];
