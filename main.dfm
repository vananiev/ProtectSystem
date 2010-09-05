object frmMain: TfrmMain
  Left = 424
  Top = 238
  BorderIcons = [biSystemMenu, biMaximize]
  BorderStyle = bsNone
  ClientHeight = 99
  ClientWidth = 357
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnCloseQuery = FormCloseQuery
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  OnKeyPress = FormKeyPress
  PixelsPerInch = 96
  TextHeight = 13
  object Msg: TLabel
    Left = 56
    Top = 32
    Width = 263
    Height = 37
    Cursor = crHandPoint
    Caption = #1047#1072#1097#1080#1090#1072' '#1086#1090#1082#1083#1102#1095#1077#1085#1072
    Color = clLime
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -32
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentColor = False
    ParentFont = False
    OnClick = MsgClick
  end
  object Label1: TLabel
    Left = 8
    Top = 152
    Width = 182
    Height = 13
    Caption = #1055#1088#1086#1084#1077#1078#1091#1090#1086#1082' '#1084#1077#1078#1076#1091' '#1089#1085#1080#1084#1082#1072#1084#1080', '#1089#1077#1082':'
  end
  object lblSens: TLabel
    Left = 16
    Top = 248
    Width = 236
    Height = 13
    Caption = #1056#1072#1079#1084#1077#1088' '#1086#1073#1098#1077#1082#1090#1072' '#1076#1074#1080#1078#1077#1085#1080#1103', '#1074' % '#1086#1090' '#1088#1072#1079#1084'. '#1082#1072#1076#1072':'
    Visible = False
  end
  object Label2: TLabel
    Left = 44
    Top = 228
    Width = 226
    Height = 13
    Caption = '(-100: '#1085#1080#1079#1082#1072#1103', 0: '#1085#1086#1088#1084#1072#1083#1100#1085#1072#1103';+100: '#1074#1099#1089#1086#1082#1072#1103')'
    Visible = False
  end
  object Label3: TLabel
    Left = 16
    Top = 216
    Width = 111
    Height = 13
    Caption = #1063#1091#1074#1089#1090#1074#1080#1090#1077#1083#1100#1085#1086#1089#1090#1100', %:'
    Visible = False
  end
  object btnSave: TButton
    Left = 4
    Top = 120
    Width = 169
    Height = 21
    Caption = #1052#1077#1089#1090#1086' '#1089#1086#1093#1088#1072#1085#1077#1085#1080#1103' '#1092#1086#1090#1086#1075#1088#1072#1092#1080#1081
    TabOrder = 0
    OnClick = btnSaveClick
  end
  object btnConfig: TButton
    Left = 4
    Top = 80
    Width = 145
    Height = 17
    Caption = #1053#1072#1089#1090#1088#1086#1081#1082#1080
    TabOrder = 1
    OnClick = btnConfigClick
  end
  object CaptureTime: TEdit
    Left = 320
    Top = 144
    Width = 33
    Height = 21
    TabOrder = 2
  end
  object MoveCapture: TCheckBox
    Left = 8
    Top = 172
    Width = 309
    Height = 17
    Caption = #1042#1082#1083#1102#1095#1080#1090#1100' '#1089#1080#1075#1085#1072#1083' '#1087#1088#1080' '#1086#1073#1085#1072#1088#1091#1078#1077#1085#1080#1080' '#1076#1074#1080#1078#1077#1085#1080#1103
    TabOrder = 3
    OnClick = MoveCaptureClick
  end
  object MoveCaptureStart: TEdit
    Left = 320
    Top = 168
    Width = 33
    Height = 21
    TabOrder = 4
    Visible = False
  end
  object NotSaveWithoutMove: TCheckBox
    Left = 16
    Top = 196
    Width = 265
    Height = 17
    Caption = #1053#1077' '#1089#1086#1093#1088#1072#1085#1103#1090#1100' '#1092#1086#1090#1086#1075#1088#1072#1092#1080#1080', '#1077#1089#1083#1080' '#1085#1077#1090' '#1076#1074#1080#1078#1077#1085#1080#1103
    Checked = True
    State = cbChecked
    TabOrder = 5
    Visible = False
  end
  object Size: TEdit
    Left = 256
    Top = 244
    Width = 97
    Height = 21
    TabOrder = 6
    Visible = False
  end
  object Sens: TEdit
    Left = 320
    Top = 224
    Width = 33
    Height = 21
    TabOrder = 7
    Visible = False
  end
  object Test: TButton
    Left = 0
    Top = 264
    Width = 357
    Height = 17
    Hint = 
      #1042' '#1082#1072#1076#1088#1077' '#1085#1077' '#1076#1086#1083#1078#1085#1086' '#1073#1099#1090#1100' '#1087#1086#1076#1074#1080#1078#1085#1099#1093' '#1087#1088#1077#1076#1084#1077#1090#1086#1074' ('#1082#1088#1086#1084#1077' '#1090#1077#1093' '#1082#1086#1090#1086#1088#1099#1077' '#1086#1078 +
      #1080#1076#1072#1102#1090#1089#1103' '#1087#1088#1080' '#1074#1082#1083#1102#1095#1077#1085#1085#1086#1081' '#1079#1072#1097#1080#1090#1077')'
    Caption = #1040#1074#1090#1086#1084#1072#1090#1080#1095#1077#1089#1082#1080' '#1089#1082#1086#1085#1092#1080#1075#1091#1088#1080#1088#1086#1074#1072#1090#1100' '#1082#1072#1084#1077#1088#1091' '#1085#1072' '#1085#1077#1087#1086#1076#1074#1080#1078#1085#1086#1089#1090#1100
    ParentShowHint = False
    ShowHint = True
    TabOrder = 8
    Visible = False
    OnClick = TestClick
  end
  object Timer: TTimer
    Enabled = False
    Interval = 100
    OnTimer = TimerTimer
  end
  object SaveDialog: TSaveDialog
    Filter = '*.bmp'
    Left = 36
  end
end
