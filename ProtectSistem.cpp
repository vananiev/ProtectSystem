//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("main.cpp", frmMain);
//---------------------------------------------------------------------------
LRESULT CALLBACK KeyboardProc(int code,WPARAM wParam,LPARAM lParam)
{
        1==1;            
        return 0;
}
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->CreateForm(__classid(TfrmMain), &frmMain);
                 //HHOOK hKeybHook;
                 //hKeybHook = SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)KeyboardProc,NULL,NULL); //устанавливаем перехватчик для Клавишных событий
                 Application->Run();
                 //UnhookWindowsHookEx(hKeybHook);
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
        return 0;
}
//---------------------------------------------------------------------------

