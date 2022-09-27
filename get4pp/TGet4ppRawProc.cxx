#include "TGet4ppRawProc.h"

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TMath.h"
#include "TVirtualFFT.h"
#include "TGo4Analysis.h"
#include "TGo4Log.h"

#include "TGo4Picture.h"
#include "TGo4MbsEvent.h"
#include "TGo4UserException.h"
#include "TGo4WinCond.h"

#include "TGet4ppRawEvent.h"
#include "TGet4ppRawParam.h"
#include "TGet4ppDisplay.h"

#include "TGo4Fitter.h"
#include "TGo4FitDataHistogram.h"
#include "TGo4FitParameter.h"
#include "TGo4FitModelPolynom.h"

#include "TGo4UserException.h"

/** enable this definition to print out event sample data explicitely*/
//#define GET4PP_DATA_VERBOSE 1
#define Get4ppDump( args... ) \
if (fPar->fVerbosity>1) printf( args);

#define Get4ppWarn( args... ) \
if(fPar->fVerbosity>0) printf( args);

//static unsigned long skipped_events = 0;

static unsigned long skipped_msgs = 0;

/* helper macros for BuildEvent to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/

#define  Get4ppEVENT_CHECK_PDATA                                    \
if((pdata - psubevt->GetDataField()) >= lwords ) \
{ \
  Get4ppWarn("############ unexpected end of event for subevent size :0x%x, skip event %ld\n", lwords, skipped_events++);\
  GO4_SKIP_EVENT \
  continue; \
}

#define  Get4ppRAW_CHECK_PDATA                                    \
if((pdata - pdatastart) > Get4ppRawEvent->fDataCount ) \
{ \
  Get4ppWarn("############ unexpected end of payload for datacount:0x%x after 0x%x words, end of event.\n", Get4ppRawEvent->fDataCount, (unsigned int) (pdata - pdatastart));\
  skipmessage=kTRUE; \
  break; \
}

//GO4_SKIP_EVENT

#define  Get4ppMSG_CHECK_PDATA                                    \
if((pdata - pdatastartMsg) > msize ) \
{ \
  Get4ppWarn("############ pdata offset 0x%x exceeds message size:0x%x, skip message \n", (unsigned int)(pdata - pdatastartMsg), msize);\
  skipmessage=kTRUE; \
  break; \
}
//  GO4_SKIP_EVENT
//  continue;

// this one is to discard last message that may be cut off by vulom daq:
#define  Get4ppEVENTLASTMSG_CHECK_PDATA                                    \
if((pdata - psubevt->GetDataField()) >= lwords ) \
 { \
    Get4ppWarn("############ pdata offset 0x%x exceeds  subevent size :0x%x, skip message %ld\n", (unsigned int) (pdata - psubevt->GetDataField()), lwords, skipped_msgs++)\
	skipmessage=kTRUE; \
  break; \
}

/*printf("############ pdata offset 0x%x exceeds  subevent size :0x%x, skip message %ld\n", (unsigned int) (pdata - psubevt->GetDataField()), lwords, skipped_msgs++);\*/

//***********************************************************
TGet4ppRawProc::TGet4ppRawProc() :
		TGo4EventProcessor(), fPar(0), Get4ppRawEvent(0)
{
}

//***********************************************************
// this one is used in standard factory
TGet4ppRawProc::TGet4ppRawProc(const char* name) :
		TGo4EventProcessor(name), Get4ppRawEvent(0),fEventCounter(0)
{
	TGo4Log::Info("TGet4ppRawProc: Create instance %s", name);
	fBoards.clear();
	SetMakeWithAutosave(kTRUE);
//// init user analysis objects:

	fPar = dynamic_cast<TGet4ppRawParam*>(MakeParameter("Get4ppRawParam",
			"TGet4ppRawParam", "set_Get4ppRawParam.C"));
	if (fPar)
		fPar->SetConfigBoards();

	for (unsigned i = 0; i < TGet4ppRawEvent::fgConfigGet4ppBoards.size(); ++i)
	{
		UInt_t uniqueid = TGet4ppRawEvent::fgConfigGet4ppBoards[i];
		fBoards.push_back(new TGet4ppBoardDisplay(uniqueid));
	}

	InitDisplay(false);

}

//***********************************************************
TGet4ppRawProc::~TGet4ppRawProc()
{
	TGo4Log::Info("TGet4ppRawProc: Delete instance");
	for (unsigned i = 0; i < fBoards.size(); ++i)
	{
		delete fBoards[i];
	}

}

/* access to histogram set for current board id*/
TGet4ppBoardDisplay* TGet4ppRawProc::GetBoardDisplay(Int_t uniqueid)
{
	for (unsigned i = 0; i < fBoards.size(); ++i)
	{
		TGet4ppBoardDisplay* theDisplay = fBoards[i];
		if (uniqueid == theDisplay->GetDevId())
			return theDisplay;
	}
	return 0;
}

void TGet4ppRawProc::InitDisplay(Bool_t replace)
{
	std::cout << "**** TGet4ppRawProc: Init Display with replace= "
			<< (replace ? "true" : "false") << std::endl;

	for (unsigned i = 0; i < fBoards.size(); ++i)
	{
		fBoards[i]->InitDisplay(replace);
	}

}

//-----------------------------------------------------------
// event function
Bool_t TGet4ppRawProc::BuildEvent(TGo4EventElement* target)
{
// called by framework from TGet4ppRawEvent to fill it
	Get4ppRawEvent = (TGet4ppRawEvent*) target;
	Get4ppRawEvent->SetValid(kFALSE);    // not store
	TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();
	if (source == 0)
	{
		cout << "AnlProc: no input event !" << endl;
		return kFALSE;
	}
	UShort_t triggertype = source->GetTrigger();

	if (triggertype > 11)
	{
		// frontend offset trigger can be one of these, we let it through to unpacking loop
		//cout << "**** TGet4ppRawProc: Skip trigger event" << endl;
		GO4_SKIP_EVENT_MESSAGE(
				"**** TGet4ppRawProc: Skip event of trigger type 0x%x",
				triggertype);
		//return kFALSE; // this would let the second step execute!
	}

	source->ResetIterator();
	TGo4MbsSubEvent* psubevt(0);

#ifdef Get4pp_DOFINETIMSAMPLES

	if(fPar->fFineTimeSampleBoard>0 &&  TGo4Analysis::Instance()->IsNewInputFile())
	{
	  fEventCounter=0;
	  TGo4Analysis::Instance()->ClearObjects("Histograms");
	  printf ("NNNNNNNNNN Found new input file: clearing histograms and resetting the counter!!\n");
	}


	fEventCounter++; // local counter, independent of MBS header
#endif









	while ((psubevt = source->NextSubEvent()) != 0)
	{    // loop over subevents
		Int_t *pdata = psubevt->GetDataField();
		Int_t lwords = psubevt->GetIntLen();

		if ((unsigned) *pdata == 0xbad00bad)
		{
			//GO4_SKIP_EVENT_MESSAGE("**** TGet4ppRawProc: Found BAD mbs event (marked 0x%x), skip it.", (*pdata));
			// JAM2020: avoid flooding message queue to GUI! better only to terminal:
			printf(
					"**** TGet4ppRawProc: Found BAD mbs event (marked 0x%x), skip it.",
					(*pdata));
			GO4_SKIP_EVENT
		}
		Bool_t skipmessage = kFALSE;
		// loop over single subevent data:
		while (pdata - psubevt->GetDataField() < lwords)
		{

			// vulom status word:
			Get4ppRawEvent->fVULOMStatus = *pdata++;
			// JAM2020: need to check if status word has valid format here:
			if (((Get4ppRawEvent->fVULOMStatus >> 12) & 0x3) != 0x3)
			{
				Get4ppWarn("VULOM: wrong vulom status word: 0x%x skip it.. \n",
						Get4ppRawEvent->fVULOMStatus);

				continue;
			}
			//event trigger counter:
			Get4ppRawEvent->fSequenceNumber = *pdata++;
			// data length
			Get4ppRawEvent->fDataCount = *pdata++;

			Get4ppDump("VULOM: status: 0x%x counter: 0x%x length: 0x%x \n",
					Get4ppRawEvent->fVULOMStatus,
					Get4ppRawEvent->fSequenceNumber, Get4ppRawEvent->fDataCount);
			if (Get4ppRawEvent->fDataCount > (lwords - 3))
			{
//        GO4_SKIP_EVENT_MESSAGE(
//            "**** TGet4ppRawProc: Mismatch with subevent len %d and data count 0x%8x - vulom status:0x%x seqnum:0x%x \n", lwords, Get4ppRawEvent->fDataCount,
//            Get4ppRawEvent->fVULOMStatus, Get4ppRawEvent->fSequenceNumber);
				// JAM2020: avoid flooding message queue to GUI! better only to terminal:
				printf(
						"**** TGet4ppRawProc: Mismatch with subevent len 0x%x and data count 0x%8x - vulom status:0x%x seqnum:0x%x \n",
						lwords, Get4ppRawEvent->fDataCount,
						Get4ppRawEvent->fVULOMStatus,
						Get4ppRawEvent->fSequenceNumber);
#ifdef Get4pp_DOFINETIMSAMPLES
				// for scanning mode, just ignore such buggy subevent
				GO4_SKIP_EVENT
#else
				GO4_STOP_ANALYSIS_MESSAGE(
						"Severe data error! mismatch in payload header, check terminal! Stopping.");

				// avoid that we run optional second step on invalid raw event!
#endif
			}

			Int_t* pdatastart = pdata; // remember begin of asic payload data section
			Get4ppDump("PPP pdatastart content: 0x%x \n", *pdatastart);

			pdata++;    // skip first  word?
			// pdatastart was here JAM
			// now fetch boardwise subcomponents for output data and histograming:
			Int_t slix = 0; // JAM here we later could evaluate a board identifier mapped to a slot/sfp number contained in subevent
			UInt_t brdid = fPar->fBoardID[slix]; // get hardware identifier from "DAQ link index" number
#ifndef Get4pp_DOFINETIMSAMPLES
			TGet4ppBoard* theBoard = Get4ppRawEvent->GetBoard(brdid);
			if (theBoard == 0)
			{
				GO4_SKIP_EVENT_MESSAGE(
						"Configuration error: Board id %d does not exist as subevent, slot index:%d",
						brdid, slix);

				return kFALSE;
			}
#endif
			TGet4ppBoardDisplay* boardDisplay = GetBoardDisplay(brdid);
			if (boardDisplay == 0)
			{
				GO4_SKIP_EVENT_MESSAGE(
						"Configuration error: Board id %d does not exist as histogram display set!",
						brdid);
				return kFALSE;
			}
			boardDisplay->ResetDisplay();

			// evaluate Get4ppection ASIC messages in the payload:

			while ((pdata - pdatastart) <= Get4ppRawEvent->fDataCount)
			{

				// first vulom wrapper containing total message length:
				Int_t vulombytecount = *pdata++;

				Get4ppDump("BY:  vulombytcount word: 0x%x \n", vulombytecount);

				if (((vulombytecount >> 28) & 0xF) == 0x4)
				{
					skipmessage = kFALSE;
//        // JAM2019 new: evaluate chip id here
					Int_t chipid = (vulombytecount >> 16) & 0xFF;
					boardDisplay->hChipId->Fill(chipid);

					Int_t* pdatastartMsg = pdata; // remember start of message for checking
					UChar_t msize = (vulombytecount & 0x3F) / sizeof(Int_t); // message size in 32bit words

					Get4ppDump("BY:  chipid: %d msize_32:%d \n", chipid, msize);

					// evaluate message type from header:
					Int_t header = *pdata++;
					Get4ppDump("HEAD:  header word: 0x%x \n", header);

					// we do not increment pdata here, do this inside msg types
					TGet4ppMsg::Get4MessageType_t mtype =
							(TGet4ppMsg::Get4MessageType_t) ((header >> 30)
									& 0x3);

					boardDisplay->hMsgTypes->Fill(mtype);
					Get4ppDump("MMMMMMMM message type %d \n", mtype);
					switch (mtype)
					{

					case TGet4ppMsg::MSG_TDC_Data:    // get4pp read out
					{
						Bool_t epochsyncflag = ((header >> 24) & 0x2) == 0x2;
						Bool_t syncflag = ((header >> 24) & 0x1) == 0x1;
						Int_t theEpoch = (header & 0xFFFFFF);
						if(epochsyncflag) boardDisplay->hSyncFlags->Fill(0);
						if(syncflag) boardDisplay->hSyncFlags->Fill(1);
						Get4ppDump("TDC  epochsync:%d sync:%d EPOCH:0x%x \n",
								epochsyncflag, syncflag, theEpoch);

						while (pdata - pdatastartMsg < msize)
						{
							Get4ppRAW_CHECK_PDATA;
							// always take chunks of 3 words = 4 x 24bit events - unless message is shorter!
							Int_t data[3] =
							{ -1 };
							// need to treat here special case that we have messages shorter than 3 words:
							//for (Int_t j = 0; ((j < 3) && (j < msize - 1)); ++j) // first word of message is header, already handled above! -> msize-1
							for (Int_t j = 0; ((j < 3) && (pdata  - pdatastartMsg < msize )); ++j)
							{
								Get4ppEVENTLASTMSG_CHECK_PDATA
								Get4ppRAW_CHECK_PDATA;
								Get4ppMSG_CHECK_PDATA;
								data[j] = *pdata++;
								Get4ppDump(" - data[%d]=0x%x \n", j, data[j]);
							} // for j
							if (!skipmessage)
							{
								// extract 24bit chunks here:
								Int_t eventdata[4] =
								{ 0 };

								eventdata[0] = (data[0] >> 8) & 0xFFFFFF;
								if (data[1] != -1)
								{
									eventdata[1] = ((data[0] & 0xFF) << 16)
											| ((data[1] >> 16) & 0xFFFF);
									if (data[2] != -1)
									{
										eventdata[2] = ((data[1] & 0xFFFF) << 8)
												| ((data[2] >> 24) % 0xFF);
										eventdata[3] = (data[2] & 0xFFFFFF);
									}
								}
								TGet4ppMsgEvent* theMsg = 0;
								for (Int_t e = 0; e < 4; ++e)
								{
									if (eventdata[e] == 0)
										continue; // not filled due to short messages in data stream
									// find out kind of message
									Int_t chan = ((eventdata[e] >> 20) & 0x3);
//									Bool_t leadingedge = ((eventdata[e] >> 19)
//											& 0x1) == 0x1;
									// probably leading edge has bit not set?
									Bool_t leadingedge = ((eventdata[e] >> 19)
											& 0x1) != 0x1;
									Char_t kind = ((eventdata[e] >> 22) & 0x3);
									Bool_t isTDC = (kind == 0x3);
									Bool_t isError = (kind == 0x2);
									Get4ppDump(
											"TDC  chan:%d leading:%d kind:%d \n",
											chan, leadingedge, (int ) kind);

									if (isTDC)
									{
										TGet4ppMsgTDCEvent* tmess =
												new TGet4ppMsgTDCEvent(chan);
										tmess->SetCoarseTime(
												UShort_t(
														(eventdata[e] >> 7)
																& 0xFFF));
										tmess->SetFineTime(
												UChar_t(eventdata[e] & 0x7F));
										theMsg = tmess;
										Get4ppDump(
												"TDC  coarse time:%d fine time:%d \n",
												tmess->GetCoarseTime(),
												tmess->GetFineTime());
									}
									else if (isError)
									{
										TGet4ppMsgErrorEvent* emess =
												new TGet4ppMsgErrorEvent(chan);
										emess->SetErrorCode(
												TGet4ppMsgErrorEvent::Get4ErrorType_t(
														eventdata[e] & 0x3F));
										emess->SetDLLPhase(
												UChar_t(
														(eventdata[e] >> 7)
																& 0xF));
										theMsg = emess;
										Get4ppDump(
												"ERR  error code %d : %s , DLL phase:%d\n",
												emess->GetErrorCode(),
												emess->GetErrorMessage().Data(),
												emess->GetDLLPhase());
									}
									else
									{
										Get4ppWarn(
												"Data error MBS event %d: evtdata %d  is neither TDC nor error, but type %d\n",
												source->GetCount(), e,
												(int ) kind);
										boardDisplay->hEventTypes[chan]->Fill(
												kind);
										continue;
									}
									theMsg->SetEpoch(theEpoch);
									theMsg->SetLeadingEdge(leadingedge);
#ifndef Get4pp_DOFINETIMSAMPLES
									theBoard->AddMessage(theMsg, chan);
#endif
///////// end of unpacking: the rest is histogram fill stuff
//////////////////////////////////////////////////////////////////////////////////////////////////
									// here directly fill histograms
									// this part may be put into a second function later parsing the unpacked messages.
									boardDisplay->hChannels->Fill(chan);
									Int_t edgeindex = (leadingedge ? 0 : 1);
									boardDisplay->hEdges[chan]->Fill(edgeindex);
									boardDisplay->hEventTypes[chan]->Fill(kind);
									boardDisplay->hEpochs[chan][edgeindex]->Fill(
											theMsg->GetEpoch());
									if (isTDC)
									{
										TGet4ppMsgTDCEvent* tmess =
												dynamic_cast<TGet4ppMsgTDCEvent*>(theMsg); // TODO: better?
										if (tmess == 0)
										{
											GO4_STOP_ANALYSIS_MESSAGE(
													"NEVER COME HERE: inconsistent data type in unpacker. Memory corruption?");
										}
										boardDisplay->hCoarseTime[chan][edgeindex]->Fill(
												tmess->GetCoarseTime());
										boardDisplay->hFineTime[chan][edgeindex]->Fill(
												tmess->GetFineTime());
										boardDisplay->hFullTime[chan][edgeindex]->Fill(
												tmess->GetFullTime());
										boardDisplay->hTimeInSeconds[chan][edgeindex]->Fill(
												tmess->GetTimeInSeconds());

										if (leadingedge)
										{
											if (boardDisplay->LastEdgeWasLeading(
													chan))
											{
												// we've lost trailing edge in between: not a message, but fill into error scaler
												boardDisplay->hLostEdges[chan]->Fill(
														1);
											}
											else
											{
												boardDisplay->SetLastEdgeLeading(
														chan, kTRUE);
												boardDisplay->SetLastLeadingTime(
														chan,
														tmess->GetTimeInSeconds());

											}

										}

										else
										{
											if (!boardDisplay->LastEdgeWasLeading(
													chan))
											{
												// we've lost leading edge in between: not a message, but fill into error scaler
												boardDisplay->hLostEdges[chan]->Fill(
														0);
											}
											else
											{

												Double_t timeOverthreshold =
														tmess->GetTimeInSeconds()
																- boardDisplay->GetLastLeadingTime(
																		chan);
												if (timeOverthreshold < 0)
												{
													// wraparound of epoch counter: correct it here
													Double_t epochmaxtime =
															((Double_t) (Get4pp_EPOCHRANGE
																	+ 1))
																	* (Double_t) ((Get4pp_COARSERANGE
																			+ 1))
																	* Get4pp_COARSETIMEUNIT; // 137.4 s
													timeOverthreshold =
															tmess->GetTimeInSeconds()
																	+ (epochmaxtime
																			- boardDisplay->GetLastLeadingTime(
																					chan));
													boardDisplay->hLostEdges[chan]->Fill(
															2); // account epoch wraps in lost edges scaler
												}

												boardDisplay->hToTinSeconds[chan]->Fill(
														timeOverthreshold);
												boardDisplay->SetLastEdgeLeading(
														chan, kFALSE);
											}
										}

										/////////////// start DEBUG time info
										//fPar->fSlowMotion=kTRUE;
										/////////////////////////////////
									}
									else if (isError)
									{
										// TODO here error infos:
										TGet4ppMsgErrorEvent* emess =
												dynamic_cast<TGet4ppMsgErrorEvent*>(theMsg); // TODO: better?
										if (emess == 0)
										{
											GO4_STOP_ANALYSIS_MESSAGE(
													"NEVER COME HERE: inconsistent data type in unpacker. Memory corruption?");
										}
										TGet4ppMsgErrorEvent::Get4ErrorType_t theError =
												emess->GetErrorCode();
										if (theError
												== TGet4ppMsgErrorEvent::ERR_Unknown)
										{
											boardDisplay->hErrorCodes[chan]->Fill(
													TGet4ppMsgErrorEvent::ERR_DLL_Reset
															+ 1);
										}
										else
										{
											boardDisplay->hErrorCodes[chan]->Fill(
													theError);
										}
										boardDisplay->hErrorDLLPhase[chan]->Fill(
												emess->GetDLLPhase());

									}
									else
									{
										printf(
												"NEVER COME HERE!!! AGAIN: Data error MBS event %d: data %d  is neither TDC nor error, but type %d  NCH\n",
												source->GetCount(), e,
												(int) kind);
										continue;
									}

								} // for e

							} // skipmessage

						} // while

					} // case
						break;
						// JAM2020: these messages do not appear in get4++ test data!
					case TGet4ppMsg::MSG_Wishbone:
						// wishbone response (error message)
					{
						// this one is suppressed in readout data stream. We keep old code from hitdet anyway
						// pdata = pdatastartMsg + msize; //JAM2020 ????
						UChar_t wishhead = (header >> 24) & 0xFF;
						Get4ppDump("MSG_WishboneEvent header=0x%x\n", wishhead);
						TGet4ppMsgWishbone* theMsg = new TGet4ppMsgWishbone(
								wishhead);
						Int_t address = 0, val = 0;
//                ;
						if (pdata - pdatastartMsg < msize)
						{
							address = *pdata++;
							theMsg->SetAddress(address);
						}
////                // here we could take rest of message as data contents:
						while (pdata - pdatastartMsg < msize)
						{
							Get4ppRAW_CHECK_PDATA;
							val = *pdata++;
							theMsg->AddData(val);
						}
						pdata--;    // rewind pointer to end of payload
						boardDisplay->hWishboneAck->Fill(theMsg->GetAckCode());
						boardDisplay->hWishboneSource->Fill(
								theMsg->GetSource());
						boardDisplay->lWishboneText->SetText(0.1, 0.9,
								theMsg->DumpMsg());
#ifndef Get4pp_DOFINETIMSAMPLES
						theBoard->AddMessage(theMsg, 0); // wishbone messages accounted for channel 0
#endif
//                if(theMsg->GetDataSize()>0)
//                {
//                  printf("Wishbone text: %s",theMsg->DumpMsg().Data());
//                  std::cout<<std::endl;
//                }

					}
						break;

					default:
						Get4ppWarn(
								"############ found unknown message type 0x%x, skip message %ld \n",
								mtype, skipped_msgs++)
						;
						//GO4_SKIP_EVENT
						pdata = pdatastartMsg + msize; // do not skip complete event, but just the current message:
						break;
					};    // switch

					if (!skipmessage && (pdata - pdatastartMsg) < msize)
					{
						// never come here if messages are treated correctly!
						Get4ppWarn(
								"############  pdata offset 0x%x has not yet reached message length 0x%x, correcting ,\n",
								(unsigned int) (pdata - pdatastartMsg), msize);
						pdata = pdatastartMsg + msize;
					}

					//printf("EEEEEEEE  end of message payload: pdata offset 0x%x msglength 0x%x\n",
					//             (unsigned int) (pdata - pdatastartMsg), msize);

				} // JAM2019 test instead inner continue

			}    // while ((pdata - pdatastart) < Get4ppRawEvent->fDataCount)

		}    // while pdata - psubevt->GetDataField() <lwords

	}    // while subevents

//

	UpdateDisplays(); // we fill the raw displays immediately, but may do additional histogramming later

	if (fPar->fSlowMotion)
	{
		Int_t evnum = source->GetCount();
		GO4_STOP_ANALYSIS_MESSAGE(
				"Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.",
				evnum);
	}

	return kTRUE;
}


Bool_t TGet4ppRawProc::UpdateDisplays()
{
#ifdef Get4pp_DOFINETIMSAMPLES
  // for labtest: write fine time histogram bins to tree when we have enough statistics
  Get4ppRawEvent->SetValid(kFALSE);    // do not store by default
  UInt_t brdid = fPar->fFineTimeSampleBoard;    // use defined board under test
  if (brdid > 0)
  {
    TGet4ppBoardDisplay *boardDisplay = GetBoardDisplay(brdid);
    if (boardDisplay == 0)
    {
      GO4_SKIP_EVENT_MESSAGE(
          "Configuration error in UpdateDisplays: Board id %d does not exist as histogram display set!", brdid);
      return kFALSE;
    }

    if (fEventCounter == fPar->fFineTimeStatsLimit)
    {
      // we reach the point to get the accuumlated samples:
      TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();

      s_filhe* head = source->GetMbsSourceHeader();
      if (head)
      {
        char* filename = head->filhe_file;
        Get4ppRawEvent->fLmdFileName = TString(filename, head->filhe_file_l);
        //We expect files of form ConfigScan_<TapConfig>_<DelayConfig>.lmd
        Int_t rev = sscanf(filename, "ConfigScan_%d_%x", &(Get4ppRawEvent->fTapConfig),
            &(Get4ppRawEvent->fDelayConfig));
        if (rev > 0)
        {
          printf("Got From filename: %s the tapconfig:%d and delayconf:0x%x \n", filename, Get4ppRawEvent->fTapConfig,
              Get4ppRawEvent->fDelayConfig);
        }
        else
        {
          printf("Error %d when scanning filename: %s", rev, filename);
        }

      }
      //Get4ppWarn
      printf("UpDateDisplays: writing channel infos to output event after %d events, filename:%s\n", fEventCounter,
          Get4ppRawEvent->fLmdFileName.Data());
      // if reached threshold, write out bins to output event
      for (Int_t chan = 0; chan < Get4pp_CHANNELS; ++chan)
      {
        for (Int_t edgeindex = 0; edgeindex < 2; ++edgeindex)
        {
          TH1 *his = boardDisplay->hFineTime[chan][edgeindex];
          for (Int_t bin = 0; bin < Get4pp_FINERANGE; ++bin)
          {
            Double_t val = his->GetBinContent(bin+1);
            if (edgeindex)
              Get4ppRawEvent->fFineTimeBinTrailing[chan][bin] = val;
            else
              Get4ppRawEvent->fFineTimeBinLeading[chan][bin] = val;
          }    //bin
        }    //edgeindex
      }    //chan

      // set output event valid for tree storage
      Get4ppRawEvent->SetValid(kTRUE);
    }
    else if (fEventCounter > fPar->fFineTimeStatsLimit)
    {
      // after sample was stored, just skip all other events in file
      Get4ppRawEvent->SetValid(kFALSE);
      if((fEventCounter%500 ==0))printf("SSSS ignoring event %d from %s ...\n", fEventCounter, Get4ppRawEvent->fLmdFileName.Data());
       //GO4_SKIP_EVENT;
      // JAM DEBUG
//      fEventCounter=0;
//      TGo4Analysis::Instance()->ClearObjects("Histograms");
//      printf ("RRRR DEBUG: clearing histograms and resetting the counter!!\n");
    }
    else
    {
      // if not yet at sampling point, do not store
      Get4ppRawEvent->SetValid(kFALSE);
    }
  }    // boardid

#else
// maybe later some advanced analysis from full output event data here

  for (unsigned i = 0; i < TGet4ppRawEvent::fgConfigGet4ppBoards.size(); ++i)
  {
    UInt_t brdid = TGet4ppRawEvent::fgConfigGet4ppBoards[i];
    TGet4ppBoard* theBoard = Get4ppRawEvent->GetBoard(brdid);
    if (theBoard == 0)
    {
      GO4_SKIP_EVENT_MESSAGE(
          "FillDisplays Configuration error: Board id %d does not exist!",
          brdid);
      //return kFALSE;
    }
    TGet4ppBoardDisplay* boardDisplay = GetBoardDisplay(brdid);
    if (boardDisplay == 0)
    {
      GO4_SKIP_EVENT_MESSAGE(
          "FillDisplays Configuration error: Board id %d does not exist as histogram display set!",
          brdid);
      //return kFALSE;
    }

    // TODO JAM 2020 TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTOOOOOOOOOOOOOOOTTTTTTTTTOOOOOOOOOOOOO

  }    // i board
#endif
  return kTRUE;
}


