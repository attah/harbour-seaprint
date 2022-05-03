# SeaPrint
Network printing for Sailfish OS

![SeaPrint icon](icons/172x172/harbour-seaprint.png)

Print PDFs and images over IPP (Internet Printing Protocol).

Most, but not all, IPP-capable printers are supported.

A minority of printers support only PCL and/or their own proprietary formats, these are not supported.

## Format support

For PDFs, your printer needs to support one of the following formats:
 * PDF
 * Postscript
 * PWG-raster
 * URF-raster

For Postscript, your printer needs to support Postscript natively. (SeaPrint just sends it as-is)

For Plaintext, SeaPrint can render it to PDF internally and then the requirements follow those of PDF.
You can also send Plaintext to supporting printers and leave the rendering up to the printer (SeaPrint will strip leading and trailing Form Feed characters and ensure CR-NL newlines, but not do any reflow or similar). Since rendering will differ between printers, this has lowest priority for automatic format selection.

For printing "Office" files, like .doc(x) and odt, you need to install
 `calligraconverter` (from the `calligra` package in Sailfish OS 4.2 onwards).
 Then the printer support follows that of PDF.

For images, your printer needs to support one of the following formats:
 * PNG
 * JPEG
 * PWG-raster
 * URF-raster
 * PDF
 * Postscript

For printing JPEG images, transferring the unaltered image has priority and the printer does the scaling.
JPEGs are however (losslessly) re-encoded to the baseline encoding profile to work with all printers (since SeaPrint 1.1).
Other image formats are lossless and SeaPrint does scale-and-rotate to fit, so JPEG has lowest prio there.

Printers with any of these IPP certifications and derivative standards should likely be supported:

 * IPP Everywhere
 * AirPrint
 * Mopria
 * WiFi Direct

(obviously SeaPrint is unaffiliated and uncertified)

## Dealing with misbehaving printers

SeaPrint, in contrast to other printing clients, is not concerned with dealing with the idiosyncrasies of individual printers.
If some manufacturer can't make a reasonably well-behaved printer that's on them, not something for SeaPrint to cover for.

There is however a possibility for you, the user, to override certain attributes (to fix names, claimed format support etc.) in the IPP data structure.

To see what your printer sent, aka "the debug info", tap it 5 times rapidly without any document selected. The format is a SeaPrint-specific JSON equivalent representation of the IPP binary data.

Create the file `/home/$USER/.config/net.attah/seaprint/overrides`. (NB: New location for 1.0, and may move again)

This file should contain a JSON structure, with the outermost key(s) being "what attribute to match on" (uuid recommended if available), the next level is "if this value matches". Everything beneath that is what to inject/replace into the printer's attributes (see debug info above).

Example:
```JSON
{
  "printer-uuid": {
    "urn:uuid:xxx-yyy-zzz-111-1234567890": {
      "printer-name": {
        "tag": 54,
        "value": "A better name"
      },
      "document-format-supported": {
        "tag": 73,
        "value": [
          "application/octet-stream", "application/pdf"
        ]
      },
      "some-other-attribute-name": {
        "tag": 42,
        "value": "some-value"
      }
    },
    "urn:uuid:other-printer-uuid": {
      ...
    }
  },
  "other-attribute-to-match-other-printer-on": {
    ...
  }
}
```

## Dealing with printers using other protocols
SeaPrint only supports IPP, not port 9100 pjl/pcl raw, lpd/lpr or anything else.

Just use a PAPPL or CUPS as an intermediary. A shared printer there is IPP by definition.
