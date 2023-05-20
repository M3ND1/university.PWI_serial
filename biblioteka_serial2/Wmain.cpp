//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Wmain.h"
#include "Tserial_event.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFmain *Fmain;

Tserial_event serial_port;



/* ======================================================== */
/* ===============  OnCharArrival     ===================== */
/* ======================================================== */
void SerialEventManager(uint32 object, uint32 event)
{
    char *buffer;
    int   size;
    Tserial_event *com;
    char   txt[512];

    com = (Tserial_event *) object;
    if (com!=0)
    {
        switch(event)
        {
            case  SERIAL_CONNECTED  :
                                        Fmain->Memo1->Lines->Append(AnsiString("Connected !"));
                                        break;
            case  SERIAL_DISCONNECTED  :
                                        Fmain->Memo1->Lines->Append(AnsiString("Disonnected !"));
                                        break;
            case  SERIAL_DATA_SENT  :
                                        //OnDataSent();
                                        Fmain->Memo1->Lines->Append(AnsiString("Data sent !"));
                                        break;
            /*
            case  SERIAL_RING       :
                                        printf("DRING ! \n");
                                        break;
            case  SERIAL_CD_ON      :
                                        printf("Carrier Detected ! \n");
                                        break;
            case  SERIAL_CD_OFF     :
                                        printf("No more carrier ! \n");
                                        break;
            */
            case  SERIAL_DATA_ARRIVAL  :
                                        size   = com->getDataInSize();
                                        buffer = com->getDataInBuffer();

                                        if (size>511)
                                            size=511;

                                        memcpy(txt,buffer, size);
                                        txt[size] = 0;
                                        Fmain->Memo1->Lines->Append(AnsiString(txt));

                                        com->dataHasBeenRead();
                                        break;
        }
    }
}





//---------------------------------------------------------------------------
__fastcall TFmain::TFmain(TComponent* Owner)
    : TForm(Owner)
{
    serial_port.setManager(SerialEventManager);
}
//---------------------------------------------------------------------------
void __fastcall TFmain::Button1Click(TObject *Sender)
{
    // connect
    serial_port.connect("COM2",38400,SERIAL_PARITY_NONE,8,false);
}
//---------------------------------------------------------------------------
void __fastcall TFmain::Button2Click(TObject *Sender)
{
    // send
    serial_port.sendData(Edit1->Text.c_str(), Edit1->Text.Length());
}
//---------------------------------------------------------------------------

void __fastcall TFmain::Button3Click(TObject *Sender)
{
    serial_port.disconnect();
}
//---------------------------------------------------------------------------
