//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <ImgList.hpp>
//---------------------------------------------------------------------------
class TfrmMain : public TForm
{
__published:	// IDE-managed Components
        TLabel *Msg;
        TTimer *Timer;
        TSaveDialog *SaveDialog;
        TButton *btnSave;
        TButton *btnConfig;
        TLabel *Label1;
        TEdit *CaptureTime;
        TCheckBox *MoveCapture;
        TEdit *MoveCaptureStart;
        TCheckBox *NotSaveWithoutMove;
        TEdit *Size;
        TLabel *lblSens;
        TLabel *Label2;
        TLabel *Label3;
        TEdit *Sens;
        TButton *Test;
        void __fastcall TimerTimer(TObject *Sender);
        void __fastcall MsgClick(TObject *Sender);
        void __fastcall FormKeyPress(TObject *Sender, char &Key);
        void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall btnSaveClick(TObject *Sender);
        void __fastcall btnConfigClick(TObject *Sender);
        void __fastcall MoveCaptureClick(TObject *Sender);
        void __fastcall TestClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
