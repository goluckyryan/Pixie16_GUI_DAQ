{
  
  evtReader * evt = new evtReader("haha.evt");
  
  evt->ScanNumberOfBlock();
  
  int nData = evt->GetNumberOfBlock();
  
  for( int i = 0; i < nData; i++){
    evt->ReadBlock();
    
    int energy = evt->data->energy;
    ULong64_t time = evt->data->time;
    printf("%4d | %d | %lld\n", i, energy, time);
  }
  
}
