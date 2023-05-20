/* ------------------------------------------------------------------------ ----                                                                          --
--                  Application example for using Tserial_event             --
--                                                                          --
--                                                                          --
--                                                                          --
--  Copyright @ 2001-2002     Thierry Schneider                             --
--                            thierry@tetraedre.com                         --
--                                                                          --
--                                                                          --
--                                                                          --
-- ------------------------------------------------------------------------ --
--                                                                          --
--  Filename : serialtest.cpp                                               --
--  Author   : Thierry Schneider                                            --
--  Created  : April 8th 2001                                               --
--  Modified : January 30th 2002                                            --
--  Plateform: Windows 95, 98, NT, 2000 (Win32)                             --
-- ------------------------------------------------------------------------ --
--                                                                          --
--  This software is given without any warranty. It can be distributed      --
--  free of charge as long as this header remains, unchanged.               --
--                                                                          --
-- ------------------------------------------------------------------------ --
--                                                                          --
--  01.04.24    # ifdef __BORLANDC__ added in the header                    --               
--                                                                          --
-- 02.01.30      Version 2.0 of the serial event object                     --
--                                                                          --
--                                                                          --
-- ------------------------------------------------------------------------ */




/* ---------------------------------------------------------------------- */
#ifdef __BORLANDC__
#pragma hdrstop             // borland specific
#include <condefs.h>
#pragma argsused
USEUNIT("Tserial_event.cpp");
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
#include "conio.h"
#include "Tserial_event.h"


/* ======================================================== */
/* ===============  OnCharArrival     ===================== */
/* ======================================================== */
void OnDataArrival(int size, char *buffer)
{
    if ((size>0) && (buffer!=0))
    {
        buffer[size] = 0;
        printf("OnDataArrival: %s ",buffer);
    }
}

/* ======================================================== */
/* ===============  OnCharArrival     ===================== */
/* ======================================================== */
void SerialEventManager(uint32 object, uint32 event)
{
    char *buffer;
    int   size;
    Tserial_event *com;

    com = (Tserial_event *) object;
    if (com!=0)
    {
        switch(event)
        {
            case  SERIAL_CONNECTED  :
                                        printf("Connected ! \n");
                                        break;
            case  SERIAL_DISCONNECTED  :
                                        printf("Disonnected ! \n");
                                        break;
            case  SERIAL_DATA_SENT  :
                                        printf("Data sent ! \n");
                                        break;
            case  SERIAL_RING       :
                                        printf("DRING ! \n");
                                        break;
            case  SERIAL_CD_ON      :
                                        printf("Carrier Detected ! \n");
                                        break;
            case  SERIAL_CD_OFF     :
                                        printf("No more carrier ! \n");
                                        break;
            case  SERIAL_DATA_ARRIVAL  :
                                        size   = com->getDataInSize();
                                        buffer = com->getDataInBuffer();
                                        OnDataArrival(size, buffer);
                                        com->dataHasBeenRead();
                                        break;
        }
    }
}
/* ======================================================== */
/* ========================   main  ======================= */
/* ======================================================== */
int main(int argc, char* argv[])
{
    int            c;
    int            erreur;
    char           txt[32];
    Tserial_event *com;


    com = new Tserial_event();
    if (com!=0)
    {
        com->setManager(SerialEventManager);
        erreur = com->connect("COM1", 19200, SERIAL_PARITY_NONE, 8, true);
        if (!erreur)
        {
            com->sendData("Hello World",11);
            com->setRxSize(5);

            // ------------------
            do
            {
                c = getch();
                printf("_%c",c);
                txt[0] = c;
                com->sendData(txt, 1);
                com->setRxSize(1);
            }
            while (c!=32);

        }
        else
            printf("ERROR : com->connect (%ld)\n",erreur);
        // ------------------
        com->disconnect();

        // ------------------
        delete com;
        com = 0;
    }
    return 0;
}

