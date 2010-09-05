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

#include <vfw.h>         //камера
#include <Clipbrd.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
bool ProtectEnable = false; //состояние защиты
bool bSignal = false;   //проигрывается сигнал
bool CamSet = false;    //есть ли камера в системе
void Signal(); //срабатывание сигнализации
String *_Key;   //ключ
String sPath="C:\\"; //путь для сохранения фоток
void RegSet(int Value); //установка реестра
void protectOn();       //вкл защиту
void protectOff();      //откл защиту
HWND hWndC;             //работа с камерой
void SetConfig();       //установить настройки
unsigned int _CaptureTime; //время между снимками в секундах
unsigned int _MoveCaptureStart; //время до начала проверки движения в камере
double ObjSize ;        //размер объекта движения в %
signed int Sensitivity; //чувствительность %
unsigned int Var_Color;    //различие цветов на снимке из-за шумов
unsigned int DoCapture=0;     //число снимков за сеанс
unsigned int cnt = 0;           //число тиков с начала защиты
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
        //окно вверху
        SetWindowPos(frmMain->Handle,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        //позиция курсора
        SetCursorPos(Left + Msg->Left + Msg->Width +10,Top + Msg->Top + Msg->Height +10);
        //клик по форме для активности
        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, Left + 1,Top + 1, 0, 0);
        //корректируем громкость  на макс для своей программы
        waveOutSetVolume((void*)WAVE_MAPPER,0xFFFF);
        //снимок
        if(cnt%(10*_CaptureTime) == 0){
                capGrabFrame(hWndC);
                if(capEditCopy(hWndC)){
                        Picture[DoCapture%2]->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
                        DoCapture++;
                        }
                //определения движения на снимках
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
                                //НЕ сохраняем если нет движения  Equal==1 и не надо сохр при отсутствии движения  NotSaveWithoutMove->Checked==1
                                if(!(NotSaveWithoutMove->Checked && Equal) || bSignal)
                                        Picture[DoCapture%2]->SaveToFile(sPath + "_" + IntToStr((int)(cnt/10)) + ".bmp");
                                if(!Equal)
                                        Signal();
                                }
                        }
                        else
                        Picture[DoCapture%2]->SaveToFile(sPath + "_" + IntToStr((int)(cnt/10)) + ".bmp");
                }
        //Мигание
        if(cnt % 5 == 0){
                if(Msg->Color == clBtnFace)
                        Msg->Color = clRed;
                        else
                        Msg->Color = clBtnFace;
                }
        // Получить информацию о питании
        TSystemPowerStatus SysPowerStatus;
        GetSystemPowerStatus(&SysPowerStatus);
        switch (SysPowerStatus.ACLineStatus){
                case 1: break;        //питаемся от сети. Все нормально.
                default: Signal();break;      //питаемся не от сети (от батареи)
        }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::MsgClick(TObject *Sender)
{
        //меняем состояние
        if(ProtectEnable == false){
                //запускаем защиту
                protectOn();
                }
}
//------------Нарушение защиты------------------------------------------------
void Signal()
{
    if(bSignal) return;
    frmMain->Timer->Interval=frmMain->Timer->Interval/10;
    if(frmMain->Timer->Interval==0)frmMain->Timer->Interval=1;
    bool sounderror = false;
    //Получаем хэндл модуля (исполняемого файла, *.exe или *.dll), в
    // котором лежит ресурс. NULL означает модуль, ассоциированный
    //с текущим процессом, хотя в этом случае можно сразу вызывать
    //FindResource, передавая первым параметром NULL,
    // а под VCL можно использовать глобальную HInstance
    HMODULE hMod=GetModuleHandle(NULL);
    // если ресурс лежит в другом файле, то параметр для GetModuleHandle
    // можно получить с помощью LoadLibrary(...);
    if(!hMod) {
        //Обработка ошибки
        sounderror = true;
    }
    //Получаем дескриптор информационного блока нашего ресурса
    HRSRC hRes=FindResource(hMod,"signal","WAVE");
    //HRSRC hRes=FindResource(hMod,"ResurceName",RT_RCDATA);
    //Если ResurceName не было определено как число, то ResurceName
    //берется в кавычках. Иначе надо (LPSTR)ResurceName
    //Если тип ресурса был задан как WAVE, то последний параметр должен
    // быть "WAVE" - в кавычках!
    if(!hRes){
        //Обработка ошибки
        sounderror = true;
    }
    //Теперь загружаем ресурс в память, получая дескриптор загру-
    //женного блока памяти.
    HGLOBAL hGlob=LoadResource(hMod,hRes);
    //Опять же, для ресурса в "текущем" модуле, первым параметром можно
    //передавать NULL
    if(!hGlob) {
        //Обработка ошибки
        sounderror = true;
    }
    //И, наконец, последнее - получаем указатель на начало массива
    BYTE *lpbArray=(BYTE*)LockResource(hGlob);
    if(!lpbArray) {
        //Обработка ошибки
        sounderror = true;
    }
    //меняем сообщение
    frmMain->Msg->Caption = "Нарушение защиты";
    //воспроизводим
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
        if((*_Key).Length() == _t.Length())//весь пароль до конца введен правильно
                        {
                        protectOff();
                        _t="";
                        }
        if(Key == '\b') //нажата ли backspace
                _t="";
        /*static int pos = 0;
        static char en[256];
        if(Key == '\b') //нажата ли backspace
                {if(pos > 0 ) pos--;}
                else
                en[pos++] = Key;
        if(pos == (*_Key).Length())
                if(!strcmp(en,((*_Key).c_str())))//весь пароль до конца введен правильно
                        {
                        ProtectEnable = false;
                        Timer->Enabled = false;
                        Msg->Color = clLime;
                        Msg->Caption = "Защита отключена";
                        pos=0;
                        }else
                        Signal();*/
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
        if(ProtectEnable)
        if(Key == 17 || Key == 18 ) //нажата Ctrl или Alt (для вызова диспечера задач)
                {
                Key=0;
                Signal();
                }
}
//---------------------------------------------------------------------------
void RegSet(int Value)
{
        TRegistry *reg = new TRegistry(KEY_ALL_ACCESS);
        if (reg) // если всё ОК
                {
                // выбираем нужный корень
                reg->RootKey = HKEY_CURRENT_USER;

                // открываем секцию
                reg->OpenKey("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", true);

                // записываем значения
                reg->WriteInteger("NoClose",Value);   //Запретить завершение работы и убрать кнопку из меню Пуск
                reg->WriteInteger("NoLogOff",Value);   //Запретить завершение сеанса
                reg->WriteInteger("StartMenuLogoff", Value); //Убрать пункт меню пуск "Выход из системы"

                // закрываем секцию
                reg->CloseKey();

                 // открываем секцию
                reg->OpenKey("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", true);

                // записываем значения
                reg->WriteInteger("DisableTaskMgr",Value);   //Запретить вызов диспечера задач
                reg->WriteInteger("DisableRegistryTools",Value);   //Запретить редактирование реестра

                // закрываем секцию
                reg->CloseKey();
                }
                
        delete reg;
}
//-----------------------------------------------------------------------------
void protectOn()
{
        _Key = new String (InputBox("Ввод ключа", "","")); //ввод ключа
        if(*_Key == ""){     //если нажали отмена
                delete(_Key);
                return;
                }
        frmMain->Msg->Caption = "Защита включена";
        frmMain->Msg->Color = clRed;
        frmMain->Timer->Interval = 100;
        frmMain->Timer->Enabled = true;
        ProtectEnable = true;
        RegSet(1);    //ставим ограничения через реестр
        frmMain->btnSave->Visible = false;
}
//-----------------------------------------------------------------------------
void protectOff()
{
        delete(_Key);
        ProtectEnable = false;
        frmMain->Timer->Enabled = false;
        frmMain->Msg->Color = clLime;
        frmMain->Msg->Caption = "Защита отключена";
        RegSet(0); //удаляем ограничения с реестра
        PlaySound(0, 0, 0); //останавливаем звук
        bSignal = false;
        frmMain->btnSave->Visible = CamSet; //включается если есть камера
        DoCapture =0; //сброс числа проверок фоток с начала проверки
        cnt = 0;        //сброс тиков
}
//-----------------------------------------------------------------------------
void __fastcall TfrmMain::FormCloseQuery(TObject *Sender, bool &CanClose)
{
        if(ProtectEnable)   //если защита включена
                CanClose = false;   //не разрешаем закрывать
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::FormCreate(TObject *Sender)
{
         //инициализация переменных
         SetConfig();
         //включаем камеру
         hWndC = capCreateCaptureWindow ( "",
            WS_CHILD,
            this->Left, this->Top,
            this->Width, this->Height,
            this->Handle, 11011);
            capDriverConnect (hWndC, 0);
            //есть ли камера
            if(capEditCopy(hWndC))  
                CamSet = true;
            frmMain->btnConfig->Enabled = CamSet; //включается если есть камера
            if(!CamSet)frmMain->btnConfig->Caption="Камера не найдена";
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
            //начальные значения
            Var_Color=50;
            SetConfig();
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::FormDestroy(TObject *Sender)
{
        //закрытие камеры
        if(hWndC)
                capDriverDisconnect(hWndC);
        //освобождение памяти
        delete[]Picture;
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnSaveClick(TObject *Sender)
{
        if(!ProtectEnable)  //если защита не вкл
                {SaveDialog->Execute();
                if(SaveDialog->FileName != "")     //если папка выбрана
                        sPath = SaveDialog->FileName;
                }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnConfigClick(TObject *Sender)
{
        if(btnConfig->Caption == "Настройки")
                {btnConfig->Caption = "Принять настройки";
                Color=clLime;
                Height = 281;}
                else
                {btnConfig->Caption = "Настройки";
                btnConfig->Font->Color=clWindowText;
                Color=clBtnFace;
                Height = 100;
                SetConfig();}
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::MoveCaptureClick(TObject *Sender)
{
        if(MoveCapture->Checked == true)
                {MoveCapture->Caption = "Включить сигнал при обнаружении движения через, сек:";
                MoveCaptureStart->Visible = true;
                NotSaveWithoutMove->Visible = true;
                lblSens->Visible = true;
                Size->Visible = true;
                Label2->Visible = true;
                Label3->Visible = true;
                Sens->Visible = true;
                Test->Visible = true;}
                else
                {MoveCapture->Caption = "Включить сигнал при обнаружении движения";
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
        Msg->Caption= "Тест камеры";
        frmMain->Refresh();
        #define  promaxgroup 256
        unsigned int promax[promaxgroup];            //группы [0]:от 0 до 16, [1]:от 17 до 32 ...
        for(int i=0; i<promaxgroup;i++) promax[i]=0; //обнуляем
        int W;
        int H;
        #define num 17
        for(int cnt=0;cnt<num;cnt++){
                //снимок
                capGrabFrame(hWndC);
                if(capEditCopy(hWndC)){
                        Picture[DoCapture%2]->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
                        DoCapture++;
                        Sleep(500);
                        }else
                        {ShowMessage("Не удалось сделать снимки. Проверьте камеру.");
                        Msg->Caption = s;
                        return;}
                //определения движения на снимках
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
                                                                promax[k]++; //число попадает в группы, если меньше порога попадания в группу
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
        //определяем максимальное число промахов и
        for(int i=0; i<promaxgroup;i++)
                if(promax[i] < percent) //менее percent пикселей изменились
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
                        ShowMessage("Ваша камера не сможет отследить движение при заданных условиях (например: недостаточное освещение). Измените условия или замените камеру.");
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


