//---------------------------------------------------------------------------

#ifndef WmainH
#define WmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TFmain : public TForm
{
__published:	// IDE-managed Components
    TButton *Button1;
    TMemo *Memo1;
    TButton *Button2;
    TEdit *Edit1;
    TButton *Button3;
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall Button2Click(TObject *Sender);
    void __fastcall Button3Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TFmain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFmain *Fmain;
//---------------------------------------------------------------------------
#endif
