
#include "TGet4ppEventSource.h"

#include "TClass.h"
#include "Riostream.h"
#include <stdlib.h>
#include <endian.h>

#include "TGo4EventErrorException.h"
#include "TGo4EventEndException.h"
#include "TGo4EventTimeoutException.h"
#include "TGo4UserSourceParameter.h"
#include "TGo4MbsEvent.h"
#include "TGo4Log.h"



TGet4ppEventSource::TGet4ppEventSource(const char* name,
                                 const char* args,
                                 Int_t port) :
   TGo4EventSource(name),
   fbIsOpen(kFALSE),
   fxArgs(args),
   fiPort(port),
   fxFile(0),
   fbNeedNewBuffer(kTRUE),
   fxBuffer(0),
   fxCursor(0),
   fxEventBuffer(0),
   fxEventData(0),
   fxBufsize(0),
   fiEventLen(0)
{
   Open();
}

TGet4ppEventSource::TGet4ppEventSource(TGo4UserSourceParameter* par) :
   TGo4EventSource(" "),
   fbIsOpen(kFALSE),
    fxArgs(""),
    fiPort(0),
    fxFile(0),
    fbNeedNewBuffer(kTRUE),
    fxBuffer(0),
	fxCursor(0),
	fxEventBuffer(0),
	fxEventData(0),
	fxBufsize(0),
	fiEventLen(0)

{
   if(par) {
      SetName(par->GetName());
      SetPort(par->GetPort());
      SetArgs(par->GetExpression());
      Open();
   } else {
      cout <<"TGet4ppEventSource constructor with zero parameter!" << endl;
   }
}

TGet4ppEventSource::TGet4ppEventSource() :
   TGo4EventSource("default Get4pp source"),
   fbIsOpen(kFALSE),
    fxArgs(""),
    fiPort(0),
    fxFile(0),
    fbNeedNewBuffer(kTRUE),
    fxBuffer(0),
    fxCursor(0),
	fxEventBuffer(0),
	fxEventData(0),
	fxBufsize(0),
	fiEventLen(0)
{
}

TGet4ppEventSource::~TGet4ppEventSource()
{
   Close();
}

Bool_t TGet4ppEventSource::CheckEventClass(TClass* cl)
{
   return cl->InheritsFrom(TGo4MbsEvent::Class());
}

std::streamsize TGet4ppEventSource::ReadFile(Char_t* dest, size_t len)
{
	fxFile->read(dest, len);
	if(fxFile->eof() || !fxFile->good())
			{
					SetCreateStatus(1);
					SetErrMess(Form("End of input file %s", GetName()));
					SetEventStatus(1);
					throw TGo4EventEndException(this);
			}
	cout <<"ReadFile reads "<< (hex) << fxFile->gcount()<<" bytes to 0x"<< (hex) <<(long) dest<< endl;
	return fxFile->gcount();
}



Bool_t TGet4ppEventSource::NextBuffer()
{
	static int vulomcounter=0;

	// JAM2020: for simulation data, we scan with own buffer size and take this as mbs/vulom event:
	fxBufsize=ReadFile(fxBuffer, Get4pp_BUFSIZE);
	fxCursor= fxBuffer; // cursor to start of buffer


	printf("Event source read buffer %d of length %ld \n",vulomcounter++, fxBufsize);

	// here generic endian swap:
	Int_t* pdata=(Int_t*)(fxBuffer);
	for(UInt_t i=0; i< fxBufsize/sizeof(Int_t); ++i)
    {
	  //uint32_t be32toh(uint32_t big_endian_32bits);
	  uint32_t bword=*pdata;
      *pdata++=be32toh(bword);
    }
	return kTRUE;

}

Bool_t TGet4ppEventSource::NextEvent(TGo4MbsEvent* target)
{
	static int evcounter=0;

	//cout <<"Next Event with short length "<<fiEventLen<< endl;
//	if(fxCursor > fxBufEnd)
//	if(!fbNeedNewBuffer)
//	{
//		// each event requires a new buffer in this scenario
//		//cout <<"++++++++++ Next Event "<< evcounter <<"needs new buffer, returns"<< endl;
//		fbNeedNewBuffer=kTRUE;
//		return kFALSE;uint32_t be32toh(uint32_t big_endian_32bits);
//	}
	fxEventData= (Int_t*) fxEventBuffer; // rewind to start
	fxSubevHead.fcControl=9;
	fxSubevHead.fcSubcrate=0;
	fxSubevHead.fsProcid=Get4pp_PROCID; // arbitrary xsys identifier
	// first we put the pseudo VULOM information here:
	*fxEventData++ =0x300d; // VULOM HEADER
	*fxEventData++ =evcounter; // SEQUENCENUMBER
	Int_t* vdlength=fxEventData++; // remember position of length info
	fxEventData++; // extra word like in the MBS data?

	// JAM 4-20202: following does not work with current simulation data:
	// file contains single tdc messages (epoch header + data. need to use chunks of lenght 1 here
	// file probably has wrong endian (Windows?), so need to wrap words first


	// here we have to copy data from file buffer to eventbuffer and insert the vulom bytecount containers:
	Int_t numchunks=fxBufsize/Get4pp_CHUNKSIZE;
	Int_t chipid = 42;
	for(int i=0; i< numchunks; ++i)
		{
		// from unpacker:
	  // (vulombytecount >> 28) & 0xF) == 0x4
		//	Int_t chipid = (vulombytecount >> 16) & 0xFF;
		//	UChar_t msize = (vulombytecount & 0x3F) / sizeof(Int_t); // message size in 32bit words
		// put 	vulombytecount here:
		*fxEventData++ =  (0x4 << 28) | ((chipid & 0xFF) << 16) | (Get4pp_CHUNKSIZE & 0x3F);
		memcpy(fxEventData, fxCursor, Get4pp_CHUNKSIZE);
		fxEventData += Get4pp_CHUNKSIZE/sizeof(Int_t);
		fxCursor+= Get4pp_CHUNKSIZE;
		}

		fiEventLen= ((Char_t* )fxEventData - (Char_t*) fxEventBuffer) /sizeof(Short_t);
		*vdlength =numchunks * (Get4pp_CHUNKSIZE + sizeof(Int_t))/sizeof(Int_t);
		//fiEventLen = ( *vdlength + 4 * sizeof(Int_t)) / sizeof(Short_t); // why not 3*
		printf("MBS subevent len: %d vulom length: %d  chunks:%d chunksize:%d\n",fiEventLen,*vdlength, numchunks, Get4pp_CHUNKSIZE);
		target->AddSubEvent(fxSubevHead.fiFullid, fxEventBuffer, fiEventLen + 2, kTRUE);
		target->SetDlen(fiEventLen+2+4); // total length of pseudo mbs event
		target->SetCount(evcounter);
		evcounter++;

//		fbNeedNewBuffer=kTRUE;
//		return kFALSE; // each event requires a new buffer in this scenario
	return kTRUE;
}


Bool_t TGet4ppEventSource::BuildEvent(TGo4EventElement* dest)
{
   TGo4MbsEvent* evnt = dynamic_cast<TGo4MbsEvent*> (dest);
   if (evnt==0) return kFALSE;
//   do {
//		   NextBuffer();
//	   }
//   while (!NextEvent(evnt));

   NextBuffer();
   NextEvent(evnt);
   // test here for error in input event

//   if(status!=0) {
//      evnt->SetValid(kFALSE);
//      // somethings wrong, display error message from f_evt_error()
//      SetErrMess("Get4pp Event Source --  ERROR !!!");
//      throw TGo4EventErrorException(this);
//   }

   return kTRUE;
}

Int_t TGet4ppEventSource::Open()
{
   if(fbIsOpen) return -1;
   cout << "Open of TGet4ppEventSource"<< endl;
   // open connection/file
   fxFile = new std::ifstream(GetName(), ios::binary);
   if((fxFile==0) || !fxFile->good()) {
      delete fxFile; fxFile = 0;
      SetCreateStatus(1);
      SetErrMess(Form("Eror opening user file:%s",GetName()));
      throw TGo4EventErrorException(this);
   }
   fxBuffer=new Char_t[Get4pp_BUFSIZE];
   fxBufsize=Get4pp_BUFSIZE;

   fxEventBuffer= new Short_t[Get4pp_BUFSIZE]; // twice the file buffer, enough room for vulom headers
   fxEventData = (Int_t*) fxEventBuffer;


   fbIsOpen=kTRUE;
   return 0;
}

Int_t TGet4ppEventSource::Close()
{
   if(!fbIsOpen) return -1;
   cout << "Close of TGet4ppEventSource"<< endl;
   delete fxEventBuffer;
   delete fxBuffer;
   Int_t status=0; // closestatus of source
   delete fxFile;
   fbIsOpen=kFALSE;
   return status;
}
