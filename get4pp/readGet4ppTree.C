void readGet4ppTree(const char* filename)
{
  TH1I* finetime = new TH1I("fine time test", "finetime leading",127,0,127);
  TString fname(filename);
    if(!fname.EndsWith(".root")) fname.Append(".root");
    TFile myfile(fname.Data());
    TTree* theTree=0;
    TKey* kee=0;
    TIter iter(myfile.GetListOfKeys());
     while ( ( kee=dynamic_cast<TKey*>(iter()) ) !=0 ) {
        theTree = dynamic_cast<TTree*>(kee->ReadObj());
        if (theTree)
           break; // we take first Tree in file, disregarding its name...
     }
     if(theTree==0)
       {
         cout <<"Error: no Tree in file "<<fname.Data() << endl;
         return;
       }

     TGet4ppRawEvent* eve= new TGet4ppRawEvent;
     theTree->SetBranchAddress("Get4ppRawEvent.",&eve); // by branchname!
     Int_t all=theTree->GetEntries(); // number of events
     for(Int_t i=0; i<all; ++i){
         theTree->GetEntry(i);
         std::cout<<"tap:"<<eve->fTapConfig<<", delay:" << eve->fDelayConfig<< std::endl;

         if(eve->fTapConfig==0 && eve->fDelayConfig==5)
         {
           std::cout<<"inspecting channel 1 fine time leading for tap 0 delay 5 ..."<<std::endl;
           for(int b=0; b<127;++b)
           {
             finetime->SetBinContent(b+1,eve->fFineTimeBinLeading[1][b]);
             std::cout<<" -- filled content:"<< eve->fFineTimeBinLeading[1][b]<<" into bin:"<< b<<std::endl;
           }
          }
     }
     finetime->Draw();


}
