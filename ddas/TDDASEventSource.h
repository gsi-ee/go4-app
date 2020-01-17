//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#ifndef TDDASEVENTSOURCE_H
#define TDDASEVENTSOURCE_H

#include "TGo4EventSource.h"
#include <fstream>

class TDDASRawEvent;
class DDASEvent;
class TGo4UserSourceParameter;

class TFile;
class TTree;
class TList;
class TGo4EventElement;


class TDDASEventSource : public TGo4EventSource {
   public:

      TDDASEventSource();

      /** Create source specifying values directly */
      TDDASEventSource(const char* name);

      /** Creat source from setup within usersource parameter object */
      TDDASEventSource(TGo4UserSourceParameter* par);




      virtual ~TDDASEventSource();




      /** This method checks if event class is suited for the source */
      virtual Bool_t CheckEventClass(TClass* cl);

      /** This methods actually fills the target event class which is passed as pointer.
        * Uses the latest event which is referenced
        * by fxEvent or fxBuffer. Does _not_ fetch a new event
        * from source, therefore one source event may be used
        * to fill several TDDASEvent classes. To get a new
        * event call NextEvent() before this method.*/
      virtual Bool_t BuildEvent(TGo4EventElement* dest);



   protected:



     /** JAM 2020: following was stolen from TGo4FileSource and may be implemented accordingly:*/


      TFile* fxFile; //!

      TTree * fxTree; //!

      DDASEvent* fDDASEventPointer; //!

         /** Number of events stored in the Tree. Used to check if
           * the complete tree is already read. */
      Long64_t fiMaxEvents;

         /** Event number in current tree. */
      Long64_t fiCurrentEvent;

         /** Global event number, starting from the first tree. */
      long int fiGlobalEvent;   //!

      /** This flag is used for lazy init of tree in Eventbuilding methods. */
      Bool_t fbActivated; //!

         /** list of files names */
      TList* fxFilesNames; //!

         /** current name of the file */
      TString fxCurrentFileName;  //!

      /** evalulate list of filenames and open first file*/
      void Init();

         /** Open next file from the files list. */
      Bool_t OpenNextFile();

         /** Close currently open file. */
      Bool_t CloseCurrentFile();


   ClassDef(TDDASEventSource, 1)
};

#endif //TGO4MBSSOURCE_H
