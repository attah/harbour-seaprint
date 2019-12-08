function supported_formats(printer)
{
    return _formats(printer).supported;
}

function format_filters(printer)
{
    return _formats(printer).filters;
}

function _formats(printer)
{
    var formats = printer.attrs["document-format-supported"].value;
    var supported = [];
    var filters = [];
     if(has(formats, "application/pdf"))
     {
         supported.push("PDF");
         filters.push("*.pdf");
     }
     if(has(formats, "image/jpeg"))
     {
         supported.push("JPEG");
         filters.push("*.jpg");
         filters.push("*.jpeg");
     }

     if(supported.length == 0)
     {
         supported.push("No relevant formats supported")
     }

     return {supported: supported.join(" "), filters: filters};
}

function has(arrayish, what)
{
    for(var i in arrayish)
    {
        if(arrayish[i] == what)
            return true
    }
    return false
}

function can_print(printer, fileName)
{
    // Move to ippprinter?
    return format_filters(printer).indexOf("*."+fileName.split('.').pop()) != -1;
}
