function supported_formats(printer)
{
    var formats = printer.attrs["document-format-supported"].value;
    var supported = [];
     if(has(formats, "application/pdf"))
     {
         supported.push("PDF");
     }
     if(has(formats, "image/jpeg"))
     {
         supported.push("JPEG");
     }

     if(supported.length == 0)
     {
         supported.push("No relevant formats supported")
     }

     return supported.join(" ");
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
