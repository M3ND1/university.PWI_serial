object Fmain: TFmain
  Left = 393
  Top = 103
  Width = 457
  Height = 337
  Caption = 'Fmain'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Button1: TButton
    Left = 8
    Top = 8
    Width = 105
    Height = 25
    Caption = 'Connect COM2'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Memo1: TMemo
    Left = 8
    Top = 48
    Width = 273
    Height = 249
    TabOrder = 1
  end
  object Button2: TButton
    Left = 296
    Top = 72
    Width = 75
    Height = 25
    Caption = 'Send'
    TabOrder = 2
    OnClick = Button2Click
  end
  object Edit1: TEdit
    Left = 296
    Top = 48
    Width = 121
    Height = 21
    TabOrder = 3
    Text = 'Hello world'
  end
  object Button3: TButton
    Left = 120
    Top = 8
    Width = 97
    Height = 25
    Caption = 'Disconnect'
    TabOrder = 4
    OnClick = Button3Click
  end
end
