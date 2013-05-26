class CZXSpectrumTZX {
 public:
  CZXSpectrumTZX(CZXSpectrum &spectrum, const std::string &filename);
 ~CZXSpectrumTZX();

  bool load();

 private:
  CZXSpectrum &spectrum_;
  std::string  filename_;
  uint         eof_;
  uint         ver1_, ver2_;
  uint         type_, pc_, mem_, len_;
};
