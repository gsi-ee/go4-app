
#include "TXSYSUnpackProc.h"

#include "Riostream.h"

#include "TClass.h"
#include "TGo4MbsEvent.h"
#include "TGo4MbsSubEvent.h"
#include "TGo4UserException.h"
#include "TXSYSUnpackEvent.h"
#include "TXSYSEventSource.h"


//***********************************************************
TXSYSUnpackProc::TXSYSUnpackProc() :
   TGo4EventProcessor()
{
}
//***********************************************************
TXSYSUnpackProc::TXSYSUnpackProc(const char* name) :
   TGo4EventProcessor(name)

{
   cout << "**** TXSYSUnpackProc: Create" << endl;

   //// init user analysis objects:
   TString obname;
   TString obtitle;
   TString foldername;


   for(int sc=0; sc<XSYS_CHANNELS;++sc)
   	{
	   obname.Form("Raw/Spectrum%2d", sc);
	   obtitle.Form("Raw spectrum %2d",sc);
	   hRaw[sc]=MakeTH1('I', obname.Data(), obtitle.Data(),XSYS_BINS,0,XSYS_BINS);
   	}

   for(int ci=0; ci<NUM_CONDS;++ci)
     	{
		   obname.Form("Region%2d", ci);
		   cCondition[ci]=MakeWinCond(obname.Data(), 200, 2000, hRaw[4]->GetName());
     	}

   fPar=(TXSYSParam *) MakeParameter("XSYSPar", "TXSYSParam");
}
//***********************************************************
TXSYSUnpackProc::~TXSYSUnpackProc()
{

}
//***********************************************************

Bool_t TXSYSUnpackProc::CheckEventClass(TClass* cl)
{
   return cl->InheritsFrom(TXSYSUnpackEvent::Class());
}


//-----------------------------------------------------------

Bool_t TXSYSUnpackProc::BuildEvent(TGo4EventElement* dest)
{
   static int evcount=0;
   static int failcount=0;

	TGo4MbsEvent *inp = dynamic_cast<TGo4MbsEvent*> (GetInputEvent());
   TXSYSUnpackEvent* poutevt = (TXSYSUnpackEvent*) (dest);

   if ((inp==0) || (poutevt==0)) {
      cout << "XSYSUnpackProc: events are not specified!"<< endl;
      return kFALSE;
   }
    inp->ResetIterator();
    TGo4MbsSubEvent* psubevt=0;
    while((psubevt = inp->NextSubEvent()) != 0)
    {
    	 // loop over subevents
         Int_t *pdata = psubevt->GetDataField();
         Int_t lwords = psubevt->GetIntLen();
         if( psubevt->GetProcid() != XSYS_PROCID)
         {
     		//GO4_STOP_ANALYSIS_MESSAGE("**** TXSYSUnpackProc: procid id %d differs from xsys procid %d, stop it",psubevt->GetProcid(), XSYS_PROCID);
			cout <<"XSYSUnpackProc - skipping wrong procid"<< psubevt->GetProcid()<<" subevent." << endl;
        	continue;
         }
         unsigned recid=(unsigned) ((psubevt->GetSubcrate()) & 0xFF);
         if(recid == 0xFE)
         {
        	 cout <<"XSYSUnpackProc - skipping BEGIN  record " << endl;

         }
         else if(recid == 0xFF)
         {
        	 cout <<"XSYSUnpackProc - skipping END  record " << endl;
         }
         else if(recid > 0x8) //0xEF
		  {
			 //cout <<"XSYSUnpackProc - skipping Control  record of type "<< recid<< endl;
		  }
         else
         {
        	 //cout <<"XSYSUnpackProc - subevent of id "<< (int) psubevt->GetSubcrate() << endl;
				 // fill poutevt here:
				   poutevt->fLen=lwords*2;
				   UShort_t* psdata= (UShort_t*) pdata;
				   //cout <<"event len is "<<lwords << endl;
				   for(Int_t t=0;t<poutevt->fLen; ++t)
				   {
					   if(t<XSYS_CHANNELS)
						   {
							   UShort_t data=(psdata[t] & 0xFFF);
							   UChar_t snum=((psdata[t] >> 12) & 0xF);
							   if(snum!=t)
							   {
								  failcount++;
								  cout <<"Spectrum of index "<<t<<" has wrong id "<< (int) snum<< "for event:"<<inp->GetCount() << endl;
								  cout <<" evcount:"<<evcount<<" failcount:"<<failcount << endl;
							   }
							   poutevt->fSpecID[t]=snum;
							   poutevt->fSpectrum[t]=data;
							   hRaw[t]->Fill(data);
						   }
				   }
         }
    } // while
    evcount++;
    return kTRUE;
}
