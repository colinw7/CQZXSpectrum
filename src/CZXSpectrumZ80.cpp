#include <CZXSpectrum.h>
#include <CZXSpectrumZ80.h>
#include <CFile.h>

struct CZXSpectrumZ80Header1 {
  uchar  a   , f;
  uchar  c   , b;
  uchar  l   , h;
  uchar  pcl , pch;
  uchar  spl , sph;
  uchar  i   , r;
  uchar  fl1 ;
  uchar  e   , d;
  uchar  c1  , b1;
  uchar  e1  , d1;
  uchar  l1  , h1;
  uchar  a1  , f1;
  uchar  iyl , iyh;
  uchar  ixl , ixh;
  uchar  iff1, iff2;
  uchar  im  ;
};

struct CZXSpectrumZ80Header2 {
  ushort len;
  ushort pc;
  uchar  hmode;
  uchar  samram;
  uchar  iface1;
  uchar  flags1;
  uchar  sout1;
  uchar  sbytes[16];
};

struct CZXSpectrumZ80Header3 {
  ushort len;
  ushort pc;
  uchar  hmode;
  uchar  samram;
  uchar  iface1;
  uchar  flags1;
  uchar  sout1;
  uchar  sbytes[16];
  ushort lowT;
  uchar  hiT;
  uchar  flags2;
  uchar  mgtf;
  uchar  multiface;
  uchar  lrom;
  uchar  hrom;
  uchar  keys1[10];
  uchar  keys2[10];
  uchar  mgt1;
  uchar  disciple1;
  uchar  Disciple2;
  uchar  out1;
};

CZXSpectrumZ80::
CZXSpectrumZ80(CZXSpectrum &spectrum, const std::string &filename) :
 spectrum_(spectrum), filename_(filename)
{
}

CZXSpectrumZ80::
~CZXSpectrumZ80()
{
}

bool
CZXSpectrumZ80::
load()
{
  CFile file(filename_);

  if (! file.exists() || ! file.isRegular()) {
    std::cerr << "Invalid file " << filename_ << std::endl;
    return false;
  }

  CZXSpectrumZ80Header1 header1;

  if (! file.read((uchar *) &header1, sizeof(header1)))
    return false;

  if (header1.pch == 0 && header1.pcl == 0) {
    // version 2/3 file

    CZXSpectrumZ80Header2 header2;

    memset(&header2, 0, sizeof(CZXSpectrumZ80Header2));

    if (! file.read((uchar *) &header2, 25))
      return false;

    std::cerr << header2.len << std::endl;

    if (header2.len == 23)
      load2(file, header1, header2);
    else {
      CZXSpectrumZ80Header3 header3;

      memset(&header3, 0, sizeof(CZXSpectrumZ80Header3));

      file.setRPos(-25);

      if (! file.read((uchar *) &header3, header2.len + 2))
        return false;

      load3(file, header1, header3);
    }

    return false;
  }
  else
    return load1(file, header1);
}

bool
CZXSpectrumZ80::
load1(CFile &file, CZXSpectrumZ80Header1 &header)
{
  if (header.fl1 == 0xff)
    header.fl1 = 0x01;

  uchar *memory = new uchar [48*1024];

  uint pos = 0;

  uchar c;

  if (TST_BIT(header.fl1, 5)) { // compressed
    while (file.read(&c, 1)) {
      if (c == 0xed) {
        if (! file.read(&c, 1)) {
          std::cerr << "Invalid data" << std::endl;
          return false;
        }

        if (c == 0xed) {
          uchar x, y;

          if (! file.read(&x, 1)) {
            std::cerr << "Invalid data" << std::endl;
            return false;
          }

          if (x == 0)
            break;

          if (! file.read(&y, 1)) {
            std::cerr << "Invalid data" << std::endl;
            return false;
          }

          for ( ; x > 0; --x)
            memory[pos++] = y;
        }
        else {
          memory[pos++] = 0xed;
          memory[pos++] = c;
        }
      }
      else
        memory[pos++] = c;
    }
  }
  else {
    while (file.read(&c, 1))
      memory[pos++] = c;
  }

  CZ80 *z80 = spectrum_.getZ80();

  z80->setAF(header.a << 8 | header.f);
  z80->setBC(header.b << 8 | header.c);
  z80->setDE(header.d << 8 | header.e);
  z80->setHL(header.h << 8 | header.l);

  z80->setSP(header.sph << 8 | header.spl);

  z80->setAF1(header.a1 << 8 | header.f1);
  z80->setBC1(header.b1 << 8 | header.c1);
  z80->setDE1(header.d1 << 8 | header.e1);
  z80->setHL1(header.h1 << 8 | header.l1);

  z80->setIX(header.ixh << 8 | header.ixl);
  z80->setIY(header.iyh << 8 | header.iyl);

  z80->setIFF(header.iff1 << 8 | header.iff2);

  z80->setI(header.i);
  z80->setR(header.r);

  z80->setPC(header.pch << 8 | header.pcl);

  z80->setBytes(memory, 16*1024, 48*1024);

  delete [] memory;

  return true;
}

bool
CZXSpectrumZ80::
load2(CFile &file, CZXSpectrumZ80Header1 &header, CZXSpectrumZ80Header2 &header1)
{
  uchar *memory = new uchar [65536];

  memset(memory, 0, 48*1024);

  uint pos1 = 0x4000;
  uint pos2 = 0x7FFF;

  while (true) {
    ushort len;

    if (! file.read((uchar *) &len, sizeof(len)))
      break;

    uchar num;

    if (! file.read((uchar *) &num, sizeof(num)))
      return false;

    if      (num == 4) {
      pos1 = 0x8000;
      pos2 = 0xBFFF;
    }
    else if (num == 5) {
      pos1 = 0xC000;
      pos2 = 0xFFFF;
    }
    else if (num == 8) {
      pos1 = 0x4000;
      pos2 = 0x7FFF;
    }
    else {
      std::cerr << "Unsupported type:" << int(num) << std::endl;
      return false;
    }

    std::cerr << "Len " << len << " " << int(num) << std::endl;

    uint len1 = 0;

    uchar c;

    while (len1 < len && file.read(&c, 1)) {
      ++len1;

      if (c == 0xed) {
        if (! file.read(&c, 1)) {
          std::cerr << "Invalid data" << std::endl;
          return false;
        }

        ++len1;

        if (c == 0xed) {
          uchar x, y;

          if (! file.read(&x, 1)) {
            std::cerr << "Invalid data" << std::endl;
            return false;
          }

          ++len1;

          if (x == 0)
            break;

          if (! file.read(&y, 1)) {
            std::cerr << "Invalid data" << std::endl;
            return false;
          }

          ++len1;

          for ( ; x > 0; --x) {
            if (pos1 < pos2) {
              memory[pos1 - 0x4000] = y; ++pos1;
            }
            else {
              std::cerr << "Invalid memory set 1: " << std::hex << pos1 << " " <<
                           std::hex << int(y) << std::endl;
            }
          }
        }
        else {
          if (pos1 < pos2 - 1) {
            memory[pos1 - 0x4000] = 0xed; ++pos1;
            memory[pos1 - 0x4000] = c   ; ++pos1;
          }
          else {
            std::cerr << "Invalid memory set 2: " << std::hex << pos1 << " " << 0xed << " " <<
                         std::hex << int(c) << std::endl;
          }
        }
      }
      else {
        if (pos1 < pos2) {
          memory[pos1 - 0x4000] = c; ++pos1;
        }
        else {
          std::cerr << "Invalid memory set 3: " << std::hex << pos1 << " " <<
                       std::hex << int(c) << std::endl;
        }
      }
    }
  }

  CZ80 *z80 = spectrum_.getZ80();

  z80->setAF(header.a << 8 | header.f);
  z80->setBC(header.b << 8 | header.c);
  z80->setDE(header.d << 8 | header.e);
  z80->setHL(header.h << 8 | header.l);

  z80->setSP(header.sph << 8 | header.spl);

  z80->setAF1(header.a1 << 8 | header.f1);
  z80->setBC1(header.b1 << 8 | header.c1);
  z80->setDE1(header.d1 << 8 | header.e1);
  z80->setHL1(header.h1 << 8 | header.l1);

  z80->setIX(header.ixh << 8 | header.ixl);
  z80->setIY(header.iyh << 8 | header.iyl);

  z80->setIFF(header.iff1 << 8 | header.iff2);

  z80->setI(header.i);
  z80->setR(header.r);

  z80->setPC(header1.pc);

  z80->setBytes(memory, 16*1024, 48*1024);

  delete [] memory;

  return false;
}

bool
CZXSpectrumZ80::
load3(CFile &file, CZXSpectrumZ80Header1 &header, CZXSpectrumZ80Header3 &header1)
{
  uchar *memory = new uchar [65536];

  memset(memory, 0, 48*1024);

  uint pos1 = 0x4000;
  uint pos2 = 0x7FFF;

  while (true) {
    ushort len;

    if (! file.read((uchar *) &len, sizeof(len)))
      break;

    uchar num;

    if (! file.read((uchar *) &num, sizeof(num)))
      return false;

    if      (num == 4) {
      pos1 = 0x8000;
      pos2 = 0xBFFF;
    }
    else if (num == 5) {
      pos1 = 0xC000;
      pos2 = 0xFFFF;
    }
    else if (num == 8) {
      pos1 = 0x4000;
      pos2 = 0x7FFF;
    }
    else {
      std::cerr << "Unsupported type:" << int(num) << std::endl;
      return false;
    }

    std::cerr << "Len " << len << " " << int(num) << std::endl;

    uint len1 = 0;

    uchar c;

    while (len1 < len && file.read(&c, 1)) {
      ++len1;

      if (c == 0xed) {
        if (! file.read(&c, 1)) {
          std::cerr << "Invalid data" << std::endl;
          return false;
        }

        ++len1;

        if (c == 0xed) {
          uchar x, y;

          if (! file.read(&x, 1)) {
            std::cerr << "Invalid data" << std::endl;
            return false;
          }

          ++len1;

          if (x == 0)
            break;

          if (! file.read(&y, 1)) {
            std::cerr << "Invalid data" << std::endl;
            return false;
          }

          ++len1;

          for ( ; x > 0; --x) {
            if (pos1 < pos2) {
              memory[pos1 - 0x4000] = y; ++pos1;
            }
            else {
              std::cerr << "Invalid memory set 1: " << std::hex << pos1 << " " <<
                           std::hex << int(y) << std::endl;
            }
          }
        }
        else {
          if (pos1 < pos2 - 1) {
            memory[pos1 - 0x4000] = 0xed; ++pos1;
            memory[pos1 - 0x4000] = c   ; ++pos1;
          }
          else {
            std::cerr << "Invalid memory set 2: " << std::hex << pos1 << " " << 0xed << " " <<
                         std::hex << int(c) << std::endl;
          }
        }
      }
      else {
        if (pos1 < pos2) {
          memory[pos1 - 0x4000] = c; ++pos1;
        }
        else {
          std::cerr << "Invalid memory set 3: " << std::hex << pos1 << " " <<
                       std::hex << int(c) << std::endl;
        }
      }
    }
  }

  CZ80 *z80 = spectrum_.getZ80();

  z80->setAF(header.a << 8 | header.f);
  z80->setBC(header.b << 8 | header.c);
  z80->setDE(header.d << 8 | header.e);
  z80->setHL(header.h << 8 | header.l);

  z80->setSP(header.sph << 8 | header.spl);

  z80->setAF1(header.a1 << 8 | header.f1);
  z80->setBC1(header.b1 << 8 | header.c1);
  z80->setDE1(header.d1 << 8 | header.e1);
  z80->setHL1(header.h1 << 8 | header.l1);

  z80->setIX(header.ixh << 8 | header.ixl);
  z80->setIY(header.iyh << 8 | header.iyl);

  z80->setIFF(header.iff1 << 8 | header.iff2);

  z80->setI(header.i);
  z80->setR(header.r);

  z80->setPC(header1.pc);

  z80->setBytes(memory, 16*1024, 48*1024);

  delete [] memory;

  return false;
}
