
#include "TXSYSEventSource.h"

#include "TClass.h"
#include "Riostream.h"
#include <stdlib.h>

#include "TGo4EventErrorException.h"
#include "TGo4EventEndException.h"
#include "TGo4EventTimeoutException.h"
#include "TGo4UserSourceParameter.h"
#include "TGo4MbsEvent.h"
#include "TGo4Log.h"



TXSYSEventSource::TXSYSEventSource(const char* name,
                                 const char* args,
                                 Int_t port) :
   TGo4EventSource(name),
   fbIsOpen(kFALSE),
   fxArgs(args),
   fiPort(port),
   fxFile(0),
   fbNeedNewBuffer(kTRUE),
   fbIsBufferSpanning(kFALSE),
   fxBuffer(0),
   fxBufsize(0),
   fxCursor(0),
   fxBufEnd(0),
   fxPrevEventCursor(0),
   fxSpanBuffer(0),
   fxSpanCursor(0),
   fxBufhead(0),
   fxEventhead(0)
{
   Open();
}

TXSYSEventSource::TXSYSEventSource(TGo4UserSourceParameter* par) :
   TGo4EventSource(" "),
   fbIsOpen(kFALSE),
    fxArgs(""),
    fiPort(0),
    fxFile(0),
    fbNeedNewBuffer(kTRUE),
    fbIsBufferSpanning(kFALSE),
    fxBuffer(0),
    fxBufsize(0),
    fxCursor(0),
    fxBufEnd(0),
    fxPrevEventCursor(0),
    fxSpanBuffer(0),
    fxSpanCursor(0),
    fxBufhead(0),
    fxEventhead(0)
{
   if(par) {
      SetName(par->GetName());
      SetPort(par->GetPort());
      SetArgs(par->GetExpression());
      Open();
   } else {
      cout <<"TXSYSEventSource constructor with zero parameter!" << endl;
   }
}

TXSYSEventSource::TXSYSEventSource() :
   TGo4EventSource("default XSYS source"),
   fbIsOpen(kFALSE),
    fxArgs(""),
    fiPort(0),
    fxFile(0),
    fbNeedNewBuffer(kTRUE),
    fbIsBufferSpanning(kFALSE),
    fxBuffer(0),
    fxBufsize(0),
    fxCursor(0),
    fxBufEnd(0),
    fxPrevEventCursor(0),
    fxSpanBuffer(0),
    fxSpanCursor(0),
    fxBufhead(0),
    fxEventhead(0)
{
}

TXSYSEventSource::~TXSYSEventSource()
{
   Close();
}

Bool_t TXSYSEventSource::CheckEventClass(TClass* cl)
{
   return cl->InheritsFrom(TGo4MbsEvent::Class());
}

std::streamsize TXSYSEventSource::ReadFile(Char_t* dest, size_t len)
{
	fxFile->read(dest, len);
	if(fxFile->eof() || !fxFile->good())
			{
					SetCreateStatus(1);
					SetErrMess(Form("End of input file %s", GetName()));
					SetEventStatus(1);
					throw TGo4EventEndException(this);
			}
	//cout <<"ReadFile reads "<< (hex) << fxFile->gcount()<<" bytes to 0x"<< (hex) <<(int) dest<< endl;
	return fxFile->gcount();
}



Bool_t TXSYSEventSource::NextBuffer()
{
	if(!fbNeedNewBuffer) return kTRUE;
	// first read buffer header:
	ReadFile(fxBuffer, sizeof(XSYS_buffer_head));
	fxBufhead=(XSYS_buffer_head*)(fxBuffer);
	//cout <<"Next Buffer expects buffer of size:"<<fxBufhead->fBufsize<< endl;

	// then read rest of buffer from file
	ReadFile(fxBuffer+sizeof(XSYS_buffer_head), fxBufhead->fBufsize - sizeof(XSYS_buffer_head));
	fxBufsize=fxBufhead->fBufsize;
	//cout <<"Next Buffer size to "<< (hex) << fxBufsize<<" bytes"<< endl;
	fxBufEnd=(Short_t*)( fxBuffer+ fxBufsize);

	// evaluate rest of buffer header:
	fxCursor=(Short_t*)(fxBuffer); // cursor to start of buffer
	Short_t buftype=((fxBufhead->fIdentity >> 10) & 0x3F);
	Short_t bufheadleng=(fxBufhead->fIdentity & 0x1FF);
	Short_t bufoff=0;
	//cout <<"Next Buffer got buffer identity:"<< (hex) << fxBufhead->fIdentity<< endl;
	//cout <<"Next Buffer got buffer header length " << (hex) << bufheadleng<< endl;
	//cout <<"Next Buffer got buffer type "<<buftype<< endl;
	switch(buftype)
		{
		case 0x39: /* four byte header*/
			//cout <<"...... 4 byte header"<<endl;
			fxPrevEventCursor=fxCursor+2;
			fiPrevEventLen=bufheadleng; // in this case, bufheadleng is number of short words
			fxCursor+=bufheadleng+2; // skip over header and previous event data  to first event data
			break;
		case 0x3A: /* eight or more bytes*/
			//cout <<"...... >8 byte header"<<endl;
			bufoff=fxBufhead->fOffset;
			//cout <<"Next Buffer got offset (bytes):"<<bufoff<< endl;
			fxPrevEventCursor=fxCursor+ bufheadleng/sizeof(Short_t); // here bufheadleng is number of bytes
			fiPrevEventLen=bufoff/sizeof(Short_t); // offset between event header and first new event is length of spanned event data in bytes
			//cout <<"Next Buffer got previous event length (shorts):"<<fiPrevEventLen<< endl;
			fxCursor+=(bufheadleng+bufoff)/sizeof(Short_t); //  skip over header and previous event data  to first event data
			break;

		default:  /* unknown type*/
			cout <<"!! unknown buffer type "<<buftype<< endl;
			break;

		};

	fbNeedNewBuffer=kFALSE;
	return kTRUE;

}

Bool_t TXSYSEventSource::NextEvent(TGo4MbsEvent* target)
{
	static int evcounter=0;
	static int spancounter=0;
	static int failcounter=0;
	if(fbIsBufferSpanning)
	{
		spancounter++;
		//cout <<"++++++++ Next Event "<< evcounter <<" with buffer spanning..."<<endl;
		// first complete spanned buffer from previous call:
		//cout <<"Next Event found rest datalen of previous span event prevlen="<<(hex) <<fiPrevEventLen<<endl;
		if(fxSpanCursor+fiPrevEventLen > fxSpanBuffer+fiEventLen)
		{
			failcounter++;
			GO4MESSAGE((1,"TXSYSEventSource::NextEvent(): rest of spanned event (%d shorts) is larger then remaining event space !",fiPrevEventLen));
			fiPrevEventLen= fiEventLen- ( (char*)(fxSpanCursor) - (char*)(fxSpanBuffer))/sizeof(Short_t); // TODO: error handling here
			cout <<"Next Event reset prevlen to "<<fiPrevEventLen<<endl;
			cout <<"event# "<<evcounter<<", span# "<<spancounter<<", fail# "<<failcounter<< endl;
		}
		memcpy(fxSpanCursor, fxPrevEventCursor, fiPrevEventLen*sizeof(Short_t)); // add rest of event to buffer
		target->AddSubEvent(fxSubevHead.fiFullid, fxSpanBuffer, fiEventLen + 2, kTRUE);
		target->SetDlen(fiEventLen+2+4);
		target->SetCount(evcounter);
		evcounter++;
		delete [] fxSpanBuffer;
		fbIsBufferSpanning=kFALSE;
		return kTRUE;
	}
	fxEventhead = (XSYS_event_head*)(fxCursor);
	size_t evlen=fxEventhead->fEventsize; // total length of event header and payload
	fiEventLen=(evlen - sizeof(XSYS_event_head))/sizeof(Short_t) ; // only account data payload here
	fxCursor=(Short_t*)(fxEventhead+1); // move to begin of data field
	//cout <<"Next Event with short length "<<fiEventLen<< endl;
	if(fxCursor > fxBufEnd)
	{
		//cout <<"++++++++++ Next Event "<< evcounter <<"needs new buffer, returns"<< endl;
		fbNeedNewBuffer=kTRUE;
		return kFALSE;
	}
	else if(fxCursor+ fiEventLen >= fxBufEnd)
	{
		//cout <<"Next Event is spanning to next buffer"<< endl;
		fbNeedNewBuffer=kTRUE;
		fbIsBufferSpanning=kTRUE;
	}
	else {}
	int mbdflag=(fxEventhead->fFlag >> 8) & 0x3F;
	int mbdchan=(fxEventhead->fFlag ) & 0xFF;
	Bool_t isrecord=kFALSE, ishalt=kFALSE;
	if(mbdchan==0xFE)
	{
		cout <<"\nNext Event found BEGIN record!"<< endl;
		isrecord=kTRUE;

	}
	else if(mbdchan==0xFF)
	{
		cout <<"\nNext Event found HALT record!"<< endl;
		isrecord=kTRUE;
		ishalt=kTRUE;
	}
	if(isrecord)
	{
		XSYS_event_record* eventrecord= (XSYS_event_record*) fxEventhead;
		eventrecord->Print();
		// record events are put to regular subevents and can be figured out by subccrate id (channel num)
	}

	//cout <<"Next Event with header flags - mbdflag:"<< mbdflag<<", mdchan:"<< mbdchan<< endl;
	fxSubevHead.fcControl=mbdflag;
	fxSubevHead.fcSubcrate=mbdchan;
	fxSubevHead.fsProcid=XSYS_PROCID; // arbitrary xsys identifier
	if(fbIsBufferSpanning)
	{
		//cout <<"Next Event is allocating buffer for spanning of length "<< (hex) <<fiEventLen<<" shorts."<< endl;
		// allocate buffer for rest of event data:
		fxSpanBuffer=new Short_t[fiEventLen];
		// copy event data into it
		int copylen=fxBufEnd-fxCursor;
		//cout <<"Next Event is copying "<< (hex) <<copylen <<" shorts to spanbuffer"<< endl;
		memcpy(fxSpanBuffer, fxCursor, copylen*sizeof(Short_t));
		fxSpanCursor=fxSpanBuffer+copylen;
		return kFALSE;// request for next buffer
	}
	else
	{
		// no spanning, just put data to current subevent
		//cout <<"Next Event is filling mbs event regularly "<< endl;
		target->AddSubEvent(fxSubevHead.fiFullid, fxCursor, fiEventLen + 2, kTRUE);
		target->SetDlen(fiEventLen+2+4); // total length of pseudo mbs event
		target->SetCount(evcounter);
		evcounter++;
		if(ishalt)
		{
				SetErrMess(Form("Found HALT event record -> End of input file %s", GetName()));
				SetEventStatus(1);
				throw TGo4EventEndException(this);
		}
		// set cursors to next event header if any:
		fxCursor+= fiEventLen;
	}

	return kTRUE;
}


Bool_t TXSYSEventSource::BuildEvent(TGo4EventElement* dest)
{
   TGo4MbsEvent* evnt = dynamic_cast<TGo4MbsEvent*> (dest);
   if (evnt==0) return kFALSE;
   do {
		   NextBuffer();
	   }
   while (!NextEvent(evnt));

   // test here for error in input event

//   if(status!=0) {
//      evnt->SetValid(kFALSE);
//      // somethings wrong, display error message from f_evt_error()
//      SetErrMess("XSYS Event Source --  ERROR !!!");
//      throw TGo4EventErrorException(this);
//   }

   return kTRUE;
}

Int_t TXSYSEventSource::Open()
{
   if(fbIsOpen) return -1;
   cout << "Open of TXSYSEventSource"<< endl;
   // open connection/file
   fxFile = new std::ifstream(GetName(), ios::binary);
   if((fxFile==0) || !fxFile->good()) {
      delete fxFile; fxFile = 0;
      SetCreateStatus(1);
      SetErrMess(Form("Eror opening user file:%s",GetName()));
      throw TGo4EventErrorException(this);
   }
   fxBuffer=new Char_t[XSYS_BUFSIZE];




   fbIsOpen=kTRUE;
   return 0;
}

Int_t TXSYSEventSource::Close()
{
   if(!fbIsOpen) return -1;
   cout << "Close of TXSYSEventSource"<< endl;
   delete fxBuffer;
   Int_t status=0; // closestatus of source
   delete fxFile;
   fbIsOpen=kFALSE;
   return status;
}
