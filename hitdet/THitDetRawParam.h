
#ifndef THitDetRawParam_H
#define THitDetRawParam_H

#include "TGo4Parameter.h"
#include "THitDetRawEvent.h"

#define HitDet_FILTERCOEFFS 5


class THitDetRawParam : public TGo4Parameter {
   public:

    enum FilterTypes
    {
      FIL_NONE = 0,
      FIL_VONHANN = 1,
      FIL_HAMMING = 2,
      FIL_BLACKMAN = 3,
      FIL_NUTALL = 4,
      FIL_BLACKNUTALL = 5,
      FIL_BLACKHARRIS = 6,
      FIL_SRSFLATTOP = 7,
      FIL_USER = 8
    };


      THitDetRawParam(const char* name = 0);

      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Int_t fBoardID[HitDet_MAXBOARDS]; // mapping of board numbers to (slot?) indices

      Int_t fNumSnapshots; // number of trace snapshots per mbs event
      Int_t fTraceLength; // length of trace to visualize (8, 16, 32)
      Bool_t fDoFFT; // do fast fourier transform on trace long when complete
      TString fFFTOptions; // ROOT fft option string
      Int_t fFilterType; // pre-FFT filter type 0-none 1-VonHann 2-Hamming 3-Blackmann 4-Nutall 5-BlackmannNutall 6-Blackmann-Harris 7-FlatTop 8-User
      Double_t fFilterCoeff[HitDet_FILTERCOEFFS]; // coefficient for generalized cosine window filters



      /* initial mapping of board id to sfp/ gosip device cabling*/
      void InitBoardMapping();

      /* copy board configuration from this parameter to static event members
       * before initialization. Usually this is done in UpdateFrom*/
      Bool_t SetConfigBoards();

   private:



   ClassDef(THitDetRawParam,2)
};

#endif // THitDetRawParam_H
