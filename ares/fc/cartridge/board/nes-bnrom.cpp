struct NES_BNROM : Board {
  Memory::Readable<uint8> programROM;
  Memory::Readable<uint8> characterROM;
  Memory::Writable<uint8> characterRAM;

  using Board::Board;

  auto load(Markup::Node document) -> void override {
    auto board = document["game/board"];
    Board::load(programROM, board["memory(type=ROM,content=Program)"]);
    Board::load(characterROM, board["memory(type=ROM,content=Character)"]);
    Board::load(characterRAM, board["memory(type=RAM,content=Character)"]);
    mirror = board["mirror/mode"].string() == "vertical";
  }

  auto save(Markup::Node document) -> void override {
    auto board = document["game/board"];
    Board::save(characterRAM, board["memory(type=RAM,content=Character)"]);
  }

  auto readPRG(uint address) -> uint8 {
    if(address < 0x8000) return cpu.mdr();
    return programROM.read(programBank << 15 | (uint15)address);
  }

  auto writePRG(uint address, uint8 data) -> void {
    if(address < 0x8000) return;
    programBank = data.bit(0,1);
  }

  auto readCHR(uint address) -> uint8 {
    if(address & 0x2000) {
      address = address >> !mirror & 0x0400 | address & 0x03ff;
      return ppu.readCIRAM(address);
    }
    if(characterROM) return characterROM.read(address);
    if(characterRAM) return characterRAM.read(address);
    return 0x00;
  }

  auto writeCHR(uint address, uint8 data) -> void {
    if(address & 0x2000) {
      address = address >> !mirror & 0x0400 | address & 0x03ff;
      return ppu.writeCIRAM(address, data);
    }
    if(characterRAM) return characterRAM.write(address, data);
  }

  auto power() -> void {
    programBank = 0;
  }

  auto serialize(serializer& s) -> void {
    characterRAM.serialize(s);
    s.integer(mirror);
    s.integer(programBank);
  }

  uint1 mirror;  //0 = horizontal, 1 = vertical
  uint2 programBank;
};