#ifndef PAPERSIZES_H
#define PAPERSIZES_H
#include <QMap>
#include <QSizeF>

static QMap<QString, QString> CalligraPaperSizes =
   {{"iso_a0_841x1189mm", "A0"},
    {"iso_a1_594x841mm", "A1"},
    {"iso_a2_420x594mm", "A2"},
    {"iso_a3_297x420mm", "A3"},
    {"iso_a4_210x297mm", "A4"},
    {"iso_a5_148x210mm", "A5"},
    {"iso_a6_105x148mm", "A6"},
    {"iso_a7_74x105mm", "A7"},
    {"iso_a8_52x74mm", "A8"},
    {"iso_a9_37x52mm", "A9"},
    {"iso_b0_1000x1414mm", "B0"},
    {"iso_b1_707x1000mm", "B1"},
    {"iso_b2_500x707mm", "B2"},
    {"iso_b3_353x500mm", "B3"},
    {"iso_b4_250x353mm", "B4"},
    {"iso_b5_176x250mm", "B5"},
    {"iso_b6_125x176mm", "B6"},
    {"iso_b7_88x125mm", "B7"},
    {"iso_b8_62x88mm", "B8"},
    {"iso_b9_44x62mm", "B9"},
    {"iso_b10_31x44mm", "B10"},
    {"iso_c5_162x229mm", "C5E"},
    {"na_number-10_4.125x9.5in", "Comm10E"},
    {"iso_dl_110x220mm", "DLE"},
    {"na_executive_7.25x10.5in", "Executive"},
    {"om_folio_210x330mm", "Folio"},
    {"na_ledger_11x17in", "Ledger"},
    {"na_legal_8.5x14in", "Legal"},
    {"na_letter_8.5x11in", "Letter"},
    {"na_ledger_11x17in", "Tabloid"} // Dimension match
   };



#endif // PAPERSIZES_H
