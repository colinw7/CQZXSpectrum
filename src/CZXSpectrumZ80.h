struct CZXSpectrumZ80Header1;
struct CZXSpectrumZ80Header2;
struct CZXSpectrumZ80Header3;

class CZXSpectrumZ80 {
 public:
  CZXSpectrumZ80(CZXSpectrum &spectrum, const std::string &filename);
 ~CZXSpectrumZ80();

  bool load();

 private:
  bool load1(CFile &file, CZXSpectrumZ80Header1 &header);
  bool load2(CFile &file, CZXSpectrumZ80Header1 &header, CZXSpectrumZ80Header2 &header1);
  bool load3(CFile &file, CZXSpectrumZ80Header1 &header, CZXSpectrumZ80Header3 &header1);

 private:
  CZXSpectrum &spectrum_;
  std::string  filename_;
};
