#include "CZXSpectrum.h"
#include "CZXSpectrumTZX.h"
#include "CFile.h"

using std::string;
using std::cerr;
using std::endl;

CZXSpectrumTZX::
CZXSpectrumTZX(CZXSpectrum &spectrum, const string &filename) :
 spectrum_(spectrum), filename_(filename)
{
}

CZXSpectrumTZX::
~CZXSpectrumTZX()
{
}

bool
CZXSpectrumTZX::
load()
{
  CFile file(filename_);

  uchar buffer[10];

  if (! file.read(buffer, 10))
    return false;

  if (strncmp((const char *) buffer, "ZXTape!", 7) != 0)
    return false;

  eof_ = buffer[7];

  ver1_ = buffer[8];
  ver2_ = buffer[9];

  cerr << "EOF : " << eof_  << endl;
  cerr << "VER1: " << ver1_ << endl;
  cerr << "VER2: " << ver2_ << endl;

  while (true) {
    uchar id;

    if (! file.read(&id, 1))
      return false;

    cerr << "Id: " << std::hex << int(id) << std::dec << endl;

    switch (id) {
      case 0x10: {
        cerr << "Speed Block" << endl;

        ushort pause;

        if (! file.read((uchar *) &pause, 2))
          return false;

        ushort len;

        if (! file.read((uchar *) &len, 2))
          return false;

        cerr << "Pause: " << pause << endl;
        cerr << "Len  : " << len   << endl;

        uchar flag;

        if (! file.read((uchar *) &flag, 1))
          return false;

        --len;

        uchar *data = new uchar [len];

        if (! file.read(data, len))
          return false;

        uchar sum = flag;

        for (uint i = 0; i < len; ++i)
          sum ^= data[i];

        if (sum != 0) {
          cerr << "Invalid checksum " << int(sum) << endl;
          return false;
        }

        --len;

        if      (flag == 0x00) { // header
          if (len != 17)
            return false;

          cerr << "Header" << endl;

          int type = int(data[0]);

          char filename[11];

          memcpy(filename, &data[1], 10);
          filename[10] = '\0';

          ushort len1, param1, param2;

          memcpy(&len1  , &data[11], 2);
          memcpy(&param1, &data[13], 2);
          memcpy(&param2, &data[15], 2);

          if      (type == 0) {
            cerr << " Program" << endl;

            pc_ = param1;

            type_ = type;
            mem_  = param1;
            len_  = len1;

            cerr << "  Filename: \"" << filename << "\"" << endl;

            cerr << "  Len     : " << int(len1) << endl;
            cerr << "  Start   : " << pc_  << endl;
            cerr << "  Vars    : " << param2 << endl;
          }
          else if (type == 3) {
            cerr << " Code" << endl;

            type_ = type;
            mem_  = param1;
            len_  = len1;

            cerr << "  Filename: \"" << filename << "\"" << endl;
            cerr << "  Len     : " << int(len1) << endl;
            cerr << "  Param1  : " << param1 << endl;
            cerr << "  Param2  : " << param2 << endl;
          }
          else {
            cerr << " Type " << int(data[0]) << endl;

            cerr << "  Filename: \"" << filename << "\"" << endl;
            cerr << "  Len     : " << int(len1) << endl;
            cerr << "  Param1  : " << param1 << endl;
            cerr << "  Param2  : " << param2 << endl;

            return false;
          }

          break;
        }

        if (flag != 0xff)
          return false;

        if      (type_ == 0) {
          cerr << "Data" << endl;

          int col = 0;

          for (uint i = 0; i < len; ++i, ++col) {
            if (col > 0 && (col % 8) == 0)
              cerr << endl;

            cerr << int(data[i]) << " ";
          }

          cerr << endl;

          //----

          spectrum_.decodeBasic(data, len);
        }
        else if (type_ == 3)
          spectrum_.getZ80()->setBytes(data, mem_, len_);

        delete [] data;

        break;
      }
      case 0x30: {
        cerr << "Text Description" << endl;

        uchar len;

        if (! file.read((uchar *) &len, 1))
          return false;

        char *text = new char [len + 1];

        if (! file.read((uchar *) text, len))
          return false;

        text[len] = '\0';

        cerr << "Text: " << text << endl;

        delete [] text;

        break;
      }
      default:
        return false;
    }
  }

  spectrum_.getZ80()->setPC(pc_);

  return true;
}
