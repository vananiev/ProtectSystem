//---------------------------------------------------------------------------
//#pragma comment(linker,"/MERGE:.rdata=.text")
//#pragma comment(linker,"/FILEALIGN:512 /SECTION:.text,EWRX /IGNORE:4078")
//#pragma comment(linker,"/ENTRY:New_WinMain")
//#pragma comment(linker,"/NODEFAULTLIB")

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include <mmsystem.h>
#include <registry.hpp>

#include <vfw.h>         //������
#include <Clipbrd.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
bool ProtectEnable = false; //��������� ������
bool bSignal = false;   //������������� ������
bool CamSet = false;    //���� �� ������ � �������
void Signal(); //������������ ������������
String *_Key;   //����
String sPath="C:\\"; //���� ��� ���������� �����
void RegSet(int Value); //��������� �������
void protectOn();       //��� ������
void protectOff();      //���� ������
HWND hWndC;             //������ � �������
void SetConfig();       //���������� ���������
unsigned int _CaptureTime; //����� ����� �������� � ��������
unsigned int _MoveCaptureStart; //����� �� ������ �������� �������� � ������
double ObjSize ;        //������ ������� �������� � %
signed int Sensitivity; //���������������� %
unsigned int Var_Color;    //�������� ������ �� ������ ��-�� �����
unsigned int DoCapture=0;     //����� ������� �� �����
unsigned int cnt = 0;           //����� ����� � ������ ������
TClipboard *pCB = Clipboard();
TPicture* Picture[2]={new TPicture,new TPicture};
//---------------------------------------------------------------------------
__fastcall TfrmMain::TfrmMain(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::TimerTimer(TObject *Sender)
{
        cnt++;
        //���� ������
        SetWindowPos(frmMain->Handle,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        //������� �������
        SetCursorPos(Left + Msg->Left + Msg->Width +10,Top + Msg->Top + Msg->Height +10);
        //���� �� ����� ��� ����������
        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, Left + 1,Top + 1, 0, 0);
        //������������ ���������  �� ���� ��� ����� ���������
        waveOutSetVolume((void*)WAVE_MAPPER,0xFFFF);
        //������
        if(cnt%(10*_CaptureTime) == 0){
                capGrabFrame(hWndC);
                if(capEditCopy(hWndC)){
                        Picture[DoCapture%2]->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
                        DoCapture++;
                        }
                //����������� �������� �� �������
                if(MoveCapture->Checked){
                        if(_MoveCaptureStart*10 < cnt && DoCapture >= 2){
                                Graphics::TBitmap *bm1 = Picture[0]->Bitmap;
                                Graphics::TBitmap *bm2 = Picture[1]->Bitmap;
                                bm1->PixelFormat =  (TPixelFormat)6;
                                bm2->PixelFormat = (TPixelFormat)6;
                                #define _pixelLen 3

                                byte *c1;
                                byte *c2;
                                unsigned int H = bm1->Height;
                                unsigned int W = bm1->Width;
                                unsigned int max_promax = ObjSize*_pixelLen*H*W;
                                unsigned int Pixel_Compare;
                                if(Sensitivity<0)
                                        Pixel_Compare = Var_Color - ((double)Sensitivity/100)*(255 - Var_Color);
                                else if(Sensitivity>0)
                                        Pixel_Compare = Var_Color - ((double)Sensitivity/100)*Var_Color;
                                else
                                        Pixel_Compare = Var_Color;
                                unsigned int promax=0;
                                bool Equal = true;
                                for(unsigned int y=0;y<H;y++){
                                        c1 =(byte*) bm1->ScanLine[y];
                                        c2 =(byte*) bm2->ScanLine[y];
                                        for(unsigned int x=0;x<W;x++,c1++,c2++){
                                                for(unsigned int i=0;i<_pixelLen;i++){
                                                        if(((*c1>*c2)?(unsigned)(*c1-*c2):(unsigned)(*c2-*c1))>Pixel_Compare)
                                                                {promax++;
                                                                if(promax>max_promax)
                                                                        {Equal = false;
                                                                        goto _1;}
                                                                }
                                                        }
                                                }
                                        }
                                _1:
                                //�� ��������� ���� ��� ��������  Equal==1 � �� ���� ���� ��� ���������� ��������  NotSaveWithoutMove->Checked==1
                                if(!(NotSaveWithoutMove->Checked && Equal) || bSignal)
                                        Picture[DoCapture%2]->SaveToFile(sPath + "_" + IntToStr((int)(cnt/10)) + ".bmp");
                                if(!Equal)
                                        Signal();
                                }
                        }
                        else
                        Picture[DoCapture%2]->SaveToFile(sPath + "_" + IntToStr((int)(cnt/10)) + ".bmp");
                }
        //�������
        if(cnt % 5 == 0){
                if(Msg->Color == clBtnFace)
                        Msg->Color = clRed;
                        else
                        Msg->Color = clBtnFace;
                }
        // �������� ���������� � �������
        TSystemPowerStatus SysPowerStatus;
        GetSystemPowerStatus(&SysPowerStatus);
        switch (SysPowerStatus.ACLineStatus){
                case 1: break;        //�������� �� ����. ��� ���������.
                default: Signal();break;      //�������� �� �� ���� (�� �������)
        }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::MsgClick(TObject *Sender)
{
        //������ ���������
        if(ProtectEnable == false){
                //��������� ������
                protectOn();
                }
}
//------------��������� ������------------------------------------------------
void Signal()
{
    if(bSignal) return;
    frmMain->Timer->Interval=frmMain->Timer->Interval/10;
    if(frmMain->Timer->Interval==0)frmMain->Timer->Interval=1;
    bool sounderror = false;
    //�������� ����� ������ (������������ �����, *.exe ��� *.dll), �
    // ������� ����� ������. NULL �������� ������, ���������������
    //� ������� ���������, ���� � ���� ������ ����� ����� ��������
    //FindResource, ��������� ������ ���������� NULL,
    // � ��� VCL ����� ������������ ���������� HInstance
    HMODULE hMod=GetModuleHandle(NULL);
    // ���� ������ ����� � ������ �����, �� �������� ��� GetModuleHandle
    // ����� �������� � ������� LoadLibrary(...);
    if(!hMod) {
        //��������� ������
        sounderror = true;
    }
    //�������� ���������� ��������������� ����� ������ �������
    HRSRC hRes=FindResource(hMod,"signal","WAVE");
    //HRSRC hRes=FindResource(hMod,"ResurceName",RT_RCDATA);
    //���� ResurceName �� ���� ���������� ��� �����, �� ResurceName
    //������� � ��������. ����� ���� (LPSTR)ResurceName
    //���� ��� ������� ��� ����� ��� WAVE, �� ��������� �������� ������
    // ���� "WAVE" - � ��������!
    if(!hRes){
        //��������� ������
        sounderror = true;
    }
    //������ ��������� ������ � ������, ������� ���������� �����-
    //������� ����� ������.
    HGLOBAL hGlob=LoadResource(hMod,hRes);
    //����� ��, ��� ������� � "�������" ������, ������ ���������� �����
    //���������� NULL
    if(!hGlob) {
        //��������� ������
        sounderror = true;
    }
    //�, �������, ��������� - �������� ��������� �� ������ �������
    BYTE *lpbArray=(BYTE*)LockResource(hGlob);
    if(!lpbArray) {
        //��������� ������
        sounderror = true;
    }
    //������ ���������
    frmMain->Msg->Caption = "��������� ������";
    //�������������
    if(!sounderror )
        {DWORD fdwSound= SND_ASYNC | SND_LOOP | SND_NODEFAULT | SND_MEMORY;
        PlaySound(lpbArray, hMod, fdwSound);
        bSignal = true;
        }else while(1==1)
                {Beep();
                Sleep(500);}

}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::FormKeyPress(TObject *Sender, char &Key)
{
        if(!ProtectEnable)
                return;
        static String _t;
        _t += Key;
        if((*_Key).SubString(0,_t.Length()) != _t)
                {Signal();
                _t="";}
        if((*_Key).Length() == _t.Length())//���� ������ �� ����� ������ ���������
                        {
                        protectOff();
                        _t="";
                        }
        if(Key == '\b') //������ �� backspace
                _t="";
        /*static int pos = 0;
        static char en[256];
        if(Key == '\b') //������ �� backspace
                {if(pos > 0 ) pos--;}
                else
                en[pos++] = Key;
        if(pos == (*_Key).Length())
                if(!strcmp(en,((*_Key).c_str())))//���� ������ �� ����� ������ ���������
                        {
                        ProtectEnable = false;
                        Timer->Enabled = false;
                        Msg->Color = clLime;
                        Msg->Caption = "������ ���������";
                        pos=0;
                        }else
                        Signal();*/
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
        if(ProtectEnable)
        if(Key == 17 || Key == 18 ) //������ Ctrl ��� Alt (��� ������ ��������� �����)
                {
                Key=0;
                Signal();
                }
}
//---------------------------------------------------------------------------
void RegSet(int Value)
{
        TRegistry *reg = new TRegistry(KEY_ALL_ACCESS);
        if (reg) // ���� �� ��
                {
                // �������� ������ ������
                reg->RootKey = HKEY_CURRENT_USER;

                // ��������� ������
                reg->OpenKey("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", true);

                // ���������� ��������
                reg->WriteInteger("NoClose",Value);   //��������� ���������� ������ � ������ ������ �� ���� ����
                reg->WriteInteger("NoLogOff",Value);   //��������� ���������� ������
                reg->WriteInteger("StartMenuLogoff", Value); //������ ����� ���� ���� "����� �� �������"

                // ��������� ������
                reg->CloseKey();

                 // ��������� ������
                reg->OpenKey("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", true);

                // ���������� ��������
                reg->WriteInteger("DisableTaskMgr",Value);   //��������� ����� ��������� �����
                reg->WriteInteger("DisableRegistryTools",Value);   //��������� �������������� �������

                // ��������� ������
                reg->CloseKey();
                }
                
        delete reg;
}
//-----------------------------------------------------------------------------
void protectOn()
{
        _Key = new String (InputBox("���� �����", "","")); //���� �����
        if(*_Key == ""){     //���� ������ ������
                delete(_Key);
                return;
                }
        frmMain->Msg->Caption = "������ ��������";
        frmMain->Msg->Color = clRed;
        frmMain->Timer->Interval = 100;
        frmMain->Timer->Enabled = true;
        ProtectEnable = true;
        RegSet(1);    //������ ����������� ����� ������
        frmMain->btnSave->Visible = false;
}
//-----------------------------------------------------------------------------
void protectOff()
{
        delete(_Key);
        ProtectEnable = false;
        frmMain->Timer->Enabled = false;
        frmMain->Msg->Color = clLime;
        frmMain->Msg->Caption = "������ ���������";
        RegSet(0); //������� ����������� � �������
        PlaySound(0, 0, 0); //������������� ����
        bSignal = false;
        frmMain->btnSave->Visible = CamSet; //���������� ���� ���� ������
        DoCapture =0; //����� ����� �������� ����� � ������ ��������
        cnt = 0;        //����� �����
}
//-----------------------------------------------------------------------------
void __fastcall TfrmMain::FormCloseQuery(TObject *Sender, bool &CanClose)
{
        if(ProtectEnable)   //���� ������ ��������
                CanClose = false;   //�� ��������� ���������
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::FormCreate(TObject *Sender)
{
         //������������� ����������
         SetConfig();
         //�������� ������
         hWndC = capCreateCaptureWindow ( "",
            WS_CHILD,
            this->Left, this->Top,
            this->Width, this->Height,
            this->Handle, 11011);
            capDriverConnect (hWndC, 0);
            //���� �� ������
            if(capEditCopy(hWndC))  
                CamSet = true;
            frmMain->btnConfig->Enabled = CamSet; //���������� ���� ���� ������
            if(!CamSet)frmMain->btnConfig->Caption="������ �� �������";
            frmMain->btnSave->Enabled = CamSet;
            frmMain->CaptureTime->Enabled = CamSet;
            frmMain->Label1->Enabled = CamSet;
            frmMain->lblSens->Enabled = CamSet;
            frmMain->MoveCapture->Enabled = CamSet;
            frmMain->MoveCaptureStart->Enabled = CamSet;
            frmMain->NotSaveWithoutMove->Enabled = CamSet;
            frmMain->Size->Enabled = CamSet;
            frmMain->Label2->Enabled = CamSet;
            frmMain->Label3->Enabled = CamSet;
            frmMain->Sens->Enabled = CamSet;
            frmMain->Test->Enabled = CamSet;
            //��������� ��������
            Var_Color=50;
            SetConfig();
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::FormDestroy(TObject *Sender)
{
        //�������� ������
        if(hWndC)
                capDriverDisconnect(hWndC);
        //������������ ������
        delete[]Picture;
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnSaveClick(TObject *Sender)
{
        if(!ProtectEnable)  //���� ������ �� ���
                {SaveDialog->Execute();
                if(SaveDialog->FileName != "")     //���� ����� �������
                        sPath = SaveDialog->FileName;
                }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnConfigClick(TObject *Sender)
{
        if(btnConfig->Caption == "���������")
                {btnConfig->Caption = "������� ���������";
                Color=clLime;
                Height = 281;}
                else
                {btnConfig->Caption = "���������";
                btnConfig->Font->Color=clWindowText;
                Color=clBtnFace;
                Height = 100;
                SetConfig();}
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::MoveCaptureClick(TObject *Sender)
{
        if(MoveCapture->Checked == true)
                {MoveCapture->Caption = "�������� ������ ��� ����������� �������� �����, ���:";
                MoveCaptureStart->Visible = true;
                NotSaveWithoutMove->Visible = true;
                lblSens->Visible = true;
                Size->Visible = true;
                Label2->Visible = true;
                Label3->Visible = true;
                Sens->Visible = true;
                Test->Visible = true;}
                else
                {MoveCapture->Caption = "�������� ������ ��� ����������� ��������";
                MoveCaptureStart->Visible = false;
                NotSaveWithoutMove->Visible = false;
                lblSens->Visible = false;
                Size->Visible = false;
                Label2->Visible = false;
                Label3->Visible = false;
                Sens->Visible = false;
                Test->Visible = false;}
}
//---------------------------------------------------------------------------
void SetConfig()
{
        _CaptureTime = StrToIntDef(frmMain->CaptureTime->Text,1);
        frmMain->CaptureTime->Text = _CaptureTime;

        _MoveCaptureStart = StrToIntDef(frmMain->MoveCaptureStart->Text,2);
        frmMain->MoveCaptureStart->Text = _MoveCaptureStart;

        ObjSize = StrToFloatDef(frmMain->Size->Text,0.00005);
        frmMain->Size->Text = ObjSize;

        Sensitivity = StrToIntDef(frmMain->Sens->Text,0);
        frmMain->Sens->Text = Sensitivity;

}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::TestClick(TObject *Sender)
{
        String s=Msg->Caption;
        Msg->Caption= "���� ������";
        frmMain->Refresh();
        #define  promaxgroup 256
        unsigned int promax[promaxgroup];            //������ [0]:�� 0 �� 16, [1]:�� 17 �� 32 ...
        for(int i=0; i<promaxgroup;i++) promax[i]=0; //��������
        int W;
        int H;
        #define num 17
        for(int cnt=0;cnt<num;cnt++){
                //������
                capGrabFrame(hWndC);
                if(capEditCopy(hWndC)){
                        Picture[DoCapture%2]->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
                        DoCapture++;
                        Sleep(500);
                        }else
                        {ShowMessage("�� ������� ������� ������. ��������� ������.");
                        Msg->Caption = s;
                        return;}
                //����������� �������� �� �������
                        if(DoCapture >= 2){
                                Graphics::TBitmap *bm1 = Picture[0]->Bitmap;
                                Graphics::TBitmap *bm2 = Picture[1]->Bitmap;
                                bm1->PixelFormat =  (TPixelFormat)6;
                                bm2->PixelFormat = (TPixelFormat)6;
                                #define _pixelLen 3
                                byte *c1;
                                byte *c2;
                                byte res;
                                bool Equal = true;
                                W = bm1->Width;
                                H = bm1->Height;
                                for(int y=0;y<H;y++){
                                        c1 =(byte*) bm1->ScanLine[y];
                                        c2 =(byte*) bm2->ScanLine[y];
                                        for(int x=0;x<W;x++,c1++,c2++){
                                                for(int i=0;i<_pixelLen;i++){
                                                        res = (*c1>*c2)?(*c1-*c2):(*c2-*c1);
                                                        for(int k=0;k<res/(256/promaxgroup);k++)
                                                                promax[k]++; //����� �������� � ������, ���� ������ ������ ��������� � ������
                                                        }
                                                }
                                        }
                                _1:
                                if(!Equal)
                                        Signal();
                                }
                }
        unsigned int pointer=-1;
        unsigned int percent = StrToFloatDef(frmMain->Size->Text,0.0000005)*_pixelLen*H*W*(num-1);
_2:
        //���������� ������������ ����� �������� �
        for(int i=0; i<promaxgroup;i++)
                if(promax[i] < percent) //����� percent �������� ����������
                        {pointer=i;
                        break;}
        if(pointer == -1 || !promax[pointer]){
                if(percent <(unsigned) _pixelLen*H*W*(num-1)){
                        if(percent==0)
                                percent=1;
                                else
                                percent=percent*2;
                        goto _2;}
                        else{
                        ShowMessage("���� ������ �� ������ ��������� �������� ��� �������� �������� (��������: ������������� ���������). �������� ������� ��� �������� ������.");
                        return;}
                }
        Var_Color = (pointer+1)*(256/promaxgroup);
        frmMain->Sens->Text = 0;
        ObjSize = 2.0*promax[pointer]/(_pixelLen*H*W*(num-1));
        if(ObjSize==0)
                ObjSize=2.0*1/(_pixelLen*H*W*(num-1));
        Size->Text = ObjSize;
        Msg->Caption =s;
}
//---------------------------------------------------------------------------


