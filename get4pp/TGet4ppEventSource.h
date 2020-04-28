//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum f�r Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------
////////////////////////////////////
// Event Source for Get4pp data format
// V 1.0 24-Jan-2011 skeleton from XSYS binary file source
// V 1.1 28-Apr-2020 adjusted to process Get4++ chip simulation data files
// Joern Adamczewski-Musch, GSI Darmstadt
// j.adamczewski@gsi.de



#ifndef TGet4ppEVENTSOURCE_H
#define TGet4ppEVENTSOURCE_H

#include "TGo4EventSource.h"
#include <fstream>

#include "TGo4MbsEvent.h"

#include "Riostream.h"
using namespace std;

#define Get4pp_BUFSIZE 0x1000
#define Get4pp_CHUNKSIZE 0x30

#define Get4pp_PROCID  0x55

class TGet4ppRawEvent;
class TGo4UserSourceParameter;



class TGet4ppEventSource : public TGo4EventSource {
   public:

      TGet4ppEventSource();

      /** Create source specifying values directly */
      TGet4ppEventSource(const char* name, const char* args, Int_t port);

      /** Creat source from setup within usersource parameter object */
      TGet4ppEventSource(TGo4UserSourceParameter* par);

      virtual ~TGet4ppEventSource();


      /** Open the file or connection. */
      virtual Int_t Open();

      /** Close the file or connection. */
      virtual Int_t Close();


      /** This method checks if event class is suited for the source */
      virtual Bool_t CheckEventClass(TClass* cl);

      /** This methods actually fills the target event class which is passed as pointer.
        * Uses the latest event which is referenced
        * by fxEvent or fxBuffer. Does _not_ fetch a new event
        * from source, therefore one source event may be used
        * to fill several TGet4ppEvent classes. To get a new
        * event call NextEvent() before this method.*/
      virtual Bool_t BuildEvent(TGo4EventElement* dest);

      const char* GetArgs() const { return fxArgs.Data(); }

      void SetArgs(const char* arg) { fxArgs=arg; }

      Int_t GetPort() const { return fiPort; }

      void SetPort(Int_t val) { fiPort=val; }

   protected:

      /* read next buffer from file into memory*/
      Bool_t NextBuffer();

      /* Fill next xsys event from buffer location into output structure*/
      Bool_t NextEvent(TGo4MbsEvent* target);

      /* Read from input file and check*/
      std::streamsize ReadFile(Char_t* dest, size_t len);

   private:

      Bool_t fbIsOpen;

      /** Optional argument string */
      TString fxArgs;

      /** Optional port number  */
      Int_t fiPort;

      /** file that contains the data in ascii format. */
      std::ifstream* fxFile; //!



      /* true if next buffer needs to be read from file*/
      Bool_t fbNeedNewBuffer;


      /* for spanned events: remember current event ids*/
      TGo4SubEventHeader10 fxSubevHead;

      /* file read buffer*/
      Char_t* fxBuffer; //!

      /* cursor in read buffer*/
      Char_t* fxCursor; //!

      /** This buffer contains formatted MBS subevent data*/
      Short_t* fxEventBuffer; //!


      /** current data pointer*/
      Int_t* fxEventData; //!


      /* actual size of bytes read into buffer most recently*/
      std::streamsize fxBufsize; //!





      /* Length of current event data in shorts*/
      Int_t fiEventLen;




   ClassDef(TGet4ppEventSource, 1)
};

#endif //TGO4MBSSOURCE_H
