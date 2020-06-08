function supported_formats(printer, ConvertChecker, considerAdditionalFormats)
{
    var formats = printer.attrs["document-format-supported"].value;
    if(considerAdditionalFormats)
    {
        formats=formats+printer.additionalDocumentFormats;
    }

    var mimetypes = [];
    var supported = [];
     if(has(formats, "application/pdf") ||
        (ConvertChecker.pdf && ( has(formats, "application/postscript") ||
                                 has(formats, "image/pwg-raster") ||
                                 has(formats, "image/urf"))) )
     {
         mimetypes.push("application/pdf");
         supported.push("PDF");
     }
     if(has(formats, "application/postscript"))
     {
         mimetypes.push("application/postscript");
         supported.push("Postscript");
     }

     if ( has(formats, "image/pwg-raster") || has(formats, "image/urf"))
     {
         mimetypes.push("image/jpeg");
         supported.push("JPEG");
         mimetypes.push("image/png");
         supported.push("PNG");
     }

     return {supported: supported.join(" "), mimetypes: mimetypes};
}

function has(arrayish, what)
{
    return arrayish.indexOf(what) != -1;
}

function ippName(name, value)
{
    switch(name) {
    case "job-state":
        switch(value) {
        case 3:
            return qsTr("pending");
        case 4:
            return qsTr("pending-held");
        case 5:
            return qsTr("processing");
        case 6:
            return qsTr("processing-stopped");
        case 7:
            return qsTr("canceled");
        case 8:
            return qsTr("aborted");
        case 9:
            return qsTr("completed");
        default:
            return qsTr("unknown state ")+value
        }
    case "print-quality":
        switch(value) {
        case 3:
            return qsTr("draft");
        case 4:
            return qsTr("normal");
        case 5:
            return qsTr("high");
        default:
            return qsTr("unknown quality ")+value
        }
    case "orientation-requested":
        switch(value) {
        case 3:
            return qsTr("portrait");
        case 4:
            return qsTr("landscape");
        case 5:
            return qsTr("reverse landscape");
        case 6:
            return qsTr("reverse portrait");
        default:
            return qsTr("unknown orientation ")+value
        }
    case "printer-resolution":
        var units = "";
        if(value.units==3) {
            units=qsTr("dpi");
        } else if (units==4){
            units=qsTr("dots/cm")
        }
        return ""+value.x+"x"+value.y+units;
    case "media":
        if(media.hasOwnProperty(value))
        {
            return media[value];
        }
        else
        {
            return value;
        }
    case "sides":
        switch(value) {
        case "one-sided":
            return qsTr("one-sided");
        case "two-sided-long-edge":
            return qsTr("two-sided");
        case "two-sided-short-edge":
            return qsTr("two-sided flipped");
        default:
            return value
        }
    case "print-color-mode":
        switch(value) {
        case "auto":
            return qsTr("automatic");
        case "auto-monochrome":
            return qsTr("auto monochrome");
        case "bi-level":
            return qsTr("bi-level");
        case "color":
            return qsTr("color");
        case "highlight":
            return qsTr("highlight");
        case "monochrome":
            return qsTr("monochrome");
        case "process-bi-level":
            return qsTr("process bi-level");
        case "process-monochrome":
            return qsTr("process monochrome");
        default:
            return value
        }
    case "document-format":
        switch(value) {
        case "application/octet-stream":
            return qsTr("auto-sense");
        case "application/pdf":
            return qsTr("PDF");
        case "application/postscript":
            return qsTr("Postscript");
        case "image/pwg-raster":
            return qsTr("PWG-raster");
        case "image/urf":
            return qsTr("URF-raster");
        case "image/png":
            return qsTr("PNG");
        case "image/jpeg":
            return qsTr("JPEG");
        case "image/gif":
            return qsTr("GIF");
        default:
            return value;
        }
    case "media-source":
        if(value.indexOf("tray-")==0)
        {
            return qsTr("tray")+" "+value.split("-")[1];
        }
        else
        {
            switch(value) {
            case "by-pass-tray":
                return qsTr("by-pass tray");
            case "auto":
                return qsTr("automatic");
            case "bottom":
                return qsTr("bottom");
            case "center":
                return qsTr("center");
            case "main":
                return qsTr("main");
            case "manual":
                return qsTr("manual");
            case "middle":
                return qsTr("middle");
            case "photo":
                return qsTr("photo");
            case "rear":
                return qsTr("rear");
            case "right":
                return qsTr("right");
            case "side":
                return qsTr("side");
            case "top":
                return qsTr("top");
            default:
                return value;
            }
        }
    }
    return value;
}

function endsWith(ending, string)
{
    return string.lastIndexOf(ending) == (string.length - ending.length);
}

function canConvertPdfTo(type)
{
    var targets = ["application/octet-stream", "application/pdf", "application/postscript", "image/pwg-raster", "image/urf"];
    return has(targets, type)
}

function canTransferPdfAs(type)
{
    var targets = ["application/octet-stream", "application/pdf"];
    return has(targets, type)
}

function canConvertImageTo(type)
{
    var targets = ["application/octet-stream", "image/jpeg", "image/png", "image/pwg-raster", "image/urf", "image/gif"];
    return has(targets, type)
}

function unitsIsDpi(resolution)
{
    return resolution.units == 3;
}


function limitChoices(name, choices, mimeType, ConvertChecker)
{
    switch(name) {
    case "document-format":
        if(mimeType == "application/pdf")
        {
            if(ConvertChecker.pdf)
            {
                return choices.filter(canConvertPdfTo)
            }
            else
            {
                return choices.filter(canTransferPdfAs)
            }

        }
        else if(mimeType == "image/jpeg" || mimeType == "image/png")
        {
            return choices.filter(canConvertImageTo);
        }
        else
        {
            return ["application/octet-stream"];
        }
    case "printer-resolution":
        return choices.filter(unitsIsDpi);
    default:
        return choices;
    }
}

var media =
       {"asme_f_28x40in": "28 x 40″",
        "choice_iso_a4_210x297mm_na_letter_8.5x11in": "A4 or US Letter",
        "iso_2a0_1189x1682mm": "2a0",
        "iso_a0_841x1189mm": "A0",
        "iso_a0x3_1189x2523mm": "A0x3",
        "iso_a10_26x37mm": "A10",
        "iso_a1_594x841mm": "A1",
        "iso_a1x3_841x1783mm": "A1x3",
        "iso_a1x4_841x2378mm": "A1x4",
        "iso_a2_420x594mm": "A2",
        "iso_a2x3_594x1261mm": "A2x3",
        "iso_a2x4_594x1682mm": "A2x4",
        "iso_a2x5_594x2102mm": "A2x5",
        "iso_a3-extra_322x445mm": "A3 (Extra)",
        "iso_a3_297x420mm": "A3",
        "iso_a3x3_420x891mm": "A3x3",
        "iso_a3x4_420x1189mm": "A3x4",
        "iso_a3x5_420x1486mm": "A3x5",
        "iso_a3x6_420x1783mm": "A3x6",
        "iso_a3x7_420x2080mm": "A3x7",
        "iso_a4-extra_235.5x322.3mm": "A4 (Extra)",
        "iso_a4-tab_225x297mm": "A4 (Tab)",
        "iso_a4_210x297mm": "A4",
        "iso_a4x3_297x630mm": "A4x3",
        "iso_a4x4_297x841mm": "A4x4",
        "iso_a4x5_297x1051mm": "A4x5",
        "iso_a4x6_297x1261mm": "A4x6",
        "iso_a4x7_297x1471mm": "A4x7",
        "iso_a4x8_297x1682mm": "A4x8",
        "iso_a4x9_297x1892mm": "A4x9",
        "iso_a5-extra_174x235mm": "A5 (Extra)",
        "iso_a5_148x210mm": "A5",
        "iso_a6_105x148mm": "A6",
        "iso_a7_74x105mm": "A7",
        "iso_a8_52x74mm": "A8",
        "iso_a9_37x52mm": "A9",
        "iso_b0_1000x1414mm": "B0",
        "iso_b10_31x44mm": "B10",
        "iso_b1_707x1000mm": "B1",
        "iso_b2_500x707mm": "B2",
        "iso_b3_353x500mm": "B3",
        "iso_b4_250x353mm": "B4",
        "iso_b5-extra_201x276mm": "B5 (Extra)",
        "iso_b5_176x250mm": "Envelope B5",
        "iso_b6_125x176mm": "B6",
        "iso_b6c4_125x324mm": "Envelope B6/C4",
        "iso_b7_88x125mm": "B7",
        "iso_b8_62x88mm": "B8",
        "iso_b9_44x62mm": "B9",
        "iso_c0_917x1297mm": "CEnvelope 0",
        "iso_c10_28x40mm": "CEnvelope 10",
        "iso_c1_648x917mm": "CEnvelope 1",
        "iso_c2_458x648mm": "CEnvelope 2",
        "iso_c3_324x458mm": "CEnvelope 3",
        "iso_c4_229x324mm": "CEnvelope 4",
        "iso_c5_162x229mm": "CEnvelope 5",
        "iso_c6_114x162mm": "CEnvelope 6",
        "iso_c6c5_114x229mm": "CEnvelope 6c5",
        "iso_c7_81x114mm": "CEnvelope 7",
        "iso_c7c6_81x162mm": "CEnvelope 7c6",
        "iso_c8_57x81mm": "CEnvelope 8",
        "iso_c9_40x57mm": "CEnvelope 9",
        "iso_dl_110x220mm": "Envelope DL",
        "iso_id-1_53.98x85.6mm": "Id-1",
        "iso_id-3_88x125mm": "Id-3",
        "iso_ra0_860x1220mm": "ISO RA0",
        "iso_ra1_610x860mm": "ISO RA1",
        "iso_ra2_430x610mm": "ISO RA2",
        "iso_ra3_305x430mm": "ISO RA3",
        "iso_ra4_215x305mm": "ISO RA4",
        "iso_sra0_900x1280mm": "ISO SRA0",
        "iso_sra1_640x900mm": "ISO SRA1",
        "iso_sra2_450x640mm": "ISO SRA2",
        "iso_sra3_320x450mm": "ISO SRA3",
        "iso_sra4_225x320mm": "ISO SRA4",
        "jis_b0_1030x1456mm": "JIS B0",
        "jis_b10_32x45mm": "JIS B10",
        "jis_b1_728x1030mm": "JIS B1",
        "jis_b2_515x728mm": "JIS B2",
        "jis_b3_364x515mm": "JIS B3",
        "jis_b4_257x364mm": "JIS B4",
        "jis_b5_182x257mm": "JIS B5",
        "jis_b6_128x182mm": "JIS B6",
        "jis_b7_91x128mm": "JIS B7",
        "jis_b8_64x91mm": "JIS B8",
        "jis_b9_45x64mm": "JIS B9",
        "jis_exec_216x330mm": "JIS Executive",
        "jpn_chou2_111.1x146mm": "Envelope Chou 2",
        "jpn_chou3_120x235mm": "Envelope Chou 3",
        "jpn_chou40_90x225mm": "Envelope Chou 40",
        "jpn_chou4_90x205mm": "Envelope Chou 4",
        "jpn_hagaki_100x148mm": "Hagaki",
        "jpn_kahu_240x322.1mm": "Envelope Kahu",
        "jpn_kaku1_270x382mm": "270 x 382mm",
        "jpn_kaku2_240x332mm": "Envelope Kahu 2",
        "jpn_kaku3_216x277mm": "216 x 277mm",
        "jpn_kaku4_197x267mm": "197 x 267mm",
        "jpn_kaku5_190x240mm": "190 x 240mm",
        "jpn_kaku7_142x205mm": "142 x 205mm",
        "jpn_kaku8_119x197mm": "119 x 197mm",
        "jpn_oufuku_148x200mm": "Oufuku Reply Postcard",
        "jpn_you4_105x235mm": "Envelope You 4",
        "na_10x11_10x11in": "10 x 11″",
        "na_10x13_10x13in": "10 x 13″",
        "na_10x14_10x14in": "10 x 14″",
        "na_10x15_10x15in": "10 x 15″",
        "na_11x12_11x12in": "11 x 12″",
        "na_11x15_11x15in": "11 x 15″",
        "na_12x19_12x19in": "12 x 19″",
        "na_5x7_5x7in": "5 x 7″",
        "na_6x9_6x9in": "6 x 9″",
        "na_7x9_7x9in": "7 x 9″",
        "na_9x11_9x11in": "9 x 11″",
        "na_a2_4.375x5.75in": "Envelope A2",
        "na_arch-a_9x12in": "9 x 12″",
        "na_arch-b_12x18in": "12 x 18″",
        "na_arch-c_18x24in": "18 x 24″",
        "na_arch-d_24x36in": "24 x 36″",
        "na_arch-e2_26x38in": "26 x 38″",
        "na_arch-e3_27x39in": "27 x 39″",
        "na_arch-e_36x48in": "36 x 48″",
        "na_b-plus_12x19.17in": "12 x 19.17″",
        "na_c5_6.5x9.5in": "Envelope C5",
        "na_c_17x22in": "17 x 22″",
        "na_d_22x34in": "22 x 34″",
        "na_e_34x44in": "34 x 44″",
        "na_edp_11x14in": "11 x 14″",
        "na_eur-edp_12x14in": "12 x 14″",
        "na_executive_7.25x10.5in": "Executive",
        "na_f_44x68in": "44 x 68″",
        "na_fanfold-eur_8.5x12in": "European Fanfold",
        "na_fanfold-us_11x14.875in": "US Fanfold",
        "na_foolscap_8.5x13in": "Foolscap",
        "na_govt-legal_8x13in": "8 x 13″",
        "na_govt-letter_8x10in": "8 x 10″",
        "na_index-3x5_3x5in": "3 x 5″",
        "na_index-4x6-ext_6x8in": "6 x 8″",
        "na_index-4x6_4x6in": "4 x 6″",
        "na_index-5x8_5x8in": "5 x 8″",
        "na_invoice_5.5x8.5in": "Statement",
        "na_ledger_11x17in": "11 x 17″",
        "na_legal-extra_9.5x15in": "US Legal (Extra)",
        "na_legal_8.5x14in": "US Legal",
        "na_letter-extra_9.5x12in": "US Letter (Extra)",
        "na_letter-plus_8.5x12.69in": "US Letter (Plus)",
        "na_letter_8.5x11in": "US Letter",
        "na_monarch_3.875x7.5in": "Envelope Monarch",
        "na_number-10_4.125x9.5in": "Envelope #10",
        "na_number-11_4.5x10.375in": "Envelope #11",
        "na_number-12_4.75x11in": "Envelope #12",
        "na_number-14_5x11.5in": "Envelope #14",
        "na_number-9_3.875x8.875in": "Envelope #9",
        "na_oficio_8.5x13.4in": "8.5 x 13.4″",
        "na_personal_3.625x6.5in": "Envelope Personal",
        "na_quarto_8.5x10.83in": "Quarto",
        "na_super-a_8.94x14in": "8.94 x 14″",
        "na_super-b_13x19in": "13 x 19″",
        "na_wide-format_30x42in": "30 x 42″",
        "oe_12x16_12x16in": "12 x 16″",
        "oe_14x17_14x17in": "14 x 17″",
        "oe_18x22_18x22in": "18 x 22″",
        "oe_a2plus_17x24in": "17 x 24″",
        "oe_business-card_2x3.5in": "2 x 3.5″",
        "oe_photo-10r_10x12in": "10 x 12″",
        "oe_photo-20r_20x24in": "20 x 24″",
        "oe_photo-l_3.5x5in": "3.5 x 5″",
        "oe_photo-s10r_10x15in": "10 x 15″",
        "oe_square-photo_4x4in": "4 x 4″",
        "oe_square-photo_5x5in": "5 x 5″",
        "om_16k_184x260mm": "184 x 260mm",
        "om_16k_195x270mm": "195 x 270mm",
        "om_business-card_55x85mm": "55 x 85mm",
        "om_business-card_55x91mm": "55 x 91mm",
        "om_card_54x86mm": "54 x 86mm",
        "om_dai-pa-kai_275x395mm": "275 x 395mm",
        "om_dsc-photo_89x119mm": "89 x 119mm",
        "om_folio-sp_215x315mm": "Folio",
        "om_folio_210x330mm": "Folio (Special)",
        "om_invite_220x220mm": "Envelope Invitation",
        "om_italian_110x230mm": "Envelope Italian",
        "om_juuro-ku-kai_198x275mm": "198 x 275mm",
        "om_large-photo_200x300": "200 x 300",
        "om_medium-photo_130x180mm": "130 x 180mm",
        "om_pa-kai_267x389mm": "267 x 389mm",
        "om_postfix_114x229mm": "Envelope Postfix",
        "om_small-photo_100x150mm": "100 x 150mm",
        "om_square-photo_89x89mm": "89 x 89mm",
        "om_wide-photo_100x200mm": "100 x 200mm",
        "prc_10_324x458mm": "Envelope Chinese #10",
        "prc_16k_146x215mm": "Chinese 16k",
        "prc_1_102x165mm": "Envelope Chinese #1",
        "prc_2_102x176mm": "Envelope Chinese #2",
        "prc_32k_97x151mm": "Chinese 32k",
        "prc_3_125x176mm": "Envelope Chinese #3",
        "prc_4_110x208mm": "Envelope Chinese #4",
        "prc_5_110x220mm": "Envelope Chinese #5",
        "prc_6_120x320mm": "Envelope Chinese #6",
        "prc_7_160x230mm": "Envelope Chinese #7",
        "prc_8_120x309mm": "Envelope Chinese #8",
        "roc_16k_7.75x10.75in": "ROC 16k",
        "roc_8k_10.75x15.5in": "ROC 8k"}
