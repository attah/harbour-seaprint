
// TODO move to IppPrinter and/or Mimer when i figure out how to handle considerAdditionalFormats there
function supported_formats(printer, ConvertChecker, considerAdditionalFormats)
{
    var formats = printer.attrs["document-format-supported"].value;
    if(considerAdditionalFormats)
    {
        formats=formats+printer.additionalDocumentFormats;
    }

    var raster = (has(formats, Mimer.PWG) || has(formats, Mimer.URF));

    var mimetypes = [];
    var pdf = false;
    var postscript = false;
    var office = false;
    var images = false;
    var plaintext = false;

    if(has(formats, Mimer.PDF) ||
       (ConvertChecker.pdf && ( has(formats, Mimer.Postscript) || raster )))
    {
        pdf = true;
        mimetypes.push(Mimer.PDF);
        plaintext = true;
        mimetypes.push(Mimer.Plaintext);
    }
    if(has(formats, Mimer.Postscript))
    {
        postscript = true;
        mimetypes.push(Mimer.Postscript);
    }

    if((ConvertChecker.pdf && ConvertChecker.calligra) &&
            ( has(formats, Mimer.PDF) || has(formats, Mimer.Postscript) || raster ))
    {
        office = true;
        mimetypes = mimetypes.concat(Mimer.OfficeFormats);
    }

    if (raster || has(formats, Mimer.JPEG) || has(formats, Mimer.PNG) ||
            has(formats, Mimer.PDF) || (ConvertChecker.pdf && has(formats, Mimer.Postscript)))
    {
        images = true;
        mimetypes.push(Mimer.JPEG);
        mimetypes.push(Mimer.PNG);
        mimetypes.push(Mimer.TIFF);
        mimetypes.push(Mimer.GIF);
    }

    return {pdf: pdf, postscript: postscript, plaintext: plaintext, office: office, images: images, mimetypes: mimetypes};
}

function has(arrayish, what)
{
    return arrayish.indexOf(what) != -1;
}

function ippName(name, value)
{
    if(value==undefined)
    {
        return ""
    }

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
    case "printer-state":
        switch(value) {
        case 3:
            return qsTr("Idle");
        case 4:
            return qsTr("Processing");
        case 5:
            return qsTr("Stopped");
        default:
            return qsTr("Unknown State ")+value
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
    case "printer-state-reasons":
        if(printerStateReasons.hasOwnProperty(value))
        {
            return printerStateReasons[value];
        }
        else if (endsWith("-report", value))
        {
            return ippName(name, value.substr(0, value.length-"-report".length))+" (Report)"
        }
        else if (endsWith("-warning", value))
        {
            return ippName(name, value.substr(0, value.length-"-warning".length))+" (Warning)"
        }
        else if (endsWith("-error", value))
        {
            return ippName(name, value.substr(0, value.length-"-error".length))+" (Error)"
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
    case "multiple-document-handling":
        switch(value) {
        case "single-document":
            return qsTr("single document");
        case "separate-documents-uncollated-copies":
            return qsTr("uncollated");
        case "separate-documents-collated-copies":
            return qsTr("collated");
        case "single-document-new-sheet":
            return qsTr("single document (new sheet)");
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
        case Mimer.OctetStream:
            return qsTr("auto-sense");
        case Mimer.PDF:
            return qsTr("PDF");
        case Mimer.Postscript:
            return qsTr("Postscript");
        case Mimer.PWG:
            return qsTr("PWG-raster");
        case Mimer.URF:
            return qsTr("URF-raster");
        case Mimer.PNG:
            return qsTr("PNG");
        case Mimer.JPEG:
            return qsTr("JPEG");
        case Mimer.GIF:
            return qsTr("GIF");
        default:
            return value;
        }
    case "media-source":
        if(value.indexOf("tray-")==0)
        {
            return qsTr("tray")+" "+value.split("-")[1];
        }
        else if(value.indexOf("roll-")==0)
        {
            return qsTr("roll")+" "+value.split("-")[1];
        }
        else
        {
            switch(value) {
            case "alternate":
                return qsTr("alternate");
            case "alternate-roll":
                return qsTr("alternate-roll");
            case "auto":
                return qsTr("automatic");
            case "bottom":
                return qsTr("bottom");
            case "by-pass-tray":
                return qsTr("by-pass tray");
            case "center":
                return qsTr("center");
            case "disc":
                return qsTr("disc");
            case "envelope":
                return qsTr("envelope");
            case "hagaki":
                return qsTr("hagaki");
            case "large-capacity":
                return qsTr("large capacity");
            case "left":
                return qsTr("left");
            case "main":
                return qsTr("main");
            case "main-roll":
                return qsTr("main roll");
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
    case "output-bin":
        if(value.indexOf("tray-")==0)
        {
            return qsTr("tray")+" "+value.split("-")[1];
        }
        else if(value.indexOf("stacker-")==0)
        {
            return qsTr("stacker")+" "+value.split("-")[1];
        }
        else if(value.indexOf("mailbox-")==0)
        {
            return qsTr("mailbox")+" "+value.split("-")[1];
        }
        else
        {
            switch(value) {
            case "auto":
                return qsTr("automatic");
            case "top":
                return qsTr("top");
            case "middle":
                return qsTr("middle");
            case "bottom":
                return qsTr("bottom");
            case "side":
                return qsTr("side");
            case "left":
                return qsTr("left");
            case "right":
                return qsTr("right");
            case "center":
                return qsTr("center");
            case "rear":
                return qsTr("rear");
            case "face-up":
                return qsTr("face-up");
            case "face-down":
                return qsTr("face-down");
            case "large-capacity":
                return qsTr("large capacity");
            case "my-mailbox":
                return qsTr("my mailbox");
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
    var targets = [Mimer.OctetStream, Mimer.PDF, Mimer.Postscript, Mimer.PWG, Mimer.URF];
    return has(targets, type)
}

function canTransferPdfAs(type)
{
    var targets = [Mimer.OctetStream, Mimer.PDF];
    return has(targets, type)
}

function canTransferPostscriptAs(type)
{
    var targets = [Mimer.OctetStream, Mimer.Postscript];
    return has(targets, type)
}

function canConvertOfficeDocumentTo(type)
{
    var targets = [Mimer.OctetStream, Mimer.PDF, Mimer.Postscript, Mimer.PWG, Mimer.URF];
    return has(targets, type)
}

function canConvertImageTo(type)
{
    var targets = [Mimer.OctetStream, Mimer.JPEG, Mimer.PNG, Mimer.PWG, Mimer.URF, Mimer.PDF, Mimer.Postscript];
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
        if(mimeType == Mimer.PDF)
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
        else if(mimeType == Mimer.Plaintext)
        {
            // We convert plaintext to PDF internally
            if(ConvertChecker.pdf)
            {
                return choices.filter(canConvertPdfTo)
            }
            else
            {
                return choices.filter(canTransferPdfAs)
            }
        }
        else if(mimeType == Mimer.Postscript)
        {
            return choices.filter(canTransferPostscriptAs)
        }
        else if(Mimer.isOffice(mimeType))
        {
            return choices.filter(canConvertOfficeDocumentTo)
        }
        else if(Mimer.isImage(mimeType))
        {
            return choices.filter(canConvertImageTo);
        }
        else
        {
            return [Mimer.OctetStream];
        }
    case "printer-resolution":
        return choices.filter(unitsIsDpi);
    case "multiple-document-handling" :
        // Only collation settings valid, multiple documents not used
        return choices.filter(function(elem) {return elem.indexOf("collated") != -1});
    default:
        return choices;
    }
}

function selectIcon(icons)
{
    for(var i=0; i < icons.length; i++)
    {
        if(endsWith("M.png", icons[i]) || endsWith("128x128.png", icons[i]))
        {
            return icons[i];
        }
    }
    return icons[0];
}

function isWaringState(printer)
{
    if(printer.attrs["printer-state"].value > 4)
    {
        return true;
    }

    if(printer.attrs.hasOwnProperty("printer-state-reasons"))
    {
        for(var i in printer.attrs["printer-state-reasons"].value)
        {
            var value = printer.attrs["printer-state-reasons"].value[i];
            if(value != "none" && !(endsWith("-report", value)))
            {
                return true;
            }
        }
    }
    return false;
}

function unknownForEmptyString(s)
{
    if(s === "")
    {
        return qsTr("Unknown");
    }
    else
    {
        return s;
    }
}

function existsAndNotEmpty(attrName, printer)
{
    if(printer.attrs.hasOwnProperty(attrName))
    {
        return printer.attrs[attrName].value != ""
    }
    else
    {
        return false
    }
}


function basename(filewithpath)
{
    return filewithpath.substring(filewithpath.lastIndexOf("/")+1);
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

var printerStateReasons =
       {"alert-removal-of-binary-change-entry": "Old Alerts Have Been Removed",
        "bander-added": "Bander Added",
        "bander-almost-empty": "Bander Almost Empty",
        "bander-almost-full": "Bander Almost Full",
        "bander-at-limit": "Bander At Limit",
        "bander-closed": "Bander Closed",
        "bander-configuration-change": "Bander Configuration Change",
        "bander-cover-closed": "Bander Cover Closed",
        "bander-cover-open": "Bander Cover Open",
        "bander-empty": "Bander Empty",
        "bander-full": "Bander Full",
        "bander-interlock-closed": "Bander Interlock Closed",
        "bander-interlock-open": "Bander Interlock Open",
        "bander-jam": "Bander Jam",
        "bander-life-almost-over": "Bander Life Almost Over",
        "bander-life-over": "Bander Life Over",
        "bander-memory-exhausted": "Bander Memory Exhausted",
        "bander-missing": "Bander Missing",
        "bander-motor-failure": "Bander Motor Failure",
        "bander-near-limit": "Bander Near Limit",
        "bander-offline": "Bander Offline",
        "bander-opened": "Bander Opened",
        "bander-over-temperature": "Bander Over Temperature",
        "bander-power-saver": "Bander Power Saver",
        "bander-recoverable-failure": "Bander Recoverable Failure",
        "bander-recoverable-storage": "Bander Recoverable Storage",
        "bander-removed": "Bander Removed",
        "bander-resource-added": "Bander Resource Added",
        "bander-resource-removed": "Bander Resource Removed",
        "bander-thermistor-failure": "Bander Thermistor Failure",
        "bander-timing-failure": "Bander Timing Failure",
        "bander-turned-off": "Bander Turned Off",
        "bander-turned-on": "Bander Turned On",
        "bander-under-temperature": "Bander Under Temperature",
        "bander-unrecoverable-failure": "Bander Unrecoverable Failure",
        "bander-unrecoverable-storage-error": "Bander Unrecoverable Storage Error",
        "bander-warming-up": "Bander Warming Up",
        "binder-added": "Binder Added",
        "binder-almost-empty": "Binder Almost Empty",
        "binder-almost-full": "Binder Almost Full",
        "binder-at-limit": "Binder At Limit",
        "binder-closed": "Binder Closed",
        "binder-configuration-change": "Binder Configuration Change",
        "binder-cover-closed": "Binder Cover Closed",
        "binder-cover-open": "Binder Cover Open",
        "binder-empty": "Binder Empty",
        "binder-full": "Binder Full",
        "binder-interlock-closed": "Binder Interlock Closed",
        "binder-interlock-open": "Binder Interlock Open",
        "binder-jam": "Binder Jam",
        "binder-life-almost-over": "Binder Life Almost Over",
        "binder-life-over": "Binder Life Over",
        "binder-memory-exhausted": "Binder Memory Exhausted",
        "binder-missing": "Binder Missing",
        "binder-motor-failure": "Binder Motor Failure",
        "binder-near-limit": "Binder Near Limit",
        "binder-offline": "Binder Offline",
        "binder-opened": "Binder Opened",
        "binder-over-temperature": "Binder Over Temperature",
        "binder-power-saver": "Binder Power Saver",
        "binder-recoverable-failure": "Binder Recoverable Failure",
        "binder-recoverable-storage": "Binder Recoverable Storage",
        "binder-removed": "Binder Removed",
        "binder-resource-added": "Binder Resource Added",
        "binder-resource-removed": "Binder Resource Removed",
        "binder-thermistor-failure": "Binder Thermistor Failure",
        "binder-timing-failure": "Binder Timing Failure",
        "binder-turned-off": "Binder Turned Off",
        "binder-turned-on": "Binder Turned On",
        "binder-under-temperature": "Binder Under Temperature",
        "binder-unrecoverable-failure": "Binder Unrecoverable Failure",
        "binder-unrecoverable-storage-error": "Binder Unrecoverable Storage Error",
        "binder-warming-up": "Binder Warming Up",
        "camera-failure": "Camera Failure",
        "chamber-cooling": "Chamber Cooling",
        "chamber-failure": "Chamber Failure",
        "chamber-heating": "Chamber Heating",
        "chamber-temperature-high": "Chamber Temperature High",
        "chamber-temperature-low": "Chamber Temperature Low",
        "cleaner-life-almost-over": "Cleaner Life Almost Over",
        "cleaner-life-over": "Cleaner Life Over",
        "configuration-change": "Configuration Change",
        "connecting-to-device": "Connecting To Device",
        "cover-open": "Cover Open",
        "deactivated": "Deactivated",
        "developer-empty": "Developer Empty",
        "developer-low": "Developer Low",
        "die-cutter-added": "Die Cutter Added",
        "die-cutter-almost-empty": "Die Cutter Almost Empty",
        "die-cutter-almost-full": "Die Cutter Almost Full",
        "die-cutter-at-limit": "Die Cutter At Limit",
        "die-cutter-closed": "Die Cutter Closed",
        "die-cutter-configuration-change": "Die Cutter Configuration Change",
        "die-cutter-cover-closed": "Die Cutter Cover Closed",
        "die-cutter-cover-open": "Die Cutter Cover Open",
        "die-cutter-empty": "Die Cutter Empty",
        "die-cutter-full": "Die Cutter Full",
        "die-cutter-interlock-closed": "Die Cutter Interlock Closed",
        "die-cutter-interlock-open": "Die Cutter Interlock Open",
        "die-cutter-jam": "Die Cutter Jam",
        "die-cutter-life-almost-over": "Die Cutter Life Almost Over",
        "die-cutter-life-over": "Die Cutter Life Over",
        "die-cutter-memory-exhausted": "Die Cutter Memory Exhausted",
        "die-cutter-missing": "Die Cutter Missing",
        "die-cutter-motor-failure": "Die Cutter Motor Failure",
        "die-cutter-near-limit": "Die Cutter Near Limit",
        "die-cutter-offline": "Die Cutter Offline",
        "die-cutter-opened": "Die Cutter Opened",
        "die-cutter-over-temperature": "Die Cutter Over Temperature",
        "die-cutter-power-saver": "Die Cutter Power Saver",
        "die-cutter-recoverable-failure": "Die Cutter Recoverable Failure",
        "die-cutter-recoverable-storage": "Die Cutter Recoverable Storage",
        "die-cutter-removed": "Die Cutter Removed",
        "die-cutter-resource-added": "Die Cutter Resource Added",
        "die-cutter-resource-removed": "Die Cutter Resource Removed",
        "die-cutter-thermistor-failure": "Die Cutter Thermistor Failure",
        "die-cutter-timing-failure": "Die Cutter Timing Failure",
        "die-cutter-turned-off": "Die Cutter Turned Off",
        "die-cutter-turned-on": "Die Cutter Turned On",
        "die-cutter-under-temperature": "Die Cutter Under Temperature",
        "die-cutter-unrecoverable-failure": "Die Cutter Unrecoverable Failure",
        "die-cutter-unrecoverable-storage-error": "Die Cutter Unrecoverable Storage Error",
        "die-cutter-warming-up": "Die Cutter Warming Up",
        "door-open": "Door Open",
        "extruder-cooling": "Extruder Cooling",
        "extruder-failure": "Extruder Failure",
        "extruder-heating": "Extruder Heating",
        "extruder-jam": "Extruder Jam",
        "extruder-temperature-high": "Extruder Temperature High",
        "extruder-temperature-low": "Extruder Temperature Low",
        "fan-failure": "Fan Failure",
        "fax-modem-life-almost-over": "Fax Modem Life Almost Over",
        "fax-modem-life-over": "Fax Modem Life Over",
        "fax-modem-missing": "Fax Modem Missing",
        "fax-modem-turned-off": "Fax Modem Turned Off",
        "fax-modem-turned-on": "Fax Modem Turned On",
        "folder-added": "Folder Added",
        "folder-almost-empty": "Folder Almost Empty",
        "folder-almost-full": "Folder Almost Full",
        "folder-at-limit": "Folder At Limit",
        "folder-closed": "Folder Closed",
        "folder-configuration-change": "Folder Configuration Change",
        "folder-cover-closed": "Folder Cover Closed",
        "folder-cover-open": "Folder Cover Open",
        "folder-empty": "Folder Empty",
        "folder-full": "Folder Full",
        "folder-interlock-closed": "Folder Interlock Closed",
        "folder-interlock-open": "Folder Interlock Open",
        "folder-jam": "Folder Jam",
        "folder-life-almost-over": "Folder Life Almost Over",
        "folder-life-over": "Folder Life Over",
        "folder-memory-exhausted": "Folder Memory Exhausted",
        "folder-missing": "Folder Missing",
        "folder-motor-failure": "Folder Motor Failure",
        "folder-near-limit": "Folder Near Limit",
        "folder-offline": "Folder Offline",
        "folder-opened": "Folder Opened",
        "folder-over-temperature": "Folder Over Temperature",
        "folder-power-saver": "Folder Power Saver",
        "folder-recoverable-failure": "Folder Recoverable Failure",
        "folder-recoverable-storage": "Folder Recoverable Storage",
        "folder-removed": "Folder Removed",
        "folder-resource-added": "Folder Resource Added",
        "folder-resource-removed": "Folder Resource Removed",
        "folder-thermistor-failure": "Folder Thermistor Failure",
        "folder-timing-failure": "Folder Timing Failure",
        "folder-turned-off": "Folder Turned Off",
        "folder-turned-on": "Folder Turned On",
        "folder-under-temperature": "Folder Under Temperature",
        "folder-unrecoverable-failure": "Folder Unrecoverable Failure",
        "folder-unrecoverable-storage-error": "Folder Unrecoverable Storage Error",
        "folder-warming-up": "Folder Warming Up",
        "fuser-over-temp": "Fuser temperature high",
        "fuser-under-temp": "Fuser temperature low",
        "hold-new-jobs": "Hold New Jobs",
        "identify-printer-requested": "Identify Printer",
        "imprinter-added": "Imprinter Added",
        "imprinter-almost-empty": "Imprinter Almost Empty",
        "imprinter-almost-full": "Imprinter Almost Full",
        "imprinter-at-limit": "Imprinter At Limit",
        "imprinter-closed": "Imprinter Closed",
        "imprinter-configuration-change": "Imprinter Configuration Change",
        "imprinter-cover-closed": "Imprinter Cover Closed",
        "imprinter-cover-open": "Imprinter Cover Open",
        "imprinter-empty": "Imprinter Empty",
        "imprinter-full": "Imprinter Full",
        "imprinter-interlock-closed": "Imprinter Interlock Closed",
        "imprinter-interlock-open": "Imprinter Interlock Open",
        "imprinter-jam": "Imprinter Jam",
        "imprinter-life-almost-over": "Imprinter Life Almost Over",
        "imprinter-life-over": "Imprinter Life Over",
        "imprinter-memory-exhausted": "Imprinter Memory Exhausted",
        "imprinter-missing": "Imprinter Missing",
        "imprinter-motor-failure": "Imprinter Motor Failure",
        "imprinter-near-limit": "Imprinter Near Limit",
        "imprinter-offline": "Imprinter Offline",
        "imprinter-opened": "Imprinter Opened",
        "imprinter-over-temperature": "Imprinter Over Temperature",
        "imprinter-power-saver": "Imprinter Power Saver",
        "imprinter-recoverable-failure": "Imprinter Recoverable Failure",
        "imprinter-recoverable-storage": "Imprinter Recoverable Storage",
        "imprinter-removed": "Imprinter Removed",
        "imprinter-resource-added": "Imprinter Resource Added",
        "imprinter-resource-removed": "Imprinter Resource Removed",
        "imprinter-thermistor-failure": "Imprinter Thermistor Failure",
        "imprinter-timing-failure": "Imprinter Timing Failure",
        "imprinter-turned-off": "Imprinter Turned Off",
        "imprinter-turned-on": "Imprinter Turned On",
        "imprinter-under-temperature": "Imprinter Under Temperature",
        "imprinter-unrecoverable-failure": "Imprinter Unrecoverable Failure",
        "imprinter-unrecoverable-storage-error": "Imprinter Unrecoverable Storage Error",
        "imprinter-warming-up": "Imprinter Warming Up",
        "input-cannot-feed-size-selected": "Input Cannot Feed Size Selected",
        "input-manual-input-request": "Input Manual Input Request",
        "input-media-color-change": "Input Media Color Change",
        "input-media-form-parts-change": "Input Media Form Parts Change",
        "input-media-size-change": "Input Media Size Change",
        "input-media-tray-failure": "Input Media Tray Failure",
        "input-media-tray-feed-error": "Input Media Tray Feed Error",
        "input-media-tray-jam": "Input Media Tray Jam",
        "input-media-type-change": "Input Media Type Change",
        "input-media-weight-change": "Input Media Weight Change",
        "input-pick-roller-failure": "Input Pick Roller Failure",
        "input-pick-roller-life-over": "Input Pick Roller Life Over",
        "input-pick-roller-life-warn": "Input Pick Roller Life Warn",
        "input-pick-roller-missing": "Input Pick Roller Missing",
        "input-tray-elevation-failure": "Input Tray Elevation Failure",
        "input-tray-missing": "Paper tray is missing",
        "input-tray-position-failure": "Input Tray Position Failure",
        "inserter-added": "Inserter Added",
        "inserter-almost-empty": "Inserter Almost Empty",
        "inserter-almost-full": "Inserter Almost Full",
        "inserter-at-limit": "Inserter At Limit",
        "inserter-closed": "Inserter Closed",
        "inserter-configuration-change": "Inserter Configuration Change",
        "inserter-cover-closed": "Inserter Cover Closed",
        "inserter-cover-open": "Inserter Cover Open",
        "inserter-empty": "Inserter Empty",
        "inserter-full": "Inserter Full",
        "inserter-interlock-closed": "Inserter Interlock Closed",
        "inserter-interlock-open": "Inserter Interlock Open",
        "inserter-jam": "Inserter Jam",
        "inserter-life-almost-over": "Inserter Life Almost Over",
        "inserter-life-over": "Inserter Life Over",
        "inserter-memory-exhausted": "Inserter Memory Exhausted",
        "inserter-missing": "Inserter Missing",
        "inserter-motor-failure": "Inserter Motor Failure",
        "inserter-near-limit": "Inserter Near Limit",
        "inserter-offline": "Inserter Offline",
        "inserter-opened": "Inserter Opened",
        "inserter-over-temperature": "Inserter Over Temperature",
        "inserter-power-saver": "Inserter Power Saver",
        "inserter-recoverable-failure": "Inserter Recoverable Failure",
        "inserter-recoverable-storage": "Inserter Recoverable Storage",
        "inserter-removed": "Inserter Removed",
        "inserter-resource-added": "Inserter Resource Added",
        "inserter-resource-removed": "Inserter Resource Removed",
        "inserter-thermistor-failure": "Inserter Thermistor Failure",
        "inserter-timing-failure": "Inserter Timing Failure",
        "inserter-turned-off": "Inserter Turned Off",
        "inserter-turned-on": "Inserter Turned On",
        "inserter-under-temperature": "Inserter Under Temperature",
        "inserter-unrecoverable-failure": "Inserter Unrecoverable Failure",
        "inserter-unrecoverable-storage-error": "Inserter Unrecoverable Storage Error",
        "inserter-warming-up": "Inserter Warming Up",
        "interlock-closed": "Interlock Closed",
        "interlock-open": "Interlock Open",
        "interpreter-cartridge-added": "Interpreter Cartridge Added",
        "interpreter-cartridge-deleted": "Interpreter Cartridge Removed",
        "interpreter-complex-page-encountered": "Interpreter Complex Page Encountered",
        "interpreter-memory-decrease": "Interpreter Memory Decrease",
        "interpreter-memory-increase": "Interpreter Memory Increase",
        "interpreter-resource-added": "Interpreter Resource Added",
        "interpreter-resource-deleted": "Interpreter Resource Deleted",
        "interpreter-resource-unavailable": "Printer resource unavailable",
        "lamp-at-eol": "Lamp At End of Life",
        "lamp-failure": "Lamp Failure",
        "lamp-near-eol": "Lamp Near End of Life",
        "laser-at-eol": "Laser At End of Life",
        "laser-failure": "Laser Failure",
        "laser-near-eol": "Laser Near End of Life",
        "make-envelope-added": "Envelope Maker Added",
        "make-envelope-almost-empty": "Envelope Maker Almost Empty",
        "make-envelope-almost-full": "Envelope Maker Almost Full",
        "make-envelope-at-limit": "Envelope Maker At Limit",
        "make-envelope-closed": "Envelope Maker Closed",
        "make-envelope-configuration-change": "Envelope Maker Configuration Change",
        "make-envelope-cover-closed": "Envelope Maker Cover Closed",
        "make-envelope-cover-open": "Envelope Maker Cover Open",
        "make-envelope-empty": "Envelope Maker Empty",
        "make-envelope-full": "Envelope Maker Full",
        "make-envelope-interlock-closed": "Envelope Maker Interlock Closed",
        "make-envelope-interlock-open": "Envelope Maker Interlock Open",
        "make-envelope-jam": "Envelope Maker Jam",
        "make-envelope-life-almost-over": "Envelope Maker Life Almost Over",
        "make-envelope-life-over": "Envelope Maker Life Over",
        "make-envelope-memory-exhausted": "Envelope Maker Memory Exhausted",
        "make-envelope-missing": "Envelope Maker Missing",
        "make-envelope-motor-failure": "Envelope Maker Motor Failure",
        "make-envelope-near-limit": "Envelope Maker Near Limit",
        "make-envelope-offline": "Envelope Maker Offline",
        "make-envelope-opened": "Envelope Maker Opened",
        "make-envelope-over-temperature": "Envelope Maker Over Temperature",
        "make-envelope-power-saver": "Envelope Maker Power Saver",
        "make-envelope-recoverable-failure": "Envelope Maker Recoverable Failure",
        "make-envelope-recoverable-storage": "Envelope Maker Recoverable Storage",
        "make-envelope-removed": "Envelope Maker Removed",
        "make-envelope-resource-added": "Envelope Maker Resource Added",
        "make-envelope-resource-removed": "Envelope Maker Resource Removed",
        "make-envelope-thermistor-failure": "Envelope Maker Thermistor Failure",
        "make-envelope-timing-failure": "Envelope Maker Timing Failure",
        "make-envelope-turned-off": "Envelope Maker Turned Off",
        "make-envelope-turned-on": "Envelope Maker Turned On",
        "make-envelope-under-temperature": "Envelope Maker Under Temperature",
        "make-envelope-unrecoverable-failure": "Envelope Maker Unrecoverable Failure",
        "make-envelope-unrecoverable-storage-error": "Envelope Maker Unrecoverable Storage Error",
        "make-envelope-warming-up": "Envelope Maker Warming Up",
        "marker-adjusting-print-quality": "Marker Adjusting Print Quality",
        "marker-cleaner-missing": "Marker Cleaner Missing",
        "marker-developer-almost-empty": "Marker Developer Almost Empty",
        "marker-developer-empty": "Marker Developer Empty",
        "marker-developer-missing": "Marker Developer Missing",
        "marker-fuser-missing": "Marker Fuser Missing",
        "marker-fuser-thermistor-failure": "Marker Fuser Thermistor Failure",
        "marker-fuser-timing-failure": "Marker Fuser Timing Failure",
        "marker-ink-almost-empty": "Marker Ink Almost Empty",
        "marker-ink-empty": "Marker Ink Empty",
        "marker-ink-missing": "Marker Ink Missing",
        "marker-opc-missing": "Marker Opc Missing",
        "marker-print-ribbon-almost-empty": "Marker Print Ribbon Almost Empty",
        "marker-print-ribbon-empty": "Marker Print Ribbon Empty",
        "marker-print-ribbon-missing": "Marker Print Ribbon Missing",
        "marker-supply-almost-empty": "Marker Supply Almost Empty",
        "marker-supply-empty": "Ink/toner empty",
        "marker-supply-low": "Ink/toner low",
        "marker-supply-missing": "Marker Supply Missing",
        "marker-toner-cartridge-missing": "Marker Toner Cartridge Missing",
        "marker-toner-missing": "Marker Toner Missing",
        "marker-waste-almost-full": "Ink/toner waste bin almost full",
        "marker-waste-full": "Ink/toner waste bin full",
        "marker-waste-ink-receptacle-almost-full": "Marker Waste Ink Receptacle Almost Full",
        "marker-waste-ink-receptacle-full": "Marker Waste Ink Receptacle Full",
        "marker-waste-ink-receptacle-missing": "Marker Waste Ink Receptacle Missing",
        "marker-waste-missing": "Marker Waste Missing",
        "marker-waste-toner-receptacle-almost-full": "Marker Waste Toner Receptacle Almost Full",
        "marker-waste-toner-receptacle-full": "Marker Waste Toner Receptacle Full",
        "marker-waste-toner-receptacle-missing": "Marker Waste Toner Receptacle Missing",
        "material-empty": "Material Empty",
        "material-low": "Material Low",
        "material-needed": "Material Needed",
        "media-drying": "Media Drying",
        "media-empty": "Paper tray is empty",
        "media-jam": "Paper jam",
        "media-low": "Paper tray is almost empty",
        "media-needed": "Load paper",
        "media-path-cannot-duplex-media-selected": "Media Path Cannot Do 2-Sided Printing",
        "media-path-failure": "Media Path Failure",
        "media-path-input-empty": "Media Path Input Empty",
        "media-path-input-feed-error": "Media Path Input Feed Error",
        "media-path-input-jam": "Media Path Input Jam",
        "media-path-input-request": "Media Path Input Request",
        "media-path-jam": "Media Path Jam",
        "media-path-media-tray-almost-full": "Media Path Media Tray Almost Full",
        "media-path-media-tray-full": "Media Path Media Tray Full",
        "media-path-media-tray-missing": "Media Path Media Tray Missing",
        "media-path-output-feed-error": "Media Path Output Feed Error",
        "media-path-output-full": "Media Path Output Full",
        "media-path-output-jam": "Media Path Output Jam",
        "media-path-pick-roller-failure": "Media Path Pick Roller Failure",
        "media-path-pick-roller-life-over": "Media Path Pick Roller Life Over",
        "media-path-pick-roller-life-warn": "Media Path Pick Roller Life Warn",
        "media-path-pick-roller-missing": "Media Path Pick Roller Missing",
        "motor-failure": "Motor Failure",
        "moving-to-paused": "Printer going offline",
        "none": "", //"None",
        "opc-life-over": "Optical Photoconductor Life Over",
        "opc-near-eol": "OPC almost at end-of-life",
        "other": "Check the printer for errors",
        "output-area-almost-full": "Output bin is almost full",
        "output-area-full": "Output bin is full",
        "output-mailbox-select-failure": "Output Mailbox Select Failure",
        "output-media-tray-failure": "Output Media Tray Failure",
        "output-media-tray-feed-error": "Output Media Tray Feed Error",
        "output-media-tray-jam": "Output Media Tray Jam",
        "output-tray-missing": "Output tray is missing",
        "paused": "Paused",
        "perforater-added": "Perforater Added",
        "perforater-almost-empty": "Perforater Almost Empty",
        "perforater-almost-full": "Perforater Almost Full",
        "perforater-at-limit": "Perforater At Limit",
        "perforater-closed": "Perforater Closed",
        "perforater-configuration-change": "Perforater Configuration Change",
        "perforater-cover-closed": "Perforater Cover Closed",
        "perforater-cover-open": "Perforater Cover Open",
        "perforater-empty": "Perforater Empty",
        "perforater-full": "Perforater Full",
        "perforater-interlock-closed": "Perforater Interlock Closed",
        "perforater-interlock-open": "Perforater Interlock Open",
        "perforater-jam": "Perforater Jam",
        "perforater-life-almost-over": "Perforater Life Almost Over",
        "perforater-life-over": "Perforater Life Over",
        "perforater-memory-exhausted": "Perforater Memory Exhausted",
        "perforater-missing": "Perforater Missing",
        "perforater-motor-failure": "Perforater Motor Failure",
        "perforater-near-limit": "Perforater Near Limit",
        "perforater-offline": "Perforater Offline",
        "perforater-opened": "Perforater Opened",
        "perforater-over-temperature": "Perforater Over Temperature",
        "perforater-power-saver": "Perforater Power Saver",
        "perforater-recoverable-failure": "Perforater Recoverable Failure",
        "perforater-recoverable-storage": "Perforater Recoverable Storage",
        "perforater-removed": "Perforater Removed",
        "perforater-resource-added": "Perforater Resource Added",
        "perforater-resource-removed": "Perforater Resource Removed",
        "perforater-thermistor-failure": "Perforater Thermistor Failure",
        "perforater-timing-failure": "Perforater Timing Failure",
        "perforater-turned-off": "Perforater Turned Off",
        "perforater-turned-on": "Perforater Turned On",
        "perforater-under-temperature": "Perforater Under Temperature",
        "perforater-unrecoverable-failure": "Perforater Unrecoverable Failure",
        "perforater-unrecoverable-storage-error": "Perforater Unrecoverable Storage Error",
        "perforater-warming-up": "Perforater Warming Up",
        "platform-cooling": "Platform Cooling",
        "platform-failure": "Platform Failure",
        "platform-heating": "Platform Heating",
        "platform-temperature-high": "Platform Temperature High",
        "platform-temperature-low": "Platform Temperature Low",
        "power-down": "Power Down",
        "power-up": "Power Up",
        "printer-manual-reset": "Printer Reset Manually",
        "printer-nms-reset": "Printer Reset Remotely",
        "printer-ready-to-print": "Printer Ready To Print",
        "puncher-added": "Puncher Added",
        "puncher-almost-empty": "Puncher Almost Empty",
        "puncher-almost-full": "Puncher Almost Full",
        "puncher-at-limit": "Puncher At Limit",
        "puncher-closed": "Puncher Closed",
        "puncher-configuration-change": "Puncher Configuration Change",
        "puncher-cover-closed": "Puncher Cover Closed",
        "puncher-cover-open": "Puncher Cover Open",
        "puncher-empty": "Puncher Empty",
        "puncher-full": "Puncher Full",
        "puncher-interlock-closed": "Puncher Interlock Closed",
        "puncher-interlock-open": "Puncher Interlock Open",
        "puncher-jam": "Puncher Jam",
        "puncher-life-almost-over": "Puncher Life Almost Over",
        "puncher-life-over": "Puncher Life Over",
        "puncher-memory-exhausted": "Puncher Memory Exhausted",
        "puncher-missing": "Puncher Missing",
        "puncher-motor-failure": "Puncher Motor Failure",
        "puncher-near-limit": "Puncher Near Limit",
        "puncher-offline": "Puncher Offline",
        "puncher-opened": "Puncher Opened",
        "puncher-over-temperature": "Puncher Over Temperature",
        "puncher-power-saver": "Puncher Power Saver",
        "puncher-recoverable-failure": "Puncher Recoverable Failure",
        "puncher-recoverable-storage": "Puncher Recoverable Storage",
        "puncher-removed": "Puncher Removed",
        "puncher-resource-added": "Puncher Resource Added",
        "puncher-resource-removed": "Puncher Resource Removed",
        "puncher-thermistor-failure": "Puncher Thermistor Failure",
        "puncher-timing-failure": "Puncher Timing Failure",
        "puncher-turned-off": "Puncher Turned Off",
        "puncher-turned-on": "Puncher Turned On",
        "puncher-under-temperature": "Puncher Under Temperature",
        "puncher-unrecoverable-failure": "Puncher Unrecoverable Failure",
        "puncher-unrecoverable-storage-error": "Puncher Unrecoverable Storage Error",
        "puncher-warming-up": "Puncher Warming Up",
        "scan-media-path-failure": "Scan Media Path Failure",
        "scan-media-path-input-feed-error": "Scan Media Path Input Feed Error",
        "scan-media-path-input-jam": "Scan Media Path Input Jam",
        "scan-media-path-input-request": "Scan Media Path Input Request",
        "scan-media-path-jam": "Scan Media Path Jam",
        "scan-media-path-output-feed-error": "Scan Media Path Output Feed Error",
        "scan-media-path-output-full": "Scan Media Path Output Full",
        "scan-media-path-output-jam": "Scan Media Path Output Jam",
        "scan-media-path-pick-roller-failure": "Scan Media Path Pick Roller Failure",
        "scan-media-path-pick-roller-life-over": "Scan Media Path Pick Roller Life Over",
        "scan-media-path-pick-roller-life-warn": "Scan Media Path Pick Roller Life Warn",
        "scan-media-path-pick-roller-missing": "Scan Media Path Pick Roller Missing",
        "scan-media-path-tray-almost-full": "Scan Media Path Tray Almost Full",
        "scan-media-path-tray-full": "Scan Media Path Tray Full",
        "scan-media-path-tray-missing": "Scan Media Path Tray Missing",
        "scanner-light-failure": "Scanner Light Failure",
        "scanner-light-life-almost-over": "Scanner Light Life Almost Over",
        "scanner-light-life-over": "Scanner Light Life Over",
        "scanner-light-missing": "Scanner Light Missing",
        "scanner-sensor-failure": "Scanner Sensor Failure",
        "scanner-sensor-life-almost-over": "Scanner Sensor Life Almost Over",
        "scanner-sensor-life-over": "Scanner Sensor Life Over",
        "scanner-sensor-missing": "Scanner Sensor Missing",
        "separation-cutter-added": "Separation Cutter Added",
        "separation-cutter-almost-empty": "Separation Cutter Almost Empty",
        "separation-cutter-almost-full": "Separation Cutter Almost Full",
        "separation-cutter-at-limit": "Separation Cutter At Limit",
        "separation-cutter-closed": "Separation Cutter Closed",
        "separation-cutter-configuration-change": "Separation Cutter Configuration Change",
        "separation-cutter-cover-closed": "Separation Cutter Cover Closed",
        "separation-cutter-cover-open": "Separation Cutter Cover Open",
        "separation-cutter-empty": "Separation Cutter Empty",
        "separation-cutter-full": "Separation Cutter Full",
        "separation-cutter-interlock-closed": "Separation Cutter Interlock Closed",
        "separation-cutter-interlock-open": "Separation Cutter Interlock Open",
        "separation-cutter-jam": "Separation Cutter Jam",
        "separation-cutter-life-almost-over": "Separation Cutter Life Almost Over",
        "separation-cutter-life-over": "Separation Cutter Life Over",
        "separation-cutter-memory-exhausted": "Separation Cutter Memory Exhausted",
        "separation-cutter-missing": "Separation Cutter Missing",
        "separation-cutter-motor-failure": "Separation Cutter Motor Failure",
        "separation-cutter-near-limit": "Separation Cutter Near Limit",
        "separation-cutter-offline": "Separation Cutter Offline",
        "separation-cutter-opened": "Separation Cutter Opened",
        "separation-cutter-over-temperature": "Separation Cutter Over Temperature",
        "separation-cutter-power-saver": "Separation Cutter Power Saver",
        "separation-cutter-recoverable-failure": "Separation Cutter Recoverable Failure",
        "separation-cutter-recoverable-storage": "Separation Cutter Recoverable Storage",
        "separation-cutter-removed": "Separation Cutter Removed",
        "separation-cutter-resource-added": "Separation Cutter Resource Added",
        "separation-cutter-resource-removed": "Separation Cutter Resource Removed",
        "separation-cutter-thermistor-failure": "Separation Cutter Thermistor Failure",
        "separation-cutter-timing-failure": "Separation Cutter Timing Failure",
        "separation-cutter-turned-off": "Separation Cutter Turned Off",
        "separation-cutter-turned-on": "Separation Cutter Turned On",
        "separation-cutter-under-temperature": "Separation Cutter Under Temperature",
        "separation-cutter-unrecoverable-failure": "Separation Cutter Unrecoverable Failure",
        "separation-cutter-unrecoverable-storage-error": "Separation Cutter Unrecoverable Storage Error",
        "separation-cutter-warming-up": "Separation Cutter Warming Up",
        "sheet-rotator-added": "Sheet Rotator Added",
        "sheet-rotator-almost-empty": "Sheet Rotator Almost Empty",
        "sheet-rotator-almost-full": "Sheet Rotator Almost Full",
        "sheet-rotator-at-limit": "Sheet Rotator At Limit",
        "sheet-rotator-closed": "Sheet Rotator Closed",
        "sheet-rotator-configuration-change": "Sheet Rotator Configuration Change",
        "sheet-rotator-cover-closed": "Sheet Rotator Cover Closed",
        "sheet-rotator-cover-open": "Sheet Rotator Cover Open",
        "sheet-rotator-empty": "Sheet Rotator Empty",
        "sheet-rotator-full": "Sheet Rotator Full",
        "sheet-rotator-interlock-closed": "Sheet Rotator Interlock Closed",
        "sheet-rotator-interlock-open": "Sheet Rotator Interlock Open",
        "sheet-rotator-jam": "Sheet Rotator Jam",
        "sheet-rotator-life-almost-over": "Sheet Rotator Life Almost Over",
        "sheet-rotator-life-over": "Sheet Rotator Life Over",
        "sheet-rotator-memory-exhausted": "Sheet Rotator Memory Exhausted",
        "sheet-rotator-missing": "Sheet Rotator Missing",
        "sheet-rotator-motor-failure": "Sheet Rotator Motor Failure",
        "sheet-rotator-near-limit": "Sheet Rotator Near Limit",
        "sheet-rotator-offline": "Sheet Rotator Offline",
        "sheet-rotator-opened": "Sheet Rotator Opened",
        "sheet-rotator-over-temperature": "Sheet Rotator Over Temperature",
        "sheet-rotator-power-saver": "Sheet Rotator Power Saver",
        "sheet-rotator-recoverable-failure": "Sheet Rotator Recoverable Failure",
        "sheet-rotator-recoverable-storage": "Sheet Rotator Recoverable Storage",
        "sheet-rotator-removed": "Sheet Rotator Removed",
        "sheet-rotator-resource-added": "Sheet Rotator Resource Added",
        "sheet-rotator-resource-removed": "Sheet Rotator Resource Removed",
        "sheet-rotator-thermistor-failure": "Sheet Rotator Thermistor Failure",
        "sheet-rotator-timing-failure": "Sheet Rotator Timing Failure",
        "sheet-rotator-turned-off": "Sheet Rotator Turned Off",
        "sheet-rotator-turned-on": "Sheet Rotator Turned On",
        "sheet-rotator-under-temperature": "Sheet Rotator Under Temperature",
        "sheet-rotator-unrecoverable-failure": "Sheet Rotator Unrecoverable Failure",
        "sheet-rotator-unrecoverable-storage-error": "Sheet Rotator Unrecoverable Storage Error",
        "sheet-rotator-warming-up": "Sheet Rotator Warming Up",
        "shutdown": "Printer offline",
        "slitter-added": "Slitter Added",
        "slitter-almost-empty": "Slitter Almost Empty",
        "slitter-almost-full": "Slitter Almost Full",
        "slitter-at-limit": "Slitter At Limit",
        "slitter-closed": "Slitter Closed",
        "slitter-configuration-change": "Slitter Configuration Change",
        "slitter-cover-closed": "Slitter Cover Closed",
        "slitter-cover-open": "Slitter Cover Open",
        "slitter-empty": "Slitter Empty",
        "slitter-full": "Slitter Full",
        "slitter-interlock-closed": "Slitter Interlock Closed",
        "slitter-interlock-open": "Slitter Interlock Open",
        "slitter-jam": "Slitter Jam",
        "slitter-life-almost-over": "Slitter Life Almost Over",
        "slitter-life-over": "Slitter Life Over",
        "slitter-memory-exhausted": "Slitter Memory Exhausted",
        "slitter-missing": "Slitter Missing",
        "slitter-motor-failure": "Slitter Motor Failure",
        "slitter-near-limit": "Slitter Near Limit",
        "slitter-offline": "Slitter Offline",
        "slitter-opened": "Slitter Opened",
        "slitter-over-temperature": "Slitter Over Temperature",
        "slitter-power-saver": "Slitter Power Saver",
        "slitter-recoverable-failure": "Slitter Recoverable Failure",
        "slitter-recoverable-storage": "Slitter Recoverable Storage",
        "slitter-removed": "Slitter Removed",
        "slitter-resource-added": "Slitter Resource Added",
        "slitter-resource-removed": "Slitter Resource Removed",
        "slitter-thermistor-failure": "Slitter Thermistor Failure",
        "slitter-timing-failure": "Slitter Timing Failure",
        "slitter-turned-off": "Slitter Turned Off",
        "slitter-turned-on": "Slitter Turned On",
        "slitter-under-temperature": "Slitter Under Temperature",
        "slitter-unrecoverable-failure": "Slitter Unrecoverable Failure",
        "slitter-unrecoverable-storage-error": "Slitter Unrecoverable Storage Error",
        "slitter-warming-up": "Slitter Warming Up",
        "spool-area-full": "Spool Area Full",
        "stacker-added": "Stacker Added",
        "stacker-almost-empty": "Stacker Almost Empty",
        "stacker-almost-full": "Stacker Almost Full",
        "stacker-at-limit": "Stacker At Limit",
        "stacker-closed": "Stacker Closed",
        "stacker-configuration-change": "Stacker Configuration Change",
        "stacker-cover-closed": "Stacker Cover Closed",
        "stacker-cover-open": "Stacker Cover Open",
        "stacker-empty": "Stacker Empty",
        "stacker-full": "Stacker Full",
        "stacker-interlock-closed": "Stacker Interlock Closed",
        "stacker-interlock-open": "Stacker Interlock Open",
        "stacker-jam": "Stacker Jam",
        "stacker-life-almost-over": "Stacker Life Almost Over",
        "stacker-life-over": "Stacker Life Over",
        "stacker-memory-exhausted": "Stacker Memory Exhausted",
        "stacker-missing": "Stacker Missing",
        "stacker-motor-failure": "Stacker Motor Failure",
        "stacker-near-limit": "Stacker Near Limit",
        "stacker-offline": "Stacker Offline",
        "stacker-opened": "Stacker Opened",
        "stacker-over-temperature": "Stacker Over Temperature",
        "stacker-power-saver": "Stacker Power Saver",
        "stacker-recoverable-failure": "Stacker Recoverable Failure",
        "stacker-recoverable-storage": "Stacker Recoverable Storage",
        "stacker-removed": "Stacker Removed",
        "stacker-resource-added": "Stacker Resource Added",
        "stacker-resource-removed": "Stacker Resource Removed",
        "stacker-thermistor-failure": "Stacker Thermistor Failure",
        "stacker-timing-failure": "Stacker Timing Failure",
        "stacker-turned-off": "Stacker Turned Off",
        "stacker-turned-on": "Stacker Turned On",
        "stacker-under-temperature": "Stacker Under Temperature",
        "stacker-unrecoverable-failure": "Stacker Unrecoverable Failure",
        "stacker-unrecoverable-storage-error": "Stacker Unrecoverable Storage Error",
        "stacker-warming-up": "Stacker Warming Up",
        "stapler-added": "Stapler Added",
        "stapler-almost-empty": "Stapler Almost Empty",
        "stapler-almost-full": "Stapler Almost Full",
        "stapler-at-limit": "Stapler At Limit",
        "stapler-closed": "Stapler Closed",
        "stapler-configuration-change": "Stapler Configuration Change",
        "stapler-cover-closed": "Stapler Cover Closed",
        "stapler-cover-open": "Stapler Cover Open",
        "stapler-empty": "Stapler Empty",
        "stapler-full": "Stapler Full",
        "stapler-interlock-closed": "Stapler Interlock Closed",
        "stapler-interlock-open": "Stapler Interlock Open",
        "stapler-jam": "Stapler Jam",
        "stapler-life-almost-over": "Stapler Life Almost Over",
        "stapler-life-over": "Stapler Life Over",
        "stapler-memory-exhausted": "Stapler Memory Exhausted",
        "stapler-missing": "Stapler Missing",
        "stapler-motor-failure": "Stapler Motor Failure",
        "stapler-near-limit": "Stapler Near Limit",
        "stapler-offline": "Stapler Offline",
        "stapler-opened": "Stapler Opened",
        "stapler-over-temperature": "Stapler Over Temperature",
        "stapler-power-saver": "Stapler Power Saver",
        "stapler-recoverable-failure": "Stapler Recoverable Failure",
        "stapler-recoverable-storage": "Stapler Recoverable Storage",
        "stapler-removed": "Stapler Removed",
        "stapler-resource-added": "Stapler Resource Added",
        "stapler-resource-removed": "Stapler Resource Removed",
        "stapler-thermistor-failure": "Stapler Thermistor Failure",
        "stapler-timing-failure": "Stapler Timing Failure",
        "stapler-turned-off": "Stapler Turned Off",
        "stapler-turned-on": "Stapler Turned On",
        "stapler-under-temperature": "Stapler Under Temperature",
        "stapler-unrecoverable-failure": "Stapler Unrecoverable Failure",
        "stapler-unrecoverable-storage-error": "Stapler Unrecoverable Storage Error",
        "stapler-warming-up": "Stapler Warming Up",
        "stitcher-added": "Stitcher Added",
        "stitcher-almost-empty": "Stitcher Almost Empty",
        "stitcher-almost-full": "Stitcher Almost Full",
        "stitcher-at-limit": "Stitcher At Limit",
        "stitcher-closed": "Stitcher Closed",
        "stitcher-configuration-change": "Stitcher Configuration Change",
        "stitcher-cover-closed": "Stitcher Cover Closed",
        "stitcher-cover-open": "Stitcher Cover Open",
        "stitcher-empty": "Stitcher Empty",
        "stitcher-full": "Stitcher Full",
        "stitcher-interlock-closed": "Stitcher Interlock Closed",
        "stitcher-interlock-open": "Stitcher Interlock Open",
        "stitcher-jam": "Stitcher Jam",
        "stitcher-life-almost-over": "Stitcher Life Almost Over",
        "stitcher-life-over": "Stitcher Life Over",
        "stitcher-memory-exhausted": "Stitcher Memory Exhausted",
        "stitcher-missing": "Stitcher Missing",
        "stitcher-motor-failure": "Stitcher Motor Failure",
        "stitcher-near-limit": "Stitcher Near Limit",
        "stitcher-offline": "Stitcher Offline",
        "stitcher-opened": "Stitcher Opened",
        "stitcher-over-temperature": "Stitcher Over Temperature",
        "stitcher-power-saver": "Stitcher Power Saver",
        "stitcher-recoverable-failure": "Stitcher Recoverable Failure",
        "stitcher-recoverable-storage": "Stitcher Recoverable Storage",
        "stitcher-removed": "Stitcher Removed",
        "stitcher-resource-added": "Stitcher Resource Added",
        "stitcher-resource-removed": "Stitcher Resource Removed",
        "stitcher-thermistor-failure": "Stitcher Thermistor Failure",
        "stitcher-timing-failure": "Stitcher Timing Failure",
        "stitcher-turned-off": "Stitcher Turned Off",
        "stitcher-turned-on": "Stitcher Turned On",
        "stitcher-under-temperature": "Stitcher Under Temperature",
        "stitcher-unrecoverable-failure": "Stitcher Unrecoverable Failure",
        "stitcher-unrecoverable-storage-error": "Stitcher Unrecoverable Storage Error",
        "stitcher-warming-up": "Stitcher Warming Up",
        "stopped-partly": "Partially stopped",
        "stopping": "Stopping",
        "subunit-added": "Subunit Added",
        "subunit-almost-empty": "Subunit Almost Empty",
        "subunit-almost-full": "Subunit Almost Full",
        "subunit-at-limit": "Subunit At Limit",
        "subunit-closed": "Subunit Closed",
        "subunit-cooling-down": "Subunit Cooling Down",
        "subunit-empty": "Subunit Empty",
        "subunit-full": "Subunit Full",
        "subunit-life-almost-over": "Subunit Life Almost Over",
        "subunit-life-over": "Subunit Life Over",
        "subunit-memory-exhausted": "Subunit Memory Exhausted",
        "subunit-missing": "Subunit Missing",
        "subunit-motor-failure": "Subunit Motor Failure",
        "subunit-near-limit": "Subunit Near Limit",
        "subunit-offline": "Subunit Offline",
        "subunit-opened": "Subunit Opened",
        "subunit-over-temperature": "Subunit Over Temperature",
        "subunit-power-saver": "Subunit Power Saver",
        "subunit-recoverable-failure": "Subunit Recoverable Failure",
        "subunit-recoverable-storage": "Subunit Recoverable Storage",
        "subunit-removed": "Subunit Removed",
        "subunit-resource-added": "Subunit Resource Added",
        "subunit-resource-removed": "Subunit Resource Removed",
        "subunit-thermistor-failure": "Subunit Thermistor Failure",
        "subunit-timing-Failure": "Subunit Timing Failure",
        "subunit-turned-off": "Subunit Turned Off",
        "subunit-turned-on": "Subunit Turned On",
        "subunit-under-temperature": "Subunit Under Temperature",
        "subunit-unrecoverable-failure": "Subunit Unrecoverable Failure",
        "subunit-unrecoverable-storage": "Subunit Unrecoverable Storage",
        "subunit-warming-up": "Subunit Warming Up",
        "timed-out": "Printer stopped responding",
        "toner-empty": "Out of toner",
        "toner-low": "Toner low",
        "trimmer-added": "Trimmer Added",
        "trimmer-almost-empty": "Trimmer Almost Empty",
        "trimmer-almost-full": "Trimmer Almost Full",
        "trimmer-at-limit": "Trimmer At Limit",
        "trimmer-closed": "Trimmer Closed",
        "trimmer-configuration-change": "Trimmer Configuration Change",
        "trimmer-cover-closed": "Trimmer Cover Closed",
        "trimmer-cover-open": "Trimmer Cover Open",
        "trimmer-empty": "Trimmer Empty",
        "trimmer-full": "Trimmer Full",
        "trimmer-interlock-closed": "Trimmer Interlock Closed",
        "trimmer-interlock-open": "Trimmer Interlock Open",
        "trimmer-jam": "Trimmer Jam",
        "trimmer-life-almost-over": "Trimmer Life Almost Over",
        "trimmer-life-over": "Trimmer Life Over",
        "trimmer-memory-exhausted": "Trimmer Memory Exhausted",
        "trimmer-missing": "Trimmer Missing",
        "trimmer-motor-failure": "Trimmer Motor Failure",
        "trimmer-near-limit": "Trimmer Near Limit",
        "trimmer-offline": "Trimmer Offline",
        "trimmer-opened": "Trimmer Opened",
        "trimmer-over-temperature": "Trimmer Over Temperature",
        "trimmer-power-saver": "Trimmer Power Saver",
        "trimmer-recoverable-failure": "Trimmer Recoverable Failure",
        "trimmer-recoverable-storage": "Trimmer Recoverable Storage",
        "trimmer-removed": "Trimmer Removed",
        "trimmer-resource-added": "Trimmer Resource Added",
        "trimmer-resource-removed": "Trimmer Resource Removed",
        "trimmer-thermistor-failure": "Trimmer Thermistor Failure",
        "trimmer-timing-failure": "Trimmer Timing Failure",
        "trimmer-turned-off": "Trimmer Turned Off",
        "trimmer-turned-on": "Trimmer Turned On",
        "trimmer-under-temperature": "Trimmer Under Temperature",
        "trimmer-unrecoverable-failure": "Trimmer Unrecoverable Failure",
        "trimmer-unrecoverable-storage-error": "Trimmer Unrecoverable Storage Error",
        "trimmer-warming-up": "Trimmer Warming Up",
        "unknown": "Unknown",
        "wrapper-added": "Wrapper Added",
        "wrapper-almost-empty": "Wrapper Almost Empty",
        "wrapper-almost-full": "Wrapper Almost Full",
        "wrapper-at-limit": "Wrapper At Limit",
        "wrapper-closed": "Wrapper Closed",
        "wrapper-configuration-change": "Wrapper Configuration Change",
        "wrapper-cover-closed": "Wrapper Cover Closed",
        "wrapper-cover-open": "Wrapper Cover Open",
        "wrapper-empty": "Wrapper Empty",
        "wrapper-full": "Wrapper Full",
        "wrapper-interlock-closed": "Wrapper Interlock Closed",
        "wrapper-interlock-open": "Wrapper Interlock Open",
        "wrapper-jam": "Wrapper Jam",
        "wrapper-life-almost-over": "Wrapper Life Almost Over",
        "wrapper-life-over": "Wrapper Life Over",
        "wrapper-memory-exhausted": "Wrapper Memory Exhausted",
        "wrapper-missing": "Wrapper Missing",
        "wrapper-motor-failure": "Wrapper Motor Failure",
        "wrapper-near-limit": "Wrapper Near Limit",
        "wrapper-offline": "Wrapper Offline",
        "wrapper-opened": "Wrapper Opened",
        "wrapper-over-temperature": "Wrapper Over Temperature",
        "wrapper-power-saver": "Wrapper Power Saver",
        "wrapper-recoverable-failure": "Wrapper Recoverable Failure",
        "wrapper-recoverable-storage": "Wrapper Recoverable Storage",
        "wrapper-removed": "Wrapper Removed",
        "wrapper-resource-added": "Wrapper Resource Added",
        "wrapper-resource-removed": "Wrapper Resource Removed",
        "wrapper-thermistor-failure": "Wrapper Thermistor Failure",
        "wrapper-timing-failure": "Wrapper Timing Failure",
        "wrapper-turned-off": "Wrapper Turned Off",
        "wrapper-turned-on": "Wrapper Turned On",
        "wrapper-under-temperature": "Wrapper Under Temperature",
        "wrapper-unrecoverable-failure": "Wrapper Unrecoverable Failure",
        "wrapper-unrecoverable-storage-error": "Wrapper Unrecoverable Storage Error",
        "wrapper-warming-up": "Wrapper Warming Up"}
