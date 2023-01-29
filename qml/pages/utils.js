.pragma library
.import seaprint.mimer 1.0 as Mimer
.import seaprint.convertchecker 1.0 as ConvertChecker
.import "strings.js" as Strings

// TODO move to IppPrinter?
function supported_formats(printer, considerAdditionalFormats)
{
    var formats = printer.attrs["document-format-supported"].value;
    if(considerAdditionalFormats)
    {
        formats=formats+printer.additionalDocumentFormats;
    }

    var raster = (has(formats, Mimer.Mimer.PWG) || has(formats, Mimer.Mimer.URF));

    var mimetypes = [];
    var pdf = false;
    var postscript = false;
    var office = false;
    var images = false;
    var plaintext = false;

    if(has(formats, Mimer.Mimer.PDF) ||
       (has(formats, Mimer.Mimer.Postscript) || raster ))
    {
        pdf = true;
        mimetypes.push(Mimer.Mimer.PDF);
    }

    if(has(formats, Mimer.Mimer.Postscript))
    {
        postscript = true;
        mimetypes.push(Mimer.Mimer.Postscript);
    }

    if(pdf || has(formats, Mimer.Mimer.Plaintext))
    {
        plaintext = true;
        mimetypes.push(Mimer.Mimer.Plaintext);
    }

    if((ConvertChecker.ConvertChecker.calligra) && pdf)
    {
        office = true;
        mimetypes = mimetypes.concat(Mimer.Mimer.OfficeFormats);
    }

    if(pdf || has(formats, Mimer.Mimer.JPEG) || has(formats, Mimer.Mimer.PNG) || has(formats, Mimer.Mimer.RBMP))
    {
        images = true;
        mimetypes.push(Mimer.Mimer.JPEG);
        mimetypes.push(Mimer.Mimer.PNG);
        mimetypes.push(Mimer.Mimer.TIFF);
        mimetypes.push(Mimer.Mimer.GIF);
    }

    if(pdf)
    {
        mimetypes.push(Mimer.Mimer.SVG);
    }

    return {pdf: pdf, postscript: postscript, plaintext: plaintext, office: office, images: images, mimetypes: mimetypes};
}

function supports_raster(printer)
{
    var formats = printer.attrs["document-format-supported"].value;
    if(considerAdditionalFormats)
    {
        formats=formats+printer.additionalDocumentFormats;
    }
    return (has(formats, Mimer.Mimer.PWG) || has(formats, Mimer.Mimer.URF));
}

function has(arrayish, what)
{
    return arrayish.indexOf(what) != -1;
}

function ippName(name, value, printerStrings)
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
        if(Strings.media.hasOwnProperty(value))
        {
            return Strings.media[value];
        }
        else
        {
            return value;
        }
    case "media-type":
        if(Strings.mediaType.hasOwnProperty(value))
        {
            return Strings.mediaType[value];
        }
        else if(printerStrings != undefined && printerStrings.hasOwnProperty("media-type."+value))
        {
            return printerStrings["media-type."+value];
        }
        else
        {
            return value;
        }
    case "printer-state-reasons":
        if(Strings.printerStateReasons.hasOwnProperty(value))
        {
            return Strings.printerStateReasons[value];
        }
        else if (endsWith("-report", value))
        {
            return ippName(name, value.substr(0, value.length-"-report".length))+" (" + qsTr("Report") + ")"
        }
        else if (endsWith("-warning", value))
        {
            return ippName(name, value.substr(0, value.length-"-warning".length))+" (" + qsTr("Warning") + ")"
        }
        else if (endsWith("-error", value))
        {
            return ippName(name, value.substr(0, value.length-"-error".length))+" (" + qsTr("Error") + ")"
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
    case "print-scaling":
        switch(value) {
        case "auto":
            return qsTr("auto");
        case "auto-fit":
            return qsTr("auto (fit)");
        case "fill":
            return qsTr("fill");
        case "fit":
            return qsTr("fit");
        case "none":
            return qsTr("none");
        default:
            return value;
        }

    case "document-format":
        switch(value) {
        case Mimer.Mimer.OctetStream:
            return qsTr("auto-sense");
        case Mimer.Mimer.PDF:
            return qsTr("PDF");
        case Mimer.Mimer.Postscript:
            return qsTr("Postscript");
        case Mimer.Mimer.Plaintext:
            return qsTr("Plaintext");
        case Mimer.Mimer.PWG:
            return qsTr("PWG-raster");
        case Mimer.Mimer.URF:
            return qsTr("URF-raster");
        case Mimer.Mimer.PNG:
            return qsTr("PNG");
        case Mimer.Mimer.JPEG:
            return qsTr("JPEG");
        case Mimer.Mimer.GIF:
            return qsTr("GIF");
        case Mimer.Mimer.SVG:
            return qsTr("SVG");
        case Mimer.Mimer.RBMP:
            return qsTr("Reverse BMP");
        default:
            return value;
        }
    case "media-top-margin":
    case "media-bottom-margin":
    case "media-left-margin":
    case "media-right-margin":
        return ""+(value/100)+"mm"
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

var pdfTargets = [Mimer.Mimer.OctetStream, Mimer.Mimer.PDF, Mimer.Mimer.Postscript, Mimer.Mimer.PWG, Mimer.Mimer.URF];


function canConvertPdfTo(type)
{
    return has(pdfTargets, type)
}

function canTransferPostscriptAs(type)
{
    var targets = [Mimer.Mimer.OctetStream, Mimer.Mimer.Postscript];
    return has(targets, type)
}

function canConvertPlaintextTo(type)
{
    var targets = pdfTargets;
    targets.push(Mimer.Mimer.Plaintext);
    return has(targets, type)
}

function canConvertOfficeDocumentTo(type)
{
    return has(pdfTargets, type)
}

function canConvertSvgTo(type)
{
    return has(pdfTargets, type)
}

function canConvertImageTo(type)
{
    var targets = [Mimer.Mimer.OctetStream, Mimer.Mimer.JPEG, Mimer.Mimer.PNG, Mimer.Mimer.RBMP,
                   Mimer.Mimer.PWG, Mimer.Mimer.URF, Mimer.Mimer.PDF, Mimer.Mimer.Postscript];
    return has(targets, type)
}

function unitsIsDpi(resolution)
{
    return resolution.units == 3;
}

function knownPaperSize(mediaName)
{
    return Strings.media.hasOwnProperty(mediaName);
}


function fixupChoices(name, choices, mimeType, printer)
{
    switch(name) {
    case "document-format":
        if(mimeType == Mimer.Mimer.PDF)
        {
            return choices.filter(canConvertPdfTo)
        }
        else if(mimeType == Mimer.Mimer.Plaintext)
        {
            return choices.filter(canConvertPlaintextTo)
        }
        else if(mimeType == Mimer.Mimer.Postscript)
        {
            return choices.filter(canTransferPostscriptAs)
        }
        else if(Mimer.Mimer.isOffice(mimeType))
        {
            return choices.filter(canConvertOfficeDocumentTo)
        }
        else if(mimeType == Mimer.Mimer.SVG)
        {
            return choices.filter(canConvertSvgTo)
        }
        else if(Mimer.Mimer.isImage(mimeType))
        {
            return choices.filter(canConvertImageTo);
        }
        else
        {
            return [Mimer.Mimer.OctetStream];
        }
    case "printer-resolution":
        return choices.filter(unitsIsDpi);
    case "multiple-document-handling" :
        // Only collation settings valid, multiple documents not used
        return choices.filter(function(elem) {return elem.indexOf("collated") != -1});
    case "number-up":
        if(choices.constructor.name === "Object")
        {
            var choice_array = [];
            for(var i=choices.low; i <= choices.high; i++)
            {
                choice_array.push(i);
            }
            return choice_array;
        }
        else
        {
            return choices;
        }
    case "media":
        if(printer.attrs.hasOwnProperty("media-ready"))
        {
            for(var m in printer.attrs["media-ready"].value)
            {
                var value = printer.attrs["media-ready"].value[m];
                if(!has(choices, value))
                {
                    choices.push(value);
                }
            }
        }
        return choices;
    default:
        return choices;
    }
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
